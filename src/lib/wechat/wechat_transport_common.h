/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#include "wechat_private.h"

#ifndef _WECHAT_TRANSPORT_COMMON_H
#define _WECHAT_TRANSPORT_COMMON_H

/*! @brief Utility function to send a WECHAT_CONNECT_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void wechatTransportCommonSendWechatConnectCfm(wechat_transport* transport, bool success);

/*! @brief Utility function to send a WECHAT_CONNECT_IND message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param success Boolean indicating success (TRUE) or failure (FALSE) of connection attempt.
 */
void wechatTransportCommonSendWechatConnectInd(wechat_transport* transport, bool success);

/*! @brief Utility function to send a WECHAT_DISCONNECT_IND message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 */
void wechatTransportCommonSendWechatDisconnectInd(wechat_transport* transport);

/*! @brief Utility function to send a WECHAT_DISCONNECT_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 */
void wechatTransportCommonSendWechatDisconnectCfm(wechat_transport* transport);

/*! @brief Utility function to send a WECHAT_SEND_PACKET_CFM message to client task.
 *
 *  @param transport The wechat transport on which the event occurred.
 *  @param packet Pointer to the packet data that was transmitted.
 *  @param success Boolean indicating success or failure of Wechat command transmission.
 */
void wechatTransportCommonSendWechatSendPacketCfm(wechat_transport *transport, uint8 *packet, bool success);

/*! @brief Utility function to send a WECHAT_START_SERVICE_CFM message to client task.
 *
 *  @param transport_type The transport type over which the service runs
 *  @param transport The transport instance over which the service runs. May be NULL for some transports which are configured on connection, such as SPP. Will also be NULL if the 'success' parameter indicates a failure to start the service.
 *  @param success Boolean indicating success or failure of the WechatStartService request
 */
void wechatTransportCommonSendWechatStartServiceCfm(wechat_transport_type transport_type, wechat_transport* transport, bool success);

/*! @brief Cleanup battery and RSSI threshold state for a given transport.
 
    Used once a Wechat transport has closed, as we no longer need to perform the
    checking or resend the timer messages.
*/
void wechatTransportCommonCleanupThresholdState(wechat_transport *transport);

#endif /* _WECHAT_TRANSPORT_COMMON_H */
