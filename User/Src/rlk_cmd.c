// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rlk_cdc.h"
#include "rlk_gpio.h"
#include "rlk_cmd.h"
#include "spi_flash.h"
#include "i2c_lcd.h"
#include "i2s_codec.h"
#include "audio_dac.h"
#include "audio_pwm.h"
#include "wave_data.h"
#include "bh1750.h"
#include "ads1115.h"
#include "adc_gd32.h"
#include "fontlib.h"
#include "menu_key.h"
#include "rlk_key_state.h"
#include "function.h"
#include "gt22l16a2y.h"
#include "ir_snd_rcv.h"
#include "uart_adapter.h"

#include "printf.h"
#include "cmb_cfg.h"

/* AT+AUDTST=F: DBG UART binary → SPI flash @0; EOF=FA05A55A → OK; >75KB → ERROR */
#define AUDTST_WRDAT_MAX_BYTES   (75u * 1024u)
#define AUDTST_WRDAT_PAGE_SIZE   256u




#define BUFFER_SIZE                   512 //256
#define EEP_FIRST_PAGE           0x00


#define I2C_OK                 0
#define I2C_FAIL               1



#define ACK_REBOOT "SYSRST:REBOOT\r\n"
#define ACK_DFU "SYSRST:DFU\r\n"
#define ACK_OK     "OK\r\n"
#define ACK_PRD "PING:AWAKENO\r\n"
#define ACK_ERR "ERROR\r\n"
#define ACK_RJT "REJECT\r\n"


extern uint8_t hw_ver ;




extern unsigned char RxBuf[] ;
extern char glb_bl_val ;

//For music
extern unsigned int const FREQUENCY[] ;
extern unsigned int const DELAY[];
extern uint8_t music_flag ;
extern int m_index ;


static int sw_ping_handler(void* cline, void* reqline, int offset);
static int sw_help_handler(void* cline, void* reqline, int offset);
static int ping_device_handler(void* cline, void* reqline, int offset);
static int system_reset_handler(void* cline, void* reqline, int offset);


static int set_vflg_handler(void* cline, void* reqline, int offset);
static int del_vflg_handler(void* cline, void* reqline, int offset);





static int set_rgb_led_handler(void* cline, void* reqline, int offset);
static int set_rb_en_handler(void* cline, void* reqline, int offset);





static int eeprom_test_handler(void* cline, void* reqline, int offset);
static int flash_test_handler(void* cline, void* reqline, int offset);
static int lcd_test_handler(void* cline, void* reqline, int offset);








static int audio_test_handler(void* cline, void* reqline, int offset);
static int get_lux_handler(void* cline, void* reqline, int offset);
static int get_adc_handler(void* cline, void* reqline, int offset);
static int send_hard_nec_handler(void* cline, void* reqline, int offset);
static int ir_mon_act_handler(void* cline, void* reqline, int offset);
static int nvm_ver_set_handler(void* cline, void* reqline, int offset);
static int nvm_ver_get_handler(void* cline, void* reqline, int offset);
static int dsn_set_handler(void* cline, void* reqline, int offset);
static int dsn_get_handler(void* cline, void* reqline, int offset);
static int version_get_handler(void* cline, void* reqline, int offset);
static int hw_ver_set_handler(void* cline, void* reqline, int offset);
static int hw_ver_get_handler(void* cline, void* reqline, int offset);


static int product_set_handler(void* cline, void* reqline, int offset);
static int product_get_handler(void* cline, void* reqline, int offset);
static int owner_set_handler(void* cline, void* reqline, int offset);
static int owner_get_handler(void* cline, void* reqline, int offset);
static int login_set_handler(void* cline, void* reqline, int offset);
static int login_get_handler(void* cline, void* reqline, int offset);
static int locat_set_handler(void* cline, void* reqline, int offset);
static int locat_get_handler(void* cline, void* reqline, int offset);

static int state_set_handler(void* cline, void* reqline, int offset);
static int state_get_handler(void* cline, void* reqline, int offset);




















void run_cmd_handler(char* cmd) ;


static uint8_t unbreak_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x62, 0x6F, 0x6F, 0x74, 0x40, 0x55, 0x53, 0x42};

extern sys_parameter_t sys_para ;
extern uint8_t vflg_data[];
extern uint8_t valid_vflg[] , inval_vflg[];																 



static at_cmd_t at_cmds[] = {
    {"AT?", "", sw_ping_handler},
    {"HELP?", "", sw_help_handler},
    {"AT+PING?", "", ping_device_handler},
    {"AT+SYSRST=", "", system_reset_handler},
    {"AT+NVMVER=", "", nvm_ver_set_handler},
    {"AT+NVMVER?", "", nvm_ver_get_handler},
    {"AT+SETVFLG=", "", set_vflg_handler},
    {"AT+DELVFLG=", "", del_vflg_handler},
    {"AT+DSN=", "", dsn_set_handler},
    {"AT+DSN?", "", dsn_get_handler},
    {"AT+VER?", "", version_get_handler},
    {"AT+HVER=", "", hw_ver_set_handler},
    {"AT+HVER?", "", hw_ver_get_handler},
    {"AT+RGB=", "",  set_rgb_led_handler},
    {"AT+RBEN=", "",  set_rb_en_handler},
    {"AT+PRODUCT=", "", product_set_handler},
    {"AT+PRODUCT?", "", product_get_handler},
    {"AT+OWNER=", "", owner_set_handler},
    {"AT+OWNER?", "", owner_get_handler},
    {"AT+LOGIN=", "", login_set_handler},
    {"AT+LOGIN?", "", login_get_handler},
    {"AT+LOCAT=", "", locat_set_handler},
    {"AT+LOCAT?", "", locat_get_handler},
    {"AT+AUDTST=", "", audio_test_handler},
    {"AT+GETLUX?", "", get_lux_handler},
    {"AT+GETADC?", "", get_adc_handler},
    {"AT+HD_NEC=", "", send_hard_nec_handler},
    {"AT+IRMON=", "", ir_mon_act_handler},
} ;



//DEBUG PORT ON/OFF
const char  DBGON[]="ON\r\n" ;
const char  DBGOFF[]="OFF\r\n" ;


//System reset
const char  RSTREBOOT[]="REBOOT\r\n" ;
const char  RSTDFU[]="DFU\r\n" ;

