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


void KoovoxResponseHealthMonitor(uint8* data, uint8 size_data);

/****************** step count  ******************/

void KoovoxResponseStepCount(uint8* data, uint8 size_data);
void KoovoxCountStep(uint8* value, uint8 size_value);


/****************** neck protect  ******************/

void KoovoxProtectNeck(uint8* value, uint8 size_value);
void KoovoxResponseNeckProtect(uint8* data, uint8 size_data);



/****************** const seat  ******************/

void KoovoxConstSeat(uint8* data, uint8 size_data);
void KoovoxResponseConstSeat(uint8* data, uint8 size_data);


/****************** head action  ******************/

void KoovoxNodHead(uint8* data, uint8 size_data);
void KoovoxShakeHead(uint8* data, uint8 size_data);

#endif

