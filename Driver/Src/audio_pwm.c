// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "audio_pwm.h"
#include "wave_data.h"
#include "i2s_codec.h"
#include "audio_dac.h"
#include "ir_snd_rcv.h"
#include "tone.h"
#include "systick.h"
#include "printf.h"

#define PWM_MID           ((AUDIO_PWM_ARR + 1u) / 2u)

static wave_file_struct pwm_wave;
static uint32_t pwm_head_idx = 0;
static uint32_t pwm_data_addr = 0;
static volatile uint32_t pwm_pcm_idx = 0;
static audio_wav_src_t pwm_wav_src = AUDIO_WAV_SRC_EMBED;
static volatile uint8_t pwm_tone_on = 0;
static volatile uint8_t pwm_wave_on = 0;
static volatile uint32_t pwm_samples_left = 0;
static volatile uint32_t pwm_phase = 0;
static volatile uint32_t pwm_phase_inc = 0;
static volatile uint32_t pwm_fed = 0;
static volatile uint8_t pwm_evt_tone_done = 0;
static volatile uint8_t pwm_evt_wave_done = 0;
static uint32_t pwm_play_fs = AUDIO_PWM_DEFAULT_FS;
static uint8_t pwm_carrier_on = 0;
static const int16_t *pwm_tone_lut = tone_sine_wave;

static uint16_t sample_to_pwm_duty(int16_t s)
{
	/* map int16 → 0..AUDIO_PWM_ARR */
	int32_t v = ((int32_t)s + 32768) * (int32_t)AUDIO_PWM_ARR / 65535;
	if (v < 0) {
		v = 0;
	} else if (v > (int32_t)AUDIO_PWM_ARR) {
		v = (int32_t)AUDIO_PWM_ARR;
	}
	return (uint16_t)v;
}

static void pwm_set_duty(uint16_t duty)
{
	timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_0, duty);
}

static uint32_t pwm_read_unit(uint8_t nbytes, endianness_enum endian)
{
	return audio_wav_read_unit(pwm_wav_src, &pwm_head_idx, nbytes,
				   (littleendian == endian) ? 1 : 0);
}

static errorcode_enum pwm_wave_parse(void)
{
	uint32_t extra = 0;
	uint32_t temp;

	pwm_head_idx = 0;
	pwm_data_addr = 0;
	pwm_pcm_idx = 0;

	if (CHUNKID != pwm_read_unit(4, bigendian)) {
		return UNVALID_RIFF_ID;
	}
	(void)pwm_read_unit(4, littleendian);
	if (FILEFORMAT != pwm_read_unit(4, bigendian)) {
		return UNVALID_WAVE_FORMAT;
	}
	if (FORMATID != pwm_read_unit(4, bigendian)) {
		return UNVALID_FORMATCHUNK_ID;
	}
	if (FORMATCHUNKSIZE != pwm_read_unit(4, littleendian)) {
		extra = 1;
	}

	pwm_wave.formattag = (uint16_t)pwm_read_unit(2, littleendian);
	if (WAVE_FORMAT_PCM != pwm_wave.formattag) {
		return UNSUPPORETD_FORMATTAG;
	}

	pwm_wave.numchannels = (uint16_t)pwm_read_unit(2, littleendian);
	pwm_wave.samplerate = pwm_read_unit(4, littleendian);
	if ((pwm_wave.samplerate < 8000u) || (pwm_wave.samplerate > 192000u)) {
		return UNSUPPORETD_SAMPLE_RATE;
	}

	pwm_wave.byterate = pwm_read_unit(4, littleendian);
	pwm_wave.blockalign = (uint16_t)pwm_read_unit(2, littleendian);
	pwm_wave.bitspersample = (uint16_t)pwm_read_unit(2, littleendian);
	if (BITS_PER_SAMPLE_16 != pwm_wave.bitspersample) {
		return UNSUPPORETD_BITS_PER_SAMPLE;
	}

	if (1u == extra) {
		if (0x00u != pwm_read_unit(2, littleendian)) {
			return UNSUPPORETD_EXTRAFORMATBYTES;
		}
		if (FACTID != pwm_read_unit(4, bigendian)) {
			return UNVALID_FACTCHUNK_ID;
		}
		temp = pwm_read_unit(4, littleendian);
		pwm_head_idx += temp;
	}

	if (DATAID != pwm_read_unit(4, bigendian)) {
		return UNVALID_DATACHUNK_ID;
	}
	pwm_wave.datasize = pwm_read_unit(4, littleendian);
	pwm_data_addr = pwm_head_idx;
	return VALID_WAVE_FILE;
}

static void pwm_sample_timer_stop(void)
{
	timer_interrupt_disable(TIMER2, TIMER_INT_UP);
	timer_disable(TIMER2);
	nvic_irq_disable(TIMER2_IRQn);
	timer_interrupt_flag_clear(TIMER2, TIMER_INT_UP);
}

