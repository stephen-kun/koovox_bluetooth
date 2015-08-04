/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_message.c

*/


#ifdef ENABLE_KOOVOX

#include "sink_koovox_message.h"
#include "sink_koovox_uart.h"
#include "sink_koovox_task.h"
#include "sink_koovox_core.h"


#define DEVICE_INFO  "1.0.1:A8"

/*************************************************************************
NAME
    KoovoxMessageStr
    
DESCRIPTION
    check the message include str or not
*/
uint16 KoovoxMessageStr(const uint8* msg, const char* str, uint16 msg_len)
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


/*************************************************************************
NAME
    KoovoxAppConnection
    
DESCRIPTION
    the mobile app connected the koovox
*/
void KoovoxAppConnection(Sink sink)
{
	koovox.sppSink = sink;
	koovox.sppValue = (uint8*)mallocPanic(MAX_DATA_BUF);
	koovox.sizeSppValue = 0;
}


/*************************************************************************
NAME
    DisposeSppMessage
    
DESCRIPTION
    dispose mobile message from spp profile
*/
void DisposeSppMessage(uint8* msg, uint16 size)
{
	uint16 cmd = 0;
	uint16 len = 0;
	uint16 status = 0;
	uint16 oid = 0;

	DEBUG(("APP_MSG:DisposeSppMessage\n"));

	cmd = ((uint16)msg[0] << 8) + (uint16)msg[1];
	len = ((uint16)msg[4] << 8) + (uint16)msg[5];
	oid = (uint16)msg[7] + ((uint16)msg[8] << 8);
	
	switch(cmd)
	{
		case CMD_GET:
			GetRequestAction(msg + INDEX_ELEMENT, len);
			break;
			
		case CMD_SET:
			status = SetRequestAction(msg + INDEX_ELEMENT, len);
			SendResponse(status, CMD_SET, oid, 0, 0, 1);
			break;
			
		default:
			break;
	}
	
}


/*************************************************************************
NAME
    GetRequestAction
    
DESCRIPTION
    dipose the get request

RETURN
	the result of the bind request
*/
uint16 GetRequestAction(uint8* value, uint16 size_value)
{
	uint16 oid = 0;
	uint8 value_data = 0;
	PacketRequest_t *req = (PacketRequest_t *)value;
		
	/* check the set request size */
	if(size_value < PACKET_SIZE)
		return ERROR;
	
	oid =(uint16)req->value_oid[0] + ((uint16)req->value_oid[1] << 8); 
		
	switch(oid)
	{	

	case OID_DEVICE_INFO:
	{
	uint8 info[10] = {0};
	memcpy(info, DEVICE_INFO, strlen(DEVICE_INFO));
	SendResponse(SUCCESS, CMD_GET, OID_DEVICE_INFO, 0, info, strlen(DEVICE_INFO));
	}
	break;	

	/* get battery value */
	case OID_BAT:				
	/* get the battery value */
	value_data = 0x30;
	SendResponse(SUCCESS, CMD_GET, OID_BAT, 0, &value_data, 1);
	break;
	
	default:
	break;
		
	}

    return SUCCESS;
}

