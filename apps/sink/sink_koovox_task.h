/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_koovox_task.h

*/

#ifdef ENABLE_KOOVOX


#ifndef __SINK_KOOVOX_TASK_H
#define __SINK_KOOVOX_TASK_H

#include <message.h>
#include <csrtypes.h>
#include <partition.h>

#include "sink_private.h"

#ifdef DEBUG_KOOVOX_TASK
#define KOOVOX_TASK_DEBUG(x)	DEBUG(x)
#else
#define KOOVOX_TASK_DEBUG(x)
#endif

/* present scence */
typedef enum{
	IDLE_SCENCE = 0x00,
	MEETING_SCENCE,
	DRIVING_SCENCE,
	LEAVING_SCENCE,
	NO_DISTURBING_SCENCE
}PresentScence;

typedef enum{
	RECORD_IDLE,
	RECORD_BUSY
}enumRecordStatus;



/* koovox task data */
typedef struct{
    TaskData        task;           /* The phone app  message handler */
    bool            initialised;    /* Flag set when phone app has been initialised for the device */
	uint8 			muteStatus;		/* the mute micphone status */
	bool			presentEnable;	/* the present business enable or disable */
	PresentScence 	presentScence;	/* the presence scence */
	uint8			recodeStatus;	/* the record status */
	uint16			last_cmd;		/* the last of the cmd to STM8 */
	uint8			repeat_times;	/* repeat send the last cmd times */

	uint8			neckEnable; 	/* enable or disable neck protect business */
	uint8			seatEnable; 	/* enable or disable const seat business */
	uint8			hbEnable;		/* enable or disable heart rate business */
	uint8			health;

	bool			ble_adv;		/* flag set when ble adverting enable or disable */
	
}gKoovoxTaskData;



/*************************************************************************
NAME
    InitKoovoxTask
    
DESCRIPTION
    init phone app task  
*/
void InitKoovoxTask(void);

extern gKoovoxTaskData koovox;

#endif /* __SINK_KOOVOX_TASK_H */

#endif

