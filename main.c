#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sndfile.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>


#include "wav.c"
#include "ftype_detect.c"

#define MINIMP3_ONLY_MP3
#define MINIMP3_USE_SIMD
#define MINIMP3_IMPLEMENTATION


#ifdef MINIMP3_FLOAT_OUTPUT
    typedef float W_D_TYPE;
    #define write_wave write_float_wav
#else
    typedef short W_D_TYPE;
    #define write_wave write_pcm_wav
#endif


#include "minimp3.h"

typedef struct {
    size_t num_samples;
    size_t channels;
    void *samples;
    float sample_rate;
} audio_data;



#define MAX_SLICES 400
#define DELIMITER ","
#define MAX_FN_LENGTH 512

#define AUTO_MODE "AUTO"
#define MAX_FILENAME 256

typedef enum {
    CUSTOM_MODE,
    AUTO_MODE_CUSTOM_TIMES,
    FIXED_LENGTH_MODE
} split_mode_t;


typedef struct {
    audio_data *audio;
    float lengths[2];
    char output_str[MAX_FN_LENGTH];
} thread_args_t;



void read_file(const char *filename, uint64_t *size, uint8_t **data) {
    *size = 0;
    *data = NULL;

    FILE *fin = fopen(filename, "rb");
    
    if (!fin) {
        printf("\nError opening input file\n");
        perror("fopen");
        return;
    }

    struct stat st;
    if (fstat(fileno(fin), &st) != 0) {
        perror("fstat");
        fclose(fin);
        return;
    }

    *size = (uint64_t)st.st_size;

    *data = malloc(*size);

    if (!*data) {
        fprintf(stderr, "Memory allocation failed\n");
        *size = 0;
        fclose(fin);
        return;
    }

    if (fread(*data, 1, *size, fin) != *size) {
        perror("fread");
        free(*data);
        *data = NULL;
        *size = 0;
    }

    fclose(fin);
}


audio_data read_wav(const char *filename){

    audio_data audio = {0};
    
    SNDFILE *file;
    SF_INFO sf_info;

    file = sf_open(filename, SFM_READ, &sf_info);
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return audio ;
    }

    audio.num_samples = (size_t)sf_info.frames * sf_info.channels;
    audio.samples     = (float*)malloc(audio.num_samples * sizeof(float));

    if (!audio.samples) {
        fprintf(stderr, "Memory allocation failed\n");
        free(audio.samples);
        audio.samples = NULL;
        sf_close(file);
        return audio ;
    }

    if (sf_readf_float(file,audio.samples, sf_info.frames) < sf_info.frames) {
        fprintf(stderr, "Error reading audio data\n");
        free(audio.samples);
        audio.samples = NULL;
        sf_close(file);
        return audio ;
    }

    audio.sample_rate = sf_info.samplerate;
    audio.channels    = sf_info.channels;

    sf_close(file);

    return audio;
}

audio_data read_mp3(char *input_filename) {
    
    audio_data audio  = {0};

    static mp3dec_t mp3d;
    mp3dec_init(&mp3d);

    uint64_t buf_size  = 0;
    uint8_t *input_buf = NULL;

    read_file(input_filename, &buf_size, &input_buf);

    if (!input_buf) {
        fprintf(stderr, "Failed to read input file: %s\n", input_filename);
        return audio;
    }

    size_t data_size       = sizeof(W_D_TYPE);
    size_t max_pcm_samples = (buf_size * MINIMP3_MAX_SAMPLES_PER_FRAME) / 128; // (mp3 max size estimate)
    size_t pcm_bsiz        = max_pcm_samples * data_size * 2;

    audio.samples = malloc(pcm_bsiz);
    if (!audio.samples) {
        fprintf(stderr, "Memory allocation failed\n");
        free(input_buf);
        return audio;
    }

    W_D_TYPE *full_pcm = (W_D_TYPE *)audio.samples; 

    uint64_t decoded_samples = 0;
    size_t remaining_size = buf_size;

    while (remaining_size > 0) {
        mp3dec_frame_info_t info;
        W_D_TYPE pcm[MINIMP3_MAX_SAMPLES_PER_FRAME * 2];

        int samples = mp3dec_decode_frame(&mp3d, input_buf, remaining_size, pcm, &info);

        if (info.frame_bytes == 0 || remaining_size < info.frame_bytes) {
            break;
        }

        if (samples > 0) {
            if ((decoded_samples + samples) * 2 > max_pcm_samples) {
                fprintf(stderr, "PCM buffer overflow prevented\n");
                break;
            }

            size_t copy_size = samples * data_size * info.channels;
            memcpy(full_pcm + (decoded_samples * info.channels), pcm, copy_size);

            audio.channels = info.channels;
            audio.sample_rate = info.hz;
            decoded_samples += samples;
        }

        input_buf += info.frame_bytes;
        remaining_size -= info.frame_bytes;
    }

    audio.num_samples = decoded_samples;

    return audio;
}

