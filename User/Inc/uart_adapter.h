// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    uart_adapter.h
    \brief   definitions for uart_adapter
*/

#ifndef UART_ADAPTER_H
#define UART_ADAPTER_H

#include "drv_uart.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* definition */
#define DBG_UART  UART_COM0   /* USART0 PA9/PA10 — printf + AT console */
#define JACK_UART UART_COM3
#define HDMI_UART UART_COM4

/* function declarations */
void uart_adapter_init(void);
void uart_adapter_data_process(void);

/* DBG USART0: return line length (>0) when a CR/LF-terminated line is ready */
int uart_dbg_receive(uint8_t **data);
void uart_dbg_send(uint8_t *data, uint32_t len);

/* DBG USART0 raw RX (binary download) */
void uart_dbg_rx_flush(void);
/* return 1 if a byte was taken, 0 if empty */
int uart_dbg_rx_byte(uint8_t *byte);

#ifdef __cplusplus
}
#endif

#endif /* UART_ADAPTER_H */
