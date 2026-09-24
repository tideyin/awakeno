// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include "rlk_key_state.h"
#include "rlk_cmd.h"
#include "function.h"

/*! Holds the current key-menu state */
static rlk_key_state_t g_key_state = SHOW_LOGO_STATE;

const STATE_TABLE_ENTRY_T key_state_table[KEY_STATE_NUM_STATES] =
{
	/* Handler Function                 Entry Function              */
	{ show_logo_state_handler,          show_logo_state_enter },
	{ show_product_state_handler,       show_product_state_enter },
	{ show_device_state_handler,        show_device_state_enter },
	{ show_user_info_state_handler,     show_user_info_state_enter },
	{ show_author_info_state_handler,   show_author_info_state_enter },
	{ show_state_info_state_handler,    show_state_info_state_enter },
};

void show_logo_state_enter(void)
{
	g_key_state = SHOW_LOGO_STATE;
	show_logo_state();
}

void show_logo_state_handler(void)
{
	beep();	
	show_product_state_enter();
}

void show_product_state_enter(void)
{
	g_key_state = SHOW_PRODUCT_STATE;
	show_product_state();
}

void show_product_state_handler(void)
{
	beep();	
	show_device_state_enter();
}

void show_device_state_enter(void)
{
	g_key_state = SHOW_DEVICE_STATE;
	show_device_state();
}

void show_device_state_handler(void)
{
	beep();	
	show_user_info_state_enter();
}

void show_user_info_state_enter(void)
{
	g_key_state = SHOW_USER_INFO_STATE;
	show_user_info_state();
}

void show_user_info_state_handler(void)
{
	beep();
	show_author_info_state_enter();
}

void show_author_info_state_enter(void)
{
	g_key_state = SHOW_AUTHOR_INFO_STATE;
	show_author_info_state();
}

void show_author_info_state_handler(void)
{
	beep();
	show_state_info_state_enter();
}

void show_state_info_state_enter(void)
{
	g_key_state = SHOW_STATE_INFO_STATE;
	show_state_info_state();
}

void show_state_info_state_handler(void)
{
	show_logo_state_enter();
}

void state_key_menu(void)
{
	if (g_key_state < KEY_STATE_NUM_STATES) {
		key_state_table[g_key_state].handler();
	}
}

void state_key_menu_init(uint8_t state)
{
	if (state >= KEY_STATE_NUM_STATES) {
		state = SHOW_LOGO_STATE;
	}
	key_state_table[state].entry_function();
}
