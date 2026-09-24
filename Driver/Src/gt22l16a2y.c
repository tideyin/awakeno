// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "gt22l16a2y.h"

#define GT22_CMD_READ   0x03
#define GT22_CMD_SLEEP  0xB9
#define GT22_CMD_WAKE   0xAB
#define GT22_DUMMY_BYTE 0xA5

/* datasheet §7: "A" 8x16 Y (15 listed; pad to 16) */
static const unsigned char gt22_ref_ascii_A_8x16[GT22_SIZE_F8X16] = {
	0x00, 0x80, 0x70, 0x08, 0x70, 0x80, 0x00, 0x00,
	0x3C, 0x03, 0x02, 0x02, 0x02, 0x03, 0x3C, 0x00
};

unsigned char GT22_F5x7[GT22_SIZE_F5X7];
unsigned char GT22_F7x8[GT22_SIZE_F7X8];
unsigned char GT22_F6x12[GT22_SIZE_F6X12];
unsigned char GT22_F12_A[GT22_SIZE_F12_PROP];
unsigned char GT22_F12_T[GT22_SIZE_F12_PROP];
unsigned char GT22_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_F16_A[GT22_SIZE_F16_PROP];
unsigned char GT22_F16_T[GT22_SIZE_F16_PROP];
unsigned char GT22_F12X24[GT22_SIZE_F12X24];
unsigned char GT22_F24[GT22_SIZE_F24_PROP];
unsigned char GT22_F16X32[GT22_SIZE_F16X32];
unsigned char GT22_F32[GT22_SIZE_F32_PROP];

unsigned char GT22_CHN_F16x16[GT22_SIZE_F16X16];
unsigned char GT22_BIG5_F16x16[GT22_SIZE_F16X16];
unsigned char GT22_JAP_F16x16[GT22_SIZE_F16X16];
unsigned char GT22_JAP_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_KOR_F16x16[GT22_SIZE_F16X16];

unsigned char GT22_LAT_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_LAT_F16[GT22_SIZE_F16_PROP];
unsigned char GT22_CYR_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_CYR_F16[GT22_SIZE_F16_PROP];
unsigned char GT22_GRE_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_GRE_F16[GT22_SIZE_F16_PROP];
unsigned char GT22_HEB_F8X16[GT22_SIZE_F8X16];
unsigned char GT22_ARB_F16[GT22_SIZE_F16_PROP];
unsigned char GT22_THA_F24[GT22_SIZE_THA];

unsigned char GT22_NUM_ROUND_F16[GT22_SIZE_NUM_11X16];
unsigned char GT22_NUM_ROUND_F24[GT22_SIZE_NUM_18X24];
unsigned char GT22_NUM_ROUND_F32[GT22_SIZE_NUM_22X32];
unsigned char GT22_NUM_ROUND_F48[GT22_SIZE_NUM_34X48];
unsigned char GT22_NUM_ROUND_F64[GT22_SIZE_NUM_40X64];
unsigned char GT22_NUM_LINE_F16[GT22_SIZE_NUM_11X16];
unsigned char GT22_NUM_LINE_F24[GT22_SIZE_NUM_18X24];
unsigned char GT22_NUM_LINE_F32[GT22_SIZE_NUM_22X32];
unsigned char GT22_NUM_LINE_F48[GT22_SIZE_NUM_34X48];
unsigned char GT22_NUM_LINE_F64[GT22_SIZE_NUM_40X64];

unsigned char GT22_NUM_CLOCK_F24[GT22_SIZE_CLK_20X24];
unsigned char GT22_NUM_CLOCK_F32[GT22_SIZE_CLK_24X32];
unsigned char GT22_NUM_CLOCK_F48[GT22_SIZE_CLK_34X48];
unsigned char GT22_NUM_CLOCK_F64[GT22_SIZE_CLK_48X64];
unsigned char GT22_NUM_BLOCK_F16[GT22_SIZE_BLK_816];
unsigned char GT22_NUM_BLOCK_F24[GT22_SIZE_BLK_1624];
unsigned char GT22_NUM_BLOCK_F32[GT22_SIZE_BLK_1632];
unsigned char GT22_NUM_BLOCK_F48[GT22_SIZE_BLK_2448];
unsigned char GT22_NUM_BLOCK_F64[GT22_SIZE_BLK_3264];
unsigned char GT22_UI_F32[GT22_SIZE_UI_F32];

static uint8_t gt22_spi_xfer(uint8_t byte)
{
	while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_TBE));
	spi_i2s_data_transmit(SPI0, byte);
	while (RESET == spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE));
	return (uint8_t)spi_i2s_data_receive(SPI0);
}

