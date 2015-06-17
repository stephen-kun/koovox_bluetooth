/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_handle_accelerate_data.c

*/


#include "sink_handle_accelerate_data.h"
#include "sink_config.h"
#include "sink_app_core.h"
#include "sink_app_task.h"
#include "sink_app_message.h"
#include "sink_koovox_uart.h"
#include "sink_audio_prompts.h"





/**********************************************************
*
******************* step count ***************************
*
***********************************************************/


#define FRAME_STEP_COUNT	6

Acc_step_var* step_var = NULL;
uint32 koovox_step = 0;


/**
* @brief  init step count variable
* @param  none
* @retval none
*/
void Koovox_init_step_var(void)
{
	if(!step_var)
		step_var = (Acc_step_var *)mallocPanic(sizeof(Acc_step_var));
		
	step_var->status = 0;
	step_var->index_max = 0;
	step_var->max_flag = 1;
	step_var->min_flag = 1;
	step_var->min_value = 100;
	step_var->pre_value = 100;

	/* 从pskey中读取步数值 */
	{
	uint16 value[2] = {0};
	ConfigRetrieve(CONFIG_SPORT_DATA, value, 2);
	koovox_step = (uint32)value[0] + (((uint32)value[1] << 16)&0xffff0000);
	}
}

/**
* @brief  koovox_free_step_var
* @param  none
* @retval none
*/
void Koovox_free_step_var(void)
{
	uint8 step[4] = {0};
	
	free(step_var);
	step_var = NULL;

	step[0] = koovox_step & 0xff;
	step[1] = (koovox_step >> 8) & 0xff;
	step[2] = (koovox_step >> 16) & 0xff;
	step[3] = (koovox_step >> 24) & 0xff;

	/* 存取步数值到pskey */
	{
		uint16 value[2] = {0};
		value[0] = koovox_step & 0xffff;
		value[1] = (koovox_step >> 16) & 0xffff;
		
		ConfigStore(CONFIG_SPORT_DATA, value, 2);
	}
}

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
	uint8 cmd = data[0];
	uint16 result = (uint16)data[1];

	if(cmd == START)
	{
		if(result == SUC)
		{
			/* 初始化计步功能参数 */
			Koovox_init_step_var();
		}
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
		return;

	{
		uint16 curr_value = 0;
		uint32 curr_index = 0;
		
		uint8 status = step_var->status;
		uint32 index_max = step_var->index_max;
		uint16 min_value = step_var->min_value;
		uint16 pre_value = step_var->pre_value;
		
		curr_value = (uint16)value[0] + ((uint16)value[1] << 8);
		curr_index = (uint32)value[2] + ((uint32)value[3] << 8) + ((uint32)value[3] << 16) + ((uint32)value[3] << 24);
		
		
		if(curr_value > pre_value)
		{
			/*检测极小值*/
			if(status)
			{
				step_var->status = 0;/*设置为上升沿*/
		
				if((pre_value < MIN_VALUE_THRESHOLD)&&(step_var->max_flag))
				{
					step_var->max_flag = 0;
					step_var->min_flag = 1;
					step_var->min_value = pre_value;
				}
			}
		}
		else
		{
			/*检测极大值*/
			if(!status)
			{
				step_var->status = 1; /*设置为下降沿*/
				
				if((pre_value > MAX_VALUE_THRESHOLD)
					&&((curr_index - index_max) >= TIME_THRESHOLD)
					&&(step_var->min_flag)
					&&((curr_value - min_value) > VALUE_THRESHOLD))
				{
					koovox_step++;
					step_var->max_flag = 1;
					step_var->min_flag = 0;
					step_var->index_max = curr_index;
		
					/**********久坐计时清零**********/
					KoovoxFillAndSendUartPacket(ENV, OBJ_STEP_COUNT, 0, 0);
				}
			}
		}
		
		step_var->pre_value = curr_value;
	}
	
}


/**********************************************************
*
***************** neck protect ****************************
*
***********************************************************/
#define NECK_PROTECT_ALARM_EVENT	((uint8)0x01)


