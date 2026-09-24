// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "i2s_codec.h"
#include "wave_data.h"
#include "systick.h"
#include "tone.h"
#include "printf.h"

static wave_file_struct wave_struct;
static uint16_t i2saudiofreq = I2S_DEFAULT_RATE;
static uint32_t headertab_index = 0;
static uint32_t datastartaddr = 0;
static volatile uint32_t audiodataindex = 0;
static uint8_t i2s_wav_dup_r = 0;
static uint16_t i2s_wav_last = 0;
static audio_wav_src_t i2s_wav_src = AUDIO_WAV_SRC_EMBED;
static volatile uint8_t i2s_wave_playing = 0;

static volatile uint8_t i2s_tone_playing = 0;
static volatile uint32_t i2s_tone_frames_left = 0;
static volatile uint32_t i2s_tone_phase = 0;
static volatile uint32_t i2s_tone_phase_inc = 0;
static volatile uint8_t i2s_tone_ch = 0;
static volatile int16_t i2s_tone_sample = 0;
static volatile uint32_t i2s_tone_fed = 0;
static volatile uint8_t i2s_amp_active_low = 1; /* PA_CTL#: low = amp on (confirmed by tone test) */
static volatile uint8_t i2s_evt_done = 0;
static volatile uint8_t i2s_evt_wave_done = 0;
static volatile uint32_t i2s_wave_fed = 0;
static const int16_t *i2s_tone_lut = tone_sine_wave;

static uint32_t read_unit(uint8_t nbrofbytes, endianness_enum bytesformat)
{
	return audio_wav_read_unit(i2s_wav_src, &headertab_index, nbrofbytes,
				   (littleendian == bytesformat) ? 1 : 0);
}

static errorcode_enum codec_wave_parsing(void)
{
	uint32_t temp;
	uint32_t extraformatbytes = 0;

	headertab_index = 0;
	datastartaddr = 0;
	audiodataindex = 0;

	if (CHUNKID != read_unit(4, bigendian)) {
		return UNVALID_RIFF_ID;
	}
	wave_struct.riffchunksize = read_unit(4, littleendian);
	if (FILEFORMAT != read_unit(4, bigendian)) {
		return UNVALID_WAVE_FORMAT;
	}
	if (FORMATID != read_unit(4, bigendian)) {
		return UNVALID_FORMATCHUNK_ID;
	}
	if (FORMATCHUNKSIZE != read_unit(4, littleendian)) {
		extraformatbytes = 1;
	}

	wave_struct.formattag = (uint16_t)read_unit(2, littleendian);
	if (WAVE_FORMAT_PCM != wave_struct.formattag) {
		return UNSUPPORETD_FORMATTAG;
	}

	wave_struct.numchannels = (uint16_t)read_unit(2, littleendian);
	wave_struct.samplerate = read_unit(4, littleendian);
	if ((wave_struct.samplerate < 8000u) || (wave_struct.samplerate > 192000u)) {
		return UNSUPPORETD_SAMPLE_RATE;
	}
	i2saudiofreq = (uint16_t)wave_struct.samplerate;

	wave_struct.byterate = read_unit(4, littleendian);
	wave_struct.blockalign = (uint16_t)read_unit(2, littleendian);
	wave_struct.bitspersample = (uint16_t)read_unit(2, littleendian);
	if (BITS_PER_SAMPLE_16 != wave_struct.bitspersample) {
		return UNSUPPORETD_BITS_PER_SAMPLE;
	}

	if (1u == extraformatbytes) {
		if (0x00u != read_unit(2, littleendian)) {
			return UNSUPPORETD_EXTRAFORMATBYTES;
		}
		if (FACTID != read_unit(4, bigendian)) {
			return UNVALID_FACTCHUNK_ID;
		}
		temp = read_unit(4, littleendian);
		headertab_index += temp;
	}

	if (DATAID != read_unit(4, bigendian)) {
		return UNVALID_DATACHUNK_ID;
	}
	wave_struct.datasize = read_unit(4, littleendian);
	datastartaddr = headertab_index;
	return VALID_WAVE_FILE;
}

