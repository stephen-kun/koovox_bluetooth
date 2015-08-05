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
#include <vm.h>
#include <memory.h>
#include <led.h>
#include <csr_voice_prompts_plugin.h>
#include <boot.h>
#include "sink_config.h"
#include "sink_configmanager.h"
#include "sink_leds.h"
#include "sink_led_manager.h"
#include "sink_tones.h"
#include "sink_audio_prompts.h"
#include "sink_buttons.h"
#include "sink_volume.h"
#include "sink_speech_recognition.h"
#include "sink_device_id.h"
#include "sink_statemanager.h"

#include "koovox_wechat_handle.h"

#include <kalimba.h>

#define DSP_WECHAT_MSG_SET_USER_PARAM                (0x121a)
#define DSP_WECHAT_MSG_GET_USER_PARAM                (0x129a)
#define DSP_WECHAT_MSG_SET_USER_GROUP_PARAM          (0x121b)
#define DSP_WECHAT_MSG_GET_USER_GROUP_PARAM          (0x129b)

/*
#define DSP_WECHAT_MSG_SET_USER_PARAM_RESP           (0x321a)
#define DSP_WECHAT_MSG_GET_USER_PARAM_RESP           (0x329a)
#define DSP_WECHAT_MSG_SET_USER_GROUP_PARAM_RESP     (0x321b)
#define DSP_WECHAT_MSG_GET_USER_GROUP_PARAM_RESP     (0x329b)
*/


/*  Wechat-global data stored in app-allocated structure */
#define wechat_data theSink.rundata->wechat_data

#ifdef CVC_PRODTEST
    #define CVC_PRODTEST_PASS           0x0001
    #define CVC_PRODTEST_FAIL           0x0002
#endif


/*************************************************************************
NAME    
    wechat_send_packet
    
DESCRIPTION
    Build and Send a Wechat protocol packet
   
*/ 
static void wechat_send_packet(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload)
{
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = WechatTransportGetFlags(wechat_data.wechat_transport);
    
    packet_length = WECHAT_HEADER_SIZE + payload_length + 2;
    packet = mallocPanic(packet_length);
    
    if (packet)
    {
        packet_length = WechatBuildResponse(packet, flags,
                                          vendor_id, command_id, 
                                          status, payload_length, payload);
        
        WechatSendPacket(wechat_data.wechat_transport, packet_length, packet);
    }
}


/*************************************************************************
NAME    
    wechat_send_response
    
DESCRIPTION
    Build and Send a Wechat acknowledgement packet
   
*/ 
void wechat_send_response(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload)
{
    wechat_send_packet(vendor_id, command_id | WECHAT_ACK_MASK, status,
                     payload_length, payload);
}


/*************************************************************************
NAME    
    wechat_send_response_16
    
DESCRIPTION
    Build and Send a Wechat acknowledgement packet from a uint16[] payload
   
*/ 
void wechat_send_response_16(uint16 command_id, uint16 status,
                          uint16 payload_length, uint16 *payload)
{
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = WechatTransportGetFlags(wechat_data.wechat_transport);
    
    packet_length = WECHAT_HEADER_SIZE + 2 * payload_length + 2;
    packet = mallocPanic(packet_length);
    
    if (packet)
    {
        packet_length = WechatBuildResponse16(packet, flags,
                                          WECHAT_VENDOR_CSR, command_id | WECHAT_ACK_MASK, 
                                          status, payload_length, payload);
        
        WechatSendPacket(wechat_data.wechat_transport, packet_length, packet);
    }
}


/*************************************************************************
NAME
    wechat_send_success
    wechat_send_success_payload
    wechat_send_invalid_parameter
    wechat_send_insufficient_resources
    wechat_send_incorrect_state
    
DESCRIPTION
    Convenience macros for common responses
*/
#define wechat_send_success(command_id) \
    wechat_send_response(WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_SUCCESS, 0, NULL)
                
#define wechat_send_success_payload(command_id, payload_len, payload) \
    wechat_send_response(WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_SUCCESS, payload_len, (uint8 *) payload)
                
#define wechat_send_invalid_parameter(command_id) \
    wechat_send_response(WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_INVALID_PARAMETER, 0, NULL)
    
#define wechat_send_insufficient_resources(command_id) \
    wechat_send_response(WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_INSUFFICIENT_RESOURCES, 0, NULL)

#define wechat_send_incorrect_state(command_id) \
    wechat_send_response(WECHAT_VENDOR_CSR, command_id, WECHAT_STATUS_INCORRECT_STATE, 0, NULL)


#ifdef DEBUG_WECHAT
static void dump(char *caption, uint16 *data, uint16 data_len)
{
    WECHAT_DEBUG(("%s: %d:", caption, data_len));
    while (data_len--)
        WECHAT_DEBUG((" %04x", *data++));
    WECHAT_DEBUG(("\n"));
}
#endif
    
    
/*************************************************************************
NAME    
    wechat_send_notification
    
DESCRIPTION
    Send a Wechat notification packet
   
*/ 
static void wechat_send_notification(uint8 event, uint16 payload_length, uint8 *payload)
{
    wechat_send_packet(WECHAT_VENDOR_CSR, WECHAT_EVENT_NOTIFICATION, event, payload_length, payload);
}


/*************************************************************************
NAME
    wpack
    
DESCRIPTION
    Pack an array of 2n uint8s into an array of n uint16s
*/
static void wpack(uint16 *dest, uint8 *src, uint16 n)
{
    while (n--)
    {
        *dest = *src++ << 8;
        *dest++ |= *src++;
    }
}

    
/*************************************************************************
NAME
    dwunpack
    
DESCRIPTION
    Unpack a uint32 into an array of 4 uint8s
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
    pitch_length_tone
    
DESCRIPTION
    Pack a pitch + length pair into a ringtone note
*/
static ringtone_note pitch_length_tone(uint8 pitch, uint8 length)
{
    return (ringtone_note) 
            ((pitch << RINGTONE_SEQ_NOTE_PITCH_POS) 
            | (length << RINGTONE_SEQ_NOTE_LENGTH_POS));
}


/*************************************************************************
NAME
    get_config_lengths
    
DESCRIPTION
    Load the lengths structure
*/
static void get_config_lengths(lengths_config_type *lengths)
{
    ConfigRetrieve(CONFIG_LENGTHS, lengths, sizeof (lengths_config_type));
}


/*************************************************************************
NAME
    send_app_message
    
DESCRIPTION
    Send a message to the main application
*/
static void send_app_message(uint16 message)
{
    MessageSend(&theSink.task, message, NULL);
}


/*************************************************************************
NAME
    set_abs_eq_bank
    
DESCRIPTION
    Select a Music Manager equaliser bank by absolute index
*/
static void set_abs_eq_bank(uint16 bank)
{
    bank += A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0;
    
    WECHAT_DEBUG(("G: EQ %u -> %u\n",
                theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_processing,
                bank));
    
    if(bank != theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_processing)
    {
        AUDIO_MODE_T mode = VolumeCheckA2dpMute() ? AUDIO_MODE_MUTE_SPEAKER : AUDIO_MODE_CONNECTED;
        usbAudioGetMode(&mode);
        
        theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_processing = bank;
        configManagerWriteSessionData() ; 
        AudioSetMode(mode, &theSink.a2dp_link_data->a2dp_audio_mode_params);
#ifdef ENABLE_PEER
        peerSendAudioEnhancements();
#endif
    }
}


/*************************************************************************
NAME    
    wechat_change_volume
    
DESCRIPTION
    Respond to WECHAT_COMMAND_CHANGE_VOLUME request by sending the
    appropriate event message to the sink device task.  The device inverts
    the message meanings if it thinks the buttons are inverted so we have
    to double bluff it.
    
    direction 0 means volume up, 1 means down.
*/    
static void wechat_change_volume(uint8 direction)
{
    uint16 message = 0;
    uint8 status;
    
    if (theSink.gVolButtonsInverted)
        direction ^= 1;
    
    if (direction == 0)
    {
        message = EventUsrVolumeUp;
        status = WECHAT_STATUS_SUCCESS;
    }
    
    else if (direction == 1)
    {
        message = EventUsrVolumeDown;
        status = WECHAT_STATUS_SUCCESS;
    }
    
    else
        status = WECHAT_STATUS_INVALID_PARAMETER;
    
    if (status == WECHAT_STATUS_SUCCESS)
        send_app_message(message);
    
    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_CHANGE_VOLUME, status, 0, NULL);  
}


/*************************************************************************
NAME    
    wechat_alert_leds
    
DESCRIPTION
    Act on a WECHAT_COMMAND_ALERT_LEDS request

*/    
static void wechat_alert_leds(uint8 *payload)
{
    uint16 time;
    LEDPattern_t *pattern;
    
    uint16 idx = 0;
    
    while ((idx < theSink.theLEDTask->gEventPatternsAllocated) 
        && ((theSink.theLEDTask->pEventPatterns[idx]).StateOrEvent != EventSysGaiaAlertLEDs))
        ++idx;
    
    WECHAT_DEBUG(("G: al: %d/%d\n", idx, theSink.theLEDTask->gEventPatternsAllocated));

    if (idx == theSink.theLEDTask->gEventPatternsAllocated)
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_LEDS);
        return;
    }
    
    pattern = &theSink.theLEDTask->pEventPatterns[idx];

    time = (payload[0] << 8) | payload[1];
    pattern->OnTime = (time >> theSink.features.LedTimeMultiplier) / 10;
    
    time = (payload[2] << 8) | payload[3];
    pattern->OffTime = (time >> theSink.features.LedTimeMultiplier) / 10;
    
    time = (payload[4] << 8) | payload[5];
    pattern->RepeatTime = (time >> theSink.features.LedTimeMultiplier) / 50;
    
    pattern->DimTime = payload[7];
    pattern->NumFlashes = payload[8];
    pattern->TimeOut = payload[9];
    pattern->LED_A = payload[10];
    pattern->LED_B = payload[11];
    pattern->OverideDisable = FALSE;
    pattern->Colour = payload[12];

    LEDManagerIndicateEvent(EventSysGaiaAlertLEDs);
    wechat_send_success(WECHAT_COMMAND_ALERT_LEDS);    
}


/*************************************************************************
NAME    
    wechat_alert_tone
    
DESCRIPTION
    Act on a WECHAT_COMMAND_ALERT_TONE request
    
    The command payload holds tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and duration values for each note.
    
NOTE
    Since we can't be sure when the tone will start playing, let alone
    when it will finish, we can't free the storage.
*/    
static void wechat_alert_tone(uint8 length, uint8 *tones)
{

/*  Must be at least 4 octets and an even number in total  */
    if ((length < 4) || (length & 1))
        wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_TONE);
    
/*  If there are no notes then we're done  */
    else if (length == 4)
        wechat_send_success(WECHAT_COMMAND_ALERT_TONE);
        
    else
    {
        if (wechat_data.alert_tone == NULL)
            wechat_data.alert_tone = mallocPanic(WECHAT_TONE_BUFFER_SIZE);
        
        if ((wechat_data.alert_tone == NULL) || (length > WECHAT_TONE_MAX_LENGTH))
            wechat_send_insufficient_resources(WECHAT_COMMAND_ALERT_TONE);
        
        else
        {
            uint16 idx;
            uint16 volume;
            ringtone_note *t = wechat_data.alert_tone;
    
            WECHAT_DEBUG(("G: at: %d %d %d %d\n", tones[0], tones[1], tones[2], tones[3]));
            
            AudioStopTone();
            
            
        /*  Set up tempo, volume, timbre and decay  */
            *t++ = RINGTONE_TEMPO(tones[0] * 4);
            volume = tones[1];
            *t++ = (ringtone_note) (RINGTONE_SEQ_TIMBRE | tones[2]);
            *t++ = RINGTONE_DECAY(tones[3]);
                
            
        /*  Set up note-duration pairs  */
            for (idx = 4; idx < length; idx += 2)
            {
                WECHAT_DEBUG((" - %d %d\n", tones[idx], tones[idx + 1]));
                *t++ = pitch_length_tone(tones[idx], tones[idx + 1]);
            }
                
        /*  Mark the end  */
            *t = RINGTONE_END;
            
            AudioPlayTone(wechat_data.alert_tone, FALSE, theSink.codec_task, volume, theSink.conf2->audio_routing_data.PluginFeatures);   
            wechat_send_success(WECHAT_COMMAND_ALERT_TONE);    
        }
    }
}