static void pwm_sample_timer_start(uint32_t sample_rate)
{
	timer_parameter_struct tip;
	uint32_t ticks;

	if (sample_rate < 4000u) {
		sample_rate = 4000u;
	}
	pwm_play_fs = sample_rate;

	rcu_periph_clock_enable(RCU_TIMER2);
	timer_deinit(TIMER2);

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
	timer_init(TIMER2, &tip);

	timer_interrupt_flag_clear(TIMER2, TIMER_INT_UP);
	timer_interrupt_enable(TIMER2, TIMER_INT_UP);
	nvic_irq_enable(TIMER2_IRQn, 1U, 1U);
	timer_enable(TIMER2);
}

/* TIMER7_CH0 high-freq PWM carrier on PC6 (steals TIMER7 from IR 38k) */
static void pwm_carrier_start(void)
{
	timer_oc_parameter_struct oc;
	timer_parameter_struct tip;

	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_TIMER7);

	gpio_init(AUDIO_PWM_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, AUDIO_PWM_PIN);

	timer_deinit(TIMER7);
	timer_struct_para_init(&tip);
	tip.prescaler = 0;
	tip.alignedmode = TIMER_COUNTER_EDGE;
	tip.counterdirection = TIMER_COUNTER_UP;
	tip.period = AUDIO_PWM_ARR;
	tip.clockdivision = TIMER_CKDIV_DIV1;
	tip.repetitioncounter = 0;
	timer_init(TIMER7, &tip);

	timer_channel_output_struct_para_init(&oc);
	oc.outputstate = TIMER_CCX_ENABLE;
	oc.outputnstate = TIMER_CCXN_DISABLE;
	oc.ocpolarity = TIMER_OC_POLARITY_HIGH;
	oc.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
	oc.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
	oc.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
	timer_channel_output_config(TIMER7, TIMER_CH_0, &oc);

	timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_0, PWM_MID);
	timer_channel_output_mode_config(TIMER7, TIMER_CH_0, TIMER_OC_MODE_PWM0);
	timer_channel_output_shadow_config(TIMER7, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

	timer_primary_output_config(TIMER7, ENABLE);
	timer_auto_reload_shadow_enable(TIMER7);
	timer_enable(TIMER7);
	pwm_carrier_on = 1;
}

static void pwm_carrier_stop_restore_ir(void)
{
	pwm_sample_timer_stop();

	if (pwm_carrier_on) {
		pwm_set_duty(0);
		timer_disable(TIMER7);
		/* idle PC6 as GPIO low (not floating AF) */
		gpio_init(AUDIO_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, AUDIO_PWM_PIN);
		gpio_bit_reset(AUDIO_PWM_PORT, AUDIO_PWM_PIN);
		pwm_carrier_on = 0;

		/* restore IR 38 kHz carrier on TIMER7_CH2 / PC8 */
		ir_frq_timer_config();
		timer_enable(TIMER7);
	}
}

static void pwm_feed_one(void)
{
	int16_t s16;

	if (pwm_tone_on) {
		if (pwm_samples_left == 0u) {
			pwm_tone_on = 0;
			pwm_sample_timer_stop();
			pwm_set_duty(PWM_MID);
			i2s_codec_amp_enable(0);
			pwm_carrier_stop_restore_ir();
			pwm_evt_tone_done = 1;
			return;
		}
		s16 = pwm_tone_lut[(pwm_phase >> 16) & TONE_WAVE_MASK];
		pwm_phase += pwm_phase_inc;
		pwm_set_duty(sample_to_pwm_duty(s16));
		pwm_samples_left--;
		pwm_fed++;
		return;
	}

	if (pwm_wave_on) {
		int32_t mix;

		if (pwm_pcm_idx >= pwm_wave.datasize) {
			pwm_wave_on = 0;
			pwm_sample_timer_stop();
			pwm_set_duty(PWM_MID);
			i2s_codec_amp_enable(0);
			pwm_carrier_stop_restore_ir();
			pwm_evt_wave_done = 1;
			return;
		}

		if (0 != audio_wav_stream_get16(&s16)) {
			if (audio_wav_stream_eof()) {
				pwm_wave_on = 0;
				pwm_sample_timer_stop();
				pwm_set_duty(PWM_MID);
				i2s_codec_amp_enable(0);
				pwm_carrier_stop_restore_ir();
				pwm_evt_wave_done = 1;
			} else {
				pwm_set_duty(PWM_MID); /* underrun → silence */
			}
			return;
		}
		if (CHANNEL_STEREO == pwm_wave.numchannels) {
			int16_t r16;

			if (0 == audio_wav_stream_get16(&r16)) {
				mix = (int32_t)s16 + (int32_t)r16;
				s16 = (int16_t)(mix / 2);
			}
			pwm_pcm_idx += 4u;
		} else {
			pwm_pcm_idx += 2u;
		}

		pwm_set_duty(sample_to_pwm_duty(s16));
		pwm_fed++;
	}
}

