/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2014

FILE NAME
    sink_gatt_server.c

DESCRIPTION
    Framework for GATT Services when device is configured as a GATT Server
*/
#ifdef GATT_SERVER_ENABLED

/* Application includes */
#include "sink_gatt_server.h"
#include "sink_private.h"
#include "sink_debug.h"
#include "sink_gatt_server_bas.h"
#include "sink_gatt.h"
#include "sink_gatt_db.h"
#include "sink_statemanager.h"
#include "sink_koovox_core.h"
#include "epb_MmBp.h"
#include "koovox_wechat_handle.h"

#ifdef BLE_ENABLED
#include "sink_ble_advertising.h"
#endif

/* Library includes */
#include <connection.h>
#include <gatt.h>

/* Firmware includes */
#include <stdlib.h>
#include <string.h>


/* Macro for GATT SERVER Debug */
#ifdef DEBUG_GATT
#include <stdio.h>
#define GATT_SERVER_DEBUG(x) DEBUG(x)
#else
#define GATT_SERVER_DEBUG(x) 
#endif

typedef enum
{
    GATT_CLIENT_DEVICE_ADDED,
    GATT_CLIENT_DEVICE_KNOWN,
    GATT_CLIENT_MAX_DEVICES

} gattAddDeviceCode_t;


/*******************************************************************************
FUNCTION
    add_client_device
    
DESCRIPTION
    Helper function to add a client device to the known devices list
*/  
static gattAddDeviceCode_t add_client_device(uint16 cid)
{
    /* Ensure device is not already known */
    uint16 i;
    for (i=0; i<MAX_GATT_CLIENT_DEVICES; i++)
    {
        if (theSink.rundata->gatt.server.devs[i].cid == cid)
        {
            GATT_SERVER_DEBUG(("GATT: Client[%x] already known\n", cid));
            return GATT_CLIENT_DEVICE_KNOWN;
        }
    }
    
    /* Add device (if possible) */
    for (i=0; i<MAX_GATT_CLIENT_DEVICES; i++)
    {
        if (theSink.rundata->gatt.server.devs[i].state == gatt_client_state_disconnected)
        {
            /* Add new device */
            theSink.rundata->gatt.server.devs[i].cid   = cid;
            theSink.rundata->gatt.server.devs[i].state = gatt_client_state_connected;
            return GATT_CLIENT_DEVICE_ADDED;
        }
    }
    
    return GATT_CLIENT_MAX_DEVICES;
}


/*******************************************************************************
FUNCTION
    remove_client_device
    
DESCRIPTION
    Helper function to remove a client device from the known devices list
*/  
static void remove_client_device(uint16 cid)
{
    /* Find the device in the known devices list and remove it */
    uint16 i;
    for (i=0; i<MAX_GATT_CLIENT_DEVICES; i++)
    {
        if (theSink.rundata->gatt.server.devs[i].cid == cid)
        {
            theSink.rundata->gatt.server.devs[i].cid   = 0;
            theSink.rundata->gatt.server.devs[i].state = gatt_client_state_disconnected;
        }
    }
}

/******************************************************************************/
void server_handle_gatt_init_cfm(GATT_INIT_CFM_T * cfm)
{
    if (cfm->status == gatt_status_success)
    {
        theSink.rundata->gatt.initialised = TRUE;
    }
    
    if (cfm->status == gatt_status_success)
    {
        /* Set the BLE role to SLAVE and wait for a connection from a remote device */
        GattConnectRequest(&theSink.rundata->gatt.task, NULL, gatt_connection_ble_slave_undirected, FALSE);        
    }
}


/******************************************************************************/
void server_handle_gatt_connect_cfm(GATT_CONNECT_CFM_T * cfm)
{
    switch(cfm->status)
    {
        case gatt_status_initialising:
        {
            GATT_SERVER_DEBUG(("GATT: Connection initialising\n"));
            #if defined(BLE_ENABLED)
            {
                /* Start Advertising so a remote device can find and connect */
				if(deviceConnected <= stateManagerGetState())
				{
					start_ble_advertising();
				}
            }
            #endif
        }
        break;
        case gatt_status_success:
        {
            /* Ensure device was able to be added */
            if (add_client_device(cfm->cid) == GATT_CLIENT_DEVICE_ADDED)
            {
                GATT_SERVER_DEBUG(("GATT: Connect success - stop advertising\n"));
                
                #if defined(BLE_ENABLED)
                {
                    stop_ble_advertising();
                }
                #endif
            }
        }
        break;
        default:
        {
            GATT_SERVER_DEBUG(("GATT: Connect failed %x\n", cfm->status));
        }
    }
}


/******************************************************************************/
void server_handle_gatt_disconnect_ind(GATT_DISCONNECT_IND_T * ind)
{
    GATT_SERVER_DEBUG(("GATT: Disconnected\n"));
    
    /* Remove device from known devices list */
    remove_client_device(ind->cid);
    
    #if defined(BLE_ENABLED)
    {
        start_ble_advertising();
    }
    #endif
    
    GATT_SERVER_DEBUG(("GATT: Wait for a connection from a remote device\n"));
    GattConnectRequest(&theSink.rundata->gatt.task, NULL, gatt_connection_ble_slave_undirected, FALSE);

	koovox_device_wechat_disconnect();
}


