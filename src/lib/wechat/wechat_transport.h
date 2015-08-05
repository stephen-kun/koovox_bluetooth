/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#ifndef _WECHAT_TRANSPORT_H
#define _WECHAT_TRANSPORT_H

/*! @brief Initialise a Wechat transport.
 */
void wechatTransportInit(wechat_transport *transport);

/*! @brief Attempt to connect Wechat to a device over a given transport.
 */
void wechatTransportConnectReq(wechat_transport *transport, bdaddr *address);

/*! @brief Respond to an incoming Wechat connection indication over a given transport.
 */
void wechatTransportConnectRes(wechat_transport *transport);

/*! @brief Attempt to disconnect Wechat over a given transport.
 */
void wechatTransportDisconnectReq(wechat_transport *transport);

/*! @brief Respond to a Wechat disconnection indication over a given transport.
 */
void wechatTransportDisconnectRes(wechat_transport *transport);

/*! @brief Clear down state of given transport.
 */
void wechatTransportDropState(wechat_transport *transport);

/*! @brief Attempt to find a free transport slot
 */
wechat_transport *wechatTransportFindFree(void);

/*! @brief Attempt to find the tranport associated with an RFCOMM channel
 */
wechat_transport *wechatTransportFromRfcommChannel(uint16 channel);
        
/*! @brief Attempt to find the tranport associated with a sink
 */
wechat_transport *wechatTransportFromSink(Sink sink);

/*! @brief Start Wechat as a server on a given transport.
 *
 * NOTE - only applicable to the SPP transport.
 */
void wechatTransportStartService(wechat_transport_type transport_type);

/*! @brief Transmit a Wechat packet over a given transport.
 */
void wechatTransportSendPacket(Task task, wechat_transport *transport, uint16 length, uint8 *data);

/*! @brief Get the stream source for a given transport.
 *
 * NOTE - only applicable to the SPP transport.
 */
Source wechatTransportGetSource(wechat_transport *transport);

/*! @brief Get the stream sink for a given transport.
 */
Sink wechatTransportGetSink(wechat_transport *transport);

/*! @brief Pass incoming message for handling by a given transport.
 */
bool wechatTransportHandleMessage(Task task, MessageId id, Message message);

#endif /* _WECHAT_TRANSPORT_H */
