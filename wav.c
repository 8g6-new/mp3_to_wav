#include <stdint.h>

#define WAV_FORMAT_PCM   0x0001
#define WAV_FORMAT_FLOAT 0x0003

#pragma pack(push, 1)
typedef struct {
    char     riff[4];         /* "RIFF"                                  */
    uint32_t file_length;     /* file length in bytes                    */
    char     wave[4];         /* "WAVE"                                  */
    char     fmt[4];          /* "fmt "                                  */
    uint32_t chunk_size;      /* size of FMT chunk in bytes (usually 16) */
    uint16_t format_tag;      /* 1=PCM, 3=IEEE float                    */
    uint16_t num_channels;    /* 1=mono, 2=stereo                       */
    uint32_t sample_rate;     /* Sampling rate in samples per second     */
    uint32_t bytes_per_sec;   /* bytes per second                       */
    uint16_t block_align;     /* bytes per sample * num_channels        */
    uint16_t bits_per_sample; /* Number of bits per sample              */
    char     data[4];         /* "data"                                 */
    uint32_t data_length;     /* data length in bytes                   */
} wav_header;
#pragma pack(pop)


static void init_wav_header(wav_header* header, int format_tag, int channels, int sample_rate, int bits_per_sample, uint32_t data_length) {
    memcpy(header->riff, "RIFF", 4);
    memcpy(header->wave, "WAVE", 4);
    memcpy(header->fmt,  "fmt ", 4);
    memcpy(header->data, "data", 4);

    header->chunk_size      = 16; 
    header->format_tag      = format_tag;
    header->num_channels    = channels;
    header->sample_rate     = sample_rate;
    header->bits_per_sample = bits_per_sample;
    header->block_align     = channels * (bits_per_sample / 8);
    header->bytes_per_sec   = sample_rate * header->block_align;
    header->data_length     = data_length;
    header->file_length     = data_length + sizeof(wav_header) - 8;  
}

int write_pcm_wav(const char *filename, const int16_t *pcm, uint32_t sample_count, int channels, int sample_rate) {
    
    wav_header header;
    uint32_t data_length = sample_count * channels * sizeof(int16_t);
    
    init_wav_header(&header, WAV_FORMAT_PCM, channels, sample_rate, 16, data_length);

    FILE *fout = fopen(filename, "wb");
    if (!fout) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fwrite(&header, sizeof(header), 1, fout) != 1) {
        perror("Error writing WAV header");
        fclose(fout);
        return -1;
    }

    if (fwrite(pcm, 1, data_length, fout) != data_length) {
        perror("Error writing WAV data");
        fclose(fout);
        return -1;
    }
    else{
        printf("%s PCM 16bit WAV file written successfully.\n",filename);
    }


    fclose(fout);
    return 0;
}

int write_float_wav(const char *filename, const float *pcm, uint32_t sample_count,int channels, int sample_rate) {
    
    wav_header header;
    uint32_t data_length = sample_count * channels * sizeof(float);

    init_wav_header(&header,WAV_FORMAT_FLOAT, channels, sample_rate, 32, data_length);
    
    FILE *fout = fopen(filename, "wb");
    if (!fout) {
        perror("Error opening file for writing");
        return -1;
    }

    if (fwrite(&header, sizeof(header), 1, fout) != 1) {
        perror("Error writing WAV header");
        fclose(fout);
        return -1;
    }

    if (fwrite(pcm, 1, data_length, fout) != data_length) {
        perror("Error writing WAV data");
        fclose(fout);
        return -1;
    }
    else{
        printf("%s Float 32 bit WAV file written successfully.\n",filename);
    }

    fclose(fout);
    return 0;
}