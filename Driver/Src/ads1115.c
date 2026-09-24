// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "ads1115.h"
#include "i2c.h"
#include "rlk_gpio.h"
#include "systick.h"
#include "printf.h"

unsigned char TXByteCtr, RXByteCtr;
unsigned char TxData[4];
unsigned char RxBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static void ads1115_i2c_restore(void)
{
	i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	i2c_enable(I2C0);
}

/*!
 * Config high byte: OS=1, MUX, PGA=±4.096V, MODE=1 (single-shot)
 * Low byte: DR=860SPS, comparator disable (COMPAT with old 0xE0 style �?0xE3)
 */
static void ads1115_fill_cfg(unsigned char channel, unsigned char *cfg)
{
	unsigned char mux_hi;

	switch (channel) {
	case 1:
		mux_hi = 0xD3u; /* AIN1 */
		break;
	case 2:
		mux_hi = 0xE3u; /* AIN2 */
		break;
	case 3:
		mux_hi = 0xF3u; /* AIN3 */
		break;
	case 4:
		mux_hi = 0xB3u; /* AIN2 - AIN3 */
		break;
	case 0:
	default:
		mux_hi = 0xC3u; /* AIN0 */
		break;
	}
	cfg[0] = mux_hi;
	cfg[1] = 0xE3u; /* 860 SPS, COMP_QUE disable */
}

unsigned char confige_1115(unsigned char channel)
{
	printf_("\r\nADS1115 Init channel %d ...", channel);
	i2c_gpio_config(RCU_GPIOB, RCU_I2C0, FUN_I2C_PORT, FUN_I2C_PINS);
	i2c_config(I2C0, ADC_I2C_SPEED, 0x00u, RCU_I2C0);

	ads1115_fill_cfg(channel, TxData);
	I2C_AD_TxWord(AD_CFG_REG, TxData);
	return 0;
}

void I2C_AD_TxWord(unsigned char reg_addr, unsigned char *Reg_data)
{
	int number_of_byte = 2;

	while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
	}

	i2c_start_on_bus(I2C0);
	while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
	}

	i2c_master_addressing(I2C0, ADC_I2C_ADDR, I2C_TRANSMITTER);
	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

	while (SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)) {
	}

	i2c_data_transmit(I2C0, reg_addr);
	while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
	}

	while (number_of_byte--) {
		i2c_data_transmit(I2C0, *Reg_data);
		Reg_data++;
		while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
		}
	}

	i2c_stop_on_bus(I2C0);
	while (I2C_CTL0(I2C0) & 0x0200u) {
	}
}

void I2C_AD_RxWord(unsigned char *p_buffer, unsigned char Reg_addr)
{
	int number_of_byte = 2;

	while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
	}

	if (2 == number_of_byte) {
		i2c_ackpos_config(I2C0, I2C_ACKPOS_NEXT);
	}

	i2c_start_on_bus(I2C0);
	while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
	}

	i2c_master_addressing(I2C0, ADC_I2C_ADDR, I2C_TRANSMITTER);
	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

	while (SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)) {
	}

	i2c_enable(I2C0);
	i2c_data_transmit(I2C0, Reg_addr);
	while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
	}

	i2c_start_on_bus(I2C0);
	while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
	}

	i2c_master_addressing(I2C0, ADC_I2C_ADDR, I2C_RECEIVER);

	if (number_of_byte < 3) {
		i2c_ack_config(I2C0, I2C_ACK_DISABLE);
	}

	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

	if (1 == number_of_byte) {
		i2c_stop_on_bus(I2C0);
	}

	while (number_of_byte) {
		if (3 == number_of_byte) {
			while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
			}
			i2c_ack_config(I2C0, I2C_ACK_DISABLE);
		}
		if (2 == number_of_byte) {
			while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
			}
			i2c_stop_on_bus(I2C0);
		}

		if (i2c_flag_get(I2C0, I2C_FLAG_RBNE)) {
			*p_buffer = i2c_data_receive(I2C0);
			p_buffer++;
			number_of_byte--;
		}
	}

	while (I2C_CTL0(I2C0) & 0x0200u) {
	}

	/* critical for shared FUN_I2C (BH1750 etc.) */
	ads1115_i2c_restore();
}

static int16_t ads1115_read_raw(unsigned char channel)
{
	unsigned char cfg[2];
	uint16_t uraw;

	ads1115_fill_cfg(channel, cfg);
	I2C_AD_TxWord(AD_CFG_REG, cfg);

	/* single-shot @ 860SPS: typ < 2ms, wait with margin */
	delay_1ms(3);

	I2C_AD_RxWord(RxBuf, AD_CONVERT_REG);
	uraw = ((uint16_t)RxBuf[0] << 8) | RxBuf[1];
	return (int16_t)uraw;
}

float ads1115_read_volt(unsigned char channel)
{
	int16_t raw = ads1115_read_raw(channel);
	return ((float)raw * ADS1115_FS_VOLT) / (float)ADS1115_FULL_SCALE;
}

int ads1115_read_getadc(float *ain0, float *ain1, float *ain2, float *diff23)
{
	if ((ain0 == 0) || (ain1 == 0) || (ain2 == 0) || (diff23 == 0)) {
		return -1;
	}

	*ain0 = ads1115_read_volt(LIT_ADC_CHANNEL);
	*ain1 = ads1115_read_volt(VBUS_ADC_CHANNEL);
	*ain2 = ads1115_read_volt(DIFF_P_ADC_CHANNEL);
	*diff23 = ads1115_read_volt(DIFF_ADC_CHANNEL);
	return 0;
}

void ads1115_init(void)
{
	i2c_gpio_config(RCU_GPIOB, RCU_I2C0, FUN_I2C_PORT, FUN_I2C_PINS);
	/* master-only own address; device addr is 0x90 in transactions */
	i2c_config(I2C0, ADC_I2C_SPEED, 0x00u, RCU_I2C0);
	ads1115_i2c_restore();

	/* kick one conversion on AIN0 to verify bus */
	(void)ads1115_read_raw(LIT_ADC_CHANNEL);
	printf_("\r\nADS1115 ready @0x90 FUN_I2C FS=%.1fV", ADS1115_FS_VOLT);
}
