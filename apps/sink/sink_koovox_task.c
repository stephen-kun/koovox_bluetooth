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
#include "sink_koovox_uart.h"


/* Single instance of phone app task */
gKoovoxTaskData koovox ;


/*************************************************************************
NAME
    KoovoxTaskMessageHandler
    
DESCRIPTION
    Handle messages between phone and koovox 
*/
static void KoovoxTaskMessageHandler( Task task, MessageId id, Message message )
{
	KOOVOX_TASK_DEBUG(("KOOVOX_TASK: id:%x\n", id));
	
	if(id == MESSAGE_STREAM_PARTITION_VERIFY)
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