static uint16_t read_half_word(void)
{
	int16_t s16;

	/* Mono WAV: I2S still clocks L then R; reuse the same PCM for R. */
	if ((CHANNEL_MONO == wave_struct.numchannels) && (0u != i2s_wav_dup_r)) {
		i2s_wav_dup_r = 0;
		return i2s_wav_last;
	}

	if (0 != audio_wav_stream_get16(&s16)) {
		/* underrun: keep playing silence until fill catches up;
		 * only true EOF ends the stream */
		if (audio_wav_stream_eof()) {
			audiodataindex = wave_struct.datasize;
		}
		return 0;
	}
	i2s_wav_last = (uint16_t)s16;
	audiodataindex += 2u;
	if (CHANNEL_MONO == wave_struct.numchannels) {
		i2s_wav_dup_r = 1;
	}
	return i2s_wav_last;
}

static uint16_t i2s_wav_gain_sample(uint16_t raw)
{
	int32_t s = (int16_t)raw;

	s *= (int32_t)I2S_WAV_GAIN;
	if (s > 32767) {
		s = 32767;
	} else if (s < -32768) {
		s = -32768;
	}
	return (uint16_t)s;
}

static void i2s_audio_data_send(void)
{
	uint16_t sample;

	sample = i2s_wav_gain_sample(read_half_word());
	spi_i2s_data_transmit(SPI2, sample);
	i2s_wave_fed++;
}

static void i2s_amp_apply(int enable)
{
	gpio_init(PA_CTL_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, PA_CTL_PIN);
	if (i2s_amp_active_low) {
		if (enable) {
			gpio_bit_reset(PA_CTL_PORT, PA_CTL_PIN);
		} else {
			gpio_bit_set(PA_CTL_PORT, PA_CTL_PIN);
		}
	} else {
		if (enable) {
			gpio_bit_set(PA_CTL_PORT, PA_CTL_PIN);
		} else {
			gpio_bit_reset(PA_CTL_PORT, PA_CTL_PIN);
		}
	}
}

void i2s_codec_amp_enable(int enable)
{
	i2s_amp_apply(enable);
}

static void i2s_tone_feed_one(void)
{
	int16_t sample;

	if (!i2s_tone_playing) {
		return;
	}
	if (i2s_tone_frames_left == 0u) {
		spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
		i2s_disable(SPI2);
		i2s_tone_playing = 0;
		i2s_amp_apply(0);
		i2s_evt_done = 1;
		return;
	}

	if (i2s_tone_ch == 0u) {
		sample = i2s_tone_lut[(i2s_tone_phase >> 16) & TONE_WAVE_MASK];
		i2s_tone_phase += i2s_tone_phase_inc;
		i2s_tone_sample = sample;
		spi_i2s_data_transmit(SPI2, (uint16_t)sample);
		i2s_tone_ch = 1u;
	} else {
		spi_i2s_data_transmit(SPI2, (uint16_t)i2s_tone_sample);
		i2s_tone_ch = 0u;
		i2s_tone_frames_left--;
		i2s_tone_fed++;
	}
}

void i2s_codec_init(uint32_t sample_rate)
{
	static uint8_t pins_ready = 0;

	if ((sample_rate < 4000u) || (sample_rate > 192000u)) {
		sample_rate = I2S_DEFAULT_RATE;
	}
	i2saudiofreq = (uint16_t)sample_rate;

	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_SPI2);

	rcu_i2s2_clock_config(RCU_I2S2SRC_CKSYS);

	/*
	 * Default I2S2 pins (NO SPI2_REMAP): PA15=WS, PB3=CK, PB5=SD, PC7=MCK.
	 * Remap SWJ once only — repeating after soft-reset with a debugger attached
	 * has been observed to glitch the debug/session path.
	 */
	if (0u == pins_ready) {
		gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
		gpio_pin_remap_config(GPIO_SPI2_REMAP, DISABLE);
		pins_ready = 1u;
	}

	i2s_amp_apply(0);

	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_5);
	gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);

	spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
	nvic_irq_disable(SPI2_IRQn);
	NVIC_ClearPendingIRQ(SPI2_IRQn);
	spi_i2s_deinit(SPI2);

	/*
	 * psc → init, leave I2SEN=0 until first data is preloaded (see tone/play).
	 * CH32B → SCLK/LRCK≈64 for ES7134; MCLK≈256×Fs on PC7.
	 */
	i2s_psc_config(SPI2, i2saudiofreq, I2S_FRAME_FMT, I2S_MCLKOUTPUT);
	i2s_init(SPI2, I2S_MODE_MASTERTX, I2S_STANDARD, I2S_CK_POLARITY);
}

