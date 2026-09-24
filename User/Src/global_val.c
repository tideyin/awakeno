// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gd32e10x.h"


uint8_t hw_ver ;

/* KEY_MENU (PB4 / EXTI4) — set in IRQ, cleared in main */
uint8_t menu_key_flag;

/* IR NEC TX / monitor (ir_snd_rcv.c) */
uint8_t ir_code_org , ir_code_bar ;
uint8_t pcba_flg ;
uint8_t ir_cid , ir_cid_bar ;
uint16_t ir_counter ;
uint8_t ir_mon_int_flag , timer5_int_flag ;


// == For music play
uint8_t music_flag = 0;
int m_index = 0;

/*
 * Music(FREQUENCY, DELAY): half-period wait = FREQUENCY * 8 us
 *   pitch_Hz ≈ 62500 / FREQUENCY
 *   duration ≈ DELAY * FREQUENCY * 8 us
 * End marker: FREQUENCY=0 and DELAY=0
 */

// Xiyouji (Journey to the West) theme — archived
unsigned int const FREQUENCY[]={316,264,316,212,236,236,212,236,264,280,316,280,236,212,264,316,212,156,212,156,176,200,212,264,236,212,200,212,236,316,212,236,212,316,264,212,236,280,212,236,316,264,236,212,212,156,212,156,176,200,212,176,236,200,212,236,264,236,236,280,212,280,316,356,316,0};
unsigned int const DELAY[] ={250,500,250,750,250,250,125,125,1500,250,500,250,750,250,250,1750,1000,750,250,500,250,250,1000,750,250,500,250,250,2000,500,500,250,250,500,1500,500,250,500,250,250,250,250,250,2000,1000,750,250,500,250,250,1000,250,500,250,250,250,500,2000,250,500,250,250,250,500,1500,0};


/* 凤凰传奇《月亮之上》副歌（DELAY 量级对齐西游记：500/1000/1500/2000）
 * 我在仰望，月亮之上，有多少梦想都在自由的飞翔
 */
#if 0 
unsigned int const FREQUENCY[] = {
	/* 我在仰望 */
	284, 239, 179, 179,
	/* 月亮之上 */
	189, 213, 239, 239,
	/* 有多少梦想 */
	239, 213, 239, 284, 319,
	/* 都在自由的飞翔 */
	357, 319, 284, 239, 213, 179, 239,
	0
};
unsigned int const DELAY[] = {
	/* 四分≈1000，附点/二分≈1500，长音≈2000，八分≈500 */
	1000, 1000, 1500, 1500,
	1000, 1000, 1500, 1500,
	 500,  500,  500,  500, 1500,
	 500,  500,  500,  500,  500, 1500, 2000,
	0
};

#endif
