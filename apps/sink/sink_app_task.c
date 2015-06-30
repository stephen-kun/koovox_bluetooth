/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2014
Part of ADK 3.5

FILE NAME
    sink_app_task.c

*/

#ifdef ENABLE_KOOVOX

#include <stdio.h>
#include <spps.h>
#include <source.h>
#include <kalimba.h>
#include <pio.h>
#include <pio_common.h>
#include <panic.h>
#include <vm.h>
	
#include "sink_app_task.h"
#include "sink_app_message.h"
#include "sink_app_core.h"
#include "sink_audio_prompts.h"
#include "sink_ble_advertising.h"
#include "sink_koovox_uart.h"






/* Single instance of phone app task */
gKoovoxTaskData koovox;

/*************************************************************************
NAME
    SetResponseStatus
    
DESCRIPTION
    set the response status 
*/
void SetResponseStatus(uint8 status)
{
	koovox.rspStatus = status;
}


/*************************************************************************
NAME
    isResponseTimeout
    
DESCRIPTION
    chech the response timeout or not 
*/
bool isResponseTimeout(void)
{
	if(koovox.rspStatus == RSP_TIMEOUT)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*************************************************************************
NAME
    isAutoMusic
    
DESCRIPTION
    chech the music is auto play or not
*/
bool isAutoMusic(void)
{
	if(koovox.musicStatus == MUSIC_AUTO)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*************************************************************************
NAME
    SetMusicStatus
    
DESCRIPTION
    set music status auto or manual
*/
void SetMusicStatus(uint8 status)
{
	koovox.musicStatus = status;
}

/*************************************************************************
NAME
    isOnlineState
    
DESCRIPTION
    chech the app connected 
*/
bool isOnlineState(void)
{
	if((DISCONNECTED != koovox.connectStatus)&&(isConnectedState()))
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isSppConnected
    
DESCRIPTION
    chech the app connected through spp profile
*/
bool isSppConnected(void)
{
	if(SPP_CONNECTED == koovox.connectStatus)
		return TRUE;
	else
		return FALSE;		
}

/*************************************************************************
NAME
    isGattConnected
    
DESCRIPTION
    chech the app connected through gatt profile
*/
bool isGattConnected(void)
{
	if(GATT_CONNECTED == koovox.connectStatus)
		return TRUE;
	else
		return FALSE;		
}


/*************************************************************************
NAME
    SetAsrStatus
    
DESCRIPTION
    set the ASR status 
*/
void SetAsrStatus(uint8 status)
{
	APP_TASK_DEBUG(("SetAsrStatus\n"));
	koovox.asrStatus = status;
}

/*************************************************************************
NAME
    SetAsrStatus
    
DESCRIPTION
    set the ASR status 
*/
bool isAsrBusy(void)
{
	if(ASR_BUSY == koovox.asrStatus)
		return TRUE;
	else
		return FALSE;
}

/*************************************************************************
NAME
    SetCurrMode
    
DESCRIPTION
    set the current mode 
*/
void SetCurrMode(ModeFSM state)
{
	APP_TASK_DEBUG(("SetCurrMode:current mode %d\n", state));
	koovox.currMode = state;
}

/*************************************************************************
NAME
    GetCurrMode
    
DESCRIPTION
    get the current mode 
*/
ModeFSM GetCurrMode(void)
{
	return koovox.currMode ;
}

/*************************************************************************
NAME
    SetSampleStatus
    
DESCRIPTION
    set the heart rate sample  
*/
void SetSampleStatus(uint8 status)
{
	koovox.sampleStatus = status;
}

/*************************************************************************
NAME
    GetSampleStatus
    
DESCRIPTION
    check the sample enable or disable 
*/
uint8 GetSampleStatus(void)
{
	return koovox.sampleStatus;
}


/*************************************************************************
NAME
    SetMuteStatus
    
DESCRIPTION
    set micphone mute status  
*/
void SetMuteStatus(uint8 status)
{
	koovox.muteStatus = status;
}


/*************************************************************************
NAME
    isMuteEnable
    
DESCRIPTION
    check the micphone mute enable or disable 
*/
bool isMuteEnable(void)
{
	if(koovox.muteStatus == MUTE_ENABLE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*************************************************************************
NAME
    StorePreviousMode
    
DESCRIPTION
    store the koovox previous mode 
*/
void StorePreviousMode(uint8 mode)
{
	koovox.prevMode = mode;
}

/*************************************************************************
NAME
    GetPreviousMode
    
DESCRIPTION
    get previous mode 
*/
ModeFSM GetPreviousMode(void)
{
	return koovox.prevMode;
}


/*************************************************************************
NAME
    RecoverPreviousMode
    
DESCRIPTION
    recover the koovox previous mode 
*/
void RecoverPreviousMode(void)
{
	SetCurrMode(koovox.prevMode);
}

/*************************************************************************
NAME
    SetRecordStatus
    
DESCRIPTION
    set the koovox record status 
*/
void SetRecordStatus(uint8 status)
{
	koovox.recordStatus = status;
}

/*************************************************************************
NAME
    isRecordBusy
    
DESCRIPTION
    check the koovox record busy or not
*/
bool isRecordBusy(void)
{
	if(koovox.recordStatus == RECORD_BUSY)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/*************************************************************************
NAME
    SetPresentStatus
    
DESCRIPTION
    set the koovox presence status 
*/
void SetPresentStatus(uint8 status)
{
	koovox.presentStatus = status;
}


/*************************************************************************
NAME
    isPresentBusy
    
DESCRIPTION
    chech the device is presence state
*/
bool isPresentBusy(void)
{
	if(PRESET_BUSY == koovox.presentStatus)
		return TRUE;
	else
		return FALSE;
}

/*************************************************************************
NAME
    SetDriverEnable
    
DESCRIPTION
    enable or disable the koovox driver business
*/
void SetDriverEnable(uint8 status)
{
	koovox.driverEnable = status;
}


/*************************************************************************
NAME
    isDriverEnable
    
DESCRIPTION
    chech the driver business is enable
*/
bool isDriverEnable(void)
{
	if(DRIVER_ENABLE == koovox.driverEnable)
		return TRUE;
	else
		return FALSE;
}

/*************************************************************************
NAME
    SetActionCallEnable
    
DESCRIPTION
    enable or disable the koovox action call business
*/
void SetActionCallEnable(uint8 status)
{
	koovox.action_call = status;
}


/*************************************************************************
NAME
    isActionCallEnable
    
DESCRIPTION
    chech the action call business is enable
*/
bool isActionCallEnable(void)
{
	if(ACTION_CALL_ENABLE == koovox.action_call)
		return TRUE;
	else
		return FALSE;
}


/*************************************************************************
NAME
    SetNeckProtectEnable
    
DESCRIPTION
    enable or disable the koovox neck protect business
*/
void SetNeckProtectEnable(uint8 status)
{
	koovox.neckEnable = status;
}


/*************************************************************************
NAME
    isNeckProtectEnable
    
DESCRIPTION
    chech the neck protect business is enable
*/
bool isNeckProtectEnable(void)
{
	if(NECK_ENABLE == koovox.neckEnable)
		return TRUE;
	else
		return FALSE;
}


/*************************************************************************
NAME
    SetPresentEnable
    
DESCRIPTION
    enable or disable the koovox presence business
*/
void SetPresentEnable(uint8 status)
{
	koovox.vp_enable = status;
}


/*************************************************************************
NAME
    isPresentEnable
    
DESCRIPTION
    chech the presence business is enable
*/
bool isPresentEnable(void)
{
	if(VP_ENABLE == koovox.vp_enable)
		return TRUE;
	else
		return FALSE;
}

/*************************************************************************
NAME
    SetPresentAuto
    
DESCRIPTION
    enable or disable auto present 
*/
void SetPresentAuto(uint8 status)
{
	koovox.vp_auto = status;
}


/*************************************************************************
NAME
    isPresentAuto
    
DESCRIPTION
    chech the presence business is auto
*/
bool isPresentAuto(void)
{
	if(VP_AUTO_ENABLE == koovox.vp_auto)
		return TRUE;
	else
		return FALSE;
}

/*************************************************************************
NAME
    SetVoiceMessageEnable
    
DESCRIPTION
    enable or disable the koovox voice message business
*/
void SetVoiceMessageEnable(uint8 status)
{
	koovox.vm_enable = status;
}


/*************************************************************************
NAME
    isVoiceMessageEnable
    
DESCRIPTION
    chech the voice message business is enable
*/
bool isVoiceMessageEnable(void)
{
	if(VM_ENABLE == koovox.vm_enable)
		return TRUE;
	else
		return FALSE;
}


/*************************************************************************
NAME
    UpdateBatteryValue
    
DESCRIPTION
   update the koovox battery value
*/
void UpdateBatteryValue(uint8 value)
{
	koovox.batteryValue = value;
}

/*************************************************************************
NAME
    GetBatteryValue
    
DESCRIPTION
   get the koovox battery value
*/
uint8 GetBatteryValue(void)
{
	return koovox.batteryValue;
}

/*************************************************************************
NAME
    UpdateStepValue
    
DESCRIPTION
   update the koovox step value
*/
void UpdateStepValue(uint16 value)
{
	koovox.step_value += (uint32)value;
}

/*************************************************************************
NAME
    ClearStepValue
    
DESCRIPTION
   clear the koovox step value
*/
void ClearStepValue(void)
{
	koovox.step_value = 0;
}

/*************************************************************************
NAME
    GetStepValue
    
DESCRIPTION
   get the koovox step value
*/
uint32 GetStepValue(void)
{
	return koovox.step_value;
}

/*************************************************************************
NAME
    UpdateTimeValue
    
DESCRIPTION
   update the koovox sport time value
*/
void UpdateTimeValue(uint16 value)
{
	koovox.time_value += value;
}

/*************************************************************************
NAME
    ClearTimeValue
    
DESCRIPTION
   clear the koovox sport time value
*/
void ClearTimeValue(void)
{
	koovox.time_value = 0;
}

/*************************************************************************
NAME
    GetTimeValue
    
DESCRIPTION
   get the koovox sport time value
*/
uint16 GetTimeValue(void)
{
	return koovox.time_value;
}


/*************************************************************************
NAME
    UpdateHeartRateValue
    
DESCRIPTION
   update the koovox heart rate value
*/
void UpdateHeartRateValue(uint8 value)
{
	koovox.heartRateValue = value;
}

/*************************************************************************
NAME
    GetHeartRateValue
    
DESCRIPTION
   get the koovox heart rate value
*/
uint8 GetHeartRateValue(void)
{
	return koovox.heartRateValue ;
}

/*************************************************************************
NAME
    SetPromptStatus
    
DESCRIPTION
    set the heart rate prompt status 
*/
void SetPromptStatus(uint8 status)
{
	koovox.promptStatus = status;
}

/*************************************************************************
NAME
    isPromptEnable
    
DESCRIPTION
    check the heart rate value prompt enable or disable 
*/
bool isPromptEnable(void)
{
	if(koovox.promptStatus == PROMPT_ENABLE)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*************************************************************************
NAME
    SetPresentScence
    
DESCRIPTION
    set the present scence
*/
void SetPresentScence(PresentScence scence)
{
	APP_TASK_DEBUG(("SetPresentScence:%d\n", scence));
	koovox.presentScence = scence;
}

/*************************************************************************
NAME
    GetPresentScenceEvent
    
DESCRIPTION
    get the present scence event
*/
uint8 GetPresentScence(void)
{
	return koovox.presentScence;
}


/*************************************************************************
NAME
    GetPresentScenceEvent
    
DESCRIPTION
    get the present scence event
*/
uint16 GetPresentScenceEvent(void)
{
	uint16 event = 0;

	if(isPresentAuto())
	{
		switch(koovox.presentScence)
		{
			case MEETING_SCENCE:
				event = EventKoovoxPromptMeeting;
				break;
			
			case DRIVING_SCENCE:
				event = EventKoovoxPromptDriving;
				break;
			
			case LEAVING_SCENCE:
				event = EventKoovoxPromptLeaving;
				break;
			
			case NO_DISTURBING_SCENCE:
				event = EventKoovoxPromptNoDisturbing;
				break;
			default:
				event = EventKoovoxPromptNoDisturbing;
				
		}
	}
	else
	{
		event = EventKoovoxPromptNoDisturbing;
	}
	
    APP_TASK_DEBUG(("GetPresentScenceEvent:%x\n", event)) ;

	return event;
}



/*************************************************************************
NAME    
    handleSPPMessage
    
DESCRIPTION
    handles messages from the SPP

RETURNS
    
*/
static void handleSPPMessage ( Task task, MessageId id, Message message )
{
    APP_TASK_DEBUG(("SPP_U_MSG: [%x][%x][%x]\n", (int)task , (int)id , (int)&message)) ;

    switch(id)
    {
        case SPP_SERVER_CONNECT_CFM:
			{
				SPP_SERVER_CONNECT_CFM_T *sscc = (SPP_SERVER_CONNECT_CFM_T *) message;
                if (spp_connect_success == sscc->status)
                {
					KoovoxAppConnection(sscc->sink, 0, SPP_CONNECTED);
					SetVoiceMessageEnable(VM_ENABLE);
#if defined(BLE_ENABLED)
					stop_ble_advertising();
#endif
					APP_TASK_DEBUG(("SPP_SERVER_CONNECT_CFM\n"));
				}
				else
				{
                    APP_TASK_DEBUG(("failure: %d\n", sscc->status));
                    Panic();
				}

			}
            break;

        case SPP_CONNECT_IND:
            {
                SPP_CONNECT_IND_T *sci = (SPP_CONNECT_IND_T *)message; 
                APP_TASK_DEBUG(("SPP_CONNECT_IND\n"));                
              
                /* Respond to accept the connection.*/
                SppConnectResponse(
                        task, 
                        (const bdaddr *)(&(sci->addr)), 
                        TRUE,                      /* Accept the connection */
                        sci->sink,
                        sci->server_channel,
                        0                       /* Use default payload size */
                        );
            }
            break;                

        case SPP_MESSAGE_MORE_DATA:
			{
				SPP_MESSAGE_MORE_DATA_T* smmd = (SPP_MESSAGE_MORE_DATA_T *) message;
				uint16 size = SourceSize(smmd->source);
				const uint8* msg = SourceMap(smmd->source);
				uint16 ret = 0;
				
				APP_TASK_DEBUG(("SPP_MESSAGE_MORE_DATA:size= %x\n", size));

#ifdef DEBUG_APP_MESSAGE
				{
					uint8 i = 0;
					DEBUG(("msg:"));
					for(; i < size; i++)
					{
						DEBUG(("%x ", msg[i]));
					}
					DEBUG(("\n"));
				}
#endif	

				ret = KoovoxMessageStr(msg, FRAME_TAIL, size);
				if(ret)
				{
					memcpy(koovox.sppValue + koovox.sizeSppValue, msg, ret);
					koovox.sizeSppValue += ret;
					DisposeSppMessage(koovox.sppValue, koovox.sizeSppValue - 2);
					memset(koovox.sppValue, 0, ret);
					koovox.sizeSppValue = 0;
					if((size > MAX_DATA_BUF)&&(size - ret) < MAX_DATA_BUF)
					{
						memcpy(koovox.sppValue + koovox.sizeSppValue, msg + ret, size - ret);
						koovox.sizeSppValue += size - ret;

						ret = KoovoxMessageStr(msg + ret, FRAME_TAIL, size - ret);
						if(ret)
						{
							/* dispose the spp message */
							DisposeSppMessage(koovox.sppValue, koovox.sizeSppValue - 2);
							memset(koovox.sppValue, 0, MAX_DATA_BUF);
							koovox.sizeSppValue = 0;
						}
					}
					
				}
															
				SourceDrop(smmd->source, SourceSize(smmd->source));
			}
            break;
			
        case SPP_MESSAGE_MORE_SPACE:
			{
				APP_TASK_DEBUG(("SPP_MESSAGE_MORE_SPACE\n"));
			}
			break;

        case SPP_DISCONNECT_IND:
            {
				SPP_DISCONNECT_IND_T *sdi = (SPP_DISCONNECT_IND_T *) message;
				SppDisconnectResponse( sdi->spp);
				KoovoxAppDisconnection();
#if defined(BLE_ENABLED)
				start_ble_advertising();
#endif
                APP_TASK_DEBUG(("SPP_DISCONNECT_IND - status\n"));
            }
            break;            
			
		default:
			APP_TASK_DEBUG(("undefined messageid!\n"));
			break;

    }
}


/*************************************************************************
NAME
    init_koovox_task
    
DESCRIPTION
    init koovox task  
*/
void init_koovox_task(void)
{
	/* Ensure phone app is never initialised more than once */
	if(koovox.initialised == FALSE)
	{
		APP_TASK_DEBUG(("init_koovox_task\n"));
		
		koovox.initialised = TRUE;

		/* init message handler */
		koovox.task.handler = koovox_message_handler;

		/* start spp server*/
		SppStartService(&(koovox.task));

		HeartRateSensorDisable();

#if 1
		/* set dsp control the i2c pio: pin6, pin7 */
		PioSetKalimbaControl32(0xc0, 0xc0);
#else
{
		uint8 value = 0;
		/*value = LIS3DH_read_reg(WHO_AM_I);*/
		InitialiseI2C_LIS3DH();
		DEBUG(("who_am_i: %d\n", value));
}
#endif

#if 0
		/* mount external flash to filesystem */
		PartitionMountFilesystem(PARTITION_SERIAL_FLASH , 0x01, PARTITION_LOWER_PRIORITY);
#endif

	}
}

/*************************************************************************
NAME
    koovox_message_handler
    
DESCRIPTION
    Handle messages between phone and koovox 
*/
void koovox_message_handler( Task task, MessageId id, Message message )
{
	APP_TASK_DEBUG(("KOOVOX_TASK: id:%x\n", id));
	
    if ( (id >= SPP_MESSAGE_BASE) && (id <= SPP_MESSAGE_TOP) )
    {
        handleSPPMessage(task, id, message);
    }
	else if((id >= EVENTS_MESSAGE_BASE) && (id <= EVENTS_LAST_EVENT))
	{
		/* dispose the koovox event */
		KoovoxEventHandler(id); 
	}
	/* rcv message from the dsp port */
	else if(id == MESSAGE_FROM_KALIMBA)
	{
		APP_TASK_DEBUG(("MESSAGE_FROM_KALIMBA\n"));
		/* read value from dsp */
		KoovoxRcvShortKalimbaMsg(message);	
	}
	else if(id == MESSAGE_FROM_KALIMBA_LONG)
	{
		KoovoxRcvLongKalimbaMsg(message);
	}
	else if(id == MESSAGE_STREAM_PARTITION_VERIFY)
	{
		DEBUG(("MESSAGE_STREAM_PARTITION_VERIFY\n"));
	}
	else
	{
		APP_TASK_DEBUG(("undefined messageid:%x\n", id));
	}
	
}

#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    

#endif /* ENABLE_KOOVOX */

