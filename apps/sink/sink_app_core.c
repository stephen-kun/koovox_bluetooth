/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2014
Part of ADK 3.5

FILE NAME
    sink_app_core.c

*/

#ifdef ENABLE_KOOVOX

#include <kalimba.h>
#include <vm.h>
#include <csr_voice_presences_plugin.h>
#include <koovox_sport_common_plugin.h>
#include <micbias.h>
#include <pio.h>
#include <pio_common.h>


#include "sink_app_task.h"
#include "sink_app_core.h"
#include "sink_app_message.h"
#include "sink_koovox_uart.h"
#include "sink_events.h"
#include "sink_statemanager.h"
#include "sink_audio_prompts.h"
#include "sink_tones.h"
#include "sink_pio.h"
#include "sink_callmanager.h"
#include "sink_config.h"

#include "sink_heart_rate_calc.h"


/****************************************************************************
FUNCTIONS pActionHandle
*/
static void KoovoxSwitchToNextMode(void);
static void KoovoxPeriodPromptHeartRate(void);
static void KoovoxEmergencyCall(void);
static void KoovoxAnswerCall(void);
static void KoovoxCancelCall(void);
static void KoovoxPresetCall(void);
static void KoovoxRejectCall(void);
static void KoovoxRecordCall(void);
static void KoovoxHangOffCall(void);
static void KoovoxPromptPrecence(void);
static void KoovoxRecoverPreviousMode(void);
static void KoovoxSmartAudioPrompt(void);
static void KoovoxStartAutoSpeechRecognition(void);
static void KoovoxStopAutoSpeechRecognition(void);
static void KoovoxPlayPauseMusic(void);
static void KoovoxPlayNext(void);
static void KoovoxPlayPrevious(void);
static void KoovoxSportAudioPrompt(void);
static void KoovoxStartStopSportMode(void);
static void KoovoxAsrCall(void);
static void KoovoxRssiPairMode(void);
static void KoovoxMuteMicrophone(void);
static void KoovoxConfirmPhoneNumber(void);


/****************************************************************************
* instance event handler
*/

/* business mode */
const static ModeFSMActionHandle_t businessMode[] = {
	{EventKoovoxButtonCClick, NULL, KoovoxSmartAudioPrompt}, 		/*  offline voice prompt or online voice prompt */
	{EventKoovoxButtonCDoubleClick, isConnectableState, KoovoxRssiPairMode}, 	/* bluetooth into rssi pair mode */
	{EventKoovoxButtonMPress, NULL, KoovoxStartAutoSpeechRecognition},			/* Automatic Speech Recognition or audio prompt:app isn't connected */
	{EventKoovoxButtonMClick, NULL, KoovoxSwitchToNextMode}, 		/*  switch to next mode */
	{EventKoovoxButtonMDoubleClick, NULL, KoovoxEmergencyCall}, 	/*  emergency call */
	{EventKoovoxPeriodPromptHeartRate, NULL, KoovoxPeriodPromptHeartRate},/* period prompt heart rate value */ 
};

/* music mode */
const static ModeFSMActionHandle_t musicMode[] = {
	{EventKoovoxButtonCClick, NULL,KoovoxPlayNext },			/* music play or pause */
	{EventKoovoxButtonCDoubleClick, NULL, KoovoxPlayPrevious },	    /* play next */
	{EventKoovoxButtonCLongClick, NULL,KoovoxPlayPauseMusic },		/* play previous */
	{EventKoovoxButtonMPress, NULL, KoovoxStartAutoSpeechRecognition},			/* Automatic Speech Recognition or audio prompt:app isn't connected */
	{EventKoovoxButtonMClick, NULL, KoovoxSwitchToNextMode},		/*	switch to next mode */
	{EventKoovoxButtonMDoubleClick, NULL, KoovoxEmergencyCall}, 	/*	emergency call */
	{EventKoovoxPeriodPromptHeartRate, NULL, KoovoxPeriodPromptHeartRate},/* period prompt heart rate value */ 
};

/* sport mode */
const static ModeFSMActionHandle_t sportMode[] = {
	{EventKoovoxButtonCClick, NULL, KoovoxSportAudioPrompt},/* start or stop sport mode */
	{EventKoovoxButtonCLongClick, NULL, KoovoxStartStopSportMode},/* voice prompt:heart rate and distance */
	{EventKoovoxButtonMPress, NULL, KoovoxStartAutoSpeechRecognition},			/* Automatic Speech Recognition or audio prompt:app isn't connected */
	{EventKoovoxButtonMClick, NULL, KoovoxSwitchToNextMode},		/*	switch to next mode */
	{EventKoovoxButtonMDoubleClick, NULL, KoovoxEmergencyCall}, 	/*	emergency call */
	{EventKoovoxPeriodPromptHeartRate, NULL, KoovoxPeriodPromptHeartRate},/* period prompt heart rate value */ 
};

/* call mode (auto) */
const static ModeFSMActionHandle_t callMode[] = {
	{EventKoovoxButtonCClick, isIncomingState, KoovoxAnswerCall},		/*  call answer */
	{EventKoovoxButtonCClick, isOutgoingState, KoovoxCancelCall},		/*  call cancel */
	{EventKoovoxButtonCClick, isActiveState, KoovoxHangOffCall},		/* call hang off */
	{EventKoovoxButtonCDoubleClick, isIncomingState, KoovoxPresetCall}, /* call presence */
	{EventKoovoxButtonCDoubleClick, isActiveState, KoovoxMuteMicrophone}, /* mute the mic */
	{EventKoovoxButtonCLongClick, isIncomingState, KoovoxRejectCall},	/* call reject */
	{EventKoovoxButtonCLongClick, isActiveState, KoovoxRecordCall},	/* call record start/stop */
	{EventKoovoxAutoLeaveCallMode, NULL, KoovoxRecoverPreviousMode},	/* recover to previous mode */
	{EventKoovoxPromptPresence, NULL, KoovoxPromptPrecence},	/* audio prompt presence */
};

/* asr mode */
const static ModeFSMActionHandle_t asrMode[] = {
	{EventKoovoxButtonCClick, NULL, KoovoxConfirmPhoneNumber},/* confirm asr phone number */
	{EventKoovoxButtonMPress, NULL, KoovoxStartAutoSpeechRecognition},			/* Automatic Speech Recognition or audio prompt:app isn't connected */
	{EventKoovoxButtonMRelease, NULL, KoovoxStopAutoSpeechRecognition},			/* stop Automatic Speech Recognition  */
	{EventKoovoxButtonMClick, NULL, KoovoxSwitchToNextMode},		/*	switch to next mode */
	{EventKoovoxAsrCall, NULL, KoovoxAsrCall},	/* call asr phone number */ 
};


void logTest(void)
{
	uint16 data[10] = {956, 856, 432, 782, 489,
						795, 965, 732, 856, 635};
	char* log_msg = (char*)mallocPanic(55);
	uint8 i = 0;
	for(; i<10; i++)
		sprintf(log_msg + 5*i,"%4d ", data[i]);
	sprintf(log_msg + 5*i, "\n");
	APP_CORE_LOG((log_msg));
	freePanic(log_msg);
	log_msg = NULL;
	
}

