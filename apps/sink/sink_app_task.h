/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2014
Part of ADK 3.5

FILE NAME
    sink_app_task.h

DESCRIPTION
    Header file for sink_app_task.c.

*/
#ifdef ENABLE_KOOVOX

#ifndef __sink_app_task_H
#define __sink_app_task_H

#include <message.h>
#include <csrtypes.h>
#include <partition.h>

#include "sink_private.h"

#ifdef DEBUG_APP_TASK
#define APP_TASK_DEBUG(x)	DEBUG(x)
#else
#define APP_TASK_DEBUG(x)
#endif

#define VALUE_MAX				(200)	/* the max of heart rate value */
#define VALUE_MIN				(20)	/* the min of heart rate value */
#define SIZE_ANGLE_VALUE		(3)		

/* app connect status */
typedef enum{
	DISCONNECTED,
	SPP_CONNECTED,
	GATT_CONNECTED
}ConnectStatus;

/* DSP message structure */
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;

typedef struct{
	uint16 id;
	uint16 size_value;
	uint16 value[1];
}dsp_long_msg;

/* present scence */
typedef enum{
	IDLE_SCENCE = 0x00,
	MEETING_SCENCE,
	DRIVING_SCENCE,
	LEAVING_SCENCE,
	NO_DISTURBING_SCENCE
}PresentScence;

/* koovox mode FSM */
typedef enum{
	BUSINESS_MODE,
	MUSIC_MODE,
	SPORT_MODE,
	CALL_MODE,
	ASR_MODE
}ModeFSM;

typedef enum{
	RECORD_IDLE,
	RECORD_BUSY
}enumRecordStatus;

typedef enum{
	PRESET_IDLE,
	PRESET_BUSY
}enumPresenceStatus;

typedef enum{
	SAMPLE_DISABLE,
	SPORT_ENABLE,
	HB_ENABLE,
	NECK_PROTECT,
	SAFE_DRIVER,
	CONST_SEAT
}enumSampleStatus;

typedef enum{
	MUTE_ENABLE,
	MUTE_DISABLE
}enumMuteStatus;

typedef enum{
	PROMPT_DISABLE,
	PROMPT_ENABLE
}enumHBPrompt;

typedef enum{
	RSP_SUCCESS,
	RSP_TIMEOUT
}enumResponseStatus;

typedef enum{
	ASR_IDLE,
	ASR_BUSY
}enumAsrStatus;

typedef enum{
    M_DSP_NOT_LOADED,
    M_DSP_LOADING,
    M_DSP_LOADED_IDLE,
    M_DSP_RUNNING
}enumDspStatus;

typedef enum{
	MUSIC_AUTO,
	MUSIC_MANUAL
}enumMusicStatus;

typedef enum{
	DRIVER_DISABLE,
	DRIVER_ENABLE
}enumDriver;

typedef enum{
	ACTION_CALL_DISABLE,
	ACTION_CALL_ENABLE
}enumActionCall;


typedef enum{
	NECK_DISABLE,
	NECK_ENABLE
}enumNeck;

typedef enum{
	SEAT_DISABLE,
	SEAT_ENABLE
}enumSeat;

typedef enum{
	VP_ENABLE,
	VP_DISABLE
}enumVPStatus;

typedef enum{
	VP_AUTO_DISABLE,
	VP_AUTO_ENABLE
}enumVPAuto;

typedef enum{
	VM_ENABLE,
	VM_DISABLE
}enumVMStatus;

/* koovox task data */
typedef struct{
    TaskData        task;           /* The phone app  message handler */
    bool            initialised;    /* Flag set when phone app has been initialised for the device */
	uint8			rspStatus;		/* status of the response from the mobile timeout or not */
	uint8			connectStatus;  /* The phone app connect status */
	uint8			currMode;		/* the current mode  */
	uint8			prevMode;		/* the previous mode */
	uint8 			recordStatus;	/* the record status */
	uint8 			presentStatus;	/* the presence status */
	uint8		 	presentScence;	/* the user set present scence */
	uint8 			asrStatus;		/* the ASR status */
	uint8 			musicStatus;	/* the music mode is auto or manual */
	uint8 			muteStatus;		/* the mute micphone status */

	uint8 			neckEnable;		/* enable or disable neck protect business */
	uint8			seatEnable;		/* enable or disable const seat business */
	uint8			hbEnable;		/* enable or disable heart rate business */
	uint16			last_cmd;		/* the last of the cmd to STM8 */

	uint8 			driverEnable;	/* enable or disable driver business */
	uint8			vp_enable;		/* enable or disable present business */
	uint8 			vp_auto;		/* set auto present business */
	uint8 			vm_enable;		/* enable or disable voice message business */
	uint8			action_call;	
	
    Sink 			sppSink;        /* Stream sink of the spp transport. */	
    uint16          cid;		    /* Connection identifier of remote device. */
    uint16          handle;		    /* Handle being accessed. */
	uint8* 			sppValue;	/* value from the mobile */
	uint16			sizeSppValue;		/* size of the value */
	uint8*			getValue;
	uint16			sizeGetValue;
	uint8*			setValue;
	uint16			sizeSetValue;
	
	uint8 			sampleStatus;	/* the heart rate sensor sample status */
	uint8			promptStatus;	/* the heart rate value prompt status*/
	uint8 			heartRateValue;	/* the heart rate value */
	uint8 			batteryValue;	/* the koovox battery value */
	uint32			step_value;		/* store the accelerate step value */
	uint16 			time_value;		/* store the sport time */

	uint8			angle_init_cnt;	/* */
	
}gKoovoxTaskData;

