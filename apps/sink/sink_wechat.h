/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2010-2014
Part of ADK 3.5

FILE NAME
    sink_wechat.h        

DESCRIPTION
    Header file for interface with Generic Application Interface Architecture
    library

NOTES

*/
#ifndef _SINK_WECHAT_H_
#define _SINK_WECHAT_H_

#include <wechat.h>
#include "sink_private.h"

#ifdef DEBUG_WECHAT
#define WECHAT_DEBUG(x) DEBUG(x)
#else
#define WECHAT_DEBUG(x) 
#endif


#define WECHAT_CONFIGURATION_LENGTH_POWER (28)
#define WECHAT_CONFIGURATION_LENGTH_HFP (24)
#define WECHAT_CONFIGURATION_LENGTH_RSSI (14)







/*************************************************************************
NAME
    handleWechatMessage
    
DESCRIPTION
    Handle messages passed up from the Wechat library
*/
void handleWechatMessage(Task task, MessageId id, Message message);



/*************************************************************************
NAME
    wechatDisconnect
    
DESCRIPTION
    Disconnect from WECHAT client
*/
void wechatDisconnect(void);





#endif /*_SINK_WECHAT_H_*/
