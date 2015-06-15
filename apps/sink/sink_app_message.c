/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2014
Part of ADK 3.5

FILE NAME
    sink_app_message.c

*/

#ifdef ENABLE_KOOVOX
#include <gatt.h>

#include "sink_gatt_db.h"
#include "sink_app_core.h"
#include "sink_app_task.h"
#include "sink_app_message.h"
#include "sink_config.h"
#include "sink_audio_prompts.h"
#include <kalimba.h>


#define DEVICE_INFO  "1.0.1:A8"


void KoovoxSynSportData(void)
{
	uint8 value[8] = {0};
	uint8 status = (GetSampleStatus() ? ON : OFF);	
	uint16 time = 0;
	uint32 step = 0;
	uint16 sport_value[SPORT_DATA_SIZE] = {0};

	ConfigRetrieve(CONFIG_SPORT_DATA, sport_value, SPORT_DATA_SIZE);
	
	time = sport_value[0];
	time += GetTimeValue();
	step = ((uint32)sport_value[1] << 16) + (uint32)sport_value[2];
	step += GetStepValue();
	
	value[0] = GetCurrMode();
	value[1] = status;
	value[2] = (time >> 8)&0xff;
	value[3] = time&0xff;
	value[4] = (step >> 24)&0xff;
	value[5] = (step >> 16)&0xff;
	value[6] = (step >> 8)&0xff;
	value[7] = step&0xff;
	SendNotifyToDevice(OID_SYSTEM, 0, value, sizeof(value));
	ClearStepValue();
	ClearTimeValue();
	
	if((time > GetTimeValue())||(step > GetStepValue()))
	{
		memset(sport_value, 0, sizeof(sport_value));
		ConfigStore(CONFIG_SPORT_DATA, sport_value, SPORT_DATA_SIZE);
	}
}

/*************************************************************************
NAME
    KoovoxAppConnection
    
DESCRIPTION
    the mobile app connected the koovox
*/
void KoovoxAppConnection(Sink sink, uint16 cid, uint8 connectStatus)
{
	APP_MSG_DEBUG(("KoovoxAppConnection\n"));

	if(isGattConnected()||isSppConnected())
		return ;

	if(GATT_CONNECTED == connectStatus)
	{
		koovox.cid = cid;
		koovox.sppSink = NULL;
		koovox.getValue = (uint8*)mallocPanic(MAX_DATA_BUF);
		koovox.setValue = (uint8*)mallocPanic(MAX_DATA_BUF);
        koovox.sizeGetValue = 0;
		koovox.sizeSetValue = 0;
	}
	else
	{
		koovox.cid = 0;
		koovox.sppSink = sink;
		koovox.sppValue = (uint8*)mallocPanic(MAX_DATA_BUF);
		koovox.sizeSppValue = 0;
	}
	
	koovox.connectStatus = connectStatus;
	
	/* 同步设备状态 */
	if(SPORT_ENABLE == GetSampleStatus())
	{	
		/* stop koovox prompt heart rate value */
		SetPromptStatus(PROMPT_DISABLE);
	}	
	
	KoovoxSynSportData();
	
	/* audio prompt app connected */
	AudioPromptPlayEvent(EventKoovoxPromptAppConnected);

}

/*************************************************************************
NAME
    KoovoxAppDisconnection
    
DESCRIPTION
    the mobile app disconnected the koovox
*/
void KoovoxAppDisconnection(void)
{
	APP_MSG_DEBUG(("KoovoxAppDisconnection\n"));

	koovox.sppSink = NULL;
	koovox.cid = 0;
	koovox.handle = 0;

	if(GATT_CONNECTED == koovox.connectStatus)
	{
		freePanic(koovox.getValue);
		koovox.getValue = NULL;
		koovox.sizeGetValue = 0;
		freePanic(koovox.setValue);
		koovox.setValue = NULL;
		koovox.sizeSetValue = 0;
	}
	else
	{
		freePanic(koovox.sppValue);
		koovox.sppValue = NULL;
		koovox.sizeSppValue = 0;
	}
	
	koovox.connectStatus = DISCONNECTED;

	/* 断开初始化 */
	if((SPORT_ENABLE != GetSampleStatus())&&(SAMPLE_DISABLE != GetSampleStatus()))
	{
		KoovoxStopNeckProtect();
	}
	
	SetPresentScence(IDLE_SCENCE);
	SetNeckProtectEnable(NECK_DISABLE);
	SetDriverEnable(DRIVER_DISABLE);
	
	SetPresentAuto(VP_AUTO_DISABLE);

	if(GetSampleStatus()&&(SPORT_MODE == GetCurrMode()))
	{
		/* enable heart rate value prompt flag */
		SetPromptStatus(PROMPT_ENABLE);
		
		/* send message to start period audio prompt heart rate value to user */
		MessageSend(&koovox.task, EventKoovoxPeriodPromptHeartRate, 0);
	}

	/* audio prompt app connected */
	AudioPromptPlayEvent(EventKoovoxPromptAppDisconnected);
	
}

