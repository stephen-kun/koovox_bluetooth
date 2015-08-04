/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_handle_accelerate_data.c

*/

#include "sink_handle_accelerate_data.h"
#include "sink_config.h"
#include "sink_koovox_core.h"
#include "sink_koovox_task.h"
#include "sink_koovox_message.h"
#include "sink_koovox_uart.h"
#include "sink_audio_prompts.h"

#include "sink_states.h"
#include "sink_statemanager.h"


/****************************************************************************
NAME 
  	KoovoxResponseHealthMonitor

DESCRIPTION
 	response the cmd of health monitor
 
RETURNS
  	void
*/ 
void KoovoxResponseHealthMonitor(uint8* data, uint8 size_data)
{
	if((data==NULL)||(size_data < SIZE_RESPONSE))
		return;

	{
		
	if(wechat_req)
	{
		/* 应答前端 */
		
		wechat_req = FALSE;
	}
	else if(button_req)
		button_req = FALSE;
	
	}
}


/**********************************************************
*
******************* step count ***************************
*
***********************************************************/


#define FRAME_STEP_COUNT	4


/****************************************************************************
NAME 
  	KoovoxResponseStepCount

DESCRIPTION
 	response the cmd of step count
 
RETURNS
  	void
*/ 
void KoovoxResponseStepCount(uint8* data, uint8 size_data)
{
	if((data==NULL)||(size_data < SIZE_RESPONSE))
		return;

	{
		
	if(wechat_req)
	{
		/* 应答前端 */
		
		wechat_req = FALSE;
	}
	
	}
}



/****************************************************************************
NAME 
  	KoovoxCountStep

DESCRIPTION
 	count the step value
 
RETURNS
  	void
*/ 
void KoovoxCountStep(uint8* value, uint8 size_value)
{
	if((size_value < FRAME_STEP_COUNT)||(value == NULL))
	{
		/* 通知前端通信异常 */
		return;
	}

	/* 将步数值同步到前端 */
	
}


/**********************************************************
*
***************** neck protect ****************************
*
***********************************************************/


/****************************************************************************
NAME 
  	KoovoxProtectNeck

DESCRIPTION
 	neck protect
 
RETURNS
  	void
*/ 
void KoovoxProtectNeck(uint8* value, uint8 size_value)
{
	if(value == NULL)
	{
		/* 通知前端通信异常 */
		
		return;
	}

	/********** 语音提醒用户颈椎保护 **********/

	/* 通知前端记录到一次颈椎保护提示 */

			
}

/***************************************************************************
NAME 
  	KoovoxProtectNeck

DESCRIPTION
 	neck protect
 
RETURNS
  	void
*/ 
void KoovoxResponseNeckProtect(uint8* data, uint8 size_data)
{
	if((data==NULL)||(size_data < SIZE_RESPONSE))
		return;

	{

	if(wechat_req)
	{
		/* 应答前端 */
		
		wechat_req = FALSE;
	}
	
	}
}


/**********************************************************
*
***************** const seat ****************************
*
***********************************************************/


/****************************************************************************
NAME 
  	KoovoxConstSeat

DESCRIPTION
 	const seat prompt
 
RETURNS
  	void
*/ 
void KoovoxConstSeat(uint8* data, uint8 size_data)
{
	if(data == NULL)
	{
		/* 通知前端通信异常 */
		
		return;
	}

	/********** 语音提醒用户已久坐 **********/

	/* 通知前端记录到一次久坐提醒 */

}


/****************************************************************************
NAME 
  	KoovoxConstSeat

DESCRIPTION
 	const seat prompt
 
RETURNS
  	void
*/ 
void KoovoxResponseConstSeat(uint8* data, uint8 size_data)
{

	if((data==NULL)||(size_data < SIZE_RESPONSE))
		return;

	{

	if(wechat_req)
	{
		/* 应答前端 */
		
		wechat_req = FALSE;
	}
	
	}
}


/**********************************************************
*
***************** head action ****************************
*
***********************************************************/

#define FRAME_HEAD_ACTION				(8)


/****************************************************************************
NAME 
  	KoovoxNodHead

DESCRIPTION
 	head action check
 
RETURNS
  	void
*/ 
void KoovoxNodHead(uint8* data, uint8 size_data)
{

	if((size_data < FRAME_HEAD_ACTION)||(data == NULL))
		return;

	/* 接听电话 */
	if(deviceIncomingCallEstablish == stateManagerGetState())
		MessageSend(&theSink.task, EventUsrAnswer, 0);

}

/****************************************************************************
NAME 
  	KoovoxShakeHead

DESCRIPTION
 	head action check
 
RETURNS
  	void
*/ 
void KoovoxShakeHead(uint8* data, uint8 size_data)
{

	if((size_data < FRAME_HEAD_ACTION)||(data == NULL))
		return;

	/* 挂断电话 */
	if(deviceActiveCallSCO == stateManagerGetState())
		MessageSend(&theSink.task, EventUsrCancelEnd, 0);

}




