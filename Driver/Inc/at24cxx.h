/*!
    \file    at24cxx.h
    \brief   the header file of AT24Cxx

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

#ifndef AT24CXX_I2C1_H
#define AT24CXX_I2C1_H

#include "gd32e10x.h"


#define EEPROM_BLOCK0_ADDRESS    0xAC

#define EEP_FIRST_PAGE         0x00

#define I2C_OK                 0
#define I2C_FAIL               1
#define AT24CXX_BIT_CHECK_COUNT 10

#if 0
enum
{
    FALSE = 0 ,
    TRUE
};
#endif


#if 0

#define AT24CXX_BIT_CHECK_SUCCESS(bit)  while(bit)

#else

#define AT24CXX_BIT_CHECK_SUCCESS(bit)                \
    {                                                 \
        int i = 0;                                    \
        for (i = 0; i < AT24CXX_BIT_CHECK_COUNT; i++) \
        {                                             \
            if (!(bit))                               \
                break;                                \
            delay_while_ms(1);                        \
        }                                             \
        if (i == AT24CXX_BIT_CHECK_COUNT)             \
            return;                                   \
    }
#endif 

/* I2C read and write functions */
uint8_t i2c_24c02_test(void);
/* I2C read and write functions */
uint8_t i2c_24c02_test_ext(void);
/* initialize EEPROM address */
void i2c_eeprom_init(void);
/* deinitialize EEPROM address */
void i2c_eeprom_deinit(void);
/* write one byte to the EEPROM */
void eeprom_byte_write(uint8_t* p_buffer, uint8_t write_address);
/* write more than one byte to the EEPROM */
void eeprom_page_write(uint8_t* p_buffer, uint8_t write_address, uint8_t number_of_byte);
/* write buffer of data to the EEPROM */
void eeprom_buffer_write(uint8_t* p_buffer, uint8_t write_address, uint16_t number_of_byte);
/* read data from the EEPROM */
void eeprom_buffer_read(uint8_t* p_buffer, uint8_t read_address, uint16_t number_of_byte);
/* wait for EEPROM standby state */
void eeprom_wait_standby_state(void);


void eeprom_buffer_read_wrapper(uint8_t* p_buffer, uint8_t read_address,
                                       uint16_t number_of_byte);
void eeprom_buffer_write_wrapper(uint8_t* p_buffer, uint8_t write_address,
                                        uint16_t number_of_byte);



#endif  /* AT24CXX_I2C1_H */
