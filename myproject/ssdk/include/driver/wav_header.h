#ifndef WAV_HEADER_H_
#define WAV_HEADER_H_
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT 0x20746d66
#define ID_DATA 0x61746164

/*44~  bytes
 */

typedef struct wav_header_s {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
    uint32_t fmt_id;
    uint32_t fmt_sz; // fmt size

    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} wav_header_t;

typedef struct data_header_s {
    uint32_t data_id;
    uint32_t data_sz;
} data_header_t;

#endif