#include "tone.h"
#include "audio_pwm.h"
#include "audio_dac.h"
#include "i2s_codec.h"
#include "printf.h"

/*
 * Jianpu map @ A4=440, +1 octave from base scale:
 * 低5=G4 … 1=C5 … 高6=A6
 */
const uint16_t tone_freq_hz[TONE_KEY_NUM] = {
	392u,  /* ROW0 COL0  低5 G4 */
	440u,  /* ROW0 COL1  低6 A4 */
	494u,  /* ROW0 COL2  低7 B4 */
	523u,  /* ROW0 COL3  1   C5 */
	587u,  /* ROW1 COL0  2   D5 */
	659u,  /* ROW1 COL1  3   E5 */
	698u,  /* ROW1 COL2  4   F5 */
	784u,  /* ROW1 COL3  5   G5 */
	880u,  /* ROW2 COL0  6   A5 */
	988u,  /* ROW2 COL1  7   B5 */
	1047u, /* ROW2 COL2  高1 C6 */
	1175u, /* ROW2 COL3  高2 D6 */
	1319u, /* ROW3 COL0  高3 E6 */
	1397u, /* ROW3 COL1  高4 F6 */
	1568u, /* ROW3 COL2  高5 G6 */
	1760u  /* ROW3 COL3  高6 A6 */
};

/* one period sine — wave data for all tones (DDS / PWM / DAC / I2S) */
const int16_t tone_sine_wave[TONE_WAVE_LEN] = {
	     0,    804,   1608,   2410,   3212,   4011,   4808,   5602,
	  6393,   7179,   7962,   8739,   9512,  10278,  11039,  11793,
	 12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,
	 18204,  18868,  19519,  20159,  20787,  21403,  22005,  22594,
	 23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,
	 27245,  27683,  28105,  28510,  28898,  29268,  29621,  29956,
	 30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
	 32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,
	 32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
	 32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,
	 30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
	 27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,
	 23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,
	 18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
	 12539,  11793,  11039,  10278,   9512,   8739,   7962,   7179,
	  6393,   5602,   4808,   4011,   3212,   2410,   1608,    804,
	     0,   -804,  -1608,  -2410,  -3212,  -4011,  -4808,  -5602,
	 -6393,  -7179,  -7962,  -8739,  -9512, -10278, -11039, -11793,
	-12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530,
	-18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
	-23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
	-27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
	-30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
	-32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
	-32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
	-32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
	-30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
	-27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
	-23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868,
	-18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
	-12539, -11793, -11039, -10278,  -9512,  -8739,  -7962,  -7179,
	 -6393,  -5602,  -4808,  -4011,  -3212,  -2410,  -1608,   -804
};

/*
 * One-period piano-like wavetable: fund + harmonics 2..10 (additive).
 * Same length as sine; DDS frequency still from tone_freq_hz[].
 */
const int16_t tone_piano_wave[TONE_WAVE_LEN] = {
	     0,   3246,   6445,   9551,  12522,  15319,  17910,  20268,
	 22373,  24213,  25783,  27086,  28129,  28929,  29505,  29882,
	 30086,  30146,  30090,  29948,  29745,  29505,  29247,  28989,
	 28743,  28515,  28312,  28131,  27972,  27828,  27692,  27556,
	 27411,  27250,  27064,  26848,  26598,  26311,  25989,  25632,
	 25245,  24833,  24403,  23961,  23516,  23073,  22640,  22222,
	 21822,  21443,  21085,  20748,  20430,  20126,  19833,  19546,
	 19258,  18966,  18665,  18350,  18018,  17670,  17303,  16919,
	 16520,  16109,  15690,  15268,  14847,  14431,  14025,  13632,
	 13255,  12894,  12551,  12224,  11913,  11613,  11322,  11035,
	 10750,  10461,  10165,   9859,   9541,   9210,   8865,   8507,
	  8137,   7760,   7377,   6993,   6611,   6236,   5870,   5518,
	  5181,   4861,   4558,   4272,   4001,   3744,   3497,   3258,
	  3023,   2790,   2555,   2316,   2073,   1825,   1573,   1319,
	  1066,    818,    578,    353,    147,    -36,   -190,   -312,
	  -400,   -452,   -469,   -451,   -402,   -327,   -230,   -119,
	     0,    119,    230,    327,    402,    451,    469,    452,
	   400,    312,    190,     36,   -147,   -353,   -578,   -818,
	 -1066,  -1319,  -1573,  -1825,  -2073,  -2316,  -2555,  -2790,
	 -3023,  -3258,  -3497,  -3744,  -4001,  -4272,  -4558,  -4861,
	 -5181,  -5518,  -5870,  -6236,  -6611,  -6993,  -7377,  -7760,
	 -8137,  -8507,  -8865,  -9210,  -9541,  -9859, -10165, -10461,
	-10750, -11035, -11322, -11613, -11913, -12224, -12551, -12894,
	-13255, -13632, -14025, -14431, -14847, -15268, -15690, -16109,
	-16520, -16919, -17303, -17670, -18018, -18350, -18665, -18966,
	-19258, -19546, -19833, -20126, -20430, -20748, -21085, -21443,
	-21822, -22222, -22640, -23073, -23516, -23961, -24403, -24833,
	-25245, -25632, -25989, -26311, -26598, -26848, -27064, -27250,
	-27411, -27556, -27692, -27828, -27972, -28131, -28312, -28515,
	-28743, -28989, -29247, -29505, -29745, -29948, -30090, -30146,
	-30086, -29882, -29505, -28929, -28129, -27086, -25783, -24213,
	-22373, -20268, -17910, -15319, -12522,  -9551,  -6445,  -3246
};

