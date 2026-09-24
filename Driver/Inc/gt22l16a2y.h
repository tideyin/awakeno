// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef _GT22L16A2Y_DRV_H
#define _GT22L16A2Y_DRV_H

#include "gd32e10x.h"
#include "GT22L16A2Y-LIB.h"

/*******************************************************************************
 * GT22L16A2Y board driver + Genitop MindCraft lib wrapper
 *
 * SPI0: PA5=SCK, PA6=MISO, PA7=MOSI
 * CS  : PC5 = /GB_LIB_CS
 *
 * Genitop .a handles BaseAdd / address map internally.
 * App uses character codes (or Sequence for zz_zf), not chip addresses.
 *
 * Naming: GT22_ / GT22_<LANG>_ buffers (fontlib style)
 *******************************************************************************/

/* glyph buffer sizes — match Genitop readme / API */
#define GT22_SIZE_F5X7           8
#define GT22_SIZE_F7X8           8
#define GT22_SIZE_F6X12          12
#define GT22_SIZE_F12_PROP       26
#define GT22_SIZE_F8X16          16
#define GT22_SIZE_F16_PROP       34
#define GT22_SIZE_F12X24         48
#define GT22_SIZE_F24_PROP       74
#define GT22_SIZE_F16X32         64
#define GT22_SIZE_F32_PROP       130
#define GT22_SIZE_F16X16         32
#define GT22_SIZE_THA            50

#define GT22_SIZE_NUM_11X16      24
#define GT22_SIZE_NUM_18X24      56
#define GT22_SIZE_NUM_22X32      90
#define GT22_SIZE_NUM_34X48      206
#define GT22_SIZE_NUM_40X64      322
#define GT22_SIZE_CLK_20X24      62
#define GT22_SIZE_CLK_24X32      98
#define GT22_SIZE_CLK_34X48      206
#define GT22_SIZE_CLK_48X64      386
#define GT22_SIZE_BLK_816        18
#define GT22_SIZE_BLK_1624       50
#define GT22_SIZE_BLK_1632       66
#define GT22_SIZE_BLK_2448       146
#define GT22_SIZE_BLK_3264       258
#define GT22_SIZE_UI_F32         130

/* CS */
#define GT22_CS_PORT             GPIOC
#define GT22_CS_PIN              GPIO_PIN_5
#define GT22_CS_LOW()            gpio_bit_reset(GT22_CS_PORT, GT22_CS_PIN)
#define GT22_CS_HIGH()           gpio_bit_set(GT22_CS_PORT, GT22_CS_PIN)
#define GT22_FLASH_CS_PORT       GPIOC
#define GT22_FLASH_CS_PIN        GPIO_PIN_4

/* -------- ASCII -------- */
extern unsigned char GT22_F5x7[GT22_SIZE_F5X7];
extern unsigned char GT22_F7x8[GT22_SIZE_F7X8];
extern unsigned char GT22_F6x12[GT22_SIZE_F6X12];
extern unsigned char GT22_F12_A[GT22_SIZE_F12_PROP];
extern unsigned char GT22_F12_T[GT22_SIZE_F12_PROP];
extern unsigned char GT22_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_F16_A[GT22_SIZE_F16_PROP];
extern unsigned char GT22_F16_T[GT22_SIZE_F16_PROP];
extern unsigned char GT22_F12X24[GT22_SIZE_F12X24];
extern unsigned char GT22_F24[GT22_SIZE_F24_PROP];
extern unsigned char GT22_F16X32[GT22_SIZE_F16X32];
extern unsigned char GT22_F32[GT22_SIZE_F32_PROP];

/* -------- CJK -------- */
extern unsigned char GT22_CHN_F16x16[GT22_SIZE_F16X16];
extern unsigned char GT22_BIG5_F16x16[GT22_SIZE_F16X16];
extern unsigned char GT22_JAP_F16x16[GT22_SIZE_F16X16];
extern unsigned char GT22_JAP_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_KOR_F16x16[GT22_SIZE_F16X16];

/* -------- UNICODE -------- */
extern unsigned char GT22_LAT_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_LAT_F16[GT22_SIZE_F16_PROP];
extern unsigned char GT22_CYR_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_CYR_F16[GT22_SIZE_F16_PROP];
extern unsigned char GT22_GRE_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_GRE_F16[GT22_SIZE_F16_PROP];
extern unsigned char GT22_HEB_F8X16[GT22_SIZE_F8X16];
extern unsigned char GT22_ARB_F16[GT22_SIZE_F16_PROP];
extern unsigned char GT22_THA_F24[GT22_SIZE_THA];

/* -------- 专用数字 圆角(A) / 线型(T) -------- */
extern unsigned char GT22_NUM_ROUND_F16[GT22_SIZE_NUM_11X16];
extern unsigned char GT22_NUM_ROUND_F24[GT22_SIZE_NUM_18X24];
extern unsigned char GT22_NUM_ROUND_F32[GT22_SIZE_NUM_22X32];
extern unsigned char GT22_NUM_ROUND_F48[GT22_SIZE_NUM_34X48];
extern unsigned char GT22_NUM_ROUND_F64[GT22_SIZE_NUM_40X64];
extern unsigned char GT22_NUM_LINE_F16[GT22_SIZE_NUM_11X16];
extern unsigned char GT22_NUM_LINE_F24[GT22_SIZE_NUM_18X24];
extern unsigned char GT22_NUM_LINE_F32[GT22_SIZE_NUM_22X32];
extern unsigned char GT22_NUM_LINE_F48[GT22_SIZE_NUM_34X48];
extern unsigned char GT22_NUM_LINE_F64[GT22_SIZE_NUM_40X64];

