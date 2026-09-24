// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "adc_gd32.h"
#include "rlk_gpio.h"
#include "systick.h"
#include "printf.h"

void adc_in_init(void)
{
	/* PB0 = ADC01_IN8 */
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_ADC0);
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);

	gpio_init(ADC_PORT, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, ADC_PIN);

	adc_deinit(ADC0);
	adc_mode_config(ADC_MODE_FREE);
	adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
	adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1U);
	adc_regular_channel_config(ADC0, 0U, ADC_CHANNEL_8, ADC_SAMPLETIME_55POINT5);
	adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_NONE);
	adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
	adc_enable(ADC0);

	delay_1ms(1);
	adc_calibration_enable(ADC0);

	printf_("\r\nADC_IN PB0/CH8 ready Vref=%.1fV", ADC_IN_VREF_VOLT);
}

float adc_in_read_volt(void)
{
	uint16_t raw;
	uint32_t guard = 100000u;

	adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
	while (!adc_flag_get(ADC0, ADC_FLAG_EOC)) {
		if (--guard == 0u) {
			return 0.0f;
		}
	}
	adc_flag_clear(ADC0, ADC_FLAG_EOC);
	raw = adc_regular_data_read(ADC0);

	return ((float)raw * ADC_IN_VREF_VOLT) / (float)ADC_IN_FULL_SCALE;
}
