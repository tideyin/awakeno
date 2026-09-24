// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include <stdio.h>
#include "rlk_cdc.h"
#include "rlk_gpio.h"
#include "rlk_cmd.h"
#include "systick.h"
#include "spi_flash.h"
#include "i2c_lcd.h"
#include "fontlib.h"
#include "menu_key.h"
#include "drv_timer.h"
#include "rlk_key_state.h"
#include "uart_adapter.h"
#include "printf.h"
#include "cm_backtrace.h"
#include "core_cm4.h"
#include "ads1115.h"
#include "mcp4725.h"
#include "i2s_codec.h"
#include "bh1750.h"
#include "adc_gd32.h"
#include "ir_snd_rcv.h"
#include "function.h"
#include "i2s_codec.h"
#include "audio_dac.h"
#include "audio_pwm.h"
#include "matrix_key.h"

 

extern unsigned char RxBuf[];



//For music
extern unsigned int const FREQUENCY[] ;
extern unsigned int const DELAY[];
extern uint8_t music_flag ;
extern int m_index ;

extern uint8_t menu_key_flag;

/* IR monitor (Raw pulse dump → Debug UART) */
extern uint16_t ir_counter ;
extern uint8_t ir_mon_int_flag , timer5_int_flag ;


/* Port Pin value */
static rlk_gpio_t awakeno_gpio_config[] = {
    /* Buzzer Pin */
    {BEE_EN_PORT, BEE_EN_PIN, 0},    

    /* LED R G B */
    {LED_R_PORT, LED_R_PIN, 0},
    {LED_G_PORT, LED_G_PIN, 0},
    {LED_B_PORT, LED_B_PIN, 1},

   /* Red and Blue high light LEDs */	
    {RED_LED_EN_PORT, RED_LED_EN_PIN, 0},
    {BLUE_LED_EN_PORT, BLUE_LED_EN_PIN, 0},

	/* EEPROM WP */
	{EEPROM_WP_PORT, EEPROM_WP_PIN, 0},   // EEPROM WR Protect

	/* PB2 = BOOT1 / PA_CTL */
	{PA_CTL_PORT, PA_CTL_PIN, 1},
};





void rlk_hw_init(void)
{
    SystemInit();

    systick_config();


	uart_adapter_init(); // Enable UART Log early 



	printf_("\r\n");
    printf_("****************************************************\r\n");
	printf_("*                                                  *\r\n");
    printf_("*                    Hello Awakeno !               *\r\n");
	printf_("*                                                  *\r\n");
    printf_("*                                Powered By TTLAB  *\r\n");
    printf_("*                         tide.yin@rayzerlink.com  *\r\n") ;
	printf_("*                                                  *\r\n");
    printf_("****************************************************\r\n");




	rlk_gpio_init(awakeno_gpio_config, sizeof(awakeno_gpio_config) / sizeof(rlk_gpio_t));	

// nvm_init must after general GPIO init , don't know why so far 2025/10/30
	nvm_init() ;

    spi_flash_init();
	
	printf_("\r\nFlash ID: 0x%x", spi_flash_read_id()) ;





#if 1	
    rlk_usb_cdc_init();
#endif




#if 1


     I2C_LCD_Init();


     printf_("\r\nNVM init finished!") ;



#endif


    menu_key_flag = 0;
    key_menu_gpio_init();
    key_menu_int_init();
    printf_("\r\nKey init finished!") ;

    matrix_key_init();
    printf_("\r\nMatrix key init finished!") ;

	ads1115_init();
	printf_("\r\nADS1115 init finished!") ;

	adc_in_init();
	printf_("\r\nADC_IN PB0 init finished!") ;

    mcp4725_set_value(0x7FF);
	printf_("\r\nMCP4725 init finished!") ;	

	bh1750_init();
	printf_("\r\nBH1750 init finished!") ;

	nvm_hw_init() ;

	i2s_codec_init(I2S_DEFAULT_RATE);
	printf_("\r\nI2S codec init finished (PA_CTL/BOOT1=PB2 off)!");

	audio_dac_init();
	printf_("\r\nAudio DAC (PA4) init finished!");

	audio_pwm_init();
	printf_("\r\nAudio PWM (PC6) init finished!");

#if 0
    OLED_CLS();
    OLED_DrawBMP(0, 0, 128, 8, (unsigned char*)BMP1);

    while(1) ;
#endif 		

#if 1
    /* LCD/GT22 touch SPI0 — must finish before SPI-flash WAV streaming */
    state_key_menu_init(SHOW_LOGO_STATE);
    printf_("\r\nMenu State init finished!") ;

#endif

    ir_emu_init();
    printf_("\r\nIR NEC TX init finished (PC8/TIMER7_CH2 + TIMER3)!");

	/*
	 * Boot greeting AFTER menu/IR: GT22 may re-init SPI0; restore flash,
	 * then start I2S. Starting play earlier underruns (no poll) and races SPI0.
	 */
	spi_flash_init();
	{
		errorcode_enum wav_err;
		uint32_t boot_ms;

		audio_dac_stop();
		audio_pwm_stop();
		wav_err = i2s_audio_play(AUDIO_WAV_SRC_SPI);
		if (VALID_WAVE_FILE != wav_err) {
			printf_("\r\nBoot I2S SPI-wave failed err=%d", (int)wav_err);
		} else {
			printf_("\r\nBoot I2S SPI-wave play started");
			/* drain greeting; cap wait so soft-reset hang cannot freeze boot */
			for (boot_ms = 0u; (boot_ms < 8000u) && i2s_tone_busy(); boot_ms++) {
				audio_wav_stream_fill();
				delay_1ms(1);
			}
			if (i2s_tone_busy()) {
				printf_("\r\nBoot I2S SPI-wave timeout — stop");
				i2s_audio_stop();
			} else {
				printf_("\r\nBoot I2S SPI-wave done");
			}
		}
	}

}