/* IR NEC keys (AT+HD_NEC=) */
const char  NECPWR[]=  "NECPWR\r\n" ;
const char  NECUP[]=   "NECUP\r\n" ;
const char  NECDWN[]=  "NECDWN\r\n" ;
const char  NECLFT[]=  "NECLFT\r\n" ;
const char  NECRIT[]=  "NECRIT\r\n" ;
const char  NECVUP[]=  "NECVUP\r\n" ;
const char  NECVDN[]=  "NECVDN\r\n" ;
const char  NECHOM[]=  "NECHOM\r\n" ;
const char  NECBAK[]=  "NECBAK\r\n" ;
const char  NECMUT[]=  "NECMUT\r\n" ;
const char  NECMNU[]=  "NECMNU\r\n" ;
const char  NECPUP[]=  "NECPUP\r\n" ;
const char  NECPDN[]=  "NECPDN\r\n" ;
const char  NECFWD[]=  "NECFWD\r\n" ;
const char  NECRWD[]=  "NECRWD\r\n" ;
const char  NECVOC[]=  "NECVOC\r\n" ;
const char  NECPLY[]=  "NECPLY\r\n" ;
const char  NECSEL[]=  "NECSEL\r\n" ;
const char  IRMONSTART[]="START\r\n" ;
const char  IRMONSTOP[]="STOP\r\n" ;


const char  OPRDSN[]="DSN\r\n" ;
const char  OPRPRD[]="PRD\r\n" ;
const char  OPRNVMVER[]="NVMVER\r\n" ;
const char  OPRUSR[]="USR\r\n" ;
const char  OPRALL[]="ALL\r\n" ;





void run_cmd_handler(char* cmd)
{
    int i = 0;
    int cmd_num = sizeof(at_cmds) / sizeof(at_cmd_t);
    int cmd_len = strlen((const char*)cmd);
    int matched = -1;

	printf_("\r\nReceived AT CMD: %s",cmd);
	if (cmd_len > 0 && cmd[cmd_len-1] != 0xa ) printf_("\r\nThis AT CMD has no CRLF.\r\n") ;
	printf_("Received AT Hex: ");

#if 1
	for(i=0 ; i< cmd_len ; i++ )
	{
	   printf_("0x%x ",cmd[i]) ;
	}
#endif

    if (cmd_len < 1) {
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return;
    }

    /*
     * USB CDC may deliver "AT?\r", "AT?\r\n", or split packets reassembled
     * until '\r'. Match on keyword only; ignore trailing CR/LF.
     */
    for (i = 0; i < cmd_num; i++) {
        size_t plen = strlen(at_cmds[i].cmd);
        char next;

        if (plen == 0u || (size_t)cmd_len < plen) {
            continue;
        }
        if (strncmp(at_cmds[i].cmd, cmd, plen) != 0) {
            continue;
        }

        next = cmd[plen];
        /* patterns ending with '=' take the rest as arguments */
        if (at_cmds[i].cmd[plen - 1u] == '=') {
            matched = i;
            break;
        }
        /* query / exact: allow end or leftover CR/LF only */
        if (next == '\0' || next == '\r' || next == '\n') {
            matched = i;
            break;
        }
    }

    if (matched >= 0) {
        at_cmds[matched].handle((void*)cmd, (void*)at_cmds[matched].req,
                                (int)strlen(at_cmds[matched].cmd));
    } else {
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
    }
}

void show_logo_state(void)
{
	/* Hebrew: ???? ????  / Arabic: ????? ??????? */
	static const unsigned short heb_hello[] = {
		0x05E9, 0x05DC, 0x05D5, 0x05DD, 0x0020,
		0x05E2, 0x05D5, 0x05DC, 0x05DD, 0x0000
	};
	static const unsigned short arb_hello[] = {
		0x0645, 0x0631, 0x062D, 0x0628, 0x0627, 0x0020,
		0x0628, 0x0627, 0x0644, 0x0639, 0x0627, 0x0644, 0x0645, 0x0000
	};

	music_flag = 0;
	OLED_CLS();

	gt22l16a2y_font_init();
	/* line1 ASCII */
	OLED_ShowStr_GT22(0, 0, "Hello World!", 2);
	/* line2 GBK "????!" ?"!" via ASCII (ShowCN skips <0x80) */
	OLED_ShowCN_GT22(0, 2, "\xCA\xC0\xBD\xE7\xC4\xE3\xBA\xC3");
	OLED_ShowStr_GT22(64, 2, "!", 2);
	/* line3 Hebrew + '!' after last glyph; line4 Arabic + '!' before first glyph */
	{
		unsigned char heb_right = OLED_ShowHeb_GT22(0, 4, heb_hello);
		if (heb_right <= 120u) {
			OLED_ShowStr_GT22(heb_right, 4, "!", 2);
		}
	}
	{
		unsigned char arb_left = OLED_ShowArb_GT22(0, 6, arb_hello);
		if (arb_left >= 8u) {
			OLED_ShowStr_GT22((unsigned char)(arb_left - 8u), 6, "!", 2);
		} else {
			OLED_ShowStr_GT22(0, 6, "!", 2);
		}
	}

	printf_("\r\nLogo drawn");
}

void show_product_state(void)
{
    OLED_CLS();

    uint8_t buf[64] = {0};
    uint8_t read_data[PRODUCT_LENGTH + 1];
    unsigned char default_data[PRODUCT_LENGTH + 1] = "<AWAKENO>";

    eeprom_buffer_read_wrapper(read_data, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "PRD:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, PRODUCT_OFFSET, PRODUCT_LENGTH);
		
		read_data[PRODUCT_LENGTH] = '\0';
		
		sprintf(buf, "PRD:%s", read_data);
    }	

    OLED_ShowStr(0, 0, buf, 2);

    uint8_t read_data1[DSN_LENGTH+1];
    uint8_t default_data1[DSN_LENGTH + 1] = "AWKN-0-0000-0000";

    eeprom_buffer_read_wrapper(read_data1, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);

	if (read_data1[0] != VALID_FLAG_L || read_data1[1] != VALID_FLAG_H) {
        sprintf(buf, "%s", default_data1);
    } else {
        eeprom_buffer_read_wrapper(read_data1, DSN_OFFSET, DSN_LENGTH);
        read_data1[DSN_LENGTH] = '\0';
        sprintf(buf, "%s", read_data1);
    }


    OLED_ShowStr(0, 2, buf, 2);




}

