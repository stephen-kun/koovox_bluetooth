/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#include "wechat.h"
#include "wechat_private.h"

#ifdef WECHAT_TRANSPORT_RFCOMM
#include <string.h>
#include <stream.h>
#include <source.h>
#include <sink.h>
#include <sppc.h>

#include "wechat_transport.h"
#include "wechat_transport_rfcomm.h"
#include "wechat_transport_common.h"


static const uint8 wechat_rfcomm_service_record[] =
{
    0x09, 0x00, 0x01,       /*  0  1  2  ServiceClassIDList(0x0001) */
    0x35,   17,             /*  3  4     DataElSeq 17 bytes */
	0x1c, 0xe5, 0xb1, 0x52, 0xed, 0x6b, 0x46, 0x09, 0xe9, 0x46, 0x78, 0x66, 0x5e, 0x9a, 0x97, 0x2c, 0xbc,
                            /*  5 .. 21  UUID WECHAT (0xe5b152ed-6b46-09e9-4678-665e9a972cbc) */
    0x09, 0x00, 0x04,       /* 22 23 24  ProtocolDescriptorList(0x0004) */
    0x35,   12,             /* 25 26     DataElSeq 12 bytes */
    0x35,    3,             /* 27 28     DataElSeq 3 bytes */
    0x19, 0x01, 0x00,       /* 29 30 31  UUID L2CAP(0x0100) */
    0x35,    5,             /* 32 33     DataElSeq 5 bytes */
    0x19, 0x00, 0x03,       /* 34 35 36  UUID RFCOMM(0x0003) */
    0x08,    0,             /* 37 38     uint8 RFCOMM channel */
#define WECHAT_RFCOMM_SR_CH_IDX (38)
    0x09, 0x00, 0x06,       /* 39 40 41  LanguageBaseAttributeIDList(0x0006) */
    0x35,    9,             /* 42 43     DataElSeq 9 bytes */
    0x09,  'e',  'n',       /* 44 45 46  Language: English */
    0x09, 0x00, 0x6A,       /* 47 48 49  Encoding: UTF-8 */
    0x09, 0x01, 0x00,       /* 50 51 52  ID base: 0x0100 */
    0x09, 0x01, 0x00,       /* 53 54 55  ServiceName 0x0100, base + 0 */
    0x25,   11,             /* 56 57     String length 11 */
    'C', 'S', 'R', ' ', 'G', 'A', 'I', 'A', 
                            /* 58 .. 65  "CSR WECHAT"  */
    0xE2, 0x84, 0xA2        /* 66 67 68  U+2122, Trade Mark sign */
};


static const rfcomm_config_params rfcomm_config = 
{
    RFCOMM_DEFAULT_PAYLOAD_SIZE,
    RFCOMM_DEFAULT_MODEM_SIGNAL,
    RFCOMM_DEFAULT_BREAK_SIGNAL,
    RFCOMM_DEFAULT_MSC_TIMEOUT
};
        

static void sdp_register_rfcomm(uint8 channel)
{
    uint8 *sr = malloc(sizeof wechat_rfcomm_service_record);
    
    if (sr == NULL)
    {
        WECHAT_DEBUG(("Out of memory (sr)\n"));
        WECHAT_PANIC();
    }
        
    else
    {
        memcpy(sr, wechat_rfcomm_service_record, sizeof wechat_rfcomm_service_record);
        sr[WECHAT_RFCOMM_SR_CH_IDX] = channel;
        wechat->spp_listen_channel = channel;
        WECHAT_TRANS_DEBUG(("wechat: ch %u\n", channel));
        ConnectionRegisterServiceRecord(&wechat->task_data, sizeof wechat_rfcomm_service_record, sr);
    /*  NOTE: firmware will free the slot  */
    }
}


/*************************************************************************
NAME
    wechatTransportRfcommDropState
    
DESCRIPTION
    Clear down transport state
*/
void wechatTransportRfcommDropState(wechat_transport *transport)
{
    transport->state.spp.sink = NULL;
    transport->connected = FALSE;
    transport->enabled = FALSE;
    transport->type = wechat_transport_none;
    
    /* No longer have a Wechat connection over this transport, ensure we reset any threshold state */
    wechatTransportCommonCleanupThresholdState(transport);
}


