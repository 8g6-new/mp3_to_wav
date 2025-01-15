#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/stat.h>
#define MINIMP3_ONLY_MP3
#define MINIMP3_USE_SIMD
#define MINIMP3_IMPLEMENTATION
#define MINIMP3_FLOAT_OUTPUT

#ifdef MINIMP3_FLOAT_OUTPUT
    typedef float DATA;
    #define writeWave write_float_wav
#else
    typedef short DATA;
    #define writeWave write_pcm_wav
#endif

#include "minimp3.h"

#include "wav.c"

#define MAX_SLICES 200
#define DELIMITER ","
#define MAX_FN_LENGTH 512

typedef struct  {
    int sr;
    int channels;
} info;

void readFile(const char *filename, uint64_t *size, uint8_t **data) {
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

info slice_mp3_to_wav(char *input_filename, char *output_fn, float lengths[][2], unsigned short length, char output_strs[][MAX_FN_LENGTH]) {

    info INFO               = {0};

    static mp3dec_t mp3d;
    mp3dec_init(&mp3d);

    uint64_t buf_size       = 0;
    uint8_t *input_buf      = NULL;
    uint8_t *orig_input_buf = NULL;  

    readFile(input_filename, &buf_size, &input_buf);
    if (!input_buf) {
        fprintf(stderr, "Failed to read input file: %s\n", input_filename);
        return INFO;
    }

    orig_input_buf         = input_buf;  

    size_t data_size       = sizeof(DATA); 

    size_t max_pcm_samples = (buf_size * MINIMP3_MAX_SAMPLES_PER_FRAME)/ 128; 
    size_t pcm_bsiz        = max_pcm_samples * data_size * 2;
    DATA *full_pcm         = malloc(pcm_bsiz);
    
    if (!full_pcm) {
        fprintf(stderr, "Memory allocation failed for full_pcm\n");
        free(orig_input_buf);
        return INFO;
    }

    uint64_t decoded_samples = 0;
    size_t remaining_size = buf_size;

    while (remaining_size > 0) {
        mp3dec_frame_info_t info;
        DATA pcm[MINIMP3_MAX_SAMPLES_PER_FRAME * 2]; 
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
            
            INFO.channels   = info.channels;
            INFO.sr         = info.hz;
            decoded_samples += samples;

            // printf("Frame bytes: %d\n", info.frame_bytes);
            // printf("Channels: %d\n", info.channels);
            // printf("Sample rate: %d Hz\n", info.hz);
            // printf("Layer: %d\n", info.layer);
            // printf("Bitrate: %d kbps\n", info.bitrate_kbps);
        }

        input_buf           += info.frame_bytes;
        remaining_size      -= info.frame_bytes;
    }
    // if (output_fn) {
    //     char full_output[512];
    //     snprintf(full_output, sizeof(full_output), "%s_full.wav", output_fn);
    //     writeWave(full_output, full_pcm, decoded_samples, INFO.channels, INFO.sr, INFO.bit_rate);
    // }

    for (int i = 0; i < length; i++) {
        if (lengths[i][0] < 0 || lengths[i][1] <= lengths[i][0] || 

            lengths[i][1] > (float)decoded_samples / INFO.sr) {
            fprintf(stderr, "Invalid time range for slice %d: [%f, %f]\n", i + 1, lengths[i][0], lengths[i][1]);
            continue;
        }

        uint64_t start_sample  = (uint64_t)(lengths[i][0] * INFO.sr) * INFO.channels;
        uint64_t end_sample    = (uint64_t)(lengths[i][1] * INFO.sr) * INFO.channels;
        uint64_t slice_samples = end_sample - start_sample;

        DATA *slice = malloc(slice_samples * data_size);
        if (!slice) {
            fprintf(stderr, "Memory allocation failed for slice %d\n", i + 1);
            continue;
        }

        memcpy(slice, full_pcm + start_sample, slice_samples * data_size);

        char output_filename[780];
        snprintf(output_filename, sizeof(output_filename), "%s_slice_%d.wav", output_strs[i],i + 1);
        
        writeWave(output_filename, slice, slice_samples/ INFO.channels, INFO.channels, INFO.sr);

        free(slice);
    }

    free(full_pcm);
    free(orig_input_buf);
    return INFO;
}

unsigned short get_lengths(char *output_fns,char *starts, char *ends, float lengths[][2], char output_strs[][MAX_FN_LENGTH]) {
    unsigned short count = 0;

    char *rest_starts = starts;
    char *rest_ends   = ends;
    char *rest_output_fns = output_fns;

    char *start_token = strtok_r(rest_starts, DELIMITER, &rest_starts);
    char *end_token   = strtok_r(rest_ends, DELIMITER, &rest_ends);
    char *output_fn_token = strtok_r(rest_output_fns, DELIMITER, &rest_output_fns);

    while (start_token != NULL && end_token != NULL && output_fn_token != NULL && count < MAX_SLICES) {

        float start_value = atof(start_token);
        float end_value   = atof(end_token);

        lengths[count][0] = start_value;
        lengths[count][1] = end_value;

        strcpy(output_strs[count], output_fn_token);

        count++;

        start_token = strtok_r(NULL, DELIMITER, &rest_starts);
        end_token   = strtok_r(NULL, DELIMITER, &rest_ends);
        output_fn_token = strtok_r(NULL, DELIMITER, &rest_output_fns);
    }

    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input mp3> <outputs> <starts> <ends>\n", argv[0]);
        return 1;
    }
    
    float lengths[MAX_SLICES][2];
    char out_fns[MAX_SLICES][MAX_FN_LENGTH];

    char *input_filename  = argv[1];
    char *output_fns      = strdup(argv[2]);
    char *starts          = strdup(argv[3]);
    char *ends            = strdup(argv[4]);

    if (!starts || !ends) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    unsigned short length = get_lengths(output_fns, starts, ends, lengths, out_fns);

    slice_mp3_to_wav(input_filename, NULL, lengths, length, out_fns);

    free(starts);
    free(ends);
    free(output_fns);

    return 0;
}