/*************************************************************************
NAME    
    wechat_alert_event
    
DESCRIPTION
    Act on a WECHAT_COMMAND_ALERT_EVENT request by indicating the associated
    event

*/    
static void wechat_alert_event(uint16 event)
{
    WECHAT_DEBUG(("G: ae: %04X\n", event));
    
    if (is_valid_event_id(event))
    {
        if (event != EventSysLEDEventComplete)
            LEDManagerIndicateEvent(event);
    
        TonesPlayEvent(event);
        
        wechat_send_success(WECHAT_COMMAND_ALERT_EVENT);
    }
    
    else
        wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_EVENT);
}


/*************************************************************************
NAME    
    wechat_alert_voice
    
DESCRIPTION
    Act on a WECHAT_COMMAND_ALERT_VOICE request
    Play the indicated voice prompt
    We have already established that theSink.num_audio_prompt_languages != 0

*/    
static void wechat_alert_voice(uint8 payload_length, uint8 *payload)
{
    lengths_config_type lengths;
    TaskData *plugin = NULL;
    uint16 nr_prompts;
    uint16 ap_id = (payload[0] << 8) | payload[1];
 
    get_config_lengths(&lengths);

    nr_prompts = lengths.num_audio_prompts / theSink.num_audio_prompt_languages;
    
    WECHAT_DEBUG(("G: ti: %d/%d + %d\n", ap_id, nr_prompts, payload_length - 2));

    if (ap_id < nr_prompts)
    { 
        plugin = (TaskData *) &csr_voice_prompts_plugin;
   
        AudioPromptPlay(plugin, ap_id, TRUE, FALSE);
        wechat_send_success(WECHAT_COMMAND_ALERT_VOICE);
    }
    
    else
        wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_VOICE);        
}


/*************************************************************************
NAME
    wechat_set_led_config
    
DESCRIPTION
    Respond to WECHAT_COMMAND_SET_LED_CONFIGURATION request
    
    State and Event config requests are 17 octets long and look like
    +--------+-----------------+-----------------+-----------------+-----------------+
    |  TYPE  |   STATE/EVENT   |   LED ON TIME   |    OFF TIME     |   REPEAT TIME   |
    +--------+--------+--------+--------+--------+--------+--------+--------+--------+ ...
    0        1        2        3        4        5        6        7        8
        +-----------------+--------+--------+--------+--------+--------+--------+
        |    DIM TIME     |FLASHES |TIMEOUT | LED_A  | LED_B  |OVERRIDE| COLOUR |
    ... +--------+--------+--------+--------+--------+--------+DISABLE-+--------+
        9       10       11       12       13       14       15       16
    
    Filter config requests are 6 to 8 octets depending on the filter action and look like
    +--------+--------+-----------------+--------+--------+--------+--------+
    |  TYPE  | INDEX  |      EVENT      | ACTION |OVERRIDE| PARAM1 |[PARAM2]|
    +--------+--------+--------+--------+--------+DISABLE-+--------+--------+ 
    0        1        2        3        4        5        6       [7]
*/    
static void wechat_set_led_config(uint16 request_length, uint8 *request)
{
    lengths_config_type lengths;
    uint16 no_entries;
    uint16 max_entries;
    uint16 config_key;
    uint16 config_length;
    uint16 index;
    uint16 event;
    uint8 expect_length;
    uint8 type = request[0];
    
    get_config_lengths(&lengths);
    
    switch (type)
    {
    case WECHAT_LED_CONFIGURATION_FILTER:        
        no_entries = lengths.no_led_filter;
        max_entries = MAX_LED_FILTERS;
        config_key = CONFIG_LED_FILTERS;
        
        switch (request[3])
        {
        case WECHAT_LED_FILTER_OVERRIDE:
            expect_length = 6;
            break;
            
        case WECHAT_LED_FILTER_CANCEL:        
        case WECHAT_LED_FILTER_COLOUR:
        case WECHAT_LED_FILTER_FOLLOW:
            expect_length = 7;
            break;
            
        case WECHAT_LED_FILTER_SPEED:
            expect_length = 8;
            break;
            
        default:
            expect_length = 0;
            break;
        }
        
        break;

    case WECHAT_LED_CONFIGURATION_STATE:
        no_entries = lengths.no_led_states;
        max_entries = MAX_LED_STATES;
        config_key = CONFIG_LED_STATES;
        expect_length = 17;
        break;

    case WECHAT_LED_CONFIGURATION_EVENT:
        no_entries = lengths.no_led_events;
        max_entries = MAX_LED_EVENTS;
        config_key = CONFIG_LED_EVENTS;
        expect_length = 17;     
        break;
        
    default:
        no_entries = 0;
        max_entries = 0;
        config_key = 0;
        expect_length = 0;
        break;
    }
    
    WECHAT_DEBUG(("G: lc: t=%d id=%d n=%d k=%d r=%d x=%d\n", 
                 type, request[2], no_entries, config_key, request_length, expect_length));
            
    if (request_length != expect_length)
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_LED_CONFIGURATION);
        return;
    }
   
            
    if (type == WECHAT_LED_CONFIGURATION_FILTER)
    {
        LEDFilter_t *config = NULL;
        bool changed = FALSE;
    
        index = request[1] - 1;
        event = (request[2] << 8) | request[3];

        if ((index > no_entries) || !is_valid_event_id(event))
        {
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_LED_CONFIGURATION);
            return;
        }
            
        if (index == no_entries)
            config_length = (no_entries + 1) * sizeof (LEDFilter_t);
        
        else
            config_length = no_entries * sizeof (LEDFilter_t);
        
        config = mallocPanic(config_length);
        if (config == NULL)
        {
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_LED_CONFIGURATION);
            return;
        }
            
        ConfigRetrieve(config_key, config, config_length);
        
        changed = (index == no_entries) || (config[index].Event != event);
        
        switch (request[4])
        {
        case WECHAT_LED_FILTER_CANCEL:
            if (changed || (config[index].FilterType != CANCEL) || (config[index].FilterToCancel != request[6]))
            {
                memset(&config[index], 0, sizeof (LEDFilter_t));
                config[index].FilterType = CANCEL;
                config[index].FilterToCancel = request[6];
                changed = TRUE;
            }
            break;
            
        case WECHAT_LED_FILTER_SPEED:
            if (changed 
                ||(config[index].FilterType != SPEED)
                || (config[index].SpeedAction != request[6])
                || (config[index].Speed != request[7]))
            {
                memset(&config[index], 0, sizeof (LEDFilter_t));
                config[index].FilterType = SPEED;
                config[index].SpeedAction = request[6];
                config[index].Speed = request[7];
                changed = TRUE;
            }
            break;
            
        case WECHAT_LED_FILTER_OVERRIDE:
            if (changed || (config[index].FilterType != OVERRIDE))
            {
                memset(&config[index], 0, sizeof (LEDFilter_t));
                config[index].FilterType = OVERRIDE;
                changed = TRUE;
            }
            break;
            
        case WECHAT_LED_FILTER_COLOUR:
            if (changed ||(config[index].FilterType != COLOUR) || (config[index].Colour != request[6]))
            {
                memset(&config[index], 0, sizeof (LEDFilter_t));
                config[index].FilterType = COLOUR;
                config[index].Colour = request[6];
                changed = TRUE;
            }
            break;
            
        case WECHAT_LED_FILTER_FOLLOW:
            if (changed 
                || (config[index].FilterType != FOLLOW)
                || (config[index].FollowerLEDDelay != request[6]))
            {
                memset(&config[index], 0, sizeof (LEDFilter_t));
                config[index].FilterType = FOLLOW;
                config[index].FollowerLEDDelay = request[6];
                changed = TRUE;
            }
            break;
            
        default:
            expect_length = 0;
            break;
        }
        
        if (changed)
        {
            config[index].Event = event;
                                
            config_length = ConfigStore(config_key, config, config_length);
            
            if ((config_length > 0) && (no_entries != lengths.no_led_filter))
            {
                lengths.no_led_filter = no_entries;
                config_length = ConfigStore(CONFIG_LENGTHS, &lengths, sizeof lengths);
                if (config_length == 0)
                {
                    WECHAT_DEBUG(("WECHAT: ConfigStore lengths failed\n"));
                    Panic();
                }
            }
        }
                    
        freePanic(config);
    }
    
    else
    {
        LEDPattern_t *config = NULL;
        bool changed = FALSE;
        uint16 on_time = 0;
        uint16 off_time = 0;
        uint16 repeat_time = 0;
         
        event = (request[1] << 8) | request[2];
        if (type == WECHAT_LED_CONFIGURATION_EVENT && !is_valid_event_id(event))
        {
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_LED_CONFIGURATION);
            return;
        }
   
        config_length = no_entries * sizeof (LEDPattern_t);        
        config = mallocPanic(config_length + sizeof (LEDPattern_t));
        if (config == NULL)
        {
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_LED_CONFIGURATION);
            return;
        }
            
        ConfigRetrieve(config_key, config, config_length);

        index = 0;
        while ((index < no_entries) && (config[index].StateOrEvent != event))
            ++index;
                    
        WECHAT_DEBUG(("G: idx %d/%d/%d\n", index, no_entries, max_entries));
        
        if (index == max_entries)
        {
            freePanic(config);
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_LED_CONFIGURATION);
            return;
        }
        
        if (index == no_entries)
        {
            WECHAT_DEBUG(("G: + %d %d at %d\n", type, event, index));
            changed = TRUE;
            ++no_entries;
            config_length += sizeof (LEDPattern_t);
        }
        
        else
            WECHAT_DEBUG(("G: = %d %d at %d\n", type, event, index));

        
        on_time = ((uint16) (request[3] << 8) | request[4]) / 10;
        off_time = ((uint16) (request[5] << 8) | request[6]) / 10;
        repeat_time = ((uint16) (request[7] << 8) | request[8]) / 50;
    
        if (changed
            || config[index].OnTime != on_time
            || config[index].OffTime != off_time
            || config[index].RepeatTime != repeat_time
            || config[index].DimTime != request[10]
            || config[index].NumFlashes != request[11]
            || config[index].TimeOut != request[12]
            || config[index].LED_A != request[13]
            || config[index].LED_B != request[14]
            || config[index].OverideDisable != request[15]
            || config[index].Colour != request[16])
        {
            memset(&config[index], 0, sizeof (LEDPattern_t));
            
            config[index].StateOrEvent = event;
            config[index].OnTime = on_time;
            config[index].OffTime = off_time;
            config[index].RepeatTime = repeat_time;
            config[index].DimTime = request[10];
            config[index].NumFlashes = request[11];
            config[index].TimeOut = request[12];
            config[index].LED_A = request[13];
            config[index].LED_B = request[14];
            config[index].OverideDisable = request[15];
            config[index].Colour = request[16];
            
            config_length = ConfigStore(config_key, config, config_length);
            if (config_length > 0)
            {
                if (type == WECHAT_LED_CONFIGURATION_STATE)
                {
                    changed = no_entries != lengths.no_led_states;
                    lengths.no_led_states = no_entries;
                }
                
                else
                {
                    changed = no_entries != lengths.no_led_events;
                    lengths.no_led_events = no_entries;
                }
                
                if (changed)
                {
                    config_length = ConfigStore(CONFIG_LENGTHS, &lengths, sizeof lengths);
                    if (config_length == 0)
                    {
                        WECHAT_DEBUG(("WECHAT: ConfigStore lengths failed\n"));
                        Panic();
                    }
                }
            }
        }
                    
        freePanic(config);
    }
    
    if (config_length == 0)
        wechat_send_insufficient_resources(WECHAT_COMMAND_SET_LED_CONFIGURATION);

    else if (type == WECHAT_LED_CONFIGURATION_FILTER)
    {
        uint8 payload[4];
        payload[0] = type;          /* type */
        payload[1] = request[1];    /* index  */
        payload[2] = event >> 8;    /* event */
        payload[3] = event & 0xFF;
        wechat_send_success_payload(WECHAT_COMMAND_SET_LED_CONFIGURATION, 4, payload);
    }
    
    else
    {
        uint8 payload[3];
        payload[0] = type;          /* type */
        payload[1] = event >> 8;    /* event */
        payload[2] = event & 0xFF;
        wechat_send_success_payload(WECHAT_COMMAND_SET_LED_CONFIGURATION, 3, payload);
    }
}

    
/*************************************************************************
NAME
    wechat_send_led_config
    
DESCRIPTION
    Respond to WECHAT_COMMAND_GET_LED_CONFIGURATION request for the
    configuration of a given state, event or filter
*/       
#define filter_config ((LEDFilter_t *)config)
#define pattern_config ((LEDPattern_t *)config)
static void wechat_send_led_config(uint8 type, uint16 index)
{
    lengths_config_type lengths;
    void *config;
    uint16 config_length;
    uint16 key;
    uint16 no_entries;
    uint16 time;          
    uint16 payload_len;
    uint8 payload[17];
    
    
    get_config_lengths(&lengths);
    
    switch (type)
    {
    case WECHAT_LED_CONFIGURATION_STATE:
        no_entries = lengths.no_led_states;
        config_length = no_entries * sizeof (LEDPattern_t);
        key = CONFIG_LED_STATES;
        break;
        
    case WECHAT_LED_CONFIGURATION_EVENT:
        no_entries = lengths.no_led_events;
        config_length = no_entries * sizeof (LEDPattern_t);
        key = CONFIG_LED_EVENTS;
        break;
        
    case WECHAT_LED_CONFIGURATION_FILTER:
        no_entries = lengths.no_led_filter;
        config_length = no_entries * sizeof (LEDFilter_t);
        key = CONFIG_LED_FILTERS;
        break;
        
    default:
        no_entries = 0;
        config_length = 0;
        key = 0;
        break;
    }
    
    if (no_entries == 0)
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_GET_LED_CONFIGURATION);
        return;
    }
        
    config = mallocPanic(config_length);
    if (config == NULL)
    {
        wechat_send_insufficient_resources(WECHAT_COMMAND_GET_LED_CONFIGURATION);
        return;
    }
    
    ConfigRetrieve(key, config, config_length);
    
    payload[0] = type;
    payload[1] = index >> 8;
    payload[2] = index & 0xFF;
            
    if (type == WECHAT_LED_CONFIGURATION_FILTER)
    {
    /*  Filter numbers are 1-based  */
        if ((index < 1) || (index > no_entries))
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_LED_CONFIGURATION);
    
        else
        {
            --index;

            payload[3] = filter_config[index].Event >> 8;
            payload[4] = filter_config[index].Event & 0xFF;
            payload[6] = filter_config[index].OverideDisable;
            
            switch (filter_config[index].FilterType)
            {
            case CANCEL:
                payload[5] = WECHAT_LED_FILTER_CANCEL;
                payload[7] = filter_config[index].FilterToCancel;
                payload_len = 8;
                break;
                
            case SPEED:
                payload[5] = WECHAT_LED_FILTER_SPEED;
                payload[7] = filter_config[index].SpeedAction;
                payload[8] = filter_config[index].Speed;
                payload_len = 9;
                break;
                
            case OVERRIDE:
                payload[5] = WECHAT_LED_FILTER_OVERRIDE;                
                payload_len = 7;
                break;
            
            case COLOUR:
                payload[5] = WECHAT_LED_FILTER_COLOUR;
                payload[7] = filter_config[index].Colour;
                payload_len = 8;
                break;
            
            case FOLLOW:
                payload[5] = WECHAT_LED_FILTER_FOLLOW;
                payload[7] = filter_config[index].FollowerLEDDelay;               
                payload_len = 8;
                break;
            
            default:
            /*  Can't infer the filter action  */
                payload_len = 0;
                break;
            }
            
            
            if (payload_len == 0)
                wechat_send_insufficient_resources(WECHAT_COMMAND_GET_LED_CONFIGURATION);
            
            else
                wechat_send_success_payload(WECHAT_COMMAND_GET_LED_CONFIGURATION, payload_len, payload);
        }
    }
    
    else
    {
        uint16 search = 0;
        
        while ((search < no_entries) && (pattern_config[search].StateOrEvent != index))
            ++search;
        
        if (search == no_entries)
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_LED_CONFIGURATION);
        
        else
        {
            WECHAT_DEBUG(("G: e: %d %d at %d\n", type, search, index));
            time = pattern_config[search].OnTime * 10;
            payload[3] = time >> 8;
            payload[4] = time & 0xFF;
            
            time = pattern_config[search].OffTime * 10;
            payload[5] = time >> 8;
            payload[6] = time & 0xFF;
            
            time = pattern_config[search].RepeatTime * 50;
            payload[7] = time >> 8;
            payload[8] = time & 0xFF;
            
            payload[9] = 0;
            payload[10] = pattern_config[search].DimTime;
            
            payload[11] = pattern_config[search].NumFlashes;
            payload[12] = pattern_config[search].TimeOut;
            payload[13] = pattern_config[search].LED_A;
            payload[14] = pattern_config[search].LED_B;
            payload[15] = pattern_config[search].OverideDisable;
            payload[16] = pattern_config[search].Colour;            
            
            payload_len = 17;
            wechat_send_success_payload(WECHAT_COMMAND_GET_LED_CONFIGURATION, payload_len, payload);
        }
    }
        
    freePanic(config);
}
#undef pattern_config
#undef filter_config


