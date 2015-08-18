/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_voice_presences.c
DESCRIPTION
    plugin implementation which plays audio prompts
NOTES
*/

#include <audio.h>
#include <codec.h>
#include <stdlib.h>
#include <panic.h>
#include <stream.h>
#include <print.h>
#include <stream.h> 
#include <message.h> 
#include <string.h>
#include <source.h>
#include <transform.h>
#include <i2c.h>

#include <file.h>
#include <stdio.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>

#include "audio.h"
#include "csr_voice_presences_plugin.h"
#include "csr_voice_presences.h"
#include "csr_i2s_audio_plugin.h"


#define PRESENT_MAX_SIZE (0xffff)

#define PRESENT_DSP_PORT (0x05)


typedef struct
{
    uint16                     no_prompts;
    uint16                     no_prompts_per_lang;
} voice_prompts_header;

typedef struct
{
    uint32              size;
    voice_prompts_codec decompression;
    uint16              playback_rate;
    bool                stereo;
} voice_prompt;

typedef struct 
{
    /*! The codec task to use to connect the audio*/
    Task                codec_task;
    /* */
    AudioPluginFeatures features;
    /*! VP source */
    Source              source;
    /*! Decompression to use */
    voice_prompts_codec decompression;
    /*! Playback rate */
    uint16              playback_rate;
    /*! The language */
    uint16              language;
    /*! The volume at which to play the tone */
    int16               prompt_volume;
    /*! stereo or mono */
    bool                stereo;
    /*! flag that indicates if the prompt is mixing with existing audio */
    bool                mixing;
    /*! prompt index Id. */
    uint16              prompt_id;   
    /*! If this is a tone, pointer to the ringtone_note */
    ringtone_note       *tone;   
	/*! The app task to use to communication the audio*/
	Task				app_task ;
	
} koovox_phrase_data_T ;

/* Decoder type to send to DSP app */
typedef enum
{
    PHRASE_NO_DECODER = 0,
    PHRASE_SBC_DECODER = 1,
    PHRASE_MP3_DECODER = 2,
    PHRASE_AAC_DECODER = 3
} PHRASE_DECODER_T;


static voice_prompts_header     present_header;
static koovox_phrase_data_T    *koovox_phrase_data = NULL;

#define SIZE_PROMPT_DATA   (12)

/****************************************************************************
*functions
*/
static Source csrVoicePresencesGetPrompt(voice_prompt* prompt, koovox_phrase_data_T * pData);
static void CsrVoicePresencesPluginStopPhraseMixable ( void ) ;
static void CsrVoicePresencesPluginPlayDigit(void) ;

/****************************************************************************
DESCRIPTION
    Plays back a voice prompt once DSP has loaded
*/
static Source csrVoicePresencesGetPrompt(voice_prompt* prompt, koovox_phrase_data_T * pData)
{
    const uint8* rx_array;
    Source lSource = NULL;
    uint16 index;

    char file_name[17];

	PRINT(("***check_the_source***\n"));

    if(!pData)
        return NULL;

    /* Must be a prompt, work out the index of the prompt */
    index = pData->prompt_id;
    
    /* Adjust for language */
    index += (pData->language * present_header.no_prompts_per_lang);

	PRINT(("***check prompt id***\n"));
	
    /* Sanity checking */
    if(index >= present_header.no_prompts || !prompt )
        return NULL;
    
    PRINT(("PRESENT: Play prompt %d of %d\n", index+1, present_header.no_prompts));
    
    /* Get the header file name */
    sprintf(file_name, "headers/%d.idx", index);
    lSource = StreamFileSource(FileFind(FILE_ROOT, file_name, strlen(file_name)));

    /* Check source created successfully */
    if(SourceSize(lSource) < SIZE_PROMPT_DATA)
    {
        /* Finished with header source, close it */
        SourceClose(lSource);
        return NULL;
    }
    
    /* Map in header */
    rx_array = SourceMap(lSource);
    
    /* Pack data into result */
    /*    rx_array[0] not used*/
    /*    rx_array[1] index, not used */
    prompt->stereo        = rx_array[4];
    prompt->size          = ((uint32)rx_array[5] << 24) | ((uint32)rx_array[6] << 16) | ((uint16)rx_array[7] << 8) | (rx_array[8]);
    prompt->decompression = rx_array[9];
    prompt->playback_rate = ((uint16)rx_array[10] << 8) | (rx_array[11]);   
    
    /* The size of the prompt must be limited to 16 bits for I2C and SPI as the firmware traps only support a 16 bit size */
    if (prompt->size > PRESENT_MAX_SIZE)
    {
        prompt->size = PRESENT_MAX_SIZE;
        PRINT(("Prompt size adjusted to 16 bit maximum\n"));
    }
    
    /* Get the prompt file name */
    sprintf(file_name, "prompts/%d.prm", index);
    
    PRINT(("File Prompt: %s dec %X rate 0x%x stereo %u size 0x%lx \n", file_name, prompt->decompression,prompt->playback_rate,prompt->stereo,prompt->size));
    
    /* Finished with header source, close it */
    if(!SourceClose(lSource))
        Panic();
    
    return StreamFileSource(FileFind(FILE_ROOT, file_name, strlen(file_name)));
}


