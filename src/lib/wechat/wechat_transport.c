/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#include "wechat.h"
#include "wechat_private.h"
#include "wechat_transport.h"
#include "wechat_transport_common.h"
#include <stream.h>
#include <audio.h>
#include <sink.h>

#include "wechat_transport_rfcomm.h"


/*! @brief Attempt to find a free transport slot
 */
wechat_transport *wechatTransportFindFree(void)
{
    uint16 idx;
    
    for (idx = 0; idx < wechat->transport_count; ++idx)
    {
        if (wechat->transport[idx].type == wechat_transport_none)
            return &wechat->transport[idx];
    }
    
    return NULL;
}

#if defined WECHAT_TRANSPORT_RFCOMM || defined WECHAT_TRANSPORT_SPP
/*! @brief Attempt to find the tranport associated with an RFCOMM channel
 */
wechat_transport *wechatTransportFromRfcommChannel(uint16 channel)
{
    uint16 idx;
    
    for (idx = 0; idx < wechat->transport_count; ++idx)
    {
        if (((wechat->transport[idx].type == wechat_transport_rfcomm) || (wechat->transport[idx].type == wechat_transport_spp))
            && (wechat->transport[idx].state.spp.rfcomm_channel == channel))
            return &wechat->transport[idx];
    }
    return NULL;
}
#endif /* def WECHAT_TRANSPORT_SPP */

/*! @brief Attempt to find the tranport associated with a sink
 */
wechat_transport *wechatTransportFromSink(Sink sink)
{
    uint16 idx;
    
    for (idx = 0; idx < wechat->transport_count; ++idx)
        if (wechatTransportGetSink(&wechat->transport[idx]) == sink)
            return &wechat->transport[idx];
    
    return NULL;
}


/*! @brief Attempt to disconnect Wechat over a given transport.
 */
void wechatTransportDisconnectReq(wechat_transport *transport)
{
    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
        case wechat_transport_rfcomm:
            wechatTransportRfcommDisconnectReq(transport);
            break;
#endif
        default:
            WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
            WECHAT_PANIC();
            break;
    }
}

/*! @brief Clear down state of given transport.
 */
void wechatTransportDropState(wechat_transport *transport)
{
    WECHAT_TRANS_DEBUG(("wechat: drop state t=%04X %d o=%04X\n",
        (uint16) transport, transport->type, (uint16) wechat->outstanding_request));

    if (wechat->outstanding_request == transport)
        wechat->outstanding_request = NULL;

    if (wechat->pfs_state != PFS_NONE)
    {
        WECHAT_TRANS_DEBUG(("wechat: drop pfs %d s=%lu r=%lu\n", 
            wechat->pfs_state, wechat->pfs_sequence, wechat->pfs_raw_size));

        wechat->pfs_sequence = 0;
        wechat->pfs_raw_size = 0;
        SinkClose(wechat->pfs.sink);
        wechat->pfs_state = PFS_NONE;
    }

    if (IsAudioBusy() == &wechat->task_data)
        SetAudioBusy(NULL);
 
    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        wechatTransportRfcommDropState(transport);
        break;
#endif
    default:
        WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
        WECHAT_PANIC();
        break;
    }    
}

/*! @brief Start Wechat as a server on a given transport.
 */
void wechatTransportStartService(wechat_transport_type transport_type)
{
    switch (transport_type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        wechatTransportRfcommStartService();
        break;
#endif
    default:
        WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport_type));
        WECHAT_PANIC();
        break;
    }

}


/*! @brief Get the stream sink for a given transport.
 */
Sink wechatTransportGetSink(wechat_transport *transport)
{
    switch (transport->type)
    {            
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        return wechatTransportRfcommGetSink(transport);
        break;
#endif
    default:
        WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
        WECHAT_PANIC();
        break;
    }

    return NULL;
}

/*! @brief Pass incoming message for handling by a given transport.
 */
bool wechatTransportHandleMessage(Task task, MessageId id, Message message)
{
#ifdef WECHAT_TRANSPORT_RFCOMM
    if (wechatTransportRfcommHandleMessage(task, id, message))
        return TRUE;
#endif

    return FALSE;
}
