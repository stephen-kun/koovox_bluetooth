/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_core.h

*/

#ifdef ENABLE_KOOVOX


#ifndef __SINK_KOOVOX_CORE_H
#define __SINK_KOOVOX_CORE_H

#include <csrtypes.h>
#include <source.h>
#include "sink_private.h"


#ifdef DEBUG_KOOVOX_CORE
#define KOOVOX_CORE_DEBUG(x)	DEBUG(x)
#else
#define KOOVOX_CORE_DEBUG(x)
#endif

#ifdef ENABLE_LOG
#define KOOVOX_CORE_LOG(x)	{KoovoxLog x;}
#else
#define KOOVOX_CORE_LOG(x)
#endif


void KoovoxLog(const char* value_data);
void KoovoxDisableAutomaticSpeechRecognition(void);
void KoovoxEnableAutomaticSpeechRecognition(void);
void KoovoxConfirmAsrNumber(void);
void KoovoxRecordControl(void);
void KoovoxMuteActiveCall(void);
void KoovoxPresentIncomingCall(void);
void KoovoxGetBluetoothAdrress(uint8* addr);
void KoovoxResultI2cTest(uint8* data, uint8 size_data);
void KoovoxStoreLastCmd(uint8 cmd, uint8 obj);
void KoovoxPromptPresentScenceVoice(void);

void KoovoxUsrEventHandle(MessageId id, Message message);

#endif /* __SINK_KOOVOX_CORE_H */

#endif