void TIMER2_IRQHandler(void)
{
	if (SET == timer_interrupt_flag_get(TIMER2, TIMER_INT_UP)) {
		timer_interrupt_flag_clear(TIMER2, TIMER_INT_UP);
		pwm_feed_one();
	}
}

void audio_pwm_init(void)
{
	rcu_periph_clock_enable(RCU_GPIOC);
	/* idle until play; keep out of AF until carrier starts */
	gpio_init(AUDIO_PWM_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, AUDIO_PWM_PIN);
	gpio_bit_reset(AUDIO_PWM_PORT, AUDIO_PWM_PIN);
	printf_("\r\nAudio PWM init (PC6/TIMER7_CH0) ARR=%u", (unsigned)AUDIO_PWM_ARR);
}

void audio_pwm_deinit(void)
{
	audio_pwm_stop();
}

void audio_pwm_stop(void)
{
	pwm_tone_on = 0;
	pwm_wave_on = 0;
	pwm_samples_left = 0;
	pwm_pcm_idx = 0;
	audio_wav_stream_stop();
	pwm_sample_timer_stop();
	if (pwm_carrier_on) {
		pwm_set_duty(PWM_MID);
	}
	i2s_codec_amp_enable(0);
	pwm_carrier_stop_restore_ir();
}

int audio_pwm_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut)
{
	uint32_t fs = AUDIO_PWM_DEFAULT_FS;

	if ((freq_hz == 0u) || (duration_ms == 0u)) {
		return -1;
	}
	if (lut == 0) {
		lut = tone_sine_wave;
	}

	i2s_audio_stop();
	audio_dac_stop();
	audio_pwm_stop();

	pwm_tone_lut = lut;
	pwm_phase = 0;
	pwm_phase_inc = tone_dds_phase_inc(freq_hz, fs);
	pwm_samples_left = (fs * duration_ms) / 1000u;
	if (pwm_samples_left == 0u) {
		pwm_samples_left = 1u;
	}
	pwm_fed = 0;
	pwm_evt_tone_done = 0;
	pwm_tone_on = 1;

	pwm_carrier_start();
	i2s_codec_amp_enable(1);

	{
		int16_t s16 = pwm_tone_lut[0];
		pwm_set_duty(sample_to_pwm_duty(s16));
		pwm_phase = pwm_phase_inc;
		pwm_samples_left--;
		pwm_fed++;
	}
	pwm_sample_timer_start(fs);
	return 0;
}

int audio_pwm_tone_1k_3s(void)
{
	int ret = audio_pwm_tone_play(1000u, 3000u, tone_sine_wave);
	if (0 == ret) {
		printf_("\r\nPWM tone 1kHz 3s fs=%u Fc~%lukHz",
			(unsigned)AUDIO_PWM_DEFAULT_FS,
			(unsigned long)(SystemCoreClock / (AUDIO_PWM_ARR + 1u) / 1000u));
	}
	return ret;
}

int audio_pwm_wave_play(audio_wav_src_t src)
{
	errorcode_enum err;

	i2s_audio_stop();
	audio_dac_stop();
	audio_pwm_stop();

	pwm_wav_src = src;
	err = pwm_wave_parse();
	if (VALID_WAVE_FILE != err) {
		printf_("\r\nPWM wave parse err=%d", (int)err);
		return (int)err;
	}

	audio_wav_stream_start(src, pwm_data_addr, pwm_wave.datasize);

	pwm_pcm_idx = 0;
	pwm_fed = 0;
	pwm_evt_wave_done = 0;
	pwm_wave_on = 1;

	pwm_carrier_start();
	i2s_codec_amp_enable(1);
	delay_1ms(200);
	audio_wav_stream_fill();
	pwm_sample_timer_start(pwm_wave.samplerate);

	printf_("\r\nPWM wave src=%u fs=%lu ch=%u bytes=%lu",
		(unsigned)src,
		(unsigned long)pwm_wave.samplerate,
		(unsigned)pwm_wave.numchannels,
		(unsigned long)pwm_wave.datasize);
	return 0;
}

int audio_pwm_busy(void)
{
	return (pwm_tone_on || pwm_wave_on) ? 1 : 0;
}

void audio_pwm_poll(void)
{
	static uint32_t mark = 0;

	if (pwm_wave_on) {
		audio_wav_stream_fill();
	}

	if (pwm_tone_on || pwm_wave_on) {
		if (pwm_fed >= (mark + (pwm_play_fs / 2u))) {
			mark = pwm_fed;
			printf_("\r\nPWM run fed=%lu", (unsigned long)pwm_fed);
		}
	} else {
		mark = 0;
	}

	if (pwm_evt_tone_done) {
		pwm_evt_tone_done = 0;
		printf_("\r\nPWM tone done fed=%lu", (unsigned long)pwm_fed);
	}
	if (pwm_evt_wave_done) {
		pwm_evt_wave_done = 0;
		printf_("\r\nPWM wave done fed=%lu", (unsigned long)pwm_fed);
	}
}
