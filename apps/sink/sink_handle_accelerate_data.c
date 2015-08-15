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
#include "sink_koovox_uart.h"
#include "sink_audio_prompts.h"

#include "sink_states.h"
#include "sink_statemanager.h"
#include "koovox_wechat_handle.h"


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
		RspWechat_t response = {0};
		response.state = S_SUC;
		
		koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
		koovox_send_data_to_wechat();
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
		RspWechat_t response = {0};
		response.state = S_SUC;
		
		koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
		koovox_send_data_to_wechat();
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
	uint16 length = sizeof(RspWechat_t) + (size_value ? (size_value - 1) : 0);
	RspWechat_t* response = (RspWechat_t*)mallocPanic(length);

	if((size_value < FRAME_STEP_COUNT)||(value == NULL))
	{
		/* 通知前端通信异常 */
		response->state = S_ERROR;
	}
	else
	{
		/* 将步数值同步到前端 */
		response->state = S_SUC;
		response->cmd = ENV;
		response->obj = OBJ_STEP_COUNT;
		response->size_value= size_value;
		if(value)
			memmove(response->value, value, size_value);
		
	}

	koovox_pack_wechat_send_data_req((uint8 *)response, length, TRUE, EDDT_manufatureSvr);
	koovox_send_data_to_wechat();

	freePanic(response);
	response = NULL;
	
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
	uint16 length = sizeof(RspWechat_t) + (size_value ? (size_value - 1) : 0);
	RspWechat_t* response = (RspWechat_t*)mallocPanic(length);

	if(value == NULL)
	{
		/* 通知前端通信异常 */
		response->state = S_ERROR;
		
	}
	else
	{
		/********** 语音提醒用户颈椎保护 **********/
		
		/* 通知前端记录到一次颈椎保护提示 */
		response->state = S_SUC;
		response->cmd = ENV;
		response->obj = OBJ_NECK_PROTECT;
		response->size_value= size_value;
		if(value)
			memmove(response->value, value, size_value);

	}
	
	koovox_pack_wechat_send_data_req((uint8 *)response, length, TRUE, EDDT_manufatureSvr);
	koovox_send_data_to_wechat();
	
	freePanic(response);
	response = NULL;
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
		RspWechat_t response = {0};
		response.state = S_SUC;
		
		koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
		koovox_send_data_to_wechat();
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
void KoovoxConstSeat(uint8* value, uint8 size_value)
{

	uint16 length = sizeof(RspWechat_t) + (size_value ? (size_value - 1) : 0);
	RspWechat_t* response = (RspWechat_t*)mallocPanic(length);

	if(value == NULL)
	{
		/* 通知前端通信异常 */
		response->state = S_ERROR;
		
	}
	else
	{
		/********** 语音提醒用户已久坐 **********/
		
		/* 通知前端记录到一次久坐提醒 */
		response->state = S_SUC;
		response->cmd = ENV;
		response->obj = OBJ_CONST_SEAT;
		response->size_value= size_value;
		if(value)
			memmove(response->value, value, size_value);

	}
	
	koovox_pack_wechat_send_data_req((uint8 *)response, length, TRUE, EDDT_manufatureSvr);
	koovox_send_data_to_wechat();
	
	freePanic(response);
	response = NULL;
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
		RspWechat_t response = {0};
		response.state = S_SUC;
		
		koovox_pack_wechat_send_data_req((uint8 *)&response, sizeof(response), TRUE, EDDT_manufatureSvr);
		koovox_send_data_to_wechat();
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