void show_device_state(void)
{
    OLED_CLS();

    uint8_t buf[64] = {0};
    uint8_t buf1[58] = {0};


    sprintf(buf, "FVER:%s", FIREWARE_VERSION);

    OLED_ShowStr(0, 0, buf, 2);

	get_hardware_ver(buf1) ;
    sprintf(buf, "HVER:%s", buf1);
    OLED_ShowStr(0, 2, buf, 2);


	
}

void show_user_info_state(void)
{
    OLED_CLS();

    uint8_t buf[64] = {0};
    uint8_t read_data[OWNER_LENGTH + 1];
    unsigned char default_data[OWNER_LENGTH + 1] = "USER";


    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "OWNER:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, OWNER_OFFSET, OWNER_LENGTH);
		
		read_data[OWNER_LENGTH] = '\0';
		
		sprintf(buf, "OWNER:%s", read_data);
    }	
    OLED_ShowStr(0, 0, buf, 2);


	sprintf(default_data, "%s", "user@");
    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "LOGIN:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, LOGIN_OFFSET, LOGIN_LENGTH);
		
		read_data[LOGIN_LENGTH] = '\0';
		
		sprintf(buf, "LOGIN:%s", read_data);
    }

    OLED_ShowStr(0, 2, buf, 1);

	sprintf(default_data, "%s", "Beijing");
    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "LOCAT:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, LOCAT_OFFSET, LOCAT_LENGTH);
		
		read_data[LOCAT_LENGTH] = '\0';
		
		sprintf(buf, "LOCAT:%s", read_data);
    }

    OLED_ShowStr(0, 3, buf, 1);
}


void show_author_info_state(void)
{
		 

		 
		 OLED_CLS() ;
		 
		 int i;
		 OLED_ShowStr(0,0,"Powered By:",2) ;
	
			  OLED_ShowStr(0,2,"T & T  (",2) ;
			  				
			  for(i=3;i<6 ;i++)
				 OLED_ShowCN(68+(i-3)*16,2 ,i) ;
		 
			  OLED_ShowStr(120,2,")",2) ;
		 
		 

			  

}


void show_state_info_state(void)
{
    OLED_CLS();

	music_flag = 1;
	m_index = 0 ; // Start from begining of music


	OLED_ShowStr(0,0,"tide.yin@ ",2) ;
	  
	OLED_ShowStr(0,2,"  rayzerlink.com",2) ;

}



#include "printf.h"
#include "cmb_cfg.h"
void fault_test_by_div0(void)
{
    volatile int* SCB_CCR = (volatile int*)0xE000ED14; // SCB->CCR
    int x, y, z;
    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */
    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
}

static int sw_ping_handler(void* cline, void* reqline, int offset)
{
    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));


    return 0;
}

static int ping_device_handler(void* cline, void* reqline, int offset)
{
	

    rlk_usb_cdc_send((uint8_t*)ACK_PRD, strlen(ACK_PRD));

    return 0;
}



static int system_reset_handler(void* cline, void* reqline, int offset)
{
//	static unsigned char display_type = VB1_4K_1DIV ;
	char* p = (char*)cline + offset;
	char* save_to_ee = (char*)reqline;

	beep() ;
	if (strlen(p) < 1) {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}
	
	
	if (!strcmp(p ,RSTREBOOT))	// If equ , N logic
	{
		rlk_usb_cdc_send((uint8_t*)ACK_REBOOT, strlen(ACK_REBOOT));
		delay_1ms(200);
		NVIC_SystemReset() ;
	}
	else if (!strcmp(p , RSTDFU) )
	{
        gpio_init(KEY_MENU_PORT, GPIO_MODE_OUT_PP , GPIO_OSPEED_50MHZ, KEY_MENU_PIN);
        rlk_gpio_set_value(KEY_MENU_PORT, KEY_MENU_PIN, 0);	
        rlk_usb_cdc_send((uint8_t*)ACK_DFU, strlen(ACK_DFU));
		delay_1ms(200);
		NVIC_SystemReset() ;
	}	
	else
	{
			rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
			return -1;
	}
	
//    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	return 0 ;

}






int set_rgb_pins(char r, char g, char b)
{

    /* LED_R */
    rlk_gpio_set_value(LED_R_PORT, LED_R_PIN, r);

    /* LED_G */
    rlk_gpio_set_value(LED_G_PORT, LED_G_PIN, g);

    /* LED_B */
    rlk_gpio_set_value(LED_B_PORT, LED_B_PIN, b);

 

    return 0;
}


int set_rb_pins(char r, char b)
{

    /* LED_R */
    rlk_gpio_set_value(RED_LED_EN_PORT, RED_LED_EN_PIN, r);

    /* LED_B */
    rlk_gpio_set_value(BLUE_LED_EN_PORT, BLUE_LED_EN_PIN, b);

 

    return 0;
}






static int set_rgb_led_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;

    if (strlen(p) < 3) {
        printf("get rgb value length is illegal!\r\n");
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

    set_rgb_pins(p[0] - '0', p[1] - '0', p[2] - '0');

    if (offset != NULL)
        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));

    return 0;
}

static int set_rb_en_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;

    if (strlen(p) < 3) {
        printf("get rgb value length is illegal!\r\n");
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

    set_rb_pins(p[0] - '0', p[1] - '0');

    if (offset != NULL)
        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));

    return 0;
}







static int eeprom_test_handler(void* cline, void* reqline, int offset)
{
	/*
    if (sys_para.case_value[0] == CASE9) {
        tmds261_config(DISALLOW);
        i2c_eeprom_init();
    }

    if (I2C_OK == i2c_24c02_test()) {
        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    } else {
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
    }

    if (sys_para.case_value[0] == CASE9) {
        i2c_eeprom_deinit();
        tmds261_config(PORT1);
    }
*/
    return 0;
}

static int flash_test_handler(void* cline, void* reqline, int offset)
{
    /* get flash id */
    uint32_t flash_id = spi_flash_read_id();
    int8_t buf[32];

    sprintf(buf, "Flash ID[%d]\r\n", flash_id);
    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}





