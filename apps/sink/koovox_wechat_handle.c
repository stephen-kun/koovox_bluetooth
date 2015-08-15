/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    koovox_wechat_handle.c

*/


#ifdef ENABLE_KOOVOX

#include "koovox_wechat_handle.h"
#include "koovox_wechat_util.h"
#include "sink_koovox_core.h"
#include "sink_debug.h"
#include "crc32.h"
#include "sink_koovox_uart.h"
#include <wechat.h>
#include <gatt.h>


WECHAT_DATA_T* g_wechat = NULL;


/*************************************************************************
NAME
    koovox_pack_wechat_head
    
DESCRIPTION
    pack the wechat head packect

RETURN
    void
*/
static void koovox_pack_wechat_head(uint16 cmdid, uint8* data)
{
	data[0] = 0xfe;
	data[1] = 0x01;
	data[2] = (g_wechat->send_data.len >> 8) & 0xff;
	data[3] = g_wechat->send_data.len & 0xff;
	data[4] = (cmdid >> 8) & 0xff;
	data[5] = cmdid & 0xff;
	data[6] =(g_wechat->state.seq >> 8) & 0xff;
	data[7] = g_wechat->state.seq & 0xff;
}




/*************************************************************************
NAME
    koovox_wechat_error_handle
    
DESCRIPTION
    device handle the error conditional

RETURN
    void
*/
static void koovox_wechat_error_handle(uint16 error_code)
{
	DEBUG(("koovox_wechat_error_handle: ERROR_CODE=%x \n", error_code));

	switch(error_code)
	{
	case errorCodeUnpackAuthResp:
		break;

	default:
		break;
	}
}


/*************************************************************************
NAME
    koovox_indicate_to_wechat
    
DESCRIPTION
    device indicate data to wechat

RETURN
    void
*/
static void koovox_indicate_to_wechat(uint16 cid, uint16 handle)
{
	if(g_wechat->send_data.len > g_wechat->send_data.offset)
	{
		uint16 len_send = 0;
		if(g_wechat->send_data.len - g_wechat->send_data.offset > SIZE_GATT_MTU)
			len_send = SIZE_GATT_MTU;
		else
			len_send = g_wechat->send_data.len - g_wechat->send_data.offset;

		theSink.rundata->gatt.server.indication = indication_wechat;
		
		/* If indications were enabled, send one */
		GattIndicationRequest(&theSink.rundata->gatt.task, cid, handle, len_send,g_wechat->send_data.data + g_wechat->send_data.offset);

		g_wechat->send_data.offset += len_send;

		DEBUG(("server_handle_gatt_indication_cfm:offset %d\n", g_wechat->send_data.offset));
	}
	else if(g_wechat->send_data.len == g_wechat->send_data.offset)
	{
		DEBUG(("****finish send data*****\n"));
		g_wechat->send_data.len = 0;
		g_wechat->send_data.offset= 0;
		freePanic(g_wechat->send_data.data);
		g_wechat->send_data.data = NULL;

		theSink.rundata->gatt.server.indication = indication_none;
	}
}


/*************************************************************************
NAME
    koovox_pack_wechat_auth_req
    
DESCRIPTION
    pack wechat auth request package

RETURN
    void
*/
static void koovox_pack_wechat_auth_req(void)
{
	AuthRequest authReq = {0};
	uint8 bd_addr[6] = {0};
	uint16 fix_head_len = FRAME_SIZE_FIX_HEAD;
	
	memset(&authReq, 0, sizeof(authReq));
	KoovoxGetBluetoothAdrress((uint8 *)bd_addr);
	
	authReq.proto_version = PROTO_VERSION;
	authReq.auth_proto = AUTH_PROTO;
	authReq.auth_method = EAM_macNoEncrypt;
	authReq.has_mac_address = TRUE;
	authReq.mac_address.len = 0x06;
	authReq.mac_address.data = (uint8*)bd_addr;
					
	g_wechat->send_data.len = epb_auth_request_pack_size(&authReq) + fix_head_len;
	g_wechat->send_data.data = (uint8*)mallocPanic(g_wechat->send_data.len);
	
	g_wechat->state.seq++;
	
	koovox_pack_wechat_head(ECI_req_auth, g_wechat->send_data.data);

	
	epb_pack_auth_request(&authReq, g_wechat->send_data.data + fix_head_len, g_wechat->send_data.len - fix_head_len);

	g_wechat->send_data.offset = 0;
					
}


