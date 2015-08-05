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
	PRESET_IDLE,
	PRESET_BUSY
}enumPresenceStatus;

typedef enum{
	RECORD_IDLE,
	RECORD_BUSY
}enumRecordStatus;



/* koovox task data */
typedef struct{
    TaskData        task;           /* The phone app  message handler */
    bool            initialised;    /* Flag set when phone app has been initialised for the device */
	uint8 			muteStatus;		/* the mute micphone status */
	uint8			presentEnable;	/* the present business enable or disable */
	uint8 			presentStatus;	/* the presence status */
	uint8			vmEnable;		/* enable or disable the voice message */
	uint8			recodeStatus;	/* the record status */
	uint8			angle_init_cnt;	/* */
	uint16			last_cmd;		/* the last of the cmd to STM8 */
	uint8			repeat_times;	/* repeat send the last cmd times */

	Sink			sppSink;		/* Stream sink of the spp transport. */ 
	uint8*			sppValue;		/* value from the mobile */
	uint16			sizeSppValue;	/* size of the value */

	uint8			neckEnable; 	/* enable or disable neck protect business */
	uint8			seatEnable; 	/* enable or disable const seat business */
	uint8			hbEnable;		/* enable or disable heart rate business */
	uint8			health;
	
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
