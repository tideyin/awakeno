// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    drv_uart.h
    \brief   definitions for COM ports hardware resources
*/

#ifndef DRV_UART_H
#define DRV_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32e10x.h"
#include <stdbool.h>

/* definition for COM ports */
#define COMn 5U
#define BUFFER_SIZE 32

/* USART0 = DBG_UART (PA9 TX / PA10 RX) */
#define UART_COM0 USART0
#define UART_COM0_CLK RCU_USART0
#define UART_COM0_TX_PIN GPIO_PIN_9
#define UART_COM0_RX_PIN GPIO_PIN_10
#define UART_COM0_TX_PORT GPIOA
#define UART_COM0_TX_CLK RCU_GPIOA
#define UART_COM0_RX_PORT GPIOA
#define UART_COM0_RX_CLK RCU_GPIOA
#define UART_COM0_IRQ USART0_IRQn

#define UART_COM1 USART1
#define UART_COM1_CLK RCU_UART3
#define UART_COM1_TX_PIN GPIO_PIN_10
#define UART_COM1_RX_PIN GPIO_PIN_11
#define UART_COM1_TX_PORT GPIOC
#define UART_COM1_TX_CLK RCU_GPIOC
#define UART_COM1_RX_PORT GPIOC
#define UART_COM1_RX_CLK RCU_GPIOC
#define UART_COM1_IRQ USART1_IRQn

#define UART_COM2 USART2
#define UART_COM2_CLK RCU_UART3
#define UART_COM2_TX_PIN GPIO_PIN_10
#define UART_COM2_RX_PIN GPIO_PIN_11
#define UART_COM2_TX_PORT GPIOC
#define UART_COM2_TX_CLK RCU_GPIOC
#define UART_COM2_RX_PORT GPIOC
#define UART_COM2_RX_CLK RCU_GPIOC
#define UART_COM2_IRQ USART2_IRQn

#define UART_COM3 UART3
#define UART_COM3_CLK RCU_UART3
#define UART_COM3_TX_PIN GPIO_PIN_10
#define UART_COM3_RX_PIN GPIO_PIN_11
#define UART_COM3_TX_PORT GPIOC
#define UART_COM3_TX_CLK RCU_GPIOC
#define UART_COM3_RX_PORT GPIOC
#define UART_COM3_RX_CLK RCU_GPIOC
#define UART_COM3_IRQ UART3_IRQn

#define UART_COM4 UART4
#define UART_COM4_CLK RCU_UART4
#define UART_COM4_TX_PIN GPIO_PIN_12
#define UART_COM4_RX_PIN GPIO_PIN_2
#define UART_COM4_TX_PORT GPIOC
#define UART_COM4_TX_CLK RCU_GPIOC
#define UART_COM4_RX_PORT GPIOD
#define UART_COM4_RX_CLK RCU_GPIOD
#define UART_COM4_IRQ UART4_IRQn

typedef void (*uart_callback_fn)(uint32_t com, uint8_t* data, uint32_t len);

/* function declarations */
void drv_uart_init(uint32_t com, uint32_t baudval, uart_callback_fn callback);
void drv_uart_send(uint32_t com, uint8_t* data, uint32_t len, bool block);

#ifdef __cplusplus
}
#endif

#endif /* DRV_UART_H */
