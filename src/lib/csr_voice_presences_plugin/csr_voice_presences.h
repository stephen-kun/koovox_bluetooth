/****************************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2005

FILE NAME
    csr_voice_presences.h

DESCRIPTION
    
    
NOTES
   
*/

#ifndef _CSR_VOICE_PRESENCES_H_
#define _CSR_VOICE_PRESENCES_H_


#define OUTPUT_RATE_48K   48000

typedef enum
{
    kalimba_idle,
    kalimba_loaded,
    kalimba_ready
} kalimba_state;

void CsrVoicePresencesPluginInit ( uint16 no_prompts, uint16 no_languages );
void CsrVoicePresencesPluginPlayPhrase(uint16 id , uint16 language, Task codec_task , uint16 prompt_volume , AudioPluginFeatures features, Task app_task);
void CsrVoicePresencesPluginStopPhrase ( void ) ;

#endif
