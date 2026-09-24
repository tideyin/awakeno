// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    drv_timer.c
    \brief   firmware functions to manage timer (TIMER4; TIMER3 used by IR NEC)
*/

#include "drv_timer.h"
#include <string.h>

drv_timer_t drv_timers[DRV_TIMER_NUM];

/*!
    \brief      timer isr
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void drv_timer_isr(void)
{
    for (int i = 0; i < DRV_TIMER_NUM; i++) {
        if (drv_timers[i].handle != NULL)
            drv_timers[i].count++;
    }
}

/*!
    \brief      this function handles Timer updata interrupt request.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void TIMER4_IRQHandler(void)
{
    if (RESET != timer_interrupt_flag_get(TIMER4, TIMER_INT_UP)) {
        timer_interrupt_flag_clear(TIMER4, TIMER_INT_UP);
        drv_timer_isr();
    }
}

/*!
    \brief      configure timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void drv_timer_init(void)
{
    timer_parameter_struct timer_basestructure;

    memset((uint8_t*)drv_timers, 0, sizeof(drv_timers));

    /* enable the TIM2 gloabal interrupt */
    nvic_irq_enable((uint8_t)TIMER4_IRQn, 1U, 0U);

    rcu_periph_clock_enable(RCU_TIMER4);

    timer_disable(TIMER4);

    timer_interrupt_disable(TIMER4, TIMER_INT_UP);

    timer_basestructure.period = 11999U;

    timer_basestructure.prescaler = 5U;
    timer_basestructure.alignedmode = TIMER_COUNTER_EDGE;
    timer_basestructure.counterdirection = TIMER_COUNTER_UP;
    timer_basestructure.clockdivision = TIMER_CKDIV_DIV1;
    timer_basestructure.repetitioncounter = 0U;

    timer_init(TIMER4, &timer_basestructure);

    timer_interrupt_flag_clear(TIMER4, TIMER_INT_UP);

    timer_auto_reload_shadow_enable(TIMER4);

    /* TIMER4 interrupt enable */
    timer_interrupt_enable(TIMER4, TIMER_INT_UP);

    /* TIMER4 enable counter */
    timer_enable(TIMER4);
}

/*!
    \brief      register timer
    \param[in]  handle
    \param[in]  callback
    \param[in]  period_ms
    \param[in]  repeat
    \param[out] none
    \retval     0:success other:fail
*/
int drv_timer_register(drv_timer_handle* handle, drv_timer_callback callback, uint32_t period_ms,
                       uint8_t repeat)
{
    uint32_t ret = 0, i;

    for (i = 0; i < DRV_TIMER_NUM; i++) {
        if (drv_timers[i].handle == handle || drv_timers[i].handle == NULL) {
            *handle = i;
            drv_timers[i].handle = handle;
            drv_timers[i].callback = callback;
            drv_timers[i].period = period_ms;
            drv_timers[i].count = 0;
            drv_timers[i].repeat = repeat;
            break;
        }
    }

    if (i == DRV_TIMER_NUM)
        ret = -1;

    return ret;
}

/*!
    \brief      unregister timer
    \param[in]  handle
    \param[out] none
    \retval     0:success other:fail
*/
int drv_timer_unregister(drv_timer_handle* handle)
{
    uint32_t ret = 0;

    if (*handle < DRV_TIMER_NUM)
        drv_timers[*handle].handle = NULL;
    else
        ret = -1;

    return ret;
}

/*!
    \brief      check timer if is running
    \param[in]  handle
    \param[out] none
    \retval     0:success other:fail
*/
bool drv_timer_is_running(drv_timer_handle* handle)
{
    bool ret = false;

    if (*handle < DRV_TIMER_NUM) {
        if (drv_timers[*handle].handle != NULL
            && drv_timers[*handle].count < drv_timers[*handle].period) {
            ret = true;
        }
    }

    return ret;
}

/*!
    \brief      timer process
    \param[in]  none
    \param[out] none
    \retval     none
*/
void drv_timer_process(void)
{
    for (int i = 0; i < DRV_TIMER_NUM; i++) {
        if (drv_timers[i].handle != NULL && drv_timers[i].count >= drv_timers[i].period) {
            drv_timers[i].callback(drv_timers[i].handle);
            if (!drv_timers[i].repeat)
                drv_timer_unregister(drv_timers[i].handle);
        }
    }
}