/******************************************************************************/
void server_handle_gatt_access_ind(GATT_ACCESS_IND_T * ind)
{

    /* Which GATT declaration/characteristic/descriptor has been requested? */
    bool is_valid = FALSE;

	DEBUG(("server_handle_gatt_access_ind: %x\n", ind->handle));
    
    /* Was the GATT characteristic requested? */
    if (ind->handle == HANDLE_GATT_SERVICE)
    {
        handle_gatt_service_access(ind);
        is_valid = TRUE;
    }
    
    /* Was a GAP characteristic requested? */
    if (ind->handle == HANDLE_GAP_SERVICE)
    {
        handle_gap_service_access(ind);
        is_valid = TRUE;
    }
    else if (ind->handle == HANDLE_DEVICE_NAME)
    {
        handle_device_name_access(ind);
        is_valid = TRUE;
    }
    else if (ind->handle == HANDLE_DEVICE_APPEARANCE)
    {
         handle_device_appearance_access(ind);
         is_valid = TRUE;
    }
    
#ifdef GATT_SERVER_BAS_ENABLED
    /* Was a BAS characteristic requested? */
    if (ind->handle == HANDLE_BATTERY_SERVICE)
    {
        handle_battery_service_access(ind);
        is_valid = TRUE;
    }
    else if (ind->handle == HANDLE_BATTERY_LEVEL)
    {
        handle_battery_level_access(ind);
        is_valid = TRUE;
    }
    else if (ind->handle == HANDLE_BATTERY_LEVEL_CLIENT_CONFIG)
    {
        handle_battery_level_c_cfg_access(ind);
        is_valid = TRUE;
    }
#endif

#ifdef ENABLE_KOOVOX
	if(ind->handle == HANDLE_WeChat)
	{
		GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
        is_valid = TRUE;
	}
	else if(ind->handle == HANDLE_WRITE_CHARACTERISTIC)
	{
        is_valid = TRUE;

		DEBUG(("flags: %x, offset:%x \n", ind->flags, ind->offset));
		
		GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);

		if(ind->flags & ATT_ACCESS_WRITE)
		{
			/* 处理消息 */
			koovox_write_from_wechat(ind->value, ind->size_value);
			
			/* 发送init req */
			if((g_wechatble_state.auth_state) 
				&& (!g_wechatble_state.init_state) 
				&& (!g_wechatble_state.init_send))
			{
				koovox_pack_wechat_init_req();
				koovox_indicate_to_wechat(ind->cid, HANDLE_WECHAR_CLIENT_CONFIG);
				g_wechatble_state.init_send = TRUE;
			}

#if 1
			/* 测试发送send data req */
			{
			static bool test_flag = FALSE;
			
			if((g_wechatble_state.init_state)&&(!test_flag))
			{
				uint8 device_name[] = {'K', 'O', 'O', 'V','O', 'X'};
				koovox_pack_wechat_send_data_req((uint8*)device_name, 6, TRUE, EDDT_manufatureSvr);
				koovox_indicate_to_wechat(ind->cid, HANDLE_WECHAR_CLIENT_CONFIG);
				test_flag = TRUE;
			}
			}
#endif
		}

	}
	else if(ind->handle == HANDLE_READ_CHARACTERISTIC)
	{
		uint8 value[6] = {0};
		KoovoxGetBluetoothAdrress((uint8*)value);
		is_valid = TRUE;
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 6, value);
	}
	else if(ind->handle == HANDLE_INDICATE_CHARACTERISTIC)
	{
		is_valid = TRUE;
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
	}
	else if(ind->handle == HANDLE_WECHAR_CLIENT_CONFIG)
	{
		if (ind->flags & ATT_ACCESS_READ)
		{
			GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
		}
		else if (ind->flags & ATT_ACCESS_WRITE)
		{
			/* Check the size of the data requested to write to the client config is correct */
			if (ind->size_value == GATT_CLIENT_CONFIG_OCTET_SIZE)
			{	 
				GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);

				koovox_handle_wechat_indicate(ind->cid, ind->handle);
				
			}
			else
			{
				g_wechatble_state.indication_state = FALSE;
				
				/* Requested data to write is the wrong length, respond to the device with appropriate error */
				GattAccessResponse(ind->cid, ind->handle, gatt_status_invalid_length, 0, NULL);
			}
		}
		
        is_valid = TRUE;
	}
#endif
    
    if (!is_valid)
    {
        GATT_SERVER_DEBUG(("ERROR : GATT_ACCESS_IND cid[%x] handle[%x] flags[%x] offset[%x] size[%x] value[%x]\n", ind->cid, ind->handle, ind->flags, ind->offset, ind->size_value, ind->value[0]));
    }
}


/******************************************************************************/
void server_handle_gatt_indication_cfm(GATT_INDICATION_CFM_T * ind)
{
    /* Which service is currently sending an indication? */
    switch(theSink.rundata->gatt.server.indication)
    {
        case indication_none:
        {
            GATT_SERVER_DEBUG(("GATT_INDICATION_CFM : ERROR, no service is doing indication\n"));
        }
        break;
		
        case indication_battery_level:
        {
            #ifdef GATT_SERVER_BAS_ENABLED
            {
                handle_battery_level_indication_cfm(ind);
            }
            #endif
        }
        break;
		
		case indication_wechat:
		{
			if(ind->status == gatt_status_success)
			{
				koovox_indicate_to_wechat(ind->cid, HANDLE_WECHAR_CLIENT_CONFIG);
			}
		}
		break;
		
		default:
			break;
    }
	
}


/******************************************************************************/
void disconnect_all_gatt_clients(void)
{
    /* Disconnect all connected devices */
    uint16 i;
    for (i=0; i<MAX_GATT_CLIENT_DEVICES; i++)
    {
        if (theSink.rundata->gatt.server.devs[i].state != gatt_client_state_disconnected)
        {
            GattDisconnectRequest(theSink.rundata->gatt.server.devs[i].cid);
            /* Handle Disconnect IND will remove the device from known devices list */
        }
    }
}


#else /* GATT_SERVER_ENABLED */

#include <csrtypes.h>
static const uint16 gatt_server_dummy = 0;

#endif /* GATT_SERVER_ENABLED */
