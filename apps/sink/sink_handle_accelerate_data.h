/****************************************************************************
Copyright (C) JoySoft . 2015-2025
Part of KOOVOX 1.0.1

FILE NAME
    sink_handle_accelerate_data.h

*/

#ifndef __SINK_HANDLE_ACC_DATA_H
#define __SINK_HANDLE_ACC_DATA_H

#include <message.h>
#include "sink_private.h"

/****************** step count  ******************/

#define MAX_VALUE_THRESHOLD		143
#define MIN_VALUE_THRESHOLD		82
#define TIME_THRESHOLD			6
#define VALUE_THRESHOLD			60

typedef struct{
	uint8 status ;/* 0:表示上升沿； 1:表示下降沿 */
	uint8 max_flag ;
	uint8 min_flag;
	uint16 min_value;
	uint16 pre_value;
	uint32 index_max;	/*前一个极大值下标*/
}Acc_step_var;

void Koovox_init_step_var(void);
void Koovox_free_step_var(void);
void KoovoxResponseStepCount(uint8* data, uint8 size_data);
void KoovoxCountStep(uint8* value, uint8 size_value);


/****************** neck protect  ******************/

#define ANGLE_X_INIT			45
#define ANGLE_Y_INIT			45
#define ANGLE_Z_INIT			0
#define ANGLE_VALUE_THRESHOLD	40

int8 Angle_search(const uint16* table, uint16 size, int16 key);
void KoovoxProtectNeck(uint8* value, uint8 size_value);
void KoovoxResponseNeckProtect(uint8* data, uint8 size_data);



/****************** const seat  ******************/

typedef struct{
	uint8 status;/* 0:表示上升沿； 1:表示下降沿	*/
	uint8 max_flag;
	uint8 max_value;
	uint16 p_value;/* 前一个加速度矢量和 */
	uint32 max_index;
}Const_seat_var;


void Koovox_init_const_seat_var(void);
void Koovox_free_const_seat_var(void);
void KoovoxConstSeat(uint8* data, uint8 size_data);
void KoovoxResponseConstSeat(uint8* data, uint8 size_data);


/****************** head action  ******************/
void KoovoxHeadAction(uint8* data, uint8 size_data);
void KoovoxResponseHeadAction(uint8* data, uint8 size_data);



#endif

