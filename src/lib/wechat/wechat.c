/*************************************************************************
Copyright ?CSR plc 2010-2014
Part of ADK 3.5

FILE
    wechat.c
    
DESCRIPTION
    Generic Application Interface Architecture
    This library implements the WECHAT protocol for a server- or client-side
    SPP connection.
    
    The library exposes a functional downstream API and a message-based upstream API.
*/
#include <connection.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vm.h>
#include <boot.h>
#include <ps.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <power.h>
#include <psu.h>
#include <partition.h>
#include <file.h>
#include <spp_common.h>
#include <audio.h>
#include <loader.h>
#include <kalimba.h>

#include "wechat_private.h"
#include "wechat_transport.h"
#include "wechat_transport_rfcomm.h"


WECHAT_T *wechat = NULL;


/*************************************************************************
NAME
    send_packet
    
DESCRIPTION
    Send a Wechat packet
*/
static void send_packet(Task task, wechat_transport *transport, uint16 packet_length, uint8 *packet)
{
    MESSAGE_PMAKE(m, WECHAT_INTERNAL_SEND_REQ_T);
        
    m->task = task;
    m->transport = transport;
    m->length = packet_length;
    m->data = packet;
    
    MessageSend(&wechat->task_data, WECHAT_INTERNAL_SEND_REQ, m);
}


           

/*************************************************************************
NAME
    register_custom_sdp
    
DESCRIPTION
    Register a custom SDP record retrieved from PS
    
    The first word in the PS data is the length after unpacking.  This
    allows us to retrieve an odd number of bytes and allows some sanity
    checking.
*/
static void register_custom_sdp(uint16 pskey)
{
    uint16 ps_length;
    
    ps_length = PsFullRetrieve(pskey, NULL, 0);
    WECHAT_DEBUG(("wechat: m %04x sdp %d\n", pskey, ps_length));
    
    if (ps_length > 1)
    {
        uint16 sr_length = 2 * (ps_length - 1);
        uint16 *sr = malloc(sr_length);
                
        if (sr && PsFullRetrieve(pskey, sr, ps_length) && (sr_length - sr[0] < 2))
        {
        /*  Unpack into uint8s, preserving overlapping word  */
            uint16 idx;
            uint16 tmp = sr[1];

            sr_length = sr[0];
            for (idx = ps_length; idx > 1; )
            {
                --idx;
                sr[2 * idx - 1] = sr[idx] & 0xFF;
                sr[2 * idx - 2] = sr[idx] >> 8;
            }
            
            sr[0] = tmp >> 8;
            
            wechat->custom_sdp = TRUE;
            ConnectionRegisterServiceRecord(&wechat->task_data, sr_length, (uint8 *) sr);
        /*  NOTE: firmware will free the slot  */
        }
        
        else
        {
            free(sr);
            WECHAT_DEBUG(("wechat: bad sr\n"));
        }
    }
}


/*************************************************************************
NAME
    wechat_init
    
DESCRIPTION
    Initialise the library
*/
static void wechat_init(void)
{
    MESSAGE_PMAKE(status, WECHAT_INIT_CFM_T);

    /*  Default API minor version (may be overridden by WechatSetApiMinorVersion())  */
    wechat->api_minor = WECHAT_API_VERSION_MINOR;
             
    /*  So we can use AUDIO_BUSY interlocking  */
    AudioLibraryInit();

    /*  Perform custom SDP registration */
    register_custom_sdp(PSKEY_MOD_MANUF0);
            
    status->success = TRUE;
    MessageSend(wechat->app_task, WECHAT_INIT_CFM, status);
}


/*************************************************************************
NAME
    message_handler
    
DESCRIPTION
    Handles internal messages and anything from the underlying SPP
*/
static void message_handler(Task task, MessageId id, Message message)
{
/*  If SDP registration confirmation is caused by a custom record,
    don't bother the transport handlers with it
*/
    if ((id == CL_SDP_REGISTER_CFM) && wechat->custom_sdp)
    {
        WECHAT_DEBUG(("wechat: CL_SDP_REGISTER_CFM (C): %d\n", 
                    ((CL_SDP_REGISTER_CFM_T *) message)->status));
        
        wechat->custom_sdp = FALSE;
        return;
    }
	
    /* see if a transport can handle this message */
    /* TODO handle multipoint case, loop through all transports? */
    if (wechatTransportHandleMessage(task, id, message))
        return;

    switch (id)
    {

    case WECHAT_SEND_PACKET_CFM:
        {
            WECHAT_SEND_PACKET_CFM_T *m = (WECHAT_SEND_PACKET_CFM_T *) message;
            PRINT(("wechat: WECHAT_SEND_PACKET_CFM: s=%d\n", VmGetAvailableAllocations()));
            free(m->packet);
        }
        break;
        

    case WECHAT_INTERNAL_INIT:
        wechat_init();
        break;
        
        
    case WECHAT_INTERNAL_SEND_REQ:
        {
            WECHAT_INTERNAL_SEND_REQ_T *m = (WECHAT_INTERNAL_SEND_REQ_T *) message;
            wechatTransportRfcommSendPacket(m->task, m->transport, m->length, m->data);
        }
        break;             

    case WECHAT_INTERNAL_DISCONNECT_REQ:
        {
            WECHAT_INTERNAL_DISCONNECT_REQ_T *m = (WECHAT_INTERNAL_DISCONNECT_REQ_T *) message;
            wechatTransportDisconnectReq(m->transport);
        }
        break;

    case CL_SM_ENCRYPTION_CHANGE_IND:
        break;


    default:
        WECHAT_DEBUG(("wechat: unh 0x%04X\n", id));
        break;
    }
}


