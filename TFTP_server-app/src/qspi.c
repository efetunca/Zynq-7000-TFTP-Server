/*
 * qspi.c
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#include "qspi.h"

static int FlashReadID(XQspiPs *QspiPtr);
static int FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);
static int FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);
static int FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);
int FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

/*****************************************************************************/
/**
*
* This function calls necessary functions to initialize QSPI Flash.
*
* In order not to encounter any errors, it must be called every time
* before doing any reading and writing operations with QSPI Flash.
*
* @param	QspiInstancePtr is a pointer to the QSPIPS driver to use.
* @param	QspiDeviceId is the XPAR_<QSPIPS_instance>_DEVICE_ID value
*			from xparameters.h.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int QspiFlashInit(XQspiPs *QspiInstancePtr, u16 QspiDeviceId)
{
	XQspiPs_Config *QspiConfig;
	static int status;

	xil_printf("QSPI Flash initialization started...\r\n\n");

	/*
	 * Looks up the device configuration based on the unique device ID.
	 */
	QspiConfig = XQspiPs_LookupConfig(QspiDeviceId);
	if (QspiConfig == NULL) {
		xil_printf("XQspiPs_LookupConfig   : Failed\r\n\n");
		return XST_FAILURE;
	}
	else
		xil_printf("XQspiPs_LookupConfig   : Success\r\n");

	/*
	 * Initializes a specific XQspiPs instance
	 * such that the driver is ready to use.
	 */
	status = XQspiPs_CfgInitialize(QspiInstancePtr, QspiConfig, QspiConfig->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("XQspiPs_CfgInitialize  : Failed\r\n\n");
		return XST_FAILURE;
	}
	else
		xil_printf("XQspiPs_CfgInitialize  : Success\r\n");

	/*
	 * Performing a self-test to check hardware build.
	 */
	status = XQspiPs_SelfTest(QspiInstancePtr);
	if (status != XST_SUCCESS) {
		xil_printf("XQspiPs_SelfTest       : Failed\r\n\n");
		return XST_FAILURE;
	}
	else{
		xil_printf("XQspiPs_SelfTest       : Success\r\n");
	}

	/*
	 * Setting Manual Start and Manual Chip select options
	 * and drive HOLD_B pin high.
	 */
	XQspiPs_SetOptions(QspiInstancePtr, XQSPIPS_MANUAL_START_OPTION |
										XQSPIPS_FORCE_SSELECT_OPTION |
										XQSPIPS_HOLD_B_DRIVE_OPTION);

	/*
	 * Setting the prescaler for QSPI clock.
	 */
	XQspiPs_SetClkPrescaler(QspiInstancePtr, XQSPIPS_CLK_PRESCALE_8);

	/*
	 * Asserting the FLASH chip select.
	 */
	XQspiPs_SetSlaveSelect(QspiInstancePtr);

	/*
	 * Reading the Serial ID that is connected to SPI device.
	 */
	FlashReadID(QspiInstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the contents of the file specified with
* the parameter 'fname' and writes it to the address in the 'FileAddr' pointer.
*
* @param	FileAddr is a pointer that specifies the address where
* 			the read data will be stored.
* @param	fname is a pointer that holds the name which will be read.
*
* @return	NumBytesRead if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
static int readFileFromSd(const char *fname)
{
	static FIL file;
	FILINFO info;
	FRESULT Res;
	UINT NumBytesRead;
	u8 *FileAddr;

	FileAddr = (u8 *)FILE_DATA_ADDR_TX;

	/*
	 * Opening the file with the name 'fname' and
	 * creating a file object named 'file'.
	 *
	 * The file will be opened in read-only mode.
	 */
	Res = f_open(&file, fname, FA_READ);
	if (Res) {
		xil_printf("ERROR: f_open %d\r\n\n", Res);
		return XST_FAILURE;
	}
	else {
		xil_printf("DONE: f_open\r\n");
	}

	/*
	 * Normally, this function is used to check the existence
	 * of a file. But here, we're using this function to
	 * get informations of the file and save them into
	 * the information object named 'info'.
	 *
	 * We're using only the 'fsize' from that object
	 * in order to get the file size information.
	 */
	Res = f_stat(fname, &info);
	if (Res) {
		xil_printf("ERROR: f_stat %d\r\n\n", Res);
		return XST_FAILURE;
	}
	else
		xil_printf("DONE: f_stat | File size: %d bytes\r\n", info.fsize);

	/*
	 * Opens the file in the file object named 'file',
	 * reads the contents of it till the EOF and
	 * saves the read data to the address
	 * specified with the name 'FileAddr'
	 */
	Res = f_read(&file, FileAddr, EOF, &NumBytesRead);
	if (Res) {
		xil_printf("ERROR: f_read %d\r\n\n", Res);
		return XST_FAILURE;
	}
	else
		xil_printf("DONE: f_read | %d bytes read\r\n", NumBytesRead);

	/*
	 * If the file size information in the 'info' is not equal
	 * to the read bytes, which are stored in 'NumBytesRead',
	 * function returns 'XST_FAILURE'.
	 */
	if (info.fsize != NumBytesRead) {
		xil_printf("ERROR: The NumBytesRead is not equal to the file size!\r\n");
		return XST_FAILURE;
	}

	f_close(&file);
	return NumBytesRead;
}

/*****************************************************************************/
/**
*
* This function gets the data from the specified address and writes them
* to the QSPI Flash memory. In order to do this, it divides entire data
* into sectors and sub-sectors.
*
* @param	QspiInstancePtr is a pointer to the QSPIPS driver to use.
* @param	fsize is the size of the file that will be written to QSPI Flash.
*
* @return	(nPages * nSectors * PAGE_SIZE) + nLast if successful,
* 			else throws the error that is occurred.
*
* @note		None.
*
*****************************************************************************/
static int writeFileToFlash(XQspiPs *QspiInstancePtr, u32 fsize, u32 nSpiAddr, u32 SRC_ADDR, u8 isBackup)
{
	static int status;

	static u32 nPage;
	static u32 nSector;

	static u32 nControlByteAddr = (NUM_SECTORS - 2) * SECTOR_SIZE;

	u32 nOffset = 0, TotalOffset = 0;

	u32 nSectorsToWritten = fsize / SECTOR_SIZE;
	u32 nPagesToWritten = NUM_PAGES / NUM_SECTORS; // 256 pages in each sector
	u32 nInitialSector = nSpiAddr / SECTOR_SIZE;

	/*
	 * Since the number of nSectorsToWritten is an integer,
	 * it does not take into account the remainder when performing
	 * the fsize / nPages operation, so it may result in an
	 * incomplete copy of the data. To avoid this, unwritten bytes
	 * must also be taken into account.
	 */
	u32 nUnwrittenBytes = fsize % (SECTOR_SIZE * nSectorsToWritten);
	u32 nUnwrittenPages = nUnwrittenBytes / PAGE_SIZE;
	u32 nLast = nUnwrittenBytes % PAGE_SIZE;

	if (!isBackup) {
		FlashErase(QspiInstancePtr, nControlByteAddr, SECTOR_SIZE - 1);
		memset(&WriteBuffer[DATA_OFFSET], 0x0B, PAGE_SIZE);
		FlashWrite(QspiInstancePtr, nControlByteAddr, PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * This loop firstly erases nPages of bytes, which equals to
	 * one sector (64 KB), starting from nSpiAddr,
	 * and then enters into a sub-loop to write data to QSPI Flash.
	 */
	for (nSector = 0; nSector < nSectorsToWritten; nSector++) {
		xil_printf("=== Sector #%d ===\r\n", nInitialSector + nSector);

		status = FlashErase(QspiInstancePtr, nSpiAddr, SECTOR_SIZE);
		if (status != XST_SUCCESS) {
			xil_printf("Flash Erase: Failed\r\n\n");
			return status;
		}
		else
			xil_printf("Flash Erase: Success\r\n");

		for (nPage = 0; nPage < nPagesToWritten; nPage++, nOffset += PAGE_SIZE, TotalOffset += PAGE_SIZE) {
			/*
			 * To write data to the QSPI Flash, the original data must be copied
			 * into the WriteBuffer array. The PAGE_SIZE bytes of data is
			 * copied to WriteBuffer array, starting from SRC_ADDR.
			 *
			 * Since nOffset is incremented by PAGE_SIZE each time this sub-loop starts over,
			 * data on the SRC_ADDR can be read PAGE_SIZE bytes at a time, without any loss.
			 */
			memcpy(&WriteBuffer[DATA_OFFSET], (u8 *)(SRC_ADDR + nOffset), PAGE_SIZE);
			usleep(10);
			status = FlashWrite(QspiInstancePtr, nSpiAddr + nOffset, PAGE_SIZE, WRITE_CMD);
			if (status != XST_SUCCESS) {
				xil_printf("Flash Write: Failed\r\n\n");
				return status;
			}
		}
		xil_printf("Flash Write: Success [TotalOffset = %d]\r\n", TotalOffset);
		xil_printf("%d bytes remaining...\r\n\n", fsize - TotalOffset);
		nSpiAddr += SECTOR_SIZE;
		SRC_ADDR += SECTOR_SIZE;
		nOffset = 0;
	}

	/*
	 * If there is any bytes remained unwritten,
	 * the same operations above will be done again for them.
	 */
	if (nUnwrittenBytes) {
		xil_printf("=== Unwritten Bytes [%d bytes] ===\r\n", nUnwrittenBytes);

		status = FlashErase(QspiInstancePtr, nSpiAddr, nUnwrittenBytes);
		if (status != XST_SUCCESS) {
			xil_printf("Flash Erase: Failed\r\n\n");
			return status;
		}
		else
			xil_printf("Flash Erase: Success\r\n");

		for (nPage = 0; nPage < nUnwrittenPages; nPage++, nOffset += PAGE_SIZE, TotalOffset += PAGE_SIZE) {
			memcpy(&WriteBuffer[DATA_OFFSET], (u8 *)(SRC_ADDR + nOffset), PAGE_SIZE);
			usleep(10);
			status = FlashWrite(QspiInstancePtr, nSpiAddr + nOffset, PAGE_SIZE, WRITE_CMD);
			if (status != XST_SUCCESS) {
				xil_printf("Flash Write: Failed\r\n\n", nOffset);
				return status;
			}
		}
		xil_printf("Flash Write: Success [TotalOffset = %d]\r\n\n", TotalOffset);
		nSpiAddr += nOffset;
		SRC_ADDR += nOffset;
		nOffset = 0;
	}

	/*
	 * If there is a last part left that is unwritten,
	 * the write operation will be done.
	 */
	if (nLast) {
		/*
		 * Since the nLast bytes are less then the maximum size of WriteBuffer array,
		 * the value of the bytes of the WriteBuffer are set to 0, excluding
		 * the first nLast bytes.
		 *
		 * This is done by subtracting nLast from PAGE_SIZE.
		 */
		memset(&WriteBuffer[DATA_OFFSET + nLast], 0, PAGE_SIZE - nLast);
		memcpy(&WriteBuffer[DATA_OFFSET], (u8 *)SRC_ADDR, nLast);
		usleep(10);
		status = FlashWrite(QspiInstancePtr, nSpiAddr, PAGE_SIZE, WRITE_CMD);
		TotalOffset += nLast;

		if (status != XST_SUCCESS) {
			xil_printf("Flash Write: Failed [nOffset = %d]\r\n\n", nOffset);
			return status;
		}
		else
			xil_printf("Flash Write: Success [TotalOffset = %d]\r\n\n", TotalOffset);
	}
	xil_printf("Writing completed successfully!\r\n");

	if (!isBackup) {
		FlashErase(QspiInstancePtr, nControlByteAddr, SECTOR_SIZE - 1);
		memset(&WriteBuffer[DATA_OFFSET], 0x0E, PAGE_SIZE);
		FlashWrite(QspiInstancePtr, nControlByteAddr, PAGE_SIZE, WRITE_CMD);
	}

	/*
	 * The returned number indicates how many bytes are written to the QSPI Flash.
	 */
	return ((nSectorsToWritten * nPagesToWritten * PAGE_SIZE) + nUnwrittenBytes);
}

/*****************************************************************************/
/**
*
* This function gets the data from the specified address, reads the data
* on the QSPI Flash and compares them to verify if the data is written correctly.
*
* @param	QspiInstancePtr is a pointer to the QSPIPS driver to use.
* @param	readSize is the size of the data that is written to QSPI Flash.
*
* @return	XST_SUCCESS if successful, else throws the error that is occurred.
*
* @note		None.
*
*****************************************************************************/
static int verifyAfterFlash(XQspiPs *QspiInstancePtr, u32 readSize, u32 Addr)
{
	static int status, index;

	u8 *srcData;
	u8 *destData;

	srcData  = (u8 *)FILE_DATA_ADDR_TX;
	destData = (u8 *)FILE_DATA_ADDR_RX;

	xil_printf("Data on flash is reading...\r\n");

	/*
	 * The FlashRead function reads the data on QSPI Flash
	 * and saves them into the ReadBuffer.
	 */
	status = FlashRead(QspiInstancePtr, Addr, readSize, FAST_READ_CMD);
	if (status != XST_SUCCESS) {
		xil_printf("Reading failed!\r\n\n");
		return status;
	}
	else
		xil_printf("Reading done!\r\n\n");
	/*
	 * To save the read data into the specified address,
	 * the ReadBuffer is copied to the destData variable.
	 */
	memcpy(destData, &ReadBuffer[DATA_OFFSET + DUMMY_SIZE], readSize);

	xil_printf("Data verification starting...\r\n");
	for (index = 0; index < readSize; index++) {
		if (srcData[index] != destData[index]) {
			xil_printf("Data verification failed!\r\n\t[Index: %d, Src = 0x%X, Dest = 0x%X]\r\n",
					index, srcData[index], destData[index]);
			return XST_FAILURE;
		}
	}
	xil_printf("Data verification completed successfully!\r\n");
	return XST_SUCCESS;
}

static int checkFileOnFlash(XQspiPs *QspiInstancePtr)
{
	static int status, index;
	static u8 checkBytesOnFlash[256];
	static u32 addr = (NUM_SECTORS - 2) * SECTOR_SIZE;

	status = FlashRead(QspiInstancePtr, addr, PAGE_SIZE, FAST_READ_CMD);
	if (status != XST_SUCCESS) {
		xil_printf("Reading Failed!\r\n\n");
		return status;
	}

	memcpy(checkBytesOnFlash, &ReadBuffer[DATA_OFFSET + DUMMY_SIZE], PAGE_SIZE);

	for (index = 1; index < PAGE_SIZE; index++) {
		if (checkBytesOnFlash[index] != checkBytesOnFlash[index - 1]) {
			xil_printf("error on index #%d", index);
			return XST_FAILURE;
		}
	}
	return checkBytesOnFlash[0];
}

/*****************************************************************************/
/**
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	QspiInstancePtr is a pointer to the QSPIPS driver to use.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
static int FlashReadID(XQspiPs *QspiPtr)
{
	int status;

	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x00;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x00;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x00;

	status = XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer, RD_ID_SIZE);
	if (status != XST_SUCCESS) {
		xil_printf("XQspiPs_PolledTransfer : Failed\r\n\n");
		return status;
	}
	else
		xil_printf("XQspiPs_PolledTransfer : Success\r\n\n");

	xil_printf("FlashID = 0x%X 0x%X 0x%X\r\n\n", ReadBuffer[1], ReadBuffer[2], ReadBuffer[3]);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial FLASH connected to the QSPI interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to write data to in the FLASH.
* @param	ByteCount contains the number of bytes to write.
* @param	Command is the command used to write data to the flash. QSPI
*			device supports only Page Program command to write data to the
*			flash.
*
* @return	XST_SUCCESS if successful, else throws the error that is occurred.
*
* @note		None.
*
******************************************************************************/
static int FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	int status;
	u8 WriteEnableCmd  = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };
	u8 FlashStatus[2];

	/*
	 * Send the write enable command to the FLASH so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write.
	 */
	status = XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));
	if (status != XST_SUCCESS) {
		xil_printf("FlashWrite XQspiPs_PolledTransfer1: Failed\r\n\n");
		return status;
	}

	/*
	 * Setup the write command with the specified address
	 * and data for the FLASH.
	 */
	WriteBuffer[COMMAND_OFFSET]		= Command;
	WriteBuffer[ADDRESS_1_OFFSET]	= (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET]	= (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET]	= (u8)(Address & 0xFF);

	/*
	 * Send the write command, address, and data to the FLASH to be
	 * written, no receive buffer is specified since there is nothing to receive.
	 */
	status = XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL, ByteCount + OVERHEAD_SIZE);
	if (status != XST_SUCCESS) {
		xil_printf("FlashWrite XQspiPs_PolledTransfer2: Failed\r\n\n");
		return status;
	}

	/*
	 * Wait for the write command to the FLASH to be completed,
	 * it takes some time for the data to be written.
	 */
	while (1) {
		/*
		 * Poll the status register of the FLASH to determine when it completes,
		 * by sending a read status command and receiving the status byte.
		 */
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd));

		/*
		 * If the status indicates the write is done, then stop waiting,
		 * if a value of 0xFF in the status byte is read from the device
		 * and this loop never exits, the device slave select is
		 * possibly incorrect such that the device status is not being read.
		 */
		FlashStatus[1] |= FlashStatus[0];
		if ((FlashStatus[1] & 0x01) == 0)
			break;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads from the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address to read data from in the FLASH.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. QSPI