static int get_lux_handler(void* cline, void* reqline, int offset)
{
	uint8_t buf[48] = {0};
	uint32_t lux = bh1750_read_lux();

	sprintf((char *)buf, "GETLUX:%lulux\r\n", (unsigned long)lux);
	rlk_usb_cdc_send(buf, strlen((char *)buf));
	return 0;
}

static int get_adc_handler(void* cline, void* reqline, int offset)
{
	uint8_t buf[112] = {0};
	float ain0 = 0.0f;
	float ain1 = 0.0f;
	float ain2 = 0.0f;
	float diff23 = 0.0f;
	float pb0 = 0.0f;

	(void)cline;
	(void)reqline;
	(void)offset;

	if (0 != ads1115_read_getadc(&ain0, &ain1, &ain2, &diff23)) {
		rlk_usb_cdc_send((uint8_t *)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}
	pb0 = adc_in_read_volt();

	/* AIN0,AIN1,AIN2,Diff(AIN2-AIN3), PB0 ADC_IN */
	sprintf((char *)buf, "GETADC:%.3fV,%.3fV,%.3fV,%.3fV,%.3fV\r\n",
	        ain0, ain1, ain2, diff23, pb0);
	rlk_usb_cdc_send(buf, strlen((char *)buf));
	return 0;
}

static int send_hard_nec_handler(void* cline, void* reqline, int offset)
{
	char* p = (char*)cline + offset;

	(void)reqline;

	if (strlen(p) < 1) {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}

	if (!strcmp(p, NECPWR)) {
		printf_("\r\nSend Hard PWRKEY NEC IR.");
		send_ir_nec(NEC_PWR);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECUP)) {
		printf_("\r\nSend Hard UPKEY NEC IR.");
		send_ir_nec(NEC_UP);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECDWN)) {
		printf_("\r\nSend Hard DWNKEY NEC IR.");
		send_ir_nec(NEC_DWN);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECVUP)) {
		printf_("\r\nSend Hard VUPKEY NEC IR.");
		send_ir_nec(NEC_VUP);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECVDN)) {
		printf_("\r\nSend Hard VDNKEY NEC IR.");
		send_ir_nec(NEC_VDN);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECPUP)) {
		printf_("\r\nSend Hard PUPKEY NEC IR.");
		send_ir_nec(NEC_PUP);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECPDN)) {
		printf_("\r\nSend Hard PDNKEY NEC IR.");
		send_ir_nec(NEC_PDN);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECSEL)) {
		printf_("\r\nSend Hard SELKEY NEC IR.");
		send_ir_nec(NEC_SEL);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECLFT)) {
		printf_("\r\nSend Hard LFTKEY NEC IR.");
		send_ir_nec(NEC_LFT);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECRIT)) {
		printf_("\r\nSend Hard RITKEY NEC IR.");
		send_ir_nec(NEC_RIT);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECMUT)) {
		printf_("\r\nSend Hard MUTKEY NEC IR.");
		send_ir_nec(NEC_MUT);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECMNU)) {
		printf_("\r\nSend Hard MNUKEY NEC IR.");
		send_ir_nec(NEC_MNU);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECHOM)) {
		printf_("\r\nSend Hard HOMKEY NEC IR.");
		send_ir_nec(NEC_HOM);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECBAK)) {
		printf_("\r\nSend Hard BAKKEY NEC IR.");
		send_ir_nec(NEC_BAK);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECFWD)) {
		printf_("\r\nSend Hard FWDKEY NEC IR.");
		send_ir_nec(NEC_FWD);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECRWD)) {
		printf_("\r\nSend Hard RWDKEY NEC IR.");
		send_ir_nec(NEC_RWD);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECVOC)) {
		printf_("\r\nSend Hard VOCKEY NEC IR.");
		send_ir_nec(NEC_VOC);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else if (!strcmp(p, NECPLY)) {
		printf_("\r\nSend Hard PLYKEY NEC IR.");
		send_ir_nec(NEC_PLY);
		rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	} else {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}

	return 0;
}

static int ir_mon_act_handler(void* cline, void* reqline, int offset)
{
	char* p = (char*)cline + offset;

	(void)reqline;

	if (strlen(p) < 1) {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}

	if (!strcmp(p, IRMONSTART)) {
		printf_("\r\nIR MON START.");
		ir_mon_gpio_init();
		ir_mon_int_init();
		nvic_irq_enable(TIMER5_IRQn, 4, 1);
		beep();
	} else if (!strcmp(p, IRMONSTOP)) {
		printf_("\r\nIR MON STOP.");
		timer_disable(TIMER5);
		timer_interrupt_flag_clear(TIMER5, TIMER_INT_UP);
		nvic_irq_disable(TIMER5_IRQn);
		ir_mon_stop_exti();
		beep();
	} else {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}

	rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	return 0;
}

/*!
 * \brief  write one payload byte into page buffer / SPI flash
 * \return 0 ok, 1 reached max, -1 flash error
 */
static int audtst_wrdat_put(uint8_t b, uint8_t *page, uint32_t *page_len,
			    uint32_t *written)
{
	/* next byte would exceed 75KB → ERROR path */
	if (*written >= AUDTST_WRDAT_MAX_BYTES) {
		return 1;
	}
	page[*page_len] = b;
	(*page_len)++;
	(*written)++;
	if (*page_len >= AUDTST_WRDAT_PAGE_SIZE) {
		if (0 != spi_flash_write(*written - *page_len, page, *page_len)) {
			return -1;
		}
		*page_len = 0u;
	}
	return 0;
}

/*!
 * \brief  AT+AUDTST=F — DBG UART binary → SPI flash @0
 * \return 0 EOF OK; 1 over 75KB; -1 flash/other fail
 */
