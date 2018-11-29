// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2014 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 */

#include <common.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/mtd/mtd.h>
#include <spi_flash.h>

static bool sf_mtd_registered;
static char sf_mtd_name[8];

static int spi_flash_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	int err;

	if (!mtd || !mtd->priv)
		return -ENODEV;

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

	if (!mtd || !mtd->priv)
		return -ENODEV;

	err = mtd->_read(mtd, from, len, retlen, buf);
	if (!err)
		*retlen = len;

	return err;
}

static int spi_flash_mtd_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	int err;

	if (!mtd || !mtd->priv)
		return -ENODEV;

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
	int ret;

	if (sf_mtd_registered) {
		ret = del_mtd_device(mtd);
		if (ret)
			return ret;

		sf_mtd_registered = false;
	}

	sf_mtd_registered = false;
	sprintf(sf_mtd_name, "nor%d", spi_flash_mtd_number());

	mtd->name = sf_mtd_name;
	mtd->_erase = spi_flash_mtd_erase;
	mtd->_read = spi_flash_mtd_read;
	mtd->_write = spi_flash_mtd_write;
	mtd->_sync = spi_flash_mtd_sync;

	/* Only uniform flash devices for now */
	mtd->numeraseregions = 0;

	ret = add_mtd_device(mtd);
	if (!ret)
		sf_mtd_registered = true;

	return ret;
}

void spi_flash_mtd_unregister(struct spi_flash *flash)
{
	struct mtd_info *mtd = &flash->mtd;
	int ret;

	if (!sf_mtd_registered)
		return;

	ret = del_mtd_device(mtd);
	if (!ret) {
		sf_mtd_registered = false;
		return;
	}

	/*
	 * Setting mtd->priv to NULL is the best we can do. Thanks to that,
	 * the MTD layer can still call mtd hooks without risking a
	 * use-after-free bug. Still, things should be fixed to prevent the
	 * spi_flash object from being destroyed when del_mtd_device() fails.
	 */
	mtd->priv = NULL;
	printf("Failed to unregister MTD %s and the spi_flash object is going away: you're in deep trouble!",
	       sf_mtd_name);
}
