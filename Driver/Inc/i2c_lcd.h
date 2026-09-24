// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef _I2C_LCD_H
#define _I2C_LCD_H

#define AWAKENNO

#ifdef AWAKENNO 
#define LCD_ADDR    0x7A
#else
#define LCD_ADDR    0x78
#endif

/* SSD1306 panel: 128x64 (8 pages, page y = 0..7) */
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  (OLED_HEIGHT / 8)

void Init_I2C(void) ;
void I2C_WriteMode(void) ;
void I2C_ReadMode(void) ;
void I2C_Txbyte(unsigned char Reg_addr,unsigned char Reg_data) ;
void I2C_Rxbyte(unsigned char Reg_addr) ;
void WriteCmd(unsigned char I2C_Command) ;
void WriteDat(unsigned char I2C_Data) ;

void I2C_LCD_Init() ;
void OLED_Fill(unsigned char fill_Data) ;

void OLED_ON(void) ;
void OLED_OFF(void) ;
void OLED_CLS(void) ;
void OLED_Fill(unsigned char fill_Data) ;
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) ;
void OLED_ShowStr_GT22(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) ;
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char n) ;
void OLED_ShowCN_GT22(unsigned char x, unsigned char y, unsigned char ch[]) ;
/* Unicode BMP codes, NUL-terminated; drawn RTL (right-to-left scripts) */
/* returns x after last glyph (right edge of Hebrew block) */
unsigned char OLED_ShowHeb_GT22(unsigned char x, unsigned char y, const unsigned short uni[]) ;
/* x = right inset (0 = flush right); returns leftmost x of Arabic block */
unsigned char OLED_ShowArb_GT22(unsigned char x, unsigned char y, const unsigned short uni[]) ;
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) ;
void OLED_SetPos(unsigned char x, unsigned char y) ;
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned char num , unsigned char TextSize) ;
void OLED_ShowValue(unsigned char x, unsigned char y, unsigned int Value , unsigned char TextSize) ;

#endif
