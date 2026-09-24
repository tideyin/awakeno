// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "audio_dac.h"
#include "wave_data.h"
#include "i2s_codec.h"
#include "audio_pwm.h"
#include "tone.h"
#include "systick.h"
#include "printf.h"

/* mid-scale idle (12-bit DAC) */
#define DAC_MID           2048u

static wave_file_struct dac_wave;
static uint32_t dac_head_idx = 0;
static uint32_t dac_data_addr = 0;
static volatile uint32_t dac_pcm_idx = 0;
static audio_wav_src_t dac_wav_src = AUDIO_WAV_SRC_EMBED;
static volatile uint8_t dac_tone_on = 0;
static volatile uint8_t dac_wave_on = 0;
static volatile uint32_t dac_samples_left = 0;
static volatile uint32_t dac_phase = 0;
static volatile uint32_t dac_phase_inc = 0;
static volatile uint32_t dac_fed = 0;
static volatile uint8_t dac_evt_tone_done = 0;
static volatile uint8_t dac_evt_wave_done = 0;
static uint32_t dac_play_fs = AUDIO_DAC_DEFAULT_FS;
static const int16_t *dac_tone_lut = tone_sine_wave;

static uint16_t sample_to_dac12(int16_t s)
{
	int32_t v = ((int32_t)s + 32768) >> 4;
	if (v < 0) {
		v = 0;
	} else if (v > 4095) {
		v = 4095;
	}
	return (uint16_t)v;
}

static void dac_write12(uint16_t code)
{
	dac_data_set(DAC0, DAC_ALIGN_12B_R, code);
}

static uint32_t dac_read_unit(uint8_t nbytes, endianness_enum endian)
{
	return audio_wav_read_unit(dac_wav_src, &dac_head_idx, nbytes,
				   (littleendian == endian) ? 1 : 0);
}

static errorcode_enum dac_wave_parse(void)
{
	uint32_t extra = 0;
	uint32_t temp;

	dac_head_idx = 0;
	dac_data_addr = 0;
	dac_pcm_idx = 0;

	if (CHUNKID != dac_read_unit(4, bigendian)) {
		return UNVALID_RIFF_ID;
	}
	(void)dac_read_unit(4, littleendian);
	if (FILEFORMAT != dac_read_unit(4, bigendian)) {
		return UNVALID_WAVE_FORMAT;
	}
	if (FORMATID != dac_read_unit(4, bigendian)) {
		return UNVALID_FORMATCHUNK_ID;
	}
	if (FORMATCHUNKSIZE != dac_read_unit(4, littleendian)) {
		extra = 1;
	}

	dac_wave.formattag = (uint16_t)dac_read_unit(2, littleendian);
	if (WAVE_FORMAT_PCM != dac_wave.formattag) {
		return UNSUPPORETD_FORMATTAG;
	}

	dac_wave.numchannels = (uint16_t)dac_read_unit(2, littleendian);
	dac_wave.samplerate = dac_read_unit(4, littleendian);
	if ((dac_wave.samplerate < 8000u) || (dac_wave.samplerate > 192000u)) {
		return UNSUPPORETD_SAMPLE_RATE;
	}

	dac_wave.byterate = dac_read_unit(4, littleendian);
	dac_wave.blockalign = (uint16_t)dac_read_unit(2, littleendian);
	dac_wave.bitspersample = (uint16_t)dac_read_unit(2, littleendian);
	if (BITS_PER_SAMPLE_16 != dac_wave.bitspersample) {
		return UNSUPPORETD_BITS_PER_SAMPLE;
	}

	if (1u == extra) {
		if (0x00u != dac_read_unit(2, littleendian)) {
			return UNSUPPORETD_EXTRAFORMATBYTES;
		}
		if (FACTID != dac_read_unit(4, bigendian)) {
			return UNVALID_FACTCHUNK_ID;
		}
		temp = dac_read_unit(4, littleendian);
		dac_head_idx += temp;
	}

	if (DATAID != dac_read_unit(4, bigendian)) {
		return UNVALID_DATACHUNK_ID;
	}
	dac_wave.datasize = dac_read_unit(4, littleendian);
	dac_data_addr = dac_head_idx;
	return VALID_WAVE_FILE;
}

