/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_voice_prompts.h
DESCRIPTION
    plugin implementation which plays audio prompts
NOTES
*/

#include <audio.h>
#include <codec.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#define DEBUG_PRINT_ENABLEDx
#include <print.h>
#include <stream.h> 
#include <message.h> 
#include <string.h>
#include <source.h>
#include <transform.h>
#include <i2c.h>

#include <file.h>
#include <stdio.h>
#include "audio_plugin_if.h" 
#include <kalimba.h>
#include <kalimba_standard_messages.h>

#include "audio.h"
#include "audio_plugin_if.h"
#include "koovox_sport_common_plugin.h"
#include "koovox_sport_common.h"

/* Messages from DSP */
#define MUSIC_READY_MSG           (0x1000)
#define MUSIC_SETMODE_MSG         (0x1001)
#define MUSIC_VOLUME_MSG          (0x1002)
#define MUSIC_CODEC_MSG           (0x1006)
#define MUSIC_TONE_END            (0x1080)
#define MUSIC_LOADPARAMS_MSG      (0x1012)


typedef struct 
{
    /*! flag that indicates if the prompt is mixing with existing audio */
    uint8                sensor_sample;
    /*! prompt index Id. */
    uint16               sensor_value;   

	/*! The app task to use to communication the audio*/
	Task				app_task ;
} SPORT_DATA_T ;

/* DSP message structure */
typedef struct
{
    uint16 id;
    uint16 a;
    uint16 b;
    uint16 c;
    uint16 d;
} DSP_REGISTER_T;

typedef struct
{
    uint16 id;
    uint16 size;
    char   buf[1];
} DSP_LONG_REGISTER_T;



static SPORT_DATA_T            *sport_data = NULL;


/****************************************************************************
DESCRIPTION
    Plays back a voice prompt once DSP has loaded
*/

void KoovoxPromptsPluginPlayDsp(kalimba_state state)
{    

    /* if the dsp has not yet been loaded, load it and wait for a call back to this function */
    if(state == kalimba_idle)
    {        
        /* Find the DSP file to use */
        const char *kap_file = "vee_heartrate/vee_heartrate.kap";
        FILE_INDEX file_index = FileFind(FILE_ROOT,(const char *) kap_file ,strlen(kap_file));

		SetCurrentDspStatus( DSP_LOADING );

        /* Load DSP */
        if (!KalimbaLoad(file_index))
            Panic();

        PRINT(("SPORT: load kalimba suc\n"));

		SetCurrentDspStatus( DSP_LOADED_IDLE );
		
        return;
    }
    
    /* Ready to go... */
    if (!KalimbaSendMessage(KALIMBA_MSG_GO, 0, 0, 0, 0))
    {
        PRINT(("VP: DSP failed to send go to kalimba\n"));
        Panic();
    }

	MessageSend(sport_data->app_task, EVENT_SPORT_KALIMBA_LOADED, 0);

	SetCurrentDspStatus( DSP_RUNNING);

	KoovoxStartSportKalimba(sport_data->sensor_sample, sport_data->sensor_value);

	SetAudioBusy(NULL);
	SetSportStatus(TRUE);
	
}

/****************************************************************************
DESCRIPTION
    plays a number phrase using the audio plugin    
*/

void KoovoxSportCommonPluginStart (uint8 sensor_sample , uint16 sensor_value, Task app_task)
{
	Task kal_task = NULL;

    if(sport_data != NULL)
		Panic();
	    
    /* Allocate the memory */
    sport_data = (SPORT_DATA_T *) PanicUnlessMalloc(sizeof(SPORT_DATA_T));
    memset(sport_data,0,sizeof(SPORT_DATA_T));
    
    /* Set up params */
    sport_data->sensor_sample  = sensor_sample; 
    sport_data->sensor_value   = sensor_value; 
	sport_data->app_task	   = app_task;
    
	(void) MessageCancelAll( (TaskData*) &koovox_sport_common_plugin, MESSAGE_FROM_KALIMBA);
    (void) MessageCancelAll( (TaskData*) &koovox_sport_common_plugin, MESSAGE_FROM_KALIMBA_LONG);
	
	kal_task = MessageKalimbaTask((TaskData*)&(koovox_sport_common_plugin));
    SetAudioBusy((TaskData*) &(koovox_sport_common_plugin));

	KoovoxPromptsPluginPlayDsp((kal_task != &koovox_sport_common_plugin) ? kalimba_idle : kalimba_ready);			  
    
}


void KoovoxPowerOffKalimba(void)
{

	PRINT(("KoovoxPowerOffKalimba\n"));
	
	if(!sport_data)
		Panic();
	
	MessageCancelAll((TaskData*)&koovox_sport_common_plugin, MESSAGE_FROM_KALIMBA);
	MessageCancelAll((TaskData*)&koovox_sport_common_plugin, MESSAGE_FROM_KALIMBA_LONG);
	MessageKalimbaTask( NULL );
	
	KalimbaPowerOff();

	SetSportStatus(FALSE);	

	SetCurrentDspStatus( DSP_NOT_LOADED );
	
	/* Tidy up */
	free(sport_data);
	sport_data = NULL;
	SetAudioBusy(NULL) ;

}

void KoovoxMessageFromKalimbaLong(Message message)
{
	const uint16 *rcv_msg = (const uint16*) message;

	switch(rcv_msg[0])
	{
		case (HB_ORIGIN_VALUE_FROM_DSP):
		case (ACC_ORIGIN_VALUE_FROM_DSP):
		{
			uint16 i;
			DSP_LONG_REGISTER_T* message = (DSP_LONG_REGISTER_T*)PanicUnlessMalloc(sizeof(DSP_LONG_REGISTER_T) + rcv_msg[1]);
			PRINT(("message from dsp\n"));
			message->id   = rcv_msg[0];
			message->size = rcv_msg[1];
			for(i=0;i<rcv_msg[1];i++)
				message->buf[i] = rcv_msg[i+2];
			MessageSend(sport_data->app_task, EVENT_SPORT_DSP_LONG_MSG, message);
		}
		break;

		default:
			break;
	}
}


void KoovoxMessageFromKalimba(Message message )
{
	const DSP_REGISTER_T *m = (const DSP_REGISTER_T *) message;
	
	 switch ( m->id )
	 {
		 case MUSIC_READY_MSG:
		 {
			 PRINT(("SPORT: KalMsg MUSIC_READY_MSG\n"));
			 KoovoxPromptsPluginPlayDsp(kalimba_loaded);
		 }
		 break;
		 
		 /* send the heart rate msg to app task */
		 case (HEARTRATE_VALUE_FROM_DSP):	
		 /* send the heart rate i2c test result to app task */
		 case (HB_I2C_TEST_RESULT_FROM_DSP):	
		 /* send the accelerate i2c test result to app task */
		 case (ACC_I2C_TEST_RESULT_FROM_DSP):
		 case (SPORT_DATA_FROM_DSP):
		 case (NECK_PROMPT_EVENT_FROM_DSP):
		 case (SEAT_PROMPT_EVENT_FROM_DSP):
		 case (DRIVER_PROMPT_EVENT_FROM_DSP):
		 {
			 uint8* msg = PanicUnlessMalloc(sizeof(DSP_REGISTER_T));
			 memcpy(msg, m, sizeof(DSP_REGISTER_T));
			 PRINT(("message from dsp\n"));
			 MessageSend(sport_data->app_task, EVENT_SPORT_DSP_SHORT_MSG, msg);
		 }
		 break;
		 
		 default:
		 	break;
	 }

}



