/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#ifndef _WECHAT_TRANSPORT_RFCOMM_H
#define _WECHAT_TRANSPORT_RFCOMM_H

#ifdef WECHAT_TRANSPORT_RFCOMM

/*! @brief
 */
void wechatTransportRfcommInit(wechat_transport *transport);

/*! @brief
 */
void wechatTransportRfcommConnectReq(wechat_transport *transport, bdaddr *address);

/*! @brief
 */
void wechatTransportRfcommConnectRes(wechat_transport *transport);

/*! @brief
 */
void wechatTransportRfcommDisconnectReq(wechat_transport *transport);

/*! @brief
 */
void wechatTransportRfcommDisconnectRes(wechat_transport *transport);

/*! @brief
 */
void wechatTransportRfcommDropState(wechat_transport *transport);

/*! @brief
 */
void wechatTransportRfcommStartService(void);

/*! @brief
 */
void wechatTransportRfcommSendPacket(Task task, wechat_transport *transport, uint16 length, uint8 *data);

/*! @brief
 */
Sink wechatTransportRfcommGetSink(wechat_transport *transport);

/*! @brief
 */
bool wechatTransportRfcommHandleMessage(Task task, MessageId id, Message message);

#endif /* WECHAT_TRANSPORT_RFCOMM */

#endif /* _WECHAT_TRANSPORT_RFCOMM_H */
