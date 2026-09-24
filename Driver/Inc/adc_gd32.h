// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef ADC_GD32_H
#define ADC_GD32_H

#include "gd32e10x.h"

/*******************************************************************************
 * GD32 PB0 — ADC_IN (ADC0 channel 8)
 * Hardware: MCP4725 output / 2.54 probe (awakeno_gpio.md)
 * Vref = 3.3 V, 12-bit
 *******************************************************************************/

#define ADC_IN_VREF_VOLT            3.3f
#define ADC_IN_FULL_SCALE           4095u

void  adc_in_init(void);
float adc_in_read_volt(void);

#endif /* ADC_GD32_H */
