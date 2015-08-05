/*************************************************************************
Copyright ?CSR plc 2010-2014
Part of ADK 3.5

FILE
    wechat.c
    
DESCRIPTION
    Generic Application Interface Architecture
    This library implements the WECHAT protocol for a server- or client-side
    SPP connection.
    
    The library exposes a functional downstream API and a message-based upstream API.
*/
#include <connection.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vm.h>
#include <boot.h>
#include <ps.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <print.h>
#include <string.h>
#include <power.h>
#include <psu.h>
#include <partition.h>
#include <file.h>
#include <spp_common.h>
#include <audio.h>
#include <loader.h>
#include <kalimba.h>

#include "wechat_private.h"
#include "wechat_transport.h"
#include "wechat_transport_rfcomm.h"

/*  Commands which can be handled by the library but can be supplanted
 *  by an application implementation.
 *  
 *  WECHAT_COMMAND_POWER_OFF is at index 0 for the benefit of
 *  WechatInit(); that's the only one initially handled by the app
 */
static const uint16 lib_commands[] =
{
    WECHAT_COMMAND_POWER_OFF,
    WECHAT_COMMAND_DEVICE_RESET,
    WECHAT_COMMAND_GET_BOOTMODE, 
    WECHAT_COMMAND_GET_API_VERSION,    
    
    WECHAT_COMMAND_GET_CURRENT_RSSI,
    WECHAT_COMMAND_GET_CURRENT_BATTERY_LEVEL,
    WECHAT_COMMAND_RETRIEVE_PS_KEY,
    WECHAT_COMMAND_RETRIEVE_FULL_PS_KEY,
    
    WECHAT_COMMAND_STORE_PS_KEY,
    WECHAT_COMMAND_SEND_APPLICATION_MESSAGE,
    WECHAT_COMMAND_SEND_KALIMBA_MESSAGE,
    WECHAT_COMMAND_DELETE_PDL,
    
    WECHAT_COMMAND_REGISTER_NOTIFICATION,
    WECHAT_COMMAND_GET_NOTIFICATION,
    WECHAT_COMMAND_CANCEL_NOTIFICATION
};

WECHAT_T *wechat = NULL;


/*************************************************************************
NAME
    dwunpack
    
DESCRIPTION
    Unpack a uint32 into an array of uint8s
*/
static void dwunpack(uint8 *dest, uint32 src)
{
    *dest++ = src >> 24;
    *dest++ = (src >> 16) & 0xFF;
    *dest++ = (src >> 8) & 0xFF;
    *dest = src & 0xFF;
}


/*************************************************************************
NAME
    wpack
    
DESCRIPTION
    Pack array of uint8s into array of uint16
*/
static void wpack(uint16 *dest, uint8 *src, uint16 length)
{
    while (length--)
    {
        *dest = *src++ << 8;
        *dest++ |= *src++;
    }
}


/*************************************************************************
NAME
    find_locus_bit
    
DESCRIPTION
    Determine if command can be handled by the library
    Return index into lib_commands array or WECHAT_INVALID_ID
*/
static uint16 find_locus_bit(uint16 command_id)
{
    uint16 idx;
    
    for (idx = 0; idx < sizeof lib_commands; ++idx)
        if (lib_commands[idx] == command_id)
            return idx;
    
    return WECHAT_INVALID_ID;
}


/*************************************************************************
NAME
    read_battery
    
DESCRIPTION
    Read the battery voltage using the appropriate ADC
*/
static void read_battery(void)
{
    if(wechat->no_vdd_sense || !AdcRequest(&wechat->task_data, adcsel_vdd_sense))
    {
        wechat->no_vdd_sense = TRUE;
        AdcRequest(&wechat->task_data, adcsel_vdd_bat);
    }
}


/*************************************************************************
NAME
    app_will_handle_command
    
DESCRIPTION
    Determine if the specified command is to be passed to the onchip app
*/
static bool app_will_handle_command(uint16 command_id)
{
/*  Not a command but an acknowledgement of an event notification  */
    if (command_id == WECHAT_ACK_NOTIFICATION)
        return FALSE;
    
/*  Look it up  */
    return WechatGetAppWillHandleCommand(command_id);
}


/*************************************************************************
NAME
    build_packet
    
DESCRIPTION
    Build a Wechat protocol packet in the given buffer
    uint8 payload variant
    If <status> is other than WECHAT_STATUS_NONE it is inserted
    before the first byte of the passed-in payload to simplify
    the building of acknowledgement and notification packets
    
    Returns the length of the packet or 0 on error
*/
static uint16 build_packet(uint8 *buffer, uint8 flags,
                              uint16 vendor_id, uint16 command_id, uint16 status, 
                              uint8 payload_length, uint8 *payload)
{
    uint8 *data = buffer;
    uint16 packet_length = WECHAT_OFFS_PAYLOAD + payload_length;

    if (status != WECHAT_STATUS_NONE)
        ++packet_length;

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    if (packet_length > WECHAT_MAX_PACKET)
        return 0;

    /*  Build the header.  It's that diagram again ... 
     *  0 bytes  1        2        3        4        5        6        7        8          9    len+8      len+9
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     */
    *data++ = WECHAT_SOF;
    *data++ = WECHAT_VERSION;
    *data++ = flags;
    *data++ = payload_length;
    *data++ = HIGH(vendor_id);
    *data++ = LOW(vendor_id);
    *data++ = HIGH(command_id);
    *data++ = LOW(command_id);

    if (status != WECHAT_STATUS_NONE)
    {
        /*  Insert status-cum-event byte and increment payload length  */
        *data++ = status;
        ++buffer[WECHAT_OFFS_PAYLOAD_LENGTH];
    }

    /*  Copy in the payload  */
    while (payload_length--)
        *data++ = *payload++;

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
    {
    /*  Compute the checksum  */
        uint8 check = 0;
        data = buffer;

        while (--packet_length)
            check ^= *data++;

        *data++ = check;
    }

    return data - buffer;    
}


/*************************************************************************
NAME
    build_packet_16
    
DESCRIPTION
    Build a Wechat protocol packet in the given buffer
    uint16 payload variant
    If <status> is other than WECHAT_STATUS_NONE it is inserted
    before the first byte of the passed-in payload to simplify
    the building of acknowledgement and notification packets
    
    payload_length is the number of uint16s in the payload
    Returns the length of the packet or 0 on error
*/
static uint16 build_packet_16(uint8 *buffer, uint8 flags,
                              uint16 vendor_id, uint16 command_id, uint16 status, 
                              uint8 payload_length, uint16 *payload)
{
    uint8 *data = buffer;
    uint16 packet_length = WECHAT_OFFS_PAYLOAD + 2 * payload_length;

    if (status != WECHAT_STATUS_NONE)
        ++packet_length;

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    if (packet_length > WECHAT_MAX_PACKET)
        return 0;

    /*  Build the header.  It's that diagram again ... 
     *  0 bytes  1        2        3        4        5        6        7        8          9    len+8      len+9
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
     *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
     */
    *data++ = WECHAT_SOF;
    *data++ = WECHAT_VERSION;
    *data++ = flags;
    *data++ = 2 * payload_length;
    *data++ = HIGH(vendor_id);
    *data++ = LOW(vendor_id);
    *data++ = HIGH(command_id);
    *data++ = LOW(command_id);

    if (status != WECHAT_STATUS_NONE)
    {
        /*  Insert status-cum-event byte and increment payload length  */
        *data++ = status;
        ++buffer[WECHAT_OFFS_PAYLOAD_LENGTH];
    }

    /*  Copy in the payload  */
    while (payload_length--)
    {
        *data++ = HIGH(*payload);
        *data++ = LOW(*payload++);
    }

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
    {
        /*  Compute the checksum  */
        uint8 check = 0;
        data = buffer;

        while (--packet_length)
            check ^= *data++;

        *data++ = check;
    }

    return data - buffer;    
}


/*************************************************************************
NAME
    send_packet
    
DESCRIPTION
    Send a Wechat packet
*/
static void send_packet(Task task, wechat_transport *transport, uint16 packet_length, uint8 *packet)
{
    MESSAGE_PMAKE(m, WECHAT_INTERNAL_SEND_REQ_T);
    
    WECHAT_DEBUG(("wechat: send_packet: t=%04x s=%d\n", (uint16) task, VmGetAvailableAllocations()));
    
    m->task = task;
    m->transport = transport;
    m->length = packet_length;
    m->data = packet;
    
    MessageSend(&wechat->task_data, WECHAT_INTERNAL_SEND_REQ, m);
}


/*************************************************************************
NAME
    send_response
    
DESCRIPTION
    Build and send a Wechat packet
*/
static void send_response(wechat_transport *transport, uint16 vendor_id, uint16 command_id,
                          uint8 status, uint16 payload_length, uint8 *payload)
{
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = transport->flags;

    packet_length = WECHAT_OFFS_PAYLOAD + payload_length + 1;
    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;
    
    packet = malloc(packet_length);
    
    if (packet)
    {
        packet_length = build_packet(packet, flags, vendor_id, command_id, 
                                     status, payload_length, payload);
        
        send_packet(NULL, transport, packet_length, packet);
    }
}


/*************************************************************************
NAME
    send_ack
    
DESCRIPTION
    Build and send a Wechat acknowledgement packet
*/
#define send_ack(transport, vendor, command, status, length, payload) \
    send_response(transport, vendor, (command) | WECHAT_ACK_MASK, status, length, payload)

            
/*************************************************************************
NAME
    send_ack_16
    
DESCRIPTION
    Build and send a Wechat acknowledgement packet from a uint16[] payload
*/
static void send_ack_16(wechat_transport *transport, uint16 vendor_id, uint16 command_id,
                          uint8 status, uint16 payload_length, uint16 *payload)
{
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = transport->flags;
    

    packet_length = WECHAT_OFFS_PAYLOAD + 2 * payload_length + 1;
    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;
    
    packet = malloc(packet_length);
    
    if (packet)
    {
        packet_length = build_packet_16(packet, flags, vendor_id, command_id | WECHAT_ACK_MASK, 
                                     status, payload_length, payload);
        
        send_packet(NULL, transport, packet_length, packet);
    }
}


/*************************************************************************
NAME
    send_simple_response
    
DESCRIPTION
    Build and send a Wechat acknowledgement packet
*/
#define send_simple_response(transport, command, status) \
    send_ack(transport, WECHAT_VENDOR_CSR, command, status, 0, NULL)

            
/*************************************************************************
NAME
    send_success
    
DESCRIPTION
    Send a successful response to the given command
*/
#define send_success(transport, command) \
    send_simple_response(transport, command, WECHAT_STATUS_SUCCESS)

    
/*************************************************************************
NAME
    send_success_payload
    
DESCRIPTION
    Send a successful response incorporating the given payload
*/
#define send_success_payload(transport, command, length, payload) \
    send_ack(transport, WECHAT_VENDOR_CSR, command, WECHAT_STATUS_SUCCESS, length, payload)

    
/*************************************************************************
NAME
    send_notification
    
DESCRIPTION
    Send a notification incorporating the given payload
*/
#define send_notification(transport, event, length, payload) \
    send_response(transport, WECHAT_VENDOR_CSR, WECHAT_EVENT_NOTIFICATION, \
                  event, length, payload)


/*************************************************************************
NAME
    send_insufficient_resources
    
DESCRIPTION
    Send an INSUFFICIENT_RESOURCES response to the given command
*/
#define send_insufficient_resources(transport, command) \
    send_simple_response(transport, command, WECHAT_STATUS_INSUFFICIENT_RESOURCES)   


/*************************************************************************
NAME
    send_invalid_parameter
    
DESCRIPTION
    Send an INVALID_PARAMETER response to the given command
*/
#define send_invalid_parameter(transport, command) \
    send_simple_response(transport, command, WECHAT_STATUS_INVALID_PARAMETER)


/*************************************************************************
NAME
    send_incorrect_state
    
DESCRIPTION
    Send an INCORRECT_STATE response to the given command
*/
#define send_incorrect_state(transport, command) \
    send_simple_response(transport, command, WECHAT_STATUS_INCORRECT_STATE)


/*************************************************************************
NAME
    send_api_version
    
DESCRIPTION
    Send a completed WECHAT_COMMAND_GET_API_VERSION response
*/
static void send_api_version(wechat_transport *transport)
{
    uint8 payload[3];
    
    payload[0] = WECHAT_VERSION;
    payload[1] = WECHAT_API_VERSION_MAJOR;
    payload[2] = wechat->api_minor;
    
    send_success_payload(transport, WECHAT_COMMAND_GET_API_VERSION, 3, payload);
}