static void gt22_bus_select(void)
{
	gpio_bit_set(GT22_FLASH_CS_PORT, GT22_FLASH_CS_PIN);
	GT22_CS_LOW();
}

static void gt22_bus_deselect(void)
{
	GT22_CS_HIGH();
}

/*!
 * Required by Genitop MindCraft .a
 * sendbuf typically: [0x03, addr23_16, addr15_8, addr7_0]
 */
unsigned char gt_read_data(unsigned char *sendbuf, unsigned char sendlen,
			   unsigned char *receivebuf, unsigned int receivelen)
{
	unsigned int i;

	gt22_bus_select();
	for (i = 0; i < sendlen; i++) {
		gt22_spi_xfer(sendbuf[i]);
	}
	for (i = 0; i < receivelen; i++) {
		receivebuf[i] = gt22_spi_xfer(GT22_DUMMY_BYTE);
	}
	gt22_bus_deselect();
	return 1u;
}

void gt22l16a2y_init(void)
{
	spi_parameter_struct spi_init_struct;

	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_SPI0);

	/* PA5/6/7 AF; PA2/PA3 (SPI0_IO2/3) left for spi_flash Quad */
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,
		  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5);
	gpio_bit_set(GT22_FLASH_CS_PORT, GT22_FLASH_CS_PIN);
	GT22_CS_HIGH();

	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_8;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI0, &spi_init_struct);

	spi_crc_polynomial_set(SPI0, 7);
	spi_enable(SPI0);
}

int gt22l16a2y_font_init(void)
{
	return GT_Font_Init();
}

void gt22l16a2y_read(uint32_t addr, uint8_t *buf, uint16_t len)
{
	unsigned char cmd[4];

	cmd[0] = GT22_CMD_READ;
	cmd[1] = (unsigned char)((addr >> 16) & 0xFF);
	cmd[2] = (unsigned char)((addr >> 8) & 0xFF);
	cmd[3] = (unsigned char)(addr & 0xFF);
	gt_read_data(cmd, 4, buf, len);
}

void gt22l16a2y_sleep(void)
{
	unsigned char cmd = GT22_CMD_SLEEP;
	gt_read_data(&cmd, 1, 0, 0);
}

void gt22l16a2y_wake(void)
{
	unsigned char cmd = GT22_CMD_WAKE;
	gt_read_data(&cmd, 1, 0, 0);
}

/* -------- ASCII via lib -------- */
void gt22_get_f5x7(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_5X7, GT22_F5x7);
}

void gt22_get_f7x8(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_7X8, GT22_F7x8);
}

void gt22_get_f6x12(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_6X12, GT22_F6x12);
}

void gt22_get_f12_a(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_12_B_A, GT22_F12_A);
}

void gt22_get_f12_t(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_12_B_T, GT22_F12_T);
}

void gt22_get_f8x16(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_8X16, GT22_F8X16);
}

void gt22_get_f16_a(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_16_A, GT22_F16_A);
}

void gt22_get_f16_t(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_16_T, GT22_F16_T);
}

void gt22_get_f12x24(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_12X24, GT22_F12X24);
}

void gt22_get_f24(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_24_B, GT22_F24);
}

void gt22_get_f16x32(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_16X32, GT22_F16X32);
}

void gt22_get_f32(uint8_t ascii)
{
	ASCII_GetData(ascii, ASCII_32_B, GT22_F32);
}

/* -------- CJK -------- */
void gt22_get_chn_f16x16(uint8_t c1, uint8_t c2, uint8_t c3, uint8_t c4)
{
	GB18030_16_GetData(c1, c2, c3, c4, GT22_CHN_F16x16);
}

void gt22_get_big5_f16x16(uint8_t h, uint8_t l)
{
	unsigned int gb = BIG52GBK(h, l);
	GB18030_16_GetData((uint8_t)(gb >> 8), (uint8_t)(gb & 0xFF), 0, 0, GT22_BIG5_F16x16);
}

void gt22_get_jap_f16x16(uint8_t msb, uint8_t lsb)
{
	JIS0208_16X16_GetData(msb, lsb, GT22_JAP_F16x16);
}

void gt22_get_jap_f8x16(uint8_t code)
{
	Shift_Jis_8X16_GetData(code, GT22_JAP_F8X16);
}

void gt22_get_kor_f16x16(uint8_t msb, uint8_t lsb)
{
	KSC5601_F_16_GetData(msb, lsb, GT22_KOR_F16x16);
}

/* -------- UNICODE -------- */
void gt22_get_lat_f8x16(uint16_t code)
{
	LATIN_GetData(code, GT22_LAT_F8X16);
}

