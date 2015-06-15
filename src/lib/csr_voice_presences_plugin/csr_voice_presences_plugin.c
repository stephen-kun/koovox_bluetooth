/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_voice_presences_plugin.c
DESCRIPTION
    plugin implementation which plays audio prompts
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
#include "csr_voice_presences.h"
#include "csr_voice_presences_plugin.h"

/* Messages from DSP */
#define MUSIC_READY_MSG           (0x1000)
#define MUSIC_SETMODE_MSG         (0x1001)
#define MUSIC_VOLUME_MSG          (0x1002)
#define MUSIC_CODEC_MSG           (0x1006)
#define MUSIC_TONE_END            (0x1080)
#define MUSIC_LOADPARAMS_MSG      (0x1012)

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

/*the task message handler*/
static void message_handler (Task task, MessageId id, Message message);

/*the local message handling functions*/
static void handleAudioMessage (Task task , MessageId id, Message message);
static void handleInternalMessage (Task task , MessageId id, Message message);
    
/*the plugin task*/
const TaskData csr_voice_presences_plugin = { message_handler };


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
        case (AUDIO_PLUGIN_VOICE_PROMPTS_INIT_MSG ):
        {
            AUDIO_PLUGIN_VOICE_PROMPTS_INIT_MSG_T * init_message = (AUDIO_PLUGIN_VOICE_PROMPTS_INIT_MSG_T *)message;
            CsrVoicePresencesPluginInit(init_message->no_prompts, init_message->no_languages);
        }
        break;
		
        case (AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG ):
        {
            AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG_T * prompt_message = (AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG_T *)message ;
                           
            /* if the audio is currently busy then queue this audio prompt request, or if the audio plugin is in an 'about to be loaded state', queue until it is
                                     loaded and running */
            if ((IsAudioBusy())||(GetCurrentDspStatus() && (GetCurrentDspStatus() != DSP_RUNNING)))
            {    
                if ( prompt_message->can_queue) 
                {                
                    MAKE_AUDIO_MESSAGE( AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG) ; 
                    memmove(message, prompt_message, sizeof(AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG_T));
                    MessageSendConditionally ( task , AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG, message ,(const uint16 *)AudioBusyPtr() ) ;
                    PRINT(("PRESENT:Queue\n")); 
                }
            }
            else
            {
                CsrVoicePresencesPluginPlayPhrase ( prompt_message->id, prompt_message->language, 
                                                  prompt_message->codec_task, prompt_message->ap_volume, prompt_message->features, prompt_message->app_task);
                PRINT(("PRESENT:start\n"));
            } 
        }
        break ;

        case (AUDIO_PLUGIN_STOP_AUDIO_PROMPT_MSG ):
        {
			PRINT(("PRESENT:stop\n"));
            if(IsAudioBusy()) 
            {
                CsrVoicePresencesPluginStopPhrase() ;
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
    const DSP_REGISTER_T *m = (const DSP_REGISTER_T *) message;
    PRINT(("handleKalimbaMessage: msg id[%x] a[%x] b[%x] c[%x] d[%x]\n", m->id, m->a, m->b, m->c, m->d));
    
    switch ( m->id ) 
    {	
        case MUSIC_READY_MSG:
        {
            PRINT(("PRESENT: KalMsg MUSIC_READY_MSG\n"));
        }
        break;
        
        case MUSIC_CODEC_MSG:
        break;
        
        case MUSIC_TONE_END:
        {
            PRINT(("PRESENT: KalMsg MUSIC_TONE_END\n"));
            if(IsAudioBusy() && (IsAudioBusy() == (TaskData*) &csr_voice_presences_plugin))
            {
                MessageCancelAll ((TaskData*)IsAudioBusy(), AUDIO_PLUGIN_STOP_AUDIO_PROMPT_MSG ) ;
                MessageSend( (TaskData*) &csr_voice_presences_plugin, AUDIO_PLUGIN_STOP_AUDIO_PROMPT_MSG, NULL );    
            }            
        }
        break;
        
        default:
        {
            PRINT(("handleKalimbaMessage: unhandled %X\n", m->id));
        }
        break;
    }
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
            PRINT(("PRESENT: MESSAGE_STREAM_DISCONNECT\n"));
   
            if(IsAudioBusy() && (IsAudioBusy() == (TaskData*) &csr_voice_presences_plugin))
            {
                MessageCancelAll ((TaskData*)IsAudioBusy(), AUDIO_PLUGIN_STOP_AUDIO_PROMPT_MSG ) ;
                MessageSend( (TaskData*) &csr_voice_presences_plugin, AUDIO_PLUGIN_STOP_AUDIO_PROMPT_MSG, NULL );
            }    
        }
        break ;
        
        case MESSAGE_MORE_SPACE:
            PRINT(("PRESENT: MESSAGE_MORE_SPACE\n"));
        break;
        
        default:
            Panic() ;
        break ;
    }
}