/*************************************************************************
NAME
    KoovoxMessageStr
    
DESCRIPTION
    check the message include str or not
*/
static void KoovoxAsciiToBcd(uint16* src, uint16 src_len, uint8* dest, uint16 dest_len)
{
	uint8 i = 0, j = 0;
	uint8 bcd_value = 0;
	APP_MSG_DEBUG(("KoovoxAsciiToBcd\n"));

	memset(dest, 0, dest_len);

	for(; i<src_len; i++)
	{
		j = i / 2;
		switch(src[i])
		{
		case '0':
			bcd_value = 1;
			break;
		case '1':
			bcd_value = 2;
			break;
		case '2':
			bcd_value = 3;
			break;
		case '3':
			bcd_value = 4;
			break;
		case '4':
			bcd_value = 5;
			break;
		case '5':
			bcd_value = 6;
			break;
		case '6':
			bcd_value = 7;
			break;
		case '7':
			bcd_value = 8;
			break;
		case '8':
			bcd_value = 9;
			break;
		case '9':
			bcd_value = 10;
			break;
		default:
			bcd_value = 0;
			break;
			
		}
		if(i%2)
		{
			dest[j] += bcd_value;
		}
		else
		{
			dest[j] += bcd_value << 4;
		}
	}

#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("BCD:"));
		for(; i < dest_len; i++)
		{
			DEBUG(("%x ", dest[i]));
		}
		DEBUG(("\n"));
	}
#endif	
	
}


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

	APP_MSG_DEBUG(("APP_MSG:DisposeSppMessage\n"));

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
    SendResponse
    
DESCRIPTION
    send response for request
*/
void SendResponse(uint16 status, uint16 cmd, uint16 value_oid, uint8 value_event, uint8* value_data, uint16 size_data)
{
	/* 判断蓝牙是否与手机连接 */
	if((isSppConnected())||(isGattConnected()))
	{
		uint16 length = sizeof(PacketResponse_t) + (size_data ? size_data - 1 : 0);
		PacketResponse_t *response = (PacketResponse_t *)mallocPanic(length);
		memset(response, 0, length);
		
		APP_MSG_DEBUG(("APP_MSG:SendResponse\n"));
		
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

	APP_MSG_DEBUG(("APP_MSG:SetRequestAction %d \n", size_value));
	
#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("msg:"));
		for(; i < size_value; i++)
		{
			DEBUG(("%x ", value[i]));
		}
		DEBUG(("\n"));
	}
#endif	
		
	/* check the set request size */
	if(size_value < PACKET_SIZE)
		return ERROR;

	oid = (uint16)req->value_oid[0] + ((uint16)req->value_oid[1] << 8);

#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("value_data:"));
		for(; i < size_value - PACKET_SIZE; i++)
		{
			DEBUG(("%x ", req->value_data[i]));
		}
		DEBUG(("\n"));
	}
