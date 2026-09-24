// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "i2c_lcd.h"
#include "fontlib.h"
#include "gt22l16a2y.h"
#include "gd32e10x.h"
#include <stdlib.h>


/****************************************************************
* Function: USCI_B0 - I2C init
* Input:
* Output:
* Describe: SMCLK=DCOCLK
****************************************************************/
void Init_I2C(void)
{
	/* enable GPIOB clock */
	rcu_periph_clock_enable(RCU_GPIOB);
	/* enable I2C1 clock */
	rcu_periph_clock_enable(RCU_I2C1);

	/* connect PB6 to I2C1_SCL */
	/* connect PB7 to I2C1_SDA */
	gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);

	/* enable I2C clock */
    rcu_periph_clock_enable(RCU_I2C1);
    /* configure I2C clock */
    i2c_clock_config(I2C1,400000,I2C_DTCY_2);
    /* configure I2C address */
    i2c_mode_addr_config(I2C1,I2C_I2CMODE_ENABLE,I2C_ADDFORMAT_7BITS,LCD_ADDR);
    /* enable I2C1 */
    i2c_enable(I2C1);
    /* enable acknowledge */
    i2c_ack_config(I2C1,I2C_ACK_ENABLE);
}
void I2C_WriteMode(void)
{

}
void I2C_ReadMode(void)
{

}

/****************************************************************
* Function: send a byte data by I2C
* Input: Reg_addr: 7-bit slave address, Reg_data
* Output:
* Describe: set a stop bit when sending the first byte,
  			Reg_data shoule be saved in TxData[0]
****************************************************************/
void I2C_Txbyte(unsigned char Reg_addr, unsigned char Reg_data)
{
	/* wait until I2C bus is idle */
    while(i2c_flag_get(I2C1, I2C_FLAG_I2CBSY));

    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C1);

    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C1, LCD_ADDR, I2C_TRANSMITTER);

    /* wait until ADDSEND bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_ADDSEND));

    /* clear the ADDSEND bit */
    i2c_flag_clear(I2C1,I2C_FLAG_ADDSEND);

    /* wait until the transmit data buffer is empty */
    while(SET != i2c_flag_get(I2C1, I2C_FLAG_TBE));

    /* send the EEPROM's internal address to write to : only one byte address */
    i2c_data_transmit(I2C1, Reg_addr);

    /* wait until BTC bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send the byte to be written */
    i2c_data_transmit(I2C1, Reg_data);

    /* wait until BTC bit is set */
    while(!i2c_flag_get(I2C1, I2C_FLAG_BTC));

    /* send a stop condition to I2C bus */
    i2c_stop_on_bus(I2C1);

    /* wait until the stop condition is finished */
    while(I2C_CTL0(I2C1)&0x0200);


}
void I2C_Rxbyte(unsigned char Reg_addr)
{
	// _DINT(); // Disable all int(important - resolve the conflict between OLED and Matrix keyboard)
	// while (UCB0STAT & UCBBUSY)
	// 	;
	// I2C_WriteMode();
	// TxData[0] = Reg_addr;
	// TXByteCtr = 1;					 // Load TX byte counter
	// UCB0CTL1 |= UCTXSTT;			 // I2C TX, start condition
	// __bis_SR_register(CPUOFF + GIE); // Enter LPM0 w/ interrupts
	// while (UCB0CTL1 & UCTXSTP)
	// 	;
	// I2C_ReadMode();
	// RXByteCtr = 1;
	// UCB0CTL1 |= UCTXSTT;
	// __bis_SR_register(CPUOFF + GIE);
	// while (IFG2 & UCB0RXIFG)
	// 	;
	// while (UCB0CTL1 & UCTXSTP)
	// 	;
	// _EINT(); //enable interrupt(important - resolve the conflict between OLED and Matrix keyboard)
}

void WriteCmd(unsigned char I2C_Command) // write command
{
	// UCB0I2CSA = LCD_ADDR;
	I2C_Txbyte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data) // write data
{
	// UCB0I2CSA = LCD_ADDR;
	I2C_Txbyte(0x40, I2C_Data);
}

// ========== LCD function from here
void I2C_LCD_Init()
{
	Init_I2C();
	// Init LCD
	// UCB0I2CSA = LCD_ADDR; // For LCD address
	delay_while_us(100000);

	WriteCmd(0xAE); //display off
	WriteCmd(0x20); //Set Memory Addressing Mode
	WriteCmd(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); //Set Page Start Address for Page Addressing Mode,0-7 (128x64)
	WriteCmd(0xc0); //上下镜像: COM scan COM0->COM[N-1] (was 0xC8)
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //adjust brightness: 0x00~0xff
	WriteCmd(0xa0); //左右镜像: SEG0 maps to col0 (was 0xA1)
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //128x64: MUX=64 -> ratio=0x3F
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12); //128x64: alternative COM
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}

