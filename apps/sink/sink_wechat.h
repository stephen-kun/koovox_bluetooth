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

#define WECHAT_API_MINOR_VERSION (2)

#define WECHAT_CONFIGURATION_LENGTH_POWER (28)
#define WECHAT_CONFIGURATION_LENGTH_HFP (24)
#define WECHAT_CONFIGURATION_LENGTH_RSSI (14)

#define WECHAT_TONE_BUFFER_SIZE (94)
#define WECHAT_TONE_MAX_LENGTH ((WECHAT_TONE_BUFFER_SIZE - 4) / 2)

#define WECHAT_ILLEGAL_PARTITION (15)
#define WECHAT_DFU_REQUEST_TIMEOUT (30)

typedef struct
{
    unsigned word:8;
    unsigned posn:4;
    unsigned size:4;
} wechat_feature_map_t;


typedef struct
{
    unsigned fixed:1;
    unsigned size:15;
} wechat_config_entry_size_t;

/*For complete information about the WECHAT commands, refer to the document WECHATHeadsetCommandReference*/

#define NUM_WORDS_WECHAT_CMD_HDR 2 /*No. of words in WECHAT Command header */

#define NUM_WORDS_WECHAT_CMD_PER_EQ_PARAM 4 /*No. of words for each user EQ parameter in WECHAT command*/

#define CHANGE_NUMBER_OF_ACTIVE_BANKS   0xFFFF
#define CHANGE_NUMBER_OF_BANDS          0xF0FF
#define CHANGE_BANK_MASTER_GAIN         0xF0FE
#define CHANGE_BAND_PARAMETER           0xF000
#define USER_EQ_BANK_INDEX              1

typedef enum
{
    PARAM_HI_OFFSET,
    PARAM_LO_OFFSET,
    VALUE_HI_OFFSET,
    VALUE_LO_OFFSET    
}wechat_cmd_payload_offset;


/*************************************************************************
NAME
    wechatReportPioChange
    
DESCRIPTION
    Relay any registered PIO Change events to the Wechat client
    We handle the PIO-like WECHAT_EVENT_CHARGER_CONNECTION here too
*/
void wechatReportPioChange(uint32 pio_state);


/*************************************************************************
NAME
    wechatReportEvent
    
DESCRIPTION
    Relay any significant application events to the Wechat client
*/
void wechatReportEvent(uint16 id);


/*************************************************************************
NAME
    wechatReportUserEvent
    
DESCRIPTION
    Relay any user-generated events to the Wechat client
*/
void wechatReportUserEvent(uint16 id);

        
/*************************************************************************
NAME
    wechatReportSpeechRecResult
    
DESCRIPTION
    Relay a speech recognition result to the Wechat client
*/
void wechatReportSpeechRecResult(uint16 id);


/*************************************************************************
NAME
    handleWechatMessage
    
DESCRIPTION
    Handle messages passed up from the Wechat library
*/
void handleWechatMessage(Task task, MessageId id, Message message);


/*************************************************************************
NAME
    wechatDfuRequest
    
DESCRIPTION
    Request Device Firmware Upgrade from the WECHAT host
*/
void wechatDfuRequest(void);


/*************************************************************************
NAME
    handleDfuSqifStatus
    
DESCRIPTION
    Handle MESSAGE_DFU_SQIF_STATUS from the loader
*/
void handleDfuSqifStatus(MessageDFUFromSQifStatus *message);


/*************************************************************************
NAME
    wechatDisconnect
    
DESCRIPTION
    Disconnect from WECHAT client
*/
void wechatDisconnect(void);


/*************************************************************************
NAME    
    wechat_send_response
    
DESCRIPTION
    Build and Send a Wechat acknowledgement packet
   
*/ 
void wechat_send_response(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload);


/*************************************************************************
NAME    
    wechat_send_response_16
    
DESCRIPTION
    Build and Send a Wechat acknowledgement packet from a uint16[] payload
   
*/ 
void wechat_send_response_16(uint16 command_id, uint16 status,
                          uint16 payload_length, uint16 *payload);


#endif /*_SINK_WECHAT_H_*/
