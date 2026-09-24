/*!
    \file    at24cxx.c
    \brief   the read and write function file

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

#include "at24cxx.h"
#include "i2c.h"
#include <stdio.h>
#include "rlk_cdc.h"
#include "rlk_gpio.h"



#define BUFFER_SIZE              256

#if 1
enum
{
    FALSE = 0 ,
    TRUE
};

#endif

static uint16_t eeprom_address;
static uint8_t g_s_eeprom_init_flag = FALSE;

//Tide change here
#define EEPROM_I2C_SPEED              100000
//#define EEPROM_I2C_ADDR               0xA0
#define I2C_PAGE_SIZE           8


/*!
    \brief      I2C read and write functions
    \param[in]  none
    \param[out] none
    \retval     I2C_OK or I2C_FAIL
*/
uint8_t i2c_24c02_test(void)
{
    uint16_t i;
    uint8_t i2c_buffer_write[BUFFER_SIZE];
    uint8_t i2c_buffer_read[BUFFER_SIZE];


    /* initialize i2c_buffer_write */
    for(i = 0;i < BUFFER_SIZE;i++){
        i2c_buffer_write[i]=i;
        if(15 == i%16){
        }
    }
    /* EEPROM data write */
    eeprom_buffer_write(i2c_buffer_write,EEP_FIRST_PAGE, BUFFER_SIZE);
    /* EEPROM data read */
    eeprom_buffer_read(i2c_buffer_read,EEP_FIRST_PAGE, BUFFER_SIZE);
    /* compare the read buffer and write buffer */
    for(i = 0;i < BUFFER_SIZE;i++){
        if(i2c_buffer_read[i] != i2c_buffer_write[i]){
            return I2C_FAIL;
        }
        if(15 == i%16){
        }
    }
    return I2C_OK;
}

/*!
    \brief      I2C read and write functions , test external EEPROM ,diff ADDRESS
    \param[in]  none
    \param[out] none
    \retval     I2C_OK or I2C_FAIL
*/
uint8_t i2c_24c02_test_ext(void)
{
    uint16_t i;
    uint8_t i2c_buffer_write[BUFFER_SIZE];
    uint8_t i2c_buffer_read[BUFFER_SIZE];


    /* configure I2C , set External EEPROM address , UBKADD is 0xA4*/
    i2c_config(I2C1,EEPROM_I2C_SPEED,EEPROM_BLOCK0_ADDRESS+4,RCU_I2C1);
	eeprom_address = EEPROM_BLOCK0_ADDRESS+4 ;
	g_s_eeprom_init_flag = FALSE ;

    /* initialize i2c_buffer_write */
    for(i = 0;i < BUFFER_SIZE;i++){
        i2c_buffer_write[i]=i;
        if(15 == i%16){
        }
    }
    /* EEPROM data write */
    eeprom_buffer_write(i2c_buffer_write,EEP_FIRST_PAGE, BUFFER_SIZE);
    /* EEPROM data read */
    eeprom_buffer_read(i2c_buffer_read,EEP_FIRST_PAGE, BUFFER_SIZE);

//Resume eeprom I2C config
	i2c_eeprom_init() ;




	
    /* compare the read buffer and write buffer */
    for(i = 0;i < BUFFER_SIZE;i++){
        if(i2c_buffer_read[i] != i2c_buffer_write[i]){
            return I2C_FAIL;
        }
        if(15 == i%16){
        }
    }
    return I2C_OK;
}