/*************************************************************************
NAME
    wechatTransportRfcommSendPacket
    
DESCRIPTION
    Copy the passed packet to the transport sink and flush it
    If <task> is not NULL, send a confirmation message
*/
void wechatTransportRfcommSendPacket(Task task, wechat_transport *transport, uint16 length, uint8 *data)
{
    bool status = FALSE;
    
    if (wechat)
    {
        Sink sink = wechatTransportRfcommGetSink(transport);
        
        if (SinkClaim(sink, length) == BAD_SINK_CLAIM)
        {
            WECHAT_TRANS_DEBUG(("wechat: no sink\n"));
        }

        else
        {
            uint8 *sink_data = SinkMap(sink);
            memcpy (sink_data, data, length);

            status = SinkFlush(sink, length);
        }   
    }
    
    if (task)
        wechatTransportCommonSendWechatSendPacketCfm(transport, data, status);
    
    else
        free(data);
}




/*! @brief
 */
void wechatTransportRfcommDisconnectReq(wechat_transport *transport)
{
    ConnectionRfcommDisconnectRequest(&wechat->task_data, wechatTransportRfcommGetSink(transport));
}

/*! @brief
 */
void wechatTransportRfcommStartService(void)
{
    ConnectionRfcommAllocateChannel(&wechat->task_data, SPP_DEFAULT_CHANNEL);
}

/*! @brief
 */
Sink wechatTransportRfcommGetSink(wechat_transport *transport)
{
    return transport->state.spp.sink;
}


/*! @brief
 */
