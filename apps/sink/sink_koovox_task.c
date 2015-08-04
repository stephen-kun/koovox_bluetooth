/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_task.c

*/

#ifdef ENABLE_KOOVOX

#include <stdio.h>
#include <spps.h>
#include <source.h>
#include <kalimba.h>
#include <pio.h>
#include <pio_common.h>
#include <panic.h>
#include <vm.h>
	
#include "sink_ble_advertising.h"
#include "sink_koovox_task.h"
#include "sink_koovox_core.h"
#include "sink_koovox_message.h"
#include "sink_koovox_uart.h"


/* Single instance of phone app task */
gKoovoxTaskData koovox ;


/*************************************************************************
NAME    
    handleSPPMessage
    
DESCRIPTION
    handles messages from the SPP

RETURNS
    
*/
static void handleSPPMessage ( Task task, MessageId id, Message message )
{
    KOOVOX_TASK_DEBUG(("SPP_U_MSG: [%x][%x][%x]\n", (int)task , (int)id , (int)&message)) ;

    switch(id)
    {
        case SPP_SERVER_CONNECT_CFM:
			{
				SPP_SERVER_CONNECT_CFM_T *sscc = (SPP_SERVER_CONNECT_CFM_T *) message;
                if (spp_connect_success == sscc->status)
                {

					KoovoxAppConnection(sscc->sink);
					
#if defined(BLE_ENABLED)
					stop_ble_advertising();
#endif
					KOOVOX_TASK_DEBUG(("SPP_SERVER_CONNECT_CFM\n"));

				}
				else
				{
                    KOOVOX_TASK_DEBUG(("failure: %d\n", sscc->status));
                    Panic();
				}

			}
            break;

        case SPP_CONNECT_IND:
            {
                SPP_CONNECT_IND_T *sci = (SPP_CONNECT_IND_T *)message; 
                KOOVOX_TASK_DEBUG(("SPP_CONNECT_IND\n"));                
              
                /* Respond to accept the connection.*/
                SppConnectResponse(
                        task, 
                        (const bdaddr *)(&(sci->addr)), 
                        TRUE,                      /* Accept the connection */
                        sci->sink,
                        sci->server_channel,
                        0                       /* Use default payload size */
                        );

            }
            break;                

        case SPP_MESSAGE_MORE_DATA:
			{
				SPP_MESSAGE_MORE_DATA_T* smmd = (SPP_MESSAGE_MORE_DATA_T *) message;
				uint16 size = SourceSize(smmd->source);
				const uint8* msg = SourceMap(smmd->source);
				uint16 ret = 0;
				
				KOOVOX_TASK_DEBUG(("SPP_MESSAGE_MORE_DATA:size= %x\n", size));

#ifdef DEBUG_PRINT_ENABLED
				{
					uint8 i = 0;
					DEBUG(("msg:"));
					for(; i < size; i++)
					{
						DEBUG(("%x ", msg[i]));
					}
					DEBUG(("\n"));
				}
#endif	

				ret = KoovoxMessageStr(msg, "{}", size);

				if(ret)
				{
					memcpy(koovox.sppValue + koovox.sizeSppValue, msg, ret);
					koovox.sizeSppValue += ret;
					DisposeSppMessage(koovox.sppValue, koovox.sizeSppValue - 2);
					memset(koovox.sppValue, 0, ret);
					koovox.sizeSppValue = 0;
					if((size > MAX_DATA_BUF)&&(size - ret) < MAX_DATA_BUF)
					{
						memcpy(koovox.sppValue + koovox.sizeSppValue, msg + ret, size - ret);
						koovox.sizeSppValue += size - ret;

						ret = KoovoxMessageStr(msg + ret, FRAME_TAIL, size - ret);
						if(ret)
						{
							/* dispose the spp message */
							DisposeSppMessage(koovox.sppValue, koovox.sizeSppValue - 2);
							memset(koovox.sppValue, 0, MAX_DATA_BUF);
							koovox.sizeSppValue = 0;
						}
					}
					
				}

															
				SourceDrop(smmd->source, SourceSize(smmd->source));
			}
            break;
			
        case SPP_MESSAGE_MORE_SPACE:
			{
				KOOVOX_TASK_DEBUG(("SPP_MESSAGE_MORE_SPACE\n"));
			}
			break;

        case SPP_DISCONNECT_IND:
            {
				SPP_DISCONNECT_IND_T *sdi = (SPP_DISCONNECT_IND_T *) message;
				SppDisconnectResponse( sdi->spp);
#if defined(BLE_ENABLED)
				start_ble_advertising();
#endif

				freePanic(koovox.sppValue);
				koovox.sppValue = NULL;
				koovox.sizeSppValue = 0;

                KOOVOX_TASK_DEBUG(("SPP_DISCONNECT_IND - status\n"));
            }
            break;            
			
		default:
			KOOVOX_TASK_DEBUG(("undefined messageid!\n"));
			break;

    }
}



/*************************************************************************
NAME
    KoovoxTaskMessageHandler
    
DESCRIPTION
    Handle messages between phone and koovox 
*/
static void KoovoxTaskMessageHandler( Task task, MessageId id, Message message )
{
	KOOVOX_TASK_DEBUG(("KOOVOX_TASK: id:%x\n", id));
	
    if ( (id >= SPP_MESSAGE_BASE) && (id <= SPP_MESSAGE_TOP) )
    {
        handleSPPMessage(task, id, message);
    }
	else if(id == MESSAGE_STREAM_PARTITION_VERIFY)
	{
		KOOVOX_TASK_DEBUG(("MESSAGE_STREAM_PARTITION_VERIFY\n"));
	}
	else
	{
		KOOVOX_TASK_DEBUG(("undefined messageid:%x\n", id));
	}
	
}


/*************************************************************************
NAME
    InitKoovoxTask
    
DESCRIPTION
    init koovox task  
*/
void InitKoovoxTask(void)
{
	/* Ensure phone app is never initialised more than once */
	if(koovox.initialised == FALSE)
	{
		KOOVOX_TASK_DEBUG(("InitKoovoxTask\n"));

		koovox.initialised = TRUE;

		/* init message handler */
		koovox.task.handler = KoovoxTaskMessageHandler;

		/* start spp server*/
		SppStartService(&(koovox.task));

#if 0
		/* mount external flash to filesystem */
		PartitionMountFilesystem(PARTITION_SERIAL_FLASH , 0x01, PARTITION_LOWER_PRIORITY);
#endif

		/* ³õÊ¼»¯´®¿Ú */
		uart_data_stream_init();
	}
}



#else  /* ENABLE_KOOVOX */

#include <csrtypes.h>
static const uint16 dummy_gatt = 0;    

#endif /* ENABLE_KOOVOX */