/*!
    \brief      initialize peripherals used by the I2C EEPROM driver
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_eeprom_init(void)
{
    if(g_s_eeprom_init_flag == TRUE)
        return;

    eeprom_address = EEPROM_BLOCK0_ADDRESS;

  //  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, EEPROM_A0 | EEPROM_A1 | EEPROM_A2);
  //  gpio_bit_reset(GPIOC, EEPROM_A0 | EEPROM_A1 | EEPROM_A2);

    gpio_init(EEPROM_WP_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, EEPROM_WP_PIN);  //  gpio_bit_reset(EEPROM_WR_PORT, EEPROM_WR_PIN);
 //   gpio_bit_set(EEPROM_WR_PORT, EEPROM_WR_PIN);
     rlk_gpio_set_value(EEPROM_WP_PORT, EEPROM_WP_PIN, 0);  // 0 , no protect
     delay_1ms(10) ;

    /* configure GPIO */
    i2c_gpio_config(RCU_GPIOB , RCU_I2C1 , COM_I2C_PORT , COM_I2C_PINS);

    /* configure I2C */
    i2c_config(I2C1,EEPROM_I2C_SPEED,EEPROM_BLOCK0_ADDRESS,RCU_I2C1);

    g_s_eeprom_init_flag = TRUE;
}

/*!
    \brief      deinitialize peripherals used by the I2C EEPROM driver
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_eeprom_deinit(void)
{
    if(g_s_eeprom_init_flag == FALSE)
        return;

    eeprom_address = NULL;

    /* configure I2C */

    /* enable I2C1 */
    i2c_disable(I2C1);

    /* disable I2C clock */
    rcu_periph_clock_disable(RCU_I2C1);

    i2c_deinit(I2C1);

    gpio_init(COM_I2C_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_I2C_PINS);

//    gpio_bit_reset(GPIOC, EEPROM_A0 | EEPROM_A2);
//    gpio_bit_set(GPIOC, EEPROM_A1);

    g_s_eeprom_init_flag = FALSE;
}

/*!
    \brief      write buffer of data to the I2C EEPROM
    \param[in]  p_buffer: pointer to the buffer  containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write to the EEPROM
    \param[out] none
    \retval     none
*/
void eeprom_buffer_write(uint8_t* p_buffer, uint8_t write_address, uint16_t number_of_byte)
{
    uint8_t number_of_page = 0, number_of_single = 0, address = 0, count = 0;

    address = write_address % I2C_PAGE_SIZE;
    count = I2C_PAGE_SIZE - address;
    number_of_page =  number_of_byte / I2C_PAGE_SIZE;
    number_of_single = number_of_byte % I2C_PAGE_SIZE;

    /* if write_address is I2C_PAGE_SIZE aligned  */
    if(0 == address){
        while(number_of_page--){
            eeprom_page_write(p_buffer, write_address, I2C_PAGE_SIZE);
            eeprom_wait_standby_state();
            write_address +=  I2C_PAGE_SIZE;
            p_buffer += I2C_PAGE_SIZE;
        }
        if(0 != number_of_single){
            eeprom_page_write(p_buffer, write_address, number_of_single);
            eeprom_wait_standby_state();
        }
    }else{
        /* if write_address is not I2C_PAGE_SIZE aligned */
        if(number_of_byte < count){
            eeprom_page_write(p_buffer, write_address, number_of_byte);
            eeprom_wait_standby_state();
        }else{
            number_of_byte -= count;
            number_of_page =  number_of_byte / I2C_PAGE_SIZE;
            number_of_single = number_of_byte % I2C_PAGE_SIZE;

            if(0 != count){
                eeprom_page_write(p_buffer, write_address, count);
                eeprom_wait_standby_state();
                write_address += count;
                p_buffer += count;
            }
            /* write page */
            while(number_of_page--){
                eeprom_page_write(p_buffer, write_address, I2C_PAGE_SIZE);
                eeprom_wait_standby_state();
                write_address +=  I2C_PAGE_SIZE;
                p_buffer += I2C_PAGE_SIZE;
            }
            /* write single */
            if(0 != number_of_single){
                eeprom_page_write(p_buffer, write_address, number_of_single);
                eeprom_wait_standby_state();
            }
        }
    }
}