static tone_wave_t s_tone_wave = TONE_WAVE_PIANO;
static tone_out_t s_tone_out = TONE_OUT_I2S;

const int16_t *tone_wave_lut(tone_wave_t wave)
{
	if (wave == TONE_WAVE_PIANO) {
		return tone_piano_wave;
	}
	return tone_sine_wave;
}

void tone_set_wave(tone_wave_t wave)
{
	s_tone_wave = (wave == TONE_WAVE_PIANO) ? TONE_WAVE_PIANO : TONE_WAVE_SINE;
}

tone_wave_t tone_get_wave(void)
{
	return s_tone_wave;
}

void tone_set_out(tone_out_t out)
{
	if (out == TONE_OUT_PWM) {
		s_tone_out = TONE_OUT_PWM;
	} else if (out == TONE_OUT_DAC) {
		s_tone_out = TONE_OUT_DAC;
	} else {
		s_tone_out = TONE_OUT_I2S;
	}
}

tone_out_t tone_get_out(void)
{
	return s_tone_out;
}

int tone_index_from_key(uint8_t row, uint8_t col)
{
	if ((row >= 4u) || (col >= 4u)) {
		return -1;
	}
	return (int)(row * 4u + col);
}

uint16_t tone_freq_from_key(uint8_t row, uint8_t col)
{
	int idx = tone_index_from_key(row, col);

	if (idx < 0) {
		return 0;
	}
	return tone_freq_hz[idx];
}

int tone_play(tone_out_t out, uint8_t row, uint8_t col, tone_wave_t wave)
{
	uint16_t freq = tone_freq_from_key(row, col);
	const int16_t *lut = tone_wave_lut(wave);
	int ret;

	if (freq == 0u) {
		return -1;
	}

	switch (out) {
	case TONE_OUT_PWM:
		ret = audio_pwm_tone_play(freq, TONE_DURATION_MS, lut);
		break;
	case TONE_OUT_DAC:
		ret = audio_dac_tone_play(freq, TONE_DURATION_MS, lut);
		break;
	case TONE_OUT_I2S:
		audio_pwm_stop();
		audio_dac_stop();
		ret = i2s_tone_play(freq, TONE_DURATION_MS, lut);
		break;
	default:
		ret = -1;
		break;
	}

	if (0 == ret) {
		printf_("\r\nTone R%uC%u %uHz %ums out=%u wave=%u",
			(unsigned)row, (unsigned)col, (unsigned)freq,
			(unsigned)TONE_DURATION_MS, (unsigned)out, (unsigned)wave);
	}
	return ret;
}

int tone_play_key(uint8_t row, uint8_t col)
{
	return tone_play(tone_get_out(), row, col, tone_get_wave());
}

int tone_play_pwm(uint8_t row, uint8_t col, tone_wave_t wave)
{
	return tone_play(TONE_OUT_PWM, row, col, wave);
}

int tone_play_dac(uint8_t row, uint8_t col, tone_wave_t wave)
{
	return tone_play(TONE_OUT_DAC, row, col, wave);
}

int tone_play_i2s(uint8_t row, uint8_t col, tone_wave_t wave)
{
	return tone_play(TONE_OUT_I2S, row, col, wave);
}
