// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    drv_uart.c
    \brief   firmware functions to manage COM ports
*/

#include "drv_uart.h"
#include "systick.h"
#include <string.h>
#include "stdbool.h"
#include "drv_timer.h"

#define UART_TX_USING_INTERRUPT 0

/* uart clocks */
static rcu_periph_enum COM_CLK[COMn] = {UART_COM0_CLK, UART_COM1_CLK, UART_COM2_CLK, UART_COM3_CLK,
                                        UART_COM4_CLK};
/* uart tx pins */
static uint32_t COM_TX_PIN[COMn] = {UART_COM0_TX_PIN, UART_COM1_TX_PIN, UART_COM2_TX_PIN,
                                    UART_COM3_TX_PIN, UART_COM4_TX_PIN};
/* uart rx pins */
static uint32_t COM_RX_PIN[COMn] = {UART_COM0_RX_PIN, UART_COM1_RX_PIN, UART_COM2_RX_PIN,
                                    UART_COM3_RX_PIN, UART_COM4_RX_PIN};
/* uart rx ports */
static uint32_t COM_TX_PORT[COMn] = {UART_COM0_TX_PORT, UART_COM1_TX_PORT, UART_COM2_TX_PORT,
                                     UART_COM3_TX_PORT, UART_COM4_TX_PORT};
/* uart tx clock */
static rcu_periph_enum COM_TX_CLK[COMn] = {UART_COM0_TX_CLK, UART_COM1_TX_CLK, UART_COM2_TX_CLK,
                                           UART_COM3_TX_CLK, UART_COM4_TX_CLK};
/* uart tx ports */
static uint32_t COM_RX_PORT[COMn] = {UART_COM0_RX_PORT, UART_COM1_RX_PORT, UART_COM2_RX_PORT,
                                     UART_COM3_RX_PORT, UART_COM4_RX_PORT};
/* uart rx clocks */
static rcu_periph_enum COM_RX_CLK[COMn] = {UART_COM0_RX_CLK, UART_COM1_RX_CLK, UART_COM2_RX_CLK,
                                           UART_COM3_RX_CLK, UART_COM4_RX_CLK};
/* uart irqs */
static uint32_t COM_IRQ[COMn] = {UART_COM0_IRQ, UART_COM1_IRQ, UART_COM2_IRQ, UART_COM3_IRQ,
                                 UART_COM4_IRQ};
/* transmit buffer and receive buffer */
static uint8_t* tx_buffer[COMn];
/* counter of transmit buffer */
static uint16_t tx_count[COMn] = {0};
/* size of transmit buffer */
static uint32_t tx_buffer_size[COMn] = {0};
/* uart callback function */
static uart_callback_fn uart_callback[COMn];

static uint32_t _drv_get_uart_num(uint32_t com)
{
    uint32_t com_id = 0U;

    switch (com) {
        case USART0:
            com_id = 0U;
            break;
        case USART1:
            com_id = 1U;
            break;
        case USART2:
            com_id = 2U;
            break;
        case UART3:
            com_id = 3U;
            break;
        case UART4:
            com_id = 4U;
            break;
        default:
            return -1;
    }

    return com_id;
}

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USART0_IRQHandler(void)
{
    uint32_t com_id = _drv_get_uart_num(USART0);
    uint8_t data = 0;

    /* clear noise/overrun so RBNE cannot storm */
    if (RESET != usart_flag_get(USART0, USART_FLAG_ORERR)) {
        (void)usart_data_receive(USART0);
    }

    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)) {
        data = usart_data_receive(USART0);
        if ((com_id < COMn) && (uart_callback[com_id] != 0)) {
            uart_callback[com_id](com_id, &data, 1);
        }
    }
#if UART_TX_USING_INTERRUPT
    if (RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE)) {
        usart_data_transmit(USART0, *(tx_buffer[com_id] + tx_count[com_id]++));
        if (tx_count[com_id] >= tx_buffer_size[com_id]) {
            usart_interrupt_disable(USART0, USART_INT_TBE);
        }
    }
#endif
}

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UART3_IRQHandler(void)
{
    uint32_t com_id = _drv_get_uart_num(UART3);
    uint8_t data = 0;

    if (RESET != usart_flag_get(UART3, USART_FLAG_ORERR)) {
        (void)usart_data_receive(UART3);
    }

    if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE)) {
        data = usart_data_receive(UART3);
        if ((com_id < COMn) && (uart_callback[com_id] != 0)) {
            uart_callback[com_id](com_id, &data, 1);
        }
    }