/*************************************************************************
NAME
    send_rssi
    
DESCRIPTION
    Send a completed WECHAT_COMMAND_GET_CURRENT_RSSI response
*/
static void send_rssi(wechat_transport *transport)
{
    int16 rssi = 0;

    if (SinkGetRssi(wechatTransportGetSink(transport), &rssi))
    {
        uint8 payload = LOW(rssi);
        send_success_payload(transport, WECHAT_COMMAND_GET_CURRENT_RSSI, 1, &payload);
    }
    
    else
        send_insufficient_resources(transport, WECHAT_COMMAND_GET_CURRENT_RSSI);
}


/*************************************************************************
NAME
    send_battery_level
    
DESCRIPTION
    Send a completed WECHAT_COMMAND_GET_CURRENT_BATTERY_LEVEL response
*/
static void send_battery_level(wechat_transport *transport)
{
    if (wechat->battery_voltage == 0)
        send_insufficient_resources(transport, WECHAT_COMMAND_GET_CURRENT_BATTERY_LEVEL);
    
    else
    {
        uint8 payload[2];
        
        payload[0] = HIGH(wechat->battery_voltage);
        payload[1] = LOW(wechat->battery_voltage);
        
        send_success_payload(transport, WECHAT_COMMAND_GET_CURRENT_BATTERY_LEVEL, 2, payload);
    }
}


/*************************************************************************
NAME
    queue_device_reset
    
DESCRIPTION
    Queue a device reset in the near future.  TATA_TIME should be time
    enough for the host to get our acknowledgement before we go.  Setting
    the 'rebooting' flag inhibits any further commands from the host.
 */
static void queue_device_reset(wechat_transport *transport, uint16 bootmode)
{        
    uint16 *reset_message;
    
    WECHAT_CMD_DEBUG(("wechat: queue_device_reset %u\n", bootmode));
    
    reset_message = malloc(sizeof (uint16));
    if (!reset_message)
        send_insufficient_resources(transport, WECHAT_COMMAND_DEVICE_RESET);
        
    else
    {
        wechat->rebooting = TRUE;
        *reset_message = bootmode;
        MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_REBOOT_REQ, reset_message, TATA_TIME);
        send_success(transport, WECHAT_COMMAND_DEVICE_RESET);
    }
}


/*************************************************************************
NAME
    send_bootmode
    
DESCRIPTION
    Handle a WECHAT_COMMAND_GET_BOOTMODE by returning the current bootmode
*/
static void send_bootmode(wechat_transport *transport)
{
    uint8 payload = BootGetMode();
    send_success_payload(transport, WECHAT_COMMAND_GET_BOOTMODE, 1, &payload);
}


/*************************************************************************
NAME
    queue_power_off
    
DESCRIPTION
    Queue a device power off in the near future.  TATA_TIME should be time
    enough for the host to get our acknowledgement before we go.  We don't
    set 'rebooting' here so we don't get stuck if the power doesn't go off
*/
static void queue_power_off(wechat_transport *transport)
{
    MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_POWER_OFF_REQ, NULL, TATA_TIME);
    send_success(transport, WECHAT_COMMAND_POWER_OFF);
}


/*! @brief Utility function to start timer for checking battery level thresholds.
 */
static void start_check_battery_threshold_timer(wechat_transport *transport)
{
    MESSAGE_PMAKE(batt_timer, WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T);
    batt_timer->transport = transport;
    MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ, batt_timer, WECHAT_CHECK_THRESHOLDS_PERIOD);
}

/*! @brief Utility function to start timer for checking RSSI level thresholds.
 */
static void start_check_rssi_threshold_timer(wechat_transport *transport)
{
    MESSAGE_PMAKE(rssi_timer, WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T);
    rssi_timer->transport = transport;
    MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ, rssi_timer, WECHAT_CHECK_THRESHOLDS_PERIOD);
}

/*************************************************************************
NAME
    register_notification
    
DESCRIPTION
    Registers the host's desire to receive notification of an event
    WECHAT_EVENT_DEBUG_MESSAGE requires WECHAT_FEATURE_DEBUG feature bit
*/
static void register_notification(wechat_transport *transport, uint16 payload_length, uint8 *payload)
{
    uint16 status = WECHAT_STATUS_INVALID_PARAMETER;
    
    WECHAT_CMD_DEBUG(("wechat: register_notification %d: %d\n", payload_length, payload[0]));
    
    switch (payload[0])
    {
    case WECHAT_EVENT_BATTERY_LOW_THRESHOLD:
        
        /* start the battery threshold check timer if necessary */
        if (!transport->threshold_count_lo_battery && !transport->threshold_count_hi_battery)
        {
            start_check_battery_threshold_timer(transport);
        }

        transport->threshold_count_lo_battery = 0;
        
    /*  Check for 1 or 2 two-byte arguments  */
        switch (payload_length)
        {
        case 5:
            transport->battery_lo_threshold[1] = W16(payload + 3);
            transport->threshold_count_lo_battery = 1;
        /*  drop through  */
        case 3:
            transport->battery_lo_threshold[0] = W16(payload + 1);
            transport->threshold_count_lo_battery++;
            status = WECHAT_STATUS_SUCCESS;
            break;
        }
            
        WECHAT_DEBUG(("wechat: batt lo %d: %d %d\n",
               transport->threshold_count_lo_battery,
               transport->battery_lo_threshold[0], 
               transport->battery_lo_threshold[1]));
                    
        break;
        
    case WECHAT_EVENT_BATTERY_HIGH_THRESHOLD:
        
        /* start the battery threshold check timer if necessary */
        if (!transport->threshold_count_lo_battery && !transport->threshold_count_hi_battery)
        {
            start_check_battery_threshold_timer(transport);
        }

        transport->threshold_count_hi_battery = 0;
        
    /*  Check for 1 or 2 two-byte arguments  */
        switch (payload_length)
        {
        case 5:
            transport->battery_hi_threshold[1] = W16(payload + 3);
            transport->threshold_count_hi_battery = 1;
        /*  drop through  */
        case 3:
            transport->battery_hi_threshold[0] = W16(payload + 1);
            transport->threshold_count_hi_battery++;
            status = WECHAT_STATUS_SUCCESS;
            break;
        }
            
        WECHAT_DEBUG(("wechat: batt hi %d: %d %d\n", 
               transport->threshold_count_hi_battery,
               transport->battery_hi_threshold[0], 
               transport->battery_hi_threshold[1]));
                    
        break;
        
    case WECHAT_EVENT_RSSI_LOW_THRESHOLD:

        /* start the RSSI check timer if necessary */
        if (!transport->threshold_count_lo_rssi && !transport->threshold_count_hi_rssi)
        {
            start_check_rssi_threshold_timer(transport);
        }

        transport->threshold_count_lo_rssi = 0;
        
    /*  Check for 1 or 2 one-byte arguments  */
        switch (payload_length)
        {
        case 3:
            transport->rssi_lo_threshold[1] = SEXT(payload[2]);
            transport->threshold_count_lo_rssi = 1;
        /*  drop through  */
        case 2:
            transport->rssi_lo_threshold[0] = SEXT(payload[1]);
            transport->threshold_count_lo_rssi++;
            status = WECHAT_STATUS_SUCCESS;
            break;
        }
        
        WECHAT_DEBUG(("wechat: RSSI lo %d: %d %d\n", 
               transport->threshold_count_lo_rssi,
               transport->rssi_lo_threshold[0], 
               transport->rssi_lo_threshold[1]));
        
        break;
        
    case WECHAT_EVENT_RSSI_HIGH_THRESHOLD:
        
        /* start the RSSI check timer if necessary */
        if (!transport->threshold_count_lo_rssi && !transport->threshold_count_hi_rssi)
        {
            start_check_rssi_threshold_timer(transport);
        }

        transport->threshold_count_hi_rssi = 0;
        
    /*  Check for 1 or 2 one-byte arguments  */
        switch (payload_length)
        {
        case 3:
            transport->rssi_hi_threshold[1] = SEXT(payload[2]);
            transport->threshold_count_hi_rssi = 1;
        /*  drop through  */
        case 2:
            transport->rssi_hi_threshold[0] = SEXT(payload[1]);
            transport->threshold_count_hi_rssi++;
            status = WECHAT_STATUS_SUCCESS;
            break;
        }
        
        WECHAT_DEBUG(("wechat: RSSI hi %d: %d %d\n",
               transport->threshold_count_hi_rssi,
               transport->rssi_hi_threshold[0], 
               transport->rssi_hi_threshold[1]));
        
        break;
    }
    
    send_ack(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_REGISTER_NOTIFICATION, status, 1, payload);
}


/*************************************************************************
NAME
    send_notification_setting
    
DESCRIPTION
    Respond to a WECHAT_COMMAND_GET_NOTIFICATION command
    The response holds the notification setting for the given event,
    if any, e.g. if two Battery Low Thresholds were configured:

    +--------+--------+--------+--------+--------+--------+--------+
    | STATUS |  TYPE  | COUNT  |   THRESHOLD 1   |   THRESHOLD 2   |
    +--------+--------+--------+--------+--------+--------+--------+

    Status: 0x00, success
    Type:   0x02, Batery Low Threshold
    Count:  2
            
    For events which are simply disabled or enabled the 'count' is 0 or 1
    
NOTE
    payload_length does not include the STATUS byte; WechatSendResponse()
    adds that later
*/
static void send_notification_setting(wechat_transport *transport, uint8 event)
{
    uint8 payload[6];
    uint8 *p = payload;
    
    *p++ = event;
    
    switch (event)
    {
    case WECHAT_EVENT_RSSI_LOW_THRESHOLD:
        *p++ = transport->threshold_count_lo_rssi;
        
        if (transport->threshold_count_lo_rssi > 0)
            *p++ = transport->rssi_lo_threshold[0];
        
        if (transport->threshold_count_lo_rssi > 1)
            *p++ = transport->rssi_lo_threshold[1];
        
        break;
        
    case WECHAT_EVENT_RSSI_HIGH_THRESHOLD:
        *p++ = transport->threshold_count_hi_rssi;
        
        if (transport->threshold_count_hi_rssi > 0)
            *p++ = transport->rssi_hi_threshold[0];
        
        if (transport->threshold_count_hi_rssi > 1)
            *p++ = transport->rssi_hi_threshold[1];
        
        break;
        
    case WECHAT_EVENT_BATTERY_LOW_THRESHOLD:
        *p++= transport->threshold_count_lo_battery;
        
        if (transport->threshold_count_lo_battery > 0)
        {
            *p++ = HIGH(transport->battery_lo_threshold[0]);
            *p++ = LOW(transport->battery_lo_threshold[0]);
        }
        
        if (transport->threshold_count_lo_battery > 1)
        {
            *p++ = HIGH(transport->battery_lo_threshold[1]);
            *p++ = LOW(transport->battery_lo_threshold[1]);
        }
        
        break;
        
    case WECHAT_EVENT_BATTERY_HIGH_THRESHOLD:
        *p++ = transport->threshold_count_hi_battery;
        
        if (transport->threshold_count_hi_battery > 0)
        {
            *p++ = HIGH(transport->battery_hi_threshold[0]);
            *p++ = LOW(transport->battery_hi_threshold[0]);
        }
        
        if (transport->threshold_count_hi_battery > 1)
        {
            *p++ = HIGH(transport->battery_hi_threshold[1]);
            *p++ = LOW(transport->battery_hi_threshold[1]);
        }
        
        break;
        
/*  Dummies for legal but unimplemented events  */
    case WECHAT_EVENT_BATTERY_CHARGED:
    case WECHAT_EVENT_DEVICE_STATE_CHANGED:
    case WECHAT_EVENT_PIO_CHANGED:
        *p++ = WECHAT_FEATURE_DISABLED;
        break;        
    }
    
    
    if (p > payload)
        send_success_payload(transport, WECHAT_COMMAND_GET_NOTIFICATION, p - payload, payload);
    
    else
        send_invalid_parameter(transport, WECHAT_COMMAND_GET_NOTIFICATION);
}

/*************************************************************************
NAME
    cancel_notification
    
DESCRIPTION
    The host desires to receive no further notification of an event   
*/
static void cancel_notification(wechat_transport *transport, uint8 event_id)
{
/*  uint16 old_flags = wechat->threshold_poll_flags;  */
    uint16 status = WECHAT_STATUS_INVALID_PARAMETER;
    
    switch(event_id)
    {
    case WECHAT_EVENT_BATTERY_LOW_THRESHOLD:
        transport->threshold_count_lo_battery = 0;
        transport->sent_notification_lo_battery_0 = FALSE;
        transport->sent_notification_lo_battery_1 = FALSE;
        status = WECHAT_STATUS_SUCCESS;
        break;
           
    case WECHAT_EVENT_BATTERY_HIGH_THRESHOLD:
        transport->threshold_count_hi_battery = 0;
        transport->sent_notification_hi_battery_0 = FALSE;
        transport->sent_notification_hi_battery_1 = FALSE;
        status = WECHAT_STATUS_SUCCESS;
        break;
           
    case WECHAT_EVENT_RSSI_LOW_THRESHOLD:
        transport->threshold_count_lo_rssi = 0;
        transport->sent_notification_lo_rssi_0 = FALSE;
        transport->sent_notification_lo_rssi_1 = FALSE;
        status = WECHAT_STATUS_SUCCESS;
        break;
        
    case WECHAT_EVENT_RSSI_HIGH_THRESHOLD:
        transport->threshold_count_hi_rssi = 0;
        transport->sent_notification_hi_rssi_0 = FALSE;
        transport->sent_notification_hi_rssi_1 = FALSE;
        status = WECHAT_STATUS_SUCCESS;
        break;
    }

    
    send_ack(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_CANCEL_NOTIFICATION, status, 1, &event_id);
}