/*************************************************************************
NAME
    KoovoxLog
    
DESCRIPTION
    send log message to mobile phone 
*/
void KoovoxLog(const char* value_data)
{
	uint16 size_data = 0;
	if(value_data == NULL)
		return ;

	size_data = strlen(value_data);
		
	if(isSppConnected())
	{
		uint16 length = sizeof(NotifyRequest_t) + (size_data ? size_data - 1 : 0) + strlen(FRAME_TAIL);
		NotifyRequest_t *req = (NotifyRequest_t *)mallocPanic(length);
		memset(req, 0, length);
				
		/* fill the req */
		req->cmd[0] = (CMD_NOTIFY >> 8) & 0xff;
		req->cmd[1] = CMD_NOTIFY & 0xff;
		req->tid[0] = (CMD_NOTIFY >> 8) & 0xff;
		req->tid[1] = CMD_NOTIFY & 0xff;
		req->size[0] = ((length - INDEX_ELEMENT - 2)>> 8) & 0xff;
		req->size[1] = (length - INDEX_ELEMENT - 2) & 0xff;
		req->id_oid = ID_OID;
		req->value_oid[0] = OID_LOG & 0xff;
		req->value_oid[1] = (OID_LOG >> 8) & 0xff;
		req->id_event = ID_EVENT;
		req->value_event = 0;
		req->id_data = ID_DATA;
		if(value_data)
		{
			memmove(req->value_data, value_data, size_data);
			memcpy(req->value_data + size_data, FRAME_TAIL, strlen(FRAME_TAIL));
		}
		
		SendMessageWithSink(koovox.sppSink, req, length);

		free(req);
		req = NULL;
	}
}

