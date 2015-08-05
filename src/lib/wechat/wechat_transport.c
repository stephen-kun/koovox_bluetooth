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


#ifdef WECHAT_TRANSPORT_RFCOMM
#include "wechat_transport_rfcomm.h"
#endif

#ifdef WECHAT_TRANSPORT_SPP
#include "wechat_transport_spp.h"
#endif

#ifdef WECHAT_TRANSPORT_MFI
#include "wechat_transport_mfi.h"
#endif

/*! @brief Initialise a Wechat transport.
 */
void wechatTransportInit(wechat_transport *transport)
{
    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
        case wechat_transport_rfcomm:
            wechatTransportRfcommInit(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_SPP
        case wechat_transport_spp:
            wechatTransportSppInit(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
        case wechat_transport_mfi:
            wechatTransportMfiInit(transport);
            break;
#endif
        default:
            WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
            WECHAT_PANIC();
            break;
    }
}


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


/*! @brief Attempt to connect Wechat to a device over a given transport.
 */
void wechatTransportConnectReq(wechat_transport *transport, bdaddr *address)
{
#if defined WECHAT_TRANSPORT_SPP || defined WECHAT_TRANSPORT_RFCOMM
    if (transport == NULL)
    {
        transport = wechatTransportFindFree();
        if (transport == NULL)
        {
        /*  No spare transports; return failure to application  */
            wechatTransportCommonSendWechatConnectCfm(NULL, FALSE);
            return;
        }

        else
        {
            transport->type = wechat_transport_spp;
        }
    }
#endif

    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_SPP
        case wechat_transport_spp:
            wechatTransportSppConnectReq(transport, address);
            break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
        case wechat_transport_mfi:
            wechatTransportMfiConnectReq(transport, address);
            break;
#endif
        default:
            WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
            WECHAT_PANIC();
            break;
    }
}

/*! @brief Respond to an incoming Wechat connection indication over a given transport.
 */
void wechatTransportConnectRes(wechat_transport *transport)
{
    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
        case wechat_transport_rfcomm:
            wechatTransportRfcommConnectRes(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_SPP
        case wechat_transport_spp:
            wechatTransportSppConnectRes(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
        case wechat_transport_mfi:
            wechatTransportMfiConnectRes(transport);
            break;
#endif
        default:
            WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
            WECHAT_PANIC();
            break;
    }

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
#ifdef WECHAT_TRANSPORT_SPP
        case wechat_transport_spp:
            wechatTransportSppDisconnectReq(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
        case wechat_transport_mfi:
            wechatTransportMfiDisconnectReq(transport);
            break;
#endif
        default:
            WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
            WECHAT_PANIC();
            break;
    }
}

/*! @brief Respond to a Wechat disconnection indication over a given transport.
 */
void wechatTransportDisconnectRes(wechat_transport *transport)
{
    switch (transport->type)
    {
#ifdef WECHAT_TRANSPORT_RFCOMM
        case wechat_transport_rfcomm:
            wechatTransportRfcommDisconnectRes(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_SPP
        case wechat_transport_spp:
            wechatTransportSppDisconnectRes(transport);
            break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
        case wechat_transport_mfi:
            wechatTransportMfiDisconnectRes(transport);
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
#ifdef WECHAT_TRANSPORT_SPP
    case wechat_transport_spp:
        wechatTransportSppDropState(transport);
        break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
    case wechat_transport_mfi:
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
#ifdef WECHAT_TRANSPORT_SPP
    case wechat_transport_spp:
        wechatTransportSppStartService();
        break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
    case wechat_transport_mfi:
        wechatTransportMfiStartService();
        break;
#endif
    default:
        WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport_type));
        WECHAT_PANIC();
        break;
    }

}

/*! @brief Transmit a Wechat packet over a given transport.
 */
void wechatTransportSendPacket(Task task, wechat_transport *transport, uint16 length, uint8 *data)
{
    switch (transport->type)
    {
    case wechat_transport_none:
    /*  Transport has been cleared down  */
        WECHAT_TRANS_DEBUG(("wechat: transport %04x gone\n", (uint16) transport));
        wechatTransportCommonSendWechatSendPacketCfm(transport, data, FALSE);
        break;
        
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        wechatTransportRfcommSendPacket(task, transport, length, data);
        break;
#endif
#ifdef WECHAT_TRANSPORT_SPP
    case wechat_transport_spp:
        wechatTransportSppSendPacket(task, transport, length, data);
        break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
    case wechat_transport_mfi:
        wechatTransportMfiSendPacket(task, transport, length, data);
        break;
#endif
    default:
        WECHAT_TRANS_DEBUG(("wechat: unknown transport %d\n", transport->type));
        WECHAT_PANIC();
        break;
    }
}


/*! @brief Get the stream source for a given transport.
 */
Source wechatTransportGetSource(wechat_transport *transport)
{
    switch (transport->type)
    {
    case wechat_transport_none:
        return NULL;
        break;
        
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        return StreamSourceFromSink(wechatTransportRfcommGetSink(transport));
        break;
#endif
        
#ifdef WECHAT_TRANSPORT_SPP
    case wechat_transport_spp:
        return StreamSourceFromSink(wechatTransportSppGetSink(transport));
        break;
#endif
        
    default:
        WECHAT_TRANS_DEBUG(("Unknown Wechat transport %d\n", transport->type));
        WECHAT_PANIC();
        break;
    }

    return NULL;
}


/*! @brief Get the stream sink for a given transport.
 */
Sink wechatTransportGetSink(wechat_transport *transport)
{
    switch (transport->type)
    {
    case wechat_transport_none:
        break;
            
#ifdef WECHAT_TRANSPORT_RFCOMM
    case wechat_transport_rfcomm:
        return wechatTransportRfcommGetSink(transport);
        break;
#endif
#ifdef WECHAT_TRANSPORT_SPP
    case wechat_transport_spp:
        return wechatTransportSppGetSink(transport);
        break;
#endif
#ifdef WECHAT_TRANSPORT_MFI
    case wechat_transport_mfi:
        return wechatTransportMfiGetSink(transport);
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
