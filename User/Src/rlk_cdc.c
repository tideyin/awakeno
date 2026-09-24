// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include <string.h>
#include <stdio.h>
#include "systick.h"
#include "drv_usb_hw.h"
#include "cdc_acm_core.h"
#include "rlk_cdc.h"
#include "uart_adapter.h"

#define USB_ACM_BUFFER_SIZE 65
static uint8_t usb_data_buffer[USB_ACM_BUFFER_SIZE];
static uint32_t usb_data_length = 0;

usb_core_driver cdc_acm; //Note: this is a global variable that will be used in other files

void rlk_usb_cdc_init(void)
{
    usb_rcu_config();

    usb_timer_init();

    usbd_init(&cdc_acm, USB_CORE_ENUM_FS, &cdc_desc, &cdc_class);

    usb_intr_config();
}

int rlk_check_usb_cdc_status(void)
{
	if(USBD_CONFIGURED == cdc_acm.dev.cur_status)
		return 1;
	else
		return 0;
}

#if 1 // Mingxing code
int rlk_usb_cdc_receive(uint8_t **data)
{
	int rlen = -1;
	usb_cdc_handler *cdc = (usb_cdc_handler *)cdc_acm.dev.class_data[CDC_COM_INTERFACE];

	if(0U == cdc_acm_check_ready(&cdc_acm)){
		cdc_acm_data_receive(&cdc_acm);

	if(cdc->receive_length > 0){
		if(cdc->receive_length <= USB_CDC_DATA_PACKET_SIZE){
			if(usb_data_length + cdc->receive_length > USB_CDC_DATA_PACKET_SIZE)
				usb_data_length = 0;
			memcpy(usb_data_buffer + usb_data_length, cdc->data, cdc->receive_length);
			usb_data_length += cdc->receive_length;
			if(cdc->data[cdc->receive_length - 1] == '\r' || cdc->data[cdc->receive_length - 2] == '\r'){
				usb_data_buffer[usb_data_length] = '\0';
				*data = usb_data_buffer;
				rlen = usb_data_length;
				usb_data_length = 0;
			}else{
				rlen = -1;
			}
		}else{
			rlen = -1;
		}
	}
}
	return rlen;
}
#else

int rlk_usb_cdc_receive(uint8_t **data)
{

	int rlen = -1;
	usb_cdc_handler *cdc = (usb_cdc_handler *)cdc_acm.dev.class_data[CDC_COM_INTERFACE];

	if(0U == cdc_acm_check_ready(&cdc_acm)){
		cdc_acm_data_receive(&cdc_acm);
	
	if(cdc->receive_length > 0){
		if(cdc->receive_length <= USB_CDC_DATA_PACKET_SIZE){
			memcpy(usb_data_buffer + usb_data_length, cdc->data, cdc->receive_length);
			usb_data_buffer[usb_data_length] = '\0';
			*data = usb_data_buffer;
			rlen = cdc->receive_length;
		}else{
			rlen = -1;
		}
	}
}
	return rlen;

}
#endif



void rlk_usb_cdc_send(uint8_t *data, int size)
{
	uint32_t count = 0;

	/* mirror AT reply to DBG USART0 */
	uart_dbg_send(data, (uint32_t)size);

	if (rlk_check_usb_cdc_status()) {
		cdc_acm_userdata_send(&cdc_acm, data, size);

		while (0U != cdc_acm_check_ready(&cdc_acm))
		{
			if(count++ < 1)
			{
				delay_while_ms(1);
			}else{
				count = 0;
				break;
			}
		}
	}
}