/*************************************************************************
NAME
    wechat_retrieve_tone_config
    
DESCRIPTION
    Get the tone configuration from config or const
    Returns the number of tones configured, 0 on failure
*/       
static uint16 wechat_retrieve_tone_config(tone_config_type **config)
{
    lengths_config_type lengths;
    uint16 config_length;
    
    get_config_lengths(&lengths);
    
    WECHAT_DEBUG(("G: rtc: %d tones\n", lengths.no_tones));
    
    config_length = lengths.no_tones * sizeof(tone_config_type);
    
    *config = mallocPanic(config_length);
    
    if (*config == NULL)
    {
        WECHAT_DEBUG(("G: rtc: no space\n"));
        return 0;
    }
    
    ConfigRetrieve(CONFIG_TONES, *config, config_length);
        
    return lengths.no_tones;
}
    
    
/*************************************************************************
NAME
    wechat_set_tone_config
    
DESCRIPTION
    Set the tone associated with a given event.  We update the stored
    configuration, not the running device.
*/   
static void wechat_set_tone_config(uint16 event, uint8 tone)
{
    lengths_config_type lengths;
    uint16 no_tones;

    if (!is_valid_event_id(event))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_TONE_CONFIGURATION);
        return;
    }
    
    get_config_lengths(&lengths);
    
    no_tones = lengths.no_tones;    
    WECHAT_DEBUG(("G: stc: %d tones\n", no_tones));
    
    if (no_tones == 0)
        wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TONE_CONFIGURATION);
    
    else
    {
        tone_config_type *config;
        uint16 config_length;
        
        config_length = no_tones * sizeof (tone_config_type);

    /*  Include space for a new event in case we add one  */
        config = mallocPanic(config_length + sizeof (tone_config_type));
        
        if (config == NULL)
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TONE_CONFIGURATION);

        else
        {
            uint16 index = 0;
            
            ConfigRetrieve(CONFIG_TONES, config, config_length);
            config[no_tones].tone = TONE_NOT_DEFINED;

            while ((index < no_tones) && (config[index].event != event))
                ++index;
           
            if (index == no_tones)
            {
                ++no_tones;
                config_length += sizeof (tone_config_type);
            }
            
            config[index].event = event;
            
            if (config[index].tone != tone)
            {
                config[index].tone = tone;        
                config_length = ConfigStore(CONFIG_TONES, config, config_length);
            }
            
            if (config_length == 0)
                wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TONE_CONFIGURATION);
                
            else
            {
                if (no_tones != lengths.no_tones)
                {
                    lengths.no_tones = no_tones;
                    config_length = ConfigStore(CONFIG_LENGTHS, &lengths, sizeof lengths);
                }
            
                if (config_length == 0)
                {
                    WECHAT_DEBUG(("WECHAT: ConfigStore lengths failed\n"));
                    Panic();
                }
                
                else
                {
                    uint8 payload[3];
                    
                    payload[0] = event >> 8;
                    payload[1] = event & 0xFF;
                    payload[2] = tone;
                    wechat_send_success_payload(WECHAT_COMMAND_SET_TONE_CONFIGURATION, sizeof payload, payload);
                }
            }
            
            freePanic(config);
        }
    }
}
    
    
/*************************************************************************
NAME
    wechat_send_tone_config
    
DESCRIPTION
    Respond to WECHAT_COMMAND_GET_TONE_CONFIGURATION request for a single
    event-tone pair
*/       
static void wechat_send_tone_config(uint16 event)
{
    tone_config_type *config;
    uint16 no_tones;
    uint16 index = 0;
    
    if (!is_valid_event_id(event))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_GET_TONE_CONFIGURATION);
        return;
    }
    
    no_tones = wechat_retrieve_tone_config(&config);
    
    if (no_tones == 0)
        wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TONE_CONFIGURATION);
        
    else
    {
        while ((index < no_tones) && (config[index].event != event))
            ++index;
        
        if (index == no_tones)
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_TONE_CONFIGURATION);
        
        else
        {
            uint8 payload[3];
            payload[0] = config[index].event >> 8;
            payload[1] = config[index].event & 0xFF;
            payload[2] = config[index].tone;
            wechat_send_success_payload(WECHAT_COMMAND_GET_TONE_CONFIGURATION, sizeof payload, payload);
        }
    }
    
    if (config != NULL)
        freePanic(config);
}
    

/*************************************************************************
NAME
    wechat_retrieve_feature_block
    
DESCRIPTION
    Return a pointer to malloc'd feature block, NULL on failure
*/
static feature_config_type *wechat_retrieve_feature_block(void)
{
    feature_config_type *feature_block = mallocPanic(sizeof (feature_config_type));
    
    if (feature_block != NULL)
    {
        ConfigRetrieve(CONFIG_FEATURE_BLOCK, feature_block, sizeof (feature_config_type)); 
    }

    return feature_block;
}


/*************************************************************************
NAME
    wechat_send_default_volumes
    
DESCRIPTION
    Respond to WECHAT_COMMAND_GET_DEFAULT_VOLUME
    Three octets after the status represent tones, speech and music volumes
*/
static void wechat_send_default_volumes(void)
{
    feature_config_type *feature_block = wechat_retrieve_feature_block();
    
    if (feature_block == NULL)
        wechat_send_insufficient_resources(WECHAT_COMMAND_GET_DEFAULT_VOLUME);
    
    else
    {
        uint8 payload[3];
        
        payload[0] = feature_block->FixedToneVolumeLevel;
        payload[1] = feature_block->DefaultVolume;
        payload[2] = feature_block->DefaultA2dpVolLevel;
        wechat_send_success_payload(WECHAT_COMMAND_GET_DEFAULT_VOLUME, 3, payload);
        
        freePanic(feature_block);
    }
}


/*************************************************************************
NAME
    wechat_set_default_volumes
    
DESCRIPTION
    Respond to WECHAT_COMMAND_SET_DEFAULT_VOLUME
    The three payload octets represent tones, speech and music volumes
*/
static void wechat_set_default_volumes(uint8 volumes[])
{
    if ((volumes[0] > 22)  /* should be def or const for MAX_SPEAKER_GAIN_INDEX */
        || (volumes[1] >= VOL_NUM_VOL_SETTINGS)
        || (volumes[2] >= VOL_NUM_VOL_SETTINGS))
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_DEFAULT_VOLUME);
   
    else
    {
        feature_config_type *feature_block = wechat_retrieve_feature_block();
        
        if (feature_block == NULL)
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_DEFAULT_VOLUME);
        
        else
        {
            feature_block->FixedToneVolumeLevel = volumes[0];
            feature_block->DefaultVolume = volumes[1];
            feature_block->DefaultA2dpVolLevel = volumes[2];
        
            if (ConfigStore(CONFIG_FEATURE_BLOCK, feature_block, sizeof (feature_config_type)) > 0)
                wechat_send_success(WECHAT_COMMAND_SET_DEFAULT_VOLUME);
        
            else
                wechat_send_insufficient_resources(WECHAT_COMMAND_SET_DEFAULT_VOLUME);
        
            freePanic(feature_block);
        }
    }
}


