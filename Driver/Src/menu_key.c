// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include "systick.h"
#include "menu_key.h"
#include "rlk_gpio.h"

extern uint8_t menu_key_flag;

void key_menu_gpio_init(void)
{
	/* enable the key GPIO clock */
	rcu_periph_clock_enable(KEY_MENU_CLOCK);
	/* configure button pin as input */
	gpio_init(KEY_MENU_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, KEY_MENU_PIN);
}

key_press_mode_t key_pressed_mode(void)
{
	key_press_mode_t mode = NO_PRESS;
	uint32_t count = 0;
	int i;

	/* check whether the button is pressed */
	if (RESET == gpio_input_bit_get(KEY_MENU_PORT, KEY_MENU_PIN)) {
		for (i = 0; i < KEY_CHECK_COUNT; i++) {
			/* delay KEY_CHECK_INTERVAL for software removing jitter */
			delay_while_ms(KEY_CHECK_INTERVAL);
			/* check whether the button is pressed */
			if (RESET == gpio_input_bit_get(KEY_MENU_PORT, KEY_MENU_PIN)) {
				count++;
			} else {
				break;
			}
		}

		if (count >= LONG_PRESS_TIME / KEY_CHECK_INTERVAL) {
			mode = LONG_PRESS;
		} else {
			mode = SHORT_PRESS;
		}
	}

	return mode;
}

void key_menu_int_init(void)
{
	rcu_periph_clock_enable(RCU_AF);

	/* low priority  */
	nvic_irq_enable(KEY_MENU_EXIT_IRQn, 2U, 0U);

	gpio_exti_source_select(KEY_MENU_PORT_SOURCE, KEY_MENU_PIN_SOURCE);
	exti_init(KEY_MENU_EXIT, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
	exti_interrupt_flag_clear(KEY_MENU_EXIT);
}

void EXTI4_IRQHandler(void)
{
	if (SET == exti_interrupt_flag_get(KEY_MENU_EXIT)) {
		menu_key_flag = 1;
		exti_interrupt_flag_clear(KEY_MENU_EXIT);
	}

	nvic_irq_disable(KEY_MENU_EXIT_IRQn);
}
