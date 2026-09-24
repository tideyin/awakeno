// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef MENU_KEY_H
#define MENU_KEY_H

#define KEY_MENU_CLOCK RCU_GPIOB

#define KEY_CHECK_INTERVAL 100
#define KEY_CHECK_COUNT 20
#define LONG_PRESS_TIME 1500

/* Interrupt GPIO — Awakeno KEY_MENU = PB4 */
#define KEY_MENU_EXIT_IRQn       EXTI4_IRQn
#define KEY_MENU_EXIT            EXTI_4
#define KEY_MENU_PORT_SOURCE     GPIO_PORT_SOURCE_GPIOB
#define KEY_MENU_PIN_SOURCE      GPIO_PIN_SOURCE_4

typedef enum {
	NO_PRESS = 0,
	SHORT_PRESS,
	LONG_PRESS
} key_press_mode_t;

void key_menu_gpio_init(void);
void key_menu_int_init(void);

key_press_mode_t key_pressed_mode(void);

#endif /* MENU_KEY_H */
