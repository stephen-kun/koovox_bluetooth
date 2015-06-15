/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_uart.c

*/


#include "sink_koovox_uart.h"
#include "sink_private.h"
#include "sink_app_core.h"
#include "sink_app_message.h"
#include "sink_app_task.h"
#include "sink_heart_rate_calc.h"

#if 0
#include "sink_handle_accelerate_data.h"
#endif

#define SIZE_UART_MSG	50

UARTStreamTaskData theUARTStreamTask;
uint8* uart_msg = NULL;
uint16 uart_offset = 0;
bool respond_flag = FALSE;

static void UARTStreamMessageHandler (Task pTask, MessageId pId, Message pMessage);
static void KoovoxUartMessageHandle(uint8 *data, uint16 length);
static void uart_data_stream_rx_data(Source src);
static void uart_data_stream_tx_data(const uint8 *data, uint16 length);


/****************************************************************************
NAME 
  	uart_data_stream_init

DESCRIPTION
 	init the uart_stream_task
 
RETURNS
  	void
*/ 
void uart_data_stream_init(void)
{

	APP_CORE_LOG(("uart_data_stream_init\n"));

	/* Assign task message handler */
	theUARTStreamTask.task.handler = UARTStreamMessageHandler;
	
	/* Configure uart settings */
	StreamUartConfigure(VM_UART_RATE_19K2, VM_UART_STOP_ONE, VM_UART_PARITY_NONE);
	
	/* Get the sink for the uart */
	theUARTStreamTask.uart_sink = StreamUartSink();
	PanicNull(theUARTStreamTask.uart_sink);
	
	/* Get the source for the uart */
	theUARTStreamTask.uart_source = StreamUartSource();
	PanicNull(theUARTStreamTask.uart_source);
	
	/* Register uart source with task */
	MessageSinkTask(StreamSinkFromSource(theUARTStreamTask.uart_source), &theUARTStreamTask.task);

	uart_msg = (uint8*)mallocPanic(SIZE_UART_MSG);
	uart_offset = 0;
	
}


/****************************************************************************
NAME 
  	uart_data_stream_tx_data

DESCRIPTION
 	send data to uart
 
RETURNS
  	void
*/ 
static void uart_data_stream_tx_data(const uint8 *data, uint16 length)
{
	uint16 offset = 0;
	uint8 *dest = NULL;
	
	/* Claim space in the sink, getting the offset to it */
	offset = SinkClaim(theUARTStreamTask.uart_sink, length);
	if(offset == 0xFFFF)
		Panic();
	
	/* Map the sink into memory space */
	dest = SinkMap(theUARTStreamTask.uart_sink);
	PanicNull(dest);
	
	/* Copy data into the claimed space */
	memcpy(dest+offset, data, length);
	
	/* Flush the data out to the uart */
	PanicZero(SinkFlush(theUARTStreamTask.uart_sink, length));

}

/****************************************************************************
NAME 
  	uart_data_stream_rx_data

DESCRIPTION
 	receive data from uart
 
RETURNS
  	void
*/ 
static void uart_data_stream_rx_data(Source src)
{
	uint16 length = 0;
	const uint8 *data = NULL;
	uint16 ret = 0;
	uint8 i = 0;
	
	/* Get the number of bytes in the specified source before the next packetboundary */
	if(!(length = SourceBoundary(src)))
		return;
	
	/* Maps the specified source into the address map */
	data = SourceMap(src);
	PanicNull((void*)data);

	for(; i<length; i++)
	{

		if(uart_offset == SIZE_UART_MSG)
		{
			KoovoxUartMessageHandle(uart_msg, uart_offset);
			uart_offset = 0;
		}
		
		uart_msg[uart_offset++] = data[i];
		
		ret = KoovoxMessageStr((const uint8*)uart_msg, FRAME_TAIL, uart_offset);
		if(ret)
		{
			/* handle the received message */
			KoovoxUartMessageHandle(uart_msg, uart_offset);
		
			memset(uart_msg, 0, SIZE_UART_MSG);
			uart_offset = 0;
		}
	}
		
	/* Discards the specified amount of bytes from the front of the specifiedsource */
	SourceDrop(src, length);
}

/****************************************************************************
NAME 
  	UARTStreamMessageHandler

DESCRIPTION
 	Read the lengths of other ps key configs
 
RETURNS
  	void
*/ 
void UARTStreamMessageHandler (Task pTask, MessageId pId, Message pMessage)
{
	switch (pId)
	{
	case MESSAGE_MORE_DATA:
	{
		uart_data_stream_rx_data(((MessageMoreData *)pMessage)->source);
	}
	break;
	default:
	break;
	}
}


/****************************************************************************
NAME 
  	KoovoxUartMessageHandle

DESCRIPTION
 	Read the lengths of other ps key configs
 
RETURNS
  	void
*/ 
static void KoovoxUartMessageHandle(uint8 *data, uint16 length)
{

	UartMsg* msg = (UartMsg*)data;

	if((!data) || (msg->len > (length - FRAME_UART_SIZE)))
		return;

#ifdef DEBUG_PRINT_ENABLED
	{
		char* str = (char*)mallocPanic(length*4);
		uint8 i = 0;
		for(; i<length; i++)
			sprintf(str + 3*i, "%2x ", data[i]);
		DEBUG((str));
		freePanic(str);
		DEBUG(("\n"));
	}
#endif

	switch(msg->cmd)
	{
	case CFM:
		{
			respond_flag = TRUE;
			switch(msg->obj)
			{
			case OBJ_HEART_RATE:
				break;
			}
		}
		break;

	case ENV:
		{
			switch(msg->obj)
			{
			case OBJ_STEP_COUNT:
				KoovoxCountStep(msg->data, msg->len);
				break;

			case OBJ_NECK_PROTECT:
				KoovoxProtectNeck(msg->data, msg->len);
				break;

			case OBJ_CONST_SEAT:
				break;

			case OBJ_HEAD_ACTION:
				break;

			case OBJ_HEART_RATE:
				if(GetSampleStatus())
					KoovoxCalculateHeartRate(msg->data, msg->len);
				break;

			case OBJ_I2C_TEST:
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

}


/****************************************************************************
NAME 
  	KoovoxFillAndSendUartPacket

DESCRIPTION
 	fill and send packet from the uart
 
RETURNS
  	void
*/ 
void KoovoxFillAndSendUartPacket(uint8 cmd, uint8 obj, uint8* value, uint8 size_value)
{
	uint16 length = sizeof(UartMsg) + (size_value ? (size_value - 1) : 0) + strlen(FRAME_TAIL);
	uint8* msg = (uint8*)mallocPanic(length);
	uint8 i = 0;

	msg[i++] = cmd;
	msg[i++] = obj;
	msg[i++] = size_value;

	if(size_value > 1)
	{
		uint8 j = 0;
		for(; j<size_value; j++)
			msg[i++] = value[j];
	}
	else
	{
		if(value)
			msg[i++] = value[0];
		else
			msg[i++] = 0x0;
	}

	msg[i++] = '{';
	msg[i++] = '}';

	uart_data_stream_tx_data((const uint8*)msg, length);

#ifdef DEBUG_PRINT_ENABLED
{
	char* str = (char*)mallocPanic(length*4);
	uint8 i = 0;
	
	for(; i<length; i++)
		sprintf(str + 3*i, "%2x ", msg[i]);
	DEBUG((str));
	freePanic(str);
	DEBUG(("\n"));
}
#endif

	freePanic(msg);
	msg = NULL;
	
}



