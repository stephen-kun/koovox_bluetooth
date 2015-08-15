/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_uart.c

*/


#include "sink_koovox_uart.h"
#include "sink_private.h"
#include "sink_koovox_core.h"
#include "sink_koovox_task.h"

#include "sink_handle_accelerate_data.h"
#include "sink_heart_rate_calc.h"
#include "koovox_wechat_handle.h"

UARTStreamTaskData theUARTStreamTask;
uint8* uart_msg = NULL;
uint16 uart_offset = 0;
bool wechat_req = FALSE;
bool button_req = FALSE;

static void UARTStreamMessageHandler (Task pTask, MessageId pId, Message pMessage);
static void KoovoxUartMessageHandle(uint8 *data, uint16 length);
static void uart_data_stream_rx_data(Source src);
static void uart_data_stream_tx_data(const uint8 *data, uint16 length);

/*************************************************************************
NAME
    KoovoxMessageStr
    
DESCRIPTION
    check the message include str or not

RETURN
	return the index of the str .if return 0, failed find the str
*/
static uint16 KoovoxMessageStr(const uint8* msg, const char* str, uint16 msg_len)
{
	uint16 i = 0, j = 0;

	if((msg == NULL)||(str == NULL))
		return 0;

	while(str[j])
	{
		if(msg[i++] == str[j])
			j++;
		else
			j = 0;

		if(i >= msg_len)
			break;
	}
	
	if(str[j] == '\0')
		return i;
	else
		return 0;

}



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

	KOOVOX_CORE_LOG(("uart_data_stream_init\n"));

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
	uint16 length = 0, size = 0;
	const uint8 *data = NULL;
	uint16 ret = 0;
	uint8 i = 0;
	
	/* Get the number of bytes in the specified source before the next packetboundary */
	if(!(length = SourceBoundary(src)))
		return;
	
	/* Maps the specified source into the address map */
	data = SourceMap(src);
	PanicNull((void*)data);

	DEBUG(("++++length:%d+++\n", length));
	
	size = length;
	while(length)
	{
		if(uart_offset == SIZE_UART_MSG)
			uart_offset = 0;
	
		if((length + uart_offset) < SIZE_UART_MSG)
		{
			memcpy(uart_msg + uart_offset, data + i, length);
			uart_offset += length;
			i += length;
			length = 0;
		}
		else
		{
			uint16 len = SIZE_UART_MSG - uart_offset;
			memcpy(uart_msg + uart_offset, data + i, len);
			i += len;
			uart_offset = SIZE_UART_MSG;
			length -= len;
		}

		ret = KoovoxMessageStr((const uint8*)uart_msg, FRAME_TAIL, uart_offset);
		if(ret)
		{
			/* handle the received message */
			KoovoxUartMessageHandle(uart_msg, ret);
		
			if(ret != uart_offset)
			{
				i -= (uart_offset - ret); 
				length += (uart_offset - ret);
			}
		
			uart_offset = 0;
		}
	}

	/* Discards the specified amount of bytes from the front of the specifiedsource */
	SourceDrop(src, size);
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
  	KoovoxResponseFrameError

DESCRIPTION
 	response the frame error conditial
 
RETURNS
  	void
*/ 
static void KoovoxResponseFrameError(uint16 value)
{
	uint8 cmd = value & 0xff;
	uint8 obj = (value >> 8) & 0xff;

	if(koovox.repeat_times >= REPEAT_TIMES)
	{
		koovox.repeat_times = 0;
		/* 语音提示用户，通信异常 */
		
	}
	else
	{
		if(button_req)
		{
			/* 重发命令 */
			KoovoxStoreLastCmd(cmd, obj);
			KoovoxFillAndSendUartPacket(cmd, obj, 0, 0);
			koovox.repeat_times++;
		}

		if(wechat_req)
		{
			/* 通知前端，通信异常 */
			RspWechat_t response = {0};
			response.state = S_ERROR;
			response.cmd  = cmd;
			response.obj  = obj;

			koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
			koovox_send_data_to_wechat();
			
			wechat_req = FALSE;
		}
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
	{
		/* 通知前端通信异常 */
		RspWechat_t response = {0};
		response.state = S_ERROR;
		response.cmd  = 0;
		response.obj  = 0;
		
		koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
		koovox_send_data_to_wechat();
		return;
	}

#ifdef DEBUG_PRINT_ENABLED
	{
		uint8 i = 0;
		for(; i<length; i++)
			DEBUG(("%3d ", data[i]));
		DEBUG(("\n"));
	}
#endif

	switch(msg->cmd)
	{
	case CFM:
		{
			switch(msg->obj)
			{
			case OBJ_HEALTH_MONITOR:
				KoovoxResponseHealthMonitor(msg->data, msg->len);
				break;
			
			case OBJ_HEART_RATE:
				KoovoxResponseHeartRate(msg->data, msg->len);
				break;

			case OBJ_NECK_PROTECT:
				KoovoxResponseNeckProtect(msg->data, msg->len);
				break;

			case OBJ_STEP_COUNT:
				KoovoxResponseStepCount(msg->data, msg->len);
				break;

			case OBJ_CONST_SEAT:
				KoovoxResponseConstSeat(msg->data, msg->len);
				break;

			case OBJ_FRAME_ERR:
				KoovoxResponseFrameError(koovox.last_cmd);
				break;

			default:
				button_req = FALSE;
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
				KoovoxConstSeat(msg->data, msg->len);
				break;

			case OBJ_NOD_HEAD:
				KoovoxNodHead(msg->data, msg->len);
				break;

			case OBJ_SHAKE_HEAD:
				KoovoxShakeHead(msg->data, msg->len);
				break;

			case OBJ_HEART_RATE:
				KoovoxCalculateHeartRate(msg->data, msg->len);
				break;

			case OBJ_I2C_TEST:
				KoovoxResultI2cTest(msg->data, msg->len);
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
	uint8 i = 0;
	
	for(; i<length; i++)
		DEBUG(("%2x ", msg[i]));
	DEBUG(("\n"));
}
#endif

	freePanic(msg);
	msg = NULL;
	
}

/****************************************************************************
NAME 
  	KoovoxControlObject

DESCRIPTION
 	control the object enable or disable
 
RETURNS
  	void
*/ 
void KoovoxControlObject(uint8 ctrl, uint8 obj)
{
	KoovoxFillAndSendUartPacket(ctrl, obj, 0, 0);
}

