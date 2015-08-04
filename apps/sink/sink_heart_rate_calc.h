/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_heart_rate_calc.h

*/

#ifndef __SINK_HEART_RATE_CALC_H
#define __SINK_HEART_RATE_CALC_H

#include <message.h>
#include "sink_private.h"

#define  HB_VALIDE_POINT	 5
#define	 HB_MAX_BUFF_SIZE	 16
#define  HB_MIN_VALUE    	 50
#define  HB_MAX_VALUE    	 150

#define HB_SMOOTH_THRESHOLD		4
#define HB_SMOOTH_BUFF_SIZE		10
#define HB_YZERO_THRESHOLD		3
#define HB_SMOOTH_BEFORE 		0
#define HB_SMOOTH_MIDDLE 		1
#define HB_SMOOTH_AFTER			2
#define HB_SMOOTH_COUNT			3
#define HB_SMOOTH_FILTER 		12
#define HB_SMOOTH_ORIGIN_SIZE	6


typedef struct{
	uint8 p_smooth_value;	/*前一个心率平滑值*/
	uint8 p_hb_value;		/*前一个心率原始值*/
	uint8 zero_num;			/*缓冲区中连续0值个数*/
	uint8 smooth_status;	/*平滑算法的阶段*/
	uint8 hb_origin_buff[HB_SMOOTH_ORIGIN_SIZE];
	uint8 hb_smooth_buff[HB_SMOOTH_BUFF_SIZE];
}Hb_smooth_var;

void Koovox_init_smooth_var(void);
void Koovox_free_smooth_var(void);

void KoovoxCalculateHeartRate(uint8* value, uint8 size_value);
void KoovoxResponseHeartRate(uint8* value, uint8 size_value);


#endif

