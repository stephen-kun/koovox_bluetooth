/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_uart.h

*/

#ifndef __SINK_KOOVOX_UART_H
#define __SINK_KOOVOX_UART_H

#include <stream.h>
#include <sink.h>
#include <source.h>
#include <string.h>
#include <panic.h>
#include <message.h>
#include <app/uart/uart_if.h>

#define FRAME_UART_SIZE		5
#define SIZE_RESPONSE		2
#define SIZE_FRAME_MSG		4
#define SIZE_UART_MSG		50

#define LOG_WIDE	5

#define REPEAT_TIMES		(3)


typedef struct
{
	TaskData task;
	Sink uart_sink;
	Source uart_source;
} UARTStreamTaskData;

typedef struct
{
	uint8 cmd;
	uint8 obj;
	uint8 len;
	uint8 data[1];
}UartMsg;

typedef enum
{
	START = 0x01,
	STOP,
	CFM,	
	CNF,	
	GET,
	ENV		
}enumCmd;

typedef enum
{
	OBJ_SYSTEM 			= 0x01,
	OBJ_HEALTH_MONITOR	= 0x02,
	OBJ_CONST_SEAT 		= 0x03,
	OBJ_NECK_PROTECT 	= 0x04,
	OBJ_NOD_HEAD		= 0x05,
	OBJ_SHAKE_HEAD		= 0x06,
	OBJ_STEP_COUNT		= 0x07,
	OBJ_HEART_RATE		= 0x08,
	OBJ_I2C_TEST		= 0x09,
	OBJ_FRAME_ERR		= 0xff
}enumObj;

typedef enum{
	SUC,
	ERR,
	PROCESS
}enumFrameDetail;

void uart_data_stream_init(void);

void KoovoxFillAndSendUartPacket(uint8 cmd, uint8 obj, uint8* value, uint8 size_value);

void KoovoxControlObject(uint8 ctrl, uint8 obj);

extern bool wechat_req;
extern bool button_req;

#endif


