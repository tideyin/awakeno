#ifndef WAVE_DATA_H
#define WAVE_DATA_H

#include <stdint.h>

extern const char wavetestdata[];
extern const uint32_t wavetestdata_size;

typedef enum {
	AUDIO_WAV_SRC_EMBED = 0,
	AUDIO_WAV_SRC_SPI   = 1   /* WAV image at SPI flash offset 0 */
} audio_wav_src_t;

#define AUDIO_WAV_SPI_OFF   0u

int audio_wav_read(audio_wav_src_t src, uint32_t off, uint8_t *dst, uint32_t len);

/* sequential header helper: read nbytes at *idx and advance */
uint32_t audio_wav_read_unit(audio_wav_src_t src, uint32_t *idx,
			     uint8_t nbytes, int little_endian);

/* PCM stream for IRQ playback (one user at a time); fill from main loop */
void audio_wav_stream_start(audio_wav_src_t src, uint32_t pcm_off, uint32_t pcm_len);
void audio_wav_stream_stop(void);
void audio_wav_stream_fill(void);
int audio_wav_stream_get16(int16_t *out); /* 0=ok, -1=underrun (or empty) */
int audio_wav_stream_eof(void);           /* file drained and queue empty */

#endif /* WAVE_DATA_H */
