/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_heart_rate_calc.c

*/


#include "sink_heart_rate_calc.h"
#include "sink_koovox_core.h"
#include "sink_koovox_task.h"
#include "sink_koovox_uart.h"
#include "koovox_wechat_handle.h"



Hb_smooth_var* smooth_var = NULL;
uint16* hb_calc_buff = NULL;

static void Fifo_insert(uint8* fifo, uint8 size, uint8 value);
static uint8 Get_average(uint8* data, uint8 size);
static uint8 Koovox_heartrate_smooth(uint8 hb_value);



/**
  * @brief  insert a value to fifo
  * @param  
  * 		fifo: the address point of the fifo
  *		size: the length of the fifo
  *		value: insert value
  * @retval none
  */
static void Fifo_insert(uint8* fifo, uint8 size, uint8 value)
{
	uint8 i = size - 1;
	uint8 temp =0;
	for(; i>0; i--)
	{
		temp = fifo[i - 1];
		fifo[i] = temp;
	}
	fifo[0] = value;
}

/**
  * @brief  get a average value
  * @param  
  * 		data: the address point of the data
  *		size: the size of the data
  * @retval a average value
  */
static uint8 Get_average(uint8* data, uint8 size)
{
	uint8 i = 0;
	uint16 sum = 0;
	uint8 cnt = 0;
	uint8 temp = 0;

	for(; i<size; i++)
	{
		temp = data[i];
		if(temp)
		{
			sum += temp;
			cnt++;
		}
	}

	if(sum)
		return (uint8)(sum / cnt);
	else 
		return 0;
}


/**
  * @brief init the heart rate smooth variable
  * @param  none
  * @retval none
  */
void Koovox_init_smooth_var(void)
{
	if(!smooth_var)
		smooth_var = (Hb_smooth_var*)mallocPanic(sizeof(Hb_smooth_var));
	
	smooth_var->p_smooth_value = 0;
	smooth_var->p_hb_value = 0;
	smooth_var->zero_num = 0;
	smooth_var->smooth_status = 0;
	memset(smooth_var->hb_origin_buff, 0, HB_SMOOTH_ORIGIN_SIZE);
	memset(smooth_var->hb_smooth_buff, 0, HB_SMOOTH_BUFF_SIZE);
}

/**
  * @brief Koovox_free_smooth_var
  * @param  none
  * @retval none
  */
void Koovox_free_smooth_var(void)
{
	free(smooth_var);
	smooth_var = NULL;
}


/**
  * @brief  smooth the heart rate value
  * @param  
  * 		hb_value: the heart rate value 
  * @retval heart rate value
  */
static uint8 Koovox_heartrate_smooth(uint8 hb_value)
{
	uint8 i = 0;

	/* 原始数据缓存*/
	Fifo_insert(smooth_var->hb_origin_buff, HB_SMOOTH_ORIGIN_SIZE, hb_value);

	smooth_var->p_hb_value = smooth_var->hb_origin_buff[1];

	switch(smooth_var->smooth_status)
	{
	case HB_SMOOTH_BEFORE:
	{
		uint16 sum = 0;
		uint8 count =0;
		uint8 temp = 0;
		uint8 aveg = 0;

		for(i=0; i<HB_SMOOTH_COUNT; i++)
		{
			temp = smooth_var->hb_origin_buff[i];
			sum += temp;
			if(temp > hb_value)
				temp = temp - hb_value;
			else
				temp = hb_value - temp;

			if(temp <= HB_SMOOTH_THRESHOLD)
				count++;
		}

		aveg = sum / HB_SMOOTH_COUNT;
		if(aveg < HB_MIN_VALUE)
		{
			smooth_var->p_smooth_value = 0;
			return 0;
		}

		if(count < HB_SMOOTH_COUNT)
		{
			smooth_var->p_smooth_value = hb_value;
			return hb_value;
		}

		smooth_var->smooth_status = HB_SMOOTH_MIDDLE;
		memset(smooth_var->hb_smooth_buff, 0, HB_SMOOTH_BUFF_SIZE);

	}
	break;
		
	case HB_SMOOTH_MIDDLE:
	{

		if(!hb_value)
		{
			if(smooth_var->p_hb_value)
				smooth_var->zero_num = 0;
			
			smooth_var->zero_num++;

			if(smooth_var->zero_num >= HB_YZERO_THRESHOLD)
				smooth_var->smooth_status = HB_SMOOTH_AFTER;
		}

		{
			uint8 threshold_1 = smooth_var->p_smooth_value + HB_SMOOTH_THRESHOLD;
			uint8 threshold_2 = smooth_var->p_smooth_value + HB_SMOOTH_FILTER;
			uint8 threshold_3 = smooth_var->p_smooth_value - HB_SMOOTH_THRESHOLD;
			uint8 threshold_4 = smooth_var->p_smooth_value - HB_SMOOTH_FILTER;
			
			if(hb_value > threshold_2)
			{
				hb_value = threshold_1;
			}
			else if(hb_value < threshold_4)
			{
				hb_value = threshold_3;
			}
			else if((hb_value > threshold_1)||(hb_value < threshold_3))
			{
				hb_value = smooth_var->p_smooth_value;
			}
		
			Fifo_insert(smooth_var->hb_smooth_buff, HB_SMOOTH_BUFF_SIZE, hb_value);
		
			hb_value = Get_average(smooth_var->hb_smooth_buff, HB_SMOOTH_BUFF_SIZE);
			
		}
			
	}
	break;
		
	case HB_SMOOTH_AFTER:
	{
		memset(smooth_var, 0, sizeof(Hb_smooth_var));
		hb_value = 0;
	}
	break;
		
	default:
		break;
	}

	smooth_var->p_smooth_value = hb_value;
	return hb_value;
	
}



/****************************************************************************
NAME 
  	KoovoxCalculateHeartRate

DESCRIPTION
 	calculate the heart rate value
 
RETURNS
  	void
*/ 
void KoovoxCalculateHeartRate(uint8* value, uint8 size_value)
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
		uint8 heart_rate = 0;
		
		heart_rate = Koovox_heartrate_smooth(value[0]);
		DEBUG(("average:%d\n", heart_rate));
		
		/* 更新心率值到前端 */
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


/****************************************************************************
NAME 
  	KoovoxResponseHeartRate

DESCRIPTION
 	response the heart rate cmd
 
RETURNS
  	void
*/ 
void KoovoxResponseHeartRate(uint8* value, uint8 size_value)
{
	if((value == NULL)||(size_value < SIZE_RESPONSE))
		return;

	{
	uint8 cmd = value[0];
	uint16 result = (uint16)value[1];
	
	DEBUG(("cmd:%d ,result:%d\n", cmd, result));

	if(cmd == START)
	{
		if(result == SUC)
		{
			/* init the space */
			Koovox_init_smooth_var();
		}
	}
	else if(cmd == STOP)
	{ 
		if(result == SUC)
		{
			/* release the space */
			Koovox_free_smooth_var();
		}
	}

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