const uint16 sin_table[] = {
17,  34,  52,  69,  87, 104, 121, 139, 156, 173,
190, 207, 224, 241, 258, 275, 292, 309, 325, 342,
358, 374, 390, 406, 422, 438, 453, 469, 484, 500,
515, 529, 544, 559, 573, 587, 601, 615, 629, 642,
656, 669, 681, 694, 707, 719, 731, 743, 754, 766,
777, 788, 798, 809, 819, 829, 838, 848, 857, 866,
874, 882, 891, 898, 906, 913, 920, 927, 933, 939,
945, 951, 956, 961, 965, 970, 974, 978, 981, 984,
987, 990, 992, 994, 996, 997, 998, 999, 999, 1000	
};


/**
  * @Angle_search
  * @param  
  * 		table: the address point of the table
  *		size: the length of the table
  *		key: search the key
  * @retval the angle value
  */
int8 Angle_search(const uint16* table, uint16 size, int16 key)
{
	uint16 low = 0;
	uint16 high = size - 1;
	uint16 mid = (low + high) / 2;
	uint16 key_value = 0;

	if(key)
		key_value = (uint16)key;
	else
		key_value = (uint16)(-key);
	

	if(key_value > table[size - 1])
		return (int8)90;

	while((low < high))
	{
		if(key_value > table[mid])
		{
			low = mid + 1;
			mid = (low + high) / 2;
		}
		else if(key_value < table[mid])
		{
			high = mid - 1;
			mid = (low + high) / 2;
		}
		else
			return (int8)mid;
	}

	return (int8)low;
}



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
		return;

	if((size_value == 1)&&(*value == NECK_PROTECT_ALARM_EVENT))
	{
		/********** 语音提醒用户颈椎保护 **********/
		AudioPromptPlayEvent(EventKoovoxPromptNectProtectAdvice);
	}
	else
	{
		int8 angle_x_curr = 0;
		int8 angle_y_curr = 0;
		int8 angle_z_curr = 0;
		int16 axis_x, axis_y, axis_z;

		axis_x = (int16)((uint16)value[0] + ((uint16)value[1] << 8));
		axis_y = (int16)((uint16)value[2] + ((uint16)value[3] << 8));
		axis_z = (int16)((uint16)value[4] + ((uint16)value[5] << 8));

		angle_x_curr = Angle_search(sin_table, 90, axis_x);
		angle_y_curr = Angle_search(sin_table, 90, (uint16)axis_y);
		angle_z_curr = 90 - Angle_search(sin_table, 90, (uint16)axis_z);

		if(axis_x < 0)
		{
			angle_x_curr = - angle_x_curr;
		}

		if(axis_y > 0)
		{
			angle_y_curr = - angle_y_curr;
		}
		
		if(axis_z > 0)
		{
			angle_z_curr = - angle_z_curr;
		}


		if((angle_x_curr > (ANGLE_VALUE_THRESHOLD + ANGLE_X_INIT))
			||((angle_x_curr + ANGLE_VALUE_THRESHOLD) < ANGLE_X_INIT))
		{
			/********* 启动定时器 ********/
			uint8 value = TRUE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
			return ;
		}

		if((angle_y_curr > (ANGLE_Y_INIT + ANGLE_VALUE_THRESHOLD))
			|| ((angle_y_curr + ANGLE_VALUE_THRESHOLD) < ANGLE_Y_INIT))
		{
			/********* 启动定时器 ********/
			uint8 value = TRUE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
			return ;
		}

		if((angle_z_curr > (ANGLE_Z_INIT + ANGLE_VALUE_THRESHOLD))
			|| ((angle_z_curr + ANGLE_VALUE_THRESHOLD) < ANGLE_Z_INIT))
		{
			/********* 启动定时器 ********/
			uint8 value = TRUE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
			return ;
		}

		/***** 关闭定时器 ****/
		{
			uint8 value = FALSE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
		}

		
	}

	
}

/****************************************************************************
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
	uint8 cmd = data[0];
	uint16 result = (uint16)data[1];

	if(cmd == START)
	{
		if(result == SUC)
		{
			koovox.neckEnable = TRUE;
		}
	}
	else if(cmd == STOP)
	{
		if(result == SUC)
		{
			koovox.neckEnable = FALSE;
		}
	}

	/* response the cmd result */
	SendResponse(result, CMD_SET, OID_NECK, 0, 0, 0);
	
	}
}


/**********************************************************
*
***************** const seat ****************************
*
***********************************************************/