void *write_wave_thread(void *arg) {
    thread_args_t *args    = (thread_args_t *)arg;
    audio_data *audio      = args->audio;
    float *lengths         = args->lengths;
    char *output_str       = args->output_str;
    size_t data_size       = sizeof(W_D_TYPE);

    uint64_t start_sample  = (uint64_t)(lengths[0] * audio->sample_rate) * audio->channels;
    uint64_t end_sample    = (uint64_t)(lengths[1] * audio->sample_rate) * audio->channels;
    uint64_t slice_samples = end_sample - start_sample;

    W_D_TYPE *slice = malloc(slice_samples * data_size);

    if (!slice) {
        fprintf(stderr, "Memory allocation failed for slice\n"); // Removed slice number
        pthread_exit(NULL);
        return NULL;  
    }

    memcpy(slice, (W_D_TYPE *)audio->samples + start_sample, slice_samples * data_size);
    char output_filename[780];
    snprintf(output_filename, sizeof(output_filename), "%s.wav", output_str);

    write_wave(output_filename, slice, slice_samples /audio->channels, audio->channels, audio->sample_rate);

    free(slice);
    pthread_exit(NULL);
    return NULL;
}

void async_sliced_write_wave(audio_data *audio, float lengths[][2], unsigned short length, char output_strs[][MAX_FN_LENGTH]) {

    if(!audio->channels)
      audio->channels = 1;

    pthread_t threads[length]; 
    thread_args_t thread_args[length];
    

    for (int i = 0; i < length; i++) {
        thread_args[i].audio = audio;
        memcpy(thread_args[i].lengths, lengths[i], sizeof(float) * 2);
        strncpy(thread_args[i].output_str, output_strs[i], MAX_FN_LENGTH - 1); 
        thread_args[i].output_str[MAX_FN_LENGTH - 1] = '\0'; 

        int rc = pthread_create(&threads[i], NULL, write_wave_thread, &thread_args[i]);
        if (rc) {
            fprintf(stderr, "Error creating thread %d, return code is %d\n", i, rc);
            break; 
        }
    }

    for (int i = 0; i < length; i++) {
        pthread_join(threads[i], NULL);
    }
}