void gt22_get_lat_f16(uint16_t code)
{
	LATIN_B_GetData(code, GT22_LAT_F16);
}

void gt22_get_cyr_f8x16(uint16_t code)
{
	CYRILLIC_GetData(code, GT22_CYR_F8X16);
}

void gt22_get_cyr_f16(uint16_t code)
{
	CYRILLIC_B_GetData(code, GT22_CYR_F16);
}

void gt22_get_gre_f8x16(uint16_t code)
{
	GREECE_GetData(code, GT22_GRE_F8X16);
}

void gt22_get_gre_f16(uint16_t code)
{
	GREECE_B_GetData(code, GT22_GRE_F16);
}

void gt22_get_heb_f8x16(uint16_t code)
{
	HEBREW_GetData(code, GT22_HEB_F8X16);
}

void gt22_get_arb_f16(uint16_t code)
{
	ALB_B_GetData(code, GT22_ARB_F16);
}

void gt22_get_tha_f24(uint16_t code)
{
	THAILAND_GetData(code, GT22_THA_F24);
}

/* -------- 专用数字 / UI via zz_zf -------- */
void gt22_get_num_round_f16(uint8_t seq)
{
	zz_zf(seq, B_11X16_A, GT22_NUM_ROUND_F16);
}

void gt22_get_num_round_f24(uint8_t seq)
{
	zz_zf(seq, B_18X24_A, GT22_NUM_ROUND_F24);
}

void gt22_get_num_round_f32(uint8_t seq)
{
	zz_zf(seq, B_22X32_A, GT22_NUM_ROUND_F32);
}

void gt22_get_num_round_f48(uint8_t seq)
{
	zz_zf(seq, B_34X48_A, GT22_NUM_ROUND_F48);
}

void gt22_get_num_round_f64(uint8_t seq)
{
	zz_zf(seq, B_40X64_A, GT22_NUM_ROUND_F64);
}

void gt22_get_num_line_f16(uint8_t seq)
{
	zz_zf(seq, B_11X16_T, GT22_NUM_LINE_F16);
}

void gt22_get_num_line_f24(uint8_t seq)
{
	zz_zf(seq, B_18X24_T, GT22_NUM_LINE_F24);
}

void gt22_get_num_line_f32(uint8_t seq)
{
	zz_zf(seq, B_22X32_T, GT22_NUM_LINE_F32);
}

void gt22_get_num_line_f48(uint8_t seq)
{
	zz_zf(seq, B_34X48_T, GT22_NUM_LINE_F48);
}

void gt22_get_num_line_f64(uint8_t seq)
{
	zz_zf(seq, B_40X64_T, GT22_NUM_LINE_F64);
}

void gt22_get_num_clock_f24(uint8_t seq)
{
	zz_zf(seq, T_FONT_20X24, GT22_NUM_CLOCK_F24);
}

void gt22_get_num_clock_f32(uint8_t seq)
{
	zz_zf(seq, T_FONT_24X32, GT22_NUM_CLOCK_F32);
}

void gt22_get_num_clock_f48(uint8_t seq)
{
	zz_zf(seq, T_FONT_34X48, GT22_NUM_CLOCK_F48);
}

void gt22_get_num_clock_f64(uint8_t seq)
{
	zz_zf(seq, T_FONT_48X64, GT22_NUM_CLOCK_F64);
}

void gt22_get_num_block_f16(uint8_t seq)
{
	zz_zf(seq, F_FONT_816, GT22_NUM_BLOCK_F16);
}

void gt22_get_num_block_f24(uint8_t seq)
{
	zz_zf(seq, F_FONT_1624, GT22_NUM_BLOCK_F24);
}

void gt22_get_num_block_f32(uint8_t seq)
{
	zz_zf(seq, F_FONT_1632, GT22_NUM_BLOCK_F32);
}

void gt22_get_num_block_f48(uint8_t seq)
{
	zz_zf(seq, F_FONT_2448, GT22_NUM_BLOCK_F48);
}

void gt22_get_num_block_f64(uint8_t seq)
{
	zz_zf(seq, F_FONT_3264, GT22_NUM_BLOCK_F64);
}

void gt22_get_ui_f32(uint8_t seq)
{
	zz_zf(seq, KCD_UI_32, GT22_UI_F32);
}

uint8_t gt22l16a2y_verify_ascii_A(void)
{
	uint8_t i;

	gt22_get_f8x16('A');
	for (i = 0; i < GT22_SIZE_F8X16; i++) {
		if (GT22_F8X16[i] != gt22_ref_ascii_A_8x16[i]) {
			return 0u;
		}
	}
	return 1u;
}
