// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#include "spi_flash.h"
#include "systick.h"

#define SPI_FLASH_CMD_WREN       0x06u
#define SPI_FLASH_CMD_RDSR1      0x05u
#define SPI_FLASH_CMD_RDSR2      0x35u
#define SPI_FLASH_CMD_WRSR2      0x31u
#define SPI_FLASH_CMD_READ_QUAD  0x6Bu  /* Fast Read Quad Output: 1-1-4 */
#define SPI_FLASH_CMD_PAGE_QUAD  0x32u  /* Quad Input Page Program: 1-1-4 */
#define SPI_FLASH_CMD_SE         0x20u  /* 4KB sector erase */
#define SPI_FLASH_CMD_CE         0xC7u  /* chip erase */
#define SPI_FLASH_CMD_RDID       0x9Fu
#define SPI_FLASH_CMD_RSTEN      0x66u  /* Winbond software reset enable */
#define SPI_FLASH_CMD_RST        0x99u  /* Winbond software reset */

#define SPI_FLASH_SR_WIP         0x01u
#define SPI_FLASH_SR2_QE         0x02u
#define SPI_FLASH_DUMMY          0xFFu

/* soft-reset / debugger leave external flash mid-command — never spin forever */
#define SPI_FLASH_XFER_GUARD     200000u
#define SPI_FLASH_BUSY_GUARD     5000000u

/* shared SPI0 with GT22: keep font CS idle high */
#define SPI_FLASH_GB_CS_PORT     GPIOC
#define SPI_FLASH_GB_CS_PIN      GPIO_PIN_5

static int spi_flash_wait_flag(uint32_t flag, FlagStatus expect, uint32_t guard)
{
	while (guard-- > 0u) {
		if (expect == spi_i2s_flag_get(SPI0, flag)) {
			return 0;
		}
	}
	return -1;
}

static uint8_t spi_flash_xfer(uint8_t byte)
{
	if (0 != spi_flash_wait_flag(SPI_FLAG_TBE, SET, SPI_FLASH_XFER_GUARD)) {
		return 0xFFu;
	}
	spi_i2s_data_transmit(SPI0, byte);
	if (0 != spi_flash_wait_flag(SPI_FLAG_RBNE, SET, SPI_FLASH_XFER_GUARD)) {
		return 0xFFu;
	}
	return (uint8_t)spi_i2s_data_receive(SPI0);
}

static void spi_flash_wait_trans_done(void)
{
	(void)spi_flash_wait_flag(SPI_FLAG_TRANS, RESET, SPI_FLASH_XFER_GUARD);
}

static void spi_flash_select(void)
{
	gpio_bit_set(SPI_FLASH_GB_CS_PORT, SPI_FLASH_GB_CS_PIN);
	SPI_FLASH_CS_LOW();
}

static void spi_flash_deselect(void)
{
	spi_flash_wait_trans_done();
	SPI_FLASH_CS_HIGH();
}

/* leave SPI0 in standard mode; drive IO2/IO3 high when idle */
static void spi_flash_quad_leave(void)
{
	spi_flash_wait_trans_done();
	qspi_disable(SPI0);
	qspi_io23_output_enable(SPI0);
}

static void spi_flash_quad_enter_read(void)
{
	spi_flash_wait_trans_done();
	qspi_read_enable(SPI0);
	qspi_enable(SPI0);
}

static void spi_flash_quad_enter_write(void)
{
	spi_flash_wait_trans_done();
	qspi_write_enable(SPI0);
	qspi_io23_output_enable(SPI0);
	qspi_enable(SPI0);
}

static uint8_t spi_flash_quad_read_byte(void)
{
	if (0 != spi_flash_wait_flag(SPI_FLAG_TBE, SET, SPI_FLASH_XFER_GUARD)) {
		return 0xFFu;
	}
	spi_i2s_data_transmit(SPI0, SPI_FLASH_DUMMY);
	if (0 != spi_flash_wait_flag(SPI_FLAG_RBNE, SET, SPI_FLASH_XFER_GUARD)) {
		return 0xFFu;
	}
	return (uint8_t)spi_i2s_data_receive(SPI0);
}