*			device supports one of the Read, Fast Read, Dual Read and Fast
*			Read commands to read data from the flash.
*
* @return	XST_SUCCESS if successful, else throws the error that is occurred.
*
* @note		None.
*
******************************************************************************/
static int FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	int status;

	/*
	 * Setup the write command with the specified address and
	 * data for the FLASH.
	 */
	WriteBuffer[COMMAND_OFFSET]		= Command;
	WriteBuffer[ADDRESS_1_OFFSET]	= (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET]	= (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET]	= (u8)(Address & 0xFF);

	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) || (Command == QUAD_READ_CMD))
		ByteCount += DUMMY_SIZE;

	memset(ReadBuffer, 0, sizeof(ReadBuffer));

	/*
	 * Send the read command to the FLASH to read the specified number
	 * of bytes from the FLASH, send the read command and address and
	 * receive the specified number of bytes of data in the data buffer.
	 */
	status = XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer, ByteCount + OVERHEAD_SIZE);
	if (status != XST_SUCCESS) {
		xil_printf("FlashRead QspiPs_PolledTransfer: Failed\r\n\n");
		return status;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial FLASH connected to the
* QSPI interface.
*
* @param	QspiPtr is a pointer to the QSPI driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
*
* @return	XST_SUCCESS if successful, else throws the error that is occurred.
*
* @note		None.
*
******************************************************************************/
static int FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	static u32 NumSector;
	static int sector;
	static int status;

	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };
	u8 FlashStatus[2];

	/*
	 * If erase size is same as the total size of the flash,
	 * use bulk erase command.
	 */
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
		xil_printf("Bulk Erase Started...\r\n\n");

		/*
		 * Send the write enable command to the FLASH so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the erase.
		 */
		status = XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));
		if (status != XST_SUCCESS) {
			xil_printf("Flash XQspiPs_PolledTransfer1: Failed\r\n\n");
			return status;
		}

		/* Setup the bulk erase command. */
		WriteBuffer[COMMAND_OFFSET] = BULK_ERASE_CMD;

		/*
		 * Send the bulk erase command; no receive buffer is specified
		 * since there is nothing to receive.
		 */
		status = XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL, BULK_ERASE_SIZE);
		if (status != XST_SUCCESS) {
			xil_printf("Flash XQspiPs_PolledTransfer2: Failed\r\n\n");
			return status;
		}

		/* Wait for the erase command to the FLASH to be completed. */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte.
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting; if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read.
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0)
				break;
		}
		return XST_SUCCESS;
	}

	/* Calculate number of sectors to erase based on byte count. */
	if (ByteCount % SECTOR_SIZE)
		NumSector = ByteCount / SECTOR_SIZE + 1;
	else
		NumSector = ByteCount / SECTOR_SIZE;

	/*
	 * If the erase size is less than the total size of the flash,
	 * use sector erase command.
	 */
	for (sector = 0; sector < NumSector; sector++) {
		/*
		 * Send the write enable command to the SEEPOM so that it can be
		 * written to, this needs to be sent as a separate transfer
		 * before the write.
		 */
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL, sizeof(WriteEnableCmd));

		/*
		 * Setup the write command with the specified address and
		 * data for the FLASH.
		 */
		WriteBuffer[COMMAND_OFFSET]		= SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET]	= (u8)(Address >> 16);
		WriteBuffer[ADDRESS_2_OFFSET]	= (u8)(Address >> 8);
		WriteBuffer[ADDRESS_3_OFFSET]	= (u8)(Address & 0xFF);

		/*
		 * Send the sector erase command and address; no receive buffer
		 * is specified since there is nothing to receive.
		 */
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL, SEC_ERASE_SIZE);

		/* Wait for the sector erase command to the FLASH to be completed. */
		while (1) {
			/*
			 * Poll the status register of the device to determine
			 * when it completes, by sending a read status command
			 * and receiving the status byte.
			 */
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus, sizeof(ReadStatusCmd));

			/*
			 * If the status indicates the write is done, then stop
			 * waiting, if a value of 0xFF in the status byte is
			 * read from the device and this loop never exits, the
			 * device slave select is possibly incorrect such that
			 * the device status is not being read.
			 */
			FlashStatus[1] |= FlashStatus[0];
			if ((FlashStatus[1] & 0x01) == 0)
				break;
		}
		Address += SECTOR_SIZE;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function calls the necessary functions to write a file to the QSPI Flash.
