/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2004-2014
Part of ADK 3.5

FILE NAME
    sink_app_core.h

DESCRIPTION
    Header file for sink_app_core.c.

*/
#ifdef ENABLE_KOOVOX

#ifndef __SINK_APP_CORE_H
#define __SINK_APP_CORE_H

#include <csrtypes.h>
#include <source.h>
#include "sink_private.h"

#define RECORD_STREAM_PORT		(4)
#define PRESENT_VOICE_PORT		(1)

#define PHONE_NUMBER_SIZE			(12)
#define SPORT_DATA_SIZE				(3)

#define HEART_RATE_SENSOR_PIO	(0x5)

/* interval times prompt heart rate to user (s)*/
#define AUDIO_PROMPT_TIME		(30)


#ifdef DEBUG_APP_CORE
#define APP_CORE_DEBUG(x)	DEBUG(x)
#else
#define APP_CORE_DEBUG(x)
#endif

#ifdef ENABLE_LOG
#define APP_CORE_LOG(x)	{KoovoxLog x;}
#else
#define APP_CORE_LOG(x)
#endif



typedef void (*pActionHandle)(void);
typedef bool (*pValidHandle)(void);


typedef struct{
	uint16 event;
	pValidHandle isValid;
	pActionHandle actionHandler;
}ModeFSMActionHandle_t;

void logTest(void);

/****************************************************************************
FUNCTIONS pValidHandle
*/
bool isIncomingState(void);
bool isOutgoingState(void);
bool isActiveState(void);
bool isConnectableState(void);
bool isConnectedState(void);
bool isA2dpStreamState(void);

/*************************************************************************
NAME
    KoovoxLog
    
DESCRIPTION
    send log message to mobile phone 
*/
void KoovoxLog(const char* value_data);

/*************************************************************************
NAME
    KoovoxTimeoutHandler
    
DESCRIPTION
    dispose the EventKoovoxTimeout event
*/
void KoovoxTimeoutHandler(void);

/*************************************************************************
NAME
    KoovoxStartConstSeatPrompt
    
DESCRIPTION
    
*/
void KoovoxStartConstSeatPrompt(void);

/*************************************************************************
NAME
    KoovoxStopConstSeatPrompt
    
DESCRIPTION
    
*/
void KoovoxStopConstSeatPrompt(void);

/*************************************************************************
NAME
    KoovoxStartNeckProtect
    
DESCRIPTION
    
*/
void KoovoxStartNeckProtect(void);

/*************************************************************************
NAME
    KoovoxStopNeckProtect
    
DESCRIPTION
    
*/
void KoovoxStopNeckProtect(void);

/*************************************************************************
NAME
    KoovoxStartSafeDriver
    
DESCRIPTION
    
*/
void KoovoxStartSafeDriver(void);

/*************************************************************************
NAME
    KoovoxStopSafeDriver
    
DESCRIPTION
    
*/
void KoovoxStopSafeDriver(void);


/*************************************************************************
NAME
    KoovoxRestartDsp
    
DESCRIPTION
    reload the kalimba file
*/
void KoovoxRestartDsp(void);

/****************************************************************************
NAME 
    KoovoPromptPlayNumber
DESCRIPTION
    Play a uint32 using the Audio prompt plugin
RETURNS    
*/
void KoovoPromptPlayNumber(uint32 number);

/****************************************************************************
NAME 
    KoovoPromptPlayHeartRate
DESCRIPTION
    audio prompt heart rate 
RETURNS    
*/
void KoovoPromptPlayHeartRate(uint32 number);

/****************************************************************************
NAME 
    KoovoxPromptPlayBattery
DESCRIPTION
    audio prompt battery
RETURNS    
*/
void KoovoxPromptPlayBattery(uint32 value);

/****************************************************************************
NAME 
    KoovoxCurrentModePrompt
DESCRIPTION
    audio prompt current mode
RETURNS    
*/
void KoovoxCurrentModePrompt(uint8 state);

/****************************************************************************
NAME 
    KoovoxSetModePrompt
DESCRIPTION
    set koovox mode and audio prompt mode
RETURNS    
*/
void KoovoxSetModePrompt(uint8 state);

