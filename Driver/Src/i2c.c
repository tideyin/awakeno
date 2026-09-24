/*!
    \file    i2c.c
    \brief   I2C configuration file

    \version 2018-03-26, V1.0.0, demo for GD32E103
    \version 2020-09-30, V1.1.0, demo for GD32E103
    \version 2020-12-31, V1.2.0, demo for GD32E103
*/

/*
    Copyright (c) 2020, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32e10x.h"
#include "i2c.h"
#include <stdio.h>

/*!
    \brief      configure the GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none 
*/
void i2c_gpio_config(rcu_periph_enum       rcu_port, rcu_periph_enum rcu_i2c , uint32_t i2c_port , uint32_t i2c_pin)
{
    /* enable GPIOB clock */
    rcu_periph_clock_enable(RCU_GPIOB);
    /* enable I2C0 clock */
    rcu_periph_clock_enable(RCU_I2C0);  
    /* enable I2C1 clock */
    rcu_periph_clock_enable(RCU_I2C1);
    /* connect PB10 to I2C1_SCL */
    /* connect PB11 to I2C1_SDA */
    gpio_init(i2c_port, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, i2c_pin);
}

/*!
    \brief      configure the I2C0 interfaces
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_config(uint32_t i2c_port , uint32_t i2c_speed , uint32_t i2c_slave_addr , rcu_periph_enum rcu_i2c)
{
    /* enable I2C clock */
    rcu_periph_clock_enable(rcu_i2c);
    /* configure I2C clock */
    i2c_clock_config(i2c_port,i2c_speed,I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(i2c_port,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,i2c_slave_addr);
    /* enable I2Cx */
    i2c_enable(i2c_port);
    /* enable acknowledge */
    i2c_ack_config(i2c_port,I2C_ACK_ENABLE);
}



