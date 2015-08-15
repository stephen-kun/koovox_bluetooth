/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2010-2014
Part of ADK 3.5

FILE NAME
    sink_wechat.c

DESCRIPTION
    Interface with the Generic Application Interface Architecture library

NOTES

*/

#ifdef ENABLE_WECHAT
#include "sink_wechat.h"
#include "koovox_wechat_handle.h"
#include <kalimba.h>



/*  Wechat-global data stored in app-allocated structure */
#define wechat_data theSink.rundata->wechat_data




/*************************************************************************
NAME
    handleWechatMessage
    
DESCRIPTION
    Handle messages passed up from the Wechat library
*/
void handleWechatMessage(Task task, MessageId id, Message message) 
{
    switch (id)
    {
    case WECHAT_INIT_CFM:
        {
            WECHAT_INIT_CFM_T *m = (WECHAT_INIT_CFM_T *) message;    
                
            WECHAT_DEBUG(("G: _INIT_CFM: %d\n", m->success));

            if (m->success)
            {
                WechatStartService(wechat_transport_rfcomm);
            }
        }
        break;
        
    case WECHAT_CONNECT_IND:
        {
            WECHAT_CONNECT_IND_T *m = (WECHAT_CONNECT_IND_T *) message;
            WECHAT_DEBUG(("G: _CONNECT_IND: %04X s=%u e=%u\n", 
                        (uint16) m->transport, 
                        m->success, 
                        theSink.features.WechatEnableSession));
            
            if (m->success)
            {
                wechat_data.wechat_transport = m->transport;
                WechatSetSessionEnable(m->transport, theSink.features.WechatEnableSession);

				koovox_wechat_connect(TYPE_RFCOMM, NULL, m->transport);
            }			
        }
        break;
        
    case WECHAT_DISCONNECT_IND:
        {
            WECHAT_DISCONNECT_IND_T *m = (WECHAT_DISCONNECT_IND_T *) message;
            WECHAT_DEBUG(("G: _DISCONNECT_IND: %04x\n", (uint16) m->transport));
            WechatDisconnectResponse(m->transport);
			koovox_wechat_disconnect();
        }
        break;
        
    case WECHAT_DISCONNECT_CFM:
        {
#ifdef DEBUG_WECHAT
            WECHAT_DISCONNECT_CFM_T *m = (WECHAT_DISCONNECT_CFM_T *) message;
            WECHAT_DEBUG(("G: WECHAT_DISCONNECT_CFM: %04x\n", (uint16) m->transport));
#endif
        }
        break;
        
    case WECHAT_START_SERVICE_CFM:
        WECHAT_DEBUG(("G: _START_SERVICE_CFM: %d\n", ((WECHAT_START_SERVICE_CFM_T *)message)->success));
        break;
        
    case WECHAT_SEND_PACKET_CFM:
        {
            WECHAT_SEND_PACKET_CFM_T *m = (WECHAT_SEND_PACKET_CFM_T *) message;
            
            if (m->packet)
                freePanic(m->packet);
        }
        break;

	case WECHAT_MESSAGE_MORE_DATA:
		{
			WECHAT_MESSAGE_MORE_DATA_T* m = (WECHAT_MESSAGE_MORE_DATA_T*)message;	
            WECHAT_DEBUG(("WECHAT_MESSAGE_MORE_DATA:size = %d\n", m->size_value));

			/* 处理消息 */
			koovox_rcv_data_from_wechat(m->value, m->size_value);
				
		}
		break;
        
    default:
        WECHAT_DEBUG(("G: unhandled message %04x\n", id));
        break;
    }
}



/*************************************************************************
NAME
    wechatDisconnect
    
DESCRIPTION
    Disconnect from WECHAT client
*/
void wechatDisconnect(void)
{
    WECHAT_DEBUG(("G: disconnect %04X\n", (uint16) wechat_data.wechat_transport));
    
    if (wechat_data.wechat_transport != NULL)
    {
        WechatDisconnectRequest(wechat_data.wechat_transport);
    }
}



#else
static const int dummy;  /* ISO C forbids an empty source file */

#endif  /*  ifdef ENABLE_WECHAT  */