static int audtst_f_from_dbg(void)
{
	static const uint8_t eof_mark[4] = { 0xFAu, 0x05u, 0xA5u, 0x5Au };
	static uint8_t page[AUDTST_WRDAT_PAGE_SIZE];
	uint32_t page_len = 0u;
	uint32_t written = 0u;
	uint8_t match = 0u;
	uint8_t b;
	int hit_eof = 0;
	int put_rc = 0;

	i2s_audio_stop();
	audio_dac_stop();
	audio_pwm_stop();

	printf_("\r\nAUDTST=F erase %lu bytes @0 ...",
		(unsigned long)AUDTST_WRDAT_MAX_BYTES);
	if (0 != spi_flash_erase(0u, AUDTST_WRDAT_MAX_BYTES)) {
		printf_("\r\nAUDTST=F erase fail");
		return -1;
	}

	uart_dbg_rx_flush();
	printf_("\r\nAUDTST=F waiting DBG binary (EOF=FA05A55A, max=%lu)",
		(unsigned long)AUDTST_WRDAT_MAX_BYTES);

	while (1) {
		if (!uart_dbg_rx_byte(&b)) {
			continue;
		}

		if (b == eof_mark[match]) {
			match++;
			if (match >= 4u) {
				hit_eof = 1;
				break;
			}
			continue;
		}

		/* mismatch: flush matched prefix as payload, then re-check b */
		if (match > 0u) {
			uint8_t i;

			for (i = 0u; i < match; i++) {
				put_rc = audtst_wrdat_put(eof_mark[i], page, &page_len, &written);
				if (put_rc != 0) {
					goto wrdat_finish;
				}
			}
			match = 0u;
			if (b == eof_mark[0]) {
				match = 1u;
				continue;
			}
		}

		put_rc = audtst_wrdat_put(b, page, &page_len, &written);
		if (put_rc != 0) {
			goto wrdat_finish;
		}
	}

wrdat_finish:
	if (page_len > 0u) {
		if (0 != spi_flash_write(written - page_len, page, page_len)) {
			printf_("\r\nAUDTST=F write fail @%lu",
				(unsigned long)(written - page_len));
			return -1;
		}
	}

	if (put_rc < 0) {
		printf_("\r\nAUDTST=F write fail @%lu", (unsigned long)written);
		return -1;
	}

	if (hit_eof) {
		printf_("\r\nAUDTST=F done EOF, wrote %lu bytes @0",
			(unsigned long)written);
		return 0;
	}

	printf_("\r\nAUDTST=F overflow >%lu, wrote %lu bytes @0",
		(unsigned long)AUDTST_WRDAT_MAX_BYTES, (unsigned long)written);
	return 1;
}

static int audio_test_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
    errorcode_enum wav_err;
    int play_err;
    int is_wav_prog;
    int is_spi_play;
    int is_flash_recv;
    uint32_t wav_len;

    (void)reqline;

    /* lone W — embed WAV → flash */
    is_wav_prog = (p != NULL) && ((p[0] == 'W') || (p[0] == 'w')) &&
		  ((p[1] == '\r') || (p[1] == '\n') || (p[1] == '\0'));
    /* F — DBG UART binary → flash @0 */
    is_flash_recv = (p != NULL) && ((p[0] == 'F') || (p[0] == 'f')) &&
		    ((p[1] == '\r') || (p[1] == '\n') || (p[1] == '\0'));
    is_spi_play = (p != NULL) &&
		  ((p[0] == 'A') || (p[0] == 'a') ||
		   (p[0] == 'B') || (p[0] == 'b') ||
		   (p[0] == 'C') || (p[0] == 'c'));
    if ((p == NULL) ||
        ((!is_wav_prog) && (!is_flash_recv) && (!is_spi_play) &&
	 ((p[0] < '0') || (p[0] > '5'))) ||
        ((p[1] != '\r') && (p[1] != '\n') && (p[1] != '\0'))) {
        printf_("\r\nAUDTST bad arg");
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

    /* AT+AUDTST=F — DBG UART binary stream → SPI flash @0 */
    if (is_flash_recv) {
        int f_rc = audtst_f_from_dbg();

        if (f_rc == 0) {
            rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
            return 0;
        }
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

    /* AT+AUDTST=W — erase then program wavetestdata[] at SPI flash offset 0 */
    if (is_wav_prog) {
        i2s_audio_stop();
        audio_dac_stop();
        audio_pwm_stop();

        wav_len = wavetestdata_size;
        printf_("\r\nAUDTST=W erase+write %lu bytes @0", (unsigned long)wav_len);
        if (0 != spi_flash_erase(0u, wav_len)) {
            printf_("\r\nAUDTST=W erase fail");
            rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
            return -1;
        }
        if (0 != spi_flash_write(0u, (const uint8_t *)wavetestdata, wav_len)) {
            printf_("\r\nAUDTST=W write fail");
            rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
            return -1;
        }
        {
            uint8_t chk[16];
            uint32_t i;

            if (0 != spi_flash_read(0u, chk, 16u)) {
                printf_("\r\nAUDTST=W verify-read fail");
                rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
                return -1;
            }
            for (i = 0; i < 16u; i++) {
                if (chk[i] != (uint8_t)wavetestdata[i]) {
                    printf_("\r\nAUDTST=W verify mismatch @%lu got=0x%02X exp=0x%02X",
                        (unsigned long)i, (unsigned)chk[i],
                        (unsigned)(uint8_t)wavetestdata[i]);
                    rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
                    return -1;
                }
            }
        }
        printf_("\r\nAUDTST=W done verify OK");
        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
        return 0;
    }

    /* reply first so AT never waits on audio start */
    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));

    if (p[0] == '0') {
        audio_dac_stop();
        audio_pwm_stop();
        if (0 != i2s_tone_1k_3s()) {
            printf_("\r\nI2S tone start failed");
            return -1;
        }
        return 0;
    }

    if (p[0] == '1') {
        audio_dac_stop();
        audio_pwm_stop();
        wav_err = i2s_audio_play(AUDIO_WAV_SRC_EMBED);
        if (VALID_WAVE_FILE != wav_err) {
            printf_("\r\nI2S wave start failed err=%d", (int)wav_err);
            return -1;
        }
        printf_("\r\nI2S wave play started");
        return 0;
    }

    if (p[0] == '2') {
        i2s_audio_stop();
        audio_pwm_stop();
        if (0 != audio_dac_tone_1k_3s()) {
            printf_("\r\nDAC tone start failed");
            return -1;
        }
        return 0;
    }

    if (p[0] == '3') {
        i2s_audio_stop();
        audio_pwm_stop();
        play_err = audio_dac_wave_play(AUDIO_WAV_SRC_EMBED);
        if (0 != play_err) {
            printf_("\r\nDAC wave start failed err=%d", play_err);
            return -1;
        }
        printf_("\r\nDAC wave play started");
        return 0;
    }

    if (p[0] == '4') {
        i2s_audio_stop();
        audio_dac_stop();
        if (0 != audio_pwm_tone_1k_3s()) {
            printf_("\r\nPWM tone start failed");
            return -1;
        }
        return 0;
    }

    /* AT+AUDTST=A — SPI flash WAV @0 via I2S */
    if ((p[0] == 'A') || (p[0] == 'a')) {
        audio_dac_stop();
        audio_pwm_stop();
        wav_err = i2s_audio_play(AUDIO_WAV_SRC_SPI);
        if (VALID_WAVE_FILE != wav_err) {
            printf_("\r\nI2S SPI-wave start failed err=%d", (int)wav_err);
            return -1;
        }
        printf_("\r\nI2S SPI-wave play started");
        return 0;
    }

    /* AT+AUDTST=B — SPI flash WAV @0 via DAC */
    if ((p[0] == 'B') || (p[0] == 'b')) {
        i2s_audio_stop();
        audio_pwm_stop();
        play_err = audio_dac_wave_play(AUDIO_WAV_SRC_SPI);
        if (0 != play_err) {
            printf_("\r\nDAC SPI-wave start failed err=%d", play_err);
            return -1;
        }
        printf_("\r\nDAC SPI-wave play started");
        return 0;
    }

    /* AT+AUDTST=C — SPI flash WAV @0 via PWM */
    if ((p[0] == 'C') || (p[0] == 'c')) {
        i2s_audio_stop();
        audio_dac_stop();
        play_err = audio_pwm_wave_play(AUDIO_WAV_SRC_SPI);
        if (0 != play_err) {
            printf_("\r\nPWM SPI-wave start failed err=%d", play_err);
            return -1;
        }
        printf_("\r\nPWM SPI-wave play started");
        return 0;
    }

    /* AT+AUDTST=5 — PWM play wavetestdata[] */
    i2s_audio_stop();
    audio_dac_stop();
    play_err = audio_pwm_wave_play(AUDIO_WAV_SRC_EMBED);
    if (0 != play_err) {
        printf_("\r\nPWM wave start failed err=%d", play_err);
        return -1;
    }
    printf_("\r\nPWM wave play started");
    return 0;
}



