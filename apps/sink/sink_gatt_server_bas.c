/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2014

FILE NAME
    sink_gatt_server_bas.c

DESCRIPTION
    Implementation of the GATT Battery Service
*/
#if defined(GATT_SERVER_ENABLED) && defined(GATT_SERVER_BAS_ENABLED) && defined(ENABLE_KOOVOX)

/* Application includes */
#include "sink_gatt_server_bas.h"
#include "sink_private.h"
#include "sink_gatt.h"
#include "sink_gatt_db.h"

#include "sink_app_message.h"
#include "sink_app_task.h"
#include "sink_app_core.h"

/* Library includes */
#include <gatt.h>
#include <power.h>

/* Firmware includes */
#include <csrtypes.h>

/* Macro for GATT SERVER Debug */
#ifdef DEBUG_GATT
#include <stdio.h>
#define GATT_SERVER_BAS_DEBUG(x) DEBUG(x)
#else
#define GATT_SERVER_BAS_DEBUG(x) 
#endif

/******************************************************************************/
void handle_heart_rate_service_access(GATT_ACCESS_IND_T * ind)
{
    GATT_SERVER_BAS_DEBUG(("HEART RATE SERVICE [%x]\n", ind->flags));
    GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
}


/******************************************************************************/
void handle_heart_rate_measurement_access(GATT_ACCESS_IND_T * ind)
{
    /* Can only read this characteristic */
    if (ind->flags & ATT_ACCESS_READ)
    {        
        /* Respond to the request with the current heart rate */
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Heart Rate=[%x]\n", koovox.heartRateValue));
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 1, &koovox.heartRateValue);
    }
    else
    {
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Heart Rate FLAGS[%x]\n", ind->flags));
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
    }
}