#define CONST_SEAT_ALARM_EVENT	((uint8)0x01)
#define GET_UP_MIN_THRESHOLD	25
#define GET_UP_MAX_THRESHOLD	144
#define GET_UP_VALUE_THRESHOLD	120
#define INDEX_THRESHOLD			10


Const_seat_var* seat_var = NULL;


/**
* @brief  Koovox_init_const_seat_var
* @param  none
* @retval none
*/
void Koovox_init_const_seat_var(void)
{
	if(!seat_var)
		seat_var = (Const_seat_var*)malloc(sizeof(Const_seat_var));
		
	seat_var->max_flag = 0;
	seat_var->max_value = 0;
	seat_var->p_value = 100;
	seat_var->status = 0;
	seat_var->max_index = 0;
}


/**
* @brief  Koovox_free_const_seat_var
* @param  none
* @retval none
*/
void Koovox_free_const_seat_var(void)
{
	free(seat_var);
	seat_var = NULL;
}



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
		return;

	if((data[0]==CONST_SEAT_ALARM_EVENT)&&(size_data == 1))
	{
		/********** 语音提醒用户已久坐 **********/
		AudioPromptPlayEvent(EventKoovoxPromptNectProtectAdvice);
	}
	else
	{
		/* 完成起身动作检测 */
		uint16 curr_value = (uint16)data[0] + (((uint16)data[1] << 8)&0xff00);
		uint16 pre_value = seat_var->p_value;
		uint32 curr_index = (uint32)data[2] + ((uint32)data[3] << 8) + ((uint32)data[3] << 16) + ((uint32)data[3] << 24);
		uint8 status = seat_var->status;

		seat_var->p_value = curr_value;

		if(curr_value > pre_value)
		{
			/* 极小值检测 */
			if(status)
			{
				/* 设置为上升沿 */
				seat_var->status = 0;

				if((pre_value < GET_UP_MIN_THRESHOLD)&&
					(seat_var->max_flag)&&
					(seat_var->max_value - pre_value > GET_UP_VALUE_THRESHOLD)&&
					(curr_index - seat_var->max_index < INDEX_THRESHOLD))
				{
					/* 检测到起身动作 */
					KoovoxFillAndSendUartPacket(ENV, OBJ_CONST_SEAT, 0, 0);

					Koovox_init_const_seat_var();
				}
			}
		}
		else
		{
			/* 极大值检测 */
			if(!status)
			{
				/* 设置为下降沿 */
				seat_var->status = 1;

				if(pre_value > GET_UP_MAX_THRESHOLD)
				{
					seat_var->max_flag = 1;
					seat_var->max_index = curr_index;
					seat_var->max_value = pre_value;
				}
			}
		}
		
		
	}

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
	uint8 cmd = data[0];
	uint16 result = (uint16)data[1];

	if(cmd == START)
	{
		if(result == SUC)
		{
			koovox.seatEnable = TRUE;
			Koovox_init_const_seat_var();
		}
	}
	else if(cmd == STOP)
	{
		if(result == SUC)
		{
			koovox.seatEnable = FALSE;
			Koovox_free_const_seat_var();
		}

	}

	/* response the cmd result */
	SendResponse(result, CMD_SET, OID_NECK, 0, 0, 0);
	
	}
}


/**********************************************************
*
***************** head action ****************************
*
***********************************************************/


/****************************************************************************
NAME 
  	KoovoxHeadAction

DESCRIPTION
 	head action check
 
RETURNS
  	void
*/ 
void KoovoxHeadAction(uint8* data, uint8 size_data)
{
#ifdef ENABLE_LOG
	{
		char* log_msg = (char*)mallocPanic((size_data+1)*LOG_WIDE);
		uint8 i = 0;
		for(; i<size_data; i++)
			sprintf(log_msg + LOG_WIDE*i,"%5d ", data[i]);
		sprintf(log_msg + LOG_WIDE*i, "\n");
		APP_CORE_LOG((log_msg));
		freePanic(log_msg);
		log_msg = NULL;
	}
#endif
}

/****************************************************************************
NAME 
  	KoovoxResponseHeadAction

DESCRIPTION
 	response the cmd of the head action
 
RETURNS
  	void
*/ 
void KoovoxResponseHeadAction(uint8* data, uint8 size_data)
{

}