static int nvm_ver_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
    uint8_t read_data[VFLG_LENGTH];

    if (strlen(p) > NVM_VER_LENGTH+2 && strlen(p) < 1) {  		printf_("\r\nNVM version Input too Long!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
        return -1;
    }

    eeprom_buffer_read_wrapper(read_data, NVM_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
        *p -= '0' ;
		printf_("\r\nNVM Version Protected! NVM ver: 0x%x",*p) ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } else {
        memcpy(sys_para.nvm_vflg, valid_vflg, VFLG_LENGTH);
        eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, VFLG_LENGTH);
        *p -= '0' ;
        memcpy(sys_para.nvm_ver, p, NVM_VER_LENGTH);
        eeprom_buffer_write_wrapper(p, NVM_VER_OFFSET, NVM_VER_LENGTH);

        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    }

    return 0;
}

static int nvm_ver_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[2];
    uint8_t default_data[NVM_VER_LENGTH] ={NVM_VER} ;

    eeprom_buffer_read_wrapper(read_data, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "NVM dft Version:0x%x\r\n", default_data[0]);
    } else {
		eeprom_buffer_read_wrapper(read_data, NVM_VER_OFFSET, NVM_VER_LENGTH);
		printf_("\r\nNVM nvm Version:0x%x", read_data[0]);
		sprintf(buf, "NVMVER:0x%x\r\n", read_data[0]) ;
    }

    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}


static int dsn_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
    uint8_t read_data[VFLG_LENGTH];

    if (strlen(p) > DSN_LENGTH+2 && strlen(p) < 1) { 		printf_("\r\nDSN Input too Long!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
        return -1;
    }

    eeprom_buffer_read_wrapper(read_data, DSN_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
		printf_("\r\nCannot setup DSN!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } else {
        memcpy(sys_para.dsn_vflg, valid_vflg, VFLG_LENGTH);
        eeprom_buffer_write_wrapper(sys_para.dsn_vflg, DSN_VFLG_OFFSET, VFLG_LENGTH);

        memcpy(sys_para.dsn, p, DSN_LENGTH);
        eeprom_buffer_write_wrapper(p, DSN_OFFSET, DSN_LENGTH);

        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    }

    return 0;
}

static int dsn_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[DSN_LENGTH+1];
    uint8_t default_data[DSN_LENGTH + 1] = "AWKN-0-0000-0000";

    eeprom_buffer_read_wrapper(read_data, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "DSN:%s\r\n", default_data);
    } else {
        eeprom_buffer_read_wrapper(read_data, DSN_OFFSET, DSN_LENGTH);

        read_data[DSN_LENGTH] = '\0';
        printf_("\r\nDSN:%s", read_data);
        sprintf(buf, "DSN:%s\r\n", read_data);
    }

    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}



