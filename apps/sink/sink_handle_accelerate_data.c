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
	else
		return;
		
	step_var->status = 0;
	step_var->index_max = 0;
	step_var->index_min = 0;
	step_var->max_flag = 1;
	step_var->min_flag = 1;
	step_var->min_value = 100;
	step_var->max_value = 100;
	step_var->pre_value = 100;

	/* 从pskey中读取步数值 */
	{
	uint16 value[2] = {0};
	uint32 step = 0;
	ConfigRetrieve(CONFIG_SPORT_DATA, value, 2);
	step = (uint32)value[0];
	step += ((uint32)value[1] << 16)&0xffff0000;
	if(koovox_step == 0)
		koovox_step = step;
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

	if(!step_var)
		return;
	
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
		koovox_step = 0;
		
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

	DEBUG(("StepCount: cmd %d result: %d\n", cmd, result));

	if(cmd == START)
	{
		if(result != ERR)
		{
			/* 初始化计步功能参数 */
			Koovox_init_step_var();
		}			
	}
	else if(cmd == STOP)
	{
		if(result == SUC)
		{
			/* 初始化计步功能参数 */
			Koovox_free_step_var();
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

	if((size_value < FRAME_STEP_COUNT)
		||(value == NULL)
		||(step_var == NULL))
		return;

#ifdef DEBUG_PRINT_ENABLEDX
	{
		uint8 i = 0;
		for(; i<size_value; i++)
			DEBUG(("%3d ", value[i]));
		DEBUG(("\n"));
	}
#endif

	{
		uint16 curr_value = 0;
		uint32 curr_index = 0;
		
		uint8 status = step_var->status;
		uint16 pre_value = step_var->pre_value;
		
		curr_value = (uint16)value[0];
		curr_value += ((uint16)value[1] << 8);
		
		curr_index = (uint32)value[2] ;
		curr_index += ((uint32)value[3] << 8);
		curr_index += ((uint32)value[4] << 8);
		curr_index += ((uint32)value[5] << 8);

#ifdef ENABLE_LOG
	{
		char* log_msg = (char*)mallocPanic(25);
		sprintf(log_msg, "%3d index:%ld\n", curr_value, curr_index);
		APP_CORE_LOG((log_msg));
		freePanic(log_msg);
		log_msg = NULL;
	}
#endif
		
		if(curr_value > pre_value)
		{
			/*检测极小值*/
			if(status)
			{
				uint16 min_value = step_var->min_value;
				step_var->status = 0;/*设置为上升沿*/

				if(step_var->max_flag)
				{
					uint32 index_min = step_var->index_min;
					if((pre_value < MIN_VALUE_THRESHOLD)&&(curr_index - index_min >= TIME_THRESHOLD))
					{
						step_var->max_flag = 0;
						step_var->min_flag = 1;
						step_var->min_value = pre_value;
						step_var->index_min = curr_index;
					}
				}
				else
				{
					/* 更新极小值 */
					if(pre_value < min_value)
					{
						step_var->min_value = pre_value;
						step_var->index_min = curr_index;
					}
				}
		
			}
		}
		else
		{
			/*检测极大值*/
			if(!status)
			{
				uint16 max_value = step_var->max_value;
				step_var->status = 1; /*设置为下降沿*/

				if(step_var->min_flag)
				{
					uint32 index_max = step_var->index_max;

					if((pre_value > MAX_VALUE_THRESHOLD)&&(curr_index - index_max >= TIME_THRESHOLD))
					{
						koovox_step++;
						step_var->max_flag = 1;
						step_var->min_flag = 0;
						step_var->index_max = curr_index;
						
						/**********久坐计时清零**********/
						KoovoxFillAndSendUartPacket(ENV, OBJ_STEP_COUNT, 0, 0);
						
#if 1
						if(isOnlineState())
						{
							uint8 sport_data[5] = {0};
							uint8 hb_value = 0;
							uint32 step = koovox_step;
							sport_data[0] = hb_value;
							sport_data[1] = (step >> 24) & 0xff;
							sport_data[2] = (step >> 16) & 0xff;
							sport_data[3] = (step >> 8) & 0xff;
							sport_data[4] = step  & 0xff;
						
							/* send the heart rate value to mobile */
							SendNotifyToDevice(OID_SPORT_VALUE, 0, sport_data, sizeof(sport_data));
						}
#endif
					}
					
				}
				else
				{
					/* 更新极大值 */
					if(pre_value > max_value)
					{
						step_var->max_value = pre_value;
						step_var->index_max = curr_index;
					}
					
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
#define ANGLE_INIT_COUNT			(5)

static int16 angle_x_int = 0;
static int16 angle_y_int = 0;
static int16 angle_z_int = 0;


const int16 sin_table[] = {
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
int8 Angle_search(const int16* table, uint16 size, int16 key)
{
	uint16 low = 0;
	uint16 high = size - 1;
	uint16 mid = (low + high) / 2;

	if(key < 0)
		key = -key;
	

	if(key > table[size - 1])
		return (int8)90;

	while(low < high)
	{
		if(key > table[mid])
		{
			low = mid + 1;
			mid = (low + high) / 2;
		}
		else if(key < table[mid])
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

#ifdef DEBUG_PRINT_ENABLED
		{
			uint8 i = 0;
			for(; i<size_value; i++)
				DEBUG(("%3d ", value[i]));
			DEBUG(("\n"));
		}
#endif

	if((size_value == 1)&&(*value == NECK_PROTECT_ALARM_EVENT))
	{
		/********** 语音提醒用户颈椎保护 **********/
		AudioPromptPlayEvent(EventKoovoxPromptNectProtectAdvice);

#ifdef ENABLE_LOG
		{
			char* log_msg = (char*)mallocPanic(15);
			sprintf(log_msg , "protect neck\n");
			APP_CORE_LOG((log_msg));
			freePanic(log_msg);
			log_msg = NULL;
		}
#endif

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
		angle_y_curr = Angle_search(sin_table, 90, axis_y);
		angle_z_curr = 90 - Angle_search(sin_table, 90, axis_z);


#ifdef ENABLE_LOG
	 {
		 char* log_msg = (char*)mallocPanic(25);
		 sprintf(log_msg, "Acc:%4d %4d %4d\n", axis_x, axis_y, axis_z);
		 APP_CORE_LOG((log_msg));
		 memset(log_msg, 0, 25);
		 sprintf(log_msg , "Angle:%3d %3d %3d\n", angle_x_curr, angle_y_curr, angle_z_curr);
		 APP_CORE_LOG((log_msg));
		 freePanic(log_msg);
		 log_msg = NULL;
	 }
#endif

		if(koovox.angle_init_cnt == ANGLE_INIT_COUNT)
		{
			angle_x_int /= ANGLE_INIT_COUNT;
			angle_y_int /= ANGLE_INIT_COUNT;
			angle_z_int /= ANGLE_INIT_COUNT;

			koovox.angle_init_cnt++;
			
#ifdef ENABLE_LOG
		 {
			 char* log_msg = (char*)mallocPanic(25);
			 sprintf(log_msg , "init:%3d %3d %3d\n", angle_x_int, angle_y_int, angle_y_int);
			 APP_CORE_LOG((log_msg));
			 freePanic(log_msg);
			 log_msg = NULL;
		 }
#endif
			
		}
		else if(koovox.angle_init_cnt < ANGLE_INIT_COUNT)
		{
			koovox.angle_init_cnt++;
			angle_x_int += angle_x_curr;
			angle_y_int += angle_y_curr;
			angle_z_int += angle_z_curr;
			return;
		}

		if(axis_x < 0)
		{
			angle_x_curr = - angle_x_curr;
		}

		if(axis_y < 0)
		{
			angle_y_curr = - angle_y_curr;
		}
		
		if(axis_z < 0)
		{
			angle_z_curr = - angle_z_curr;
		}

		if((angle_x_curr - (int8)angle_x_int > ANGLE_VALUE_THRESHOLD )
			||(((int8)angle_x_int - angle_x_curr) > ANGLE_VALUE_THRESHOLD))
		{
			/********* 启动定时器 ********/
			uint8 value = TRUE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
			return ;
		}

		if((angle_y_curr - (int8)angle_y_int > ANGLE_VALUE_THRESHOLD )
			||(((int8)angle_y_int - angle_y_curr) > ANGLE_VALUE_THRESHOLD))
		{
			/********* 启动定时器 ********/
			uint8 value = TRUE;
			KoovoxFillAndSendUartPacket(ENV, OBJ_NECK_PROTECT, &value, 1);
			return ;
		}

		if((angle_z_curr - (int8)angle_z_int > ANGLE_VALUE_THRESHOLD )
			||(((int8)angle_z_int - angle_z_curr) > ANGLE_VALUE_THRESHOLD))
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
	uint8 cmd = data[0];
	uint16 result = (uint16)data[1];

	if(cmd == START)
	{
		if(result == SUC)
		{
			koovox.neckEnable = TRUE;
			koovox.angle_init_cnt = 0;
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
#define GET_UP_MIN_THRESHOLD	37
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

#ifdef ENABLE_LOG
		{
			char* log_msg = (char*)mallocPanic(15);
			sprintf(log_msg , "const seat\n");
			APP_CORE_LOG((log_msg));
			freePanic(log_msg);
			log_msg = NULL;
		}
#endif
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

#ifdef ENABLE_LOG
				{
					char* log_msg = (char*)mallocPanic(15);
					sprintf(log_msg , "sit up\n");
					APP_CORE_LOG((log_msg));
					freePanic(log_msg);
					log_msg = NULL;
				}
#endif
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

#define FRAME_HEAD_ACTION				(8)
#define HEAD_LEN_MAX_MIN				(3)
#define HEAD_VALUE_THRESHOLD			(5)
#define HEAD_ACTION_CHECK				(3)


Extremum_param* nod_x_var = NULL;
Extremum_param* nod_y_var = NULL;

uint32 head_action_cnt = 0;


/****************************************************************************
NAME 
  	Koovox_init_nod_head_var

DESCRIPTION
 	init the head action business variable
 
RETURNS
  	void
*/ 
void Koovox_init_nod_head_var(void)
{
	if(!nod_x_var)
		nod_x_var = (Extremum_param*)mallocPanic(sizeof(Extremum_param));

	if(!nod_y_var)
		nod_y_var = (Extremum_param*)mallocPanic(sizeof(Extremum_param));

	nod_x_var->status = 0;
	nod_x_var->min_flag = 0;
	nod_x_var->max_flag = 1;
	nod_x_var->min_value = 0;
	nod_x_var->max_value = 0;
	nod_x_var->pre_value = 0;
	nod_x_var->min_index = 0;
	nod_x_var->max_index = 0;

	nod_y_var->status = 0;
	nod_y_var->min_flag = 1;
	nod_y_var->max_flag = 0;
	nod_y_var->min_value = 0;
	nod_y_var->max_value = 0;
	nod_y_var->pre_value = 10;
	nod_y_var->min_index = 0;
	nod_y_var->max_index = 0;

	head_action_cnt = 0;
}

/****************************************************************************
NAME 
  	Koovox_free_nod_head_var

DESCRIPTION
 	free the head action business variable
 
RETURNS
  	void
*/ 
void Koovox_free_nod_head_var(void)
{
	if((!nod_x_var)&&(!nod_y_var))
		return;

	freePanic(nod_x_var);
	nod_x_var = NULL;

	freePanic(nod_y_var);
	nod_y_var = NULL;
}

/****************************************************************************
NAME 
  	Koovox_find_max_min_status

DESCRIPTION
 	free the head action business variable
 
RETURNS
  	void
*/ 
void Koovox_find_max_min_value(uint32 index, int16 curr_value, Extremum_param* param)
{
	uint8 status = param->status;
	uint8 min_flag = param->min_flag;
	uint8 max_flag = param->max_flag;
	int16 pre_value = param->pre_value;
	
	if(curr_value > pre_value)
	{
		/* 检测极小值 */
		if(status)
		{
			param->status = 0;/* 0:表示上升沿； 1:表示下降沿 */
			
			if(max_flag)
			{
				uint32 max_index = param->max_index;
				int16 max_value = param->max_value;
				/* 存取极小值*/
				if((index - max_index > HEAD_LEN_MAX_MIN)&&(max_value - pre_value >= HEAD_VALUE_THRESHOLD))
				{
					param->min_flag = 1;
					param->max_flag = 0;
					param->min_index = index;
					param->min_value = pre_value;
				}
			}
			else
			{
				int16 min_value = param->min_value;
				
				/* 更新极小值*/
				if(pre_value <= min_value)
				{
					param->min_value = pre_value;
					param->min_index = index;
				}
			}
			
		}
		
	}
	else if(curr_value < pre_value)
	{
		/* 检测极大值 */
		if(!status)
		{
			param->status = 1;/* 0:表示上升沿； 1:表示下降沿 */ 

			if(min_flag)
			{
				uint32 min_index = param->min_index;
				int16 min_value = param->min_value;
				/* 存取极大值*/
				if((index - min_index > HEAD_LEN_MAX_MIN)&&(pre_value - min_value >= HEAD_VALUE_THRESHOLD))
				{
					param->min_flag = 0;
					param->max_flag = 1;
					param->max_index = index;
					param->max_value = pre_value;					
				}			
			}
			else
			{
				int16 max_value = param->max_value;
				
				/* 更新极大值*/
				if(pre_value + 1>= max_value)
				{
					param->max_value = pre_value;
					param->max_index = index;
				}
			}
			
		}
		
	}
	else
	{
		int16 min_value = param->min_value;
		int16 max_value = param->max_value;
		
		if(curr_value >= max_value)
		{
			/* 更新极大值*/
			param->max_value = pre_value;
			param->max_index = index;			
		}
		else if(curr_value <= min_value)
		{
			/* 更新极小值*/
			param->min_value = pre_value;
			param->min_index = index;			
		}
	}
	
}



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

#ifdef DEBUG_PRINT_ENABLED
{
	uint8 i = 0;
	uint16 value = 0;
	for(; i<size_data; )
	{
		value = (uint16)data[i++];
		value += (uint16)data[i++] << 8;
		DEBUG(("%d ", (int16)value));
	}
	DEBUG(("\n"));
}
#endif

	if((size_data < FRAME_HEAD_ACTION)
		||(data == NULL)
		||(nod_x_var == NULL)
		||(nod_y_var == NULL))
		return;

	{
		int16 axis_x, axis_y;
		uint16 temp = 0;
		uint32 index = 0;

		temp = (uint16)data[0];
		temp += (uint16)data[1] << 8;
		axis_x = (int16)temp;
		axis_x /= 10;

		temp = 0;
		temp = (uint16)data[2];
		temp += (uint16)data[3] << 8;
		axis_y = (int16)temp;
		axis_y /= 10;
			

		index = head_action_cnt++;

#ifdef ENABLE_LOG
		{
			char* log_msg = (char*)mallocPanic(20);
			sprintf(log_msg , "%3d %3d %ld\n", axis_x, axis_y, index);
			APP_CORE_LOG((log_msg));
			freePanic(log_msg);
			log_msg = NULL;
		}
#endif

		/* find the axis_x min value */
		Koovox_find_max_min_value(index, axis_x, nod_x_var);

		/* find the axis_y max value */
		Koovox_find_max_min_value(index, axis_y, nod_y_var);

		nod_x_var->pre_value = axis_x;
		nod_y_var->pre_value = axis_y;

#ifdef ENABLE_LOGx
		{
			char* log_msg = (char*)mallocPanic(25);
			sprintf(log_msg , "index:%ld %ld\n", nod_x_var->min_index, nod_y_var->max_index);
			APP_CORE_LOG((log_msg));
			freePanic(log_msg);
			log_msg = NULL;
		}
#endif

		/* check the nod head action */
		if((nod_x_var->min_flag)&&(nod_y_var->max_flag))
		{
			uint32 x_index = nod_x_var->min_index;
			uint32 y_index = nod_y_var->max_index;

			if((x_index < y_index + HEAD_ACTION_CHECK)
				||(y_index < x_index + HEAD_ACTION_CHECK))
			{
#ifdef ENABLE_LOGx			
				/* 检测到点头动作 */
				DEBUG(("**check the nod action**\n"));
				APP_CORE_LOG(("check the nod action\n"));
#endif

				/* 接听电话 */
				/*MessageSend(theSink.task, EventUsrAnswer, 0);*/
			}
			else
			{
				nod_x_var->min_flag = 0;
				nod_y_var->max_flag = 0;
			}
		}

		
	}

}

/****************************************************************************
NAME 
  	KoovoxResponseNodHead

DESCRIPTION
 	response the cmd of the head action
 
RETURNS
  	void
*/ 
void KoovoxResponseNodHead(uint8* data, uint8 size_data)
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
				/* init head action variable */
				Koovox_init_nod_head_var();
			}
		}
		else if(cmd == STOP)
		{
			if(result == SUC)
			{
				/* free head action variable */
				Koovox_free_nod_head_var();
			}
		}
	}
}