#if UART_TX_USING_INTERRUPT
    if (RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_TBE)) {
        /* transmit data */
        usart_data_transmit(UART3, *(tx_buffer[com_id] + tx_count[com_id]++));
        if (tx_count[com_id] >= tx_buffer_size[com_id]) {
            usart_interrupt_disable(UART3, USART_INT_TBE);
        }
    }
#endif
}

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UART4_IRQHandler(void)
{
    uint32_t com_id = _drv_get_uart_num(UART4);
    uint8_t data = 0;

    if (RESET != usart_flag_get(UART4, USART_FLAG_ORERR)) {
        (void)usart_data_receive(UART4);
    }

    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)) {
        data = usart_data_receive(UART4);
        if ((com_id < COMn) && (uart_callback[com_id] != 0)) {
            uart_callback[com_id](com_id, &data, 1);
        }
    }
#if UART_TX_USING_INTERRUPT
    if (RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_TBE)) {
        /* transmit data */
        usart_data_transmit(UART4, *(tx_buffer[com_id] + tx_count[com_id]++));
        if (tx_count[com_id] >= tx_buffer_size[com_id]) {
            usart_interrupt_disable(UART4, USART_INT_TBE);
        }
    }
#endif
}

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
                only one parameter can be selected which is shown as below:
    \arg        BOARD_COM: COM on the board
    \param[in]  baudval: baud rate value
    \param[in]  callback: uart receive callback
    \retval     none
*/
void drv_uart_init(uint32_t com, uint32_t baudval, uart_callback_fn callback)
{
    uint32_t com_id = 0U;

    /* get uart num index */
    com_id = _drv_get_uart_num(com);
    if (com_id >= COMn) {
        return;
    }

    /* uart callback function (NULL = TX-only, no RX IRQ) */
    uart_callback[com_id] = callback;

    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_TX_CLK[com_id]);
    rcu_periph_clock_enable(COM_RX_CLK[com_id]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_id]);

    /* connect port to USARTx_Tx */
    gpio_init(COM_TX_PORT[com_id], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_id]);

    /* connect port to USARTx_Rx — pull-up idle (floating PD2/PC11 storms RBNE after soft-reset) */
    gpio_init(COM_RX_PORT[com_id], GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_id]);

    /* USART configure */
    usart_deinit(com);
    usart_baudrate_set(com, baudval);
    usart_receive_config(com, USART_RECEIVE_ENABLE);
    usart_transmit_config(com, USART_TRANSMIT_ENABLE);
    usart_enable(com);

    if (callback != 0) {
        /* pri 3: below SysTick(0) / USB(1) / SPI2(2) — floating RX must not starve audio/keys */
        nvic_irq_enable(COM_IRQ[com_id], 3, 0);
        usart_interrupt_enable(com, USART_INT_RBNE);
    }
}

/*!
    \brief      uart send
    \param[in]  com: COM on the board
    \param[in]  data: data to send
    \param[in]  len: data length
    \param[in]  block: block or not
    \param[out] none
    \retval     none
*/
void drv_uart_send(uint32_t com, uint8_t* data, uint32_t len, bool block)
{
    uint32_t com_id = _drv_get_uart_num(com);

    if ((com_id >= COMn) || (data == 0) || (len == 0u)) {
        return;
    }

    tx_buffer[com_id] = data;
    tx_buffer_size[com_id] = len;
    tx_count[com_id] = 0;
#if UART_TX_USING_INTERRUPT
    /* enable USART TBE interrupt */
    usart_interrupt_enable(com, USART_INT_TBE);

     if (block) {
        /* wait uart tx complete */
        while (tx_count[com_id] < tx_buffer_size[com_id]) {
        }
    }
#else
    if (block) {
        /* wait uart tx complete */
        while (tx_count[com_id] < tx_buffer_size[com_id]) {
            if (RESET != usart_flag_get(com, USART_FLAG_TBE)) {
                /* transmit data */
                usart_data_transmit(com, *(tx_buffer[com_id] + tx_count[com_id]++));
            }
        }
    }
#endif
}
