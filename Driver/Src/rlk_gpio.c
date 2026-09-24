// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x_rcu.h"
#include "gd32e10x_gpio.h"
#include "rlk_gpio.h"

extern uint8_t hw_ver ;





void rlk_gpio_set_value(uint32_t port, uint32_t pin, char value)
{
	gpio_bit_write(port, pin, (bit_status)value);
}

int rlk_gpio_init(rlk_gpio_t *gpio_config, uint32_t gpios)
{
    int i = 0;
	
	printf_("\r\nTTLAB GPIO init...") ;

		if (!gpio_config || !gpios) {
				return -1;
		}


	rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE); //Disable JTAG for PA15

	  for (i = 0; i < gpios; i++) {
			gpio_init(gpio_config[i].port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, gpio_config[i].pin);
			rlk_gpio_set_value(gpio_config[i].port, gpio_config[i].pin, gpio_config[i].value);
    } 

	  /* configure COM_I2C GPIO */
	  i2c_gpio_config(COM_I2C_PORT , RCU_I2C1 , COM_I2C_PORT , COM_I2C_PINS);
	  /* configure FUN_I2C GPIO */
	  i2c_gpio_config(FUN_I2C_PORT , RCU_I2C0 , FUN_I2C_PORT , FUN_I2C_PINS);


		
  	  return 0;
		
}



