// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef BH1750_H
#define BH1750_H

#include "gd32e10x.h"

/*******************************************************************************
 * BH1750FVI — digital ambient light sensor (I2C)
 *
 * Bus: I2C0 FUN_I2C — PB6=SCL, PB7=SDA (shared GYRO/ADC/LIT)
 * Handbook: Materials/handbook/BH1750FVI-TR_I2C_Light_Sensor.PDF
 *
 * ADDR pin = H → 7-bit 0x5C → 8-bit write 0xB8 (board fixed)
 *******************************************************************************/

#define BH1750_I2C_SPEED            100000u
#define BH1750_I2C_ADDR             0xB8u

/* Instruction set (datasheet) */
#define BH1750_CMD_POWER_DOWN       0x00u
#define BH1750_CMD_POWER_ON         0x01u
#define BH1750_CMD_RESET            0x07u
#define BH1750_CMD_CONT_H_RES       0x10u  /* 1 lx,   typ 120 ms */
#define BH1750_CMD_CONT_H_RES2      0x11u  /* 0.5 lx, typ 120 ms */
#define BH1750_CMD_CONT_L_RES       0x13u  /* 4 lx,   typ 16 ms  */
#define BH1750_CMD_ONCE_H_RES       0x20u
#define BH1750_CMD_ONCE_H_RES2      0x21u
#define BH1750_CMD_ONCE_L_RES       0x23u

void     bh1750_init(void);
int      bh1750_write_cmd(uint8_t cmd);
int      bh1750_read_raw(uint16_t *raw);
/* integer lux; returns 0 on I2C/sensor error */
uint32_t bh1750_read_lux(void);
void     bh1750_power_down(void);
void     bh1750_power_on(void);
int      bh1750_reset(void);
int      bh1750_set_mode(uint8_t mode);

#endif /* BH1750_H */
