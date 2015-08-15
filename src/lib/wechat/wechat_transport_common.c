/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#include <panic.h>
#include "wechat_private.h"
#include "wechat_transport_common.h"


/*! @brief Utility function to send a WECHAT_CONNECT_IND message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void wechatTransportCommonSendWechatConnectInd(wechat_transport *transport, bool success)
{
    MESSAGE_PMAKE(gci, WECHAT_CONNECT_IND_T);
    gci->transport = (WECHAT_TRANSPORT*)transport;
    gci->success = success;
    MessageSend(wechat->app_task, WECHAT_CONNECT_IND, gci);
}

/*! @brief Utility function to send a WECHAT_DISCONNECT_IND message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 */
void wechatTransportCommonSendWechatDisconnectInd(wechat_transport *transport)
{
    MESSAGE_PMAKE(gdi, WECHAT_DISCONNECT_IND_T);
    gdi->transport = (WECHAT_TRANSPORT*)transport;
    MessageSend(wechat->app_task, WECHAT_DISCONNECT_IND, gdi);
}

/*! @brief Utility function to send a WECHAT_DISCONNECT_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 */
void wechatTransportCommonSendWechatDisconnectCfm(wechat_transport *transport)
{
    MESSAGE_PMAKE(gdc, WECHAT_DISCONNECT_CFM_T);
    gdc->transport = (WECHAT_TRANSPORT*)transport;
    MessageSend(wechat->app_task, WECHAT_DISCONNECT_CFM, gdc);
}

/*! @brief Utility function to send a GAIA_SEND_PACKET_CFM message to client task.
 *
 *  @param transport The gaia transport on which the event occurred.
 *  @param packet Pointer to the packet data that was transmitted.
 *  @param success Boolean indicating success or failure of Gaia command transmission.
 */
void wechatTransportCommonSendWechatSendPacketCfm(wechat_transport *transport, uint8 *packet, bool success)
{
    MESSAGE_PMAKE(gspc, WECHAT_SEND_PACKET_CFM_T);
    gspc->transport = (WECHAT_TRANSPORT*)transport;
    gspc->packet = packet;
    gspc->success = success;
    MessageSend(wechat->app_task, WECHAT_SEND_PACKET_CFM, gspc);
}



/*! @brief Utility function to send a WECHAT_START_SERVICE_CFM message to client task.
 *
 *  @param transport The wechat transport on which the service runs
 *  @param success Boolean indicating success or failure of the WechatStartService request
 */
void wechatTransportCommonSendWechatStartServiceCfm(wechat_transport_type transport_type, wechat_transport* transport, bool success)
{
    MESSAGE_PMAKE(gssc, WECHAT_START_SERVICE_CFM_T);
    gssc->transport_type = transport_type;
    gssc->transport = (WECHAT_TRANSPORT*)transport;
    gssc->success = success;
    MessageSend(wechat->app_task, WECHAT_START_SERVICE_CFM, gssc);
}