#endif	

	switch(oid)
	{
	/* asr call */
	case OID_CALL:
	{
		SetResponseStatus(RSP_SUCCESS);
		/* write the number to pskey */
		ConfigStore(CONFIG_ASR_PHONE_NUMBER, req->value_data, PHONE_NUMBER_SIZE);
		MessageSend(&koovox.task, EventKoovoxAsrCall, 0);
	}
	break;

	/* mode set */
	case OID_MODE:
	/* set koovox mode and audio prompt  */
	KoovoxSetModePrompt(*(req->value_data));
	break;

	/* present scence set */
	case OID_VP_MODE:
	{
		uint8 vp_mode = 0;
		uint8 vp_auto = 0;
		
		APP_MSG_DEBUG(("OID_VP_MODE \n"));
		vp_mode = req->value_data[0];
		vp_auto = req->value_data[1];
		SetPresentScence(vp_mode);
		if(ON == vp_auto)
		{
			DEBUG(("**VP_AUTO_ENABLE\n"));
			SetPresentAuto(VP_AUTO_ENABLE);
		}
		else
		{
			DEBUG(("**VP_AUTO_DISABLE\n"));
			SetPresentAuto(VP_AUTO_DISABLE);
		}
	}
	break;

	case OID_HB:
	{
		APP_MSG_DEBUG(("OID_HB \n"));
		if(ON == *(req->value_data))
		{
			if(GetSampleStatus())
			{
				return PROGRESS;
			}
			else
			{
				KoovoxStartHeartRateMeasure();
			}
		}
		else
		{
			KoovoxStopHeartRateMeasure();
		}
	}
	break;

	case OID_BUTTON_C:
	{
		if(LONG_CLICK == req->value_event)
		{
			uint8 mode = req->value_data[0];
			uint8 action = req->value_data[1];

			if(mode != GetCurrMode())
				SetCurrMode(mode);
		
			if(ON == action)
			{
				switch(mode)
				{
				case SPORT_MODE:
					AudioPromptPlayEvent(EventKoovoxPromptStartSport);
					/* start heart rate measure */
					KoovoxStartSportMode();
					break;
				default:
					break;
				}
			}
			else if(OFF == action)
			{
				switch(mode)
				{
				case SPORT_MODE:
					/* start heart rate measure */
					KoovoxStopSportMode();
					AudioPromptPlayEvent(EventKoovoxPromptStopSport);
					break;
				default:
					break;
				}
			}
		}
	}
	break;

	/* emergency number set */
	case OID_EC_NUM:
	{
		APP_MSG_DEBUG(("OID_EMERGENCY_NUM \n"));
		/* write the number to pskey */
		ConfigStore(CONFIG_PHONE_NUMBER, req->value_data, PHONE_NUMBER_SIZE);
	}
	break;

	case OID_NECK:
	{
		APP_MSG_DEBUG(("set OID_NECK\n"));
		if(ON == *(req->value_data))
		{
			SetNeckProtectEnable(NECK_ENABLE);
			if(SAMPLE_DISABLE == GetSampleStatus())
			{
				KoovoxStartNeckProtect();
				/*AudioPromptPlayEvent(EventKoovoxPromptNectProtectEnable);*/
			}
		}
		else
		{
			SetNeckProtectEnable(NECK_DISABLE);
			if(NECK_PROTECT == GetSampleStatus())
			{
				KoovoxStopNeckProtect();
				/*AudioPromptPlayEvent(EventKoovoxPromptNectProtectDisable);*/
			}
		}
	}
	break;

	case OID_SEAT_PROMPT:
	{
		APP_MSG_DEBUG(("set OID_SEAT_PROMPT\n"));
		if(ON == *(req->value_data))
		{
			if(SAMPLE_DISABLE == GetSampleStatus())
			{
				KoovoxStartConstSeatPrompt();
				AudioPromptPlayEvent(EventKoovoxPromptNectProtectEnable);
			}
		}
		else
		{
			if(CONST_SEAT == GetSampleStatus())
			{
				KoovoxStopConstSeatPrompt();
				AudioPromptPlayEvent(EventKoovoxPromptNectProtectDisable);
			}
		}
	}
	break;

	case OID_DRIVER_ENABLE:
	{
#if 1
		APP_MSG_DEBUG(("set OID_DRIVER_ENABLE\n"));
		if(ON == *(req->value_data))
		{
			SetDriverEnable(DRIVER_ENABLE);
			if(SAMPLE_DISABLE == GetSampleStatus())
			{
				KoovoxStartSafeDriver();
				AudioPromptPlayEvent(EventKoovoxPromptSafeDriverEnable);
			}
		}
		else
		{
			SetDriverEnable(DRIVER_DISABLE);
			if(SAFE_DRIVER == GetSampleStatus())
			{
				KoovoxStopSafeDriver();
				AudioPromptPlayEvent(EventKoovoxPromptSafeDriverDisable);
			}
		}
#else
		if(ON == *(req->value_data))
		{
			DEBUG(("set OID_DRIVER_ENABLE\n"));
			SetActionCallEnable(ACTION_CALL_ENABLE);
		}
		else
		{
			DEBUG(("set OID_DRIVER_DISABLE\n"));
			SetActionCallEnable(ACTION_CALL_DISABLE);
		}
#endif
	}		
	break;

	case OID_VP_ENABLE:
	{
		APP_MSG_DEBUG(("set OID_VP_ENABLE\n"));
		if(ON == *(req->value_data))
		{
			SetPresentEnable(VP_ENABLE);
		}
		else
		{
			SetPresentEnable(VP_DISABLE);
		}
	}
	break;

	case OID_VP_AUTO:
	{
		APP_MSG_DEBUG(("set OID_VP_AUTO\n"));
		if(ON == *(req->value_data))
		{
			SetPresentAuto(VP_AUTO_ENABLE);
		}
		else
		{
			SetPresentAuto(VP_AUTO_DISABLE);
		}
	}
	break;

	case OID_VM_ENABLE:
	{
		APP_MSG_DEBUG(("set OID_VM_ENABLE\n"));
		if(ON == *(req->value_data))
		{
			SetVoiceMessageEnable(VM_ENABLE);
		}
		else
		{
			SetVoiceMessageEnable(VM_DISABLE);
		}
	}
	break;

	case OID_DFU:
	{
		APP_MSG_DEBUG(("set dfu \n"));
		MessageSend(&theSink.task, EventUsrGaiaDFURequest, 0);
	}
	break;

	case OID_PROMPT_TIME:
	{
		uint8 hour = 0;
		uint8 min = 0;
		hour = req->value_data[0];
		min = req->value_data[1];
		APP_MSG_DEBUG(("OID_PROMPT_TIME %2d:%2d\n", hour, min));
		KoovoxAudioPromptTime(hour, min);
	}
	break;

	case OID_PROMPT_SPORT:
	{
		uint8 hour = 0;
		uint8 min = 0;
		uint16 mileage = 0;		/* 单位 米 */

		mileage = ((uint16)req->value_data[0] << 8) + (uint16)req->value_data[1];
		hour = req->value_data[2];
		min = req->value_data[3];
		APP_MSG_DEBUG(("OID_PROMPT_SPORT=%d- %2d:%2d\n",mileage, hour, min));
		KoovoxAudioPromptSportData(mileage, hour, min);
	}
	
	default:
		break;
	}
	
    return SUCCESS;
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

	APP_MSG_DEBUG(("APP_MSG:GetRequestAction\n"));