/*************************************************************************
NAME
    SetNeckProtectEnable
    
DESCRIPTION
    enable or disable the koovox neck protect business
*/
void SetNeckProtectEnable(uint8 status);

/*************************************************************************
NAME
    isNeckProtectEnable
    
DESCRIPTION
    chech the neck protect business is enable
*/
bool isNeckProtectEnable(void);

/*************************************************************************
NAME
    SetDriverEnable
    
DESCRIPTION
    enable or disable the koovox driver business
*/
void SetDriverEnable(uint8 status);

/*************************************************************************
NAME
    isDriverEnable
    
DESCRIPTION
    chech the driver business is enable
*/
bool isDriverEnable(void);

/*************************************************************************
NAME
    SetActionCallEnable
    
DESCRIPTION
    enable or disable the koovox action call business
*/
void SetActionCallEnable(uint8 status);


/*************************************************************************
NAME
    isActionCallEnable
    
DESCRIPTION
    chech the action call business is enable
*/
bool isActionCallEnable(void);

/*************************************************************************
NAME
    SetPresentEnable
    
DESCRIPTION
    enable or disable the koovox presence business
*/
void SetPresentEnable(uint8 status);


/*************************************************************************
NAME
    isPresentEnable
    
DESCRIPTION
    chech the presence business is enable
*/
bool isPresentEnable(void);

/*************************************************************************
NAME
    SetPresentAuto
    
DESCRIPTION
    enable or disable auto present 
*/
void SetPresentAuto(uint8 status);

/*************************************************************************
NAME
    isPresentAuto
    
DESCRIPTION
    chech the presence business is auto
*/
bool isPresentAuto(void);

/*************************************************************************
NAME
    SetVoiceMessageEnable
    
DESCRIPTION
    enable or disable the koovox voice message business
*/
void SetVoiceMessageEnable(uint8 status);

/*************************************************************************
NAME
    isVoiceMessageEnable
    
DESCRIPTION
    chech the voice message business is enable
*/
bool isVoiceMessageEnable(void);


/*************************************************************************
NAME
    SetResponseStatus
    
DESCRIPTION
    set response status 
*/
void SetResponseStatus(uint8 status);


/*************************************************************************
NAME
    isAutoMusic
    
DESCRIPTION
    chech the music is auto play or not
*/
bool isAutoMusic(void);

/*************************************************************************
NAME
    SetMusicStatus
    
DESCRIPTION
    set music status auto or manual
*/
void SetMusicStatus(uint8 status);

/*************************************************************************
NAME
    isResponseTimeout
    
DESCRIPTION
    chech the response timeout or not 
*/
bool isResponseTimeout(void);

/*************************************************************************
NAME
    isOnlineState
    
DESCRIPTION
    chech the app connected 
*/
bool isOnlineState(void);

/*************************************************************************
NAME
    isSppConnected
    
DESCRIPTION
    chech the app connected through spp profile
*/
bool isSppConnected(void);

/*************************************************************************
NAME
    isGattConnected
    
DESCRIPTION
    chech the app connected through gatt profile
*/
bool isGattConnected(void);

/*************************************************************************
NAME
    UpdateBatteryValue
    
DESCRIPTION
   update the koovox battery value
*/
void UpdateBatteryValue(uint8 value);

/*************************************************************************
NAME
    GetBatteryValue
    
DESCRIPTION
   get the koovox battery value
*/
uint8 GetBatteryValue(void);

/*************************************************************************
NAME
    UpdateStepValue
    
DESCRIPTION
   update the koovox step value
*/
void UpdateStepValue(uint16 value);

/*************************************************************************
NAME
    ClearStepValue
    
DESCRIPTION
   clear the koovox step value
*/
void ClearStepValue(void);