static void spi_flash_quad_write_byte(uint8_t byte)
{
	if (0 != spi_flash_wait_flag(SPI_FLAG_TBE, SET, SPI_FLASH_XFER_GUARD)) {
		return;
	}
	spi_i2s_data_transmit(SPI0, byte);
	if (0 != spi_flash_wait_flag(SPI_FLAG_RBNE, SET, SPI_FLASH_XFER_GUARD)) {
		return;
	}
	(void)spi_i2s_data_receive(SPI0);
}

static void spi_flash_write_enable(void)
{
	spi_flash_quad_leave();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_WREN);
	spi_flash_deselect();
}

static void spi_flash_wait_busy(void)
{
	uint8_t sr;
	uint32_t guard = SPI_FLASH_BUSY_GUARD;

	spi_flash_quad_leave();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_RDSR1);
	do {
		sr = spi_flash_xfer(SPI_FLASH_DUMMY);
		if (--guard == 0u) {
			break;
		}
	} while (sr & SPI_FLASH_SR_WIP);
	spi_flash_deselect();
}

/*!
 * \brief  recover external W25Q after MCU soft-reset / debugger halt
 *         (chip is NOT reset by SYSRESETREQ; may be stuck in continuous read)
 */
static void spi_flash_recover(void)
{
	uint32_t i;

	spi_flash_quad_leave();
	SPI_FLASH_CS_HIGH();
	gpio_bit_set(SPI_FLASH_GB_CS_PORT, SPI_FLASH_GB_CS_PIN);

	/* clocks with CS high help exit some continuous-read modes */
	for (i = 0u; i < 16u; i++) {
		(void)spi_flash_xfer(SPI_FLASH_DUMMY);
	}

	/* Winbond software reset */
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_RSTEN);
	spi_flash_deselect();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_RST);
	spi_flash_deselect();
	delay_1ms(1);
}

static void spi_flash_send_addr(uint32_t addr)
{
	spi_flash_xfer((uint8_t)((addr >> 16) & 0xFFu));
	spi_flash_xfer((uint8_t)((addr >> 8) & 0xFFu));
	spi_flash_xfer((uint8_t)(addr & 0xFFu));
}

static int spi_flash_check_range(uint32_t offset, uint32_t len)
{
	if (len == 0u) {
		return 0;
	}
	if ((offset >= SPI_FLASH_SIZE) || (len > (SPI_FLASH_SIZE - offset))) {
		return -1;
	}
	return 0;
}

/*!
 * \brief  set Status Register-2 QE bit (required for IO2/IO3 data)
 */
static void spi_flash_enable_qe(void)
{
	uint8_t sr2;

	spi_flash_quad_leave();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_RDSR2);
	sr2 = spi_flash_xfer(SPI_FLASH_DUMMY);
	spi_flash_deselect();

	if (sr2 & SPI_FLASH_SR2_QE) {
		return;
	}

	spi_flash_write_enable();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_WRSR2);
	spi_flash_xfer((uint8_t)(sr2 | SPI_FLASH_SR2_QE));
	spi_flash_deselect();
	spi_flash_wait_busy();
}

/*!
 * \brief  init SPI0 Quad pins + enable Flash QE
 */
void spi_flash_init(void)
{
	spi_parameter_struct spi_init_struct;

	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_SPI0);

	/* PA5=SCK, PA6=IO1, PA7=IO0, PA2=IO2, PA3=IO3 — all AF for SPI0 Quad */
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,
		  GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	/* PC4=Flash CS, PC5=Font CS — both idle high FIRST (abort any open xfer) */
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4 | GPIO_PIN_5);
	SPI_FLASH_CS_HIGH();
	gpio_bit_set(SPI_FLASH_GB_CS_PORT, SPI_FLASH_GB_CS_PIN);

	spi_i2s_deinit(SPI0);
	spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_init_struct.device_mode          = SPI_MASTER;
	spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_init_struct.nss                  = SPI_NSS_SOFT;
	spi_init_struct.prescale             = SPI_PSC_4;
	spi_init_struct.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI0, &spi_init_struct);

	spi_crc_polynomial_set(SPI0, 7);
	qspi_disable(SPI0);
	qspi_io23_output_enable(SPI0);
	spi_enable(SPI0);

	/* soft-reset after debugger/SYSRESET — external chip keeps prior state */
	spi_flash_recover();
	spi_flash_enable_qe();
}