/* -------- 时钟体 / 方块体 / UI -------- */
extern unsigned char GT22_NUM_CLOCK_F24[GT22_SIZE_CLK_20X24];
extern unsigned char GT22_NUM_CLOCK_F32[GT22_SIZE_CLK_24X32];
extern unsigned char GT22_NUM_CLOCK_F48[GT22_SIZE_CLK_34X48];
extern unsigned char GT22_NUM_CLOCK_F64[GT22_SIZE_CLK_48X64];
extern unsigned char GT22_NUM_BLOCK_F16[GT22_SIZE_BLK_816];
extern unsigned char GT22_NUM_BLOCK_F24[GT22_SIZE_BLK_1624];
extern unsigned char GT22_NUM_BLOCK_F32[GT22_SIZE_BLK_1632];
extern unsigned char GT22_NUM_BLOCK_F48[GT22_SIZE_BLK_2448];
extern unsigned char GT22_NUM_BLOCK_F64[GT22_SIZE_BLK_3264];
extern unsigned char GT22_UI_F32[GT22_SIZE_UI_F32];

/* board SPI + Genitop init (calls GT_Font_Init) */
void gt22l16a2y_init(void);
int  gt22l16a2y_font_init(void);

/* low-level SPI (also used internally) */
void gt22l16a2y_read(uint32_t addr, uint8_t *buf, uint16_t len);
void gt22l16a2y_sleep(void);
void gt22l16a2y_wake(void);

/* required by Genitop .a — do not rename */
unsigned char gt_read_data(unsigned char *sendbuf, unsigned char sendlen,
			   unsigned char *receivebuf, unsigned int receivelen);

/* ---- fill GT22_* via Genitop API (no BaseAdd) ---- */
void gt22_get_f5x7(uint8_t ascii);
void gt22_get_f7x8(uint8_t ascii);
void gt22_get_f6x12(uint8_t ascii);
void gt22_get_f12_a(uint8_t ascii);
void gt22_get_f12_t(uint8_t ascii);
void gt22_get_f8x16(uint8_t ascii);
void gt22_get_f16_a(uint8_t ascii);
void gt22_get_f16_t(uint8_t ascii);
void gt22_get_f12x24(uint8_t ascii);
void gt22_get_f24(uint8_t ascii);
void gt22_get_f16x32(uint8_t ascii);
void gt22_get_f32(uint8_t ascii);

/* GB18030: 2-byte use c3=c4=0; 4-byte rare chars set all four */
void gt22_get_chn_f16x16(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4);
void gt22_get_big5_f16x16(uint8_t h, uint8_t l);
void gt22_get_jap_f16x16(uint8_t msb, uint8_t lsb);
void gt22_get_jap_f8x16(uint8_t code);
void gt22_get_kor_f16x16(uint8_t msb, uint8_t lsb);

void gt22_get_lat_f8x16(uint16_t code);
void gt22_get_lat_f16(uint16_t code);
void gt22_get_cyr_f8x16(uint16_t code);
void gt22_get_cyr_f16(uint16_t code);
void gt22_get_gre_f8x16(uint16_t code);
void gt22_get_gre_f16(uint16_t code);
void gt22_get_heb_f8x16(uint16_t code);
void gt22_get_arb_f16(uint16_t code);
void gt22_get_tha_f24(uint16_t code);

/* Sequence: 1..N per Genitop zz_zf */
void gt22_get_num_round_f16(uint8_t seq);
void gt22_get_num_round_f24(uint8_t seq);
void gt22_get_num_round_f32(uint8_t seq);
void gt22_get_num_round_f48(uint8_t seq);
void gt22_get_num_round_f64(uint8_t seq);
void gt22_get_num_line_f16(uint8_t seq);
void gt22_get_num_line_f24(uint8_t seq);
void gt22_get_num_line_f32(uint8_t seq);
void gt22_get_num_line_f48(uint8_t seq);
void gt22_get_num_line_f64(uint8_t seq);
void gt22_get_num_clock_f24(uint8_t seq);
void gt22_get_num_clock_f32(uint8_t seq);
void gt22_get_num_clock_f48(uint8_t seq);
void gt22_get_num_clock_f64(uint8_t seq);
void gt22_get_num_block_f16(uint8_t seq);
void gt22_get_num_block_f24(uint8_t seq);
void gt22_get_num_block_f32(uint8_t seq);
void gt22_get_num_block_f48(uint8_t seq);
void gt22_get_num_block_f64(uint8_t seq);
void gt22_get_ui_f32(uint8_t seq);

/* self-check: ASCII 'A' 8x16 vs datasheet pattern; 1=ok */
uint8_t gt22l16a2y_verify_ascii_A(void);

#endif /* _GT22L16A2Y_DRV_H */
