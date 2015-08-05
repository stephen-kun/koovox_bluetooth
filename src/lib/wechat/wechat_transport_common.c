/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#include <panic.h>
#include "wechat_private.h"
#include "wechat_transport_common.h"

/*! @brief Utility function to send a WECHAT_CONNECT_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void wechatTransportCommonSendWechatConnectCfm(wechat_transport *transport, bool success)
{
    MESSAGE_PMAKE(gcc, WECHAT_CONNECT_CFM_T);
    gcc->transport = (WECHAT_TRANSPORT*)transport;
    gcc->success = success;
    MessageSend(wechat->app_task, WECHAT_CONNECT_CFM, gcc);
}

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

/*! @brief Utility function to send a WECHAT_SEND_PACKET_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param packet Pointer to the packet data that was transmitted.
 *  @param success Boolean indicating success or failure of Wechat command transmission.
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


/*! @brief Cleanup battery and RSSI threshold state for a given transport.
 
    Used once a Wechat transport has closed, as we no longer need to perform the
    checking or resend the timer messages.
*/
void wechatTransportCommonCleanupThresholdState(wechat_transport *transport)
{
    /* reset battery threshold state */
    transport->threshold_count_lo_battery = 0;
    transport->sent_notification_lo_battery_0 = FALSE;
    transport->sent_notification_lo_battery_1 = FALSE;
    transport->threshold_count_hi_battery = 0;
    transport->sent_notification_hi_battery_0 = FALSE;
    transport->sent_notification_hi_battery_1 = FALSE;

    /* reset RSSI threshold state */
    transport->threshold_count_lo_rssi = 0;
    transport->sent_notification_lo_rssi_0 = FALSE;
    transport->sent_notification_lo_rssi_1 = FALSE;
    transport->threshold_count_hi_rssi = 0;
    transport->sent_notification_hi_rssi_0 = FALSE;
    transport->sent_notification_hi_rssi_1 = FALSE;
}