/*************************************************************************
NAME
    koovox_pack_wechat_init_req
    
DESCRIPTION
    pack wechat init request package

RETURN
    void
*/
static void koovox_pack_wechat_init_req(void)
{	
	InitRequest initReq = {0};
	uint16 fix_head_len = FRAME_SIZE_FIX_HEAD;

	/* 需要根据实际情况修改 */
	uint8 resp_field[] = {0x7f, 0x0, 0x0, 0x0};
	
	memset(&initReq, 0, sizeof(initReq));
	g_wechat->state.seq++;
	
	initReq.has_resp_field_filter = TRUE;
	initReq.resp_field_filter.len = sizeof(resp_field);
	initReq.resp_field_filter.data = resp_field;
	initReq.has_challenge = TRUE;
	initReq.challenge.len = CHALLENAGE_LENGTH;
	initReq.challenge.data = g_wechat->challeange;
	
	g_wechat->send_data.len = epb_init_request_pack_size(&initReq)+ fix_head_len;
	g_wechat->send_data.data = (uint8*)mallocPanic(g_wechat->send_data.len);

	koovox_pack_wechat_head(ECI_req_init, g_wechat->send_data.data);
	
	epb_pack_init_request(&initReq, g_wechat->send_data.data + fix_head_len, g_wechat->send_data.len - fix_head_len);
	
	g_wechat->send_data.offset = 0;
	
}


/*************************************************************************
NAME
    koovox_rcv_data_handle
    
DESCRIPTION
    device confirm the response from the wechat

RETURN
    void
*/
static uint16 koovox_rcv_data_handle(uint8* data, uint16 size_data)
{
	uint16 result = 0;
	uint16 cmd = 0;
	uint16 fix_head_len = FRAME_SIZE_FIX_HEAD;

	if(size_data < FRAME_SIZE_FIX_HEAD)
		return errorCodeUnpackErrorDecode;

	cmd = ((uint16)data[4] << 8) + (uint16)data[5];

	DEBUG(("========cmdid:%x \n", cmd));
	
	switch(cmd)
	{
	case ECI_none:
	break;
	
	case ECI_resp_auth:
	{
		AuthResponse* authResp;
		authResp = epb_unpack_auth_response(data + fix_head_len, size_data - fix_head_len);
		if(!authResp)
		{
			return errorCodeUnpackAuthResp;
		}
		
		if(authResp->base_response)
		{
			if(authResp->base_response->err_code == 0)
			{
				g_wechat->state.auth_state = TRUE;
			}
			else
			{
				result = (uint16)authResp->base_response->err_code;
			}
		}
		epb_unpack_auth_response_free(authResp);

		if((g_wechat->state.auth_state) 
			&& (!g_wechat->state.init_state) 
			&& (!g_wechat->state.init_send))
		{
			koovox_pack_wechat_init_req();
			koovox_send_data_to_wechat();
		}

	}
	break;
	
	case ECI_resp_sendData:
	{
	
		SendDataResponse *sendDataResp;
		sendDataResp = epb_unpack_send_data_response(data + fix_head_len,size_data - fix_head_len);
		if(!sendDataResp)
		{
			return errorCodeUnpackSendDataResp;
		}
		
		if(sendDataResp->base_response->err_code)
		{
			result = (uint16)sendDataResp->base_response->err_code;
		}
		epb_unpack_send_data_response_free(sendDataResp);
	}
	break;
	
	case ECI_resp_init:
	{
		InitResponse *initResp = epb_unpack_init_response(data + fix_head_len, size_data - fix_head_len);
		if(!initResp)
		{
			return errorCodeUnpackInitResp;
		}
		
		if(initResp->base_response)
		{
			if(initResp->base_response->err_code == 0)
			{
				if(initResp->has_challeange_answer)
				{
					uint32 ret = crc32(0,g_wechat->challeange,CHALLENAGE_LENGTH);
					if(ret == initResp->challeange_answer)
					{
						g_wechat->state.init_state = TRUE;
					}

					DEBUG(("crc32:%ld ,challeange_answer:%ld\n", ret, initResp->challeange_answer));

				}
				else 
					g_wechat->state.init_state = TRUE;
				
				g_wechat->state.wechats_switch_state = TRUE;
			}
			else
			{
				result = (uint16)initResp->base_response->err_code;
			}
		}
		epb_unpack_init_response_free(initResp);
		
	}
	break;
	
	case ECI_push_recvData:
	{
		RecvDataPush *recvDatPush = epb_unpack_recv_data_push(data + fix_head_len, size_data - fix_head_len);
		KoovoxData* k_data = (KoovoxData*)recvDatPush->data.data;
				
		if(!recvDatPush)
		{
			return errorCodeUnpackRecvDataPush;
		}

#ifdef DEBUG_PRINT_ENABLED
		{
		uint16 i = 0;

		DEBUG(("obj = %x\n", k_data->obj));
		for(; i<recvDatPush->data.len; i++ )
			DEBUG(("%x ", recvDatPush->data.data[i]));

		DEBUG(("\n"));
		}
#endif
		
		switch(k_data->obj)
		{
		/* 填充自己的命令 */
		case OBJ_HEART_RATE:
		{
			if(k_data->cmd == START)
			{
				DEBUG(("Enable heart rate\n"));
			}
			else if(k_data->cmd == STOP)
			{
				DEBUG(("Disable heart rate\n"));
			}
		}
		break;
		

		default:
			break;
		}
			
		epb_unpack_recv_data_push_free(recvDatPush);
		g_wechat->state.push_data_seq++;
	}
	break;
	
	case ECI_push_switchView:
	{		
		SwitchViewPush *swichViewPush = epb_unpack_switch_view_push(data + fix_head_len, size_data - fix_head_len);		
		g_wechat->state.wechats_switch_state = !g_wechat->state.wechats_switch_state;

		if(!swichViewPush)
		{
			return errorCodeUnpackSwitchViewPush;
		}
		epb_unpack_switch_view_push_free(swichViewPush);
	}
	break;
	
	case ECI_push_switchBackgroud:
	{
		SwitchBackgroudPush *switchBackgroundPush = epb_unpack_switch_backgroud_push(data + fix_head_len, size_data - fix_head_len);

		if(! switchBackgroundPush)
		{
			return errorCodeUnpackSwitchBackgroundPush;
		}	
		
		epb_unpack_switch_backgroud_push_free(switchBackgroundPush);
	}
	break;
	
	case ECI_err_decode:
		break;
	default:
		break;
	}

	return result;
	
}


