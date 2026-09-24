// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef RLK_KEY_STATE_H
#define RLK_KEY_STATE_H

#include <stdint.h>

/* Key-menu display states (table-driven, MSP430 TchMini style) */
typedef enum {
	SHOW_LOGO_STATE = 0,
	SHOW_PRODUCT_STATE,
	SHOW_DEVICE_STATE,
	SHOW_USER_INFO_STATE,
	SHOW_AUTHOR_INFO_STATE,
	SHOW_STATE_INFO_STATE,
	KEY_STATE_NUM_STATES
} rlk_key_state_t;

/*! Structure defining format of state table entries */
typedef struct {
	/*! Pointer to state event/timeout handler function */
	void (*handler)(void);
	/*! Pointer to state entry function */
	void (*entry_function)(void);
} STATE_TABLE_ENTRY_T;

/**************************   STATE FUNCTIONS **************************/
void show_logo_state_enter(void);
void show_logo_state_handler(void);

void show_product_state_enter(void);
void show_product_state_handler(void);

void show_device_state_enter(void);
void show_device_state_handler(void);

void show_user_info_state_enter(void);
void show_user_info_state_handler(void);

void show_author_info_state_enter(void);
void show_author_info_state_handler(void);

void show_state_info_state_enter(void);
void show_state_info_state_handler(void);

void state_key_menu(void);
void state_key_menu_init(uint8_t state);

#endif /* RLK_KEY_STATE_H */
