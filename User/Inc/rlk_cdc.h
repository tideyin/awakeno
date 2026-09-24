// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef __RLK_CDC_H__
#define __RLK_CDC_H__
#include "gd32e10x.h"

void rlk_usb_cdc_init(void);
int  rlk_check_usb_cdc_status(void);
int  rlk_usb_cdc_receive(uint8_t **data);
void rlk_usb_cdc_send(uint8_t *data, int size);

#endif