/*************************************************************************
NAME
    koovox_device_wechat_send_data
    
DESCRIPTION
    pack wechat send data request package

RETURN
    void
*/
void koovox_pack_wechat_send_data_req(uint8* data, uint16 size_data, bool has_type, EmDeviceDataType type)
{
	SendDataRequest data_request = {0};
	uint16 fix_head_len = FRAME_SIZE_FIX_HEAD;
	
	memset(&data_request, 0, sizeof(data_request));
	g_wechat->state.seq++;
	g_wechat->state.send_data_seq++;
	
	data_request.data.data = data;
	data_request.data.len = size_data;
	
	data_request.has_type = has_type;
	data_request.type = type;
	
	g_wechat->send_data.len = epb_send_data_request_pack_size(&data_request)+ fix_head_len;
	g_wechat->send_data.data = (uint8*)mallocPanic(g_wechat->send_data.len);

	koovox_pack_wechat_head(ECI_req_sendData, g_wechat->send_data.data);

	epb_pack_send_data_request(&data_request, g_wechat->send_data.data + fix_head_len, g_wechat->send_data.len - fix_head_len);
	
	g_wechat->send_data.offset = 0;
	
}


/*************************************************************************
NAME
    koovox_init_wechat
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
static void koovox_init_wechat(void)
{
	if(g_wechat == NULL)
		g_wechat = (WECHAT_DATA_T*)mallocPanic(sizeof(WECHAT_DATA_T));

	memset(g_wechat, 0, sizeof(WECHAT_DATA_T));

	g_wechat->challeange[0] = 0x11;
	g_wechat->challeange[1] = 0x22;
	g_wechat->challeange[2] = 0x33;
	g_wechat->challeange[3] = 0x44;
}

/*************************************************************************
NAME
    koovox_init_wechat
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
static void koovox_free_wechat(void)
{
	if(g_wechat)
	{
		freePanic(g_wechat);
		g_wechat = NULL;
	}
}

/*************************************************************************
NAME
    koovox_wechat_connect
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
void koovox_wechat_connect(WechatTramsportType type, Ble* ble, WECHAT_TRANSPORT* rfcomm)
{

	koovox_init_wechat();

	if(!g_wechat->state.connect_state)
	{
		g_wechat->state.connect_state = TRUE;
		g_wechat->type = type;

		/* 发送Auth 请求 */
		koovox_pack_wechat_auth_req();
		if(type == TYPE_BLE)
		{	
			g_wechat->transport.ble.cid = ble->cid;
			g_wechat->transport.ble.handle = ble->handle;
			if(theSink.rundata->gatt.server.indication == indication_none)
			{
				koovox_send_data_to_wechat();
			}
			else
			{
				/* 排队等待 */
				MessageSendLater(&theSink.task, EventKoovoxRepeatSendIndication, 0, 1000);
			}
		}
		else if(type == TYPE_RFCOMM)
		{
			g_wechat->transport.rfcomm = rfcomm;
			koovox_send_data_to_wechat();
		}
		
		g_wechat->state.auth_send = TRUE;
	}
}