static void dac_timer_stop(void)
{
	timer_interrupt_disable(TIMER6, TIMER_INT_UP);
	timer_disable(TIMER6);
	nvic_irq_disable(TIMER6_IRQn);
	timer_interrupt_flag_clear(TIMER6, TIMER_INT_UP);
}

static void dac_timer_start(uint32_t sample_rate)
{
	timer_parameter_struct tip;
	uint32_t ticks;

	if (sample_rate < 4000u) {
		sample_rate = 4000u;
	}
	dac_play_fs = sample_rate;

	rcu_periph_clock_enable(RCU_TIMER6);
	timer_deinit(TIMER6);

	ticks = SystemCoreClock / sample_rate;
	if (ticks < 2u) {
		ticks = 2u;
	}

	timer_struct_para_init(&tip);
	tip.prescaler = 0;
	tip.alignedmode = TIMER_COUNTER_EDGE;
	tip.counterdirection = TIMER_COUNTER_UP;
	tip.period = ticks - 1u;
	tip.clockdivision = TIMER_CKDIV_DIV1;
	tip.repetitioncounter = 0;
	timer_init(TIMER6, &tip);

	timer_interrupt_flag_clear(TIMER6, TIMER_INT_UP);
	timer_interrupt_enable(TIMER6, TIMER_INT_UP);
	nvic_irq_enable(TIMER6_IRQn, 1U, 1U);
	timer_enable(TIMER6);
}

static void dac_feed_one(void)
{
	int16_t s16;

	if (dac_tone_on) {
		if (dac_samples_left == 0u) {
			dac_tone_on = 0;
			dac_timer_stop();
			dac_write12(DAC_MID);
			i2s_codec_amp_enable(0);
			dac_evt_tone_done = 1;
			return;
		}
		s16 = dac_tone_lut[(dac_phase >> 16) & TONE_WAVE_MASK];
		dac_phase += dac_phase_inc;
		dac_write12(sample_to_dac12(s16));
		dac_samples_left--;
		dac_fed++;
		return;
	}

	if (dac_wave_on) {
		int16_t s16;
		int32_t mix;

		if (dac_pcm_idx >= dac_wave.datasize) {
			dac_wave_on = 0;
			dac_timer_stop();
			dac_write12(DAC_MID);
			i2s_codec_amp_enable(0);
			dac_evt_wave_done = 1;
			return;
		}

		if (0 != audio_wav_stream_get16(&s16)) {
			if (audio_wav_stream_eof()) {
				dac_wave_on = 0;
				dac_timer_stop();
				dac_write12(DAC_MID);
				i2s_codec_amp_enable(0);
				dac_evt_wave_done = 1;
			} else {
				dac_write12(DAC_MID); /* underrun → silence */
			}
			return;
		}
		if (CHANNEL_STEREO == dac_wave.numchannels) {
			int16_t r16;

			if (0 == audio_wav_stream_get16(&r16)) {
				mix = (int32_t)s16 + (int32_t)r16;
				s16 = (int16_t)(mix / 2);
			}
			dac_pcm_idx += 4u;
		} else {
			dac_pcm_idx += 2u;
		}

		dac_write12(sample_to_dac12(s16));
		dac_fed++;
	}
}

void TIMER6_IRQHandler(void)
{
	if (SET == timer_interrupt_flag_get(TIMER6, TIMER_INT_UP)) {
		timer_interrupt_flag_clear(TIMER6, TIMER_INT_UP);
		dac_feed_one();
	}
}

void audio_dac_init(void)
{
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_DAC);

	/* PA4 analog → DAC_OUT0 */
	gpio_init(AUDIO_DAC_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, AUDIO_DAC_PIN);

	dac_deinit();
	dac_trigger_disable(DAC0);
	dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
	dac_output_buffer_enable(DAC0);
	dac_enable(DAC0);
	dac_write12(DAC_MID);

	printf_("\r\nAudio DAC init (PA4/DAC0) mid=%u", (unsigned)DAC_MID);
}

