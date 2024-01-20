/*
 * rtc.h
 *
 *  Created on: 21 Jan 2024
 *      Author: Efe Tunca
 */

#ifndef SRC_RTC_H_
#define SRC_RTC_H_

#include "xparameters.h"
#include "xil_printf.h"
#include "xiicps.h"

#define MUX_ADDR			0x74
#define I2C_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define I2C_SCLK_RATE		100000
#define RTC_ADDR			0x51
#define RTC_MUX_CHANNEL		0x10

#define RTC_CONTROL1 		0x00
#define RTC_CONTROL2 		0x01
#define RTC_SECONDS  		0x02
#define RTC_MINUTES  		0x03
#define RTC_HOURS    		0x04
#define RTC_DAYS     		0x05
#define RTC_WEEKDAYS 		0x06
#define RTC_MONTHS	 		0x07
#define RTC_YEARS    		0x08
#define RTC_MINUTE_ALARM  	0x09
#define RTC_HOUR_ALARM    	0x0a
#define RTC_DAY_ALARM     	0x0b
#define RTC_WEEKDAY_ALARM 	0x0c
#define RTC_CLKOUT_FREQ   	0x0d
#define RTC_TIMER_CONTROL 	0x0e
#define RTC_TIMER         	0x0f

typedef struct {
	u8  year; 	// RTC year
	u8  month;  // RTC month
	u8  day;  	// RTC day
	u8  week; 	// RTC week
	u8  hour; 	// RTC hour
	u8  minute;	// RTC minute
	u8  second;	// RTC second
} RTC_INFO;

int I2CInit(u16 DeviceId);
int RtcWrite(u32 addr, u32 data);
int RtcRead(u32 addr);
int RtcSetTime(u8 year, u8 month, u8 day, u8 weekday, u8 hour, u8 minute, u8 second);
int GetCurrentTime(RTC_INFO *info);
u8 *GetWeekString(u32 week);
u8 IntToBcd(u8 number);
u8 BcdToInt(u8 bcdNumber);

XIicPs I2C;

#endif /* SRC_RTC_H_ */
