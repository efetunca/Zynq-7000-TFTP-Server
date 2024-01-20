/*
 * rtc.c
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#include "rtc.h"

/*****************************************************************************/
/**
*
* This function calls necessary functions to initialize I2C bus.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*			XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		This function must be called every time before getting the
* 			time data from RTC module.
*
*****************************************************************************/
int I2CInit(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;
	u8 RTC_channel = RTC_MUX_CHANNEL;

	/*
	 * Initialize the I2C driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (Config == NULL)
		return XST_FAILURE;

	Status = XIicPs_CfgInitialize(&I2C, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Set the I2C serial clock rate. */
	Status = XIicPs_SetSClk(&I2C, I2C_SCLK_RATE);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Send the buffer using the I2C and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */
	Status = XIicPs_MasterSendPolled(&I2C, &RTC_channel, 1, MUX_ADDR);
	while (XIicPs_BusIsBusy(&I2C));
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes the desired data to the specified address.
*
* @param	addr is the register address of the RTC module
* 			to which data is to be sent.
* @param	data is the data that will be written to RTC.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int RtcWrite(u32 addr, u32 data)
{
	int status;
	u8 sendData[2];

	sendData[0] = (u8)addr;
	sendData[1] = (u8)data;

	/* Writing the sendData array to the address defined by the name 'RTC_ADDR'. */
	status = XIicPs_MasterSendPolled(&I2C, sendData, 2, RTC_ADDR);
	while (XIicPs_BusIsBusy(&I2C));
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the data in the specified address.
*
* @param	addr is the register address of the RTC module
* 			to which data is to be read.
*
* @return	recvData if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int RtcRead(u32 addr)
{
	int status;
	u8 sendData, recvData;

	sendData = (u8)addr;

	/*
	 * To read data from RTC module, the register address
	 * that contains the data to be read is written to the RTC module.
	 */
	status = XIicPs_MasterSendPolled(&I2C, &sendData, 1, RTC_ADDR);
	while (XIicPs_BusIsBusy(&I2C));
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	/* The response is saved into the recvData variable. */
	status = XIicPs_MasterRecvPolled(&I2C, &recvData, 1, RTC_ADDR);
	while (XIicPs_BusIsBusy(&I2C));
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	return (u32)recvData;
}

/*****************************************************************************/
/**
*
* This function sets the current time of the RTC module.
*
* @param	day is the day value that is wanted to be set.
* @param	month is the month value that is wanted to be set.
* @param	year is the year value that is wanted to be set.
* @param	weekday is the weekday that is wanted to be set.
* @param	hour is the hour value that is wanted to be set.
* @param	minute is the minute value that is wanted to be set.
* @param	second is the second value that is wanted to be set.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		Only the last 2 digits of the year information
* 			should be entered (e.g '23' for '2023').
* @note		The abbreviations for weekdays can be found in the
* 			GetWeekString function.
*
******************************************************************************/
int RtcSetTime(u8 day, u8 month, u8 year, u8 weekday, u8 hour, u8 minute, u8 second)
{
	RtcWrite(RTC_CONTROL1, 0x20);
	RtcWrite(RTC_CONTROL2, 0x00);

	RtcWrite(RTC_HOURS,   IntToBcd(hour));
	RtcWrite(RTC_MINUTES, IntToBcd(minute));
	RtcWrite(RTC_SECONDS, IntToBcd(second));

	RtcWrite(RTC_YEARS,		IntToBcd(year));
	RtcWrite(RTC_MONTHS,	IntToBcd(month));
	RtcWrite(RTC_DAYS,		IntToBcd(day));
	RtcWrite(RTC_WEEKDAYS,	IntToBcd(weekday));

	RtcWrite(RTC_MINUTE_ALARM,	0x00);
	RtcWrite(RTC_HOUR_ALARM,	0x00);
	RtcWrite(RTC_DAY_ALARM,		0x00);
	RtcWrite(RTC_WEEKDAY_ALARM,	0x00);

	RtcWrite(RTC_CLKOUT_FREQ,	0x00);
	RtcWrite(RTC_TIMER_CONTROL,	0x00);
	RtcWrite(RTC_TIMER,			0x00);

	RtcWrite(RTC_CONTROL1, 0x00);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets the current time information and
* saves it into the RTC_INFO struct.
*
* @param	info is the pointer to the RTC_INFO struct.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int GetCurrentTime(RTC_INFO *info)
{
	/*
	 * Since the read values are in BCD format except weekdays,
	 * these values are converted to integer to be able to interpret them.
	 */
	info->week		= RtcRead(RTC_WEEKDAYS) & 0x07;
	info->day		= BcdToInt(RtcRead(RTC_DAYS) & 0x3F);
	info->month		= BcdToInt(RtcRead(RTC_MONTHS) & 0x1F);
	info->year		= BcdToInt(RtcRead(RTC_YEARS));
	info->hour		= BcdToInt(RtcRead(RTC_HOURS) & 0x3F);
	info->minute	= BcdToInt(RtcRead(RTC_MINUTES) & 0x7F);
	info->second	= BcdToInt(RtcRead(RTC_SECONDS) & 0x7F);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function gets the integer value of the weekday and returns it as a string.
*
* @param	week is the integer value of the weekday.
*
* @return	Specified element of WEEK array if successful.
*
* @note		Week starts from Sunday.
*
******************************************************************************/
u8 *GetWeekString(u32 week)
{
	static const char *WEEK[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
	return (u8*) WEEK[week];
}

/*****************************************************************************/
/**
*
* This function converts integer numbers to BCD format.
*
* @param	number is the integer that will be converted to BCD.
*
* @return	The BCD value of the integer that is entered as a parameter.
*
* @note		None.
*
******************************************************************************/
u8 IntToBcd(u8 number)
{
	return ((number / 10) << 4) + (number % 10);
}

/*****************************************************************************/
/**
*
* This function converts BCD numbers to integer.
*
* @param	bcdNumber is the number in BCD format
* 			that will be converted to integer.
*
* @return	The integer value of the BCD number that is entered as a parameter.
*
* @note		None.
*
******************************************************************************/
u8 BcdToInt(u8 bcdNumber)
{
	return (((bcdNumber >> 4) * 10) + (bcdNumber & 0x0F));
}