/****************************************************************************
NAME
    KoovoxPowerOnPrompt
    
DESCRIPTION
    
*/
void KoovoxPowerOnPrompt(void);

/*************************************************************************
NAME
    KoovoxSwitchToCallMode
    
DESCRIPTION
    switch to call mode 
*/
void KoovoxSwitchToCallMode(void);

/*******************************************************************************
FUNCTION
    get_battery_level_as_percentage
    
DESCRIPTION
    Helper function to read the battery level and retun as a value 0-100
*/
uint8 get_battery_level_as_percentage(void);

/*************************************************************************
NAME
    KoovoxStartHeartRateMeasure
    
DESCRIPTION
    enable heart rate sensor and start sampling
*/
void KoovoxStartHeartRateMeasure(void);

/*************************************************************************
NAME
    KoovoxStopHeartRateMeasure
    
DESCRIPTION
    stop sport mode ( disable heart rate sensor )
*/
void KoovoxStopHeartRateMeasure(void);

/*************************************************************************
NAME
    KoovoxStartSportMode
    
DESCRIPTION
    enable heart rate sensor and start sampling
*/
void KoovoxStartSportMode(void);

/*************************************************************************
NAME
    KoovoxStopSportMode
    
DESCRIPTION
    stop sport mode ( disable heart rate sensor )
*/
void KoovoxStopSportMode(void);


/*************************************************************************
NAME
    KoovoxResultI2cTest
    
DESCRIPTION
    
*/
void KoovoxResultI2cTest(uint8* data, uint8 size_data);


/****************************************************************************
NAME
    KoovoxAudioPresencePlayEvent
    
DESCRIPTION
    play presece audio to peer listenning
*/
bool KoovoxAudioPresencePlayEvent (uint16 event);


/*************************************************************************
NAME    
    KoovoxRcvShortKalimbaMsg
    
DESCRIPTION
    receive short message from the kalimba

RETURNS
    void
*/
void KoovoxRcvShortKalimbaMsg(Message msg);

/*************************************************************************
NAME    
    KoovoxRcvLongKalimbaMsg
    
DESCRIPTION
    receive long message from the kalimba

RETURNS
*/
void KoovoxRcvLongKalimbaMsg(Message msg);

/****************************************************************************
NAME
    KoovoxAutoLeaveToMusicMode
    
DESCRIPTION
    koovox auto switch to music mode 
*/
void KoovoxAutoSwitchToMusicMode(void);

/****************************************************************************
NAME
    KoovoxAutoLeaveToMusicMode
    
DESCRIPTION
    koovox auto leave to music mode 
*/
void KoovoxAutoLeaveToMusicMode(void);

/*************************************************************************
NAME
    DisposeUserEventPromptHBValue
    
DESCRIPTION
    audio prompt the heart rate value to user
*/
void KoovoxEventHandler(uint16 event);

/****************************************************************************
NAME
    KoovoxHandleEventCvcKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventCvcKalimbaLoaded(void);

/****************************************************************************
NAME
    KoovoxHandleEventCodecKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventCodecKalimbaLoaded(void);


/****************************************************************************
NAME
    KoovoxHandleEventSportKalimbaLoaded
    
DESCRIPTION
    
*/
void KoovoxHandleEventSportKalimbaLoaded(void);

/****************************************************************************
NAME
    KoovoxAudioPromptTime
    
DESCRIPTION
    
*/
void KoovoxAudioPromptTime(uint8 hour, uint8 min);

/****************************************************************************
NAME
    KoovoxAudioPromptSportData
    
DESCRIPTION
    
*/
void KoovoxAudioPromptSportData(uint16 mileage, uint8 hour, uint8 min);


/****************************************************************************
NAME	
	HeartRateSensorEnable  
    
RETURNS
	void
*/
void HeartRateSensorEnable(void);

/****************************************************************************
NAME	
	HeartRateSensorDisable  
    
RETURNS
	void
*/
void HeartRateSensorDisable(void);


#endif /* __SINK_APP_CORE_H */

#endif /* ENABLE_KOOVOX */