/*************************************************************************
NAME
    isIncomingState
    
DESCRIPTION
    chech device is incoming state 
*/
bool isIncomingState(void)
{
	if(deviceIncomingCallEstablish == stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isOutgoingState
    
DESCRIPTION
    chech device is outgoing state 
*/
bool isOutgoingState(void)
{
	if(deviceOutgoingCallEstablish == stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isActiveState
    
DESCRIPTION
    chech device is active state 
*/
bool isActiveState(void)
{
	if(deviceActiveCallSCO == stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isConnectedState
    
DESCRIPTION
    chech the device connected 
*/
bool isConnectableState(void)
{
	if(deviceConnDiscoverable >= stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}


/*************************************************************************
NAME
    isConnectedState
    
DESCRIPTION
    chech the device connected 
*/
bool isConnectedState(void)
{
	if(deviceConnected <= stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isA2dpStreamState
    
DESCRIPTION
    chech the device is a2dpstreaming state
*/
bool isA2dpStreamState(void)
{
	if(deviceA2DPStreaming == stateManagerGetState())
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    KoovoxTimeoutHandler
    
DESCRIPTION
    dispose the EventKoovoxTimeout event
*/
void KoovoxTimeoutHandler(void)
{
	APP_CORE_DEBUG(("KoovoxTimeoutHandler\n")) ;

	if(isResponseTimeout())
	{
		APP_CORE_DEBUG(("++++Timeout\n")) ;
		/* reset the response status */
		SetResponseStatus(RSP_SUCCESS);

		if(ASR_MODE == GetCurrMode())
		{
			RecoverPreviousMode();
		}
	}
}

/*************************************************************************
NAME
    KoovoxConfirmPhoneNumber
    
DESCRIPTION
    confirm the asr phone number
*/
static void KoovoxConfirmPhoneNumber(void)
{
	uint8 value = ASR_MODE;
	
	APP_CORE_DEBUG(("KoovoxConfirmPhoneNumber\n")) ;

	/* send ASR phone number confirm  event to mobile */
	SendNotifyToDevice(OID_BUTTON_C, CLICK, &value, 1);
}

/*************************************************************************
NAME
    KoovoxRestartDsp
    
DESCRIPTION
    reload the kalimba file
*/
void KoovoxRestartDsp(void)
{
	APP_CORE_DEBUG(("KoovoxRestartDsp\n")) ;

	if(GetSampleStatus())
	{
		TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
		uint8 sensor_sample = 0;
		uint16 sensor_value = 0;
		
		switch(GetSampleStatus())
		{
		case SPORT_ENABLE:
			sensor_sample = HB_SENSOR + ACC_SENSOR;
			sensor_value = STEP_VALUE;
			if(!isOnlineState())
				sensor_value += TIME_VALUE;
		break;
		
		case HB_ENABLE:
			sensor_sample = HB_SENSOR;
		break;
		
		case NECK_PROTECT:
			sensor_sample = ACC_SENSOR;
			sensor_value = NECK_EVENT;
		break;
		
		case SAFE_DRIVER:
			sensor_sample = ACC_SENSOR;
			sensor_value = DRIVER_EVENT;
		break;

		case CONST_SEAT:
			sensor_sample = ACC_SENSOR;
			sensor_value = SEAT_EVENT;
		break;
		
		}
		
		AudioSportModePlayPrompt( task, sensor_sample, sensor_value, &theSink.task);
	}
}

/*************************************************************************
NAME
    KoovoxMuteMicrophone
    
DESCRIPTION
    mute the microphone
*/
static void KoovoxMuteMicrophone(void)
{
	APP_CORE_DEBUG(("KoovoxMuteMicrophone\n"));

	if(isMuteEnable())
	{
		MessageSend(&theSink.task, EventUsrMuteOn, 0);
		SetMuteStatus(MUTE_DISABLE);
	}
	else
	{
		MessageSend(&theSink.task, EventUsrMuteOff, 0);
		SetMuteStatus(MUTE_ENABLE);
	}

}

/*************************************************************************
NAME
    KoovoxStartNeckProtect
    
DESCRIPTION
    
*/
void KoovoxStartNeckProtect(void)
{
	APP_CORE_DEBUG(("KoovoxStartNeckProtect\n")) ;

	/* enable the health mode */
	SetSampleStatus(NECK_PROTECT);

#if 1
	{
		KoovoxFillAndSendUartPacket(START, OBJ_HEAD_ACTION, 0, 0);	
	}
#else
	{
	TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
	uint8 sensor_sample = ACC_SENSOR;
	uint16 sensor_value = NECK_EVENT;

	AudioSportModePlayPrompt( task, sensor_sample, sensor_value, &theSink.task);
	}
#endif


}


/*************************************************************************
NAME
    KoovoxStopNeckProtect
    
DESCRIPTION
    
*/
void KoovoxStopNeckProtect(void)
{	
	APP_CORE_DEBUG(("KoovoxStopNeckProtect\n")) ;

	/* disable the health mode */
	SetSampleStatus(SAMPLE_DISABLE);

#if 1
	{
		DEBUG(("KoovoxStopNeckProtect\n"));
		KoovoxFillAndSendUartPacket(STOP, OBJ_HEAD_ACTION, 0, 0);	
	}
#else
	{
		TaskData * task    = NULL;
		task = (TaskData *) &koovox_sport_common_plugin;
		AudioSportModeStopPrompt(task);
	}

#endif

	
}


/*************************************************************************
NAME
    KoovoxResultI2cTest
    
DESCRIPTION
    
*/
void KoovoxResultI2cTest(uint8* data, uint8 size_data)
{
	uint16 value = 0;
	ConfigRetrieve(CONFIG_ACC_I2C_TEST_RESULT,&value, 1);
	DEBUG(("acc_i2c result:%d pskey_value:%d\n", data[0], value));
	if(value != (uint16)data[0])
		ConfigStore(CONFIG_ACC_I2C_TEST_RESULT, (const void*)data, 1);

	if(data[0])
	{
		/*启动计步功能*/
		KoovoxFillAndSendUartPacket(START, OBJ_STEP_COUNT, 0, 0);
		APP_CORE_LOG(("I2C suc\n"));
	}
	else
		APP_CORE_LOG(("I2C failed\n"));
}


/*************************************************************************
NAME
    KoovoxStartConstSeatPrompt
    
DESCRIPTION
    
*/
void KoovoxStartConstSeatPrompt(void)
{
	APP_CORE_DEBUG(("KoovoxStartConstSeatPrompt\n")) ;

#if 0
	/* enable the health mode */
	SetSampleStatus(CONST_SEAT);

	{
	TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
	uint8 sensor_sample = ACC_SENSOR;
	uint16 sensor_value = SEAT_EVENT;

	AudioSportModePlayPrompt( task, sensor_sample, sensor_value, &theSink.task);
	}
#else
	KoovoxFillAndSendUartPacket(START, OBJ_I2C_TEST, 0, 0);
#endif

}


/*************************************************************************
NAME
    KoovoxStopConstSeatPrompt
    
DESCRIPTION
    
*/
void KoovoxStopConstSeatPrompt(void)
{	
	APP_CORE_DEBUG(("KoovoxStopConstSeatPrompt\n")) ;

	/* disable the health mode */
	SetSampleStatus(SAMPLE_DISABLE);

	{
		TaskData * task    = NULL;
		task = (TaskData *) &koovox_sport_common_plugin;
		AudioSportModeStopPrompt(task);
	}
	
}


/*************************************************************************
NAME
    KoovoxStartSafeDriver
    
DESCRIPTION
    
*/
void KoovoxStartSafeDriver(void)
{
	APP_CORE_DEBUG(("KoovoxStartSafeDriver\n")) ;

	/* enable the health mode */
	SetSampleStatus(SAFE_DRIVER);

	{
	TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
	uint8 sensor_sample = ACC_SENSOR;
	uint16 sensor_value = NECK_EVENT;
	
	AudioSportModePlayPrompt( task, sensor_sample, sensor_value, &theSink.task);
	}

}


/*************************************************************************
NAME
    KoovoxStopSafeDriver
    
DESCRIPTION
    
*/
void KoovoxStopSafeDriver(void)
{	
	APP_CORE_DEBUG(("KoovoxStopSafeDriver\n")) ;

	/* disable the health mode */
	SetSampleStatus(SAMPLE_DISABLE);

	{
		TaskData * task    = NULL;
		task = (TaskData *) &koovox_sport_common_plugin;
		AudioSportModeStopPrompt(task);
	}
	
}


/*************************************************************************
NAME
    KoovoxRssiPairMode
    
DESCRIPTION
    
*/
static void KoovoxRssiPairMode(void)
{
	APP_CORE_DEBUG(("KoovoxRssiPairMode\n")) ;

	MessageSend(&theSink.task, EventUsrRssiPair, 0);
}

/****************************************************************************
NAME    
    KoovoxAsrCall
    
DESCRIPTION
	Dials a number auto speech recognition
    If HFP and connected - issues command
    If HFP and not connected - connects and issues if not in call
    If HSP sends button press


RETURNS
    void
*/
static void KoovoxAsrCall (void)
{	   
    uint16 ret_len;
	Sink   sink;
    uint16 phone_number_key[PHONE_NUMBER_SIZE];
    
	APP_CORE_DEBUG(("KoovoxAsrCall\n")) ;

	RecoverPreviousMode();
    
    if ((ret_len = ConfigRetrieve(CONFIG_ASR_PHONE_NUMBER, phone_number_key, PHONE_NUMBER_SIZE )))
	{        
        if((HfpLinkGetSlcSink(hfp_primary_link, &sink)) && SinkIsValid(sink))
        {
            /* Send the dial request now */
			APP_CORE_DEBUG(("Dial ASR Number (Connected) len=%x\n",ret_len)) ;	
            HfpDialNumberRequest(hfp_primary_link, ret_len, (uint8 *)&phone_number_key[0]);  
        }
        else
        {
            /* Not connected, connect and queue the dial request */
            MessageSend ( &theSink.task , EventUsrEstablishSLC , 0 ) ;
            sinkQueueEvent( EventKoovoxAsrCall ) ;
        }
    }
	
}

/*************************************************************************
NAME
    KoovoxStartStopSportMode
    
DESCRIPTION
    start or stop sport mode 
*/
static void KoovoxStartStopSportMode(void)
{
	uint8 value = 0;
	APP_CORE_DEBUG(("KoovoxStartStopSportMode\n"));

	if(SPORT_ENABLE == GetSampleStatus())
	{
		if(isOnlineState())
		{
			value = OFF;
			SendNotifyToDevice(OID_BUTTON_C, LONG_CLICK, &value,1);
		}
		else
		{
			uint16 time = 0;
			uint32 step = 0;
			uint16 sport_value[SPORT_DATA_SIZE] = {0};
			/* 存取运动数据到flash */
			ConfigRetrieve(CONFIG_SPORT_DATA, sport_value, SPORT_DATA_SIZE);

			time = sport_value[0];
			time += GetTimeValue();
			step = ((uint32)sport_value[1] << 16) + (uint32)sport_value[2];
			step += GetStepValue();
			sport_value[0] = time;
			sport_value[1] = (step >> 16) &0xffff;
			sport_value[2] = step &0xffff;
			ConfigStore(CONFIG_SPORT_DATA, sport_value, SPORT_DATA_SIZE);		
		}
		/* stop sport mode */
		KoovoxStopSportMode();
		AudioPromptPlayEvent(EventKoovoxPromptStopSport);
	}
	else
	{
		AudioPromptPlayEvent(EventKoovoxPromptStartSport);
		/* start sport mode */
		KoovoxStartSportMode();

		if(isOnlineState())
		{
			value = ON;
			SendNotifyToDevice(OID_BUTTON_C, LONG_CLICK, &value,1);
		}
	}

}

/*************************************************************************
NAME
    KoovoxSwitchToNextMode
    
DESCRIPTION
    koovox switch to next mode 
*/
static void KoovoxSwitchToNextMode(void)
{
	ModeFSM nextMode = BUSINESS_MODE;
	APP_CORE_DEBUG(("KoovoxSwitchToNextMode\n"));

	switch(GetCurrMode())
	{
		case BUSINESS_MODE:
			if(isConnectedState())
			{
				/* switch to music mode */
				nextMode = MUSIC_MODE;
				SetMusicStatus(MUSIC_MANUAL);
			}
			else 
			{
				/* switch to sport mode */
				nextMode = SPORT_MODE;				
			}
			break;
			
		case MUSIC_MODE:
			/* switch to sport mode */
			nextMode = SPORT_MODE;
			SetMusicStatus(MUSIC_AUTO);
			break;

		case SPORT_MODE:
			/*switch to idle mode */
			nextMode = BUSINESS_MODE;
			break;

		case ASR_MODE:
			/*switch to idle mode */
			nextMode = BUSINESS_MODE;
			break;

		default:
			break;
	}

	KoovoxSetModePrompt(nextMode);

	/* send button m event to mobile ,synchronization the koovox mode */
	if(isOnlineState())
	{
		SendNotifyToDevice(OID_BUTTON_M, CLICK, (uint8*)&nextMode, 1);
	}
	
}

/*************************************************************************
NAME
    KoovoxSmartAudioPrompt
    
DESCRIPTION
    smart audio prompt
*/
static void KoovoxSmartAudioPrompt(void)
{	
	APP_CORE_DEBUG(("KoovoxSmartAudioPrompt\n"));

	/* get the battery value */
	UpdateBatteryValue(get_battery_level_as_percentage());
	
	if(isOnlineState())
	{
		uint8 value[2] = {0};
		value[0] = BUSINESS_MODE;
		value[1] = koovox.batteryValue;
		
		/* send notify to mobile smart audio prompt */
		SendNotifyToDevice(OID_BUTTON_C, CLICK, value, 2);
	}
	else
	{

		if(!isConnectableState())
		{
			/* 手机已连接 ，应用未连接 */
			AudioPromptPlayEvent(EventKoovoxPromptPhoneConnected);
			AudioPromptPlayEvent(EventKoovoxPromptAppDisconnected);
		}
		else
		{
			/* 手机未连接 */
			AudioPromptPlayEvent(EventKoovoxPromptPhoneDisconnected);
		}

		/* prompt the battery state */
		KoovoxPromptPlayBattery(koovox.batteryValue);
	}

}

/*************************************************************************
NAME
    KoovoxSmartAudioPrompt
    
DESCRIPTION
    audio prompt sport message
*/
static void KoovoxSportAudioPrompt(void)
{
	uint8 value = 0;
	APP_CORE_DEBUG(("ActRequestMobilePrompts\n"));

	if(isOnlineState())
	{
		value = SPORT_MODE;
		/* send notify to device play the heart rate value and other message */
		SendNotifyToDevice(OID_BUTTON_C, CLICK, &value, 1);
	}
	else
	{
		/* 播报心率 */
		KoovoPromptPlayHeartRate(koovox.heartRateValue);
	}
}


/*************************************************************************
NAME
    KoovoxStartAutoSpeechRecognition
    
DESCRIPTION
    koovox start automatic speech recognition
*/
static void KoovoxStartAutoSpeechRecognition(void)
{
	APP_CORE_DEBUG(("KoovoxStartAutoSpeechRecognition\n"));

	if(isOnlineState())
	{
		SetAsrStatus(ASR_BUSY);
		
		if(ASR_MODE != GetCurrMode())
		{
			StorePreviousMode(GetCurrMode());
			SetCurrMode(ASR_MODE);
		}
				
		/* send ASR start event to mobile */
		SendNotifyToDevice(OID_BUTTON_M, PRESS, 0, 1);
	}
	else 
	{
		/* app 未连接，无法使用语言拨号 */
		AudioPromptPlayEvent(EventKoovoxPromptAppDisconnected);
	}
}

/*************************************************************************
NAME
    KoovoxStopAutoSpeechRecognition
    
DESCRIPTION
    koovox stop automatic speech recognition
*/
static void KoovoxStopAutoSpeechRecognition(void)
{
	APP_CORE_DEBUG(("KoovoxStopAutoSpeechRecognition\n"));

	SetAsrStatus(ASR_IDLE);

	if(isOnlineState())
	{
		/* send ASR stop event to mobile */
		SendNotifyToDevice(OID_BUTTON_M, RELEASE, 0, 1);

		/* 超时检测 */
		MessageCancelAll(&theSink.task, EventKoovoxTimeout);
		MessageSendLater(&theSink.task, EventKoovoxTimeout, 0, D_SEC(30));
	}
}

/*************************************************************************
NAME
    KoovoxPlayPauseMusic
    
DESCRIPTION
    play or pause the music 
*/
static void KoovoxPlayPauseMusic(void)
{
	APP_CORE_DEBUG(("KoovoxPlayPauseMusic\n"));

	/* play or pause music */
	MessageSend(&(theSink.task), EventUsrAvrcpPlayPause, 0);
}

/*************************************************************************
NAME
    KoovoxPlayNext
    
DESCRIPTION
    play the next song  
*/
static void KoovoxPlayNext(void)
{
	APP_CORE_DEBUG(("KoovoxPlayNext\n"));

	/* play next song */
	MessageSend(&(theSink.task), EventUsrAvrcpSkipForward, 0);

}

/*************************************************************************
NAME
    KoovoxPlayPrevious
    
DESCRIPTION
    play the previous song  
*/
static void KoovoxPlayPrevious(void)
{
	APP_CORE_DEBUG(("KoovoxPlayPrevious\n"));

	/* play next song */
	MessageSend(&(theSink.task), EventUsrAvrcpSkipBackward, 0);
}



/*************************************************************************
NAME
    KoovoxSwitchToCallMode
    
DESCRIPTION
    switch to call mode 
*/
void KoovoxSwitchToCallMode(void)
{
	uint8 curr_mode = BUSINESS_MODE;
	
	APP_CORE_DEBUG(("KoovoxSwitchToCallMode\n"));

	/* store current mode */
	curr_mode = GetCurrMode();
	StorePreviousMode(curr_mode);

	switch(curr_mode)
	{
		case SPORT_MODE:
			if(!isOnlineState())
            {
				/* disable the heart rate value prompt */
				SetPromptStatus(PROMPT_DISABLE);
			}
			break;

		default:
			break;
	}

	/* switch to call mode */
	SetCurrMode(CALL_MODE);
	
}

/*************************************************************************
NAME
    KoovoxAnswerCall
    
DESCRIPTION
    answer the incoming call
*/
static void KoovoxAnswerCall(void)
{
	APP_CORE_DEBUG(("KoovoxAnswerCall\n"));

	/* send EventUsrAnswer event to sink task */
	MessageSend(&(theSink.task), EventUsrAnswer, 0);
}

/*************************************************************************
NAME
    KoovoxRejectCall
    
DESCRIPTION
    reject the incoming call
*/
static void KoovoxRejectCall(void)
{
	APP_CORE_DEBUG(("KoovoxRejectCall\n"));

	/* send EventUsrReject event to sink task */
	MessageSend(&(theSink.task), EventUsrReject, 0);
}


/*************************************************************************
NAME
	KoovoxCancelCall
	
DESCRIPTION
	cancel the outgoing call
*/
static void KoovoxCancelCall(void)
{
	APP_CORE_DEBUG(("KoovoxCancelCall\n"));

	/* send EventUsrReject event to sink task */
	MessageSend(&(theSink.task), EventUsrCancelEnd, 0);
}

/*************************************************************************
NAME
	KoovoxHangOffCall
	
DESCRIPTION
	hang off the active call
*/
static void KoovoxHangOffCall(void)
{
	APP_CORE_DEBUG(("KoovoxHangOffCall\n"));

	/* send EventUsrReject event to sink task */
	MessageSend(&(theSink.task), EventUsrCancelEnd, 0);
}

/*************************************************************************
NAME
	KoovoxRecordCall
	
DESCRIPTION
	record the active call
*/
static void KoovoxRecordCall(void)
{
	
	APP_CORE_DEBUG(("KoovoxRecordCall\n"));

	if(isOnlineState())
	{
		uint8 value = 0;
		if(isRecordBusy())
		{
			SetRecordStatus(RECORD_IDLE);

			value = OFF;
			/* disconnected record stream */
			SendNotifyToDevice(OID_RECODE, LONG_CLICK, &value, 1);
			AudioPromptPlayEvent(EventKoovoxPromptRecordStop);
		}
		else
		{
			SetRecordStatus(RECORD_BUSY);

			value = ON;
			/* send a notify to mobile */
			SendNotifyToDevice(OID_RECODE, LONG_CLICK, &value, 1);
			AudioPromptPlayEvent(EventKoovoxPromptRecordStart);
		}
	}
	else
	{
		/* audio prompt is offline state */
		AudioPromptPlayEvent(EventKoovoxPromptAppDisconnected);
	}

}

/*************************************************************************
NAME
	KoovoxPresetCall
	
DESCRIPTION
	preset the incoming call
*/
static void KoovoxPresetCall(void)
{
	APP_CORE_DEBUG(("KoovoxPresetCall\n"));

	if((!isPresentBusy())&&(isPresentEnable()))
	{
		SetPresentStatus(PRESET_BUSY);

		MessageCancelAll(&theSink.task, EventKoovoxButtonCDoubleClick);
		
		/* send EventUsrAnswer event to sink task */
		MessageSend(&(theSink.task), EventUsrAnswer, 0);
	}	
}

/*************************************************************************
NAME
	KoovoxPromptPrecence
	
DESCRIPTION
	prepare for the precence call
*/
static void KoovoxPromptPrecence(void)
{

	APP_CORE_DEBUG(("KoovoxPromptPrecence\n"));

	/* disconnect the connect bitween sco sink with sco port */
	/*StreamDisconnect( StreamSourceFromSink(theSink.routed_audio), StreamKalimbaSink(1));*/

	if(isVoiceMessageEnable())
	{
		uint8 value = ON;
		/* start record */
		SendNotifyToDevice(OID_VOICE_MSG, DOUBLE_CLICK, &value, 1);
	
		/* 语言提示主叫留言 */
		KoovoxAudioPresencePlayEvent(GetPresentScenceEvent());
		KoovoxAudioPresencePlayEvent(EventKoovoxPromptVoiceMeassage);
	}
	else
	{
		/* 播放预置音 */
		KoovoxAudioPresencePlayEvent(GetPresentScenceEvent());
		KoovoxAudioPresencePlayEvent(EventKoovoxPromptCallLater);
	}
	
}

/*************************************************************************
NAME
	KoovoxRecoverPreviousMode
	
DESCRIPTION
	leave call mode recover to previous mode
*/
static void KoovoxRecoverPreviousMode(void)
{
	APP_CORE_DEBUG(("KoovoxRecoverPreviousMode\n"));

	/* recove to previous mode */
	RecoverPreviousMode();
}


/*************************************************************************
NAME
    KoovoxPeriodPromptHeartRate
    
DESCRIPTION
    period audio prompt heart rate value to user
*/
static void KoovoxPeriodPromptHeartRate(void)
{

	if(isPromptEnable())
	{
		APP_CORE_DEBUG(("KoovoxPeriodPromptHeartRate:value=%d\n", GetHeartRateValue()));
		
		/* interval prompt heart rate to user */
		MessageSendLater(&(koovox.task), EventKoovoxPeriodPromptHeartRate, 0, D_SEC(AUDIO_PROMPT_TIME));

		if(koovox.heartRateValue)
			KoovoPromptPlayHeartRate(koovox.heartRateValue);
	}
}

/*************************************************************************
NAME
    KoovoxStopHeartRateMeasure
    
DESCRIPTION
    stop sport mode ( disable heart rate sensor )
*/
void KoovoxStopHeartRateMeasure(void)
{
	APP_CORE_DEBUG(("KoovoxStopHeartRateMeasure\n")) ;

	/* disable heart rate sampling */
	SetSampleStatus(SAMPLE_DISABLE);

	/* reset the heart rate value */
	UpdateHeartRateValue(0);
#if 0

	{
		TaskData * task    = NULL;
		task = (TaskData *) &koovox_sport_common_plugin;
		AudioSportModeStopPrompt(task);
	}
	
	/* disable the heart rate sensor */
	HeartRateSensorDisable();
#else
	KoovoxFillAndSendUartPacket(STOP, OBJ_HEART_RATE, 0, 0);

#endif

}

/*************************************************************************
NAME
    KoovoxStartHeartRateMeasure
    
DESCRIPTION
    enable heart rate sensor and start sampling
*/
void KoovoxStartHeartRateMeasure(void)
{

	APP_CORE_DEBUG(("KoovoxStartHeartRateMeasure\n"));

	/* enable heart rate sample  */
	SetSampleStatus(HB_ENABLE);

#if 1

	KoovoxFillAndSendUartPacket(START, OBJ_HEART_RATE, 0, 0);
#else
	/* enable heart rate sensor */
	HeartRateSensorEnable();

	{
		TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
		uint8 sensor_sample = HB_SENSOR ;
		
		AudioSportModePlayPrompt( task, sensor_sample, 0, &theSink.task);
	}
#endif
		
}


/*************************************************************************
NAME
    KoovoxStopSportMode
    
DESCRIPTION
    stop sport mode 
*/
void KoovoxStopSportMode(void)
{
	APP_CORE_DEBUG(("KoovoxStopSportMode\n")) ;

	/* disable heart rate sampling */
	SetSampleStatus(SAMPLE_DISABLE);

#if 0

	/* disable herat rate value prompt */
	SetPromptStatus(PROMPT_DISABLE);

	/* reset the heart rate value */
	UpdateHeartRateValue(0);

	{
		TaskData * task    = NULL;
		task = (TaskData *) &koovox_sport_common_plugin;
		AudioSportModeStopPrompt(task);
	}
	
	/* disable the heart rate sensor */
	HeartRateSensorDisable();

	ClearStepValue();
	ClearTimeValue();
#else
	/*KoovoxFillAndSendUartPacket(STOP, OBJ_HEAD_ACTION, 0, 0);*/
#endif
}

/*************************************************************************
NAME
    KoovoxStartSportMode
    
DESCRIPTION
    start sport mode
*/
void KoovoxStartSportMode(void)
{

	APP_CORE_DEBUG(("KoovoxStartSportMode\n"));

	/* enable heart rate sample  */
	SetSampleStatus(SPORT_ENABLE);
#if 0

	/* enable heart rate sensor */
	HeartRateSensorEnable();

#if 0
	/* load DSP file or not ? */
	if((DSP_RUNNING != GetCurrentDspStatus())&&(!GetHealthEnable()))
	{
		/* load dsp file */
		loadHeartRateDspFile();
	}	

	/* send heartrate enable cmd to dsp */
	KalimbaSendMessage(HB_SAMPLE_ENABLE_CMD_TO_DSP, 0, 0, 0, 0);

	KalimbaSendMessage(ACC_SAMPLE_ENABLE_CMD_TO_DSP, 0, 0, 0, 0);
	
	KalimbaSendMessage(STEP_VALUE_ENABLE_CMD_TO_DSP, 0, 0, 0, 0);
#else
	{
		TaskData * task    = (TaskData *) &koovox_sport_common_plugin;
		uint8 sensor_sample = HB_SENSOR + ACC_SENSOR;
		uint16 sensor_value = STEP_VALUE;
		if(!isOnlineState())
			sensor_value += TIME_VALUE;
		
		AudioSportModePlayPrompt( task, sensor_sample, sensor_value, &theSink.task);
	}
#endif

	if(!isOnlineState())
	{
		/* enable herat rate value prompt */
		SetPromptStatus(PROMPT_ENABLE);

		/* send message to start period audio prompt heart rate value to user */
		MessageSend(&koovox.task, EventKoovoxPeriodPromptHeartRate, 0);
	}

#else
	/*KoovoxFillAndSendUartPacket(START, OBJ_HEAD_ACTION, 0, 0);*/
#endif	

}



/*************************************************************************
NAME
    KoovoxEmergencyCall
    
DESCRIPTION
    start a  emergency call 
*/
static void KoovoxEmergencyCall(void)
{
	APP_CORE_DEBUG(("KoovoxEmergencyCall\n"));

	if(deviceConnected <= stateManagerGetState())
	{
		/* call the user set emergency number */
		MessageSend(&(theSink.task), EventUsrDialStoredNumber, 0);
	}
	else
	{
		/* 手机未连接， 无法使用此功能 */
		AudioPromptPlayEvent(EventKoovoxPromptPhoneDisconnected);
	}

}


/*************************************************************************
NAME
    KoovoxEventEngine
    
DESCRIPTION
    match the event and the actionHandler then execute the actionHandler
*/
static void KoovoxEventEngine(uint16 event, const ModeFSMActionHandle_t* pfunc, uint16 len)
{
	uint8 i = 0;
	
	APP_CORE_DEBUG(("KoovoxEventEngine:len=%d\n", len));

	for(i = 0; i < len; i++)
	{
		if((event == pfunc[i].event) &&
			((NULL == pfunc[i].isValid )||(*(pfunc[i].isValid))())) 
		{
			/* executing the actionHandler */
			if (pfunc[i].actionHandler)
			{
				APP_CORE_DEBUG(("KoovoxEventEngine:actionHandler=%d\n", i));
				(*(pfunc[i].actionHandler))();			
				break;
			}
		}
	}
}

/*************************************************************************
NAME
    KoovoxEventHandler
    
DESCRIPTION
    handle the koovox events
*/
void KoovoxEventHandler(uint16 event)
{
	ModeFSM mode = BUSINESS_MODE;

	/* get the koovox work mode */
	mode = GetCurrMode();

	APP_CORE_DEBUG(("KoovoxEventHandler:event=%x mode= %d\n", event, mode));
	switch(mode)
	{
		case BUSINESS_MODE:
			KoovoxEventEngine(event, businessMode, sizeof(businessMode) / sizeof(ModeFSMActionHandle_t));
			break;
			
		case MUSIC_MODE:
			KoovoxEventEngine(event, musicMode, sizeof(musicMode) / sizeof(ModeFSMActionHandle_t));
			break;
			
		case SPORT_MODE:
			KoovoxEventEngine(event, sportMode, sizeof(sportMode) / sizeof(ModeFSMActionHandle_t));
			break;
			
		case CALL_MODE:
			KoovoxEventEngine(event, callMode, sizeof(callMode) / sizeof(ModeFSMActionHandle_t));
			break;
			
		case ASR_MODE:
			KoovoxEventEngine(event, asrMode, sizeof(asrMode) / sizeof(ModeFSMActionHandle_t));
			break;
			
		default:
			break;
	}
	
}


/*************************************************************************
NAME    
    KoovoxRcvShortKalimbaMsg
    
DESCRIPTION
    receive short message from the kalimba

RETURNS
    void
*/
void KoovoxRcvShortKalimbaMsg(Message msg)
{
	const DSP_REGISTER_T* dr = (const DSP_REGISTER_T*)msg;

	APP_CORE_DEBUG(("KoovoxRcvShortKalimbaMsg:%x\n", dr->id));

	switch(dr->id)
	{
	case HEARTRATE_VALUE_FROM_DSP:
	{
		/* update the heart rate value */
		UpdateHeartRateValue(dr->a & 0xff);
		if(isOnlineState())
		{
			/* send the heart rate value to mobile */
			SendNotifyToDevice(OID_HB , ON, &koovox.heartRateValue, 1);
		}
	}
	break;
	
	case HB_I2C_TEST_RESULT_FROM_DSP:
	{
		uint16 value = 0;
		ConfigRetrieve(CONFIG_HB_I2C_TEST_RESULT,&value, 1);
		DEBUG(("hb_i2c result:%d pskey_value:%d\n", dr->a, value));
		if(value != dr->a)
			ConfigStore(CONFIG_HB_I2C_TEST_RESULT, (const void*)(&(dr->a)), 1);
	}
	break;
	
	case ACC_I2C_TEST_RESULT_FROM_DSP:
	{
		uint16 value = 0;
		ConfigRetrieve(CONFIG_ACC_I2C_TEST_RESULT,&value, 1);
		DEBUG(("acc_i2c result:%d pskey_value:%d\n", dr->a, value));
		if(value != dr->a)
			ConfigStore(CONFIG_ACC_I2C_TEST_RESULT, (const void*)(&(dr->a)), 1);
	}
	break;
	
	case DRIVER_PROMPT_EVENT_FROM_DSP:
	{
		DEBUG(("++++DRIVER_PROMPT_EVENT_FROM_DSP\n"));
		AudioPromptPlayEvent(EventKoovoxPromptSafeDriverAdvice);
	}
	break;
		
	case NECK_PROMPT_EVENT_FROM_DSP:
	{
		DEBUG(("++++NECK_PROMPT_EVENT_FROM_DSP\n"));
		AudioPromptPlayEvent(EventKoovoxPromptNectProtectAdvice);
	}
	break;

	case SEAT_PROMPT_EVENT_FROM_DSP:
	{
		DEBUG(("++++SEAT_PROMPT_EVENT_FROM_DSP\n"));
		AudioPromptPlayEvent(EventKoovoxPromptNectProtectAdvice);
	}
	break;
		
	case SPORT_DATA_FROM_DSP:
	{
		APP_CORE_DEBUG(("hb:%d, step:%d ,time:%d\n", dr->a, dr->b, dr->c));
		UpdateHeartRateValue(dr->a & 0xff);
		UpdateStepValue(dr->b & 0xff);
		UpdateTimeValue(dr->c & 0xff);
		if(isOnlineState())
		{
			uint8 sport_data[5] = {0};
			uint8 hb_value = GetHeartRateValue();
			uint32 step = GetStepValue();
			sport_data[0] = hb_value;
			sport_data[1] = (step >> 24) & 0xff;
			sport_data[2] = (step >> 16) & 0xff;
			sport_data[3] = (step >> 8) & 0xff;
			sport_data[4] = step  & 0xff;

			/* send the heart rate value to mobile */
			SendNotifyToDevice(OID_SPORT_VALUE, 0, sport_data, sizeof(sport_data));
		}
	}
	break;

	case NOD_ACTION_EVENT_FROM_DSP:
	{
		/* 接听电话 */
		MessageSend(&(theSink.task), EventUsrAnswer, 0);		
	}
	break;

	case SHAKE_ACTION_EVENT_FROM_DSP:
	{
		if(isActiveState())
		{
			/* 挂断电话 */
			MessageSend(&(theSink.task), EventUsrCancelEnd, 0);
		}
		else if(isIncomingState())
		{
			/* 拒接电话 */
			MessageSend(&(theSink.task), EventUsrReject, 0);
		}
	}
	break;
		
	default:
		break;
	}

}


/*************************************************************************
NAME    
    KoovoxRcvLongKalimbaMsg
    
DESCRIPTION
    receive long message from the kalimba

RETURNS
*/
void KoovoxRcvLongKalimbaMsg(Message message)
{
	const dsp_long_msg* msg = (const dsp_long_msg*)message;
	APP_CORE_DEBUG(("KoovoxRcvLongKalimbaMsg:id->%x size=%d\n",msg->id, msg->size_value));

	switch(msg->id)
	{
	
	default:
#ifdef DEBUG_PRINT_ENABLED
		{
			uint8 i = 0;
			for(; i<msg->size_value; i++)
				DEBUG(("%d ", msg->value[i]));
			DEBUG(("\n"));
		}
#endif

#ifdef ENABLE_LOG
		{
			char* log_msg = (char*)mallocPanic((msg->size_value+1)*LOG_WIDE);
			uint8 i = 0;
			for(; i<msg->size_value; i++)
				sprintf(log_msg + LOG_WIDE*i,"%5d ", msg->value[i]);
			sprintf(log_msg + LOG_WIDE*i, "\n");
			APP_CORE_LOG((log_msg));
			freePanic(log_msg);
			log_msg = NULL;
		}
#endif
		
		break;
	}

}

/****************************************************************************
NAME 
    KoovoPromptPlayNumber
DESCRIPTION
    Play a uint32 using the Audio prompt plugin
RETURNS    
*/
void KoovoPromptPlayNumber(uint32 number)
{
    char num_string[3];
	uint8_t len = 0;
	uint16 event = 0;
	uint32 ten, hundrend, temp;
	
    sprintf(num_string, "%ld",number);
	len = strlen(num_string);

	switch(len)
	{
	case 1:
		event = EventSysToneDigit0 + number;
		AudioPromptPlayEvent(event);
	break;

	case 2:
		ten = number / 10;
		temp = number % 10;
		if(ten > 1)
		{
			event = EventSysToneDigit0 + ten;
			AudioPromptPlayEvent(event);
		}
		AudioPromptPlayEvent(EventKoovoxPromptTen);
		if(temp)
		{
			event = EventSysToneDigit0 + temp;
			AudioPromptPlayEvent(event);
		}		
	break;

	case 3:
		hundrend = number / 100;
		temp = number % 100;
		if(hundrend > 1)
		{
			event = EventSysToneDigit0 + hundrend;
			AudioPromptPlayEvent(event);
		}
		AudioPromptPlayEvent(EventKoovoxPromptHundrend);
		if(temp)
		{
			ten = temp / 10;
			temp = temp % 10;
			if(ten > 1)
			{
				event = EventSysToneDigit0 + ten;
				AudioPromptPlayEvent(event);
			}
			AudioPromptPlayEvent(EventKoovoxPromptTen);
			if(temp)
			{
				event = EventSysToneDigit0 + temp;
				AudioPromptPlayEvent(event);
			}		
		}	
	break;

	default:
		break;
	}
	
}


/****************************************************************************
NAME 
    KoovoPromptPlayHeartRate
DESCRIPTION
    audio prompt heart rate 
RETURNS    
*/
void KoovoPromptPlayHeartRate(uint32 value)
{
	APP_CORE_DEBUG(("KoovoPromptPlayHeartRate\n")) ;

	AudioPromptPlayEvent(EventKoovoxPromptHeartRateValue);
	KoovoPromptPlayNumber(value);
}


/****************************************************************************
NAME 
    KoovoxPromptPlayBattery
DESCRIPTION
    audio prompt battery
RETURNS    
*/
void KoovoxPromptPlayBattery(uint32 value)
{
	uint32 hour, min;
	APP_CORE_DEBUG(("KoovoxPromptPlayBattery:%d\n", (uint16)value)) ;

	if(value > 100)
		return;

	if(value)
	{
		AudioPromptPlayEvent(EventKoovoxPromptEnduranceTime);
		
		hour = value / 10;
		min = (value % 10)*6;
		if(hour)
		{
			KoovoPromptPlayNumber(hour);
			AudioPromptPlayEvent(EventKoovoxPromptHour);
		}
		
		if(min)
		{
			KoovoPromptPlayNumber(min);
			AudioPromptPlayEvent(EventKoovoxPromptMinute);
		}
	}
}

/****************************************************************************
NAME 
    KoovoxCurrentModePrompt
DESCRIPTION
    audio prompt current mode
RETURNS    
*/
void KoovoxCurrentModePrompt(uint8 state)
{
	APP_CORE_DEBUG(("KoovoxCurrentModePrompt\n")) ;

	switch(state)
	{
	case BUSINESS_MODE:
		AudioPromptPlayEvent(EventKoovoxPromptBusinessMode);
		break;
	case MUSIC_MODE:
		AudioPromptPlayEvent(EventKoovoxPromptMusicMode);
		break;
	case SPORT_MODE:
		AudioPromptPlayEvent(EventKoovoxPromptSportMode);
		break;
	default:
		break;
	}

}


/****************************************************************************
NAME 
    KoovoxSetModePrompt
DESCRIPTION
    set koovox mode and audio prompt mode
RETURNS    
*/
void KoovoxSetModePrompt(uint8 state)
{
	APP_CORE_DEBUG(("KoovoxSetModePrompt\n")) ;

	/* switch mode */
	SetCurrMode(state);

	/* audio prompt set mode */
	KoovoxCurrentModePrompt(state);

}



/****************************************************************************
NAME
    KoovoxAudioPresencePlayEvent
    
DESCRIPTION
    play presece audio to peer listenning
*/
bool KoovoxAudioPresencePlayEvent (uint16 event)
{  
    uint16 lEventIndex = event;
	uint16 state_mask  = 1 << stateManagerGetState();
    audio_prompts_config_type* ptr  = NULL;
	TaskData * task    = NULL;
    bool event_match   = FALSE;

	APP_CORE_DEBUG(("KoovoxAudioPresencePlayEvent\n")) ;
    
    if(theSink.conf4)
    {
        ptr = theSink.conf4->audioPromptEvents;
    }
    else
    {
        /* no config */
        return FALSE;
    }
     
    task = (TaskData *) &csr_voice_presences_plugin;

	/* While we have a valid Audio Prompt event */
	while(ptr->prompt_id != AUDIO_PROMPT_NOT_DEFINED)
	{           
	    /* Play Audio Prompt if the event matches and we're not in a blocked state or in streaming A2DP state */
	    if((ptr->event == lEventIndex) && 
	       ((ptr->state_mask & state_mask) && 
	       (!(ptr->sco_block && theSink.routed_audio)||(state_mask & (1<<deviceA2DPStreaming)))))
	    {
	        APP_CORE_DEBUG(("AP: EvPl[%x][%x][%x][%x][%x]\n", event, lEventIndex, ptr->event, ptr->prompt_id,  ptr->cancel_queue_play_immediate )) ;
	        event_match = TRUE;

		   /* turn on audio amp */
		   PioSetPio ( theSink.conf1->PIOIO.pio_outputs.DeviceAudioActivePIO , pio_drive, TRUE) ;

		   /* start check to turn amp off again if required */ 
		   MessageSendLater(&theSink.task , EventSysCheckAudioAmpDrive, 0, 1000);	 
		   	   
		   AudioPlayAudioPrompt(task, (uint16) ptr->prompt_id, theSink.audio_prompt_language, TRUE,
						theSink.codec_task, TonesGetToneVolume(FALSE), 
						theSink.conf2->audio_routing_data.PluginFeatures, ptr->cancel_queue_play_immediate, &koovox.task);

           
           /* If the event is a power off, then there is no need to connect an audio plugin again after the prompt */
           if((GetAudioPlugin()!= task)&&(event == EventUsrPowerOff))
           {
               MessageCancelAll(GetAudioPlugin(),AUDIO_PLUGIN_CONNECT_MSG);
           }                   
	    }
	    ptr++;
	}
    return event_match;
} 

/****************************************************************************
NAME
    KoovoxAutoLeaveToMusicMode
    
DESCRIPTION
    koovox auto switch to music mode 
*/
void KoovoxAutoSwitchToMusicMode(void)
{

	APP_CORE_DEBUG(("KoovoxAutoSwitchToMusicMode\n")) ;

	/* auto switch to music mode */
	if(BUSINESS_MODE == GetCurrMode())
	{
		DEBUG(("++ switch mode ++\n"));
		StorePreviousMode(BUSINESS_MODE);
		SetCurrMode(MUSIC_MODE);
	}
	else if(MUSIC_MODE == GetCurrMode())
	{
		DEBUG(("++ do not switch mode ++\n"));
		StorePreviousMode(MUSIC_MODE);
	}

}

/****************************************************************************
NAME
    KoovoxAutoLeaveToMusicMode
    
DESCRIPTION
    koovox auto leave to music mode 
*/
void KoovoxAutoLeaveToMusicMode(void)
{

	APP_CORE_DEBUG(("KoovoxAutoLeaveToMusicMode\n")) ;
	
	if((MUSIC_MODE == GetCurrMode()) && isAutoMusic())
	{
		DEBUG(("+++recover previous mode+++\n"));
		RecoverPreviousMode();
	}
	
}

/****************************************************************************
NAME
    KoovoxPowerOnPrompt
    
DESCRIPTION
    
*/
void KoovoxPowerOnPrompt(void)
{
	APP_CORE_DEBUG(("KoovoxPowerOnPrompt\n")) ;

	AudioPromptPlayEvent(EventKoovoxPromptWelcome);
}


/****************************************************************************
NAME
    KoovoxHandleEventCvcKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventCvcKalimbaLoaded(void)
{
	APP_CORE_DEBUG(("KoovoxHandleEventCvcKalimbaLoaded\n")) ;

	if(isPresentBusy())
	{
		/* disconnect the connect bitween sco source with sco port */
		StreamDisconnect( StreamSourceFromSink(theSink.routed_audio), StreamKalimbaSink(1));
	}
	else
	{
		/* send message to dsp to enable the micphone */
		KalimbaSendMessage(MUTE_MICROPHONE_CMD_TO_DSP, 0, 0, 0, 0);
	}

	if((isGattConnected())&&(isAsrBusy()))
	{
		/* 播放提示音 */
		TonesPlayEvent(EventKoovoxPromptAsrBegin);
	}
}


/****************************************************************************
NAME
    KoovoxHandleEventCodecKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventCodecKalimbaLoaded(void)
{
	KoovoxRestartDsp();

	{
	uint16 hb_value = GetHeartRateValue();
	/* send message to dsp to recover the heartrate */
	KalimbaSendMessage(RECOVER_HB_VALUE_CMD_TO_DSP, hb_value, 0, 0, 0);
	}
}

/****************************************************************************
NAME
    KoovoxHandleEventSportKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventSportKalimbaLoaded(void)
{
	uint16 hb_value = GetHeartRateValue();
	/* send message to dsp to recover the heartrate */
	KalimbaSendMessage(RECOVER_HB_VALUE_CMD_TO_DSP, hb_value, 0, 0, 0);
}

/****************************************************************************
NAME
    KoovoxAudioPromptTime
    
DESCRIPTION
    
*/
void KoovoxAudioPromptTime(uint8 hour, uint8 min)
{
	APP_CORE_DEBUG(("KoovoxAudioPromptTime\n")) ;

	/* 播报系统时间 */
	if(hour)
	{
		KoovoPromptPlayNumber(hour);
		AudioPromptPlayEvent(EventKoovoxPromptHour);
	}

	if(min)
	{
		KoovoPromptPlayNumber(min);
		AudioPromptPlayEvent(EventKoovoxPromptMinute);
	}
		
	/* 播报续航时间 */
	/* get the battery value */
	UpdateBatteryValue(get_battery_level_as_percentage());
	KoovoxPromptPlayBattery(koovox.batteryValue);
}

/****************************************************************************
NAME
    KoovoxAudioPromptSportData
    
DESCRIPTION
    
*/
void KoovoxAudioPromptSportData(uint16 mileage, uint8 hour, uint8 min)
{
	APP_CORE_DEBUG(("KoovoxAudioPromptTime\n")) ;

	/* 播报运动时间 */
	if(hour)
	{
		KoovoPromptPlayNumber(hour);
		AudioPromptPlayEvent(EventKoovoxPromptHour);
	}

	if(min)
	{
		KoovoPromptPlayNumber(min);
		AudioPromptPlayEvent(EventKoovoxPromptMinute);
	}
	
	/* 播报运动里程 */
	if(mileage)
	{
		KoovoPromptPlayNumber(mileage);
	}
	
	/* 播报当前心率 */
	

	/* 播报运动步数 */
	
}


/****************************************************************************
NAME	
	HeartRateSensorEnable  
    
RETURNS
	void
*/
void HeartRateSensorEnable(void)
{
	PioCommonSetPin(HEART_RATE_SENSOR_PIO, pio_drive , FALSE);
}

/****************************************************************************
NAME	
	HeartRateSensorDisable  
    
RETURNS
	void
*/
void HeartRateSensorDisable(void)
{
	PioCommonSetPin(HEART_RATE_SENSOR_PIO, pio_drive , TRUE);
}



/*******************************************************************************
FUNCTION
    get_battery_level_as_percentage
    
DESCRIPTION
    Helper function to read the battery level and retun as a value 0-100
*/
uint8 get_battery_level_as_percentage(void)
{
#ifdef ENABLE_BATTERY_OPERATION
    
    /* Get current battery level as voltage */
    voltage_reading reading;
    PowerBatteryGetVoltage(&reading);
    
    /* calculate %battery level using: (currentV - minV)/(maxV - minV)*100 */
    if (theSink.rundata->battery_limits.max_battery_v > theSink.rundata->battery_limits.min_battery_v)
    {
        if (reading.voltage < theSink.rundata->battery_limits.min_battery_v)
        {
            return 0;
        }
        else if (reading.voltage > theSink.rundata->battery_limits.max_battery_v)
        {
            return 100;
        }
        else
        {
            return (uint8)(((uint32)(reading.voltage - theSink.rundata->battery_limits.min_battery_v)  * (uint32)100) / (uint32)(theSink.rundata->battery_limits.max_battery_v - theSink.rundata->battery_limits.min_battery_v));
        }
    }
    else
    {
        /* Invalid battery config */
        return 0;
    }
#else
    /* Battery operation is disabled */
    return 0;
#endif
}


#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    
#endif /* ENABLE_KOOVOX */