/*************************************************************************
NAME
    GetStepValue
    
DESCRIPTION
   get the koovox step value
*/
uint32 GetStepValue(void);

/*************************************************************************
NAME
    UpdateTimeValue
    
DESCRIPTION
   update the koovox sport time value
*/
void UpdateTimeValue(uint16 value);

/*************************************************************************
NAME
    ClearTimeValue
    
DESCRIPTION
   clear the koovox sport time value
*/
void ClearTimeValue(void);

/*************************************************************************
NAME
    GetTimeValue
    
DESCRIPTION
   get the koovox sport time value
*/
uint16 GetTimeValue(void);


/*************************************************************************
NAME
    UpdateHeartRateValue
    
DESCRIPTION
   update the koovox heart rate value
*/
void UpdateHeartRateValue(uint8 value);

/*************************************************************************
NAME
    GetHeartRateValue
    
DESCRIPTION
   get the koovox heart rate value
*/
uint8 GetHeartRateValue(void);

/*************************************************************************
NAME
    init_koovox_task
    
DESCRIPTION
    init phone app task  
*/
void init_koovox_task(void);


/*************************************************************************
NAME
    koovox_message_handler
    
DESCRIPTION
    Handle messages between phone and bluetooth 
*/
void koovox_message_handler( Task task, MessageId id, Message message );

/*************************************************************************
NAME
    SetSampleStatus
    
DESCRIPTION
    set the heart rate sample  
*/

void SetSampleStatus(uint8 status);

/*************************************************************************
NAME
    GetSampleStatus
    
DESCRIPTION
    check the sample enable or disable 
*/
uint8 GetSampleStatus(void);

/*************************************************************************
NAME
    SetMuteStatus
    
DESCRIPTION
    set micphone mute status  
*/
void SetMuteStatus(uint8 status);


/*************************************************************************
NAME
    isMuteEnable
    
DESCRIPTION
    check the micphone mute enable or disable 
*/
bool isMuteEnable(void);

/*************************************************************************
NAME
    SetPromptStatus
    
DESCRIPTION
    set the heart rate prompt status 
*/
void SetPromptStatus(uint8 status);

/*************************************************************************
NAME
    isPromptEnable
    
DESCRIPTION
    check the heart rate value prompt enable or disable 
*/
bool isPromptEnable(void);

/*************************************************************************
NAME
    StorePreviousMode
    
DESCRIPTION
    store the koovox previous mode 
*/
void StorePreviousMode(uint8 mode);

/*************************************************************************
NAME
    GetPreviousMode
    
DESCRIPTION
    get previous mode 
*/
ModeFSM GetPreviousMode(void);

/*************************************************************************
NAME
    RecoverPreviousMode
    
DESCRIPTION
    recover the koovox previous mode 
*/
void RecoverPreviousMode(void);

/*************************************************************************
NAME
    SetAsrStatus
    
DESCRIPTION
    set the ASR status 
*/
void SetAsrStatus(uint8 status);

/*************************************************************************
NAME
    isAsrBusy
    
DESCRIPTION
    check the koovox ASR busy or not
*/
bool isAsrBusy(void);

/*************************************************************************
NAME
    SetCurrMode
    
DESCRIPTION
    set the current mode 
*/
void SetCurrMode(ModeFSM state);

/*************************************************************************
NAME
    GetCurrMode
    
DESCRIPTION
    get the current mode 
*/
ModeFSM GetCurrMode(void);

/*************************************************************************
NAME
    SetRecordStatus
    
DESCRIPTION
    set the koovox record status 
*/
void SetRecordStatus(uint8 status);

/*************************************************************************
NAME
    isRecordBusy
    
DESCRIPTION
    check the koovox record busy or not
*/
bool isRecordBusy(void);


/*************************************************************************
NAME
    SetPresentStatus
    
DESCRIPTION
    set the koovox presence status 
*/
void SetPresentStatus(uint8 status);

/*************************************************************************
NAME
    isPresentBusy
    
DESCRIPTION
    chech the device is presence state
*/
bool isPresentBusy(void);

/*************************************************************************
NAME
    SetPresentScence
    
DESCRIPTION
    set the present scence
*/
void SetPresentScence(PresentScence scence);

/*************************************************************************
NAME
    GetPresentScenceEvent
    
DESCRIPTION
    get the present scence event
*/
uint8 GetPresentScence(void);

/*************************************************************************
NAME
    GetPresentScenceEvent
    
DESCRIPTION
    get the present scence event
*/
uint16 GetPresentScenceEvent(void);


/*the phone app task data structure - visible to all (so don't pass it between functions!)*/
extern gKoovoxTaskData koovox;

#endif /* __sink_app_task_H */

#endif /* ENABLE_KOOVOX */