/*************************************************************************
NAME
    wechat_set_ap_config
    
DESCRIPTION
    Set the Audio Prompts configuration requested by a
    WECHAT_COMMAND_SET_VOICE_PROMPT_CONFIGURATION command
*/
static void wechat_set_ap_config(uint8 *payload)
{
    lengths_config_type lengths;
    audio_prompts_config_type *config;
    uint16 config_len;
    uint16 num_ap_events;
    uint16 event = (payload[0] << 8) | payload[1];
    uint8 status = WECHAT_STATUS_SUCCESS;
    
    if (!is_valid_event_id(event))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_VOICE_PROMPT_CONFIGURATION);
        return;
    }

    get_config_lengths(&lengths);
    
    WECHAT_DEBUG(("G: AP: %d %02X\n", lengths.num_audio_prompt_events, event));
    
    num_ap_events = lengths.num_audio_prompt_events;
    config_len = num_ap_events * sizeof (audio_prompts_config_type);
    config = mallocPanic(config_len + sizeof (audio_prompts_config_type));
    
    if (config == NULL)
        status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
    
    else
    {
        uint16 idx = 0;
        
        ConfigRetrieve(CONFIG_AUDIO_PROMPTS, config, config_len);
        
        while ((idx < lengths.num_audio_prompt_events) && (config[idx].event != event))
            ++idx;
    
        if (idx == lengths.num_audio_prompt_events)
        {
            ++num_ap_events;
            config_len += sizeof (audio_prompts_config_type);
        }
            
        config[idx].event = event;
        config[idx].prompt_id = payload[2];
        config[idx].sco_block = payload[3];
        config[idx].state_mask = (payload[4] << 8) | payload[5];
        
        if (ConfigStore(CONFIG_AUDIO_PROMPTS, config, config_len) == 0)
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
        
        else if (num_ap_events > lengths.num_audio_prompt_events)
        {
            lengths.num_audio_prompt_events = num_ap_events;
            if (ConfigStore(CONFIG_LENGTHS, &lengths, sizeof lengths) == 0)
            {
                WECHAT_DEBUG(("WECHAT: ConfigStore lengths failed\n"));
                Panic();
            }
        }
        
        freePanic(config);
    }

    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_SET_VOICE_PROMPT_CONFIGURATION, 
                     status, 0, NULL);
}


/*************************************************************************
NAME
    wechat_send_ap_config
    
DESCRIPTION
    Send the Audio Prompt Event configuration requested by a
    WECHAT_COMMAND_GET_VOICE_PROMPT_CONFIGURATION command. The configuration
    for an event is sent in six octets representing
        o  event, upper 8 bits
        o  event, lower 8 bits
        o  prompt_id (the audio prompt number)
        o  sco_block (TRUE meaning 'do not play if SCO is present')
        o  state_mask, upper 6 bits (states in which prompt will play)
        o  state_mask, lower 8 bits
*/
static void wechat_send_ap_config(uint16 event)
{
    lengths_config_type lengths;
    uint16 payload_len = 0;
    uint8 payload[6];
    uint8 status;
    
    if (!is_valid_event_id(event))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_GET_VOICE_PROMPT_CONFIGURATION);
        return;
    }
    
    get_config_lengths(&lengths);
    
    WECHAT_DEBUG(("G: AP: %d %02X\n", lengths.num_audio_prompt_events, event));
    
    if (lengths.num_audio_prompt_events == 0)
        status = WECHAT_STATUS_INVALID_PARAMETER;
    
    else
    {
        uint16 config_len = lengths.num_audio_prompt_events * sizeof (audio_prompts_config_type);
        audio_prompts_config_type *config = mallocPanic(config_len);
        
        if (config == NULL)
            status = WECHAT_STATUS_INSUFFICIENT_RESOURCES;
        
        else
        {
            uint16 idx = 0;
            
            ConfigRetrieve(CONFIG_AUDIO_PROMPTS, config, config_len);
            
            while ((idx < lengths.num_audio_prompt_events) && (config[idx].event != event))
                ++idx;
                        
            if (idx == lengths.num_audio_prompt_events)
                status = WECHAT_STATUS_INVALID_PARAMETER;
            
            else
            {
                payload[0] = event >> 8;
                payload[1] = event & 0xFF;
                payload[2] = config[idx].prompt_id;
                payload[3] = config[idx].sco_block;
                payload[4] = config[idx].state_mask >> 8;
                payload[5] = config[idx].state_mask & 0xFF;
                
                payload_len = 6;
                status = WECHAT_STATUS_SUCCESS;
            }
            
            freePanic(config);
        }
    }
    
    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_GET_VOICE_PROMPT_CONFIGURATION, 
                     status, payload_len, payload);
}


/*************************************************************************
NAME
    wechat_set_timer_config
    
DESCRIPTION
    Handle WECHAT_COMMAND_SET_TIMER_CONFIGURATION
    For simplicity we just pack the payload into a Timeouts_t structure,
    which works so long as nobody minds EncryptionRefreshTimeout_m being
    in minutes
*/
static void wechat_set_timer_config(uint8 payload_len, uint8 *payload)
{
    uint16 config_length = sizeof (Timeouts_t);
    
    if (payload_len != (2 * config_length))
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_TIMER_CONFIGURATION);
    
    else
    {
        uint16 *config = mallocPanic(config_length);
        
        if (config == NULL)
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TIMER_CONFIGURATION);
        
        else
        {
            wpack(config, payload, config_length);
            
            if (ConfigStore(CONFIG_TIMEOUTS, config, config_length))
                wechat_send_success(WECHAT_COMMAND_SET_TIMER_CONFIGURATION);
            
            else
                wechat_send_insufficient_resources(WECHAT_COMMAND_SET_TIMER_CONFIGURATION);
            
            freePanic(config);
        }
    }
}


/*************************************************************************
NAME
    wechat_send_timer_config
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_TIMER_CONFIGURATION
    For simplicity we just bundle up the Timeouts_t structure, which works
    so long as nobody minds EncryptionRefreshTimeout_m being in minutes
*/
static void wechat_send_timer_config(void)
{
    uint16 config_length = sizeof (Timeouts_t);
    Timeouts_t *timeouts = mallocPanic(config_length);
    
    if (timeouts == NULL)
        wechat_send_insufficient_resources(WECHAT_COMMAND_GET_TIMER_CONFIGURATION);
    
    else
    {
        ConfigRetrieve(CONFIG_TIMEOUTS, 
                       timeouts, config_length);
        
        wechat_send_response_16(WECHAT_COMMAND_GET_TIMER_CONFIGURATION, 
                               WECHAT_STATUS_SUCCESS, config_length, (uint16 *) timeouts);
        freePanic(timeouts);
    }
}


/*************************************************************************
NAME
    wechat_set_audio_gain_config
    
DESCRIPTION
    Handle WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION
    The command payload contains five octets for each volume mapping,
    corresponding to IncVol, DecVol, Tone, A2dpGain and VolGain
*/
static void wechat_set_audio_gain_config(uint8 payload_len, uint8 *payload)
{
    if (payload_len != (VOL_NUM_VOL_SETTINGS * 5))
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);
    
    else
    {
        uint16 config_len = sizeof (volume_configuration_t);
        volume_configuration_t *config = mallocPanic(config_len);
        
        if (config == NULL)
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);
            
        else
        {
            const uint8 max_vol = 15;
            const uint8 max_tone = 161;  /* WAV_RING_TONE_TOP - 1, not quite right  */
            const uint8 max_gain = 22;
            
            uint16 idx;
            uint8* data = payload;
            bool data_error = FALSE;
            
        
            for (idx = 0; !data_error && (idx < VOL_NUM_VOL_SETTINGS); ++idx)
            {
                if (*data > max_vol)
                    data_error = TRUE;
                
                config->gVolMaps[idx].IncVol = *data++;
                
                if (*data > max_vol)
                    data_error = TRUE;
                
                config->gVolMaps[idx].DecVol = *data++;
                
                if (*data > max_tone)
                    data_error = TRUE;

                config->gVolMaps[idx].Tone = *data++;
                
                if (*data > max_gain)
                    data_error = TRUE;
                
                config->gVolMaps[idx].A2dpGain = *data++;

                if (*data > max_gain)
                    data_error = TRUE;
                
                config->gVolMaps[idx].VolGain = *data++;
            }
            
            if (data_error)
                wechat_send_invalid_parameter(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);
            
            else if (ConfigStore(CONFIG_VOLUME_CONTROL, config, config_len) == 0)
                wechat_send_insufficient_resources(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);
            
            else
                wechat_send_success(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);

            freePanic(config);
        }
    }  
}

        
/*************************************************************************
NAME
    wechat_send_audio_gain_config
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_AUDIO_GAIN_CONFIGURATION
    The response contains five octets for each volume mapping, corresponding
    to IncVol, DecVol, Tone, A2dpGain and VolGain
*/
static void wechat_send_audio_gain_config(void)
{
    uint16 payload_length = VOL_NUM_VOL_SETTINGS * 5;
    uint16 config_length = sizeof (volume_configuration_t);
    uint8 *payload = mallocPanic(payload_length);
    volume_configuration_t *config = mallocPanic(config_length);
                    
    if (config == NULL || 
        payload == NULL || 
        ConfigRetrieve(CONFIG_VOLUME_CONTROL, config, config_length) != config_length)
    {
        free(config);   /* Don't panic, either can be NULL */
        free(payload);
        wechat_send_insufficient_resources(WECHAT_COMMAND_GET_AUDIO_GAIN_CONFIGURATION);
    }
    
    else
    {
        uint16 idx;
        uint8 *data = payload;
                
        for (idx = 0; idx < VOL_NUM_VOL_SETTINGS; ++idx)
        {
            *data++ = config->gVolMaps[idx].IncVol;
            *data++ = config->gVolMaps[idx].DecVol;
            *data++ = config->gVolMaps[idx].Tone;
            *data++ = config->gVolMaps[idx].A2dpGain;
            *data++ = config->gVolMaps[idx].VolGain;
        }

        freePanic(config);        
        wechat_send_success_payload(WECHAT_COMMAND_GET_AUDIO_GAIN_CONFIGURATION, payload_length, payload);
        freePanic(payload);
    }  
}


/*************************************************************************
NAME
    wechat_set_user_tone_config
    
DESCRIPTION
    Handle WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION command
    The command payload contains a tone number (1 to
    MAX_NUM_VARIABLE_TONES) and note data for the indicated tone.
    
    Note data consists of tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and length values for each note.
    
    If the indicated tone number is not in use we add it, otherwise we
    delete the existing data, compact the free space and add the new tone
    data at the end of the list.
*/
static void wechat_set_user_tone_config(uint8 payload_len, uint8 *payload)
{
    lengths_config_type lengths;
    uint16 config_len;
    uint16 *note_data;
    uint16 tone_len;
    uint16 idx;
    uint16 old;
    uint8 tone_idx;
    
    if ((payload_len < 5) 
        || ((payload_len & 1) == 0) 
        || (payload[0] < 1) 
        || (payload[0] > MAX_NUM_VARIABLE_TONES))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION);
        return;
    }
    
    note_data = mallocPanic(WECHAT_TONE_BUFFER_SIZE);
    
    if (note_data == NULL)
    {
        wechat_send_insufficient_resources(WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION);
        return;
    }
            
    get_config_lengths(&lengths);
    
    config_len = lengths.userTonesLength;
    if (config_len == 0)
    {
        /* No tones; create empty list  */
        config_len = MAX_NUM_VARIABLE_TONES;
        for (idx = 0; idx < config_len; ++idx)
            note_data[idx] = 0;    
    }
    
    else
        ConfigRetrieve(CONFIG_USER_TONES, note_data, config_len);
    
#ifdef DEBUG_WECHAT
    dump("before", note_data, config_len);
#endif
    
    tone_idx = payload[0] - 1;

    old = note_data[tone_idx];
    /* If the index in use, purge it  */
    if (old)
    {
        /*  Find the end  */
        idx = old;
        while (note_data[idx] != (uint16) RINGTONE_END)
            ++idx;
        
        tone_len = idx - old + 1;

        /* Adjust any offsets into purged space  */
        for (idx = 0; idx < MAX_NUM_VARIABLE_TONES; ++idx)
            if (note_data[idx] > old)
                note_data[idx] -= tone_len;
        
        /* Close the gap  */
        config_len -= tone_len;
        for (idx = old; idx < config_len; ++idx)
            note_data[idx] = note_data[idx + tone_len];
    }
        
#ifdef DEBUG_WECHAT
    dump("during", note_data, config_len);
