#ifndef TONE_H
#define TONE_H

/*******************************************************************************
 * Matrix keypad tones (jianpu) — Util/tone
 *
 * Scale (16 keys, ROW-major, +1 octave): 低5 低6 低7 1…高6 (G4…A6)
 * Each tone duration: TONE_DURATION_MS (0.3 s)
 *
 * Waveforms (256 pts): sine / piano; select via tone_wave_t.
 * Output path: PWM / DAC / I2S; select via tone_out_t (tone_set_out).
 *******************************************************************************/

#include <stdint.h>

#define TONE_KEY_NUM            16u
#define TONE_DURATION_MS        300u
#define TONE_SAMPLE_RATE        16000u
#define TONE_WAVE_LEN           256u
#define TONE_WAVE_MASK          (TONE_WAVE_LEN - 1u)
/* legacy aliases */
#define TONE_SINE_LEN           TONE_WAVE_LEN
#define TONE_SINE_MASK          TONE_WAVE_MASK

typedef enum {
	TONE_OUT_PWM = 0,
	TONE_OUT_DAC,
	TONE_OUT_I2S
} tone_out_t;

typedef enum {
	TONE_WAVE_SINE = 0,
	TONE_WAVE_PIANO
} tone_wave_t;

/* 256-point wavetables (int16), shared by DAC / PWM / I2S DDS */
extern const int16_t tone_sine_wave[TONE_WAVE_LEN];
extern const int16_t tone_piano_wave[TONE_WAVE_LEN];

const int16_t *tone_wave_lut(tone_wave_t wave);

/* default timbre / output path for matrix (tone_play_key) */
void tone_set_wave(tone_wave_t wave);
tone_wave_t tone_get_wave(void);
void tone_set_out(tone_out_t out);
tone_out_t tone_get_out(void);

/* DDS: phase_inc = freq * TONE_WAVE_LEN * 2^16 / fs (uint64, no overflow) */
static inline uint32_t tone_dds_phase_inc(uint32_t freq_hz, uint32_t fs)
{
	uint32_t inc;

	if ((freq_hz == 0u) || (fs == 0u)) {
		return 1u;
	}
	inc = (uint32_t)(((uint64_t)freq_hz * (uint64_t)TONE_WAVE_LEN * 65536ull) /
			 (uint64_t)fs);
	return (inc == 0u) ? 1u : inc;
}

/* Hz for key index 0..15 (ROW*4+COL) */
extern const uint16_t tone_freq_hz[TONE_KEY_NUM];

int tone_index_from_key(uint8_t row, uint8_t col);
uint16_t tone_freq_from_key(uint8_t row, uint8_t col);

/* play 0.3 s tone; out = PWM/DAC/I2S, wave = SINE/PIANO */
int tone_play(tone_out_t out, uint8_t row, uint8_t col, tone_wave_t wave);
/* matrix helper: uses tone_get_out() + tone_get_wave() */
int tone_play_key(uint8_t row, uint8_t col);
int tone_play_pwm(uint8_t row, uint8_t col, tone_wave_t wave);
int tone_play_dac(uint8_t row, uint8_t col, tone_wave_t wave);
int tone_play_i2s(uint8_t row, uint8_t col, tone_wave_t wave);

#endif /* TONE_H */