// ========== OTHER oled function from : OLED_I2C.c
void OLED_Fill(unsigned char fill_Data) //fill full screen
{
	unsigned char m, n;
	for (m = 0; m < OLED_PAGES; m++)
	{
		WriteCmd(0xb0 + m); //page0-page7 (128x64)
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for (n = 0; n < OLED_WIDTH; n++)
		{
			WriteDat(fill_Data);
		}
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : OLED wake up
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D); //set the charge pump
	WriteCmd(0X14); //turn on the charge pump
	WriteCmd(0XAF); //OLED wake up
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : OLED sleep -- In sleep mode, the power consumption of OLED is less than 10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D); //���õ�ɱ�
	WriteCmd(0X10); //�رյ�ɱ�
	WriteCmd(0XAE); //OLED����
}

void OLED_CLS(void) //clear the full screen
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- starting point coords(x:0~127, y:0~7); ch[] -- the string displayed; TextSize -- the string size(1:6*8 ; 2:8*16)
// Description    : display the char in codetab.h, size: 6*8 or 8*16
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0, i = 0, j = 0;
	switch (TextSize)
	{
	case 1:
	{
		while (ch[j] != '\0')
		{
			c = ch[j] - 32;
			if (x > 126)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x, y);
			for (i = 0; i < 6; i++)
				WriteDat(F6x8[c][i]);
			x += 6;
			j++;
		}
	}
	break;
	case 2:
	{
		while (ch[j] != '\0')
		{
			c = ch[j] - 32;
			if (x > 120)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++)
				WriteDat(F8X16[c * 16 + i]);
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++)
				WriteDat(F8X16[c * 16 + i + 8]);
			x += 8;
			j++;
		}
	}
	break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowStr_GT22(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Parameters     : x,y -- starting point coords(x:0~127, y:0~7); ch[] -- ASCII string; TextSize -- 1:7*8 ; 2:8*16
// Description    : same flow as OLED_ShowStr, glyph from GT22L16A2Y (竖置横排)
//--------------------------------------------------------------
void OLED_ShowStr_GT22(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char i = 0, j = 0;

	switch (TextSize)
	{
	case 1:
	{
		while (ch[j] != '\0')
		{
			if (x > 120)
			{
				x = 0;
				y++;
			}
			gt22_get_f7x8(ch[j]);
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++)
				WriteDat(GT22_F7x8[i]);
			x += 8;
			j++;
		}
	}
	break;
	case 2:
	{
		while (ch[j] != '\0')
		{
			if (x > 120)
			{
				x = 0;
				y++;
			}
			gt22_get_f8x16(ch[j]);
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++)
				WriteDat(GT22_F8X16[i]);
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++)
				WriteDat(GT22_F8X16[i + 8]);
			x += 8;
			j++;
		}
	}
	break;
	}
}

void OLED_ShowNum(unsigned char x, unsigned char y, unsigned char num, unsigned char TextSize)
{
	unsigned char c = num + 16, i = 0;

	switch (TextSize)
	{
	case 1:
	{

		if (x > 126)
		{
			x = 0;
			y++;
		}
		OLED_SetPos(x, y);
		for (i = 0; i < 6; i++)
			WriteDat(F6x8[c][i]);
		x += 6;
	}
	break;
	case 2:
	{

		if (x > 120)
		{
			x = 0;
			y++;
		}
		OLED_SetPos(x, y);
		for (i = 0; i < 8; i++)
			WriteDat(F8X16[c * 16 + i]);
		OLED_SetPos(x, y + 1);
		for (i = 0; i < 8; i++)
			WriteDat(F8X16[c * 16 + i + 8]);
		x += 8;
	}
	break;
	}
}

