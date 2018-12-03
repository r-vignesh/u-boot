# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

obj-$(CONFIG_DM_SPI_FLASH) += sf-uclass.o
spi-flash-y := sf_probe.o spi-nor-ids.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_SPI_BOOT)	+= fsl_espi_spl.o
ifeq ($(CONFIG_SPL_SPI_FLASH_TINY),y)
spi-flash-y += spi-nor-tiny.o
else
spi-flash-y += spi-nor.o
endif
else
spi-flash-y += spi-nor.o
endif

obj-$(CONFIG_SPI_FLASH) += spi-flash.o
obj-$(CONFIG_SPI_FLASH_DATAFLASH) += sf_dataflash.o sf.o
obj-$(CONFIG_SPI_FLASH_MTD) += sf_mtd.o
obj-$(CONFIG_SPI_FLASH_SANDBOX) += sandbox.o