/*!
    \brief      write one byte to the I2C EEPROM
    \param[in]  p_buffer: pointer to the buffer containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[out] none
    \retval     none
*/
void eeprom_byte_write(uint8_t* p_buffer, uint8_t write_address)
{
	// fix write error issue with 6m delay
	delay_1ms(6) ;

    /* wait until I2C bus is idle */
    AT24CXX_BIT_CHECK_SUCCESS(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, eeprom_address, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    AT24CXX_BIT_CHECK_SUCCESS(SET != i2c_flag_get(I2C1, I2C_FLAG_TBE));

    /* send the EEPROM's internal address to write to : only one byte address */
    i2c_data_transmit(I2C1, write_address);

    /* wait until BTC bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send the byte to be written */
    i2c_data_transmit(I2C1, *p_buffer);

    /* wait until BTC bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C1);

    /* wait until the stop condition is finished */
    AT24CXX_BIT_CHECK_SUCCESS(I2C_CTL0(I2C1)&0x0200);
}

/*!
    \brief      write more than one byte to the EEPROM with a single write cycle
    \param[in]  p_buffer: pointer to the buffer containing the data to be written to the EEPROM
    \param[in]  write_address: EEPROM's internal address to write to
    \param[in]  number_of_byte: number of bytes to write to the EEPROM
    \param[out] none
    \retval     none
*/
void eeprom_page_write(uint8_t* p_buffer, uint8_t write_address, uint8_t number_of_byte)
{
	// fix write error issue with 6m delay
	delay_1ms(6) ;

    /* wait until I2C bus is idle */
    AT24CXX_BIT_CHECK_SUCCESS(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, eeprom_address, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));
    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    AT24CXX_BIT_CHECK_SUCCESS( SET != i2c_flag_get(I2C1, I2C_FLAG_TBE));

    /* send the EEPROM's internal address to write to : only one byte address */
    i2c_data_transmit(I2C1, write_address);

    /* wait until BTC bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* while there is data to be written */
    while(number_of_byte--){
        i2c_data_transmit(I2C1, *p_buffer);

        /* point to the next byte to be written */
        p_buffer++;

        /* wait until BTC bit is set */
        AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));
    }
    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C1);

    /* wait until the stop condition is finished */
    AT24CXX_BIT_CHECK_SUCCESS(I2C_CTL0(I2C1)&0x0200);
}

/*!
    \brief      read data from the EEPROM
    \param[in]  p_buffer: pointer to the buffer that receives the data read from the EEPROM
    \param[in]  read_address: EEPROM's internal address to start reading from
    \param[in]  number_of_byte: number of bytes to reads from the EEPROM
    \param[out] none
    \retval     none
*/
void eeprom_buffer_read(uint8_t* p_buffer, uint8_t read_address, uint16_t number_of_byte)
{
    /* wait until I2C bus is idle */
    AT24CXX_BIT_CHECK_SUCCESS(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

    if(2 == number_of_byte){
        i2c_ackpos_config(I2C1,I2C_ACKPOS_NEXT);
    }

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, eeprom_address, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    AT24CXX_BIT_CHECK_SUCCESS(SET != i2c_flag_get( I2C1 , I2C_FLAG_TBE));

    /* enable I2C1*/
    i2c_enable(I2C1);

    /* send the EEPROM's internal address to write to */
    i2c_data_transmit(I2C1, read_address);

    /* wait until BTC bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, eeprom_address, I2C_RECEIVER);

    if(number_of_byte < 3){
        /* disable acknowledge */
        i2c_ack_config(I2C1,I2C_ACK_DISABLE);
    }

    /* wait until ADDSEND bit is set */
    AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    if(1 == number_of_byte){
        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(I2C1);
    }

    /* while there is data to be read */
    while(number_of_byte){
        if(3 == number_of_byte){
            /* wait until BTC bit is set */
            AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

            /* disable acknowledge */
            i2c_ack_config(I2C1,I2C_ACK_DISABLE);
        }
        if(2 == number_of_byte){
            /* wait until BTC bit is set */
            AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C1);
        }

        /* wait until the RBNE bit is set and clear it */
        if(i2c_flag_get(I2C1, I2C_FLAG_RBNE)){
            /* read a byte from the EEPROM */
            *p_buffer = i2c_data_receive(I2C1);

            /* point to the next location where the byte read will be saved */
            p_buffer++;

            /* decrement the read bytes counter */
            number_of_byte--;
        }
    }

    /* wait until the stop condition is finished */
    AT24CXX_BIT_CHECK_SUCCESS(I2C_CTL0(I2C1)&0x0200);

    /* enable acknowledge */
    i2c_ack_config(I2C1,I2C_ACK_ENABLE);

    i2c_ackpos_config(I2C1,I2C_ACKPOS_CURRENT);
}