#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("msg:"));
		for(; i < size_value; i++)
		{
			DEBUG(("%x ", value[i]));
		}
		DEBUG(("\n"));
	}
#endif	
		
	/* check the set request size */
	if(size_value < PACKET_SIZE)
		return ERROR;
	
	oid =(uint16)req->value_oid[0] + ((uint16)req->value_oid[1] << 8); 
		
	switch(oid)
	{	
	/* get koovox mode */
	case OID_MODE:
	value_data = GetCurrMode();
	/* response the koovox mode */
	SendResponse(SUCCESS, CMD_GET, OID_MODE, 0, &value_data, 1);
	break;

	/* get emergency number */
	case OID_EC_NUM:
	{
	/* response emergency number */
	uint16 phone_number[PHONE_NUMBER_SIZE] = {0};
	memset(phone_number, 0, sizeof(phone_number));
	ConfigRetrieve(CONFIG_PHONE_NUMBER , phone_number, PHONE_NUMBER_SIZE);
	if(isSppConnected())
	{
		SendResponse(SUCCESS, CMD_GET, OID_EC_NUM, 0, (uint8*)phone_number, PHONE_NUMBER_SIZE);
	}
	else
	{
		uint8 bcd_phone_num[PHONE_NUMBER_SIZE/2] = {0};
		KoovoxAsciiToBcd(phone_number, PHONE_NUMBER_SIZE,bcd_phone_num, PHONE_NUMBER_SIZE/2);
		SendResponse(SUCCESS, CMD_GET, OID_EC_NUM, 0, bcd_phone_num, PHONE_NUMBER_SIZE/2);
	}
	}		
	break;

	/* get battery value */
	case OID_BAT:				
	/* get the battery value */
	UpdateBatteryValue(get_battery_level_as_percentage());
	value_data = GetBatteryValue();
	SendResponse(SUCCESS, CMD_GET, OID_BAT, 0, &value_data, 1);
	break;

	case OID_DEVICE_INFO:
	{
	uint8 info[10] = {0};
	memcpy(info, DEVICE_INFO, strlen(DEVICE_INFO));
	SendResponse(SUCCESS, CMD_GET, OID_DEVICE_INFO, 0, info, strlen(DEVICE_INFO));
	}
	break;	
	
	/* get heart rate value */
	case OID_HB:
	value_data = GetHeartRateValue();
	SendResponse(SUCCESS, CMD_GET, OID_HB, ON, &value_data, 1);
	break;
		
	/* get accelerate angle value */
	case OID_STEP:
	{
	uint8 step[4] = {0};
	uint32 value = GetStepValue();
	step[0] = (value>> 24) & 0xff;
	step[1] = (value >> 16) & 0xff;
	step[2] = (value >> 8) & 0xff;
	step[3] = value & 0xff;
	SendResponse(SUCCESS, CMD_GET, OID_STEP, ON, step, 4);
	}
	break;

	case OID_NECK:
	{
	uint8 value = 0;
	value = koovox.neckEnable;
	SendResponse(SUCCESS, CMD_GET, OID_NECK, 0, &value, 1);
	}
	break;

	case OID_DRIVER_ENABLE:
	{
	uint8 value = 0;
	value = koovox.driverEnable;
	SendResponse(SUCCESS, CMD_GET, OID_DRIVER_ENABLE, 0, &value, 1);
	}
	break;

	case OID_VM_ENABLE:
	{
	uint8 value = 0;
	value = koovox.vm_enable;
	SendResponse(SUCCESS, CMD_GET, OID_VM_ENABLE, 0, &value, 1);
	}
	break;
	
	default:
	{
	if(isGattConnected())
		GattAccessResponse(koovox.cid, koovox.handle, gatt_status_success, 0, 0);
	}
	break;
		
	}

    return SUCCESS;
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

	APP_MSG_DEBUG(("APP_MSG:SendMessageWithSink\n"));
	
	offset = SinkClaim(sink, size);
	if (offset == 0xFFFF)
	{
		APP_MSG_DEBUG(("APP_MSG:the sink has not more space:size=%x !\n", size));
	}
	else
	{
		memcpy(SinkMap(sink) + offset, msg, size);
		SinkFlush(sink,size);
		APP_MSG_DEBUG(("APP_MSG:the sink send msg suc!\n"));
	}
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