void audio_dac_deinit(void)
{
	audio_dac_stop();
	dac_disable(DAC0);
	dac_deinit();
}

void audio_dac_stop(void)
{
	dac_timer_stop();
	dac_tone_on = 0;
	dac_wave_on = 0;
	dac_samples_left = 0;
	dac_pcm_idx = 0;
	audio_wav_stream_stop();
	dac_write12(DAC_MID);
	i2s_codec_amp_enable(0);
}

int audio_dac_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut)
{
	uint32_t fs = AUDIO_DAC_DEFAULT_FS;

	if ((freq_hz == 0u) || (duration_ms == 0u)) {
		return -1;
	}
	if (lut == 0) {
		lut = tone_sine_wave;
	}

	i2s_audio_stop();
	audio_pwm_stop();
	audio_dac_stop();

	dac_tone_lut = lut;
	dac_phase = 0;
	dac_phase_inc = tone_dds_phase_inc(freq_hz, fs);
	dac_samples_left = (fs * duration_ms) / 1000u;
	if (dac_samples_left == 0u) {
		dac_samples_left = 1u;
	}
	dac_fed = 0;
	dac_evt_tone_done = 0;
	dac_tone_on = 1;

	i2s_codec_amp_enable(1);
	{
		int16_t s16 = dac_tone_lut[0];
		dac_write12(sample_to_dac12(s16));
		dac_phase = dac_phase_inc;
		dac_samples_left--;
		dac_fed++;
	}
	dac_timer_start(fs);
	return 0;
}

int audio_dac_tone_1k_3s(void)
{
	int ret = audio_dac_tone_play(1000u, 3000u, tone_sine_wave);
	if (0 == ret) {
		printf_("\r\nDAC tone 1kHz 3s fs=%u", (unsigned)AUDIO_DAC_DEFAULT_FS);
	}
	return ret;
}

int audio_dac_wave_play(audio_wav_src_t src)
{
	errorcode_enum err;

	i2s_audio_stop();
	audio_dac_stop();

	dac_wav_src = src;
	err = dac_wave_parse();
	if (VALID_WAVE_FILE != err) {
		printf_("\r\nDAC wave parse err=%d", (int)err);
		return (int)err;
	}

	audio_wav_stream_start(src, dac_data_addr, dac_wave.datasize);

	dac_pcm_idx = 0;
	dac_fed = 0;
	dac_evt_wave_done = 0;
	dac_wave_on = 1;

	i2s_codec_amp_enable(1);
	delay_1ms(200);
	audio_wav_stream_fill();
	dac_timer_start(dac_wave.samplerate);

	printf_("\r\nDAC wave src=%u fs=%lu ch=%u bytes=%lu",
		(unsigned)src,
		(unsigned long)dac_wave.samplerate,
		(unsigned)dac_wave.numchannels,
		(unsigned long)dac_wave.datasize);
	return 0;
}

int audio_dac_busy(void)
{
	return (dac_tone_on || dac_wave_on) ? 1 : 0;
}

void audio_dac_poll(void)
{
	static uint32_t mark = 0;

	if (dac_wave_on) {
		audio_wav_stream_fill();
	}

	if (dac_tone_on || dac_wave_on) {
		if (dac_fed >= (mark + (dac_play_fs / 2u))) {
			mark = dac_fed;
			printf_("\r\nDAC run fed=%lu", (unsigned long)dac_fed);
		}
	} else {
		mark = 0;
	}

	if (dac_evt_tone_done) {
		dac_evt_tone_done = 0;
		printf_("\r\nDAC tone done fed=%lu", (unsigned long)dac_fed);
	}
	if (dac_evt_wave_done) {
		dac_evt_wave_done = 0;
		printf_("\r\nDAC wave done fed=%lu", (unsigned long)dac_fed);
	}
}
