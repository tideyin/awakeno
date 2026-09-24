// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef AUDIO_DAC_H
#define AUDIO_DAC_H

/*******************************************************************************
 * Analog audio out: PA4 = DAC_OUT0 → amp (jumper-selected path)
 *
 * AT+AUDTST=2 : 1 kHz sine via DAC (~3 s)
 * AT+AUDTST=3 : play embedded wavetestdata[] (wave_data.h) via DAC
 *
 * Sample clock: TIMER6 update IRQ
 *******************************************************************************/

#include "gd32e10x.h"
#include "wave_data.h"

#define AUDIO_DAC_PIN           GPIO_PIN_4
#define AUDIO_DAC_PORT          GPIOA
#define AUDIO_DAC_DEFAULT_FS    16000u

void audio_dac_init(void);
void audio_dac_deinit(void);
void audio_dac_stop(void);

/* tone: freq_hz for duration_ms; lut=NULL → sine. returns 0 on start ok */
int audio_dac_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut);
int audio_dac_tone_1k_3s(void);

/* play WAV PCM (stereo→mono mix); src = embed or SPI flash @0; 0=ok */
int audio_dac_wave_play(audio_wav_src_t src);

int audio_dac_busy(void);
void audio_dac_poll(void);

#endif /* AUDIO_DAC_H */