bool wechatTransportRfcommHandleMessage(Task task, MessageId id, Message message)
{
    bool msg_handled = TRUE;    /* default position is we've handled the message */

    switch (id)
    {
        case WECHAT_INTERNAL_MORE_DATA:
            {
            }
            break;
            
            
        case MESSAGE_MORE_DATA:
            {

				MessageMoreData *m = (MessageMoreData *) message;
				uint16 size = SourceSize(m->source);
				const uint8* data = SourceMap(m->source);
				uint16 size_msg = sizeof(WECHAT_MESSAGE_MORE_DATA_T) + (size ? size - 1 : 0);
				WECHAT_MESSAGE_MORE_DATA_T* msg = (WECHAT_MESSAGE_MORE_DATA_T*)PanicUnlessMalloc(size_msg);
				
				msg->size_value = size;
				memmove(msg->value, data, size);
				
				MessageSend(wechat->app_task, WECHAT_MESSAGE_MORE_DATA, msg);
				
				SourceDrop(m->source, size);
			
            }
            break;
            

        case CL_RFCOMM_REGISTER_CFM:
            {
                CL_RFCOMM_REGISTER_CFM_T *m = (CL_RFCOMM_REGISTER_CFM_T *) message;
                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_REGISTER_CFM: %d = %d\n", m->server_channel, m->status));
                
                if (m->status == success)
                    sdp_register_rfcomm(m->server_channel);
                
                else
                    wechatTransportCommonSendWechatStartServiceCfm(wechat_transport_rfcomm, NULL, FALSE);
            }
            break;
        
            
        case CL_SDP_REGISTER_CFM:
            {
                CL_SDP_REGISTER_CFM_T *m = (CL_SDP_REGISTER_CFM_T *) message;
                WECHAT_TRANS_DEBUG(("wechat: CL_SDP_REGISTER_CFM: %d\n", m->status));
                
                if (m->status == sds_status_success)
                {
                    if (wechat->spp_sdp_handle == 0)
                        wechatTransportCommonSendWechatStartServiceCfm(wechat_transport_rfcomm, NULL, TRUE);
                    
                    wechat->spp_sdp_handle = m->service_handle;
                }
                
                else
                    wechatTransportCommonSendWechatStartServiceCfm(wechat_transport_rfcomm, NULL, FALSE);
            }
            break;

            
        case CL_RFCOMM_CONNECT_IND:
            {
                CL_RFCOMM_CONNECT_IND_T *m = (CL_RFCOMM_CONNECT_IND_T *) message;
                wechat_transport *transport = wechatTransportFindFree();

                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_CONNECT_IND\n"));
                
                if (transport == NULL)
                    ConnectionRfcommConnectResponse(task, FALSE, m->sink, m->server_channel, &rfcomm_config);
                
                else
                {
                    transport->type = wechat_transport_rfcomm;
                    transport->state.spp.sink = m->sink;
                    transport->state.spp.rfcomm_channel = m->server_channel;
                    ConnectionRfcommConnectResponse(task, TRUE, m->sink, m->server_channel, &rfcomm_config);
                }
            }
            break;
            
            
        case CL_RFCOMM_SERVER_CONNECT_CFM:
            {
                CL_RFCOMM_SERVER_CONNECT_CFM_T *m = (CL_RFCOMM_SERVER_CONNECT_CFM_T *) message;
                wechat_transport *transport = wechatTransportFromRfcommChannel(m->server_channel);
                
                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_SERVER_CONNECT_CFM: ch=%d sts=%d\n", 
                                  m->server_channel, m->status));
                
                if (m->status == rfcomm_connect_success)
                {
                    transport->state.spp.sink = m->sink;
                    transport->state.spp.rfcomm_channel = m->server_channel;
                    ConnectionUnregisterServiceRecord(task, wechat->spp_sdp_handle);
                    wechatTransportCommonSendWechatConnectInd(transport, TRUE);
                    transport->connected = TRUE;
                    transport->enabled = TRUE;
                }
                
                else
                    wechatTransportCommonSendWechatConnectInd(transport, FALSE);
                    
            }
            break;
            
            
        case CL_SDP_UNREGISTER_CFM:
            {
                CL_SDP_UNREGISTER_CFM_T *m = (CL_SDP_UNREGISTER_CFM_T *) message;
                WECHAT_TRANS_DEBUG(("wechat: CL_SDP_UNREGISTER_CFM: %d\n", m->status));
                if (m->status == success)
                {
                /*  Get another channel from the pool  */
                    ConnectionRfcommAllocateChannel(task, SPP_DEFAULT_CHANNEL);
                }
            }
            break;
 
            
        case CL_RFCOMM_DISCONNECT_IND:
            {
                CL_RFCOMM_DISCONNECT_IND_T *m = (CL_RFCOMM_DISCONNECT_IND_T *) message;
                wechat_transport *transport = wechatTransportFromSink(m->sink);
                
                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_DISCONNECT_IND\n"));
                
                ConnectionRfcommDisconnectResponse(m->sink);

            /*  release channel for re-use  */
                ConnectionRfcommDeallocateChannel(task, transport->state.spp.rfcomm_channel);
            }
            break;
        

        case CL_RFCOMM_DISCONNECT_CFM:
            {
                CL_RFCOMM_DISCONNECT_CFM_T *m = (CL_RFCOMM_DISCONNECT_CFM_T *) message;
                wechat_transport *transport = wechatTransportFromSink(m->sink);
                
                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_DISCONNECT_CFM\n"));
                wechatTransportDropState(transport);
                wechatTransportCommonSendWechatDisconnectCfm(transport);
            }
            break;
            
            
        case CL_RFCOMM_UNREGISTER_CFM:
            {
                CL_RFCOMM_UNREGISTER_CFM_T *m = (CL_RFCOMM_UNREGISTER_CFM_T *) message;
                WECHAT_TRANS_DEBUG(("wechat: CL_RFCOMM_UNREGISTER_CFM\n"));
                
                if (m->status == success)
                {
                    wechat_transport *transport = wechatTransportFromRfcommChannel(m->server_channel);
                    wechatTransportCommonSendWechatDisconnectInd(transport);                            
                }
            }
            break;
            
        
             /*  Things to ignore  */
        case MESSAGE_MORE_SPACE:
        case MESSAGE_SOURCE_EMPTY:
        case CL_RFCOMM_PORTNEG_IND:
        case CL_RFCOMM_CONTROL_IND:
        case CL_RFCOMM_LINE_STATUS_IND:
            break;

        default:
            {
                /* indicate we couldn't handle the message */
            /*  WECHAT_DEBUG(("wechat: rfcomm: unh 0x%04X\n", id));  */
                msg_handled = FALSE;
            }
            break;
    }

    return msg_handled;
}

#endif /* WECHAT_TRANSPORT_RFCOMM */