void OLED_ShowValue(unsigned char x, unsigned char y, unsigned int Value, unsigned char TextSize)
{

	OLED_ShowNum(x, y, Value / 10000, TextSize);
	x = x + 8;
	OLED_ShowStr(x, y, ".", TextSize);
	x = x + 8;
	OLED_ShowNum(x, y, Value % 10000 / 1000, TextSize);
	x = x + 8;
	OLED_ShowNum(x, y, Value % 10000 % 1000 / 100, TextSize);
	x = x + 8;
	OLED_ShowNum(x, y, Value % 10000 % 1000 % 100 / 10, TextSize);
	x = x + 8;
	//  OLED_ShowNum(x,y,Value%10,TextSize) ;
	//  x=x+8 ;
	if (y == 6)
		OLED_ShowStr(x, y, "A", TextSize);
	else
		OLED_ShowStr(x, y, "V", TextSize);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          :
// Parameters     : x,y -- starting point coords(x:0~127, y:0~7); N:index in codetab.h
// Description    : Chinese characters in codetab.h, size: 16*16
//--------------------------------------------------------------
#if 1
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char n)
{
	unsigned char wm = 0;

	unsigned int adder = 32 * n;
	OLED_SetPos(x, y);
	for (wm = 0; wm < 16; wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x, y + 1);
	for (wm = 0; wm < 16; wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN_GT22(unsigned char x, unsigned char y, unsigned char ch[])
// Parameters     : x,y -- starting point coords(x:0~127, y:0~7);
//                  ch[] -- GB18030/GBK Chinese string (2 bytes per char, e.g. "啊")
// Description    : same layout as OLED_ShowCN, glyph from GT22L16A2Y 16x16
//--------------------------------------------------------------
void OLED_ShowCN_GT22(unsigned char x, unsigned char y, unsigned char ch[])
{
	unsigned char wm = 0;
	unsigned char j = 0;

	while (ch[j] != '\0')
	{
		if (ch[j] < 0x80)
		{
			j++;
			continue;
		}
		if (x > 112)
		{
			x = 0;
			y += 2;
		}
		gt22_get_chn_f16x16(ch[j], ch[j + 1], 0, 0);
		OLED_SetPos(x, y);
		for (wm = 0; wm < 16; wm++)
		{
			WriteDat(GT22_CHN_F16x16[wm]);
		}
		OLED_SetPos(x, y + 1);
		for (wm = 0; wm < 16; wm++)
		{
			WriteDat(GT22_CHN_F16x16[16 + wm]);
		}
		x += 16;
		j += 2;
	}
}

//--------------------------------------------------------------
// Prototype      : unsigned char OLED_ShowHeb_GT22(...)
// Parameters     : uni[] -- Unicode Hebrew (0x0590..) / space 0x0020, 0-terminated
// Returns        : x just after the last glyph (right edge of block)
// Description    : 8x16 Hebrew from GT22; draw RTL (logical string, reverse paint)
//--------------------------------------------------------------
unsigned char OLED_ShowHeb_GT22(unsigned char x, unsigned char y, const unsigned short uni[])
{
	unsigned char i;
	unsigned char n = 0;
	int idx;

	while (uni[n] != 0u) {
		n++;
	}

	for (idx = (int)n - 1; idx >= 0; idx--) {
		if (x > 120) {
			break;
		}
		if (uni[idx] == 0x0020u) {
			gt22_get_f8x16(' ');
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_F8X16[i]);
			}
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_F8X16[i + 8]);
			}
		} else {
			gt22_get_heb_f8x16(uni[idx]);
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_HEB_F8X16[i]);
			}
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_HEB_F8X16[i + 8]);
			}
		}
		x = (unsigned char)(x + 8u);
	}

	return x;
}

//--------------------------------------------------------------
// Prototype      : unsigned char OLED_ShowArb_GT22(...)
// Parameters     : x -- right inset (0 = flush to right edge);
//                  uni[] -- Unicode Arabic / space 0x0020, 0-terminated
// Returns        : leftmost x of the painted Arabic block
// Description    : right-aligned; reverse logical order (LTR paint)
//--------------------------------------------------------------
unsigned char OLED_ShowArb_GT22(unsigned char x, unsigned char y, const unsigned short uni[])
{
	unsigned char i;
	unsigned char n = 0;
	unsigned char w;
	unsigned char total = 0;
	unsigned char start;
	unsigned char pen;
	int idx;

	while (uni[n] != 0u) {
		n++;
	}

	for (idx = 0; idx < (int)n; idx++) {
		if (uni[idx] == 0x0020u) {
			total = (unsigned char)(total + 8u);
			continue;
		}
		gt22_get_arb_f16(uni[idx]);
		w = GT22_ARB_F16[1];
		if (w == 0u || w > 16u) {
			w = 16u;
		}
		total = (unsigned char)(total + w);
	}

	if (total >= (OLED_WIDTH - x)) {
		start = 0;
	} else {
		start = (unsigned char)(OLED_WIDTH - x - total);
	}

	pen = start;
	/* reverse logical order, paint left → right */
	for (idx = (int)n - 1; idx >= 0; idx--) {
		if (uni[idx] == 0x0020u) {
			gt22_get_f8x16(' ');
			OLED_SetPos(pen, y);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_F8X16[i]);
			}
			OLED_SetPos(pen, y + 1);
			for (i = 0; i < 8; i++) {
				WriteDat(GT22_F8X16[i + 8]);
			}
			pen = (unsigned char)(pen + 8u);
			continue;
		}

		gt22_get_arb_f16(uni[idx]);
		w = GT22_ARB_F16[1];
		if (w == 0u || w > 16u) {
			w = 16u;
		}
		if ((unsigned short)pen + w > OLED_WIDTH) {
			break;
		}
		OLED_SetPos(pen, y);
		for (i = 0; i < w; i++) {
			WriteDat(GT22_ARB_F16[2 + i]);
		}
		OLED_SetPos(pen, y + 1);
		for (i = 0; i < w; i++) {
			WriteDat(GT22_ARB_F16[18 + i]);
		}
		pen = (unsigned char)(pen + w);
	}

	return start;
}
#endif

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- starting point coords(x0:0~127, y0:0~7); x1,y1 -- end coords(x1:1~128,y1:1~8)
// Description    : display BMP bitmap
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		OLED_SetPos(x0, y);
		for (x = x0; x < x1; x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}

void OLED_SetPos(unsigned char x, unsigned char y) // set start column/page
{
	WriteCmd(0xb0 + y);
	WriteCmd(((x & 0xf0) >> 4) | 0x10); // higher column: 0x10 | (x>>4)
	WriteCmd(x & 0x0f);                 // lower column: 0x00 | (x&0x0f); do NOT |0x01
}

