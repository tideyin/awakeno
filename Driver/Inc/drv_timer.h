// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    drv_timer.h
    \brief   definitions for timer ports hardware resources
*/

#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32e10x.h"
#include <stdbool.h>

/* typedef */
typedef uint32_t drv_timer_handle;
typedef void (*drv_timer_callback)(drv_timer_handle* handle);

typedef struct {
    drv_timer_handle* handle;
    drv_timer_callback callback;
    uint32_t period;
    uint32_t count;
    uint8_t repeat;
} drv_timer_t;

/* definition */
#define DRV_TIMER_NUM 32

/* function declarations */
void drv_timer_init(void);
int drv_timer_register(drv_timer_handle* handle, drv_timer_callback callback, uint32_t period_ms,
                       uint8_t repeat);
int drv_timer_unregister(drv_timer_handle* handle);
bool drv_timer_is_running(drv_timer_handle* handle);
void drv_timer_process(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_TIMER_H */