#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("msg:"));
		for(; i < length; i++)
		{
			DEBUG(("%x ", msg[i]));
		}
		DEBUG(("\n"));
	}
#endif	

	APP_MSG_DEBUG(("APP_MSG:SendResponseToDevice:%d\n", length));

	if(isSppConnected())
	{
		SendMessageWithSink(koovox.sppSink, msg, size + 2);
	}
	else if(isGattConnected())
	{
		GattAccessResponse(koovox.cid, koovox.handle, gatt_status_success, length, msg);            
	}	

	freePanic(msg);
	msg = NULL;
}

/*************************************************************************
NAME
    SendNotifyToDevice
    
DESCRIPTION
    send notify to device
*/
void SendNotifyToDevice(uint16 value_oid, uint8 value_event, uint8* value_data, uint16 size_data)
{
	uint16 length = sizeof(NotifyRequest_t) + (size_data ? size_data - 1 : 0);
	NotifyRequest_t *req = (NotifyRequest_t *)mallocPanic(length);
	memset(req, 0, length);

	APP_MSG_DEBUG(("APP_MSG:SendNotifyToDevice\n"));

	/* fill the req */
	req->cmd[0] = (CMD_NOTIFY >> 8) & 0xff;
	req->cmd[1] = CMD_NOTIFY & 0xff;
	req->tid[0] = (CMD_NOTIFY >> 8) & 0xff;
	req->tid[1] = CMD_NOTIFY & 0xff;
	req->size[0] = ((length - INDEX_ELEMENT)>> 8) & 0xff;
	req->size[1] = (length - INDEX_ELEMENT) & 0xff;
	req->id_oid = ID_OID;
	req->value_oid[0] = value_oid & 0xff;
	req->value_oid[1] = (value_oid >> 8) & 0xff;
	req->id_event = ID_EVENT;
	req->value_event = value_event;
	req->id_data = ID_DATA;
	if(value_data)
		memmove(req->value_data, value_data, size_data);

	SendMessageToDevice((uint8*)req, length);

	free(req);
	req = NULL;

}