/****************************************************************************
DESCRIPTION
    plays One digital number using the audio plugin    
*/

static void CsrVoicePresencesPluginPlayDigit(void) 
{
    Source lSource ;
    voice_prompt prompt;
    
    /* Get the prompt data*/
    lSource = csrVoicePresencesGetPrompt(&prompt, koovox_phrase_data);
    if(!lSource) Panic();
    
    SetAudioBusy((TaskData*) &csr_voice_presences_plugin);

    /* Stash the source */
    koovox_phrase_data->source = lSource;
    koovox_phrase_data->decompression = prompt.decompression;
    koovox_phrase_data->stereo = prompt.stereo;
    koovox_phrase_data->playback_rate =  (prompt.playback_rate ? prompt.playback_rate : 8000);
    koovox_phrase_data->mixing = FALSE;    /* overridden later if this prompt is mixed */
    
    SetVpPlaying(TRUE);

	PRINT(("==decompression:%d===\n", prompt.decompression));

    /* Connect the stream to the DAC */
    switch(prompt.decompression)
    {
        case voice_prompts_codec_ima_adpcm:
        case voice_prompts_codec_none:     
        case voice_prompts_codec_tone:  
        {
            Sink lSink = NULL;
            Task taskdata = NULL;
            
            /* if DSP is already running, the voice prompt can be mixed with the dsp
               audio via the kalimba mixing port (3), either the CVC plugin or
               the music plugin will have already determined the output source and connected the 
               appropriate ports to the hardware, the volume will already have been set, it is not
               necessary to do it again */
            if(GetCurrentDspStatus())
            {            
                /* Configure tone or prompt playback */    
                if(prompt.decompression == voice_prompts_codec_tone)
                {
                    PRINT(("PRESENT: play tone\n"));
                    KalimbaSendMessage(MESSAGE_SET_TONE_RATE_MESSAGE_ID, koovox_phrase_data->playback_rate , 0/*Mono Bit 0 =0, TONE BIT 1 = 0*/, 0, 0); 
                }                    
                else
                {
                    PRINT(("PRESENT: play adpcm\n"));
                    KalimbaSendMessage(MESSAGE_SET_TONE_RATE_MESSAGE_ID, koovox_phrase_data->playback_rate , /*PROMPT_MONO*/PROMPT_ISPROMPT, 0, 0);                                  
                }
                /* stream voice prompt data to the DSP tone mixing port */                
                koovox_phrase_data->mixing = TRUE;
                lSink = StreamKalimbaSink(PRESENT_DSP_PORT);
                PRINT(("PRESENT: play dsp mix lSink = %x lSource = %x\n",(uint16)lSink,(uint16)lSource));
                SinkConfigure(lSink, STREAM_CODEC_OUTPUT_RATE, koovox_phrase_data->playback_rate);
                
                /* Get messages when source has finished */
                taskdata = MessageSinkTask( lSink , (TaskData*) &csr_voice_presences_plugin);

                /* connect the kalimba port to the audio_prompt */
                if(prompt.decompression == voice_prompts_codec_ima_adpcm)
                {   
                    PanicFalse(TransformStart(TransformAdpcmDecode(lSource, lSink)));
                }
                else
                {                      
                    PanicFalse(StreamConnect(lSource, lSink)); 
                }   
                
            }
			else
			{
				CsrVoicePresencesPluginStopPhrase();
			}
        }
        break;
        
        case voice_prompts_codec_pcm:        
        {    
            Sink lSink = NULL;
            Task taskdata = NULL;
            
            /* determine whether the prompt is being played as a mixed with audio via the dsp, if the dsp is being
               used for audio mixing there is no need to set the volume as this will already have been performed 
               plugin used to load the dsp for audio processing */            
            if(GetCurrentDspStatus())
            {
                /* store that this PCM prompt is mixing */
                koovox_phrase_data->mixing = TRUE;
                
                /* stream voice prompt data to the DSP tone mixing port */                
                lSink = StreamKalimbaSink(PRESENT_DSP_PORT);
                PRINT(("PRESENT: play dsp mix lSink = %x lSource = %x\n",(uint16)lSink,(uint16)lSource));
                SinkConfigure(lSink, STREAM_CODEC_OUTPUT_RATE, koovox_phrase_data->playback_rate);                               
                
                /* Get messages when source has finished */
                taskdata = MessageSinkTask( lSink , (TaskData*) &csr_voice_presences_plugin);

				PRINT(("PRESENT: sink task now %x was %x.\n",(uint16)&csr_voice_presences_plugin,(uint16)taskdata));
                /* Configure PCM prompt playback */    
                KalimbaSendMessage(MESSAGE_SET_TONE_RATE_MESSAGE_ID, koovox_phrase_data->playback_rate , (koovox_phrase_data->stereo?PROMPT_STEREO:0)|PROMPT_ISPROMPT, 0, 0);        
                
                /* Connect source to PCM */
                PanicFalse(StreamConnect(lSource, lSink));
            }
        }
        break;

        default:
            PRINT(("PRESENT: Codec Invalid\n"));
            Panic();
        break;
    }

}

