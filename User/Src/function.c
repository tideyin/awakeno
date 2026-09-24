// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "function.h"
#include "systick.h"
#include "rlk_gpio.h"

extern uint8_t hw_ver ;


void beep(void)
{
    int i , j = 0;
	for (i=0 ; i< 100 ; i++)
	{
		  rlk_gpio_set_value(BEE_EN_PORT, BEE_EN_PIN, j);
		  j = j^1 ;
      delay_1ms(1) ;
	}
}

void Music(unsigned int frq ,unsigned int dly)
{
   static unsigned char buzz_level =0 ;
   int i,j;
   for(i=0;i<dly;i++)
   {  
      buzz_level = buzz_level^1 ;
      rlk_gpio_set_value(BEE_EN_PORT, BEE_EN_PIN,buzz_level ) ;
      for(j=0;j<frq;j++)
      {
      	delay_while_us(8) ;
    	}
   } 
} 

void get_hardware_ver(unsigned char *buf ) 
{
	// HW version 1 (hard code)
	if(hw_ver == 1)
	  sprintf(buf, "%s", "AWKN-HVER1");
	else
	  sprintf(buf, "%s", "AWKN-UNKNOW");
}
