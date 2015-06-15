/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_heart_rate_calc.c

*/


#include "sink_heart_rate_calc.h"
#include "sink_app_core.h"
#include "sink_app_task.h"
#include "sink_app_message.h"
#include "sink_koovox_uart.h"



Hb_smooth_var* smooth_var = NULL;
uint16* hb_calc_buff = NULL;

static void Fifo_insert(uint8* fifo, uint8 size, uint8 value);
static uint8 Get_average(uint8* data, uint8 size);
static uint8 Koovox_heartrate_calc(uint16* data, uint16 len);
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
  * @brief  calc heart rate value
  * @param  
  * 		data: the peload of the data
  *		len: the length of the data
  * @retval heart rate value
  */
static uint8 Koovox_heartrate_calc(uint16* data, uint16 len)
{
	
	uint8 hb_max_buff[HB_MAX_BUFF_SIZE] = {0};
	uint8 hb_point_buff[HB_MAX_BUFF_SIZE] = {0};
	uint8 temp = 0;
	uint8 count_index = 0;
	
	uint8 i, j;

	/************ 寻找极大值并存取极大值间点个数 *****************/
	{
		uint8 status = 0; 	/* 0:表示上升沿； 1:表示下降沿*/	
		uint8 max_flag = 1;
		uint8 min_flag = 1;
		int8 max_index = -(HB_MIN_POINT);
		int8 min_index = -(HB_MIN_POINT);
		uint16 max_value = 0;
		uint16 min_value = 4096;
		
		uint16 curr_data = 0;
		uint16 prev_data = *data;
		int8 curr_index = 0;

		for(i=1; i<len; i++)
		{
			curr_data = *(data + i);
			curr_index = i;

			if(curr_data > prev_data)
			{
				/* 极小值检测*/
				if(status)
				{
					status = 0;	/*设置为上升沿*/

					if(max_flag)
					{
						/* 存取极小值*/
						if((curr_index > (HB_MAX_MIN_LEN + max_index))&&(curr_index >(HB_MIN_POINT + min_index)))
						{
							max_flag = 0;
							min_flag = 1;
							min_value = prev_data;
							min_index = curr_index;
						}
					}
					else
					{
						/* 更新极小值*/
						if((prev_data <= min_value)&&(curr_index <= (HB_MIN_POINT + min_index)))
						{
							min_index = curr_index;
							min_value = prev_data;
						}
					}
						
				}
			}
			else 
			{
				/* 极大值检测*/
				if(!status)
				{
					status = 1; /*设置为下降沿*/

					if(min_flag)
					{
						/* 存取极大值*/
						if((curr_index > (HB_MIN_MAX_LEN + min_index))&&(curr_index > (HB_MIN_POINT + max_index)))
						{
							min_flag = 0;
							max_flag = 1;
							hb_max_buff[count_index++] = curr_index - max_index;
							max_index = curr_index;
							max_value = prev_data;
						}			
					}
					else
					{
						temp = curr_index - max_index;
						/* 更新极大值*/
						if((max_value <= prev_data)&&(temp <= HB_MIN_POINT))
						{
							count_index--;
							temp += hb_max_buff[count_index];
							hb_max_buff[count_index++] = temp;
							max_index = curr_index;
							max_value = prev_data;
						}
					}
				}
			}

			prev_data = curr_data;
		}
	}

#ifdef DEBUG_PRINT_ENABLED
{
	uint8 i = 0;

	for(;i<HB_MAX_BUFF_SIZE;i++)
	{
		DEBUG(("%d ", hb_max_buff[i]));
	}
	DEBUG(("\n"));
}
#endif

	/* 计算心率周期个数以及总时长 */
	if((count_index - HB_VALIDE_POINT) >= 0)
	{
		/****** 密集区查找 *****************/
		{
			uint8 curr_out = 0;/*外循环当前值*/
			uint8 curr_in = 0; /*内循环当前值*/
			uint8 add_cnt = 0;
			
			for(i = 0; i<count_index; i++)
			{
				add_cnt = 0;
				curr_out = hb_max_buff[i];
				for(j=0; j<count_index; j++)
				{
					curr_in = hb_max_buff[j];
					if(curr_out > curr_in)
						temp = curr_out - curr_in;
					else
						temp = curr_in - curr_out;
			
					if(temp <= HB_POINT_THRESHOLD)
						add_cnt++;
				}
			
				hb_point_buff[i] = add_cnt;
			}
		}

#ifdef DEBUG_PRINT_ENABLED
		{
			uint8 i = 0;
		
			for(;i<HB_MAX_BUFF_SIZE;i++)
			{
				DEBUG(("%d ", hb_point_buff[i]));
			}
			DEBUG(("\n"));
		}
#endif
		
		/************* 密集点有效性判断 ****************/
		{
			uint8 density_num = 0;		/*密集值个数*/
			uint8 period_num = 0;		/*统计有效周期个数*/
			uint16 time_sum = 0;		/*统计有效周期总时间*/
			uint8 period_min = 0;
			uint8 curr_point =0;		/*当前点个数*/

			if(count_index % 2)
			{
				period_min = (count_index / 2) + 1;
			}
			else
			{
				period_min = count_index / 2;
			}

			for(i=0; i<count_index; i++)
			{
				curr_point = hb_max_buff[i];
				density_num = hb_point_buff[i];

				if(density_num >= period_min)
				{
					temp = HR_SAMPLE_POINT_NUM;
					temp /= curr_point;		/* 密集点对应的周期数*/	

					if((temp < 10 )||(count_index >= temp))
					{
						period_num++;
						time_sum += curr_point;
					}
				}
			}

			/*DEBUG(("total time:%d - period:%d - count_index:%d\n", time_sum, period_num, count_index));*/

			/* 计算平均心率值 */	
			if(period_num >= period_min)
			{
				uint16 sum = 60000;
				uint16 div = time_sum*HR_SAMPLE_TIMES;
				uint8 hb_value = 0;

				div /= period_num;

				hb_value = sum / div;
				if((hb_value > HB_MAX_VALUE)||(hb_value < HB_MIN_VALUE))
					return 0;
				else
					return hb_value;
			}
			else
			{
				return 0;
			}
			
		}
		
	}
	else
	{
		return 0;		
	}	
}