/****************************************************************************
DESCRIPTION
    plays a number phrase using the audio plugin    
*/

void CsrVoicePresencesPluginPlayPhrase (uint16 id , uint16 language, Task codec_task , uint16 prompt_volume , AudioPluginFeatures features, Task app_task)
{
    if(koovox_phrase_data != NULL)
        Panic();
    
    PRINT(("PRESENT: Play Phrase:\n"));
    
    /* Allocate the memory */
    koovox_phrase_data = (koovox_phrase_data_T *) PanicUnlessMalloc(sizeof(koovox_phrase_data_T));
    memset(koovox_phrase_data,0,sizeof(koovox_phrase_data_T));
    
    /* Set up params */
    koovox_phrase_data->language      = language;
    koovox_phrase_data->codec_task    = codec_task;
    koovox_phrase_data->prompt_volume = prompt_volume;
    koovox_phrase_data->features      = features;
    koovox_phrase_data->prompt_id     = id;
    koovox_phrase_data->mixing        = FALSE; /* currently unknown so set to false */
    koovox_phrase_data->tone          = NULL;  /* not a tone */
	koovox_phrase_data->app_task	  = app_task;
    
    MessageCancelAll((TaskData*) &csr_voice_presences_plugin, MESSAGE_STREAM_DISCONNECT );
    MessageCancelAll((TaskData*) &csr_voice_presences_plugin, MESSAGE_FROM_KALIMBA);
    
    CsrVoicePresencesPluginPlayDigit(); 
    
    SetVpPlaying(TRUE);
}