void sliced_write_wave(audio_data *audio, float lengths[][2], unsigned short length, char output_strs[][MAX_FN_LENGTH]) {

    size_t data_size = sizeof(W_D_TYPE);
    
    for (int i = 0; i < length; i++) {
        if (lengths[i][0] < 0 || lengths[i][1] <= lengths[i][0] || 
            lengths[i][1] > (float)audio->num_samples / audio->sample_rate) {
            fprintf(stderr, "Invalid time range for slice %d: [%f, %f]\n", i + 1, lengths[i][0], lengths[i][1]);
            continue;
        }

        uint64_t start_sample = (uint64_t)(lengths[i][0] * audio->sample_rate) * audio->channels;
        uint64_t end_sample    = (uint64_t)(lengths[i][1] * audio->sample_rate) * audio->channels;
        uint64_t slice_samples = end_sample - start_sample;

        W_D_TYPE *slice = malloc(slice_samples * data_size);

        if (!slice) {
            fprintf(stderr, "Memory allocation failed for slice %d\n", i + 1);
            continue;
        }

        memcpy(slice, (W_D_TYPE *)audio->samples + start_sample, slice_samples * data_size);
        char output_filename[780];
        snprintf(output_filename, sizeof(output_filename), "%s.wav", output_strs[i]);

        write_wave(output_filename, slice, slice_samples / audio->channels, audio->channels, audio->sample_rate);

        free(slice);
    }
}
int is_numeric(const char *str) {
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

split_mode_t detect_split_mode(const char *outputs, const char *starts) {
    if (strcmp(outputs, AUTO_MODE) == 0) {
        if (is_numeric(starts)) {
            return FIXED_LENGTH_MODE;
        }
        return AUTO_MODE_CUSTOM_TIMES;
    }
    return CUSTOM_MODE;
}

void generate_auto_filename(char *output, const char *input_filename, int index) {
    snprintf(output, MAX_FN_LENGTH, "%s_%d", input_filename, index + 1);
}

unsigned short get_lengths(char *output_fns, char *starts, char *ends,float lengths[][2], char output_strs[][MAX_FN_LENGTH],const char *input_filename, audio_data *audio) {
    
    split_mode_t mode = detect_split_mode(output_fns, starts);
    unsigned short count = 0;

    switch (mode) {
        case FIXED_LENGTH_MODE: {
            float segment_length = atof(starts);
            float total_duration = (float)audio->num_samples / (audio->sample_rate * audio->channels);
            float current_time = 0;

            while (current_time < total_duration && count < MAX_SLICES) {
                lengths[count][0] = current_time;
                lengths[count][1] = current_time + segment_length;
                if (lengths[count][1] > total_duration) {
                    lengths[count][1] = total_duration;
                }
                generate_auto_filename(output_strs[count], ends, count);
                current_time += segment_length;
                count++;
            }
            break;
        }

        case AUTO_MODE_CUSTOM_TIMES: {
            char *rest_starts = starts;
            char *rest_ends = ends;
            char *start_token = strtok_r(rest_starts, DELIMITER, &rest_starts);
            char *end_token = strtok_r(rest_ends, DELIMITER, &rest_ends);

            while (start_token && end_token && count < MAX_SLICES) {
                lengths[count][0] = atof(start_token);
                lengths[count][1] = atof(end_token);
                generate_auto_filename(output_strs[count], input_filename, count);
                count++;
                start_token = strtok_r(NULL, DELIMITER, &rest_starts);
                end_token = strtok_r(NULL, DELIMITER, &rest_ends);
            }
            break;
        }

        case CUSTOM_MODE: {
            char *rest_starts = starts;
            char *rest_ends = ends;
            char *rest_output_fns = output_fns;
            char *start_token = strtok_r(rest_starts, DELIMITER, &rest_starts);
            char *end_token = strtok_r(rest_ends, DELIMITER, &rest_ends);
            char *output_fn_token = strtok_r(rest_output_fns, DELIMITER, &rest_output_fns);

            while (start_token && end_token && output_fn_token && count < MAX_SLICES) {
                lengths[count][0] = atof(start_token);
                lengths[count][1] = atof(end_token);
                strncpy(output_strs[count], output_fn_token, MAX_FN_LENGTH - 1);
                output_strs[count][MAX_FN_LENGTH - 1] = '\0';
                count++;
                start_token = strtok_r(NULL, DELIMITER, &rest_starts);
                end_token = strtok_r(NULL, DELIMITER, &rest_ends);
                output_fn_token = strtok_r(NULL, DELIMITER, &rest_output_fns);
            }
            break;
        }
    }

    return count;
}



int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_file> <outputs> <starts> <ends>\n", argv[0]);
        fprintf(stderr, "Modes:\n");
        fprintf(stderr, "1. Custom names: <names> <start_times> <end_times>\n");
        fprintf(stderr, "2. Auto names: AUTO <start_times> <end_times>\n");
        fprintf(stderr, "3. Fixed length: AUTO <segment_length> \"\"\n");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    float lengths[MAX_SLICES][2];
    char out_fns[MAX_SLICES][MAX_FN_LENGTH];

    char *input_filename = argv[1];
    char *output_fns = strdup(argv[2]);
    char *starts = strdup(argv[3]);
    char *ends = strdup(argv[4]);

    if (!starts || !ends || !output_fns) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    audio_type type = detect_audio_type(input_filename);
    audio_data audio = {0};

    switch (type) {
        case 1:
            audio = read_mp3(input_filename);
            break;
        case 2:
            audio = read_wav(input_filename);
            break;
        default:
            fprintf(stderr, "Unsupported audio format\n");
            return 1;
    }

    unsigned int length = get_lengths(output_fns, starts, ends, lengths, out_fns, input_filename, &audio);
    
    async_sliced_write_wave(&audio, lengths, length, out_fns);

    // sliced_write_wave(&audio, lengths, length, out_fns);


    clock_gettime(CLOCK_MONOTONIC, &end);
    long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    printf("\nTime taken: %ld microseconds\n", elapsed_time);

    free(starts);
    free(ends);
    free(output_fns);
    free(audio.samples);

    return 0;
}
