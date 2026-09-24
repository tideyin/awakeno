// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

/*!
    \file    uart_adapter.c
    \brief   firmware functions to manage uart
*/

#include "uart_adapter.h"
#include "drv_timer.h"
#include "ringbuffer.h"
#include <string.h>
#include "systick.h"
#include "printf.h"

#define RINGBUF_MAX_SIZE RING_BUFFER_SIZE
#define UART_RCV_TIMEOUT 5
#define UART_RCV_MAX_SIZE 1024
#define UART_ADAPTER_USING_CACHE 1
#define DBG_LINE_MAX_SIZE 128

static uint32_t _jack_uart_rcv_count = 0;
static uint32_t _hdmi_uart_rcv_count = 0;
static ring_buffer_t _jack_uart_ringbuf_handle = {0};
static ring_buffer_t _hdmi_uart_ringbuf_handle = {0};
static ring_buffer_t _dbg_uart_ringbuf_handle = {0};
static uint8_t _dbg_line_buf[DBG_LINE_MAX_SIZE];
static uint32_t _dbg_line_len = 0;
#if UART_ADAPTER_USING_CACHE
static uint8_t _jack_uart_send_buf[RINGBUF_MAX_SIZE];
static uint8_t _hdmi_uart_send_buf[RINGBUF_MAX_SIZE];
#endif

static void _dbg_uart_callback_fn(uint32_t com, uint8_t* data, uint32_t len)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_dbg_uart_ringbuf_handle;

    (void)com;
    ring_buffer_queue_arr(rBuf, (const char*)data, len);
}

static void _jack_uart_callback_fn(uint32_t com, uint8_t* data, uint32_t len)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_jack_uart_ringbuf_handle;
    uint8_t jack_uart_rcv_data;

    jack_uart_rcv_data = *data;
    ring_buffer_queue_arr(rBuf, (const char*)&jack_uart_rcv_data, len);
}

static void _hdmi_uart_callback_fn(uint32_t com, uint8_t* data, uint32_t len)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_hdmi_uart_ringbuf_handle;
    uint8_t hdmi_uart_rcv_data;

    hdmi_uart_rcv_data = *data;
    ring_buffer_queue_arr(rBuf, (const char*)&hdmi_uart_rcv_data, len);
}

/*!
    \brief      uart adapter data process
    \param[in]  none
    \param[out] none
    \retval     none
*/
void uart_adapter_data_process(void)
{
    ring_buffer_t* rBuf;
    uint32_t size = 0;
#if !UART_ADAPTER_USING_CACHE
    int32_t lenToEnd = 0;
    char* pBase;
    char* pRp;
#endif

    if (_jack_uart_rcv_count < UART_RCV_MAX_SIZE) {
        rBuf = (ring_buffer_t*)&_jack_uart_ringbuf_handle;
        size =
            (uint32_t)ring_buffer_dequeue_arr(rBuf, (char*)_jack_uart_send_buf, RING_BUFFER_SIZE);
        if (size) {
#if !UART_ADAPTER_USING_CACHE
            lenToEnd = RINGBUF_GET_PARAM(rBuf, limit) - RINGBUF_GET_PARAM(rBuf, rp);
            pBase = RINGBUF_GET_PARAM(rBuf, base);
            pRp = RINGBUF_GET_PARAM(rBuf, rp);

            if (lenToEnd >= size) {
                drv_uart_send(HDMI_UART, pRp, size, true);
            } else {
                drv_uart_send(HDMI_UART, pRp, lenToEnd, true);
                drv_uart_send(HDMI_UART, pBase, size - lenToEnd, true);
            }
            Ringbuf_flush(rBuf, size);

            _jack_uart_rcv_count += size;
            if (_jack_uart_rcv_count >= UART_RCV_MAX_SIZE) {
                _jack_uart_rcv_count = 0;
                delay_while_ms(UART_RCV_TIMEOUT);
            }
#else
            drv_uart_send(HDMI_UART, _jack_uart_send_buf, size, true);
            _jack_uart_rcv_count += size;
            if (_jack_uart_rcv_count >= UART_RCV_MAX_SIZE) {
                _jack_uart_rcv_count = 0;
                delay_while_ms(UART_RCV_TIMEOUT);
            }

#endif
        }
    }

    if (_hdmi_uart_rcv_count < UART_RCV_MAX_SIZE) {
        rBuf = (ring_buffer_t*)&_hdmi_uart_ringbuf_handle;
        size =
            (uint32_t)ring_buffer_dequeue_arr(rBuf, (char*)_hdmi_uart_send_buf, RING_BUFFER_SIZE);
        if (size) {
#if !UART_ADAPTER_USING_CACHE
            lenToEnd = RINGBUF_GET_PARAM(rBuf, limit) - RINGBUF_GET_PARAM(rBuf, rp);
            pBase = RINGBUF_GET_PARAM(rBuf, base);
            pRp = RINGBUF_GET_PARAM(rBuf, rp);

            // printf("pRp[%d],pBase[%d],size[%d],lenToEnd[%d]", pRp, pBase, size, lenToEnd);

            if (lenToEnd >= size) {
                drv_uart_send(JACK_UART, pRp, size, true);
            } else {
                drv_uart_send(JACK_UART, pRp, lenToEnd, true);
                drv_uart_send(JACK_UART, pBase, size - lenToEnd, true);
            }
            Ringbuf_flush(rBuf, size);

            _hdmi_uart_rcv_count += size;
            if (_hdmi_uart_rcv_count >= UART_RCV_MAX_SIZE) {
                _hdmi_uart_rcv_count = 0;
                delay_while_ms(UART_RCV_TIMEOUT);
            }
#else
            drv_uart_send(JACK_UART, _hdmi_uart_send_buf, size, true);
            _hdmi_uart_rcv_count += size;
            if (_hdmi_uart_rcv_count >= UART_RCV_MAX_SIZE) {
                _hdmi_uart_rcv_count = 0;
                delay_while_ms(UART_RCV_TIMEOUT);
            }
#endif
        }
    }
}