static int hw_ver_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
    uint8_t read_data[VFLG_LENGTH];

    if (strlen(p) > HWVER_LENGTH+2 && strlen(p) < 1) {  		printf_("\r\nNVM version Input too Long!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
        return -1;
    }
	
	*p -= '0' ;
	if (*p > 2)
	{
	   	printf_("\r\nInvalid HW Version Input!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1 ;
	}


    eeprom_buffer_read_wrapper(read_data, DSN_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) 
	{
		printf_("\r\nHW Version Protected!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } 
	else 
	{
        memcpy(sys_para.hwver, p, HWVER_LENGTH);
        eeprom_buffer_write_wrapper(p, HWVER_OFFSET, HWVER_LENGTH);
		hw_ver = *p ;
        rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    }

    return 0;
}

static int hw_ver_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[2];
    uint8_t default_data[HWVER_LENGTH] ={0} ;

    eeprom_buffer_read_wrapper(read_data, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "HW Version dft :0x%x\r\n", default_data[0]);
    } else {
		eeprom_buffer_read_wrapper(read_data, HWVER_OFFSET, HWVER_LENGTH);
		hw_ver = read_data[0] ;
		printf_("\r\nHW Version:0x%x", read_data[0]);
		sprintf(buf, "HVER:AWKN-HVER%x\r\n", read_data[0]) ;
    }

    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}


static int set_vflg_handler(void* cline, void* reqline, int offset)
{
//	static unsigned char display_type = VB1_4K_1DIV ;
	char* p = (char*)cline + offset;
	char* save_to_ee = (char*)reqline;
	
	if (strlen(p) < 1) {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}
	
	
	
	if (!strcmp(p ,OPRDSN))	// If equ , N logic
	{
		memcpy(sys_para.dsn_vflg, valid_vflg, DSN_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.dsn_vflg, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);
	}
	else if (!strcmp(p , OPRPRD) )
	{
		memcpy(sys_para.prd_vflg, valid_vflg, PRD_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.prd_vflg, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);
	}
	else if (!strcmp(p , OPRNVMVER) )
	{
		memcpy(sys_para.nvm_vflg, valid_vflg, NVM_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);
	}	
	else if (!strcmp(p , OPRUSR) )
	{
		memcpy(sys_para.usr_vflg, valid_vflg, USR_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.usr_vflg, USR_VFLG_OFFSET, USR_VFLG_LENGTH);
	}			
	else if (!strcmp(p , OPRALL) )
	{
		memcpy(sys_para.dsn_vflg, valid_vflg, DSN_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.dsn_vflg, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);
		memcpy(sys_para.prd_vflg, valid_vflg, PRD_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.prd_vflg, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);
		memcpy(sys_para.nvm_vflg, valid_vflg, NVM_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);
		memcpy(sys_para.usr_vflg, valid_vflg, USR_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.usr_vflg, USR_VFLG_OFFSET, USR_VFLG_LENGTH);
	}	
	else
	{
			rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
			return -1;
	}


	
    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	return 0 ;

}



static int del_vflg_handler(void* cline, void* reqline, int offset)
{
//	static unsigned char display_type = VB1_4K_1DIV ;
	char* p = (char*)cline + offset;
	char* save_to_ee = (char*)reqline;
	
	if (strlen(p) < 1) {
		rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
		return -1;
	}
	
	
	
	if (!strcmp(p ,OPRDSN))	// If equ , N logic
	{
		memcpy(sys_para.dsn_vflg, inval_vflg, DSN_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.dsn_vflg, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);
	}
	else if (!strcmp(p , OPRPRD) )
	{
		memcpy(sys_para.prd_vflg, inval_vflg, PRD_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.prd_vflg, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);
	}
	else if (!strcmp(p , OPRNVMVER) )
	{
		memcpy(sys_para.nvm_vflg, inval_vflg, NVM_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);
	}	
	else if (!strcmp(p , OPRUSR) )
	{
		memcpy(sys_para.usr_vflg, inval_vflg, USR_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.usr_vflg, USR_VFLG_OFFSET, USR_VFLG_LENGTH);
	}	
	else if (!strcmp(p , OPRALL) )
	{
		memcpy(sys_para.dsn_vflg, inval_vflg, DSN_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.dsn_vflg, DSN_VFLG_OFFSET, DSN_VFLG_LENGTH);
		memcpy(sys_para.prd_vflg, inval_vflg, PRD_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.prd_vflg, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);
		memcpy(sys_para.nvm_vflg, inval_vflg, NVM_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.nvm_vflg, NVM_VFLG_OFFSET, NVM_VFLG_LENGTH);
		memcpy(sys_para.usr_vflg, inval_vflg, USR_VFLG_LENGTH);
		eeprom_buffer_write_wrapper(sys_para.usr_vflg, USR_VFLG_OFFSET, USR_VFLG_LENGTH);
	}	
	else
	{
			rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
			return -1;
	}
	
    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	return 0 ;

}


static int product_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
    uint8_t read_data[VFLG_LENGTH];


    if (strlen(p) > PRODUCT_LENGTH+2 && strlen(p) < 1) {         printf_("\r\nInvalid Product Length") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
        return -1;
    }


	eeprom_buffer_read_wrapper(read_data, PRD_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
		printf_("\r\nCannot setup DSN!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    	}  
	else
	{

    memset(sys_para.product, '\0', PRODUCT_LENGTH);
    memcpy(sys_para.product, p, strchr(p, '\r') - p);
    eeprom_buffer_write_wrapper(sys_para.product, PRODUCT_OFFSET, PRODUCT_LENGTH);

    rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    }

    return 0;

}

static int product_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[PRODUCT_LENGTH + 1];
    unsigned char default_data[PRODUCT_LENGTH + 2 + 1] = "<AWAKENO>\r\n";


    eeprom_buffer_read_wrapper(read_data, PRD_VFLG_OFFSET, PRD_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "PRODUCT:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, PRODUCT_OFFSET, PRODUCT_LENGTH);
		
		read_data[PRODUCT_LENGTH] = '\0';
		
		sprintf(buf, "PRODUCT:%s\r\n", read_data);
    }	



    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}


static int owner_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
	uint8_t read_data[VFLG_LENGTH];

    if (strlen(p) > OWNER_LENGTH+2 && strlen(p) < 1) {         rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
        return -1;
    }

    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
		printf_("\r\nVFLG locked Cannot setup Owner!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } 
	else 
	{	

      memset(sys_para.owner, '\0', OWNER_LENGTH);
      memcpy(sys_para.owner, p, strchr(p, '\r') - p);
      eeprom_buffer_write_wrapper(sys_para.owner, OWNER_OFFSET, OWNER_LENGTH);

      rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
    }

    return 0;
}

static int owner_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[OWNER_LENGTH + 1];
    unsigned char default_data[OWNER_LENGTH + 2 + 1] = "USER\r\n";


    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "OWNER:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, OWNER_OFFSET, OWNER_LENGTH);
		
		read_data[OWNER_LENGTH] = '\0';
		
		sprintf(buf, "OWNER:%s\r\n", read_data);
    }	



    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}

static int login_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
	uint8_t read_data[VFLG_LENGTH];

    if (strlen(p) > LOGIN_LENGTH+2 && strlen(p) < 1) {         rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

	eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
		printf_("\r\nVFLG locked Cannot setup Login!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } 
	else 
	{	
      memset(sys_para.login, '\0', LOGIN_LENGTH);
      memcpy(sys_para.login, p, strchr(p, '\r') - p);
      eeprom_buffer_write_wrapper(sys_para.login, LOGIN_OFFSET, LOGIN_LENGTH);

      rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	}

    return 0;
}

static int login_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[LOGIN_LENGTH + 1];
    unsigned char default_data[LOGIN_LENGTH + 2 + 1] = "user@\r\n";


    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "LOGIN:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, LOGIN_OFFSET, LOGIN_LENGTH);
		
		read_data[LOGIN_LENGTH] = '\0';
		
		sprintf(buf, "LOGIN:%s\r\n", read_data);
    }	





    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}