void i2s_codec_deinit(void)
{
	i2s_audio_stop();
	i2s_amp_apply(0);
	i2s_disable(SPI2);
	spi_i2s_deinit(SPI2);
}

void i2s_audio_stop(void)
{
	uint32_t primask = __get_PRIMASK();

	__disable_irq();
	spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
	nvic_irq_disable(SPI2_IRQn);
	NVIC_ClearPendingIRQ(SPI2_IRQn);
	i2s_disable(SPI2);
	i2s_wave_playing = 0;
	i2s_tone_playing = 0;
	i2s_tone_frames_left = 0;
	audiodataindex = 0;
	i2s_wav_dup_r = 0;
	i2s_wav_last = 0;
	audio_wav_stream_stop();
	i2s_amp_apply(0);
	__set_PRIMASK(primask);
}

errorcode_enum i2s_audio_play(audio_wav_src_t src)
{
	errorcode_enum errorcode;
	uint32_t play_rate;
	uint32_t i;

	i2s_audio_stop();
	i2s_wav_src = src;
	errorcode = codec_wave_parsing();
	if (VALID_WAVE_FILE != errorcode) {
		return errorcode;
	}

	audio_wav_stream_start(src, datastartaddr, wave_struct.datasize);

	/* slower than file rate → longer duration, lower pitch */
	play_rate = (wave_struct.samplerate * I2S_WAV_SPEED_NUM) / I2S_WAV_SPEED_DEN;
	if (play_rate < 4000u) {
		play_rate = 4000u;
	}

	i2s_codec_init(play_rate); /* leaves amp OFF */
	audio_wav_stream_fill();
	audio_wav_stream_fill();

	i2s_wave_fed = 0;
	i2s_evt_wave_done = 0;
	i2s_wave_playing = 0; /* silence prime — do not pull WAV yet */

	printf_("\r\nI2S wave src=%u fileFs=%lu playFs=%u gain=%ux ch=%u bytes=%lu",
		(unsigned)src,
		(unsigned long)wave_struct.samplerate, (unsigned)i2saudiofreq,
		(unsigned)I2S_WAV_GAIN, (unsigned)wave_struct.numchannels,
		(unsigned long)wave_struct.datasize);

	nvic_irq_enable(SPI2_IRQn, 2, 0); /* below SysTick(0) — else delay_1ms deadlocks */
	/* 1) start I2S clocks with silence, amp still off */
	spi_i2s_data_transmit(SPI2, 0);
	spi_i2s_data_transmit(SPI2, 0);
	i2s_enable(SPI2);
	spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
	for (i = 0u; i < 64u; i++) {
		uint32_t guard = 200000u;

		while (RESET == spi_i2s_flag_get(SPI2, SPI_FLAG_TBE)) {
			if (--guard == 0u) {
				break;
			}
		}
		if (guard == 0u) {
			break;
		}
		spi_i2s_data_transmit(SPI2, 0);
	}

	/* 2) unmute PA after clocks are running; cold boot needs longer settle */
	i2s_amp_apply(1);
	delay_1ms(200);

	/* 3) refill (SPI may have been idle during settle) then switch to WAV */
	audio_wav_stream_fill();
	audio_wav_stream_fill();
	i2s_wav_dup_r = 0;
	i2s_wav_last = 0;
	audiodataindex = 0;
	i2s_wave_playing = 1;
	i2s_audio_data_send();
	i2s_audio_data_send();
	spi_i2s_interrupt_enable(SPI2, SPI_I2S_INT_TBE);
	return VALID_WAVE_FILE;
}