/******************************************************************************/
void handle_heart_rate_measurement_c_cfg_access(GATT_ACCESS_IND_T * ind)
{
    GATT_SERVER_BAS_DEBUG(("FLAGS[%x] SIZE[%x]\n", ind->flags, ind->size_value));
    
    if (ind->flags & ATT_ACCESS_READ)
    {
        /* Convert from UINT16 to UINT8 array and return current value of C.CFG descriptor for Heart Rate */
        uint8 heart_ccfg_value[2];
        heart_ccfg_value[0] = theSink.rundata->gatt.server.bas_service.heart_rate_notify & 0xFF;
        heart_ccfg_value[1] = (theSink.rundata->gatt.server.bas_service.heart_rate_notify >> 8) & 0xFF;         
        
        GATT_SERVER_BAS_DEBUG(("GATT REQ: READ Heart Rate C.CFG[%x]\n", theSink.rundata->gatt.server.bas_service.heart_rate_notify));
        GattAccessResponse(ind->cid, ind->handle, 0, 2, heart_ccfg_value);
    }
    else if (ind->flags & ATT_ACCESS_WRITE)
    {
        /* Check the size of the data requested to write to the client config is correct */
        if (ind->size_value == GATT_CLIENT_CONFIG_OCTET_SIZE)
        {
            /* Update the C.CFG descriptor for the Heart Rate */
            GATT_SERVER_BAS_DEBUG(("GATT REQ: WRITE Heart Rate C.CFG[%x] new[%x,%x]\n", theSink.rundata->gatt.server.bas_service.heart_rate_notify, ind->value[0], ind->value[1]));
            theSink.rundata->gatt.server.bas_service.heart_rate_notify = ind->value[0];
            
            /* Respond to the write request */
            GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, NULL);
            
            /* If notifications were enabled, send one */
            send_heart_rate_notification_to_device(ind->cid);
            
            /* If indications were enabled, send one */
            send_heart_rate_indication_to_device(ind->cid);
        }
        else
        {
            /* Requested data to write is the wrong length, respond to the device with appropriate error */
            GattAccessResponse(ind->cid, ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
}


/******************************************************************************/
void send_heart_rate_notification_to_device(uint16 cid)
{
    /* Only send a notification if the remote device has enabled heart rate notifications */
    if (theSink.rundata->gatt.server.bas_service.heart_rate_notify & GATT_CLIENT_CONFIG_NOTIFY_BIT)
    {        
        GATT_SERVER_BAS_DEBUG(("GATT : Send heart rate notification, rate=%x\n", theSink.rundata->gatt.server.bas_service.heart_rate));
        
        /* Send a notification (containing current heart rate) to the device */
        GattNotificationRequest(&theSink.rundata->gatt.task, cid, HANDLE_HEART_RATE_MEASUREMENT, 1, &theSink.rundata->gatt.server.bas_service.heart_rate);
    }
}


/******************************************************************************/
void send_heart_rate_indication_to_device(uint16 cid)
{
	/* Only send a notification if the remote device has enabled battery level notifications */
	if (theSink.rundata->gatt.server.bas_service.heart_rate_notify & GATT_CLIENT_CONFIG_INICATE_BIT)
	{
		/* Only send indication if there is no other indication in progress */
		if (theSink.rundata->gatt.server.indication == indication_none)
		{
			/* Set the indication flag as battery level indication is now in progress */
			theSink.rundata->gatt.server.indication = indication_heart_rate_measurement;
					
			GATT_SERVER_BAS_DEBUG(("GATT : Send heart rate indication, rate=%x\n", theSink.rundata->gatt.server.bas_service.heart_rate));
			
			/* Send an indication (containing current heart rate) to the device */
			GattIndicationRequest(&theSink.rundata->gatt.task, cid, HANDLE_HEART_RATE_MEASUREMENT, 1, &theSink.rundata->gatt.server.bas_service.heart_rate);
		}
		else
		{
			/* TODO : Queue heart rate indication to send once other indication(s) have completed */
		}
	}
}


/******************************************************************************/
void handle_heart_rate_indication_cfm(GATT_INDICATION_CFM_T * ind)
{
    /* Has the indication just been sent to the device? */
    if (ind->status == gatt_status_success_sent)
    {
        /* Now wait for device to respond to the heart rate indication */
    }
    else
    {
        if (ind->status == gatt_status_success)
        {
            /* Remote device has responded to the indication, so heart rate indication has now completed, clear the flag */
            theSink.rundata->gatt.server.indication = indication_none;
            
            /* TODO : Optional, could ensure indication data returned by the client is correct */
            
            GATT_SERVER_BAS_DEBUG(("GATT_INDICATION_CFM - Heart rate Success\n"));
        }
        else
        {
            /* Some error occured sending the indication to the remote device */
            theSink.rundata->gatt.server.indication = indication_none;
            
            GATT_SERVER_BAS_DEBUG(("GATT_INDICATION_CFM - Heart rate fail\n"));
        }
    }
}



/******************************************************************************/
void handle_battery_service_access(GATT_ACCESS_IND_T * ind)
{
    GATT_SERVER_BAS_DEBUG(("BATT SERVICE [%x]\n", ind->flags));
    GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
}


/******************************************************************************/
void handle_battery_level_access(GATT_ACCESS_IND_T * ind)
{
    /* Can only read this characteristic */
    if (ind->flags & ATT_ACCESS_READ)
    {
        /* Get the battery level as a value 0-100 */
        theSink.rundata->gatt.server.bas_service.battery_level = get_battery_level_as_percentage();
        
        /* Respond to the request with the current battery level */
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Battery Level=[%x]\n", theSink.rundata->gatt.server.bas_service.battery_level));
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 1, &theSink.rundata->gatt.server.bas_service.battery_level );
    }
    else
    {
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Battery Level FLAGS[%x]\n", ind->flags));
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
    }
}


/******************************************************************************/
void handle_battery_level_c_cfg_access(GATT_ACCESS_IND_T * ind)
{
    GATT_SERVER_BAS_DEBUG(("FLAGS[%x] SIZE[%x]\n", ind->flags, ind->size_value));
    
    if (ind->flags & ATT_ACCESS_READ)
    {
        /* Convert from UINT16 to UINT8 array and return current value of C.CFG descriptor for Battery Level */
        uint8 bat_ccfg_value[2];
        bat_ccfg_value[0] = theSink.rundata->gatt.server.bas_service.battery_notify & 0xFF;
        bat_ccfg_value[1] = (theSink.rundata->gatt.server.bas_service.battery_notify >> 8) & 0xFF;         
        
        GATT_SERVER_BAS_DEBUG(("GATT REQ: READ Battery Level C.CFG[%x]\n", theSink.rundata->gatt.server.bas_service.battery_notify));
        GattAccessResponse(ind->cid, ind->handle, 0, 2, bat_ccfg_value);
    }
    else if (ind->flags & ATT_ACCESS_WRITE)
    {
        /* Check the size of the data requested to write to the client config is correct */
        if (ind->size_value == GATT_CLIENT_CONFIG_OCTET_SIZE)
        {
            /* Update the C.CFG descriptor for the Battery Level */
            GATT_SERVER_BAS_DEBUG(("GATT REQ: WRITE Battery Level C.CFG[%x] new[%x,%x]\n", theSink.rundata->gatt.server.bas_service.battery_notify, ind->value[0], ind->value[1]));
            theSink.rundata->gatt.server.bas_service.battery_notify = ind->value[0];
            
            /* Respond to the write request */
            GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, NULL);
            
            /* If notifications were enabled, send one */
            send_battery_level_notification_to_device(ind->cid);
            
            /* If indications were enabled, send one */
            send_battery_level_indication_to_device(ind->cid);
        }
        else
        {
            /* Requested data to write is the wrong length, respond to the device with appropriate error */
            GattAccessResponse(ind->cid, ind->handle, gatt_status_invalid_length, 0, NULL);
        }
    }
}


/******************************************************************************/
void send_battery_level_notification_to_device(uint16 cid)
{
    /* Only send a notification if the remote device has enabled battery level notifications */
    if (theSink.rundata->gatt.server.bas_service.battery_notify & GATT_CLIENT_CONFIG_NOTIFY_BIT)
    {
        /* Get the current battery level as a value 0-100 */
        theSink.rundata->gatt.server.bas_service.battery_level = get_battery_level_as_percentage();
        
        GATT_SERVER_BAS_DEBUG(("GATT : Send battery level notification, level=%x\n", theSink.rundata->gatt.server.bas_service.battery_level));
        
        /* Send a notification (containing current battery level) to the device */
        GattNotificationRequest(&theSink.rundata->gatt.task, cid, HANDLE_BATTERY_LEVEL, 1, &theSink.rundata->gatt.server.bas_service.battery_level);
    }
}


