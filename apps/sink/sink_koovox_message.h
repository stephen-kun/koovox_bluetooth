/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_message.h

*/

#ifdef ENABLE_KOOVOX


#ifndef __SINK_KOOVOX_MESSAGE_H
#define __SINK_KOOVOX_MESSAGE_H

#include <message.h>
#include <gatt.h>

#include "sink_private.h"


/* the size of request and response */
#define FRAME_SIZE		(13)
#define PACKET_SIZE		(6)
#define INDEX_ELEMENT	(6)

/* The status code of app req */
#define PROGRESS	((uint16)100)
#define SUCCESS		((uint16)200)
#define ERROR		((uint16)400)
#define UNAUTHENTICATION ((uint16)407)

#define MAX_DATA_BUF	(30)

#define FRAME_TAIL				"{}"

#define RSP_MASK				(0x8000)

typedef enum{
	OID_SYSTEM 			= 0x0000,
	OID_BATTERY			= 0x0001,
	OID_BUTTON_C		= 0x0002,
	OID_BUTTON_M		= 0x0003,
	OID_ACC_SENSOR		= 0x0004,
	OID_HB_SENSOR		= 0x0005,
	OID_CALL			= 0x0100,
	OID_RECODE			= 0x0200,
	OID_MODE			= 0x0300,
	OID_BAT				= 0x0400,
	OID_HB				= 0x0500,
	OID_STEP			= 0x0600,
	OID_ACC 			= 0x0700,
	OID_NECK 			= 0x0800,
	OID_VP_MODE			= 0x0900,
	OID_EC_NUM			= 0x0A00,
	OID_VOICE_MSG		= 0x0B00,
	OID_DEVICE_INFO		= 0x0C00,
	OID_DRIVER_ENABLE	= 0x0D00,
	OID_VP_ENABLE		= 0x0E00,
	OID_VP_AUTO			= 0x0F00,
	OID_VM_ENABLE		= 0x1000,
	OID_VM_AUTO			= 0x1100,
	OID_LOG				= 0x1200,
	OID_DFU				= 0x1300,
	OID_SPORT_VALUE		= 0x1400,
	OID_PROMPT_TIME		= 0x1500,
	OID_PROMPT_SPORT	= 0x1600,
	OID_SEAT_PROMPT		= 0x1700
}OidValue;

typedef enum{
	ON = 0x01,
	OFF,
	CLICK,
	DOUBLE_CLICK,
	LONG_CLICK,
	PRESS,
	RELEASE
}EnumUserEvent;

typedef enum{
	ID_OID = 0x01,			/* object id cmd id */
	ID_EVENT,
	ID_DATA
}EnumId;

typedef enum{
	CMD_BIND = 0x2B00,
	CMD_GET,
	CMD_SET,
	CMD_NOTIFY = 0x2B10
}EnumCmd;


/* the struct of request */
typedef struct{
	uint8 id_oid;
	uint8 value_oid[2];
	uint8 id_event;
	uint8 value_event;
	uint8 id_data;
	uint8 value_data[1];
}PacketRequest_t;

/* the struct of notify request */
typedef struct{
	uint8 cmd[2];
	uint8 tid[2];
	uint8 size[2];		/* the len of the notify request element */
	uint8 id_oid;
	uint8 value_oid[2];
	uint8 id_event;
	uint8 value_event;
	uint8 id_data;
	uint8 value_data[1];
}NotifyRequest_t;

/* the struct of response */
typedef struct{
	uint8 status[2];
	uint8 tid[2];
	uint8 size[2];		/* the len of the set response element */
	uint8 id_oid;
	uint8 value_oid[2];
	uint8 id_event;
	uint8 value_event;
	uint8 id_data;
	uint8 value_data[1];
}PacketResponse_t;


/*************************************************************************
NAME
    KoovoxAppConnection
    
DESCRIPTION
    the mobile app connected the koovox
*/
void KoovoxAppConnection(Sink sink);


/*************************************************************************
NAME
    KoovoxMessageStr
    
DESCRIPTION
    check the message include str or not

RETURN
	return the index of the str .if return 0, failed find the str
*/
uint16 KoovoxMessageStr(const uint8* msg, const char* str, uint16 msg_len);

/*************************************************************************
NAME
    DisposeSppMessage
    
DESCRIPTION
    dispose mobile message from spp profile
*/
void DisposeSppMessage(uint8* msg, uint16 size);

/*************************************************************************
NAME
    SendResponse
    
DESCRIPTION
    send response for request
*/
void SendResponse(uint16 status, uint16 cmd, uint16 value_oid, uint8 value_event, uint8* value_data, uint16 size_data);
/*************************************************************************
NAME
    SetRequestAction
    
DESCRIPTION
    dipose the set request

RETURN
	the result of the set request
*/
uint16 SetRequestAction(uint8* value, uint16 size_value);

/*************************************************************************
NAME
    GetRequestAction
    
DESCRIPTION
    dipose the get request

RETURN
	the result of the bind request
*/
uint16 GetRequestAction(uint8* value, uint16 size_value);


/*************************************************************************
NAME
    SendMessageWithSink
    
DESCRIPTION
    send message with the sink
*/
void SendMessageWithSink(Sink sink, void* msg, uint16 size);


void SendResponseToDevice(uint8* rsp, uint16 size);

#endif /* __SINK_KOOVOX_MESSAGE_H */


#endif

