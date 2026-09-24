// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "bh1750.h"
#include "i2c.h"
#include "rlk_gpio.h"
#include "systick.h"
#include "printf.h"

static uint8_t bh1750_mode = BH1750_CMD_CONT_H_RES;
static uint8_t bh1750_ready = 0;

static void bh1750_i2c_recover(void)
{
	/* ADS1115 leaves ACKPOS/ACK dirty �?restore before each BH1750 xfer */
	if (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
		i2c_stop_on_bus(I2C0);
		{
			uint32_t g = 10000u;
			while ((I2C_CTL0(I2C0) & 0x0200u) && (--g != 0u)) {
			}
		}
	}
	i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);
	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	i2c_flag_clear(I2C0, I2C_FLAG_AERR);
	i2c_enable(I2C0);
}

static int bh1750_wait_addr_acked(void)
{
	uint32_t guard = 100000u;

	while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND)) {
		if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
			i2c_flag_clear(I2C0, I2C_FLAG_AERR);
			i2c_stop_on_bus(I2C0);
			return -1;
		}
		if (--guard == 0u) {
			i2c_stop_on_bus(I2C0);
			return -1;
		}
	}
	i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);
	if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
		i2c_flag_clear(I2C0, I2C_FLAG_AERR);
		i2c_stop_on_bus(I2C0);
		return -1;
	}
	return 0;
}

static uint16_t bh1750_mode_wait_ms(uint8_t mode)
{
	switch (mode) {
	case BH1750_CMD_CONT_L_RES:
	case BH1750_CMD_ONCE_L_RES:
		return 24u;
	case BH1750_CMD_CONT_H_RES:
	case BH1750_CMD_CONT_H_RES2:
	case BH1750_CMD_ONCE_H_RES:
	case BH1750_CMD_ONCE_H_RES2:
	default:
		return 180u;
	}
}

int bh1750_write_cmd(uint8_t cmd)
{
	uint32_t guard = 100000u;

	bh1750_i2c_recover();

	while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
		if (--guard == 0u) {
			return -1;
		}
	}

	i2c_start_on_bus(I2C0);
	guard = 100000u;
	while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
		if (--guard == 0u) {
			return -1;
		}
	}

	i2c_master_addressing(I2C0, BH1750_I2C_ADDR, I2C_TRANSMITTER);
	if (0 != bh1750_wait_addr_acked()) {
		return -1;
	}

	guard = 100000u;
	while (SET != i2c_flag_get(I2C0, I2C_FLAG_TBE)) {
		if (--guard == 0u) {
			i2c_stop_on_bus(I2C0);
			return -1;
		}
	}

	i2c_data_transmit(I2C0, cmd);
	guard = 100000u;
	while (!i2c_flag_get(I2C0, I2C_FLAG_BTC)) {
		if (i2c_flag_get(I2C0, I2C_FLAG_AERR)) {
			i2c_flag_clear(I2C0, I2C_FLAG_AERR);
			i2c_stop_on_bus(I2C0);
			return -1;
		}
		if (--guard == 0u) {
			i2c_stop_on_bus(I2C0);
			return -1;
		}
	}

	i2c_stop_on_bus(I2C0);
	guard = 100000u;
	while (I2C_CTL0(I2C0) & 0x0200u) {
		if (--guard == 0u) {
			return -1;
		}
	}
	return 0;
}

int bh1750_read_raw(uint16_t *raw)
{
	uint8_t hi = 0;
	uint8_t lo = 0;
	uint32_t guard = 100000u;

	if (raw == 0) {
		return -1;
	}

	bh1750_i2c_recover();

	while (i2c_flag_get(I2C0, I2C_FLAG_I2CBSY)) {
		if (--guard == 0u) {
			return -1;
		}
	}

	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);

	i2c_start_on_bus(I2C0);
	guard = 100000u;
	while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND)) {
		if (--guard == 0u) {
			return -1;
		}
	}

	i2c_master_addressing(I2C0, BH1750_I2C_ADDR, I2C_RECEIVER);
	if (0 != bh1750_wait_addr_acked()) {
		return -1;
	}

	guard = 100000u;
	while (!i2c_flag_get(I2C0, I2C_FLAG_RBNE)) {
		if (--guard == 0u) {
			i2c_stop_on_bus(I2C0);
			return -1;
		}
	}
	i2c_ack_config(I2C0, I2C_ACK_DISABLE);
	i2c_stop_on_bus(I2C0);
	hi = (uint8_t)i2c_data_receive(I2C0);

	guard = 100000u;
	while (!i2c_flag_get(I2C0, I2C_FLAG_RBNE)) {
		if (--guard == 0u) {
			return -1;
		}
	}
	lo = (uint8_t)i2c_data_receive(I2C0);

	guard = 100000u;
	while (I2C_CTL0(I2C0) & 0x0200u) {
		if (--guard == 0u) {
			break;
		}
	}

	i2c_ack_config(I2C0, I2C_ACK_ENABLE);
	i2c_ackpos_config(I2C0, I2C_ACKPOS_CURRENT);

	*raw = ((uint16_t)hi << 8) | lo;
	return 0;
}

void bh1750_power_down(void)
{
	(void)bh1750_write_cmd(BH1750_CMD_POWER_DOWN);
}

void bh1750_power_on(void)
{
	(void)bh1750_write_cmd(BH1750_CMD_POWER_ON);
}

int bh1750_reset(void)
{
	bh1750_power_on();
	return bh1750_write_cmd(BH1750_CMD_RESET);
}

int bh1750_set_mode(uint8_t mode)
{
	if (0 != bh1750_write_cmd(mode)) {
		return -1;
	}
	bh1750_mode = mode;
	delay_1ms(bh1750_mode_wait_ms(mode));
	return 0;
}

uint32_t bh1750_read_lux(void)
{
	uint16_t raw = 0;

	if (!bh1750_ready) {
		return 0u;
	}

	if (0 != bh1750_set_mode(BH1750_CMD_ONCE_H_RES)) {
		printf_("\r\nBH1750 measure cmd fail");
		return 0u;
	}

	if (0 != bh1750_read_raw(&raw)) {
		printf_("\r\nBH1750 read fail");
		return 0u;
	}

	printf_("\r\nBH1750 raw=0x%04x", raw);
	return ((uint32_t)raw * 5u) / 6u;
}

void bh1750_init(void)
{
	bh1750_ready = 0;

	/* FUN_I2C0 PB6/PB7 �?shared with ADS1115 / gyro.
	 * Own address 0x00 (master only); sensor fixed at 0xB8. */
	i2c_gpio_config(RCU_GPIOB, RCU_I2C0, FUN_I2C_PORT, FUN_I2C_PINS);
	i2c_config(I2C0, BH1750_I2C_SPEED, 0x00u, RCU_I2C0);
	bh1750_i2c_recover();

	if (0 != bh1750_write_cmd(BH1750_CMD_POWER_ON)) {
		printf_("\r\nBH1750 power-on fail (addr 0xB8)");
		return;
	}
	delay_1ms(10);
	if (0 != bh1750_write_cmd(BH1750_CMD_RESET)) {
		printf_("\r\nBH1750 reset fail");
		return;
	}
	delay_1ms(10);

	if (0 != bh1750_set_mode(BH1750_CMD_CONT_H_RES)) {
		printf_("\r\nBH1750 set mode fail");
		return;
	}

	bh1750_ready = 1;
	printf_("\r\nBH1750 ready @0xB8");
}