/*************************************************************************
 *                                                                       *
 *  Public interface functions                                           *
 *                                                                       *
 *************************************************************************/

/*************************************************************************
NAME
    WechatInit
    
DESCRIPTION
    Initialise the Wechat protocol handler library
*/
void WechatInit(Task task, uint16 max_connections)
{ 
    /* size of buffer required for WECHAT_T + transport data */
    uint16 buf_size = sizeof (WECHAT_T) + (max_connections - 1) * sizeof (wechat_transport);   
    
    WECHAT_DEBUG(("wechat: WechatInit n=%d s=%d\n", max_connections, buf_size));
    
    if ((wechat == NULL) && 
    (max_connections > 0) && 
    (max_connections <= MAX_SUPPORTED_WECHAT_TRANSPORTS) &&
    (wechat = PanicUnlessMalloc(buf_size)))
    {
        /* initialise message handler */
        memset(wechat, 0, buf_size);
        wechat->task_data.handler = message_handler;
        wechat->app_task = task;
        
        wechat->transport_count = max_connections;
        
        MessageSend(&wechat->task_data, WECHAT_INTERNAL_INIT, NULL);
    }
    
    else
    {
        MESSAGE_PMAKE(status, WECHAT_INIT_CFM_T);
        status->success = FALSE;
        MessageSend(task, WECHAT_INIT_CFM, status);
    }
}



/*************************************************************************
NAME
    WechatDisconnectRequest
    
DESCRIPTION
    Disconnect from host
*/
void WechatDisconnectRequest(WECHAT_TRANSPORT *transport)
{    
    if (wechat != NULL)
        wechatTransportDisconnectReq((wechat_transport *) transport);
}


/*************************************************************************
NAME
    WechatDisconnectResponse
    
DESCRIPTION
    Indicates that the client has processed a WECHAT_DISCONNECT_IND message
*/
void WechatDisconnectResponse(WECHAT_TRANSPORT *transport)
{
    wechatTransportDropState((wechat_transport *) transport);
}


/*************************************************************************
NAME
    WechatStartService
    
DESCRIPTION
    Start a service of the given type
*/
void WechatStartService(wechat_transport_type transport_type)
{
    if (wechat != NULL)
        wechatTransportStartService(transport_type);
}


/*************************************************************************
NAME
    WechatSendPacket
    
DESCRIPTION
    Send a Wechat packet over the indicated connection
*/
void WechatSendPacket(WECHAT_TRANSPORT *transport, uint16 packet_length, uint8 *packet)
{
    if (wechat != NULL)
        send_packet(wechat->app_task, (wechat_transport *) transport, packet_length, packet);
}




/*************************************************************************
NAME
    WechatSetApiMinorVersion
    
DESCRIPTION
    Changes the API Minor Version reported by WECHAT_COMMAND_GET_API_VERSION
    Returns TRUE on success, FALSE if the value is out of range (0..15) or
    the WECHAT storage is not allocated
*/
bool WechatSetApiMinorVersion(uint8 version)
{
    if (wechat == NULL || version > WECHAT_API_VERSION_MINOR_MAX)
        return FALSE;
    
    wechat->api_minor = version;
    return TRUE;
}



/*************************************************************************
NAME
    WechatGetSessionEnable
    
DESCRIPTION
    Returns TRUE if WECHAT session is enabled for the given transport instance
*/
bool WechatGetSessionEnable(WECHAT_TRANSPORT *transport)
{
    return ((wechat_transport *) transport)->enabled;
}


/*************************************************************************
NAME
    WechatSetSessionEnable
    
DESCRIPTION
    Enables or disables WECHAT session for the given transport instance
*/
void WechatSetSessionEnable(WECHAT_TRANSPORT *transport, bool enable)
{
    ((wechat_transport *) transport)->enabled = enable;
}




