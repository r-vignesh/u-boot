// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2014 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 */

#include <common.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/mtd/mtd.h>
#include <spi_flash.h>

static char sf_mtd_name[8];

static int spi_flash_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int err;

	instr->state = MTD_ERASING;

	err = mtd->_erase(mtd, instr);
	if (err) {
		instr->state = MTD_ERASE_FAILED;
		instr->fail_addr = MTD_FAIL_ADDR_UNKNOWN;
		return -EIO;
	}

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int spi_flash_mtd_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	int err;

	err = mtd->_read(mtd, from, len, retlen, buf);
	if (!err)
		*retlen = len;

	return err;
}

static int spi_flash_mtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	int err;

	err = mtd->_write(mtd, to, len, retlen, buf);
	if (!err)
		*retlen = len;

	return err;
}

static void spi_flash_mtd_sync(struct mtd_info *mtd)
{
}

static int spi_flash_mtd_number(void)
{
#ifdef CONFIG_SYS_MAX_FLASH_BANKS
	return CONFIG_SYS_MAX_FLASH_BANKS;
#else
	return 0;
#endif
}

int spi_flash_mtd_register(struct spi_flash *flash)
{
	struct mtd_info *mtd = &flash->mtd;
	sprintf(sf_mtd_name, "nor%d", spi_flash_mtd_number());

	mtd->name = sf_mtd_name;
	mtd->_erase = spi_flash_mtd_erase;
	mtd->_read = spi_flash_mtd_read;
	mtd->_write = spi_flash_mtd_write;
	mtd->_sync = spi_flash_mtd_sync;

	/* Only uniform flash devices for now */
	mtd->numeraseregions = 0;

	return add_mtd_device(mtd);
}

void spi_flash_mtd_unregister(struct spi_flash *flash)
{
	del_mtd_device(&flash->mtd);
}