/*************************************************************************
NAME
    SetRequestAction
    
DESCRIPTION
    dipose the set request

RETURN
	the result of the set request
*/
uint16 SetRequestAction(uint8* value, uint16 size_value)
{
	PacketRequest_t *req = (PacketRequest_t *)value;
	uint16 oid = 0;

	DEBUG(("APP_MSG:SetRequestAction %d \n", size_value));
			
	/* check the set request size */
	if(size_value < PACKET_SIZE)
		return ERROR;

	oid = (uint16)req->value_oid[0] + ((uint16)req->value_oid[1] << 8);

	switch(oid)
	{
	
	case OID_HB:
	{
		DEBUG(("OID_HB \n"));
		if(ON == *(req->value_data))
		{
			KoovoxFillAndSendUartPacket(START, OBJ_HEART_RATE, 0, 0);	
		}
		else
		{
			KoovoxFillAndSendUartPacket(STOP, OBJ_HEART_RATE, 0, 0);	
		}
	}
	break;

	case OID_NECK:
	{
		DEBUG(("set OID_NECK\n"));
		if(ON == *(req->value_data))
		{
			KoovoxFillAndSendUartPacket(START, OBJ_NECK_PROTECT, 0, 0);	
		}
		else
		{
			KoovoxFillAndSendUartPacket(STOP, OBJ_NECK_PROTECT, 0, 0);	
		}
	}
	break;

	case OID_SEAT_PROMPT:
	{
		DEBUG(("set OID_SEAT_PROMPT\n"));
		if(ON == *(req->value_data))
		{
			KoovoxFillAndSendUartPacket(START, OBJ_CONST_SEAT, 0, 0);	
		}
		else
		{
			KoovoxFillAndSendUartPacket(STOP, OBJ_CONST_SEAT, 0, 0);	
		}
	}
	break;

	case OID_DRIVER_ENABLE:
	{
		DEBUG(("set OID_DRIVER_ENABLE\n"));
		if(ON == *(req->value_data))
		{
			KoovoxFillAndSendUartPacket(START, OBJ_NOD_HEAD, 0, 0);	
		}
		else
		{
			KoovoxFillAndSendUartPacket(STOP, OBJ_NOD_HEAD, 0, 0);	
		}
	}		
	break;

	case OID_VM_ENABLE:
	{
		DEBUG(("set OID_VM_ENABLE\n"));
		if(ON == *(req->value_data))
		{
			KoovoxFillAndSendUartPacket(START, OBJ_I2C_TEST, 0, 0);	
		}
	}
	break;

	case OID_DFU:
	{
		DEBUG(("OID_DFU \n"));
		MessageSend(&theSink.task, EventUsrGaiaDFURequest, 0);
	}
	break;
	
	default:
		break;
	}
	
    return SUCCESS;
}


/*************************************************************************
NAME
    SendResponse
    
DESCRIPTION
    send response for request
*/
void SendResponse(uint16 status, uint16 cmd, uint16 value_oid, uint8 value_event, uint8* value_data, uint16 size_data)
{
	/* 判断蓝牙是否与手机连接 */
	uint16 length = sizeof(PacketResponse_t) + (size_data ? size_data - 1 : 0);
	PacketResponse_t *response = (PacketResponse_t *)mallocPanic(length);
	memset(response, 0, length);
	
	DEBUG(("APP_MSG:SendResponse\n"));
	
	/* fill the rsp */
	response->status[0] = (status >> 8) & 0xff;
	response->status[1] = status & 0xff;
	response->tid[0] = ((cmd + RSP_MASK) >> 8) & 0xff;
	response->tid[1] = (cmd + RSP_MASK) & 0xff;
	response->size[0] = ((PACKET_SIZE + size_data) >> 8) & 0xff;
	response->size[1] = (PACKET_SIZE + size_data) & 0xff;
	response->id_oid = ID_OID;
	response->value_oid[0] = value_oid & 0xff;
	response->value_oid[1] = (value_oid >> 8) & 0xff;
	response->id_event = ID_EVENT;
	response->value_event = value_event;
	response->id_data = ID_DATA;
	if(value_data)
		memmove(response->value_data, value_data, size_data);
	
	SendResponseToDevice((uint8*)response, length);
	
	free(response);
	response = NULL;

}

/*************************************************************************
NAME
    SendResponseToDevice
    
DESCRIPTION
    send response to device
*/
void SendResponseToDevice(uint8* rsp, uint16 size)
{

    uint16 length = size + 2;
	uint8* msg = (uint8*)mallocPanic(length);
	memcpy(msg, rsp, size);
	memcpy(msg + size, FRAME_TAIL, strlen(FRAME_TAIL));

	DEBUG(("APP_MSG:SendResponseToDevice:%d\n", length));

	SendMessageWithSink(koovox.sppSink, msg, size + 2);

	freePanic(msg);
	msg = NULL;
}

/*************************************************************************
NAME
    SendMessageWithSink
    
DESCRIPTION
    send message with the sink
*/
void SendMessageWithSink(Sink sink, void* msg, uint16 size)
{
	uint16 offset = 0;

	DEBUG(("APP_MSG:SendMessageWithSink\n"));
	
	offset = SinkClaim(sink, size);
	if (offset != 0xFFFF)
	{
		memcpy(SinkMap(sink) + offset, msg, size);
		SinkFlush(sink,size);
		DEBUG(("APP_MSG:the sink send msg suc!\n"));
	}
}




#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    

#endif /* ENABLE_KOOVOX */