int main(void)
{
    int len = 0;
    uint8_t* cmd_buffer = NULL;
    key_press_mode_t press_mode;

    rlk_hw_init();	


#ifdef USE_CMBACKTRACE
    cm_backtrace_init("CmBacktrace", "V1.0.0", FIREWARE_VERSION);
#endif

#if 0
		while(1)
	{
    delay_1ms(1000) ;
	}	

#endif


    while (1) {
        /* SPI-flash WAV fill first — USB/keys/printf must not starve the queue */
        i2s_codec_poll();
        audio_dac_poll();
        audio_pwm_poll();

        /* AT command path: USB CDC only (USART0 is debug log, not AT) */
        if (rlk_check_usb_cdc_status()) {
            len = rlk_usb_cdc_receive(&cmd_buffer);
            if (len > 0 && cmd_buffer) {
                run_cmd_handler((char*)cmd_buffer);
            }
        }
		
// For music play		
		if(FREQUENCY[m_index]!=0||DELAY[m_index]!=0) 
		{ 

		   if(music_flag == 1)
		   {  
			 Music(FREQUENCY[m_index],DELAY[m_index]); 
			 m_index++ ;
		   }  
	    }
		else
	      m_index = 0 ;

		
#if 1
		/* KEY_MENU: EXTI falling → flag; debounce + short/long in main */
		if (menu_key_flag == 1) {
			press_mode = key_pressed_mode();
			menu_key_flag = 0;

			if (SHORT_PRESS == press_mode) {
				state_key_menu();
			} else if (LONG_PRESS == press_mode) {
				/* reserved */
			}

			press_mode = NO_PRESS;
			nvic_irq_enable(KEY_MENU_EXIT_IRQn, 2U, 0U);
		}
#endif     
     

        /* IR Raw dump — same format as WuKong-FB (was ADC UART; here Debug USART0) */
        if (ir_mon_int_flag == 1 || timer5_int_flag == 1) {
            if (ir_counter > 0 && ir_counter < 41800) {
                if (SET == gpio_input_bit_get(IR_MON_IO_PORT, IR_MON_IO_PIN))
                    printf_("+");
                else
                    printf_("-");
                printf_("%d ", ir_counter);
            }

            /* gap >= 41.8ms while line idle-low → end of one frame */
            if (ir_mon_int_flag == 1 && ir_counter >= 41800
                && RESET == gpio_input_bit_get(IR_MON_IO_PORT, IR_MON_IO_PIN)) {
                printf_(",\r\n}\r\n");
                timer5_int_flag = 0;
                ir_counter = 0;
                timer_counter_value_config(TIMER5, 0);
            }

            if (ir_mon_int_flag == 1) {
                ir_mon_int_flag = 0;
                exti_interrupt_enable(IR_MON_EXIT);
            }

            if (0 == ir_counter && 1 != timer5_int_flag) {
                printf_("\r\n{");
                printf_("\r\n    Raw:");
            }

            if (1 == timer5_int_flag) {
                printf_(",\r\n}\r\n");
                timer5_int_flag = 0;
            }
        }

        drv_timer_process();
        uart_adapter_data_process();
        /* second fill pass after slow USB/key work */
        i2s_codec_poll();
        matrix_key_process();
    }

    return 0;
}

/* retarget tiny-printf (printf_ / printf) to USART0 */
void _putchar(char character)
{
    drv_uart_send(DBG_UART, (uint8_t*)&character, 1, true);
}

/* retarget newlib/stdio printf to USART0 */
#ifdef __GNUC__
int _write(int file, char *ptr, int len)
{
    int i;

    (void)file;
    for (i = 0; i < len; i++) {
        _putchar(ptr[i]);
    }
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    (void)f;
    _putchar((char)ch);
    return ch;
}
#endif