/*************************************************************************
NAME
    send_module_id
    
DESCRIPTION
    Respond to a WECHAT_COMMAND_GET_MODULE_ID command
    
    The response holds the chip id, design id and module id proper.
    For Flash devices, chip id will be zero and module id information
    is read from persistent store if present.
*/
static void send_module_id(wechat_transport *transport)
{
    uint16 module_id[4];
    memset(module_id, 0, sizeof module_id);
    
    PsFullRetrieve(PSKEY_MODULE_DESIGN, module_id + 1, 1);
    PsFullRetrieve(PSKEY_MODULE_ID, module_id + 2, 2);
    
    send_ack_16(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_GET_MODULE_ID, 
        WECHAT_STATUS_SUCCESS, sizeof module_id, module_id);
}


/*************************************************************************
NAME
    wechat_handle_control_command
    
DESCRIPTION
    Handle a control command or return FALSE if we can't
*/
static bool wechat_handle_control_command(wechat_transport *transport, uint16 command_id,
                                        uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case WECHAT_COMMAND_DEVICE_RESET:
        if (payload_length == 0)
            queue_device_reset(transport, BootGetMode());
         
        else if ((payload_length == 1) && (payload[0] <= PS_MAX_BOOTMODE))
            queue_device_reset(transport, payload[0]);
    
        else
            send_invalid_parameter(transport, WECHAT_COMMAND_DEVICE_RESET);
    
        return TRUE;
                    
        
    case WECHAT_COMMAND_GET_BOOTMODE:
        send_bootmode(transport);
        return TRUE;
        
        
    case WECHAT_COMMAND_POWER_OFF:
        queue_power_off(transport);
        return TRUE;
    }
    
    return FALSE;
}


/*************************************************************************
NAME
    wechat_handle_status_command
    
DESCRIPTION
    Handle a Polled Status command or return FALSE if we can't
*/
static bool wechat_handle_status_command(wechat_transport *transport, uint16 command_id, 
                                       uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case WECHAT_COMMAND_GET_API_VERSION:
        send_api_version(transport);
        return TRUE;
        
    case WECHAT_COMMAND_GET_CURRENT_RSSI:
        send_rssi(transport);
        return TRUE;
        
    case WECHAT_COMMAND_GET_CURRENT_BATTERY_LEVEL:
    /*  If we're checking thresholds use its averaged reading, else fire off an ADC request  */
        if (transport->threshold_count_lo_battery || transport->threshold_count_hi_battery)
            send_battery_level(transport);
        
        else if (wechat->outstanding_request == NULL)
        {
            wechat->outstanding_request = transport;
            read_battery();
        }

        return TRUE;
        
    case WECHAT_COMMAND_GET_MODULE_ID:
        send_module_id(transport);
        return TRUE;
    }
    
    return FALSE;
}
                
      
/*************************************************************************
NAME
    wechat_handle_feature_command
    
DESCRIPTION
    Handle a Feature Control command or return FALSE if we can't
*/
static bool wechat_handle_feature_command(wechat_transport *transport, uint16 command_id, 
                                        uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case WECHAT_COMMAND_SET_SESSION_ENABLE:
        if ((payload_length > 0) && (payload[0] < 2))
        {
            transport->enabled = payload[0];
            send_success(transport, command_id);
        }
        
        else
            send_invalid_parameter(transport, command_id);
        
        return TRUE;
        
    case WECHAT_COMMAND_GET_SESSION_ENABLE:
        {
           uint8 response;
           response = transport->enabled;
           send_success_payload(transport, command_id, 1, &response);
           return TRUE;
        }
    }
    return FALSE;
}

/*************************************************************************
NAME
    dfu_reset_timeout
    
DESCRIPTION
    Reset the DFU start / transfer timeout
    
*/
static void dfu_reset_timeout(uint16 period)
{
    WECHAT_DFU_DEBUG(("wechat: dfu tmo=%u\n", period));

    MessageCancelAll(&wechat->task_data, WECHAT_INTERNAL_DFU_TIMEOUT);
    
    if (period)
    {
        MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_DFU_TIMEOUT, NULL, D_SEC(period));
    }
}


/*************************************************************************
NAME
    dfu_confirm
    
DESCRIPTION
    Send a WECHAT_DFU_CFM response to the application task
 
*/
static void dfu_confirm(wechat_transport *transport, bool success)
{
    MESSAGE_PMAKE(cfm, WECHAT_DFU_CFM_T);
    cfm->transport = (WECHAT_TRANSPORT *) transport;
    cfm->success = success;
    MessageSend(wechat->app_task, WECHAT_DFU_CFM, cfm);
}

/*************************************************************************
NAME
    dfu_indicate
    
DESCRIPTION
    Send a WECHAT_DFU_IND to the application task
 
*/
static void dfu_indicate(uint8 state)
{
    MESSAGE_PMAKE(ind, WECHAT_DFU_IND_T);
    ind->transport = (WECHAT_TRANSPORT *) wechat->outstanding_request;
    ind->state = state;
    MessageSend(wechat->app_task, WECHAT_DFU_IND, ind);
}


/*************************************************************************
NAME
    dfu_send_state
    
DESCRIPTION
    Send a DFU State Change notification


*/
static void dfu_send_state(uint8 state)
{
    if (wechat->outstanding_request)
        send_notification(wechat->outstanding_request, WECHAT_EVENT_DFU_STATE, 1, &state);

    dfu_indicate(state);
}

#ifdef HAVE_PARTITION_FILESYSTEM

/*************************************************************************
NAME
    pfs_status
    
DESCRIPTION
    Return status of given storage partition
 
*/
static partition_state pfs_status(uint8 device, uint8 partition)
{
    uint32 value = 0;
    bool ok;

    ok = PartitionGetInfo(device, partition, PARTITION_INFO_TYPE, &value) && 
            (value == PARTITION_TYPE_FILESYSTEM);
    
    WECHAT_DEBUG(("wechat: pfs %u %u sts %u %lu\n", device, partition, ok, value));
    
    if (!ok)
        return PARTITION_INVALID;
        
    ok = PartitionGetInfo(device, partition, PARTITION_INFO_IS_MOUNTED, &value);
     
    WECHAT_DEBUG(("wechat: pfs %u %u mnt %u %lu\n", device, partition, ok, value));
   
    if (!ok)
        return PARTITION_INVALID;
       
    if (value)
        return PARTITION_MOUNTED;
    
    return PARTITION_UNMOUNTED;
}


/*************************************************************************
NAME
    send_pfs_status
    
DESCRIPTION
    Respond to a WECHAT_COMMAND_GET_STORAGE_PARTITION_STATUS command with
    details for given partition

    +--------+--------+--------+-----------------------------------+--------+
    | DEVICE |PARTIT'N|  TYPE  |                SIZE               |MOUNTED |
    +--------+--------+--------+--------+--------+--------+--------+--------+
        0        1        2        3        4        5        6        7
*/
static void send_pfs_status(wechat_transport *transport, uint8 device, uint8 partition)
{
    uint32 value;
    bool ok;
    uint8 response[8];

    ok = PartitionGetInfo(device, partition, PARTITION_INFO_TYPE, &value);

    if (ok)
    {
        response[0] = device;
        response[1] = partition;
        response[2] = value;
        
        ok = PartitionGetInfo(device, partition, PARTITION_INFO_SIZE, &value);
    }

    if (ok)
    {
        dwunpack(response + 3, value);
        ok = PartitionGetInfo(device, partition, PARTITION_INFO_IS_MOUNTED, &value);
    }

    if (ok)
    {
        response[7] = value;

        send_success_payload(transport, WECHAT_COMMAND_GET_STORAGE_PARTITION_STATUS,
            sizeof response, response);
    }

    else
        send_invalid_parameter(transport, WECHAT_COMMAND_GET_STORAGE_PARTITION_STATUS);
}


