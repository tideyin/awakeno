// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"


#define MCP4725_ADDR 0xC0

void mcp4725_set_value(uint16_t val_trans)
{

//	val_trans = 0x0FFF ;

	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, MCP4725_ADDR, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while(SET != i2c_flag_get(I2C1, I2C_FLAG_TBE));

    /* send the EEPROM's internal address to write to : only one byte address */
    i2c_data_transmit(I2C1, (val_trans >> 8) & 0xf);

    /* wait until BTC bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send the byte to be written */
    i2c_data_transmit(I2C1, val_trans & 0xff);

    /* wait until BTC bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C1);

    /* wait until the stop condition is finished */
    while(I2C_CTL0(I2C1)&0x0200);
}