/****************************************************************************
DESCRIPTION
    Stop prompt where DSP has not been loaded by the plugin, e.g. (adpcm or pcm) 
    Prompt is either mixing in an existing DSP app or not using the DSP.
*/
static void CsrVoicePresencesPluginStopPhraseMixable ( void ) 
{
    Sink lSink=NULL;
    Task taskdata = NULL;
    
    /* Check for DSP mixing */
    if(GetCurrentDspStatus())
    {
        lSink = StreamKalimbaSink(PRESENT_DSP_PORT);
        /* reset the volume levels of the dsp plugin */
        if(GetAudioPlugin())           	    
            MessageSend( GetAudioPlugin(), AUDIO_PLUGIN_RESET_VOLUME_MSG, 0 ) ;
    }
    else    /* Must be ADPCM not mixing */
    {
        switch(koovox_phrase_data->features.audio_output_type)
        {
            /* is the I2S required? */
            case OUTPUT_INTERFACE_TYPE_I2S:
            {               
                CsrI2SAudioOutputDisconnect( koovox_phrase_data->features.stereo);  
            }
            break;
            
            /* spdif output? */
            case OUTPUT_INTERFACE_TYPE_SPDIF:
            {
                Sink rSink = NULL;
                                
                /* obtain source to SPDIF hardware and disconnect it */
                lSink = StreamAudioSink(AUDIO_HARDWARE_SPDIF, AUDIO_INSTANCE_0, SPDIF_CHANNEL_A );
                rSink = StreamAudioSink(AUDIO_HARDWARE_SPDIF, AUDIO_INSTANCE_0, SPDIF_CHANNEL_B );
                StreamDisconnect(0, lSink);
                StreamDisconnect(0, rSink);
                SinkClose(lSink);
                SinkClose(rSink);
            }
            break;
            
            /* use built in codec */
            default:
            {
                lSink = StreamAudioSink(AUDIO_HARDWARE_CODEC,AUDIO_INSTANCE_0, (koovox_phrase_data->features.stereo ? AUDIO_CHANNEL_A_AND_B : AUDIO_CHANNEL_A));
                /* Disconnect PCM source/sink */
                StreamDisconnect(StreamSourceFromSink(lSink), lSink); 
            }
        }
    }    
    /* close sink and cancel any messages if valid */
    if(lSink)
    {
        /* Cancel all the messages relating to VP that have been sent */
        taskdata = MessageSinkTask(lSink, NULL);
        SinkClose(lSink);
    }
    PRINT(("PRESENT: SinkTask now NULL was %x\n",(uint16)taskdata));
    MessageCancelAll((TaskData*) &csr_voice_presences_plugin, MESSAGE_STREAM_DISCONNECT);
}

/****************************************************************************
DESCRIPTION
    Initialise indexing.
*/

void CsrVoicePresencesPluginInit ( uint16 no_prompts, uint16 no_languages )
{
    PRINT(("PRESENT: Init %d prompts %d languages \n", no_prompts, no_languages));
    present_header.no_prompts = no_prompts;
    present_header.no_prompts_per_lang = no_prompts / no_languages;
}


/****************************************************************************
DESCRIPTION
    Stop a prompt from currently playing
*/
void CsrVoicePresencesPluginStopPhrase ( void ) 
{
    if(!koovox_phrase_data)
        Panic();
            
    PRINT(("PRESENT: Terminated\n"));
                
    if(koovox_phrase_data->mixing)
    {
        /* If DSP already loaded and the prompt was mixed */         
        CsrVoicePresencesPluginStopPhraseMixable();
    }
	
    /* Make sure prompt source is disposed */
    if(SourceIsValid(koovox_phrase_data->source))
        StreamDisconnect(koovox_phrase_data->source, NULL);
    
    /* Tidy up */
    free(koovox_phrase_data);
    koovox_phrase_data = NULL;
    SetAudioBusy(NULL) ;
    SetVpPlaying(FALSE);
    
}