/**
  * @brief init the heart rate calculate variable
  * @param  none
  * @retval none
  */
void Koovox_init_hb_calc_var(void)
{
	if(!hb_calc_buff)
		hb_calc_buff = (uint16*)mallocPanic(sizeof(uint16)*HR_SAMPLE_POINT_NUM);

}

/**
  * @brief Koovox_free_hb_calc_var
  * @param  none
  * @retval none
  */
void Koovox_free_hb_calc_var(void)
{
	free(hb_calc_buff);
	hb_calc_buff = NULL;
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
	
	uint16 i = 0, j = 0;
	uint8 heart_rate = 0;

#if 1
	if(!GetSampleStatus())
		return;
#endif

	for(; i<(HR_SAMPLE_POINT_NUM - HR_SAMPLE_FREQUENCE); i++)
	{
		hb_calc_buff[i] = hb_calc_buff[i + HR_SAMPLE_FREQUENCE];
	}

	for(; i<HR_SAMPLE_POINT_NUM; i++)
	{
		hb_calc_buff[i] = ((uint16)value[j++] << 8) & 0x0f00;
		hb_calc_buff[i] += value[j++];
#if 0
		DEBUG(("%d ", hb_calc_buff[i]));
	}
	DEBUG(("\n"));
#else
	}
#endif

#if 1
	heart_rate = Koovox_heartrate_calc(hb_calc_buff, HR_SAMPLE_POINT_NUM);
	DEBUG(("origin:%d ", heart_rate));
	heart_rate = Koovox_heartrate_smooth(heart_rate);
	DEBUG(("average:%d\n", heart_rate));
#else
	heart_rate = Koovox_heartrate_calc(hb_calc_buff, HR_SAMPLE_POINT_NUM);
	heart_rate = Koovox_heartrate_smooth(heart_rate);
#endif

	/* update the heart rate value */
	UpdateHeartRateValue(heart_rate);
	if(isOnlineState())
	{
		/* send the heart rate value to mobile */
		SendNotifyToDevice(OID_HB , ON, &koovox.heartRateValue, 1);
	}

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
	uint8 cmd = 0;
	uint16 result = 0;
	
	cmd = value[0];
	result = (uint16)value[1];

	if(cmd == START)
	{
		if(result == SUC)
		{
			/* init the space */
			Koovox_init_smooth_var();
			Koovox_init_hb_calc_var();
		}
	}
	else if(cmd == STOP)
	{ 
		if(result == SUC)
		{
			/* release the space */
			Koovox_free_hb_calc_var();
			Koovox_free_smooth_var();
		}
	}

	/* response the cmd result */
	SendResponse(result, CMD_SET, OID_HB, 0, 0, 0);
	}

}