static int locat_set_handler(void* cline, void* reqline, int offset)
{
    char* p = (char*)cline + offset;
	uint8_t read_data[VFLG_LENGTH];


    if (strlen(p) > LOCAT_LENGTH+2 && strlen(p) < 1) {         rlk_usb_cdc_send((uint8_t*)ACK_ERR, strlen(ACK_ERR));
        return -1;
    }

	eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, VFLG_LENGTH);

    if (read_data[0] == VALID_FLAG_L && read_data[1] == VALID_FLAG_H) {
		printf_("\r\nVFLG locked Cannot setup Location!") ;
        rlk_usb_cdc_send((uint8_t*)ACK_RJT, strlen(ACK_RJT));
    } 
	else 
	{	

      memset(sys_para.locat, '\0', LOCAT_LENGTH);
      memcpy(sys_para.locat, p, strchr(p, '\r') - p);
      eeprom_buffer_write_wrapper(sys_para.locat, LOCAT_OFFSET, LOCAT_LENGTH);

		
      rlk_usb_cdc_send((uint8_t*)ACK_OK, strlen(ACK_OK));
	}
    return 0;
}

static int locat_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};
    uint8_t read_data[LOCAT_LENGTH + 1];
    unsigned char default_data[LOCAT_LENGTH + 2 + 1] = "Beijing\r\n";



    eeprom_buffer_read_wrapper(read_data, USR_VFLG_OFFSET, USR_VFLG_LENGTH);

	if (read_data[0] != VALID_FLAG_L || read_data[1] != VALID_FLAG_H) {
        sprintf(buf, "LOCAT:%s", default_data);
    } else {
		eeprom_buffer_read_wrapper(read_data, LOCAT_OFFSET, LOCAT_LENGTH);
		
		read_data[LOCAT_LENGTH] = '\0';
		
		sprintf(buf, "LOCAT:%s\r\n", read_data);

    }	




    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}


static int version_get_handler(void* cline, void* reqline, int offset)
{
    uint8_t buf[64] = {0};

    sprintf(buf, "VER:%s\r\n", FIREWARE_VERSION);

    rlk_usb_cdc_send((uint8_t*)buf, strlen(buf));

    return 0;
}







static uint8_t help_copyright[] =
	"\r\n******************************************************\r\n"
	"* Copyright (c) 2026 RayzerLink. Licensed under MIT. *\r\n"
	"*                        website: www.rayzerlink.com *\r\n"
	"*               slack/email: tide.yin@rayzerlink.com *\r\n"
	"*                           linkedin/wechat: tideyin *\r\n"
	"******************************************************\r\n";
static uint8_t help_header[] = "======== TTLAB AT Commands ========\r\n";
static const char * const help_lines[] = {
	"AT? --Return OK\r\n",
	"HELP? --List AT commands\r\n",
	"AT+PING? --Return PING:AWAKENO\r\n",
	"AT+SYSRST=<Value> Value:REBOOT/DFU --System reset\r\n",
	"AT+NVMVER=<String> --Set NVM version string\r\n",
	"AT+NVMVER? --Return NVM version\r\n",
	"AT+SETVFLG=<Value> Value:DSN/PRD/NVMVER/USR/ALL --Set valid flag\r\n",
	"AT+DELVFLG=<Value> Value:DSN/PRD/NVMVER/USR/ALL --Clear valid flag\r\n",
	"AT+DSN=<String> --Set device serial number\r\n",
	"AT+DSN? --Return AWAKENO DSN\r\n",
	"AT+VER? --Return Firmware Version\r\n",
	"AT+HVER=<Value> --Set hardware version\r\n",
	"AT+HVER? --Return Hardware Version\r\n",
	"AT+RGB=<RGB> RGB:3 digits 0/1 --Set LED_R/G/B\r\n",
	"AT+RBEN=<RB> RB:2 digits 0/1 --Set RED_EN/BLUE_EN\r\n",
	"AT+PRODUCT=<String> --Set product name\r\n",
	"AT+PRODUCT? --Return product name\r\n",
	"AT+OWNER=<String> --Set owner\r\n",
	"AT+OWNER? --Return owner\r\n",
	"AT+LOGIN=<String> --Login / set session\r\n",
	"AT+LOGIN? --Return login status\r\n",
	"AT+LOCAT=<String> --Set location\r\n",
	"AT+LOCAT? --Return location\r\n",
	"AT+AUDTST=<Mode> Mode:0 I2S 1k/3s;1 I2S embed WAV;2 DAC 1k/3s;3 DAC embed WAV;4 PWM 1k/3s;5 PWM embed WAV;A/B/C SPI-flash WAV via I2S/DAC/PWM;W write embed WAV to flash;F DBG UART binary to flash\r\n",
	"AT+GETLUX? --Return GETLUX:<n>lux\r\n",
	"AT+GETADC? --Return GETADC:AIN0,AIN1,AIN2,Diff23,PB0 (V)\r\n",
	"AT+HD_NEC=<Key> Key:NECPWR/NECUP/NECDWN/NECLFT/NECRIT/NECVUP/NECVDN/NECHOM/NECBAK/NECMUT/NECMNU/NECPUP/NECPDN/NECFWD/NECRWD/NECVOC/NECPLY/NECSEL --TX IR NEC\r\n",
	"AT+IRMON=<Value> Value:START/STOP --IR monitor\r\n",
};

static int sw_help_handler(void* cline, void* reqline, int offset)
{
	unsigned i;

	(void)cline;
	(void)reqline;
	(void)offset;

	rlk_usb_cdc_send(help_copyright, strlen((char *)help_copyright));
	rlk_usb_cdc_send(help_header, strlen((char *)help_header));
	for (i = 0u; i < (unsigned)(sizeof(help_lines) / sizeof(help_lines[0])); i++) {
		rlk_usb_cdc_send((uint8_t *)help_lines[i], strlen(help_lines[i]));
	}

	return 0;
}
