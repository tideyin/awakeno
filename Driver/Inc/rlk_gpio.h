// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef __RLK_GPIO_H__
#define __RLK_GPIO_H__
#include "gd32e10x_gpio.h"


//GPIOA
#define BEE_EN_PIN                GPIO_PIN_1
#define EEPROM_WP_PIN             GPIO_PIN_8  
  
#define BEE_EN_PORT                GPIOA  
#define EEPROM_WP_PORT             GPIOA  



//GPIOB
#define ADC_PIN                     GPIO_PIN_0
#define PA_CTL_PIN                  GPIO_PIN_2   /* BOOT1 work as PA Control */
#define KEY_MENU_PIN                GPIO_PIN_4


#define FUN_I2C_PINS              GPIO_PIN_6|GPIO_PIN_7    //I2C0
#define COM_I2C_PINS              GPIO_PIN_10|GPIO_PIN_11  //I2C1
#define RED_LED_EN_PIN              GPIO_PIN_14
#define BLUE_LED_EN_PIN             GPIO_PIN_15

#define ADC_PORT                    GPIOB
#define PA_CTL_PORT                 GPIOB
#define KEY_MENU_PORT               GPIOB


#define FUN_I2C_PORT               GPIOB
#define COM_I2C_PORT               GPIOB 
#define RED_LED_EN_PORT            GPIOB
#define BLUE_LED_EN_PORT           GPIOB



//GPIOC
#define IR_EMU_IO_PIN              GPIO_PIN_8   /* IR_SND / TIMER7_CH2 */
#define IR_MON_IO_PIN              GPIO_PIN_9   /* IR_RCV / TIMER7_CH3 */
#define PWM_AUDIO_PIN              GPIO_PIN_6   /* PWM_AUDIO / TIMER7_CH0 */

#define LED_R_PIN                 GPIO_PIN_13
#define LED_G_PIN                 GPIO_PIN_14
#define LED_B_PIN                 GPIO_PIN_15

#define IR_EMU_IO_PORT              GPIOC
#define IR_MON_IO_PORT              GPIOC
#define PWM_AUDIO_PORT              GPIOC

#define LED_R_PORT                 GPIOC
#define LED_G_PORT                 GPIOC
#define LED_B_PORT                 GPIOC





typedef struct {
    uint32_t port;
    uint32_t pin;
    uint32_t value;
} rlk_gpio_t;

int rlk_gpio_init(rlk_gpio_t *gpio_config, uint32_t gpios);
void rlk_gpio_set_value(uint32_t port, uint32_t pin, char value);
#endif