*
* @param	fname is the pointer to the file which will be written.
*
* @return	1 if successful, else 0.
*
* @note		None.
*
******************************************************************************/
int doQspiFlash(const char *fname)
{
	static XQspiPs QspiInstance;
	static int filesize;
	static int status;

	QspiFlashInit(&QspiInstance, QSPI_DEVICE_ID);

	xil_printf("Transfer of \"%s\" file to QSPI Flash started...\r\n\n", fname);

	/*
	 * Reading the file named 'fname' in the SD Card and
	 * saving it to the specified address.
	 *
	 * The returned value is saved in the 'status' variable.
	 */
	status = readFileFromSd(fname);
	if (status == XST_FAILURE) {
		xil_printf("===== Reading file from SD Card failed! =====\r\n");
		return 0;
	}
	else {
		/*
		 * If the function runs without any problems,
		 * then it will return the file size.
		 *
		 * If such a situation occurs, the file size is
		 * saved into the 'filesize' variable.
		 */
		filesize = status;
		xil_printf("===== Reading file from SD Card completed successfully! =====\r\n\n");
	}

	/*
	 * Writing the data, which is read above, to the QSPI Flash.
	 *
	 * The returned value is saved in the 'status' variable.
	 */
	status = writeFileToFlash(&QspiInstance, filesize, 0, FILE_DATA_ADDR_TX, 0);
	if (status == XST_FAILURE) {
		xil_printf("===== Writing file to flash failed! =====\r\n");
		return 0;
	}
	else
		xil_printf("===== Writing file to flash completed successfully! =====\r\n\n");

	/* Checking the data on the flash. */
	status = verifyAfterFlash(&QspiInstance, filesize, 0);
	if (status != XST_SUCCESS) {
		xil_printf("===== Data verification failed! =====\r\n");
		return 0;
	}
	else
		xil_printf("===== Data verification completed successfully! =====\r\n\n");

	xil_printf("The file \"%s\" is successfully written to flash!\r\n", fname);
	xil_printf("Now you can turn off the board to boot from QSPI and then turn it back on by activating the QSPI boot mode.\r\n\n");
	return 1;
}
