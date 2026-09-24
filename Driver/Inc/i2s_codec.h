// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef I2S_CODEC_H
#define I2S_CODEC_H

/*******************************************************************************
 * I2S2 �?ES7134LV DAC �?NS4165 amp (Awakeno)
 *
 * ES7134LV: Philips I2S only, MCLK=CLKIN (256×Fs with GD32 MCKO),
 *           SCLK/LRCK=64 �?use 16-bit data in 32-bit channel.
 *
 * Pins (SPI2 default �?do NOT set GPIO_SPI2_REMAP):
 *   PA15 = WS/LRCK,  PB3 = CK/SCLK,  PB5 = SD/SDATA,  PC7 = MCK/CLKIN
 * Amp:
 *   PB2  = /PA_CTL (BOOT1), active-low enable
 *******************************************************************************/

#include "gd32e10x.h"
#include "rlk_gpio.h"
#include "wave_data.h"

/* WAV / RIFF constants (wave_data.h PCM asset) */
#define CHUNKID             0x52494646u
#define FILEFORMAT          0x57415645u
#define FORMATID            0x666D7420u
#define DATAID              0x64617461u
#define FACTID              0x66616374u
#define WAVE_FORMAT_PCM     0x01u
#define FORMATCHUNKSIZE     0x10u
#define CHANNEL_MONO        0x01u
#define CHANNEL_STEREO      0x02u
#define BITS_PER_SAMPLE_16  16u

/* ES7134LV: Philips I2S + MCLK on; 48 kHz typical in datasheet */
#define I2S_STANDARD        I2S_STD_PHILLIPS
#define I2S_MCLKOUTPUT      I2S_MCKOUT_ENABLE
#define I2S_FRAME_FMT       I2S_FRAMEFORMAT_DT16B_CH32B
/* sibling boards use HIGH; ES7134 also accepts LOW �?flip in tone test via amp log */
#define I2S_CK_POLARITY     I2S_CKPL_HIGH
#define I2S_DEFAULT_RATE    48000u

/* WAV playback tweaks (AT+AUDTST=1 only; tone test unchanged) */
#define I2S_WAV_GAIN        3u   /* digital gain, soft-clip to int16 */
#define I2S_WAV_SPEED_NUM   1u   /* play_rate = file_rate * NUM / DEN */
#define I2S_WAV_SPEED_DEN   1u   /* 1/1 = normal speed */

/* /PA_CTL �?PB2 / BOOT1, active low */
#define I2S_AMP_PORT        PA_CTL_PORT
#define I2S_AMP_PIN         PA_CTL_PIN

typedef struct {
	uint32_t riffchunksize;
	uint16_t formattag;
	uint16_t numchannels;
	uint32_t samplerate;
	uint32_t byterate;
	uint16_t blockalign;
	uint16_t bitspersample;
	uint32_t datasize;
} wave_file_struct;

typedef enum {
	VALID_WAVE_FILE = 0,
	UNVALID_RIFF_ID,
	UNVALID_WAVE_FORMAT,
	UNVALID_FORMATCHUNK_ID,
	UNSUPPORETD_FORMATTAG,
	UNSUPPORETD_NUMBER_OF_CHANNEL,
	UNSUPPORETD_SAMPLE_RATE,
	UNSUPPORETD_BITS_PER_SAMPLE,
	UNVALID_DATACHUNK_ID,
	UNSUPPORETD_EXTRAFORMATBYTES,
	UNVALID_FACTCHUNK_ID
} errorcode_enum;

typedef enum {
	littleendian,
	bigendian
} endianness_enum;

/* GPIO + I2S2 master TX @ sample_rate; amp left disabled */
void i2s_codec_init(uint32_t sample_rate);
void i2s_codec_deinit(void);
void i2s_codec_amp_enable(int enable);

/* play WAV via SPI2 TBE IRQ; src = embed or SPI flash @0; non-blocking */
errorcode_enum i2s_audio_play(audio_wav_src_t src);
void i2s_audio_stop(void);

/* start stereo tone (SPI2 TBE IRQ); lut=NULL ? sine; call i2s_codec_poll() in main */
int i2s_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut);
int i2s_tone_1k_3s(void);
int i2s_tone_busy(void);
void i2s_codec_poll(void);

#endif /* I2S_CODEC_H */