/*************************************************************************
NAME
    koovox_rcv_data_from_wechat
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
void koovox_rcv_data_from_wechat(uint8* data, uint16 size_data)
{
		uint16 chunk_size = 0;
		uint16 error_code = 0;
		uint16 offset	= 0;
		
#ifdef DEBUG_PRINT_ENABLED
		{
			uint8 i = 0;
			for(; i<size_data; i++)
				DEBUG(("%x ", data[i]));
	
			DEBUG(("\n"));
		}
#endif    
	
		if(size_data < FRAME_SIZE_FIX_HEAD)
			return;
	
		while(size_data)
		{
			if((g_wechat->rcv_data.len == 0))
			{
				uint16 lenght = ((uint16)data[offset + 2] << 8) + (uint16)data[offset + 3] ;
	
				/* 包头检测 */
				if((data[0] != 0xfe)&&(data[1] != 0x01))
					return;
				
				g_wechat->rcv_data.len = lenght;
				g_wechat->rcv_data.offset = 0;
				g_wechat->rcv_data.data = (uint8*)mallocPanic(g_wechat->rcv_data.len);
	
			}
			
			chunk_size = g_wechat->rcv_data.len - g_wechat->rcv_data.offset;
			chunk_size = chunk_size < size_data ? chunk_size : size_data;
			memcpy(g_wechat->rcv_data.data + g_wechat->rcv_data.offset, data + offset, chunk_size);
			size_data -= chunk_size;
			offset += chunk_size;
			g_wechat->rcv_data.offset += chunk_size;
			
			if (g_wechat->rcv_data.len <= g_wechat->rcv_data.offset) 
			{
				/* 应答消息验证 */
				error_code = koovox_rcv_data_handle(g_wechat->rcv_data.data, g_wechat->rcv_data.len);
				
				/* 出错处理 */
				koovox_wechat_error_handle(error_code);
	
				if(g_wechat->rcv_data.data)
				{
					DEBUG(("free g_rcv_data.data\n"));
					freePanic(g_wechat->rcv_data.data);
					g_wechat->rcv_data.data = NULL;
				}
				
				g_wechat->rcv_data.len = 0;
				g_wechat->rcv_data.offset = 0;

				if(error_code)
					size_data = 0;
	
			}	
		}
}


/*************************************************************************
NAME
    koovox_send_data_to_wechat
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
void koovox_send_data_to_wechat(void)
{
	if(g_wechat->type == TYPE_BLE)
	{
		koovox_indicate_to_wechat(g_wechat->transport.ble.cid, g_wechat->transport.ble.handle);	
	}
	else if(g_wechat->type == TYPE_RFCOMM)
	{
		WechatSendPacket(g_wechat->transport.rfcomm, g_wechat->send_data.len, g_wechat->send_data.data);
	}
	
}


/*************************************************************************
NAME
    koovox_send_data_from_wechat
    
DESCRIPTION
    device disconnect to wechat

RETURN
    void
*/
void koovox_wechat_disconnect(void)
{
	koovox_free_wechat();
}



#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    

#endif

