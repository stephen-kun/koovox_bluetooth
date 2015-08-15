/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#ifndef _WECHAT_PRIVATE_H_
#define _WECHAT_PRIVATE_H_

#define MESSAGE_PMAKE(NAME,TYPE) TYPE * const NAME = PanicUnlessNew(TYPE)


#include "wechat.h"

/* configure which transports to include in the library
 * start with RFCOMM defined unless removed by VARIANTS */
#ifdef WECHAT_TRANSPORT_NO_RFCOMM
#undef WECHAT_TRANSPORT_RFCOMM
#else
#define WECHAT_TRANSPORT_RFCOMM
#endif


/* sanity check; we must have at least one of the transports defined */
#if !defined(WECHAT_TRANSPORT_RFCOMM) && !defined(WECHAT_TRANSPORT_SPP) && !defined(WECHAT_TRANSPORT_MFI)
#error : no transport defined
#endif

#define MAX_SUPPORTED_WECHAT_TRANSPORTS   4           /*!< Maximum number of transports Wechat can support */
/* end transport configuration */


/* configure what debug is generated */
#ifdef DEBUG_WECHAT
#include <stdio.h>
#include <panic.h>
#define WECHAT_DEBUG(x) printf x
#define WECHAT_PANIC() Panic()
#else
#define WECHAT_DEBUG(x)
#define WECHAT_PANIC()
#endif


#ifdef DEBUG_WECHAT_TRANSPORT                         /* transport specific debug */
#include <stdio.h>
#define WECHAT_TRANS_DEBUG(x) printf x
#else
#define WECHAT_TRANS_DEBUG(x)
#endif


#define BAD_SINK ((Sink) 0xFFFF)
#define BAD_SINK_CLAIM (0xFFFF)


#define PSKEY_BDADDR (0x0001)
#define PSKEY_LOCAL_DEVICE_NAME (0x0108)
#define PSKEY_MODULE_ID (0x0259)
#define PSKEY_MODULE_DESIGN (0x025A)
#define PSKEY_MOD_MANUF0 (0x025E)
#define PSKEY_USB_VENDOR_ID (0x02BE)


/*  Internal message ids  */
typedef enum
{
    WECHAT_INTERNAL_INIT = 1,
    WECHAT_INTERNAL_MORE_DATA,
    WECHAT_INTERNAL_SEND_REQ,
    WECHAT_INTERNAL_DISCONNECT_REQ
} wechat_internal_message;

typedef struct _wechat_transport wechat_transport;
typedef struct _WECHAT_T WECHAT_T;


typedef struct
{
    wechat_transport *transport;
} WECHAT_INTERNAL_MORE_DATA_T;

    
typedef struct
{
    Task task;
    wechat_transport *transport;
    uint16 length;
    uint8 *data;
} WECHAT_INTERNAL_SEND_REQ_T;


typedef struct
{
    wechat_transport *transport;
} WECHAT_INTERNAL_DISCONNECT_REQ_T;

/*! @brief definition of internal timer message to check battery level for host on specified transport.
 */
typedef struct
{
    wechat_transport *transport;
} WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T;

/*! @brief definition of internal timer message to check RSSI on a specified transport.
 */
typedef struct
{
    wechat_transport *transport;
} WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T;


/*! @brief Wechat SPP transport state.
 */
typedef struct
{
    uint16 rfcomm_channel;          /*!< RFCOMM channel used by this transport. */
    Sink sink;                  /*!< Stream sink of this transport. */
} wechat_transport_spp_data;


/*! @brief Generic Wechat transport data structure.
 */
struct _wechat_transport
{
    wechat_transport_type type;       /*!< the transport type of this instance. */
    unsigned connected:1;           /*!< is this transport connected? */
    unsigned flags:1;               /*!< does this link require Wechat checksum? */
    unsigned enabled:1;             /*!< is this session enabled? */
    unsigned unused:13;             /*!< explicitly track unused bits in this word */
    
    wechat_transport_spp_data spp;
  
};


/*! @brief Wechat library main task and state structure. */
struct _WECHAT_T
{
    TaskData task_data;
    Task app_task;
    wechat_transport *outstanding_request;

    uint32 spp_sdp_handle;      /* not per-transport, there can be only one  */
    uint16 spp_listen_channel;  /* not per-transport, there can be only one  */

    uint8 custom_sdp;
    
    uint16 transport_count;
    wechat_transport transport[1];    /*!< storage for the wechat transports */
};

extern WECHAT_T* wechat; 


#endif /* ifdef _WECHAT_PRIVATE_H_ */