/*************************************************************************
NAME
    SendMessageToDevice
    
DESCRIPTION
    send message to device
*/
void SendMessageToDevice(uint8* msg, uint16 size)
{
	uint16 length = size + strlen(FRAME_TAIL);
	uint8* buff = (uint8*)mallocPanic(length);
	memcpy(buff, msg, size);
	memcpy(buff + size, FRAME_TAIL, strlen(FRAME_TAIL));

	APP_MSG_DEBUG(("APP_MSG:SendMessageToDevice\n"));

	if(isSppConnected())
	{
		SendMessageWithSink(koovox.sppSink, buff, length);
	}
	else if(isGattConnected())
	{
#if defined(GATT_SERVER_ENABLED) && defined(GATT_SERVER_BAS_ENABLED)
		send_notify_characteristic_to_device(buff, length);
#endif
	}	

#ifdef DEBUG_APP_MESSAGE
	{
		uint8 i = 0;
		DEBUG(("msg:"));
		for(; i < length; i++)
		{
			DEBUG(("%x ", buff[i]));
		}
		DEBUG(("\n"));
	}
#endif	

	freePanic(buff);
	buff = NULL;
}


#if defined(GATT_SERVER_ENABLED) && defined(GATT_SERVER_BAS_ENABLED)
/*************************************************************************
NAME
    send_notify_characteristic_to_device
    
DESCRIPTION
    send notify to device 
*/
void send_notify_characteristic_to_device(uint8* notify, uint16 size)
{
	APP_MSG_DEBUG(("APP_MSG:send_notify_characteristic_to_device \n"));

	/* Send a notification to the device */
	GattNotificationRequest(&theSink.rundata->gatt.task, koovox.cid, HANDLE_NOTIFY_CHARACTERISTIC, size, notify);
}

/*************************************************************************
NAME
    DisposeGattGetCharacteristicValue

DESCRIPTION
    dispose the get characteristic   
*/
void DisposeGattGetCharacteristicValue(GATT_ACCESS_IND_T* ind)
{
	uint16 ret = 0;
	uint16 oid = 0;
	APP_MSG_DEBUG(("APP_MSG:DisposeGattGetCharacteristicValue \n"));

	if(koovox.sizeGetValue)
	{
		ret = KoovoxMessageStr(koovox.getValue, FRAME_TAIL, koovox.sizeGetValue);
		
		if(ret)
		{
			/* dispose get request */
			GetRequestAction(koovox.getValue, koovox.sizeGetValue - 2);
		}
		else
		{
			oid =(uint16)koovox.getValue[1] + ((uint16)koovox.getValue[2] << 8); 
			SendResponse(ERROR, CMD_GET, oid, 0, 0, 0);
		}

		koovox.sizeGetValue = 0;
		memset(koovox.getValue, 0, MAX_DATA_BUF);
	}
	else
	{
		GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
	}
	

}

/*************************************************************************
NAME
    DisposeGattSetCharacteristicValue

DESCRIPTION
    dispose the set characteristic   
*/
void DisposeGattSetCharacteristicValue(GATT_ACCESS_IND_T* ind)
{	
	uint16 ret = 0;
	uint16 oid = 0;
	APP_MSG_DEBUG(("APP_MSG:DisposeGattSetCharacteristicValue \n"));

	if(koovox.sizeSetValue)
	{
		ret = KoovoxMessageStr(koovox.setValue, FRAME_TAIL, koovox.sizeSetValue);
		oid =(uint16)koovox.setValue[1] + ((uint16)koovox.setValue[2] << 8); 

		if(ret)
		{
			/* dispose set request */
			ret = SetRequestAction(koovox.setValue, koovox.sizeSetValue - 2);
			/* response set request */
			SendResponse(ret, CMD_SET, oid, 0, 0, 0);
		}
		else
		{
			SendResponse(ERROR, CMD_SET, oid, 0, 0, 0);
		}

		koovox.sizeSetValue = 0;
		memset(koovox.setValue, 0, MAX_DATA_BUF);
	}
	else
	{
		GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
	}
	
}
#endif /* GATT_SERVER_ENABLED &&  GATT_SERVER_BAS_ENABLED */


#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    
#endif /* ENABLE_KOOVOX */