#endif
        
    tone_len = (payload_len - 4) / 2 + 1;
    
    if ((config_len + tone_len) > WECHAT_TONE_BUFFER_SIZE)  /* TODO: test this sooner! */
        wechat_send_insufficient_resources(WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION);
    
    else
    {
        /* Copy new tone into free space  */
        note_data[tone_idx] = config_len;
        
        if (payload[1] != 0)
            note_data[config_len++] = (uint16) RINGTONE_TEMPO(payload[1] * 4);
        
        if (payload[2] != 0)
            note_data[config_len++] = (uint16) RINGTONE_VOLUME(payload[2]);
        
        if (payload[3] != 0)
            note_data[config_len++] = (uint16) RINGTONE_SEQ_TIMBRE | payload[3];
        
        if (payload[4] != 0)
            note_data[config_len++] = (uint16) RINGTONE_DECAY(payload[4]);
         
        for (idx = 5; idx < payload_len; idx += 2)
            note_data[config_len++] = (uint16) pitch_length_tone(payload[idx], payload[idx + 1]);
        
        note_data[config_len++] = (uint16) RINGTONE_END;
        
#ifdef DEBUG_WECHAT
        dump("after ", note_data, config_len);
#endif

        /*  Write back to persistent store  */
        if (ConfigStore(CONFIG_USER_TONES, note_data, config_len) == 0)
            wechat_send_insufficient_resources(WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION);
        
        else
        {
            /*  Adjust the lengths structure if necessary */
            if (config_len != lengths.userTonesLength)
            {
                lengths.userTonesLength = config_len;
                if (ConfigStore(CONFIG_LENGTHS, &lengths, sizeof lengths) == 0)
                {
                    WECHAT_DEBUG(("WECHAT: ConfigStore lengths failed\n"));
                    Panic();
                }
            }
            
            wechat_send_success(WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION);
        }
    }
        
    freePanic(note_data);
}


/*************************************************************************
NAME
    wechat_send_user_tone_config
    
DESCRIPTION
    Act on a WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION command to send the
    note data for the indicated user-defined tone configuration.

    Note data consists of tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and length values for each note.
    
    At most one of each tempo, volume, timbre and decay value is used and
    the tempo value is scaled, so if the configuration was set other than
    by Wechat it might not be accurately represented.
    
    We expect sizeof (ringtone_note) to be 1; see ringtone_if.h
*/
static void wechat_send_user_tone_config(uint8 id)
{
    lengths_config_type lengths;
    uint16 config_len;
    uint16 *note_data = NULL;

    get_config_lengths(&lengths);

    if ((id < 1) || (id > MAX_NUM_VARIABLE_TONES) || (lengths.userTonesLength == 0))
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION);
        return;
    }
    
    config_len = lengths.userTonesLength;
    note_data = mallocPanic(config_len);
    
    if (note_data == NULL)
        wechat_send_insufficient_resources(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION);
    
    else
    {
        ConfigRetrieve(CONFIG_USER_TONES, note_data, config_len);
        
        if (note_data[id - 1] == 0)
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION);
        
        else
        {
            uint16 *start = note_data + note_data[id - 1];
            uint16 *find = start;
            uint16 tone_length = 0;
            uint8 payload_length = 0;
            uint8 *payload = NULL;
            
            note_data[0] = 30; /* default tempo (30 * 4 = 120) */
            note_data[1] = 31; /* default volume (max) */
            note_data[2] = ringtone_tone_sine;  /* default timbre (sine) */
            note_data[3] = 32;  /* default decay rate */
            
        /*  Find the tempo, volume, timbre and decay  */
            while (*find != (uint16) RINGTONE_END)
            {
                if (*find & RINGTONE_SEQ_CONTROL_MASK)
                {
                    switch (*find & (RINGTONE_SEQ_CONTROL_MASK | RINGTONE_SEQ_CONTROL_COMMAND_MASK))
                    {
                    case RINGTONE_SEQ_TEMPO:
                        note_data[0] = ((*find & RINGTONE_SEQ_TEMPO_MASK) + 2) / 4;
                        break;
                        
                    case RINGTONE_SEQ_VOLUME:
                        note_data[1] = *find & RINGTONE_SEQ_VOLUME_MASK;
                        break;
                        
                    case RINGTONE_SEQ_TIMBRE:
                        note_data[2] = *find & RINGTONE_SEQ_TIMBRE_MASK;
                        break;
                        
                    case RINGTONE_SEQ_DECAY:
                        note_data[3] = *find & RINGTONE_SEQ_DECAY_RATE_MASK;
                        break;
                    }
                }
                
                else
                    ++tone_length;
                
                ++find;
            }
            
            payload_length = 2 * tone_length + 5;
            payload = mallocPanic(payload_length);
            
            if (payload == NULL)
                wechat_send_insufficient_resources(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION);
            
            else
            {
                payload[0] = id;
                payload[1] = note_data[0];
                payload[2] = note_data[1];
                payload[3] = note_data[2];
                payload[4] = note_data[3];
                
                find = start;
                payload_length = 5;
                
            /*  Find the pitch and length of each note  */
                while (*find != (uint16) RINGTONE_END)
                {
                    if ((*find & RINGTONE_SEQ_CONTROL_MASK) == 0)
                    {
                        payload[payload_length++] = (*find & RINGTONE_SEQ_NOTE_PITCH_MASK) >> RINGTONE_SEQ_NOTE_PITCH_POS;
                        payload[payload_length++] = *find & RINGTONE_SEQ_NOTE_LENGTH_MASK;
                    }
                    
                    ++find;
                }
                
                wechat_send_success_payload(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION,
                                          payload_length, payload);
                
                freePanic(payload);
            }
        }
        
        freePanic(note_data);
    }
}
        

/*************************************************************************
NAME
    wechat_send_application_version
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_APPLICATION_VERSION by sending the Device ID
    setting
*/
static void wechat_send_application_version(void)
{    
#ifdef DEVICE_ID_CONST
/*  Device ID is defined in sink_device_id.h  */
    uint16 payload[] = {DEVICE_ID_VENDOR_ID_SOURCE,
                        DEVICE_ID_VENDOR_ID,
                        DEVICE_ID_PRODUCT_ID,
                        DEVICE_ID_BCD_VERSION};
    
    wechat_send_response_16(WECHAT_COMMAND_GET_APPLICATION_VERSION, 
                         WECHAT_STATUS_SUCCESS, sizeof payload, payload);
#else
/*  Read Device ID from config, irrespective of DEVICE_ID_PSKEY  */
    uint16 payload[8];
    uint16 payload_length;
    
    payload_length = ConfigRetrieve(CONFIG_DEVICE_ID, payload, sizeof payload);

    wechat_send_response_16(WECHAT_COMMAND_GET_APPLICATION_VERSION, 
                         WECHAT_STATUS_SUCCESS, payload_length, payload);    
#endif
}
  

/*************************************************************************
NAME
    wechat_set_feature
    
DESCRIPTION
    Handle WECHAT_COMMAND_SET_FEATURE
*/
static void wechat_set_feature(uint8 payload_length, uint8 *payload)
{
    if ((payload_length == 0) || (payload[0] > WECHAT_MAX_FEATURE_BIT))
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_FEATURE);
    
    else
        wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_SET_FEATURE, WECHAT_STATUS_NOT_AUTHENTICATED, 1, payload);
}


/*************************************************************************
NAME
    wechat_get_feature
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_FEATURE
*/
static void wechat_get_feature(uint8 payload_length, uint8 *payload)
{
    if ((payload_length != 1) || (payload[0] > WECHAT_MAX_FEATURE_BIT))
        wechat_send_invalid_parameter(WECHAT_COMMAND_SET_FEATURE);
    
    else
    {
        uint8 response[2];
        response[0] = payload[0];
        response[1] = 0x00;
        wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_GET_FEATURE, WECHAT_STATUS_SUCCESS, 2, response);
    }
}


#if defined ENABLE_PEER && defined PEER_TWS
/*************************************************************************
NAME
    wechat_set_tws_routing
    
DESCRIPTION
    Handle WECHAT_COMMAND_SET_TWS_AUDIO_ROUTING
*/
static void wechat_set_tws_routing(uint8 device, uint8 mode)
{
    uint8 response[2];
    uint8 peer_mode;
    
    response[0] = device;
    response[1] = mode;
    
    if (mode == PEER_TWS_ROUTING_LEFT)
        peer_mode = PEER_TWS_ROUTING_RIGHT;
    
    else if (mode == PEER_TWS_ROUTING_RIGHT)
        peer_mode = PEER_TWS_ROUTING_LEFT;
    
    else
        peer_mode = mode;
    
    if (device == 0)
    {
        theSink.a2dp_link_data->a2dp_audio_mode_params.master_routing_mode = mode;
        theSink.a2dp_link_data->a2dp_audio_mode_params.slave_routing_mode = peer_mode;
    }
    
    else
    {
        theSink.a2dp_link_data->a2dp_audio_mode_params.slave_routing_mode = mode;
        theSink.a2dp_link_data->a2dp_audio_mode_params.master_routing_mode = peer_mode;
    }
    
    WECHAT_DEBUG(("G: tws set rtg %u %u/%u\n", device, mode, peer_mode));
    
    if (theSink.routed_audio)
        AudioSetMode(AUDIO_MODE_CONNECTED, &theSink.a2dp_link_data->a2dp_audio_mode_params);           
        
    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_SET_TWS_AUDIO_ROUTING, 
                       WECHAT_STATUS_SUCCESS, sizeof response, response);    
}


/*************************************************************************
NAME
    wechat_send_tws_routing
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_TWS_AUDIO_ROUTING
*/
static void wechat_send_tws_routing(uint8 device)
{
    uint8 response[2];
    
    response[0] = device;
    response[1] = (device == 0) ?
                  theSink.a2dp_link_data->a2dp_audio_mode_params.master_routing_mode :
                  theSink.a2dp_link_data->a2dp_audio_mode_params.slave_routing_mode;
    
    WECHAT_DEBUG(("G: tws get rtg %u %u\n", device, response[1]));
        
    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_GET_TWS_AUDIO_ROUTING, 
                       WECHAT_STATUS_SUCCESS, sizeof response, response);
}


/*************************************************************************
NAME
    wechat_trim_tws_volume
    
DESCRIPTION
    Handle WECHAT_COMMAND_TRIM_TWS_VOLUME
    
    role 0 means master, 1 means slave
    direction 0 means volume up, 1 means down.

*/
static void wechat_trim_tws_volume(uint8 role, uint8 direction)
{
    if ((role < 2) && (direction < 2))
    {
        uint16 event = EventInvalid;
        
        switch ((role << 1) | direction)
        {
        case 0:
            event = EventUsrMasterDeviceTrimVolumeUp;
            break;
            
        case 1:
            event = EventUsrMasterDeviceTrimVolumeDown;
            break;
            
        case 2:
            event = EventUsrSlaveDeviceTrimVolumeUp;
            break;
            
        case 3:
            event = EventUsrSlaveDeviceTrimVolumeDown;
            break;
        }
        
        send_app_message(event);
        wechat_send_success(WECHAT_COMMAND_TRIM_TWS_VOLUME);
    }
    
    else
    {
        wechat_send_invalid_parameter(WECHAT_COMMAND_TRIM_TWS_VOLUME);
    }
}
#endif /* defined ENABLE_PEER && defined PEER_TWS */


/*************************************************************************
NAME
    wechat_register_notification
    
DESCRIPTION
    Handle WECHAT_COMMAND_REGISTER_NOTIFICATION
*/
static void wechat_register_notification(uint8 payload_length, uint8 *payload)
{
    if (payload_length == 0)
        wechat_send_invalid_parameter(WECHAT_COMMAND_REGISTER_NOTIFICATION);
    
    else
    {
        uint8 status = WECHAT_STATUS_INVALID_PARAMETER;
        
        WECHAT_DEBUG(("G: reg: %02X %d\n", payload[0], payload_length));
        switch (payload[0])
        {
        case WECHAT_EVENT_PIO_CHANGED:
            if (payload_length == 5)
            {
                uint32 mask;
                
                mask = payload[1];
                mask = (mask << 8) | payload[2];
                mask = (mask << 8) | payload[3];
                mask = (mask << 8) | payload[4];
                
                wechat_data.pio_change_mask = mask;
                
                WECHAT_DEBUG(("G: pm: %08lX\n", mask));
                status = WECHAT_STATUS_SUCCESS;
            }

            break;
                
            
        case WECHAT_EVENT_BATTERY_CHARGED:
            wechat_data.notify_battery_charged = TRUE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
            
        case WECHAT_EVENT_CHARGER_CONNECTION:
            wechat_data.notify_charger_connection = TRUE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
            
        case WECHAT_EVENT_USER_ACTION:
            wechat_data.notify_ui_event = TRUE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
            
#ifdef ENABLE_SPEECH_RECOGNITION
        case WECHAT_EVENT_SPEECH_RECOGNITION:
            wechat_data.notify_speech_rec = TRUE;
            status = WECHAT_STATUS_SUCCESS;
            break;
#endif
        }
        
        wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_REGISTER_NOTIFICATION, status, 1, payload);
        
        if (payload[0] == WECHAT_EVENT_CHARGER_CONNECTION)
        {
            status = powerManagerIsChargerConnected();
            wechat_send_notification(WECHAT_EVENT_CHARGER_CONNECTION, 1, &status);
        }
    } 
}


