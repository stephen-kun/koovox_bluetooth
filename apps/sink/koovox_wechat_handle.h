/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    koovox_wechat_handle.h

*/

#ifdef ENABLE_KOOVOX


#ifndef __KOOVOX_WECHAT_HANDLE_H
#define __KOOVOX_WECHAT_HANDLE_H

#include <message.h>
#include <gatt.h>

#include "sink_private.h"
#include "epb_MmBp.h"


#define GATT_DEFAULT_MTU		(23)
#define PROTO_VERSION 			0x010004
#define AUTH_PROTO 				1
#define SIZE_GATT_MTU			(GATT_DEFAULT_MTU - 3)
#define FRAME_SIZE_FIX_HEAD		8
#define CHALLENAGE_LENGTH		4



#define	errorCodeUnpackAuthResp   			0x9990
#define	errorCodeUnpackInitResp  	 		0x9991
#define	errorCodeUnpackSendDataResp			0x9992
#define	errorCodeUnpackCtlCmdResp			0x9993
#define	errorCodeUnpackRecvDataPush 		0x9994
#define	errorCodeUnpackSwitchViewPush 		0x9995
#define	errorCodeUnpackSwitchBackgroundPush 0x9996
#define	errorCodeUnpackErrorDecode 			0x9997

typedef enum
{
	S_SUC,
	S_PROCESS,
	S_ERROR
}WechatState;

typedef enum
{
	sendTextReq = 0x01,
	sendTextResp = 0x1001,
	openLightPush = 0x2001,
	closeLightPush = 0x2002
}WeChatCmdID;

typedef enum
{
	TYPE_NONE,
	TYPE_RFCOMM,
	TYPE_BLE
}WechatTramsportType;

typedef struct 
{
	bool	connect_state;
	bool 	wechats_switch_state; /* 公众账号切换到前台的状态 */
	bool 	auth_state;
	bool 	init_state;
	bool 	auth_send;
	bool 	init_send;
	uint16 	send_data_seq;
	uint16 	push_data_seq;
	uint16 	seq; 
}Wechatble_state;

typedef struct
{
	uint8 *data;
	uint16 len;
	uint16 offset;
}Data_info;

typedef struct
{
	uint16 cid;
	uint16 handle;
}Ble;

typedef struct
{
	WechatTramsportType type;		/* 连接微信的方式 */
	Wechatble_state 	state;
	uint8				challeange[CHALLENAGE_LENGTH];
	Data_info			rcv_data;
	Data_info			send_data;

	union
	{
		Ble 				ble;
		WECHAT_TRANSPORT* 	rfcomm;
	}transport;
	
}WECHAT_DATA_T;

typedef struct{
	uint8 	bMagicNumber;
	uint8 	bVer;
	uint16 	nLength;
	uint16 	nCmdId;
	uint16 	nSeq;
}BpFixHead;

typedef struct
{
	uint8 cmd;
	uint8 obj;
	uint8 len;
	uint8 data[1];
}ReqWechat_t;

typedef struct
{
	uint8 state;
	uint8 cmd;
	uint8 obj;
}RspWechat_t;

void koovox_wechat_connect(WechatTramsportType type, Ble* ble, WECHAT_TRANSPORT* rfcomm);
void koovox_rcv_data_from_wechat(uint8* data, uint16 size_data);
void koovox_pack_wechat_send_data_req(uint8* data, uint16 size_data, bool has_type, EmDeviceDataType type);
void koovox_send_data_to_wechat(void);
void koovox_wechat_disconnect(void);


#endif /* __KOOVOX_WECHAT_HANDLE_H */


#endif