uint32_t spi_flash_read_id(void)
{
	uint32_t id;

	spi_flash_quad_leave();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_RDID);
	id  = ((uint32_t)spi_flash_xfer(SPI_FLASH_DUMMY)) << 16;
	id |= ((uint32_t)spi_flash_xfer(SPI_FLASH_DUMMY)) << 8;
	id |= ((uint32_t)spi_flash_xfer(SPI_FLASH_DUMMY));
	spi_flash_deselect();

	return id;
}

/*!
 * \brief  erase sectors covering [offset, offset+len)
 * \note   hardware erase unit is 4KB; partial sector is fully erased
 */
int spi_flash_erase(uint32_t offset, uint32_t len)
{
	uint32_t addr;
	uint32_t end;

	if (len == 0u) {
		return 0;
	}
	if (spi_flash_check_range(offset, len) != 0) {
		return -1;
	}

	addr = offset & ~(SPI_FLASH_SECTOR_SIZE - 1u);
	end  = offset + len;

	while (addr < end) {
		spi_flash_write_enable();
		spi_flash_select();
		spi_flash_xfer(SPI_FLASH_CMD_SE);
		spi_flash_send_addr(addr);
		spi_flash_deselect();
		spi_flash_wait_busy();
		addr += SPI_FLASH_SECTOR_SIZE;
	}

	return 0;
}

int spi_flash_chip_erase(void)
{
	spi_flash_write_enable();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_CE);
	spi_flash_deselect();
	spi_flash_wait_busy();
	return 0;
}

/*!
 * \brief  Quad Input Page Program (0x32); crosses pages automatically
 */
int spi_flash_write(uint32_t offset, const uint8_t *buf, uint32_t len)
{
	uint32_t page_remain;
	uint32_t chunk;
	uint32_t i;

	if ((buf == 0) || (len == 0u)) {
		return (len == 0u) ? 0 : -1;
	}
	if (spi_flash_check_range(offset, len) != 0) {
		return -1;
	}

	while (len > 0u) {
		page_remain = SPI_FLASH_PAGE_SIZE - (offset % SPI_FLASH_PAGE_SIZE);
		chunk = (len < page_remain) ? len : page_remain;

		spi_flash_write_enable();
		spi_flash_select();
		spi_flash_xfer(SPI_FLASH_CMD_PAGE_QUAD);
		spi_flash_send_addr(offset);

		spi_flash_quad_enter_write();
		for (i = 0; i < chunk; i++) {
			spi_flash_quad_write_byte(buf[i]);
		}
		spi_flash_quad_leave();
		spi_flash_deselect();
		spi_flash_wait_busy();

		offset += chunk;
		buf    += chunk;
		len    -= chunk;
	}

	return 0;
}

/*!
 * \brief  Fast Read Quad Output (0x6B)
 */
int spi_flash_read(uint32_t offset, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	if ((buf == 0) || (len == 0u)) {
		return (len == 0u) ? 0 : -1;
	}
	if (spi_flash_check_range(offset, len) != 0) {
		return -1;
	}

	spi_flash_quad_leave();
	spi_flash_select();
	spi_flash_xfer(SPI_FLASH_CMD_READ_QUAD);
	spi_flash_send_addr(offset);
	spi_flash_xfer(SPI_FLASH_DUMMY); /* 8 dummy clocks */

	spi_flash_quad_enter_read();
	for (i = 0; i < len; i++) {
		buf[i] = spi_flash_quad_read_byte();
	}
	spi_flash_quad_leave();
	spi_flash_deselect();

	return 0;
}
