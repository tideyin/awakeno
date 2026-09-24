// ******************************************************
// * Copyright (c) 2026 RayzerLink. Licensed under MIT. *
// *                        website: www.rayzerlink.com *
// *               slack/email: tide.yin@rayzerlink.com *
// *                           linkedin/wechat: tideyin *
// ******************************************************

#ifndef _SPI_FLASH_H
#define _SPI_FLASH_H

#include "gd32e10x.h"

/*******************************************************************************
 * W25Q64JVSSIQ Quad SPI Flash (Awakeno)
 *
 * SPI0 hardware Quad (1-1-4):
 *   PA5 = SCK, PA7 = IO0(MOSI), PA6 = IO1(MISO)
 *   PA2 = IO2, PA3 = IO3
 *   PC4 = /SPI_FLASH_CS
 *
 * Cmd/addr on 1 line; data on 4 lines (0x6B read / 0x32 program).
 * Capacity: 8 MByte, page 256B, sector 4KB
 *******************************************************************************/

#define SPI_FLASH_SIZE           (8u * 1024u * 1024u)
#define SPI_FLASH_PAGE_SIZE      256u
#define SPI_FLASH_SECTOR_SIZE    4096u

#define SPI_FLASH_CS_PORT        GPIOC
#define SPI_FLASH_CS_PIN         GPIO_PIN_4
#define SPI_FLASH_CS_LOW()       gpio_bit_reset(SPI_FLASH_CS_PORT, SPI_FLASH_CS_PIN)
#define SPI_FLASH_CS_HIGH()      gpio_bit_set(SPI_FLASH_CS_PORT, SPI_FLASH_CS_PIN)

/* Winbond JEDEC: manufacturer 0xEF, memory type 0x40, capacity 0x17 (64Mbit) */
#define SPI_FLASH_JEDEC_WINBOND  0xEF4017u

void spi_flash_init(void);
uint32_t spi_flash_read_id(void);

/* erase all 4KB sectors covering [offset, offset+len); len==0 means no-op */
int spi_flash_erase(uint32_t offset, uint32_t len);

/* program any length; does NOT erase — call spi_flash_erase first */
int spi_flash_write(uint32_t offset, const uint8_t *buf, uint32_t len);

/* read any length from offset */
int spi_flash_read(uint32_t offset, uint8_t *buf, uint32_t len);

/* erase entire chip */
int spi_flash_chip_erase(void);

#endif /* _SPI_FLASH_H */