/*************************************************************************
NAME
    open_storage_partition
    
DESCRIPTION
    Prepare a storage partition for writing
 
*/
static void open_storage_partition(wechat_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if (IsAudioBusy())
    {
        WECHAT_DEBUG(("wechat: open_storage_partition - audio busy error 0x%x (0x%x)\n",(uint16)IsAudioBusy(), (uint16)&wechat->task_data ));
        status = WECHAT_STATUS_INCORRECT_STATE;
    }
    else if ((payload_length != 3) && (payload_length != 7))
    /*  may or may not have 32-bit CRC  */
        status = WECHAT_STATUS_INVALID_PARAMETER;

    else if ((payload[0] != PARTITION_SERIAL_FLASH) || (payload[2] != WECHAT_PFS_MODE_OVERWRITE))
    /*  Currently we can only overwrite, and it has to be ext flash  */
        status = WECHAT_STATUS_INVALID_PARAMETER;
    
    else if ((wechat->pfs_state != PFS_NONE) || (wechat->outstanding_request != NULL))
    /*  We support only one stream  */
        status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;

    else if (pfs_status(payload[0], payload[1]) != PARTITION_UNMOUNTED)
    /*  The partition must be a filesystem and must be unmounted  */
        status = WECHAT_STATUS_INCORRECT_STATE;
        
    else
    {               
        bool ok; 
        
        WECHAT_DEBUG(("wechat: open_storage_partition - set audio busy\n"));
        SetAudioBusy(&wechat->task_data);
       
        WECHAT_DEBUG(("wechat: start overwrite sink %lu\n", VmGetClock()));
        wechat->pfs.sink = StreamPartitionOverwriteSink(payload[0], payload[1]);
        WECHAT_DEBUG(("wechat: finish overwrite sink %lu\n", VmGetClock()));

        if (payload_length == 3)
            ok = PartitionSetMessageDigest(wechat->pfs.sink, PARTITION_MESSAGE_DIGEST_SKIP, NULL, 0);
        
        else
        {
            uint32 crc;
            crc = payload[3];
            crc = (crc << 8) | payload[4];
            crc = (crc << 8) | payload[5];
            crc = (crc << 8) | payload[6];
         
            WECHAT_DEBUG(("wechat: write pfs crc %lu\n", crc));
                 
            ok = PartitionSetMessageDigest(wechat->pfs.sink, PARTITION_MESSAGE_DIGEST_CRC, (uint16 *) &crc, sizeof crc);
        }
        
        if (ok)
        {
            uint8 payload;
            
            wechat->outstanding_request = transport;
            MessageSinkTask(wechat->pfs.sink, &wechat->task_data);
            
            wechat->pfs_sequence = 0;
            wechat->pfs_state = PFS_PARTITION;
            payload = ++wechat->pfs_open_stream;            
            
            WECHAT_DEBUG(("wechat: pfs slack=%u\n", SinkSlack(wechat->pfs.sink)));
            status = WECHAT_STATUS_SUCCESS;
            send_success_payload(transport, WECHAT_COMMAND_OPEN_STORAGE_PARTITION, 1, &payload);
        }
        
        else
        {
            SetAudioBusy(NULL);
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    
    if (status != WECHAT_STATUS_SUCCESS)
    {
        send_simple_response(transport, WECHAT_COMMAND_OPEN_STORAGE_PARTITION, status);
    }
}


/*************************************************************************
NAME
    mount_storage_partition
    
DESCRIPTION
    Mount a storage partition
 
    The payload holds three octets corresponding to the three word arguments
    of PartitionMountFilesystem()
    
    +--------+--------+--------+
    | DEVICE |PARTIT'N|PRIORITY|
    +--------+--------+--------+
        0        1        2     
*/
static void mount_storage_partition(wechat_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status = WECHAT_STATUS_INVALID_PARAMETER;
    
    if (payload_length == 3)
    {
        switch (pfs_status(payload[0], payload[1]))
        {
        case PARTITION_UNMOUNTED:
            if (PartitionMountFilesystem(payload[0], payload[1], payload[2]))
            {
                send_success_payload(transport, WECHAT_COMMAND_MOUNT_STORAGE_PARTITION, 3, payload);
                status = WECHAT_STATUS_SUCCESS;
            }
            
            else
                status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
            
            break;
            
        case PARTITION_MOUNTED:
            status = WECHAT_STATUS_INCORRECT_STATE;
            break;
            
        case PARTITION_INVALID:
            break;
       }
    }
    
    if (status != WECHAT_STATUS_SUCCESS)
        send_simple_response(transport, WECHAT_COMMAND_MOUNT_STORAGE_PARTITION, status);
}


/*************************************************************************
NAME
    write_partition_data
    
DESCRIPTION
    Write data to partition
 
*/
static void write_partition_data(wechat_transport *transport, uint8 *payload, uint8 length)
{
    uint8 response[5];
    uint8 status = WECHAT_STATUS_SUCCESS;

    if ((length < 5) || (payload[0] != wechat->pfs_open_stream))
        status = WECHAT_STATUS_INVALID_PARAMETER;

    else if (wechat->pfs_state != PFS_PARTITION)
        status = WECHAT_STATUS_INCORRECT_STATE;

    else
    {
        uint32 sequence;

        sequence = payload[1];
        sequence = (sequence << 8) | payload[2];
        sequence = (sequence << 8) | payload[3];
        sequence = (sequence << 8) | payload[4];

        WECHAT_DEBUG(("wechat: write pfs %lu\n", sequence));

        if (sequence != wechat->pfs_sequence)
            status = WECHAT_STATUS_INVALID_PARAMETER;

        else
        {
            uint16 data_length = length - 5;
            
            response[0] = wechat->pfs_open_stream;
            dwunpack(response + 1, sequence);

            if (data_length)
            {
                if (SinkClaim(wechat->pfs.sink, data_length) == BAD_SINK_CLAIM)
                    status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;

                else
                {
                    memcpy(SinkMap(wechat->pfs.sink), payload + 5, data_length);
                    SinkFlush(wechat->pfs.sink, data_length);
                }
            }
        }
    }

    if (status == WECHAT_STATUS_SUCCESS)
    {
        send_success_payload(transport, WECHAT_COMMAND_WRITE_STORAGE_PARTITION, 5, response);
        ++wechat->pfs_sequence;
    }

    else
    {
        if (IsAudioBusy() == &wechat->task_data)
        {
            WECHAT_DEBUG(("wechat: write_partition_data - clear audio busy\n"));
            SetAudioBusy(NULL);
        }
        send_simple_response(transport, WECHAT_COMMAND_WRITE_STORAGE_PARTITION, status);
    }
}


/*************************************************************************
NAME
    close_storage_partition
    
DESCRIPTION
    Close the open storage partition
 
*/
static void close_storage_partition(wechat_transport *transport)
{
    uint8 status;
   
    WECHAT_DEBUG(("wechat: close pfs\n"));
    
    if (wechat->pfs_state == PFS_PARTITION)
    {
        if (SinkClose(wechat->pfs.sink))
            status = WECHAT_STATUS_SUCCESS;
        
        else
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
    }    
    else
        status = WECHAT_STATUS_INCORRECT_STATE;
        
    if (IsAudioBusy() == &wechat->task_data)
    {
        WECHAT_DEBUG(("wechat: close_storage_partition - clear audio busy\n"));
        SetAudioBusy(NULL);
    }
    
    if (status != WECHAT_STATUS_SUCCESS)
    {
        uint8 response = wechat->pfs_open_stream;
        send_ack(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_CLOSE_STORAGE_PARTITION, 
             status, 1, &response);
    }
    
    
}

#ifdef HAVE_DFU_FROM_SQIF
/*************************************************************************
NAME
    dfu_begin
    
DESCRIPTION
    Prepare a storage partition for DFU
 
*/
static void dfu_begin(wechat_transport *transport, uint8 *payload, uint8 payload_length)
{        
    uint8 status;
    bool ok = FALSE; 


    if (wechat->dfu_state != DFU_READY)
    {
        WECHAT_DEBUG(("wechat: dfu_begin: not ready\n"));
        status = WECHAT_STATUS_INCORRECT_STATE;
    }

    else
    {
        dfu_reset_timeout(0);

        if (IsAudioBusy())
        {
            WECHAT_DEBUG(("wechat: dfu_begin: audio busy\n"));
            status = WECHAT_STATUS_INCORRECT_STATE;
        }
    
        else if (payload_length != 8)
            status = WECHAT_STATUS_INVALID_PARAMETER;
    
        else if ((wechat->pfs_state != PFS_NONE) || (wechat->outstanding_request != NULL))
        /*  We support only one stream  */
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
    
        else
        {
            uint32 crc;
            uint32 size;
            uint32 space;
                        
            size = payload[0];
            size = (size << 8) | payload[1];
            size = (size << 8) | payload[2];
            size = (size << 8) | payload[3];
     
            if (PartitionGetInfo(PARTITION_SERIAL_FLASH, wechat->pfs_open_stream, PARTITION_INFO_SIZE, &space))
            {
            /*  Partition size is in words, image size is in bytes */
                space = 2 * (space - DFU_PARTITION_OVERHEAD);
            }
            
            else
            {
                space = 0;
            }
            
            
            if (size > space)
            {
                WECHAT_DEBUG(("wechat: dfu_begin: size %lu > space %lu\n", size, space));
                ok = FALSE;
            }
            
            else
            {
                SetAudioBusy(&wechat->task_data);
               
                crc = VmGetClock(); /* stack is precious */
                wechat->pfs.sink = StreamPartitionOverwriteSink(PARTITION_SERIAL_FLASH, wechat->pfs_open_stream);
                WECHAT_DEBUG(("wechat: dfu_begin %lu\n", VmGetClock() - crc));
        
                ok = wechat->pfs.sink != NULL;
            }
            
            if (ok)
            {
                WECHAT_DEBUG(("wechat: dfu_begin overwrite %u OK\n", wechat->pfs_open_stream));

                crc = payload[4];
                crc = (crc << 8) | payload[5];
                crc = (crc << 8) | payload[6];
                crc = (crc << 8) | payload[7];
                   
                wechat->pfs_raw_size = size;
    
                WECHAT_DEBUG(("wechat: dfu_begin: size=%lu space=%lu CRC=0x%08lX\n", size, space, crc));
                     
                ok = PartitionSetMessageDigest(wechat->pfs.sink, PARTITION_MESSAGE_DIGEST_CRC, (uint16 *) &crc, sizeof crc);
            }

            if (ok)
            {
                wechat->outstanding_request = transport;
                
                MessageSinkTask(wechat->pfs.sink, &wechat->task_data);
                
                wechat->pfs_state = PFS_DFU;
                dfu_reset_timeout(DFU_PACKET_TIMEOUT);
                
                WECHAT_DEBUG(("wechat: pfs slack=%u\n", SinkSlack(wechat->pfs.sink)));
                status = WECHAT_STATUS_SUCCESS;
            }
            
            else
            {
                WECHAT_DEBUG(("wechat: dfu_begin FAIL\n"));
                SetAudioBusy(NULL);
                status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }  
    
    send_simple_response(transport, WECHAT_COMMAND_DFU_BEGIN, status);

    if (ok)
    {
        wechat->dfu_state = DFU_DOWNLOAD;
        dfu_send_state(WECHAT_DFU_STATE_DOWNLOAD);
    }

    else
    {
        wechat->dfu_state = DFU_IDLE;
        dfu_indicate(WECHAT_DFU_STATE_DOWNLOAD_FAILURE);
    }
}
#endif /* def HAVE_DFU_FROM_SQIF */
#endif /* def HAVE_PARTITION_FILESYSTEM */

/*************************************************************************
NAME
    open_file
    
DESCRIPTION
    Prepare a file for reading
 
*/
static void open_file(wechat_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if ((payload_length < 2) || (payload[0] != WECHAT_PFS_MODE_READ))
        status = WECHAT_STATUS_INVALID_PARAMETER;
    
    else if (wechat->pfs_state != PFS_NONE)
    /*  We support only one stream  */
        status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;

    else
    {               
        FILE_INDEX file = FileFind(FILE_ROOT, (char *) (payload + 1), payload_length - 1);
        
        if (file == FILE_NONE)
            status = WECHAT_STATUS_INVALID_PARAMETER;
        
        else
        {
            wechat->pfs.source = StreamFileSource(file);
            if (wechat->pfs.source == NULL)
                status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
            
            else
            {
                uint8 payload;
                
                wechat->pfs_sequence = 0;
                wechat->pfs_state = PFS_FILE;
                payload = ++wechat->pfs_open_stream;            
                
                send_success_payload(transport, WECHAT_COMMAND_OPEN_FILE, 1, &payload);
                
                status = WECHAT_STATUS_SUCCESS;
            }
        }
    }
    
    if (status != WECHAT_STATUS_SUCCESS)
        send_simple_response(transport, WECHAT_COMMAND_OPEN_FILE, status);
}


/*************************************************************************
NAME
    read_file
    
DESCRIPTION
    Read and send a packetful of data from the open file

    |<----------- File packet header ----------->| <- File data -->|
    +--------+--------+--------+--------+--------+--------+--/ /---+
    | STREAM |              SEQUENCE             |  DATA     ...   |
    +--------+--------+--------+--------+--------+--------+--/ /---+
         0        1        2        3        4        5      ...
*/
static void read_file(wechat_transport *transport, uint8 *payload, uint8 payload_length)
{
    uint8 status;

    if ((payload_length != 5) || (payload[0] != wechat->pfs_open_stream))
        status = WECHAT_STATUS_INVALID_PARAMETER;

    else if (wechat->pfs_state != PFS_FILE)
        status = WECHAT_STATUS_INCORRECT_STATE;

    else 
    {
        uint32 sequence;
        uint8 *response;

        sequence = payload[1];
        sequence = (sequence << 8) | payload[2];
        sequence = (sequence << 8) | payload[3];
        sequence = (sequence << 8) | payload[4];

        WECHAT_DEBUG(("wechat: read file %lu\n", sequence));

        if (sequence != wechat->pfs_sequence)
            status = WECHAT_STATUS_INVALID_PARAMETER;

        else
        {
            response = malloc(FILE_PACKET_HEADER_SIZE + FILE_BUFFER_SIZE);
            
            if (response == NULL)
                status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
            
            else
            {
                uint16 length = SourceSize(wechat->pfs.source);
                
                if (length)                
                {
                    const uint8 *data = SourceMap(wechat->pfs.source);
                    
                    if (length > FILE_BUFFER_SIZE)
                        length = FILE_BUFFER_SIZE;
                    
                    memcpy(response + FILE_PACKET_HEADER_SIZE, data, length);
                    SourceDrop(wechat->pfs.source, length);
                }

                response[0] = wechat->pfs_open_stream;
                dwunpack(response + 1, sequence);
                send_success_payload(transport, WECHAT_COMMAND_READ_FILE, FILE_PACKET_HEADER_SIZE + length, response);
                
                free(response);
                ++wechat->pfs_sequence;
                    
                status = WECHAT_STATUS_SUCCESS;
            }
        }
    }
    
    if (status != WECHAT_STATUS_SUCCESS)
        send_simple_response(transport, WECHAT_COMMAND_READ_FILE, status);
}


/*************************************************************************
NAME
    close_file
    
DESCRIPTION
    Close the open data file
 
*/
static void close_file(wechat_transport *transport)
{
    uint8 response = wechat->pfs_open_stream;
    uint8 status = WECHAT_STATUS_SUCCESS;
   
    WECHAT_DEBUG(("wechat: close file\n"));
    
    if (wechat->pfs_state == PFS_FILE)
    {
        if (SourceClose(wechat->pfs.source))
            wechat->pfs_state = PFS_NONE;
            
        else
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
    }
    
    else
        status = WECHAT_STATUS_INCORRECT_STATE;
    
    send_ack(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_CLOSE_FILE, 
             status, 1, &response);
}


/*************************************************************************
NAME
    wechat_handle_data_transfer_command
    
DESCRIPTION
    Handle a Data Transfer command or return FALSE if we can't
 
*/
static bool wechat_handle_data_transfer_command(wechat_transport *transport,
    uint16 command_id, uint8 payload_length, uint8 *payload)
{
#ifdef HAVE_PARTITION_FILESYSTEM

    WECHAT_DEBUG(("wechat: PFS %u\n", wechat->have_pfs));

    if(wechat->have_pfs)
    {
        switch (command_id)
        {
        case WECHAT_COMMAND_GET_STORAGE_PARTITION_STATUS:
            if (payload_length == 2)
                send_pfs_status(transport, payload[0], payload[1]);

            else
                send_invalid_parameter(transport, command_id);

            return TRUE;


        case WECHAT_COMMAND_OPEN_STORAGE_PARTITION:
            open_storage_partition(transport, payload, payload_length);
            return TRUE;


        case WECHAT_COMMAND_MOUNT_STORAGE_PARTITION:
            mount_storage_partition(transport, payload, payload_length);
            return TRUE;
        

        case WECHAT_COMMAND_WRITE_STORAGE_PARTITION:
            write_partition_data(transport, payload, payload_length);
            return TRUE;


        case WECHAT_COMMAND_CLOSE_STORAGE_PARTITION:
            if ((payload_length == 1) && (payload[0] == wechat->pfs_open_stream))
                close_storage_partition(transport);
        
            else
                send_invalid_parameter(transport, command_id);
            
            return TRUE;


        case WECHAT_COMMAND_DFU_REQUEST | WECHAT_ACK_MASK:
            if (wechat->dfu_state == DFU_WAITING)
            {
                bool ok = (payload_length == 1) && (payload[0] == WECHAT_STATUS_SUCCESS);
                
                if (ok)
                    wechat->dfu_state = DFU_READY;
                    
                else
                {
                    dfu_reset_timeout(0);
                    wechat->dfu_state = DFU_IDLE;
                }

                    
                dfu_confirm(transport, ok);
            }
            return TRUE;

#ifdef HAVE_DFU_FROM_SQIF
        case WECHAT_COMMAND_DFU_BEGIN:
            dfu_begin(transport, payload, payload_length);
            return TRUE;
#endif
        }
    } 
#endif /* def HAVE_PARTITION_FILESYSTEM */
    
    switch (command_id)
    {
    case WECHAT_COMMAND_OPEN_FILE:
        open_file(transport, payload, payload_length);
        return TRUE;
    
        
    case WECHAT_COMMAND_READ_FILE:
        read_file(transport, payload, payload_length);
        return TRUE;

    
    case WECHAT_COMMAND_CLOSE_FILE:
        if ((payload_length == 1) && (payload[0] == wechat->pfs_open_stream))
            close_file(transport);
        
        else
            send_invalid_parameter(transport, command_id);

		return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    validate_payload_length
    
DESCRIPTION
    Check that the Wechat command payload is within the expected range.
    Returns TRUE if so, otherwise responds immediately with
    WECHAT_STATUS_INVALID_PARAMETER and returns FALSE
*/
static bool validate_payload_length(wechat_transport *transport, uint16 command_id, uint8 length, uint8 min, uint8 max)
{
    if ((length < min) || (length > max))
    {
        send_invalid_parameter(transport, command_id);
        return FALSE;
    }
    
    return TRUE;
}
    

/*************************************************************************
NAME
    ps_retrieve
    
DESCRIPTION
    Process Wechat Debugging Commands WECHAT_COMMAND_RETRIEVE_PS_KEY and
    WECHAT_COMMAND_RETRIEVE_FULL_PS_KEY
*/
static void ps_retrieve(wechat_transport *transport, uint16 command_id, uint16 key)
{
    uint16 key_length;
    
    if (command_id == WECHAT_COMMAND_RETRIEVE_FULL_PS_KEY)    
        key_length = PsFullRetrieve(key, NULL, 0);
    
    else
        key_length = PsRetrieve(key, NULL, 0);
    
    if (key_length == 0)
        send_invalid_parameter(transport, command_id);

    else
    {
        uint16 *key_data = malloc(key_length + 1);
        
        if (key_data)
        {
            if (command_id == WECHAT_COMMAND_RETRIEVE_FULL_PS_KEY)    
                PsFullRetrieve(key, key_data + 1, key_length);
    
            else
                PsRetrieve(key, key_data + 1, key_length);
           
            *key_data = key;
                
            send_ack_16(transport, WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_SUCCESS,
                                 key_length + 1, key_data);
            free(key_data);
        }

        else
            send_insufficient_resources(transport, command_id);
    }    
}


/*************************************************************************
NAME
    store_ps_key
    
DESCRIPTION
    Process Wechat Debugging Command WECHAT_COMMAND_STORE_PS_KEY
*/
static void store_ps_key(wechat_transport *transport, uint16 payload_length, uint8 *payload)
{
    uint16 status;
    
    if (((payload_length & 1) != 0) || (payload_length < 2))
        status = WECHAT_STATUS_INVALID_PARAMETER;
    
    else
    {
        uint16 key_index = W16(payload);
        uint16 *packed_key = malloc(payload_length / 2);
                   
        if (!packed_key)
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
        
        else
        {
            uint16 key_length = (payload_length - 2) / 2;
            uint16 ps_length;
            
            wpack(packed_key, payload + 2, key_length);
            
            ps_length = PsStore(key_index, packed_key, key_length);
            free(packed_key);
            
            if (key_length && !ps_length)
                status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
            
            else
                status = WECHAT_STATUS_SUCCESS;
        }    
    }
    
    send_simple_response(transport, WECHAT_COMMAND_STORE_PS_KEY, status);
}


/*************************************************************************
NAME
    get_memory_slots
    
DESCRIPTION
    Process Wechat Debugging Command WECHAT_COMMAND_GET_MEMORY_SLOTS
    Responds with the number of malloc() slots and the amount of
    writable storage available for PS keys
*/
static void get_memory_slots(wechat_transport *transport)
{
    uint16 malloc_slots = VmGetAvailableAllocations();
    uint16 ps_space = PsFreeCount(0);
    uint8 payload[4];
    
    payload[0] = HIGH(malloc_slots);
    payload[1] = LOW(malloc_slots);
    
    payload[2] = HIGH(ps_space);
    payload[3] = LOW(ps_space);
    
    send_success_payload(transport, WECHAT_COMMAND_GET_MEMORY_SLOTS, 4, payload);
}


/*************************************************************************
NAME
    send_kalimba_message
    
DESCRIPTION
    Send an arbitrary (short) message to the Kalimba DSP
*/
static void send_kalimba_message(wechat_transport *transport, uint8 *payload)
{
    if (KalimbaSendMessage(
        (payload[0] << 8) | payload[1],
        (payload[2] << 8) | payload[3],
        (payload[4] << 8) | payload[5],
        (payload[6] << 8) | payload[7],
        (payload[8] << 8) | payload[9]))
    {
    /*  Respond immediately -- we don't handle MESSAGE_FROM_KALIMBA here (yet)  */
        send_success(transport, WECHAT_COMMAND_SEND_KALIMBA_MESSAGE);
    }

    else
    {
        send_insufficient_resources(transport, WECHAT_COMMAND_SEND_KALIMBA_MESSAGE);
    }
}


/*************************************************************************
NAME
    wechat_handle_debug_command
    
DESCRIPTION
    Handle a debugging command or return FALSE if we can't
*/
static bool wechat_handle_debug_command(wechat_transport *transport, uint16 command_id, 
                                      uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case WECHAT_COMMAND_NO_OPERATION:
        send_success(transport, WECHAT_COMMAND_NO_OPERATION);
        return TRUE;
                        
    case WECHAT_COMMAND_RETRIEVE_PS_KEY:
    case WECHAT_COMMAND_RETRIEVE_FULL_PS_KEY:
        if (validate_payload_length(transport, command_id, payload_length, 2, 2))
            ps_retrieve(transport, command_id, W16(payload));
            
        return TRUE;
                
    case WECHAT_COMMAND_STORE_PS_KEY:
        store_ps_key(transport, payload_length, payload);
        return TRUE;
                
    case WECHAT_COMMAND_FLOOD_PS:
        PsFlood();
        send_success(transport, WECHAT_COMMAND_FLOOD_PS);
        return TRUE;
        
    case WECHAT_COMMAND_GET_MEMORY_SLOTS:
        get_memory_slots(transport);
        return TRUE;
     
    case WECHAT_COMMAND_DELETE_PDL:
        ConnectionSmDeleteAllAuthDevices(0);
        send_success(transport, WECHAT_COMMAND_DELETE_PDL);
        return TRUE;

    case WECHAT_COMMAND_SEND_APPLICATION_MESSAGE:
        if (validate_payload_length(transport, command_id, payload_length, 2, 2))
            MessageSend(wechat->app_task, W16(payload), NULL);
            
        return TRUE;
        
    case WECHAT_COMMAND_SEND_KALIMBA_MESSAGE:
        if (validate_payload_length(transport, command_id, payload_length, 10, 10))
            send_kalimba_message(transport, payload);

        return TRUE;
    }
    
    return FALSE;
}


/*************************************************************************
NAME
    wechat_handle_notification_command
    
DESCRIPTION
    Handle a debugging command or return FALSE if we can't
*/
static bool wechat_handle_notification_command(wechat_transport *transport, uint16 command_id, 
                                             uint8 payload_length, uint8 *payload)
{
    switch (command_id)
    {
    case WECHAT_COMMAND_REGISTER_NOTIFICATION:
        if (payload_length == 0)
            send_invalid_parameter(transport, WECHAT_COMMAND_REGISTER_NOTIFICATION);
        
        else if (WechatGetAppWillHandleNotification(payload[0]))
            return FALSE;
            
        else 
            register_notification(transport, payload_length, payload);
        
        return TRUE;
       
    case WECHAT_COMMAND_GET_NOTIFICATION:
        if (payload_length != 1)
            send_invalid_parameter(transport, WECHAT_COMMAND_GET_NOTIFICATION);
        
        else if (WechatGetAppWillHandleNotification(payload[0]))
            return FALSE;
            
        else 
            send_notification_setting(transport, payload[0]);
        
        return TRUE;

    case WECHAT_COMMAND_CANCEL_NOTIFICATION:
        if (payload_length != 1)
            send_invalid_parameter(transport, WECHAT_COMMAND_CANCEL_NOTIFICATION);
        
        else if (WechatGetAppWillHandleNotification(payload[0]))
            return FALSE;
            
        else 
            cancel_notification(transport, payload[0]);
        
        return TRUE;
                
    case WECHAT_ACK_NOTIFICATION:
        if ((payload_length > 0) && WechatGetAppWillHandleNotification(payload[0]))
            return FALSE;
                   
        return TRUE;
    }
    
    return FALSE;
}


/*************************************************************************
NAME
    check_enable
    
DESCRIPTION
    Validate SESSION_ENABLE credentials
*/
static bool check_enable(wechat_transport *transport, uint16 vendor_id, uint16 command_id, uint8 payload_length, uint8 *payload)
{
    uint16 usb_vid;
    bool ok;
    
    if (command_id & WECHAT_ACK_MASK)
        ok = TRUE;
        
    else if (vendor_id != WECHAT_VENDOR_CSR) 
        ok = FALSE;
        
    else if (command_id == WECHAT_COMMAND_GET_SESSION_ENABLE)
        ok = TRUE;
        
    else if ((command_id != WECHAT_COMMAND_SET_SESSION_ENABLE) ||
        (payload_length != 3))
        ok = FALSE;
        
    else if (PsFullRetrieve(PSKEY_USB_VENDOR_ID, &usb_vid, 1) == 0)
        ok = FALSE;
    
    else if (W16(payload + 1) != usb_vid)
        ok = FALSE;
              
    else
        ok = TRUE;
    
    WECHAT_DEBUG(("wechat: %04X:%04X en %u\n", vendor_id, command_id, ok));
    return ok;
}


/*************************************************************************
NAME
    process_packet
    
DESCRIPTION
    Analyse an inbound command packet and either do something based on
    the vendor and command identifiers or pass it up as unhandled
*/
void process_packet(wechat_transport *transport, uint8 *packet)
{
/*  0 bytes  1        2        3        4        5        6        7        8      len+8      len+9
 *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
 *  |  SOF   |VERSION | FLAGS  | LENGTH |    VENDOR ID    |   COMMAND ID    | | PAYLOAD   ...   | | CHECK  |
 *  +--------+--------+--------+--------+--------+--------+--------+--------+ +--------+--/ /---+ +--------+
 */
    uint8 protocol_version = packet[WECHAT_OFFS_VERSION];
    uint8 payload_length = packet[WECHAT_OFFS_PAYLOAD_LENGTH];
    uint16 vendor_id = W16(packet + WECHAT_OFFS_VENDOR_ID);
    uint16 command_id = W16(packet + WECHAT_OFFS_COMMAND_ID);
    uint8 *payload = packet + WECHAT_OFFS_PAYLOAD;
    
    if (wechat->rebooting)
        send_ack(transport, vendor_id, command_id, WECHAT_STATUS_INCORRECT_STATE, 0, NULL);

    else if (protocol_version == WECHAT_VERSION)
    {
        bool handled;
        
        if (!transport->enabled)
        {
            if (!check_enable(transport, vendor_id, command_id, payload_length, payload))
            {
                send_incorrect_state(transport, command_id);
                return;
            }
        }

        if (vendor_id == WECHAT_VENDOR_CSR)
            handled = !app_will_handle_command(command_id);
        
        else
            handled = FALSE;
        
        WECHAT_CMD_DEBUG(("wechat: <- %04x:%04x %c\n", vendor_id, command_id, handled ? 'L' : 'A'));
    
        if (handled)
        {
            switch (command_id & WECHAT_COMMAND_TYPE_MASK)
            {
            case WECHAT_COMMAND_TYPE_CONTROL:
                handled = wechat_handle_control_command(transport, command_id, payload_length, payload);
                break;
                
            case WECHAT_COMMAND_TYPE_STATUS:
                handled = wechat_handle_status_command(transport, command_id, payload_length, payload);
                break;
                
            case WECHAT_COMMAND_TYPE_FEATURE:
                handled = wechat_handle_feature_command(transport, command_id, payload_length, payload);
                break;
                
            case WECHAT_COMMAND_TYPE_DATA_TRANSFER:
                handled = wechat_handle_data_transfer_command(transport, command_id, payload_length, payload);
                break;
                
            case WECHAT_COMMAND_TYPE_DEBUG:
                handled = wechat_handle_debug_command(transport, command_id, payload_length, payload);
                break;

            case WECHAT_COMMAND_TYPE_NOTIFICATION:
                handled = wechat_handle_notification_command(transport, command_id, payload_length, payload);
                break;
                
            default:
                handled = FALSE;
                break;
            }   
        }
        
        if (!handled)
        {
        /*  Pass it on sans the start-of-frame and checksum bytes  */
            WECHAT_UNHANDLED_COMMAND_IND_T *gunk = malloc(sizeof (WECHAT_UNHANDLED_COMMAND_IND_T) + payload_length);
            
            WECHAT_CMD_DEBUG(("wechat: unhandled command\n"));
            
            if (gunk)
            {
                gunk->transport = (WECHAT_TRANSPORT *) transport;
                gunk->protocol_version = protocol_version;
                gunk->size_payload = payload_length;
                gunk->vendor_id = vendor_id;
                gunk->command_id = command_id;
                
                if (payload_length > 0)
                    memcpy(gunk->payload, payload, payload_length);
                
                MessageSend(wechat->app_task, WECHAT_UNHANDLED_COMMAND_IND, gunk);
            }
            
            else
                send_ack(transport, vendor_id, command_id, WECHAT_STATUS_INSUFFICIENT_RESOURCES, 0, NULL);
        }
    }
}


/*************************************************************************
NAME
    update_battery_voltage
    
DESCRIPTION
    Calculate the battery voltage from the ADC readings.  Use double
    exponential smoothing to keep the noise down
*/
static void update_battery_voltage(uint16 reading)
{
    if (wechat->battery_reference != 0)
    {
#ifdef WECHAT_BATTERY_NO_SMOOTHING
        wechat->battery_voltage = (((uint32) VmReadVrefConstant()) * reading) / wechat->battery_reference;
        WECHAT_THRES_DEBUG(("wechat: vbat %d\n", wechat->battery_voltage));
#else
        uint16 voltage = (((uint32) VmReadVrefConstant()) * reading) / wechat->battery_reference;
        
        if (wechat->battery_voltage == 0)
        {
            wechat->battery_voltage = voltage;
            wechat->battery_trend = 0;
        }
        
        else
        {
            int16 old = wechat->battery_voltage;
            wechat->battery_voltage = (9L * old + wechat->battery_trend + voltage + 5) / 10;
            wechat->battery_trend = (wechat->battery_trend + wechat->battery_voltage - old) / 2;
        }
            
        WECHAT_THRES_DEBUG(("wechat: vbat %d trend %d ave %d\n", 
            voltage, wechat->battery_trend, wechat->battery_voltage));
#endif
    }
}



/*************************************************************************
NAME
    check_battery_thresholds
    
DESCRIPTION
    Called periodically to see if the battery voltage has reached one of
    the notification thresholds
*/
static void check_battery_thresholds(wechat_transport *transport)
{
    if (wechat->battery_voltage != 0)
    {
        bool notify = FALSE;            
        
    /*  Check low thresholds  */
        if ((transport->threshold_count_lo_battery > 0) 
            && (wechat->battery_voltage <= transport->battery_lo_threshold[0]) 
            && !transport->sent_notification_lo_battery_0)
        {
            notify = TRUE;
            transport->sent_notification_lo_battery_0 = TRUE;
        }
    
        else if (transport->sent_notification_lo_battery_0 
            && (wechat->battery_voltage > transport->battery_lo_threshold[0] + WECHAT_BATT_HYSTERESIS))
            transport->sent_notification_lo_battery_0 = FALSE;
                                                 
        if ((transport->threshold_count_lo_battery > 1) 
            && (wechat->battery_voltage <= transport->battery_lo_threshold[1]) 
            && !transport->sent_notification_lo_battery_1)
        {
            notify = TRUE;
            transport->sent_notification_lo_battery_1 = TRUE;
        }
        
        else if (transport->sent_notification_lo_battery_1
            && (wechat->battery_voltage > transport->battery_lo_threshold[1] + WECHAT_BATT_HYSTERESIS))
            transport->sent_notification_lo_battery_1 = FALSE;
        
        WECHAT_THRES_DEBUG(("wechat: chk bat lo %d (%d %d) %d\n", 
               wechat->battery_voltage, 
               transport->battery_lo_threshold[0], 
               transport->battery_lo_threshold[1], 
               notify));
        
        if (notify)
        {
            uint8 payload[2];
            
            payload[0] = HIGH(wechat->battery_voltage);
            payload[1] = LOW(wechat->battery_voltage);
            
            send_notification(transport, WECHAT_EVENT_BATTERY_LOW_THRESHOLD, 2, payload);
        }
        
    /*  Check high thresholds  */
        notify = FALSE;
        
        if ((transport->threshold_count_hi_battery > 0) 
            && (wechat->battery_voltage >= transport->battery_hi_threshold[0]) 
            && !transport->sent_notification_hi_battery_0)
        {
            notify = TRUE;
            transport->sent_notification_hi_battery_0 = TRUE;
        }
    
        else if (transport->sent_notification_hi_battery_0 
            && (wechat->battery_voltage < transport->battery_hi_threshold[0] - WECHAT_BATT_HYSTERESIS))
            transport->sent_notification_hi_battery_0 = FALSE;
                                                 
        if ((transport->threshold_count_hi_battery > 1) 
            && (wechat->battery_voltage >= transport->battery_hi_threshold[1]) 
            && !transport->sent_notification_hi_battery_1)
        {
            notify = TRUE;
            transport->sent_notification_hi_battery_1 = TRUE;
        }
        
        else if (transport->sent_notification_hi_battery_1 
            && (wechat->battery_voltage < transport->battery_hi_threshold[1] - WECHAT_BATT_HYSTERESIS))
            transport->sent_notification_hi_battery_1 = FALSE;
        
        WECHAT_THRES_DEBUG(("wechat: chk batt hi %d (%d %d) %d\n", 
               wechat->battery_voltage, 
               transport->battery_hi_threshold[0], 
               transport->battery_hi_threshold[1], 
               notify));
        
        if (notify)
        {
            uint8 payload[2];
            
            payload[0] = HIGH(wechat->battery_voltage);
            payload[1] = LOW(wechat->battery_voltage);
            
            send_notification(transport, WECHAT_EVENT_BATTERY_HIGH_THRESHOLD, 2, payload);
        }
    }
}
                

/*************************************************************************
NAME
    check_rssi_thresholds
    
DESCRIPTION
    Called periodically to see if the RSSI has reached one of the
    notification thresholds
*/
static void check_rssi_thresholds(wechat_transport *transport)
{
    bool notify = FALSE;
    int16 rssi = 0;
    
    /* get the current RSSI */
    SinkGetRssi(wechatTransportGetSink(transport), &rssi);
    
    /*  Check low thresholds  */
    if ((transport->threshold_count_lo_rssi > 0) 
        && (rssi <= transport->rssi_lo_threshold[0]) 
        && !transport->sent_notification_lo_rssi_0)
    {
        notify = TRUE;
        transport->sent_notification_lo_rssi_0 = TRUE;
    }

    else if (transport->sent_notification_lo_rssi_0 && (rssi > (transport->rssi_lo_threshold[0] + WECHAT_RSSI_HYSTERESIS)))
        transport->sent_notification_lo_rssi_0 = FALSE;
                                             
    if ((transport->threshold_count_lo_rssi > 1) 
        && (rssi <= transport->rssi_lo_threshold[1]) 
        && !transport->sent_notification_lo_rssi_1)
    {
        notify = TRUE;
        transport->sent_notification_lo_rssi_1 = TRUE;
    }
    
    else if (transport->sent_notification_lo_rssi_1 && (rssi > (transport->rssi_lo_threshold[1] + WECHAT_RSSI_HYSTERESIS)))
        transport->sent_notification_lo_rssi_1 = FALSE;
    
    WECHAT_THRES_DEBUG(("wechat: chk RSSI lo %d (%d %d) %d\n", 
           rssi, 
           transport->rssi_lo_threshold[0], 
           transport->rssi_lo_threshold[1], 
           notify));
    
    if (notify)
    {
        uint8 payload = LOW(rssi);
        send_notification(transport, WECHAT_EVENT_RSSI_LOW_THRESHOLD, 1, &payload);
    }
    
    /*  Check high thresholds  */
    notify = FALSE;
    
    if ((transport->threshold_count_hi_rssi > 0) 
        && (rssi >= transport->rssi_hi_threshold[0]) 
        && !transport->sent_notification_hi_rssi_0)
    {
        notify = TRUE;
        transport->sent_notification_hi_rssi_0 = TRUE;
    }

    else if (transport->sent_notification_hi_rssi_0 && ((rssi < transport->rssi_hi_threshold[0] - WECHAT_RSSI_HYSTERESIS)))
        transport->sent_notification_hi_rssi_0 = FALSE;
                                             
    if ((transport->threshold_count_hi_rssi > 1) 
        && (rssi >= transport->rssi_hi_threshold[1]) 
        && !transport->sent_notification_hi_rssi_1)
    {
        notify = TRUE;
        transport->sent_notification_hi_rssi_1 = TRUE;
    }
    
    else if (transport->sent_notification_hi_rssi_1 && (rssi < (transport->rssi_hi_threshold[1] - WECHAT_RSSI_HYSTERESIS)))
        transport->sent_notification_hi_rssi_1 = FALSE;
    
    WECHAT_THRES_DEBUG(("wechat: chk RSSI hi %d (%d %d) %d\n", 
           rssi, 
           transport->rssi_hi_threshold[0], 
           transport->rssi_hi_threshold[1], 
           notify));
    
    if (notify)
    {
        uint8 payload = LOW(rssi);
        send_notification(transport, WECHAT_EVENT_RSSI_HIGH_THRESHOLD, 1, &payload);
    }
}


/*************************************************************************
NAME
    register_custom_sdp
    
DESCRIPTION
    Register a custom SDP record retrieved from PS
    
    The first word in the PS data is the length after unpacking.  This
    allows us to retrieve an odd number of bytes and allows some sanity
    checking.
*/
static void register_custom_sdp(uint16 pskey)
{
    uint16 ps_length;
    
    ps_length = PsFullRetrieve(pskey, NULL, 0);
    WECHAT_DEBUG(("wechat: m %04x sdp %d\n", pskey, ps_length));
    
    if (ps_length > 1)
    {
        uint16 sr_length = 2 * (ps_length - 1);
        uint16 *sr = malloc(sr_length);
                
        if (sr && PsFullRetrieve(pskey, sr, ps_length) && (sr_length - sr[0] < 2))
        {
        /*  Unpack into uint8s, preserving overlapping word  */
            uint16 idx;
            uint16 tmp = sr[1];

            sr_length = sr[0];
            for (idx = ps_length; idx > 1; )
            {
                --idx;
                sr[2 * idx - 1] = sr[idx] & 0xFF;
                sr[2 * idx - 2] = sr[idx] >> 8;
            }
            
            sr[0] = tmp >> 8;
            
            wechat->custom_sdp = TRUE;
            ConnectionRegisterServiceRecord(&wechat->task_data, sr_length, (uint8 *) sr);
        /*  NOTE: firmware will free the slot  */
        }
        
        else
        {
            free(sr);
            WECHAT_DEBUG(("wechat: bad sr\n"));
        }
    }
}


/*************************************************************************
NAME
    wechat_init
    
DESCRIPTION
    Initialise the library
*/
static void wechat_init(void)
{
    MESSAGE_PMAKE(status, WECHAT_INIT_CFM_T);

    /*  See lib_commands[] above; WECHAT_COMMAND_POWER_OFF (index 0) handled by app  */
    wechat->command_locus_bits = 0x00000001;
    
    /*  Default API minor version (may be overridden by WechatSetApiMinorVersion())  */
    wechat->api_minor = WECHAT_API_VERSION_MINOR;

    /*  App initially responsible for all but RSSI and Battery events  */
    wechat->event_locus_bits = ~((1 << WECHAT_EVENT_RSSI_LOW_THRESHOLD)
            | (1 << WECHAT_EVENT_RSSI_HIGH_THRESHOLD)
            | (1 << WECHAT_EVENT_BATTERY_LOW_THRESHOLD)
            | (1 << WECHAT_EVENT_BATTERY_HIGH_THRESHOLD));
             
    /*  So we can use AUDIO_BUSY interlocking  */
    AudioLibraryInit();

    /*  Perform custom SDP registration */
    register_custom_sdp(PSKEY_MOD_MANUF0);
            
    status->success = TRUE;
    MessageSend(wechat->app_task, WECHAT_INIT_CFM, status);
}


/*************************************************************************
NAME
    dfu_finish
    
DESCRIPTION
    Clean up after a DFU transfer
*/
static void dfu_finish(bool success)
{
    dfu_reset_timeout(0);

    wechat->pfs_raw_size = 0;
    SinkClose(wechat->pfs.sink);
    SetAudioBusy(NULL);

    if (success)
    {
        dfu_send_state(WECHAT_DFU_STATE_VERIFICATION);
    }

    else
    {
        wechat->pfs_state = PFS_NONE;
        dfu_send_state(WECHAT_DFU_STATE_DOWNLOAD_FAILURE);
    }
}


/*************************************************************************
NAME
    message_handler
    
DESCRIPTION
    Handles internal messages and anything from the underlying SPP
*/
static void message_handler(Task task, MessageId id, Message message)
{
/*  If SDP registration confirmation is caused by a custom record,
    don't bother the transport handlers with it
*/
    if ((id == CL_SDP_REGISTER_CFM) && wechat->custom_sdp)
    {
        WECHAT_DEBUG(("wechat: CL_SDP_REGISTER_CFM (C): %d\n", 
                    ((CL_SDP_REGISTER_CFM_T *) message)->status));
        
        wechat->custom_sdp = FALSE;
        return;
    }

	if(id == MESSAGE_MORE_DATA)
	{
		MessageMoreData *m = (MessageMoreData *) message;
		uint16 size = SourceSize(m->source);
		const uint8* data = SourceMap(m->source);
		uint16 i = 0;
		uint16 size_msg = sizeof(WECHAT_MESSAGE_MORE_DATA_T) + (size ? size - 1 : 0);
		WECHAT_MESSAGE_MORE_DATA_T* msg = (WECHAT_MESSAGE_MORE_DATA_T*)PanicUnlessMalloc(size_msg);

		msg->size_value = size;
		memmove(msg->value, data, size);
		
		MessageSend(wechat->app_task, WECHAT_MESSAGE_MORE_DATA, msg);

		for(; i<size; i++)
			printf("%x ", data[i]);
			
		printf("\n");
	}
    
	if (id == MESSAGE_MORE_DATA && wechat->pfs_raw_size)
	{
    /*  Data is to be copied directly to the PFS sink  */
		MessageMoreData *m = (MessageMoreData *) message;
		uint16 size = SourceSize(m->source);

		WECHAT_DFU_DEBUG(("wechat: raw %u of %lu\n", size, wechat->pfs_raw_size));

        if (size)
        {
            dfu_reset_timeout(DFU_PACKET_TIMEOUT);
            
		    if (size > wechat->pfs_raw_size)
            {
		        WECHAT_DEBUG(("wechat: DFU: too much data\n"));

                SourceDrop(m->source, size);
                dfu_finish(FALSE);
                return;
            }

            else
            {
                while (size)                
                {
                    uint16 chunk = (size > WECHAT_RAW_CHUNK_SIZE) ? WECHAT_RAW_CHUNK_SIZE : size;
                    
                    if (SinkClaim(wechat->pfs.sink, chunk) == BAD_SINK_CLAIM)
                    {
		                WECHAT_DEBUG(("wechat: DFU: bad sink %u\n", chunk));
                        SourceDrop(m->source, size);
                        dfu_finish(FALSE);
                        return;
                    }

                    else
                    {
                        memcpy(SinkMap(wechat->pfs.sink), SourceMap(m->source), chunk);
                        SinkFlush(wechat->pfs.sink, chunk);
                        SourceDrop(m->source, chunk);
                    }
    
                    size -= chunk;
                    wechat->pfs_raw_size -= chunk;
                }

                if (wechat->pfs_raw_size == 0)
                {
		            WECHAT_DEBUG(("wechat: DFU: transfer complete\n"));
                    dfu_finish(TRUE);
                }
            }
        }
        
		return;
	}

    /* see if a transport can handle this message */
    /* TODO handle multipoint case, loop through all transports? */
    if (wechatTransportHandleMessage(task, id, message))
        return;

    switch (id)
    {
    case MESSAGE_ADC_RESULT:
        {
            MessageAdcResult *m = (MessageAdcResult *) message;
            WECHAT_THRES_DEBUG(("wechat: adc %d = %d\n", m->adc_source, m->reading));
            switch (m->adc_source)
            {
            case adcsel_vref:
                wechat->battery_reference = m->reading;
                WECHAT_THRES_DEBUG(("wechat: vref %d\n", VmReadVrefConstant()));
                break;
                
            case adcsel_vdd_sense:
            case adcsel_vdd_bat:
            /*  wechat->battery_voltage = ((uint32) VmReadVrefConstant() * m->reading) / wechat->battery_reference;  */
                update_battery_voltage(m->reading);
                
                if (wechat->outstanding_request && (wechat->pfs_state == PFS_NONE))
                {
                    send_battery_level(wechat->outstanding_request);
                    wechat->outstanding_request = NULL;
                }
                break;
                
            default:
                break;
            }
        }
        break;
        
        
    case MESSAGE_STREAM_PARTITION_VERIFY:
        {
        /*  We have finished processing a WECHAT_COMMAND_CLOSE_STORAGE_PARTITION or DFU  */
            MessageStreamPartitionVerify *m = (MessageStreamPartitionVerify *) message;
            WECHAT_DEBUG(("wechat: pfs %u verify %u\n", m->partition, m->verify_result));
                       
            if (wechat->outstanding_request)
            {
                if ((wechat->pfs_state == PFS_PARTITION) || (wechat->pfs_state == PFS_FILE))
                {
                    uint8 status;
                    uint8 response = wechat->pfs_open_stream;

                    if (m->verify_result == PARTITION_VERIFY_PASS)
                        status = WECHAT_STATUS_SUCCESS;
                
                    else
                        status = WECHAT_STATUS_INVALID_PARAMETER;
                
                    send_ack(wechat->outstanding_request, WECHAT_VENDOR_CSR, WECHAT_COMMAND_CLOSE_STORAGE_PARTITION,
                         status, 1, &response);
                }

                else if (wechat->pfs_state == PFS_DFU)
                {
                    if (m->verify_result == PARTITION_VERIFY_PASS)
                    {
                        wechat->rebooting = TRUE;
                        dfu_send_state(WECHAT_DFU_STATE_VERIFICATION_SUCCESS);
                        MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_DFU_REQ, NULL, TATA_TIME);
                    }
                    
                    else
                    {
                        wechat->dfu_state = DFU_IDLE;
                        dfu_send_state(WECHAT_DFU_STATE_VERIFICATION_FAILURE);
                    }
                }

                wechat->outstanding_request = NULL;
            }
            
            wechat->pfs_state = PFS_NONE; 
        }
        break;
        
#ifdef WECHAT_TRANSPORT_SPP
    case SPP_CLIENT_CONNECT_CFM:
    case SPP_DISCONNECT_IND:
    case SPP_MESSAGE_MORE_DATA:
        wechatTransportSppHandleMessage(task, id, message);
        break;

    case SPP_MESSAGE_MORE_SPACE:
        break;
#endif
        
    case WECHAT_SEND_PACKET_CFM:
        {
            WECHAT_SEND_PACKET_CFM_T *m = (WECHAT_SEND_PACKET_CFM_T *) message;
            PRINT(("wechat: WECHAT_SEND_PACKET_CFM: s=%d\n", VmGetAvailableAllocations()));
            free(m->packet);
        }
        break;
        

    case WECHAT_INTERNAL_INIT:
        wechat_init();
        break;
        
        
    case WECHAT_INTERNAL_SEND_REQ:
        {
            WECHAT_INTERNAL_SEND_REQ_T *m = (WECHAT_INTERNAL_SEND_REQ_T *) message;
            wechatTransportSendPacket(m->task, m->transport, m->length, m->data);
        }
        break;
          
    case WECHAT_INTERNAL_BATTERY_MONITOR_TIMER:
        {
            if (wechat->battery_reference == 0)  /*  vref read failed; try again  */
                AdcRequest(&wechat->task_data, adcsel_vref);
            
            else
                read_battery();
            
            MessageSendLater(&wechat->task_data, WECHAT_INTERNAL_BATTERY_MONITOR_TIMER, NULL, WECHAT_CHECK_THRESHOLDS_PERIOD);
        }
        break;

    case WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ:
        {
            WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T *req = (WECHAT_INTERNAL_CHECK_BATTERY_THRESHOLD_REQ_T *)message;
            
            if (req->transport->connected)
            {
                /* if we have any battery thresholds set for this transport */
                if (req->transport->threshold_count_lo_battery || req->transport->threshold_count_hi_battery)
                {
                    /* check if a threshold has been reached, inform host, and restart the timer */
                    check_battery_thresholds(req->transport);
                    start_check_battery_threshold_timer(req->transport);
                }
            }
        }
        break;
        
    case WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ:
        {
            WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T *req = (WECHAT_INTERNAL_CHECK_RSSI_THRESHOLD_REQ_T *)message;
            if (req->transport->connected)
            {
                /* if we have any RSSI thresholds set for this transport */
                if (req->transport->threshold_count_lo_rssi || req->transport->threshold_count_hi_rssi)
                {
                    /* check if a threshold has been reached, inform host, and restart the timer */
                    check_rssi_thresholds(req->transport);
                    start_check_rssi_threshold_timer(req->transport);
                }
            }
        }
        break;
        
        
    case WECHAT_INTERNAL_REBOOT_REQ:
        BootSetMode(* (uint16 *) message);
        break;

        
    case WECHAT_INTERNAL_DFU_REQ:
        WECHAT_DEBUG(("wechat: LoaderPerformDfuFromSqif(%u)\n", wechat->pfs_open_stream));
#ifdef HAVE_DFU_FROM_SQIF
        LoaderPerformDfuFromSqif(wechat->pfs_open_stream);
#endif
        break;


    case WECHAT_INTERNAL_DISCONNECT_REQ:
        {
            WECHAT_INTERNAL_DISCONNECT_REQ_T *m = (WECHAT_INTERNAL_DISCONNECT_REQ_T *) message;
            wechatTransportDisconnectReq(m->transport);
        }
        break;
              
        
    case WECHAT_INTERNAL_POWER_OFF_REQ:
    /*  Try ALL for BC7, just SMPS0 for BC5  */
        if (!PsuConfigure(PSU_ALL, PSU_ENABLE, FALSE))
            PsuConfigure(PSU_SMPS0, PSU_ENABLE, FALSE);

        break;
        

    case WECHAT_INTERNAL_DFU_TIMEOUT:
        WECHAT_DFU_DEBUG(("wechat: dfu tmo in state %d\n", wechat->dfu_state));

        if (wechat->dfu_state == DFU_DOWNLOAD)
            dfu_finish(FALSE);
            	
        if (wechat->dfu_state == DFU_WAITING)
            dfu_confirm(wechat->outstanding_request, FALSE);
            
        else
            dfu_indicate(WECHAT_DFU_STATE_DOWNLOAD_FAILURE);
        
        wechat->dfu_state = DFU_IDLE;

        break;


    case CL_SM_ENCRYPTION_CHANGE_IND:
        break;


    default:
        WECHAT_DEBUG(("wechat: unh 0x%04X\n", id));
        break;
    }
}


/*************************************************************************
 *                                                                       *
 *  Public interface functions                                           *
 *                                                                       *
 *************************************************************************/

/*************************************************************************
NAME
    WechatInit
    
DESCRIPTION
    Initialise the Wechat protocol handler library
*/
void WechatInit(Task task, uint16 max_connections)
{ 
    /* size of buffer required for WECHAT_T + transport data */
    uint16 buf_size = sizeof (WECHAT_T) + (max_connections - 1) * sizeof (wechat_transport);   
    
    WECHAT_DEBUG(("wechat: WechatInit n=%d s=%d\n", max_connections, buf_size));
    
    if ((wechat == NULL) && 
    (max_connections > 0) && 
    (max_connections <= MAX_SUPPORTED_WECHAT_TRANSPORTS) &&
    (wechat = PanicUnlessMalloc(buf_size)))
    {
        /* initialise message handler */
        memset(wechat, 0, buf_size);
        wechat->task_data.handler = message_handler;
        wechat->app_task = task;
        
        wechat->transport_count = max_connections;
        
        MessageSend(&wechat->task_data, WECHAT_INTERNAL_INIT, NULL);
    }
    
    else
    {
        MESSAGE_PMAKE(status, WECHAT_INIT_CFM_T);
        status->success = FALSE;
        MessageSend(task, WECHAT_INIT_CFM, status);
    }
}



/*************************************************************************
NAME
    WechatBtConnectRequest
    
DESCRIPTION
    Request a connection to the given address
*/
void WechatBtConnectRequest(WECHAT_TRANSPORT *transport, bdaddr *address)
{
    WECHAT_DEBUG(("WechatBtConnectRequest\n"));
    
    if (wechat != NULL)
        wechatTransportConnectReq((wechat_transport *) transport, address);
}


/*************************************************************************
NAME
    WechatDisconnectRequest
    
DESCRIPTION
    Disconnect from host
*/
void WechatDisconnectRequest(WECHAT_TRANSPORT *transport)
{    
    if (wechat != NULL)
        wechatTransportDisconnectReq((wechat_transport *) transport);
}


/*************************************************************************
NAME
    WechatDisconnectResponse
    
DESCRIPTION
    Indicates that the client has processed a WECHAT_DISCONNECT_IND message
*/
void WechatDisconnectResponse(WECHAT_TRANSPORT *transport)
{
    wechatTransportDropState((wechat_transport *) transport);
}


/*************************************************************************
NAME
    WechatStartService
    
DESCRIPTION
    Start a service of the given type
*/
void WechatStartService(wechat_transport_type transport_type)
{
    if (wechat != NULL)
        wechatTransportStartService(transport_type);
}


/*************************************************************************
NAME
    WechatSendPacket
    
DESCRIPTION
    Send a Wechat packet over the indicated connection
*/
void WechatSendPacket(WECHAT_TRANSPORT *transport, uint16 packet_length, uint8 *packet)
{
    if (wechat != NULL)
        send_packet(wechat->app_task, (wechat_transport *) transport, packet_length, packet);
}


/*************************************************************************
NAME
    WechatGetAppWillHandleCommand
    
DESCRIPTION
    Return TRUE if the given command is to be passed to application code
    rather than being handled by the library
*/
bool WechatGetAppWillHandleCommand(uint16 command_id)
{
    uint16 idx = find_locus_bit(command_id);
    
    if ((idx == WECHAT_INVALID_ID) || (wechat == NULL))
        return FALSE;
    
    return (wechat->command_locus_bits & (1UL << idx)) != 0;
}


/*************************************************************************
NAME
    WechatSetApiMinorVersion
    
DESCRIPTION
    Changes the API Minor Version reported by WECHAT_COMMAND_GET_API_VERSION
    Returns TRUE on success, FALSE if the value is out of range (0..15) or
    the WECHAT storage is not allocated
*/
bool WechatSetApiMinorVersion(uint8 version)
{
    if (wechat == NULL || version > WECHAT_API_VERSION_MINOR_MAX)
        return FALSE;
    
    wechat->api_minor = version;
    return TRUE;
}


/*************************************************************************
NAME
    WechatSetAppWillHandleCommand
    
DESCRIPTION
    Request that the given command be passed to application code
    rather than being handled by the library.  Returns TRUE on success.
*/
bool WechatSetAppWillHandleCommand(uint16 command_id, bool value)
{
    uint16 idx;
    
    if (wechat == NULL)
        return TRUE;
    
    idx = find_locus_bit(command_id);
    
    if (idx != WECHAT_INVALID_ID)
    {
        if (value)
            wechat->command_locus_bits |= (1UL << idx);
        
        else
            wechat->command_locus_bits &= ~(1UL << idx);
            
        return TRUE;
    }

    return FALSE;
}


/*************************************************************************
NAME
    WechatGetAppWillHandleNotification
    
DESCRIPTION
    Return TRUE if the given event is to be notified by application code
    rather than being handled by the library.
*/
bool WechatGetAppWillHandleNotification(uint8 event_id)
{
    if ((event_id > WECHAT_IMP_MAX_EVENT_BIT) || (wechat == NULL))
        return FALSE;
    
    return (wechat->event_locus_bits & (1 << event_id)) != 0;
}


/*************************************************************************
NAME
    WechatSetAppWillHandleNotification
    
DESCRIPTION
    Request that the given event be raised by application code
    rather than by the library.  Returns TRUE on success.
*/
bool WechatSetAppWillHandleNotification(uint8 event_id, bool value)
{
    if ((event_id > WECHAT_IMP_MAX_EVENT_BIT) || (wechat == NULL))
        return FALSE;
    
    
    if (value)
        wechat->event_locus_bits |= (1 << event_id);
    
    else
        wechat->event_locus_bits &= ~(1 << event_id);
        
    return TRUE;
}


/*************************************************************************
NAME
    WechatBuildPacket
    
DESCRIPTION
    Build an arbitrary Wechat packet into the supplied buffer
*/
uint16 WechatBuildPacket(uint8 *buffer, uint8 flags,
                            uint16 vendor_id, uint16 command_id, 
                            uint8 size_payload, uint8 *payload)
{
    return build_packet(buffer, flags, vendor_id, command_id, 
                        WECHAT_STATUS_NONE, size_payload, payload);
}


/*************************************************************************
NAME
    WechatBuildPacket16
    
DESCRIPTION
    Build an arbitrary Wechat packet into the supplied buffer from a
    uint16[] payload
*/
uint16 WechatBuildPacket16(uint8 *buffer, uint8 flags, uint16 vendor_id, uint16 command_id, 
                         uint8 size_payload, uint16 *payload)
{
    return build_packet_16(buffer, flags, vendor_id, command_id, 
                        WECHAT_STATUS_NONE, size_payload, payload);
}


/*************************************************************************
NAME
    WechatBuildResponse
    
DESCRIPTION
    Build a Wechat acknowledgement packet into the supplied buffer
*/
uint16 WechatBuildResponse(uint8 *buffer, uint8 flags, 
                         uint16 vendor_id, uint16 command_id, 
                         uint8 status, uint8 size_payload, uint8 *payload)
{
    return build_packet(buffer, flags, vendor_id, command_id,
                        status, size_payload, payload);
}


/*************************************************************************
NAME
    WechatBuildResponse16
    
DESCRIPTION
    Build a Wechat acknowledgement packet into the supplied buffer from a
    uint16 payload
*/
uint16 WechatBuildResponse16(uint8 *buffer, uint8 flags, 
                            uint16 vendor_id, uint16 command_id, 
                            uint8 status, uint8 size_payload, uint16 *payload)
{
    return build_packet_16(buffer, flags, vendor_id, command_id,
                        status, size_payload, payload);
}


/*************************************************************************
NAME
    WechatBuildAndSendSynch
    
DESCRIPTION
    Build a Wechat packet in the transport sink and flush it
    The payload is an array of uint8s; contrast WechatBuildAndSendSynch16()
    The function completes synchronously and no confirmation message is
    sent to the calling task
*/
void WechatBuildAndSendSynch(WECHAT_TRANSPORT *transport, 
                           uint16 vendor_id, uint16 command_id, uint8 status, 
                           uint8 size_payload, uint8 *payload)
{
    Sink sink = wechatTransportGetSink((wechat_transport *) transport);
    uint16 packet_length = WECHAT_OFFS_PAYLOAD + size_payload;
    uint8 flags = ((wechat_transport *)transport)->flags;
    
    if (wechat == NULL || sink == NULL)
        return;

    if (status != WECHAT_STATUS_NONE)
        ++packet_length;

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    if (packet_length > WECHAT_MAX_PACKET)
        return;

    WECHAT_TRANS_DEBUG(("wechat: bss %d\n", packet_length));

    if (SinkClaim(sink, packet_length) == BAD_SINK_CLAIM)
    {
        WECHAT_TRANS_DEBUG(("wechat: no sink\n"));
        return;
    }
    
    build_packet(SinkMap(sink), flags, vendor_id, command_id,
                        status, size_payload, payload);
    
#ifdef DEBUG_WECHAT_TRANSPORT
    {
        uint16 idx;
        uint8 *data = SinkMap(sink);
        WECHAT_DEBUG(("wechat: put"));
        for (idx = 0; idx < packet_length; ++idx)
            WECHAT_DEBUG((" %02x", data[idx]));
        WECHAT_DEBUG(("\n"));
    }
#endif

    SinkFlush(sink, packet_length);
}


/*************************************************************************
NAME
    WechatBuildAndSendSynch16
    
DESCRIPTION
    Build a Wechat packet in the transport sink and flush it
    The payload is an array of uint16s; contrast WechatBuildAndSendSynch()
    The function completes synchronously and no confirmation message is
    sent to the calling task
*/
void WechatBuildAndSendSynch16(WECHAT_TRANSPORT *transport, 
                             uint16 vendor_id, uint16 command_id, uint8 status, 
                             uint16 size_payload, uint16 *payload)
{
    Sink sink = wechatTransportGetSink((wechat_transport *) transport);
    uint16 packet_length = WECHAT_OFFS_PAYLOAD + 2 * size_payload;
    uint8 flags = ((wechat_transport *)transport)->flags;

    if (wechat == NULL || sink == NULL)
        return;

    if (status != WECHAT_STATUS_NONE)
        ++packet_length;

    if (flags & WECHAT_PROTOCOL_FLAG_CHECK)
        ++packet_length;

    WECHAT_TRANS_DEBUG(("wechat: bss16 %d\n", packet_length));
    
    if (packet_length > WECHAT_MAX_PACKET)
        return;

    if (SinkClaim(sink, packet_length) == BAD_SINK_CLAIM)
    {
        WECHAT_TRANS_DEBUG(("wechat: no sink\n"));
        return;
    }
    
    build_packet_16(SinkMap(sink), flags, vendor_id, command_id,
                        status, size_payload, payload);
    
    SinkFlush(sink, packet_length);
}


/*************************************************************************
NAME
    WechatTransportGetFlags
    
DESCRIPTION
    Returns the control flags for the given transport instance
*/
uint8 WechatTransportGetFlags(WECHAT_TRANSPORT *transport)
{
    if (transport)
        return ((wechat_transport *) transport)->flags;

    return 0;
}


/*************************************************************************
NAME
    WechatTransportGetType
    
DESCRIPTION
    Returns the transport type for the given transport instance
*/
wechat_transport_type WechatTransportGetType(WECHAT_TRANSPORT *transport)
{
   if (transport)
       return ((wechat_transport *) transport)->type;

   return wechat_transport_none;
}


/*************************************************************************
NAME
    WechatTransportSetFlags
    
DESCRIPTION
    Sets the control flags for the given transport instance
*/
void WechatTransportSetFlags(WECHAT_TRANSPORT *transport, uint8 flags)
{
    ((wechat_transport *) transport)->flags = flags;
}


/*************************************************************************
NAME
    WechatGetSessionEnable
    
DESCRIPTION
    Returns TRUE if WECHAT session is enabled for the given transport instance
*/
bool WechatGetSessionEnable(WECHAT_TRANSPORT *transport)
{
    return ((wechat_transport *) transport)->enabled;
}


/*************************************************************************
NAME
    WechatSetSessionEnable
    
DESCRIPTION
    Enables or disables WECHAT session for the given transport instance
*/
void WechatSetSessionEnable(WECHAT_TRANSPORT *transport, bool enable)
{
    ((wechat_transport *) transport)->enabled = enable;
}


/*************************************************************************
NAME
    WechatDfuRequest
    
DESCRIPTION
    Enables and Requests Device Firmware Upgrade

    Does some sanity checks:
    o The <partition> is of the correct type
    o The <period> is reasonable
    o Wechat is initialised
    o A DFU is not already pending or in progress
    o No other Partition Filesystem operation is in progress
    o Exactly one transport is connected

*/
void WechatDfuRequest(WECHAT_TRANSPORT *transport, uint16 partition, uint16 period)
{
#ifdef HAVE_PARTITION_FILESYSTEM
    uint32 pfs_type = 0xFF;
    uint16 idx;
    bool ok = TRUE;
   
    ok = PartitionGetInfo(PARTITION_SERIAL_FLASH, partition, PARTITION_INFO_TYPE, &pfs_type);
	
    WECHAT_DEBUG(("wechat: dfu request p=%u s=%u t=%lu\n", partition, ok, pfs_type));
	
    if ((wechat == NULL) ||
        (transport == NULL) ||
        (wechat->dfu_state != DFU_IDLE) || 
        (wechat->pfs_state != PFS_NONE) ||
        (partition > PFS_MAX_PARTITION) ||
        (period < 1) || 
        (period > DFU_MAX_ENABLE_PERIOD) ||
        (pfs_type != PARTITION_TYPE_RAW_SERIAL))
        ok = FALSE;
	
    if (ok)
    {
    /*  Make sure there is only one transport  */
        for (idx = 0; ok && idx < wechat->transport_count; ++idx)
        {
            if ((wechat->transport[idx].type != wechat_transport_none) &&
                ((WECHAT_TRANSPORT *) &wechat->transport[idx] != transport))
                ok = FALSE;
        }
    }

    if (ok)
    {
        wechat->dfu_state = DFU_WAITING;
        wechat->pfs_open_stream = partition;
        WechatBuildAndSendSynch(transport, WECHAT_VENDOR_CSR, WECHAT_COMMAND_DFU_REQUEST, WECHAT_STATUS_NONE, 0, NULL);
        dfu_reset_timeout(period);
    }

    else
#endif
       dfu_confirm((wechat_transport *) transport, FALSE);
}