/*!
    \brief      wait for EEPROM standby state
    \param[in]  none
    \param[out] none
    \retval     none
*/
void eeprom_wait_standby_state(void)
{
    __IO uint32_t val = 0;

    while(1){
        /* wait until I2C bus is idle */
        AT24CXX_BIT_CHECK_SUCCESS(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

        /* send a start condition to I2C bus */
        i2c_start_on_bus(I2C1);

        /* wait until SBSEND bit is set */
        AT24CXX_BIT_CHECK_SUCCESS(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

        /* send slave address to I2C bus */
        i2c_master_addressing(I2C1, eeprom_address, I2C_TRANSMITTER);
#if 0
        /* keep looping till the Address is acknowledged or the AE flag is set (address not acknowledged at time) */
        do{
            /* get the current value of the I2C_STAT0 register */
            val = I2C_STAT0(I2C1);

        }while(0 == (val & (I2C_STAT0_ADDSEND | I2C_STAT0_AERR)));
#endif

        for(int i = 0; i < 10; i++){
            /* get the current value of the I2C_STAT0 register */
            val = I2C_STAT0(I2C1);
            if((0 != (val & (I2C_STAT0_ADDSEND | I2C_STAT0_AERR))))
                break;
            delay_while_ms(1);
        }

        /* check if the ADDSEND flag has been set */
        if(val & I2C_STAT0_ADDSEND){

            /* clear ADDSEND flag */
            i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

            /* send a stop condition to I2C bus */
            i2c_stop_on_bus(I2C1);

            /* exit the function */
            return ;

        }else{
            /* clear the bit of AE */
            i2c_flag_clear(I2C1,I2C_FLAG_AERR);
        }

        /* send a stop condition to I2C bus */
        i2c_stop_on_bus(I2C1);
        /* wait until the stop condition is finished */
        AT24CXX_BIT_CHECK_SUCCESS(I2C_CTL0(I2C1)&0x0200);

        /* exit the function */
        return;
    }
}



void eeprom_buffer_read_wrapper(uint8_t* p_buffer, uint8_t read_address,
                                       uint16_t number_of_byte)
{
	 i2c_eeprom_init();
	 eeprom_buffer_read(p_buffer, read_address, number_of_byte);
 //  rlk_gpio_set_value(EEPROM_WP_PORT, EEPROM_WP_PIN, );  // 0 ,  protect
  //   i2c_eeprom_deinit();


}

void eeprom_buffer_write_wrapper(uint8_t* p_buffer, uint8_t write_address,
                                        uint16_t number_of_byte)
{
     i2c_eeprom_deinit();
	 delay_1ms(10);
     i2c_eeprom_init();
	 rlk_gpio_set_value(EEPROM_WP_PORT, EEPROM_WP_PIN, 0);  // 0 , no protect
	 delay_1ms(2);
	 eeprom_buffer_write(p_buffer, write_address, number_of_byte);
	 delay_1ms(2);
	 rlk_gpio_set_value(EEPROM_WP_PORT, EEPROM_WP_PIN,1 );  // 1 ,  protect
										   
	 

}

