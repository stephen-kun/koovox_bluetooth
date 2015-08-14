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
    wechat_handle_disconnect
    
DESCRIPTION
    Handle a disconnection from the Wechat library
    Cancel all notifications and clear the transport
*/
static void wechat_handle_disconnect(void)
{
    wechat_data.pio_change_mask = 0UL;
    wechat_data.notify_battery_charged = FALSE;
    wechat_data.notify_charger_connection = FALSE;
    wechat_data.notify_ui_event = FALSE;
    wechat_data.notify_speech_rec = FALSE;
    wechat_data.wechat_transport = NULL;
}


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
                WechatSetApiMinorVersion(WECHAT_API_MINOR_VERSION);
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
            }

			{

				g_wechatble_state.indication_state = TRUE;
				
				if((g_wechatble_state.indication_state) 
					&& (!g_wechatble_state.auth_state) 
					&& (!g_wechatble_state.auth_send) )
				{
					koovox_pack_wechat_auth_req();
					WechatSendPacket(m->transport, g_send_data.len, g_send_data.data);
					g_wechatble_state.auth_send = TRUE;
				}
				
			}

			
        }
        break;
        
    case WECHAT_DISCONNECT_IND:
        {
            WECHAT_DISCONNECT_IND_T *m = (WECHAT_DISCONNECT_IND_T *) message;
            WECHAT_DEBUG(("G: _DISCONNECT_IND: %04x\n", (uint16) m->transport));
            wechat_handle_disconnect();
            WechatDisconnectResponse(m->transport);
			koovox_device_wechat_disconnect();
        }
        break;
        
    case WECHAT_DISCONNECT_CFM:
        {
#ifdef DEBUG_WECHAT
            WECHAT_DISCONNECT_CFM_T *m = (WECHAT_DISCONNECT_CFM_T *) message;
            WECHAT_DEBUG(("G: WECHAT_DISCONNECT_CFM: %04x\n", (uint16) m->transport));
#endif
            wechat_handle_disconnect();
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

			{
				/* 处理消息 */
				koovox_write_from_wechat(m->value, m->size_value);
				
				/* 发送init req */
				if((g_wechatble_state.auth_state) 
					&& (!g_wechatble_state.init_state) 
					&& (!g_wechatble_state.init_send))
				{
					koovox_pack_wechat_init_req();
					WechatSendPacket(wechat_data.wechat_transport, g_send_data.len, g_send_data.data);
					g_wechatble_state.init_send = TRUE;

				}
			}
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
