// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef AUDIO_PWM_H
#define AUDIO_PWM_H

/*******************************************************************************
 * PWM audio out: PC6 = TIMER7_CH0 → amp (jumper-selected PWM_AUDIO)
 *
 * AT+AUDTST=4 : 1 kHz sine via PWM (~3 s)
 * AT+AUDTST=5 : play embedded wavetestdata[] via PWM duty
 *
 * Carrier: TIMER7_CH0 (~469 kHz, 8-bit duty)
 * Sample clock: TIMER2 update IRQ
 * Note: temporarily reconfigures TIMER7 (IR 38k on CH2 paused until stop)
 *******************************************************************************/

#include "gd32e10x.h"
#include "wave_data.h"

#define AUDIO_PWM_PIN           GPIO_PIN_6
#define AUDIO_PWM_PORT          GPIOC
#define AUDIO_PWM_DEFAULT_FS    16000u
#define AUDIO_PWM_ARR           255u   /* 8-bit duty; Fc ≈ SystemCoreClock/(ARR+1) */

void audio_pwm_init(void);
void audio_pwm_deinit(void);
void audio_pwm_stop(void);

int audio_pwm_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut);
int audio_pwm_tone_1k_3s(void);
int audio_pwm_wave_play(audio_wav_src_t src);

int audio_pwm_busy(void);
void audio_pwm_poll(void);

#endif /* AUDIO_PWM_H */