/*************************************************************************
NAME
    wechat_get_notification
    
DESCRIPTION
    Handle WECHAT_COMMAND_GET_NOTIFICATION
    Respond with the current notification setting (enabled/disabled status
    and notification-specific data)
*/
static void wechat_get_notification(uint8 payload_length, uint8 *payload)
{
    uint16 response_len = 0;
    uint8 response[6];
    uint8 status = WECHAT_STATUS_INVALID_PARAMETER;
    
    if (payload_length > 0)
    {
        response[0] = payload[0];
        response_len = 1;
        
        switch (payload[0])
        {
        case WECHAT_EVENT_PIO_CHANGED:
            response[1] = wechat_data.pio_change_mask != 0;
            dwunpack(response + 2, wechat_data.pio_change_mask);     
            response_len = 6;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_BATTERY_CHARGED:
            response[1] = wechat_data.notify_battery_charged;
            response_len = 2;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_CHARGER_CONNECTION:
            response[1] = wechat_data.notify_charger_connection;
            response_len = 2;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_USER_ACTION:
            response[1] = wechat_data.notify_ui_event;
            response_len = 2;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
#ifdef ENABLE_SPEECH_RECOGNITION
        case WECHAT_EVENT_SPEECH_RECOGNITION:
            response[1] = wechat_data.notify_speech_rec;
            response_len = 2;
            status = WECHAT_STATUS_SUCCESS;
            break;
#endif
        }
    }
    
    wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_GET_NOTIFICATION, status, response_len, response);
}


/*************************************************************************
NAME
    wechat_cancel_notification
    
DESCRIPTION
    Handle WECHAT_COMMAND_CANCEL_NOTIFICATION
*/
static void wechat_cancel_notification(uint8 payload_length, uint8 *payload)
{
    if (payload_length != 1)
        wechat_send_invalid_parameter(WECHAT_COMMAND_CANCEL_NOTIFICATION);
    
    else
    {
        uint8 status;
        
        switch (payload[0])
        {
        case WECHAT_EVENT_PIO_CHANGED:
            wechat_data.pio_change_mask = 0UL;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_BATTERY_CHARGED:
            wechat_data.notify_battery_charged = FALSE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_CHARGER_CONNECTION:
            wechat_data.notify_charger_connection = FALSE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
        case WECHAT_EVENT_USER_ACTION:
            wechat_data.notify_ui_event = FALSE;
            status = WECHAT_STATUS_SUCCESS;
            break;
            
#ifdef ENABLE_SPEECH_RECOGNITION            
        case WECHAT_EVENT_SPEECH_RECOGNITION:
            wechat_data.notify_speech_rec = FALSE;
            status = WECHAT_STATUS_SUCCESS;
            break;
#endif
            
        default:
            status = WECHAT_STATUS_INVALID_PARAMETER;
            break;
        }
        
        wechat_send_response(WECHAT_VENDOR_CSR, WECHAT_COMMAND_CANCEL_NOTIFICATION, status, 1, payload);
    } 
}


/*************************************************************************
NAME
    wechat_handle_configuration_command
    
DESCRIPTION
    Handle a Wechat configuration command or return FALSE if we can't
*/
static bool wechat_handle_configuration_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{
    switch (command->command_id)
    {
    case WECHAT_COMMAND_SET_LED_CONFIGURATION:
        if (command->size_payload < 7)
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_LED_CONFIGURATION);

        else
            wechat_set_led_config(command->size_payload, command->payload);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_LED_CONFIGURATION:
        if (command->size_payload == 3)
            wechat_send_led_config(command->payload[0], (command->payload[1] << 8) |command->payload[2]);

        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_LED_CONFIGURATION);
        
        return TRUE;
        
                
    case WECHAT_COMMAND_SET_TONE_CONFIGURATION:
        if (command->size_payload == 3)
            wechat_set_tone_config((command->payload[0] << 8) | command->payload[1], command->payload[2]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_TONE_CONFIGURATION);
        
        return TRUE;

        
    case WECHAT_COMMAND_GET_TONE_CONFIGURATION:
        if (command->size_payload == 2)
            wechat_send_tone_config((command->payload[0] << 8) | command->payload[1]);

        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_TONE_CONFIGURATION);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_DEFAULT_VOLUME:
        if (command->size_payload == 3)
            wechat_set_default_volumes(command->payload);

        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_DEFAULT_VOLUME);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_DEFAULT_VOLUME:
        wechat_send_default_volumes();
        return TRUE;
        
            
    case WECHAT_COMMAND_SET_VOICE_PROMPT_CONFIGURATION:
        if (command->size_payload == 6)
            wechat_set_ap_config(command->payload);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_VOICE_PROMPT_CONFIGURATION);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_VOICE_PROMPT_CONFIGURATION:
        if (command->size_payload == 2)
            wechat_send_ap_config((command->payload[0] << 8) | command->payload[1]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_VOICE_PROMPT_CONFIGURATION);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_TIMER_CONFIGURATION:
        wechat_set_timer_config(command->size_payload, command->payload);
        return TRUE;
        
            
    case WECHAT_COMMAND_GET_TIMER_CONFIGURATION:
        wechat_send_timer_config();
        return TRUE;
        
            
    case WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION:
        if (command->size_payload != 5 * VOL_NUM_VOL_SETTINGS)
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_AUDIO_GAIN_CONFIGURATION);

        else
            wechat_set_audio_gain_config(command->size_payload, command->payload);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_AUDIO_GAIN_CONFIGURATION:
        wechat_send_audio_gain_config();
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_USER_TONE_CONFIGURATION:
        wechat_set_user_tone_config(command->size_payload, command->payload);
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION:
        if (command->size_payload == 1)
            wechat_send_user_tone_config(command->payload[0]);

        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_USER_TONE_CONFIGURATION);
        
        return TRUE;

#ifdef ENABLE_SQIFVP
    case WECHAT_COMMAND_GET_MOUNTED_PARTITIONS:
        {
            uint8 response[1];   
            response[0] = theSink.rundata->partitions_mounted;
            wechat_send_success_payload(WECHAT_COMMAND_GET_MOUNTED_PARTITIONS, 1, response);
            
            /*reread available partitions*/
            configManagerSqifPartitionsInit();
            return TRUE;          
        }
#endif  
        
    default:
        return FALSE;
    }
}

static void updatePEQ(uint8* payload){
    
    if(payload){
        
        uint16 param = MAKEWORD(payload[PARAM_LO_OFFSET] , payload[PARAM_HI_OFFSET]);    
        uint16 value = MAKEWORD(payload[VALUE_LO_OFFSET] , payload[VALUE_HI_OFFSET]);
        uint16 bankIndex = 0;
        uint16 bandIndex = 0;
        uint16 bandParam;
        
        if(!(param & CHANGE_NUMBER_OF_ACTIVE_BANKS)){
            /* No WECHAT command is expected to change number of active EQ banks */
            WECHAT_DEBUG(("WECHAT: Number of Active Banks , param->%04x", 
                            (uint16) param));       
            return;
        }
        else if(!(param & CHANGE_NUMBER_OF_BANDS)){
            /* No WECHAT command in expected as the number of bands in user EQ is fixed to 5 */
            WECHAT_DEBUG(("WECHAT: Number of Bands in the Specified Bank , param->%04x", 
                            (uint16) param)); 
            return;
        }
        else if(!(param & CHANGE_BANK_MASTER_GAIN)){
            /* If master gain for EQ bank is to be adjusted, we are only interested in the bank index field of the parameter ID */
            bankIndex = (param>>8)&0xF; 

            if(bankIndex == USER_EQ_BANK_INDEX)
                theSink.PEQ->preGain = value;
        }
        else if(!(param & CHANGE_BAND_PARAMETER))
        {   /* Band index and bank index valuea are both required to update filter type, gain, frequency and bandwidth of the individual EQ bands */
            bankIndex = (param>>8)&0xF;
            bandIndex = (param>>4)&0xF;
            
            /* Check if bank index and band index values are valid */
            if((bankIndex == USER_EQ_BANK_INDEX) && (bandIndex > 0) && (bandIndex < 6)){
                
                bandParam = param&0xF;
                switch(bandParam) 
                {
                    /* Update values shifting band index by 1 as WECHAT EQ commands are intended only for bands 1-2-3-4-5 in EQ bank 1 */
                    case 0:
                      theSink.PEQ->bands[bandIndex-1].filter = value;
                      break;
                    case 1:
                      theSink.PEQ->bands[bandIndex-1].freq = value;
                      break;
                    case 2:
                      theSink.PEQ->bands[bandIndex-1].gain = value;
                      break;
                    case 3:
                      theSink.PEQ->bands[bandIndex-1].Q = value;
                      break;
                    default:
                      break;
                }     
            }
        }
    }
}    

