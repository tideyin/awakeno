// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef ADS1115_H
#define ADS1115_H

#include "gd32e10x.h"

/*******************************************************************************
 * ADS1115 — 16-bit ADC (I2C)
 *
 * Bus: I2C0 FUN_I2C — PB6=SCL, PB7=SDA
 * Address: 0x90 (ADDR → GND)
 * PGA: ±4.096 V (closest to 3.3 V rail). Voltage = raw * 4.096 / 32768
 *******************************************************************************/

#define AD_CONVERT_REG              0x00u
#define AD_CFG_REG                  0x01u
#define AD_LO_LIM_REG               0x02u
#define AD_HI_LIM_REG               0x03u

#define ADC_I2C_SPEED               100000u
#define ADC_I2C_ADDR                0x90u

#define LIT_ADC_CHANNEL             0u   /* AIN0 */
#define VBUS_ADC_CHANNEL            1u   /* AIN1 = VBUS 1/2 */
#define DIFF_P_ADC_CHANNEL          2u   /* AIN2 */
#define DIFF_N_ADC_CHANNEL          3u   /* AIN3 */
#define DIFF_ADC_CHANNEL            4u   /* AIN2 - AIN3 */

/* Must match PGA in ads1115_fill_cfg (001 = ±4.096 V) */
#define ADS1115_FS_VOLT             4.096f
#define ADS1115_FULL_SCALE          32768

/* legacy */
unsigned char confige_1115(unsigned char channel);
void I2C_AD_TxWord(unsigned char reg_addr, unsigned char *Reg_data);
void I2C_AD_RxWord(unsigned char *p_buffer, unsigned char Reg_addr);

void     ads1115_init(void);
/* channel: 0=AIN0 .. 3=AIN3, 4=AIN2-AIN3 diff; returns volts (signed for diff) */
float    ads1115_read_volt(unsigned char channel);
/* AIN0, AIN1, AIN2, Diff(AIN2-AIN3) */
int      ads1115_read_getadc(float *ain0, float *ain1, float *ain2, float *diff23);

#endif /* ADS1115_H */