/******************************************************************************/
void send_battery_level_indication_to_device(uint16 cid)
{
    /* Only send a notification if the remote device has enabled battery level notifications */
    if (theSink.rundata->gatt.server.bas_service.battery_notify & GATT_CLIENT_CONFIG_INICATE_BIT)
    {
        /* Only send indication if there is no other indication in progress */
        if (theSink.rundata->gatt.server.indication == indication_none)
        {
            /* Set the indication flag as battery level indication is now in progress */
            theSink.rundata->gatt.server.indication = indication_battery_level;
            
            /* Get the current battery level as a value 0-100 */
            theSink.rundata->gatt.server.bas_service.battery_level = get_battery_level_as_percentage();
            
            GATT_SERVER_BAS_DEBUG(("GATT : Send battery level notification, level=%x\n", theSink.rundata->gatt.server.bas_service.battery_level));
            
            /* Send an indication (containing current battery level) to the device */
            GattIndicationRequest(&theSink.rundata->gatt.task, cid, HANDLE_BATTERY_LEVEL, 1, &theSink.rundata->gatt.server.bas_service.battery_level);
        }
        else
        {
            /* TODO : Queue battery indication to send once other indication(s) have completed */
        }
    }
}


/******************************************************************************/
void handle_battery_level_indication_cfm(GATT_INDICATION_CFM_T * ind)
{
    /* Has the indication just been sent to the device? */
    if (ind->status == gatt_status_success_sent)
    {
        /* Now wait for device to respond to the battery level indication */
    }
    else
    {
        if (ind->status == gatt_status_success)
        {
            /* Remote device has responded to the indication, so battery level indication has now completed, clear the flag */
            theSink.rundata->gatt.server.indication = indication_none;
            
            /* TODO : Optional, could ensure indication data returned by the client is correct */
            
            GATT_SERVER_BAS_DEBUG(("GATT_INDICATION_CFM - Bat Level Success\n"));
        }
        else
        {
            /* Some error occured sending the indication to the remote device */
            theSink.rundata->gatt.server.indication = indication_none;
            
            GATT_SERVER_BAS_DEBUG(("GATT_INDICATION_CFM - Bat Level fail\n"));
        }
    }
}


/******************************************************************************/
void handle_koovox_service_access(GATT_ACCESS_IND_T * ind)
{
    GATT_SERVER_BAS_DEBUG(("KOOVOX_SERVICE [%x]\n", ind->flags));
    GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
}


/******************************************************************************/
void handle_get_characteristic_access(GATT_ACCESS_IND_T * ind)
{
    /* Can only read this characteristic */
    if (ind->flags & ATT_ACCESS_READ)
    {   
    	GATT_SERVER_BAS_DEBUG(("GET_CHARA:size_value = %x\n", ind->size_value));
		/* dispose the app get value */
		DisposeGattGetCharacteristicValue(ind);
    }
    else
    {
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Get_Charac FLAGS[%x]\n", ind->flags));
		
#ifdef DEBUG_APP_MESSAGE
		{
			uint8 i = 0;
			DEBUG(("msg:"));
			for(; i < ind->size_value; i++)
			{
				DEBUG(("%x ", ind->value[i]));
			}
			DEBUG(("\n"));
		}
#endif	
		memcpy(koovox.getValue + koovox.sizeGetValue, ind->value, ind->size_value);
		koovox.sizeGetValue += ind->size_value;
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);
    }
}

/******************************************************************************/
void handle_set_characteristic_access(GATT_ACCESS_IND_T * ind)
{
    /* Can only write this characteristic */
    if (ind->flags & ATT_ACCESS_WRITE)
    {   
    	GATT_SERVER_BAS_DEBUG(("SET_CHARA:size_value = %x\n", ind->size_value));
#ifdef DEBUG_APP_MESSAGE
		{
			uint8 i = 0;
			DEBUG(("msg:"));
			for(; i < ind->size_value; i++)
			{
				DEBUG(("%x ", ind->value[i]));
			}
			DEBUG(("\n"));
		}
#endif	

		memcpy(koovox.setValue + koovox.sizeSetValue, ind->value, ind->size_value);
		koovox.sizeSetValue += ind->size_value;
        GattAccessResponse(ind->cid, ind->handle, gatt_status_success, 0, 0);

    }
    else 
    {
        GATT_SERVER_BAS_DEBUG(("GATT REQ: Set_Charac FLAGS[%x]\n", ind->flags));

		/* dispose the app set value */
		DisposeGattSetCharacteristicValue(ind);
    }
}




/******************************************************************************/


#else
#include <csrtypes.h>
static const uint16 gatt_server_bas = 0;
#endif /* defined(GATT_SERVER_ENABLED) && defined(GATT_SERVER_BAS_ENABLED)*/