/*************************************************************************
NAME
    wechat_handle_control_command
    
DESCRIPTION
    Handle a Wechat control command or return FALSE if we can't
*/
static bool wechat_handle_control_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{
    uint8 response[1];
    
    switch (command->command_id)
    {
    case WECHAT_COMMAND_CHANGE_VOLUME:
        if (command->size_payload == 1)
            wechat_change_volume(command->payload[0]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_CHANGE_VOLUME);
        
        return TRUE;

        
    case WECHAT_COMMAND_ALERT_LEDS:
        if (command->size_payload == 13)
            wechat_alert_leds(command->payload);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_LEDS);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_ALERT_TONE:
        wechat_alert_tone(command->size_payload, command->payload);
        return TRUE;
        
        
    case WECHAT_COMMAND_ALERT_EVENT:
        if (command->size_payload == 2)
            wechat_alert_event((command->payload[0] << 8) | command->payload[1]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_EVENT);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_ALERT_VOICE:
        if (theSink.num_audio_prompt_languages == 0)
            return FALSE;
               
        if (command->size_payload >= 2)
            wechat_alert_voice(command->size_payload, command->payload);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_ALERT_VOICE);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_POWER_OFF:
        send_app_message(EventUsrPowerOff);
        wechat_send_success(WECHAT_COMMAND_POWER_OFF);
        return TRUE;

    
    case WECHAT_COMMAND_SET_VOLUME_ORIENTATION:
        if ((command->size_payload == 1) && (command->payload[0] == 0))
        {
            if (theSink.gVolButtonsInverted)
            {
                theSink.gVolButtonsInverted = FALSE;
                configManagerWriteSessionData();
            }
            
            wechat_send_success(WECHAT_COMMAND_SET_VOLUME_ORIENTATION);
        }
        
        else if ((command->size_payload == 1) && (command->payload[0] == 1))
        {
            if (!theSink.gVolButtonsInverted)
            {
                theSink.gVolButtonsInverted = TRUE;
                configManagerWriteSessionData();
            }
            
            wechat_send_success(WECHAT_COMMAND_SET_VOLUME_ORIENTATION);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_VOLUME_ORIENTATION);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_VOLUME_ORIENTATION:
        response[0] = theSink.gVolButtonsInverted;
        wechat_send_success_payload(WECHAT_COMMAND_GET_VOLUME_ORIENTATION, 1, response);
        return TRUE;
    
    
    case WECHAT_COMMAND_SET_LED_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] == 0))
        {
            WECHAT_DEBUG(("G: SET_LED_CONTROL: 0\n")); 
            LedManagerDisableLEDS();
        /*  LedsIndicateNoState();  */
        /*  configManagerWriteSessionData();  */
            LedConfigure(LED_0, LED_ENABLE, FALSE);
            LedConfigure(LED_1, LED_ENABLE, FALSE);
            LedConfigure(LED_2, LED_ENABLE, FALSE);
            wechat_send_success(WECHAT_COMMAND_SET_LED_CONTROL);
        }
        
        else if ((command->size_payload == 1) && (command->payload[0] == 1))
        {
            WECHAT_DEBUG(("G: SET_LED_CONTROL: 1\n")); 
            LedManagerEnableLEDS();
        /*  configManagerWriteSessionData();  */
            wechat_send_success(WECHAT_COMMAND_SET_LED_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_LED_CONTROL);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_LED_CONTROL:
        response[0] = theSink.theLEDTask->gLEDSEnabled;
        wechat_send_success_payload(WECHAT_COMMAND_GET_LED_CONTROL, 1, response);
        return TRUE;
        
    case WECHAT_COMMAND_PLAY_TONE:
        if (command->size_payload == 1) 
        {
            TonesPlayTone(command->payload[0], TRUE, FALSE);
            wechat_send_success(WECHAT_COMMAND_PLAY_TONE);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_PLAY_TONE);

        return TRUE;
        
        
    case WECHAT_COMMAND_SET_VOICE_PROMPT_CONTROL:
        if ((command->size_payload == 1) && ((command->payload[0] == 0) || (command->payload[0] == 1)))
        {
            WECHAT_DEBUG(("G: SET_VOICE_PROMPT_CONTROL: %d\n", command->payload[0])); 
            if (theSink.audio_prompts_enabled != command->payload[0])
            {
                theSink.audio_prompts_enabled = command->payload[0];
                configManagerWriteSessionData();
            }
            
            wechat_send_success(WECHAT_COMMAND_SET_VOICE_PROMPT_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_VOICE_PROMPT_CONTROL);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_VOICE_PROMPT_CONTROL:
        response[0] = theSink.audio_prompts_enabled;
        wechat_send_success_payload(WECHAT_COMMAND_GET_VOICE_PROMPT_CONTROL, 1, response);
        return TRUE;
        
    case WECHAT_COMMAND_CHANGE_AUDIO_PROMPT_LANGUAGE:
        if (theSink.num_audio_prompt_languages == 0)
            return FALSE;
        
        send_app_message(EventUsrSelectAudioPromptLanguageMode);
        wechat_send_success(WECHAT_COMMAND_CHANGE_AUDIO_PROMPT_LANGUAGE);
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_AUDIO_PROMPT_LANGUAGE:
        if (theSink.num_audio_prompt_languages == 0)
            return FALSE;
        
        if ((command->size_payload == 1) && (command->payload[0] < theSink.num_audio_prompt_languages))
        {
            theSink.audio_prompt_language = command->payload[0];
            configManagerWriteSessionData();
            wechat_send_success(WECHAT_COMMAND_SET_AUDIO_PROMPT_LANGUAGE);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_AUDIO_PROMPT_LANGUAGE);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_AUDIO_PROMPT_LANGUAGE:
        if (theSink.num_audio_prompt_languages == 0)
            return FALSE;
        
        response[0] = theSink.audio_prompt_language;
        wechat_send_success_payload(WECHAT_COMMAND_GET_AUDIO_PROMPT_LANGUAGE, 1, response);
        return TRUE;
        
#ifdef ENABLE_SPEECH_RECOGNITION
    case WECHAT_COMMAND_SET_SPEECH_RECOGNITION_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrSSROn : EventUsrSSROff);
            wechat_send_success(WECHAT_COMMAND_SET_SPEECH_RECOGNITION_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_SPEECH_RECOGNITION_CONTROL);
        
        return TRUE;
        
    case WECHAT_COMMAND_GET_SPEECH_RECOGNITION_CONTROL:
        response[0] = theSink.ssr_enabled;
        wechat_send_success_payload(WECHAT_COMMAND_GET_SPEECH_RECOGNITION_CONTROL, 1, response);
        return TRUE;
        
    case WECHAT_COMMAND_START_SPEECH_RECOGNITION:
        speechRecognitionStart();
        wechat_send_success(WECHAT_COMMAND_START_SPEECH_RECOGNITION);
        return TRUE;
#endif
        
    case WECHAT_COMMAND_SWITCH_EQ_CONTROL:
        send_app_message(EventUsrSwitchAudioMode);
        wechat_send_success(WECHAT_COMMAND_SWITCH_EQ_CONTROL);
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_EQ_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] <= A2DP_MUSIC_MAX_EQ_BANK))
        {
            set_abs_eq_bank(command->payload[0]);
            wechat_send_success(WECHAT_COMMAND_SET_EQ_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_EQ_CONTROL);
        
        return TRUE;

                        
    case WECHAT_COMMAND_GET_EQ_CONTROL:
        response[0] = theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_processing;
        if (response[0] < A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0)
            wechat_send_insufficient_resources(WECHAT_COMMAND_GET_EQ_CONTROL);

        else
        {
            response[0] -= A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0;
            wechat_send_success_payload(WECHAT_COMMAND_GET_EQ_CONTROL, 1, response);
        }
        return TRUE;
        
        
    case WECHAT_COMMAND_SET_BASS_BOOST_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrBassBoostOn : EventUsrBassBoostOff);
            wechat_send_success(WECHAT_COMMAND_SET_BASS_BOOST_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_BASS_BOOST_CONTROL);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_BASS_BOOST_CONTROL:
        response[0] = theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_enhancements & MUSIC_CONFIG_BASS_BOOST_BYPASS ?
                      1 : 0;
        
        wechat_send_success_payload(WECHAT_COMMAND_GET_BASS_BOOST_CONTROL, 1, response);
        return TRUE;
        
        
    case WECHAT_COMMAND_TOGGLE_BASS_BOOST_CONTROL:
        send_app_message(EventUsrBassBoostEnableDisableToggle);
        wechat_send_success(WECHAT_COMMAND_TOGGLE_BASS_BOOST_CONTROL);
        return TRUE;
            
        
    case WECHAT_COMMAND_SET_3D_ENHANCEMENT_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsr3DEnhancementOn : EventUsr3DEnhancementOff);
            wechat_send_success(WECHAT_COMMAND_SET_3D_ENHANCEMENT_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_3D_ENHANCEMENT_CONTROL);
        
        return TRUE;

        
    case WECHAT_COMMAND_GET_3D_ENHANCEMENT_CONTROL:
        response[0] = theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_enhancements & MUSIC_CONFIG_SPATIAL_BYPASS ?
                      1 : 0;
        
        wechat_send_success_payload(WECHAT_COMMAND_GET_3D_ENHANCEMENT_CONTROL, 1, response);
        return TRUE;
        
        
    case WECHAT_COMMAND_TOGGLE_3D_ENHANCEMENT_CONTROL:
        send_app_message(EventUsr3DEnhancementEnableDisableToggle);
        wechat_send_success(WECHAT_COMMAND_TOGGLE_3D_ENHANCEMENT_CONTROL);
        return TRUE;     
        
        
    case WECHAT_COMMAND_GET_USER_EQ_CONTROL:
         response[0] = theSink.a2dp_link_data->a2dp_audio_mode_params.music_mode_enhancements & MUSIC_CONFIG_USER_EQ_BYPASS ?
                      1 : 0;
        
        wechat_send_success_payload(WECHAT_COMMAND_GET_USER_EQ_CONTROL, 1, response);
        return TRUE;
       
        
    case WECHAT_COMMAND_SET_USER_EQ_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrUserEqOn : EventUsrUserEqOff);
            wechat_send_success(WECHAT_COMMAND_SET_USER_EQ_CONTROL);
        }
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_USER_EQ_CONTROL);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_TOGGLE_USER_EQ_CONTROL:
        send_app_message(EventUsrUserEqOnOffToggle);
        wechat_send_success(WECHAT_COMMAND_TOGGLE_USER_EQ_CONTROL);
        return TRUE;     

        
    case WECHAT_COMMAND_SET_USER_EQ_PARAMETER:
        if (KalimbaSendMessage(DSP_WECHAT_MSG_SET_USER_PARAM,
                               MAKEWORD(command->payload[1] , command->payload[0]),
                               MAKEWORD(command->payload[3] , command->payload[2]),
                               command->payload[4],0))
        {
            wechat_send_success(WECHAT_COMMAND_SET_USER_EQ_PARAMETER);

            /* Modified EQ settings should be stored in dynamic memory */
            updatePEQ(command->payload);

#if defined ENABLE_PEER && defined PEER_TWS
            /*Update the peer with the user EQ settings */
            peerSendUserEqSettings();
#endif

            
#ifdef ENABLE_WECHAT_PERSISTENT_USER_EQ_BANK
            /* EQ settings in PS Store should be updated once the user-defined inactivity period has expired */
            if(theSink.conf1->timeouts.StoreCurrentPEQSettingsTimeout_s)
            {
                MessageCancelAll(&theSink.task, EventSysWechatEQChangesStoreTimeout);
                MessageSendLater(&theSink.task, EventSysWechatEQChangesStoreTimeout , 0, D_SEC(theSink.conf1->timeouts.StoreCurrentPEQSettingsTimeout_s));
            }
#endif
        }        
        else
        {
            wechat_send_incorrect_state(WECHAT_COMMAND_SET_USER_EQ_PARAMETER);
        }
        
        return TRUE;     

        
    case WECHAT_COMMAND_SET_USER_EQ_GROUP_PARAMETER:
        {
            uint16 size = command->size_payload;
            uint16 *dspMsgPayload = mallocPanic(size/NUM_WORDS_WECHAT_CMD_HDR  + 1);
            uint16 i;
            
            if (dspMsgPayload)
            {
                for(i=0;i<(size/NUM_WORDS_WECHAT_CMD_HDR );i++)
                {
                    dspMsgPayload[i] = MAKEWORD(command->payload[i*NUM_WORDS_WECHAT_CMD_HDR +1], command->payload[i*NUM_WORDS_WECHAT_CMD_HDR ]);
                }                
                dspMsgPayload[i] = command->payload[i*NUM_WORDS_WECHAT_CMD_HDR ];

                /* Skip the first two index as they point to the number of EQ parameters to modify which we are not interested in 
                since we already know that updatePEQ() processes 4 payload index at once */                    
                for(i=0;i<(size-NUM_WORDS_WECHAT_CMD_HDR )/NUM_WORDS_WECHAT_CMD_PER_EQ_PARAM ;i++)
                {
                    updatePEQ(command->payload+NUM_WORDS_WECHAT_CMD_HDR +(i*NUM_WORDS_WECHAT_CMD_PER_EQ_PARAM ));
                }
                
                if (KalimbaSendLongMessage(DSP_WECHAT_MSG_SET_USER_GROUP_PARAM,size/NUM_WORDS_WECHAT_CMD_HDR  + 1,dspMsgPayload))
                {
#ifdef ENABLE_WECHAT_PERSISTENT_USER_EQ_BANK
                        
                    /* EQ settings in PS Store should be updated once the user-defined inactivity period has expired */
                    if(theSink.conf1->timeouts.StoreCurrentPEQSettingsTimeout_s)
                    {
                        MessageCancelAll(&theSink.task, EventSysWechatEQChangesStoreTimeout);
                        MessageSendLater(&theSink.task, EventSysWechatEQChangesStoreTimeout , 0, D_SEC(theSink.conf1->timeouts.StoreCurrentPEQSettingsTimeout_s));
                    }
#endif

#if defined ENABLE_PEER && defined PEER_TWS
                    /*Update the peer with the user EQ settings */
                    peerSendUserEqSettings();
#endif
                    wechat_send_success(WECHAT_COMMAND_SET_USER_EQ_GROUP_PARAMETER);
                }
                
                else
                {
                    wechat_send_incorrect_state(WECHAT_COMMAND_SET_USER_EQ_GROUP_PARAMETER);
                }
                
                free(dspMsgPayload);
            }
            
            else
            {
                wechat_send_insufficient_resources(WECHAT_COMMAND_SET_USER_EQ_GROUP_PARAMETER);
            }
            
            return TRUE;     
        }
        
    case WECHAT_COMMAND_GET_USER_EQ_PARAMETER:
        if (!KalimbaSendMessage(DSP_WECHAT_MSG_GET_USER_PARAM,(command->payload[0]<<8)^command->payload[1],0,0,0))
        {
             wechat_send_incorrect_state(WECHAT_COMMAND_GET_USER_EQ_PARAMETER);
        }
        
        /* no "wechat_send_success" as this is done when message is returned from DSP */
        return TRUE;     

    case WECHAT_COMMAND_GET_USER_EQ_GROUP_PARAMETER:
        {
            uint16 size = command->size_payload;
            uint16 *dspMsgPayload = mallocPanic(size/2);
            uint16 i;
            
            if (dspMsgPayload)
            {
                for(i=0;i<(size/2);i++)
                    dspMsgPayload[i] = (command->payload[i*2]<<8)^command->payload[i*2+1];
                
                if (!KalimbaSendLongMessage(DSP_WECHAT_MSG_GET_USER_GROUP_PARAM,size/2,dspMsgPayload))
                {
                    wechat_send_incorrect_state(WECHAT_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
                }
                
                /* no "wechat_send_success" as this is done when message is returned from DSP */
                free(dspMsgPayload);
            }
            
            else
            {
                wechat_send_insufficient_resources(WECHAT_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
            }
            
            return TRUE;     
        }
        
        
    case WECHAT_COMMAND_ENTER_BT_PAIRING_MODE:
        {
            sinkState sink_state = stateManagerGetState();
            
            if (sink_state == deviceLimbo || sink_state == deviceConnDiscoverable)
            {
                wechat_send_incorrect_state(WECHAT_COMMAND_ENTER_BT_PAIRING_MODE);
            }
            
            else
            {
                send_app_message(EventUsrEnterPairing);
                wechat_send_success(WECHAT_COMMAND_ENTER_BT_PAIRING_MODE);
            }
        }
        return TRUE;
        
        
#if defined ENABLE_PEER && defined PEER_TWS
    case WECHAT_COMMAND_SET_TWS_AUDIO_ROUTING:
        if (command->size_payload == 2 && command->payload[0] < 2 && command->payload[1] < 4)
            wechat_set_tws_routing(command->payload[0], command->payload[1]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_SET_TWS_AUDIO_ROUTING);
        
        return TRUE;
        
        
    case WECHAT_COMMAND_GET_TWS_AUDIO_ROUTING:
        if (command->size_payload == 1 && command->payload[0] < 2)
            wechat_send_tws_routing(command->payload[0]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_GET_TWS_AUDIO_ROUTING);
        
        return TRUE;
        
                
    case WECHAT_COMMAND_TRIM_TWS_VOLUME:
        if (command->size_payload == 2)
            wechat_trim_tws_volume(command->payload[0], command->payload[1]);
        
        else
            wechat_send_invalid_parameter(WECHAT_COMMAND_TRIM_TWS_VOLUME);
        
        return TRUE;
#endif /* defined ENABLE_PEER && defined PEER_TWS */
                
    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    wechat_handle_status_command
    
DESCRIPTION
    Handle a Wechat polled status command or return FALSE if we can't
*/
static bool wechat_handle_status_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{       
    switch (command->command_id)
    {
    case WECHAT_COMMAND_GET_APPLICATION_VERSION:
        wechat_send_application_version();
        return TRUE;
                   
    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    wechat_handle_feature_command
    
DESCRIPTION
    Handle a Wechat feature command or return FALSE if we can't
*/
static bool wechat_handle_feature_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{
    switch (command->command_id)
    {
    case WECHAT_COMMAND_SET_FEATURE:
        wechat_set_feature(command->size_payload, command->payload);
        return TRUE;
        
    case WECHAT_COMMAND_GET_FEATURE:
        wechat_get_feature(command->size_payload, command->payload);
        return TRUE;
        
    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    wechat_handle_notification_command
    
DESCRIPTION
    Handle a Wechat notification command or return FALSE if we can't
    Notification acknowledgements are swallowed
*/
static bool wechat_handle_notification_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{
    if (command->command_id & WECHAT_ACK_MASK)
    {
        WECHAT_DEBUG(("G: NOTIF ACK\n")); 
        return TRUE;
    }
    
    switch (command->command_id)
    {
    case WECHAT_COMMAND_REGISTER_NOTIFICATION:
        wechat_register_notification(command->size_payload, command->payload);
        return TRUE;
        
    case WECHAT_COMMAND_GET_NOTIFICATION:
        wechat_get_notification(command->size_payload, command->payload);
        return TRUE;
        
    case WECHAT_COMMAND_CANCEL_NOTIFICATION:
        wechat_cancel_notification(command->size_payload, command->payload);
        return TRUE;
        
    case WECHAT_EVENT_NOTIFICATION:
        wechat_send_invalid_parameter(WECHAT_EVENT_NOTIFICATION);
        return TRUE;
        
    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    wechat_handle_command
    
DESCRIPTION
    Handle a WECHAT_UNHANDLED_COMMAND_IND from the Wechat library
*/
static void wechat_handle_command(Task task, WECHAT_UNHANDLED_COMMAND_IND_T *command)
{
    bool handled = FALSE;
    
    WECHAT_DEBUG(("G: cmd: %04x:%04x %d\n", 
                command->vendor_id, command->command_id, command->size_payload));
        
    if (command->vendor_id == WECHAT_VENDOR_CSR)
    {
        switch (command->command_id & WECHAT_COMMAND_TYPE_MASK)
        {
        case WECHAT_COMMAND_TYPE_CONFIGURATION:
            handled = wechat_handle_configuration_command(task, command);
            break;
            
        case WECHAT_COMMAND_TYPE_CONTROL:
            handled = wechat_handle_control_command(task, command);
            break;
            
        case WECHAT_COMMAND_TYPE_STATUS :
            handled = wechat_handle_status_command(task, command);
            break;
            
        case WECHAT_COMMAND_TYPE_FEATURE:
            handled = wechat_handle_feature_command(task, command);
            break;
                        
        case WECHAT_COMMAND_TYPE_NOTIFICATION:
             handled = wechat_handle_notification_command(task, command);
            break;
            
        default:
            WECHAT_DEBUG(("G: type unknown\n"));
            break;
        }
    }
        
    if (!handled)
        wechat_send_response(command->vendor_id, command->command_id, WECHAT_STATUS_NOT_SUPPORTED, 0, NULL);
}


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

#ifdef CVC_PRODTEST
            if (BootGetMode() != BOOTMODE_CVC_PRODTEST)
            {
                WECHAT_DEBUG(("G: StartService\n"));
                if (m->success)
                {
#ifdef ENABLE_WECHAT_SPP
                    WechatStartService(wechat_transport_spp);
#else
                    WechatStartService(wechat_transport_rfcomm);
#endif /* ENABLE_WECHAT_SPP */
                }
            }
            else
            {
                if (m->success)
                {
                    WECHAT_DEBUG(("G: CVC_PRODTEST_PASS\n"));
                    exit(CVC_PRODTEST_PASS);
                }
                else
                {
                    WECHAT_DEBUG(("G: CVC_PRODTEST_FAIL\n"));
                    exit(CVC_PRODTEST_FAIL);
                }
            }
#else
            if (m->success)
            {
                WechatSetApiMinorVersion(WECHAT_API_MINOR_VERSION);
#ifdef ENABLE_WECHAT_SPP
                WechatStartService(wechat_transport_spp);
#else
                WechatStartService(wechat_transport_rfcomm);
#endif /* ENABLE_WECHAT_SPP */
            }
#endif /*CVC_PRODTEST */
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

			koovox_pack_wechat_auth_req();
			WechatSendPacket(m->transport, g_send_data.len, g_send_data.data);
        }
        break;
        
    case WECHAT_DISCONNECT_IND:
        {
            WECHAT_DISCONNECT_IND_T *m = (WECHAT_DISCONNECT_IND_T *) message;
            WECHAT_DEBUG(("G: _DISCONNECT_IND: %04x\n", (uint16) m->transport));
            wechat_handle_disconnect();
            WechatDisconnectResponse(m->transport);
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
        
    case WECHAT_UNHANDLED_COMMAND_IND:
        WECHAT_DEBUG(("G: _UNHANDLED_COMMAND_IND\n"));
        wechat_handle_command(task, (WECHAT_UNHANDLED_COMMAND_IND_T *) message);
        break;

    case WECHAT_SEND_PACKET_CFM:
        {
            WECHAT_SEND_PACKET_CFM_T *m = (WECHAT_SEND_PACKET_CFM_T *) message;
            WECHAT_DEBUG(("G: _SEND_PACKET_CFM: s=%d\n", VmGetAvailableAllocations()));
            
            if (m->packet)
                freePanic(m->packet);
        }
        break;

	case WECHAT_MESSAGE_MORE_DATA:
		{
			WECHAT_MESSAGE_MORE_DATA_T* m = (WECHAT_MESSAGE_MORE_DATA_T*)message;	
			uint16 size = m->size_value;
			uint16 i = 0;
            WECHAT_DEBUG(("WECHAT_MESSAGE_MORE_DATA:size = %d\n", size));

			for(; i<size; i++)
				DEBUG(("%x ", m->value[i]));
			DEBUG(("\n"));
		}
		break;
        
    default:
        WECHAT_DEBUG(("G: unhandled message %04x\n", id));
        break;
    }
}

/*************************************************************************
NAME
    handleDfuSqifStatus
    
DESCRIPTION
    Handle MESSAGE_DFU_SQIF_STATUS from the loader
*/
void handleDfuSqifStatus(MessageDFUFromSQifStatus *message) 
{
    wechat_data.dfu_boot_status = message->status;
    send_app_message(EventUsrPowerOn);
}

/*************************************************************************
NAME
    wechatReportPioChange
    
DESCRIPTION
    Relay any registered PIO Change events to the Wechat client
*/
void wechatReportPioChange(uint32 pio_state)
{
    uint8 payload[8];

    if ((pio_state ^ wechat_data.pio_old_state) & wechat_data.pio_change_mask) 
    {  
        dwunpack(payload + 4, VmGetClock());
        dwunpack(payload, pio_state);        
        WECHAT_DEBUG(("G: _EVENT_PIO_CHANGED: %08lx\n", pio_state));
        wechat_send_notification(WECHAT_EVENT_PIO_CHANGED, sizeof payload, payload);
    }
        
    wechat_data.pio_old_state = pio_state;
}


/*************************************************************************
NAME
    wechatReportEvent
    
DESCRIPTION
    Relay any significant application events to the Wechat client
*/
void wechatReportEvent(uint16 id)
{
    uint8 payload[1];
    
    switch (id)
    {
    case EventSysChargeComplete:
        if (wechat_data.notify_battery_charged)
            wechat_send_notification(WECHAT_EVENT_BATTERY_CHARGED, 0, NULL);
        
        break;

    case EventUsrChargerConnected:
        if (wechat_data.notify_charger_connection)
        {
            payload[0] = 1;
            wechat_send_notification(WECHAT_EVENT_CHARGER_CONNECTION, 1, payload);
        }
        break;

    case EventUsrChargerDisconnected:
        if (wechat_data.notify_charger_connection)
        {
            payload[0] = 0;
            wechat_send_notification(WECHAT_EVENT_CHARGER_CONNECTION, 1, payload);
        }
        break;
    }
}


/*************************************************************************
NAME
    wechatReportUserEvent
    
DESCRIPTION
    Relay any user-generated events to the Wechat client
*/
void wechatReportUserEvent(uint16 id)
{
    if (wechat_data.wechat_transport != NULL)
    {
        WECHAT_DEBUG(("G: ev %04x en %u\n", id, wechat_data.notify_ui_event));
           
        if (wechat_data.notify_ui_event)
        {
            uint8 payload[2];
            
            switch (id)
            {
        /*  Avoid redundant and irrelevant messages  */
            case EventSysLongTimer:
            case EventSysVLongTimer:
            case EventUsrLedsOnOffToggle:
                break;
                
            default:
                payload[0] = id >> 8;
                payload[1] = id & 0xFF;
                wechat_send_notification(WECHAT_EVENT_USER_ACTION, 2, payload);
                break;
            }
        }
    }
}

/*************************************************************************
NAME
    wechatDfuRequest
    
DESCRIPTION
    Request Device Firmware Upgrade from the WECHAT host
*/
void wechatDfuRequest(void)
{
    if ((wechat_data.wechat_transport != NULL) &&            
        (wechat_data.dfu_partition != WECHAT_ILLEGAL_PARTITION))
        WechatDfuRequest(wechat_data.wechat_transport, 
                       wechat_data.dfu_partition, 
                       WECHAT_DFU_REQUEST_TIMEOUT);
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


#ifdef ENABLE_SPEECH_RECOGNITION
/*************************************************************************
NAME
    wechatReportSpeechRecResult
    
DESCRIPTION
    Relay a speech recognition result to the Wechat client
*/
void wechatReportSpeechRecResult(uint16 id)
{
    if (wechat_data.notify_speech_rec)
    {
        uint8 payload[1];
        bool ok = TRUE;
        
        switch (id)
        {
        case CSR_SR_WORD_RESP_YES:
            payload[0] = WECHAT_ASR_RESPONSE_YES;
            break;
    
        case CSR_SR_WORD_RESP_NO:  
            payload[0] = WECHAT_ASR_RESPONSE_NO;
            break;
            
        case CSR_SR_WORD_RESP_FAILED_YES:
        case CSR_SR_WORD_RESP_FAILED_NO:
        case CSR_SR_WORD_RESP_UNKNOWN:
        case CSR_SR_APP_TIMEOUT:
        /*  Known unknowns  */
            payload[0] = WECHAT_ASR_RESPONSE_UNRECOGNISED;
            break;
    
        default:
            WECHAT_DEBUG(("wechat: ASR: %04x?\n", id));
            ok = FALSE;
            break;
        }
        
        if (ok)
            wechat_send_notification(WECHAT_EVENT_SPEECH_RECOGNITION, 1, payload);
    }
}
#endif  /*  ifdef ENABLE_SPEECH_RECOGNITION  */

#else
static const int dummy;  /* ISO C forbids an empty source file */

#endif  /*  ifdef ENABLE_WECHAT  */
