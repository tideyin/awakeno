/**
 * @file GT22L16A2Y.h
 * @author D.
 * @brief
 * @version 1.2.4
 * @date 2023-09-25 16:36:35
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _GT22L16A2Y_H_
#define _GT22L16A2Y_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/
/* ascii_type */
#define ASCII_5X7              1
#define ASCII_7X8              2
#define ASCII_6X12             3
#define ASCII_12_B_A           4
#define ASCII_12_B_T           5
#define ASCII_8X16             6
#define ASCII_16_A             7
#define ASCII_16_T             8
#define ASCII_12X24            9
#define ASCII_24_B             10
#define ASCII_16X32            11
#define ASCII_32_B             12

/* type of zz_zf */
#define B_11X16_A              13
#define B_18X24_A              14
#define B_22X32_A              15
#define B_34X48_A              16
#define B_40X64_A              17
#define B_11X16_T              18
#define B_18X24_T              19
#define B_22X32_T              20
#define B_34X48_T              21
#define B_40X64_T              22
#define T_FONT_20X24           23
#define T_FONT_24X32           24
#define T_FONT_34X48           25
#define T_FONT_48X64           26
#define F_FONT_816             27
#define F_FONT_1624            28
#define F_FONT_1632            29
#define F_FONT_2448            30
#define F_FONT_3264            31
#define KCD_UI_32              32



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/* ---------------------------------------------------------------------------------------------------- *\
 * @brief step 1 ：Reference routine, implement the following functions.
 *                 参考例程，实现以下函数
 * ---------------------------------------------------------------------------------------------------- */
/* 外部函数声明 */
/**
 * @brief 发送读取函数
 *
 * @param sendbuf 发送数据的buff
 * @param sendlen 发送数据长度
 * @param receivebuf 读取数据的buff
 * @param receivelen 读取数据长度
 */
extern unsigned char gt_read_data(unsigned char *sendbuf, unsigned char sendlen, unsigned char *receivebuf, unsigned int receivelen);


/* ---------------------------------------------------------------------------------------------------- *\
 * @brief step 2 ：Initialization 初始化
 * ---------------------------------------------------------------------------------------------------- */
/**
 * @brief 字库初始化
 *          Follow system initialization and place before font library calls.
 *          随系统初始化，置于spi初始化成功之后，字库调用之前
 * @return Initialization Result : if (0 >= ret_val) fail, else success.
 *          初始化结果：如果 (0 >= ret_val) 失败；其余值为成功
 */
int GT_Font_Init(void);


/* ---------------------------------------------------------------------------------------------------- *\
 * @brief step 3 ：Functions for font libraries. 字库调用函数
 * ---------------------------------------------------------------------------------------------------- */

unsigned char ASCII_GetData(unsigned char ascii_code, unsigned long ascii_type, unsigned char *dz_data);
unsigned long GB18030_16_GetData(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4, unsigned char *DZ_Data);
unsigned long JIS0208_16X16_GetData(unsigned char MSB, unsigned char LSB, unsigned char *DZ_Data);
unsigned long KSC5601_F_16_GetData(unsigned char MSB, unsigned char LSB, unsigned char *DZ_Data);
unsigned long Shift_Jis_8X16_GetData(unsigned char FontCode, unsigned char *DZ_Data);
unsigned long LATIN_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long CYRILLIC_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long GREECE_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long HEBREW_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long LATIN_B_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long CYRILLIC_B_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long GREECE_B_GetData(unsigned int FontCode, unsigned char *DZ_Data);
unsigned long ALB_B_GetData(unsigned int unicode_alb, unsigned char *DZ_Data);
unsigned long THAILAND_GetData(unsigned int FontCode, unsigned char *DZ_Data);

unsigned int U2G(unsigned int unicode);
unsigned int BIG52GBK(unsigned char h, unsigned char l);
unsigned int U2J(unsigned short unicode);
unsigned int U2K(unsigned short unicode);
unsigned int SJIS2JIS(unsigned short sj_code);
unsigned char zz_zf(unsigned char Sequence, unsigned char type, unsigned char *DZ_Data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT22L16A2Y_H_