int i2s_tone_play(uint32_t freq_hz, uint32_t duration_ms, const int16_t *lut)
{
	uint32_t sample_rate;
	uint32_t primask;

	if ((freq_hz == 0u) || (duration_ms == 0u)) {
		return -1;
	}
	if (lut == 0) {
		lut = tone_sine_wave;
	}

	i2s_audio_stop();
	sample_rate = I2S_DEFAULT_RATE;
	i2s_amp_active_low = 1;
	i2s_codec_init(sample_rate);

	i2s_tone_lut = lut;
	i2s_tone_phase = 0;
	i2s_tone_phase_inc = tone_dds_phase_inc(freq_hz, sample_rate);
	i2s_tone_frames_left = (sample_rate * duration_ms) / 1000u;
	if (i2s_tone_frames_left == 0u) {
		i2s_tone_frames_left = 1u;
	}
	i2s_tone_ch = 0;
	i2s_tone_sample = 0;
	i2s_tone_fed = 0;
	i2s_evt_done = 0;

	/* busy-wait settle — not delay_1ms (must not depend on SysTick vs TBE) */
	i2s_amp_apply(1);
	delay_while_ms(5);

	/* arm IRQ + preload atomically — soft-reset left TBE pending otherwise races init */
	primask = __get_PRIMASK();
	__disable_irq();
	i2s_tone_playing = 1;
	NVIC_ClearPendingIRQ(SPI2_IRQn);
	nvic_irq_enable(SPI2_IRQn, 2, 0);
	i2s_tone_feed_one();
	i2s_tone_feed_one();
	i2s_enable(SPI2);
	spi_i2s_interrupt_enable(SPI2, SPI_I2S_INT_TBE);
	if (RESET == (SPI_STAT(SPI2) & SPI_STAT_TRANS)) {
		i2s_tone_feed_one();
		i2s_tone_feed_one();
		i2s_enable(SPI2);
	}
	__set_PRIMASK(primask);

	return 0;
}

int i2s_tone_1k_3s(void)
{
	return i2s_tone_play(1000u, 3000u, tone_sine_wave);
}

int i2s_tone_busy(void)
{
	return (i2s_tone_playing || i2s_wave_playing) ? 1 : 0;
}

/*!
 * \brief main-loop only: deferred logs (feeding is SPI2 TBE IRQ)
 */
void i2s_codec_poll(void)
{
	if (i2s_wave_playing) {
		/* keep SPI-flash fill hot — no printf here (blocks → underrun/glitch) */
		audio_wav_stream_fill();
		audio_wav_stream_fill();
	}

	if (i2s_evt_done) {
		i2s_evt_done = 0;
		printf_("\r\nI2S tone done fed=%lu", (unsigned long)i2s_tone_fed);
	}
	if (i2s_evt_wave_done) {
		i2s_evt_wave_done = 0;
		printf_("\r\nI2S wave done fed=%lu", (unsigned long)i2s_wave_fed);
	}
}

void SPI2_IRQHandler(void)
{
	if (RESET == spi_i2s_interrupt_flag_get(SPI2, SPI_I2S_INT_FLAG_TBE)) {
		return;
	}

	if (i2s_tone_playing) {
		i2s_tone_feed_one();
		return;
	}

	if (i2s_wave_playing) {
		if (audiodataindex >= wave_struct.datasize) {
			spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
			i2s_disable(SPI2);
			i2s_wave_playing = 0;
			i2s_amp_apply(0);
			i2s_evt_wave_done = 1;
			return;
		}
		i2s_audio_data_send();
		return;
	}

	spi_i2s_interrupt_disable(SPI2, SPI_I2S_INT_TBE);
}
