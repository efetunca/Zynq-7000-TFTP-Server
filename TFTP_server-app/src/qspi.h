/*
 * qspi.h
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#ifndef SRC_QSPI_H_
#define SRC_QSPI_H_

#include <string.h>
#include <stdlib.h>

#include "xparameters.h"
#include "xil_printf.h"
#include "xqspips.h"
#include "ff.h"
#include "sleep.h"

#define QSPI_DEVICE_ID		XPAR_XQSPIPS_0_DEVICE_ID

#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD			0x02
#define READ_CMD			0x03
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define SEC_ERASE_CMD		0xD8
#define READ_ID				0x9F

#define COMMAND_OFFSET		0
#define ADDRESS_1_OFFSET	1
#define ADDRESS_2_OFFSET	2
#define ADDRESS_3_OFFSET	3
#define DATA_OFFSET			4
#define DUMMY_OFFSET		4
#define DUMMY_SIZE			1
#define RD_ID_SIZE			4
#define BULK_ERASE_SIZE		1
#define SEC_ERASE_SIZE		4
#define OVERHEAD_SIZE		4

#define SECTOR_SIZE			65536
#define PAGE_SIZE			256

#define NUM_SECTORS			256
#define NUM_SUBSECTORS		NUM_SECTORS * 16
#define NUM_PAGES			NUM_SUBSECTORS * 16

#define FILE_DATA_ADDR_TX	0x20000000
#define FILE_DATA_ADDR_RX	0x30000000
#define BACKUP_BOOT_ADDR	0x800000

u8 ReadBuffer[6000000];
u8 WriteBuffer[6000000];

int doQspiFlash(const char *fname);

#endif /* SRC_QSPI_H_ */
