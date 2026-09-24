// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef __AT_CMD_H__
#define __AT_CMD_H__
#include "gd32e10x.h"
#include "nvm.h"

/* AWAKENO FW Version */
#define FIREWARE_VERSION "00.00.06"
#define PERMISSION_DENIED "Permission denied\r\n"

#define COLOR_NULL    "000"
#define COLOR_BLUE    "001"
#define COLOR_PURPLE  "101"
#define COLOR_GREEN   "010"
#define COLOR_WHITE   "111"
#define COLOR_YELLOW  "011"
#define COLOR_ORANGE  "110"
#define COLOR_RED     "100"




typedef int (*cmd_handle_t)(void *, void *, int);


typedef struct {
    char cmd[16];
    char req[16];
    cmd_handle_t handle;
} at_cmd_t;

typedef enum {
    CASE0 = '0',
    CASE1 = '1',
    CASE2 = '2',
    CASE3 = '3',
    CASE4 = '4',
    CASE5 = '5',
    CASE6 = '6',
    CASE7 = '7',
    CASE8 = '8',
    CASE9 = '9'
}case_def_t;






void sys_para_load(void);
void run_cmd_handle(char *cmd);
void show_logo_state(void);
void show_product_state(void);
void show_device_state(void);
void show_author_info_state(void);
void show_user_info_state(void);
void show_adc_info_state(void);
void show_state_info_state(void);
void case_switch(void);




#endif

