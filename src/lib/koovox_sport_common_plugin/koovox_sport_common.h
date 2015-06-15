/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    koovox_sport_common.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _KOOVOX_SPORT_COMMON_H_
#define _KOOVOX_SPORT_COMMON_H_

typedef enum
{
    kalimba_idle,
    kalimba_loaded,
    kalimba_ready
} kalimba_state;

void KoovoxPromptsPluginPlayDsp(kalimba_state state);
void KoovoxMessageFromKalimba(Message message ) ;
void KoovoxMessageFromKalimbaLong(Message message);
void KoovoxSportCommonPluginStart (uint8 sensor_sample , uint16 sensor_value, Task app_task);
void KoovoxPowerOffKalimba(void);

#endif


