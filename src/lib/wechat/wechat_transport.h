/*****************************************************************
Copyright (C) Cambridge Silicon Radio Ltd. 2011-2014
*/

#ifndef _WECHAT_TRANSPORT_H
#define _WECHAT_TRANSPORT_H


/*! @brief Attempt to disconnect Wechat over a given transport.
 */
void wechatTransportDisconnectReq(wechat_transport *transport);


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


/*! @brief Get the stream sink for a given transport.
 */
Sink wechatTransportGetSink(wechat_transport *transport);

/*! @brief Pass incoming message for handling by a given transport.
 */
bool wechatTransportHandleMessage(Task task, MessageId id, Message message);

#endif /* _WECHAT_TRANSPORT_H */
