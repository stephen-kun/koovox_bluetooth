/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2010-2014
Part of ADK 3.5

*/
 
/*!
    @file wechat.h
    @brief Header file for the Generic Application Interface Architecture library

    This library implements the WECHAT protocol.

    The library exposes a functional downstream API and a message-based upstream API.
*/

#ifndef _WECHAT_H_
#define _WECHAT_H_

#include <library.h>
#include <stdlib.h>
#include <bdaddr.h>
#include <message.h>



/*! @brief Enumeration of types of transport which Wechat supports.
 */
typedef enum
{
    wechat_transport_none,                /*!< No transport assigned */
    wechat_transport_spp,                 /*!< Serial Port Profile (SPP) transport */
    wechat_transport_rfcomm,              /*!< RFCOMM using CSR WECHAT UUID */
    wechat_transport_mfi                  /*!< Made For iPod (MFi) transport */
} wechat_transport_type;


/*! @brief Enumeration of messages a client task may receive from the Wechat library.
 */
typedef enum
{
    WECHAT_INIT_CFM = WECHAT_MESSAGE_BASE,  /*!< Confirmation that the library has initialised */  
    WECHAT_CONNECT_CFM,                   /*!< Confirmation of an outbound connection request */
    WECHAT_CONNECT_IND,                   /*!< Indication of an inbound connection */
    WECHAT_DISCONNECT_IND,                /*!< Indication that the connection has closed */
    WECHAT_DISCONNECT_CFM,                /*!< Confirmation that a requested disconnection has completed */
    WECHAT_START_SERVICE_CFM,             /*!< Confirmation that a Wechat server has started */
    WECHAT_SEND_PACKET_CFM,               /*!< Confirmation that a WechatSendPacket request has completed */
	WECHAT_MESSAGE_MORE_DATA,
    WECHAT_MESSAGE_TOP
} WechatMessageId;


/*! @brief Definition of a WECHAT_TRANSPORT handle.
 *
 * Used to identity a specific Wechat connection with a remote device over a specific transport instance.
 */
typedef struct __WECHAT_TRANSPORT WECHAT_TRANSPORT;


/*! @brief Message confirming status of Wechat library initialisation.
 */
typedef struct
{
    bool success;                       /*!< The success of the initialisation request */
} WECHAT_INIT_CFM_T;

/*! @brief Message confirming status of Wechat library initialisation.
 */
typedef struct
{
    uint16 size_value;                       
	uint8  value[1];
} WECHAT_MESSAGE_MORE_DATA_T;


/*! @brief Message indicating an incoming connection attempt.
 */
typedef struct
{
    WECHAT_TRANSPORT *transport;          /*!< Indicates the connected instance */
    bool success;                       /*!< Indication of the success of the connection */
} WECHAT_CONNECT_IND_T;

/*! @brief Message indicating a Wechat connection has been disconnected.
 */
typedef struct
{
    WECHAT_TRANSPORT *transport;          /*!< Indicates the disconnected instance */
} WECHAT_DISCONNECT_IND_T;

/*! @brief Message confirming the status of a request by a client task to disconnect a Wechat connection.
 */
typedef struct
{
    WECHAT_TRANSPORT *transport;          /*!< Indicates the disconnected instance */
} WECHAT_DISCONNECT_CFM_T;

/*! @brief Message confirming the status of a connection request initiated by a client task.
 */
typedef struct
{
    WECHAT_TRANSPORT *transport;          /*!< Indicates the connected instance */
    bool success;                       /*!< The success of the connection request */  
} WECHAT_CONNECT_CFM_T;

/*! @brief Message confirming the status of a Start Service request initiated by a client task.
 */
typedef struct
{
    wechat_transport_type transport_type; /*!< Indicates the transport type */
    WECHAT_TRANSPORT *transport;          /*!< Indicates the transport instance */
    bool success;                       /*!< The success of the request */  
} WECHAT_START_SERVICE_CFM_T;


/*! @brief Message confirming the status of a request to transmit a Wechat packet by a client task.
 */
typedef struct
{
    WECHAT_TRANSPORT *transport;          /*!< Indicates the Wechat instance */
    uint8* packet;                      /*!< Pointer to the packet Wechat was given to transmit */
    bool success;                       /*!< Success/failure of transmission */
} WECHAT_SEND_PACKET_CFM_T;


/*! 
    @brief Disconnect from host
*/
void WechatDisconnectRequest(WECHAT_TRANSPORT *transport);


/*! 
    @brief Respond to a WECHAT_DISCONNECT_IND message
*/
void WechatDisconnectResponse(WECHAT_TRANSPORT *transport);


/*! 
    @brief Initialise the Wechat protocol handler library
    
    @param task The task that is responsible for handling messages sent from the library.
    @param max_connections The maximum number of concurrent connections to be accommodated.
*/
void WechatInit(Task task, uint16 max_connections);



/*!
    @brief Request that the Wechat library transmit a Wechat command over a given transport.

    @param transport Pointer to a WECHAT_TRANSPORT over which to transmit.

    @param packet_length Length of the 'packet' buffer containing the Wechat command to transmit.
    
    @param packet Pointer to a buffer containing the Wechat command to transmit.
*/
void WechatSendPacket(WECHAT_TRANSPORT *transport, uint16 packet_length, uint8 *packet);



/*! 
    @brief Starts the Wechat server for the transport type.
    
    @param transport_type The transport type which is to be served.
*/
void WechatStartService(wechat_transport_type transport_type);



/*! 
    @brief Returns TRUE if WECHAT session is enabled for the given transport instance

    @param transport Pointer to a WECHAT_TRANSPORT instance.
*/
bool WechatGetSessionEnable(WECHAT_TRANSPORT *transport);


/*! 
    @brief Enables or disables WECHAT session for the given transport instance

    @param transport Pointer to a WECHAT_TRANSPORT instance.
    
    @param enable TRUE to enable the session, FALSE to disable it
*/
void WechatSetSessionEnable(WECHAT_TRANSPORT *transport, bool enable);




#endif /* ifndef _WECHAT_H_ */