/*!
    \brief      configure uart adapter
    \param[in]  none
    \param[out] none
    \retval     none
*/
void uart_adapter_init(void)
{
    ring_buffer_t* rBuf;

    /* DBG USART0: printf console + RX (AT) — 921600 for WRDAT binary download */
    drv_uart_init(DBG_UART, 921600U, _dbg_uart_callback_fn);

    drv_uart_init(JACK_UART, 115200U, _jack_uart_callback_fn);
    drv_uart_init(HDMI_UART, 115200U, _hdmi_uart_callback_fn);

    rBuf = (ring_buffer_t*)&_dbg_uart_ringbuf_handle;
    ring_buffer_init(rBuf);

    rBuf = (ring_buffer_t*)&_jack_uart_ringbuf_handle;
    ring_buffer_init(rBuf);

    rBuf = (ring_buffer_t*)&_hdmi_uart_ringbuf_handle;
    ring_buffer_init(rBuf);

    _dbg_line_len = 0;
}

/*!
 * \brief  assemble DBG RX into a CR/LF-terminated line
 * \return line length (>0) when ready; -1 if incomplete
 */
int uart_dbg_receive(uint8_t **data)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_dbg_uart_ringbuf_handle;
    char ch;

    while (ring_buffer_dequeue(rBuf, &ch)) {
        if ((ch == '\r') || (ch == '\n')) {
            if (_dbg_line_len == 0u) {
                continue;
            }
            _dbg_line_buf[_dbg_line_len] = '\0';
            *data = _dbg_line_buf;
            {
                int rlen = (int)_dbg_line_len;
                _dbg_line_len = 0;
                return rlen;
            }
        }

        if (_dbg_line_len < (DBG_LINE_MAX_SIZE - 1u)) {
            _dbg_line_buf[_dbg_line_len++] = (uint8_t)ch;
        } else {
            _dbg_line_len = 0;
        }
    }

    return -1;
}

void uart_dbg_send(uint8_t *data, uint32_t len)
{
    if ((data == 0) || (len == 0u)) {
        return;
    }
    drv_uart_send(DBG_UART, data, len, true);
}

void uart_dbg_rx_flush(void)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_dbg_uart_ringbuf_handle;
    char ch;

    _dbg_line_len = 0;
    while (ring_buffer_dequeue(rBuf, &ch)) {
    }
}

int uart_dbg_rx_byte(uint8_t *byte)
{
    ring_buffer_t* rBuf = (ring_buffer_t*)&_dbg_uart_ringbuf_handle;
    char ch;

    if (byte == 0) {
        return 0;
    }
    if (!ring_buffer_dequeue(rBuf, &ch)) {
        return 0;
    }
    *byte = (uint8_t)ch;
    return 1;
}
