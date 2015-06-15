/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_simple_text_to_speech_plugin.c
DESCRIPTION
    an audio plugin
NOTES
*/

#include <audio.h>
#include <codec.h>
#include <stdlib.h>
#define DEBUG_PRINT_ENABLEDx
#include <print.h>
#include <stream.h> /*for the ringtone_note*/
#include <string.h>
#include "audio_plugin_if.h" /*the messaging interface*/
#include "koovox_sport_common_plugin.h"
#include "koovox_sport_common.h"
#include <kalimba.h>

/* DSP message structure */
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;


/*the local kalimba message handling function*/
static void handleKalimbaMessage (Task task , Message message);

/*the local kalimba message long handling function*/
static void handleKalimbaMessageLong (Task task , Message message);

/*the task message handler*/
static void message_handler (Task task, MessageId id, Message message);

/*the local message handling functions*/
static void handleAudioMessage (Task task , MessageId id, Message message);
static void handleInternalMessage (Task task , MessageId id, Message message);
    
/*the plugin task*/
const TaskData koovox_sport_common_plugin = { message_handler };

/****************************************************************************
DESCRIPTION
    The main task message handler
*/
static void message_handler ( Task task, MessageId id, Message message ) 
{
    if ( (id >= AUDIO_DOWNSTREAM_MESSAGE_BASE ) && (id <= AUDIO_DOWNSTREAM_MESSAGE_TOP) )
    {
        handleAudioMessage (task , id, message ) ;
    }
    else if (id == MESSAGE_FROM_KALIMBA)
    {
        handleKalimbaMessage (task , message ) ;
    }
	else if (id == MESSAGE_FROM_KALIMBA_LONG)
	{
		handleKalimbaMessageLong(task, message);
	}
    else
    {
        handleInternalMessage (task , id , message ) ;
    }
}    

/****************************************************************************
DESCRIPTION

    messages from the audio library are received here. 
    and converted into function calls to be implemented in the 
    plugin module
*/ 
static void handleAudioMessage ( Task task , MessageId id, Message message )     
{
    switch (id)
    {
        
        case (AUDIO_PLUGIN_SPORT_MODE_PLAY_PROMPT_MSG):
        {
            AUDIO_PLUGIN_SPORT_MODE_PLAY_PROMPT_MSG_T* sport_message = (AUDIO_PLUGIN_SPORT_MODE_PLAY_PROMPT_MSG_T *)message ;

			PRINT(("AUDIO_PLUGIN_SPORT_MODE_PLAY_PROMPT_MSG\n"));
            if ((IsAudioBusy())||(GetCurrentDspStatus() && (GetCurrentDspStatus() != DSP_RUNNING))||IsSportRunning())
            {
            
			}
			else
			{
				KoovoxSportCommonPluginStart ( sport_message->sensor_sample, sport_message->sensor_value, sport_message->app_task);
				PRINT(("SPORT:start\n"));
			}		
        }
        break ;
        
        case (AUDIO_PLUGIN_SPORT_MODE_STOP_PROMPT_MSG ):
        {
			PRINT(("AUDIO_PLUGIN_SPORT_MODE_STOP_PROMPT_MSG\n"));

			if(GetCurrentDspStatus() == DSP_RUNNING)
			{
				KoovoxStopSportKalimba();
				KoovoxPowerOffKalimba();
			}
        }
        break;
		
        default:
            /*other messages do not need to be handled by the voice prompts plugin*/
        break ;
    }
}

/****************************************************************************
DESCRIPTION
    kalimba messages to the task are handled here
*/ 
static void handleKalimbaMessage ( Task task , Message message )
{
    KoovoxMessageFromKalimba(message);
}

/****************************************************************************
DESCRIPTION
    kalimba long messages to the task are handled here
*/ 
static void handleKalimbaMessageLong( Task task , Message message )
{
    KoovoxMessageFromKalimbaLong(message);
}


/****************************************************************************
DESCRIPTION
    Internal messages to the task are handled here
*/ 
static void handleInternalMessage ( Task task , MessageId id, Message message )     
{
    switch (id)
    {
        case MESSAGE_STREAM_DISCONNECT:
        {        
   
        }
        break ;
        
        case MESSAGE_MORE_SPACE:
            PRINT(("VP: MESSAGE_MORE_SPACE\n"));
        break;
        
        default:
        break ;
    }
}
