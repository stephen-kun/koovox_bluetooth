// *******************************************************************************************
// Copyright (C) Cambridge Silicon Radio plc 2003-2012 http://www.csr.com
// *******************************************************************************************
//
// DESCRIPTION
// An example app for routing audio through the Kalimba DSP from ADC to DAC
//
// NOTES
//
// What the code does:
//    Sets up cbuffers (circular connection buffers) for reading audio from
//    the ADC and routing to the DAC. Cbuffers are serviced by timer
//    interrupts.
// *******************************************************************************************
#define HB_ORIGIN_VALUEx
#define METHOD_TESTx
#define DEBUG_TESTx

//the message between VM and DSP.
#define VM_HEARTRATE_SAMPLE_START_ID		0x4f00	//from VM to DSP
#define VM_HEARTRATE_SAMPLE_STOP_ID			0x4f01	//from VM to DSP
#define VM_ACCELERATE_SAMPLE_START_ID		0x4f02	//from VM to DSP
#define VM_ACCELERATE_SAMPLE_STOP_ID		0x4f03	//from VM to DSP
#define VM_ENABLE_STEP_VALUE_ID             0x4f05  //from VM to DSP
#define VM_ENABLE_TIME_VALUE_ID             0x4f06  //from VM to DSP    
#define VM_ENABLE_NECK_EVENT_ID             0x4f07  //from VM to DSP             
#define VM_ENABLE_DRIVER_EVENT_ID           0x4f08  //from VM to DSP
#define VM_RECOVER_HB_VALUE_ID				0x4f09	//from VM to DSP
#define VM_ENABLE_SEAT_EVENT_ID				0x4f0A  //from VM to DSP

#define HEARTRATE_VALUE_TO_VM_ID			0x4f20  //from DSP to VM
#define SPORT_DATA_VALUE_TO_VM_ID			0x4f21  //from DSP to VM
#define DRIVER_PROMPT_EVENT_TO_VM_ID		0x4f22  //from DSP to VM
#define NECT_PROMPT_EVENT_TO_VM_ID			0x4f23  //from DSP to VM
#define HB_I2C_TEST_TO_VM_ID				0x4f24  //from DSP to VM
#define ACC_I2C_TEST_TO_VM_ID				0x4f25  //from DSP to VM
#define SEAT_PROMPT_EVENT_TO_VM_ID			0x4f26  //from DSP to VM
#define NOD_ACTION_EVENT_TO_VM_ID			0x4f27	//from DSP to VM
#define SHAKE_ACTION_EVENT_TO_VM_ID			0x4f28	//from DSP to VM

// debug ids
#define $DEBUG_HB_ORIGIN_VALUE_ID               0x4087	//from DSP to VM
#define $DEBUG_ACC_ORIGIN_VALUE_ID		        0x4088	//from DSP to VM


// **********   the i2c interface  **************
#define $I2C_MAK		1
#define $I2C_NMAK		0


// heart rate sensor
#define HZ_40x
#ifndef HZ_40
#define $HR_SAMPLE_CBUF_SIZE 121 
#define $HR_SAMPLE_POINT_NUM 120
#define $HR_SAMPLE_TIMES     50
#define $HR_SAMPLE_PERIOD    $HR_SAMPLE_TIMES*1000 //50ms, so we get 20 samples per second.
#define $HR_SAMPLE_FREQUENCE	20

#else
#define $HR_SAMPLE_CBUF_SIZE 241 
#define $HR_SAMPLE_POINT_NUM 240
#define $HR_SAMPLE_TIMES     25
#define $HR_SAMPLE_PERIOD    $HR_SAMPLE_TIMES*1000 //25ms, so we get 40 samples per second.
#define $HR_SAMPLE_FREQUENCE	40

#endif
#define $HR_SPORT_TIME_COUNT	1000*1000		// 1000 ms

#define BIT_MIN         0x01
#define BIT_MAX         0x02
#define MASK_MAX        0xfffffd
#define MASK_MIN        0xfffffe
#define BIT_STORE       0x03

#define HB_MIN_VALUE    50
#define HB_MAX_VALUE    150


#define $I2C_HB_WD      	0x90	// write address 
#define $I2C_HB_RD      	0x91	// read address
#define $I2C_HB_CFG_REG         0x01	// config register address
#define $I2C_HB_CVS_REG     	0x0	// conversion register address
#define $I2C_HB_CFG_MSB         0x85	// set msb value of the config register
#define $I2C_HB_CFG_LSB         0x83	// set lsb value of the config register

// Standard includes
#include "core_library.h"
#include "cbops_library.h"
#include "frame_sync_tsksched.h"

#define HB_TEMP_BUFF    18

// *****************************************************************************
// MODULE:
//    vee.heartrate
//
// DESCRIPTION:
//    sample the heartrate adc value and calculate the heartrate value
//
// *****************************************************************************
.MODULE $vee.heartrate;
	.CODESEGMENT PM;
	.DATASEGMENT DM;   

	.VAR/DMCIRC sample_cbuffer[$HR_SAMPLE_CBUF_SIZE];
	.VAR sample_cbuffer_struc[]=
		LENGTH(sample_cbuffer),
		&sample_cbuffer,
		&sample_cbuffer;

	.VAR/DMCIRC calc_cbuffer[$HR_SAMPLE_CBUF_SIZE];
	.VAR calc_cbuffer_struc[]=
		LENGTH(calc_cbuffer),
		&calc_cbuffer,
		&calc_cbuffer;

	.VAR  hb_enable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  hb_disable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  time_value_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  recover_hb_value_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  sample_timer_struc[$timer.STRUC_SIZE];
	.VAR  sport_time_timer_struc[$timer.STRUC_SIZE];

	.VAR calc_buff[$HR_SAMPLE_POINT_NUM];
	   
	.VAR hb_sample_enable = 0;
	.VAR hb_time_enable = 0;
   
 	.VAR heartrate = 0;
	.VAR time_value = 0;
     
	.VAR value_lsb;
	.VAR value_msb;
	
#ifdef METHOD_TEST
	.VAR hb_temp_buff[HB_TEMP_BUFF];
#endif

#ifdef DEBUG_TEST
	.VAR hb_debug_buff[HB_TEMP_BUFF];
#endif

	.VAR hb_addr_min[HB_TEMP_BUFF];
	.VAR hb_addr_max[HB_TEMP_BUFF];

#ifdef   HB_ORIGIN_VALUE
	.VAR send_buff[$HR_SAMPLE_FREQUENCE];
#endif


//// *******************************************************************************************
// MODULE:
//	vee.heartrate.start
//
// DESCRIPTION:
//	 start heartrate module.
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  none 
//
// TRASHED REGISTERS:
//	 
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.start:
	// Push rLink onto stack
	$push_rLink_macro;	
    
	call $vee.heartrate.i2c_test;
		
	// register message handler for VM_HEARTRATE_SAMPLE_START_ID message
	r1 = &hb_enable_from_vm_message_struc;
	r2 = VM_HEARTRATE_SAMPLE_START_ID;
	r3 = &$hb_enable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_HEARTRATE_SAMPLE_STOP_ID message
	r1 = &hb_disable_from_vm_message_struc;
	r2 = VM_HEARTRATE_SAMPLE_STOP_ID;
	r3 = &$hb_disable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_ENABLE_TIME_VALUE_ID message
	r1 = &time_value_from_vm_message_struc;
	r2 = VM_ENABLE_TIME_VALUE_ID;
	r3 = &$time_value_cmd_from_vm;
	call $message.register_handler;
	
	// register message handler for VM_RECOVER_HB_VALUE_ID message
	r1 = &recover_hb_value_from_vm_message_struc;
	r2 = VM_RECOVER_HB_VALUE_ID;
	r3 = &$vee.heartrate.hb_recover;
	call $message.register_handler;	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//	vee.heartrate.i2c_test
//
// DESCRIPTION:
//	 test the heartrate sensor can communication with i2c bus.
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  none
//
// TRASHED REGISTERS:
//	  
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.i2c_test:
	// Push rLink onto stack
	$push_rLink_macro;	
    
	// send i2c start bit
	call $i2c.start_bit;	

	// write to config register
	r0 =  $I2C_HB_WD;    // send first byte 0b1001 0000
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump i2c_test_err;
	
	r0 = $I2C_HB_CFG_REG;
	call $i2c.send_byte;    // send second byte 0b0000 0001
	NULL = r0;
	if NZ jump i2c_test_err;

	r0 = $I2C_HB_CFG_MSB;       // send third byte 0b1000 0101
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump i2c_test_err;

	r0 = $I2C_HB_CFG_LSB;
	call $i2c.send_byte;    // send fourth byte 0b1000 0011 
	NULL = r0;
	if NZ jump i2c_test_err;

	// send i2c stop bit
	call $i2c.stop_bit;

	// send i2c start bit
	call $i2c.start_bit;

	// write to pointer register
	r0 =  $I2C_HB_WD;     // send first byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump i2c_test_err;

	r0 = $I2C_HB_CFG_REG;   // send second byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump i2c_test_err;

	// send i2c stop bit
	call $i2c.stop_bit;

	// send i2c start bit
	call $i2c.start_bit;

	// read configer register
	r0 =  $I2C_HB_RD;     // send first byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump i2c_test_err;

	r0 = 1;
	call $i2c.receive_byte;  
	
	r0 = 1;
	call $i2c.receive_byte;
	NULL = r0 - 0x83;
	if NZ jump i2c_test_err;

	// send i2c stop bit
	call $i2c.stop_bit;	

	// send test success result to vm
	r2 = HB_I2C_TEST_TO_VM_ID;
	r3 = 0;		// r3 = 0,success.
	call $message.send_short;	

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

i2c_test_err:

	// send test failed result to vm
	r2 = HB_I2C_TEST_TO_VM_ID;
	r3 = 1;		// r3 = 1, failed.
	call $message.send_short;	

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    hb_enable_cmd_from_vm
//
// DESCRIPTION:
//   enable the heartrate sample
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$hb_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	r1 = M[hb_sample_enable];
	NULL = r1 - 1;
	if POS jump $pop_rLink_and_rts;

	r1 = 1;
	M[hb_sample_enable] = r1;

	// start timer that copies audio samples
	r1 = &$vee.heartrate.sample_timer_struc;
	r2 = $HR_SAMPLE_PERIOD;
	r3 = &$vee.heartrate.sample_timer_handler;
	call $timer.schedule_event_in;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

	
//// *******************************************************************************************
// MODULE:
//    hb_disable_cmd_from_vm
//
// DESCRIPTION:
//   disable the heartrate sample
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$hb_disable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

    call $interrupt.block;
	r1 = 0;
	M[hb_sample_enable] = r1;
	M[hb_time_enable] = r1;
    call $interrupt.unblock;
    
	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    time_value_cmd_from_vm
//
// DESCRIPTION:
//   
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$time_value_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;	

	r1 = M[hb_time_enable];
	NULL = r1 - 1;
	if POS jump $pop_rLink_and_rts;

	r1 = 1;
	M[hb_time_enable] = r1;

	// start timer that copies audio samples
	r1 = &$vee.heartrate.sport_time_timer_struc;
	r2 = $HR_SPORT_TIME_COUNT;
	r3 = &$vee.heartrate.sport_time_timer_handler;
	call $timer.schedule_event_in;
	 
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
	
//// *******************************************************************************************
// MODULE:
//    vee.heartrate.sport_time_timer_handler
//
// DESCRIPTION:
//   count the sport times.
//
// INPUTS:
//   none
//
// OUTPUTS:

// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.sport_time_timer_handler:
	
	// Push rLink onto stack
	$push_rLink_macro;
	
	r0 = M[hb_time_enable];
	NULL = r0;
	if Z jump $pop_rLink_and_rts;	

    call $interrupt.block;
	r1 = M[time_value];
	r1 = r1 + 1;
	M[time_value] = r1;
    call $interrupt.unblock;
	
	// start timer that copies audio samples
	r1 = &$vee.heartrate.sport_time_timer_struc;
	r2 = $HR_SPORT_TIME_COUNT;
	r3 = &$vee.heartrate.sport_time_timer_handler;
	call $timer.schedule_event_in;
 
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	

//// *******************************************************************************************
// MODULE:
//    vee.heartrate.sample_timer_handler
//
// DESCRIPTION:
//   handler the heartrate sample from VM.
//
// INPUTS:
//   none
//
// OUTPUTS:
//    r7 store the sample_cbuffer_struc len
//    r8 store the adc value
//
// TRASHED REGISTERS:
//    r8
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.sample_timer_handler:
	
	// Push rLink onto stack
	$push_rLink_macro;

	r0 = M[hb_sample_enable];
	NULL = r0;
	if Z jump $pop_rLink_and_rts;

	// read heart rate adc value
	call $vee.heartratevalue.read;
	NULL = r3;
	if NEG jump vee_heartrate_sample_ret;

	// store the adc value to sample_cbuffer_struc
	call $vee.heartrate.input;

	//check if the data amount is enough to calculate
	r0 = &$vee.heartrate.sample_cbuffer_struc;
	call $cbuffer.calc_amount_data;

	//return when data amount is less than calcurate reqirement.
	NULL = r0 - $HR_SAMPLE_POINT_NUM;
	if NEG jump vee_heartrate_sample_ret;

	// copy the sample_cbuffer to calc_cbuffer
	call $vee.heartrate.sample_copy;
		
vee_heartrate_sample_ret:

	// post another sample timer.
	r1 = &$vee.heartrate.sample_timer_struc;
	r2 = $HR_SAMPLE_PERIOD;
	r3 = &$vee.heartrate.sample_timer_handler;
	call $timer.schedule_event_in_period;
 
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.heartratevalue.read
//
// DESCRIPTION:
//   get the heart rate adc value from i2c bus.
//
// INPUTS:
//    none
//
// OUTPUTS:
//    r3 = adc value ,if r3 = -1 ,read error
//
// TRASHED REGISTERS:
//    r8
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartratevalue.read:
	$push_rLink_macro;

	/*** frame write config register   ***/
	// send i2c start bit
	call $i2c.start_bit;	

	// write to config register
	r0 =  $I2C_HB_WD;    // send first byte 0b1001 0000
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump send_err;
	
	r0 = $I2C_HB_CFG_REG;
	call $i2c.send_byte;    // send second byte 0b0000 0001
	NULL = r0;
	if NZ jump send_err;

	r0 = $I2C_HB_CFG_MSB;       // send third byte 0b1000 0101
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump send_err;

	r0 = $I2C_HB_CFG_LSB;
	call $i2c.send_byte;    // send fourth byte 0b1000 0011 
	NULL = r0;
	if NZ jump send_err;

	// send i2c stop bit
	call $i2c.stop_bit;

	/*** frame write pointer register   ***/
	// send i2c start bit
	call $i2c.start_bit;

	// write to pointer register
	r0 =  $I2C_HB_WD;     // send first byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump send_err;

	r0 = $I2C_HB_CVS_REG;   // send second byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump send_err;

	// send i2c stop bit
	call $i2c.stop_bit;

	/*** frame read conversion register   ***/
	// send i2c start bit
	call $i2c.start_bit;

	// read conversion register
	r0 =  $I2C_HB_RD;     // send first byte
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump send_err;

	r0 = 1;
	call $i2c.receive_byte;  
	M[value_msb] = r0;

	r0 = 1;
	call $i2c.receive_byte;
	M[value_lsb] = r0;

	// send i2c stop bit
	call $i2c.stop_bit;

	r1 = M[value_msb];
	r1 = r1 ASHIFT 8;
	r3 = r1 + M[value_lsb];
	r3 = r3 LSHIFT -4;          // 2^4代表一个LSB

	NULL = r3 - 0x7ff;
	if POS r3 = 0;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

send_err:
	r3 = -1;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
//// *******************************************************************************************
// MODULE:
//	  vee.heartrate.sample_copy
//
// DESCRIPTION:
//	 copy the sample_cbuffer to calc_cbuffer and reset the sample_cbuffer read address point
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  none
//
// TRASHED REGISTERS:
//
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I4, L4, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.sample_copy:
	// Push rLink onto stack
	$push_rLink_macro;

	// get the sample_cbuffer write address point
	r0 = &$vee.heartrate.sample_cbuffer_struc;	
#ifdef BASE_REGISTER_MODE
	call $cbuffer.get_read_address_and_size_and_start_address;
	push r2;
	pop B4;
#else
	call $cbuffer.get_read_address_and_size;
#endif
	I0 = r0;
	L0 = r1;

	// get the calc_cbuffer write address point
	r0 = &$vee.heartrate.calc_cbuffer_struc;	
#ifdef BASE_REGISTER_MODE
	call $cbuffer.get_write_address_and_size_and_start_address;
	push r2;
	pop B4;
#else
	call $cbuffer.get_write_address_and_size;
#endif
	I4 = r0;
	L4 = r1;

	// copy the value of sample_cbuffer to calc_cbuffer
	r10 = $HR_SAMPLE_POINT_NUM;
	do cbuf_copy;
		r1 = M[I0, 1];
		M[I4, 1] = r1;
	cbuf_copy:

	// update the calc_cbuffer write address point
	r0 = &$vee.heartrate.calc_cbuffer_struc; 
	r1 = I4;
	call $cbuffer.set_write_address;
	L0 = 0;

	// get the sample_cbuffer read address point
	r0 = &$vee.heartrate.sample_cbuffer_struc;	
#ifdef BASE_REGISTER_MODE
	call $cbuffer.get_read_address_and_size_and_start_address;
	push r2;
	pop B4;
#else
	call $cbuffer.get_read_address_and_size;
#endif
	I0 = r0;
	L0 = r1;

#ifdef HB_ORIGIN_VALUE
	I4 = &$vee.heartrate.send_buff;
	L4 = 0;
#endif

	// update read address
	r10 = $HR_SAMPLE_FREQUENCE;
	do move_read_address;
		r1 = M[I0, 1];
#ifdef HB_ORIGIN_VALUE		
		M[I4, 1] = r1;
#endif
	move_read_address:

	r0 = &$vee.heartrate.sample_cbuffer_struc; 
	r1 = I0;
	call $cbuffer.set_read_address;
	L0 = 0;
        
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	

//// *******************************************************************************************
// MODULE:
//	  vee.heartrate.calc_cbuffer_copy
//
// DESCRIPTION:
//	 copy the calc_cbuffer to calc_buff and clear the calc_cbuffer
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  none
//
// TRASHED REGISTERS:
//
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I4, L4, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.calc_cbuffer_copy:
	// Push rLink onto stack
	$push_rLink_macro;

	// get the calc_cbuffer read address point
	r0 = &$vee.heartrate.calc_cbuffer_struc;	
#ifdef BASE_REGISTER_MODE
	call $cbuffer.get_read_address_and_size_and_start_address;
	push r2;
	pop B4;
#else
	call $cbuffer.get_read_address_and_size;
#endif
	I0 = r0;
	L0 = r1;

	I4 = &$vee.heartrate.calc_buff;
	L4 = 0;

    r10 = $HR_SAMPLE_POINT_NUM;
	do calc_cbuffer_copy1;
		r1 = M[I0, 1];
		M[I4, 1] = r1;
	calc_cbuffer_copy1:

	// clear the calc_cbuffer
	r0 = &$vee.heartrate.calc_cbuffer_struc;
    call $cbuffer.empty_buffer;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

    
//// *******************************************************************************************
// MODULE:
//	  input
//
// DESCRIPTION:
//	 input one sample to cbuffer.
//
// INPUTS:
//	  r3 = sample value
//
// OUTPUTS:
//	  none 
//
// TRASHED REGISTERS:
//	  r8
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.input:
	// Push rLink onto stack
	$push_rLink_macro;

	// get the output cbuffer address into r0
	r0 = &$vee.heartrate.sample_cbuffer_struc;
#ifdef BASE_REGISTER_MODE
	call $cbuffer.get_write_address_and_size_and_start_address;
	push r2;
	pop B4;
#else
	call $cbuffer.get_write_address_and_size;
#endif
	I0 = r0;
	L0 = r1;
	
	r0 = r3;
	M[I0, 1] = r0;

	r0 = &$vee.heartrate.sample_cbuffer_struc;
	r1 = I0;
	call $cbuffer.set_write_address;
	L0 = 0;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    vee.heartrate.average_hb
//
// DESCRIPTION:
//	calc the heartrate average value 
//	
// INPUTS:
//    r0 : 总数据点个数
//    r1 : 周期个数
//    
// OUTPUTS:
//    r0 : 心率平均值
//    
// TRASHED REGISTERS:
// 
//    Called buffer routines also trash:
//    r0, r1, r2
//
// *******************************************************************************************
$vee.heartrate.average_hb:
	$push_rLink_macro;

	/* 计算平均心率值 */
	r2 = $HR_SAMPLE_TIMES;   
	r0 = r0*r2 (int);          // hb all peroid times
    
	rMAC = r0 ASHIFT 0 (LO); 
	Div = rMAC / r1;
	r1 = DivResult;

	r2 = 60000;                 // 1min
 	rMAC = r2 ASHIFT 0 (LO); 
				           
	Div = rMAC / r1;
	r0 = DivResult;

    // 心率有效值范围 50 - 150
	NULL = r0 - HB_MIN_VALUE;
	if NEG r0 = 0;
	NULL = r0 - HB_MAX_VALUE;
	if POS r0 = 0;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
//// *******************************************************************************************
// MODULE:
//    vee.heartrate.calc
//
// DESCRIPTION:
//    calc the heartratevalue and send to vm
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
// 
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.calc:
	$push_rLink_macro;
    
	// block the interrupt
	call $interrupt.block;
	
	//check if the calc_cbuffer amount is enough to calculate
	r0 = &$vee.heartrate.calc_cbuffer_struc;
	call $cbuffer.calc_amount_data; 

	NULL = r0 - $HR_SAMPLE_POINT_NUM;
	if NEG jump do_not_calc;

	// unblock the interrupt
	call $interrupt.unblock;        

	// copy calc_cbuffer to calc_buff
	call $vee.heartrate.calc_cbuffer_copy;

	// calculate the heartrate value	
	r0 = &$vee.heartrate.calc_buff;
    r1 = $HR_SAMPLE_POINT_NUM;
	call $vee.heartrate.hb_calc;

	call $vee.heartrate.hb_smooth;

	call $vee.heartrate.output;

#ifdef HB_ORIGIN_VALUE
    // send send_buff to vm
    r3 = $DEBUG_HB_ORIGIN_VALUE_ID;
    r4 = $HR_SAMPLE_FREQUENCE;
    r5 = &$vee.heartrate.send_buff;
    call $message.send_long;
#endif

    r0 = &$vee.heartrate.hb_addr_min;
    r1 = HB_TEMP_BUFF;
    call $vee.heartrate.zero_buff;

    r0 = &$vee.heartrate.hb_addr_max;
    r1 = HB_TEMP_BUFF;
    call $vee.heartrate.zero_buff;
	
#ifdef METHOD_TEST
    r0 = &$vee.heartrate.hb_temp_buff;
    r1 = HB_TEMP_BUFF;
	call $vee.heartrate.test_output;

    r0 = &$vee.heartrate.hb_temp_buff;
    r1 = HB_TEMP_BUFF;
	call $vee.heartrate.zero_buff;

#endif  // METHOD_TEST

#ifdef DEBUG_TEST
    r0 = &$vee.heartrate.hb_debug_buff;
    r1 = HB_TEMP_BUFF;
	call $vee.heartrate.test_output;

    r0 = &$vee.heartrate.hb_debug_buff;
    r1 = HB_TEMP_BUFF;
	call $vee.heartrate.zero_buff;

#endif  // DEBUG_TEST

	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

do_not_calc:

	// unblock the interrupt
	call $interrupt.unblock;        

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.heartrate.zero_buff
//
// DESCRIPTION:
//	  zero the buff
// INPUTS:
//    r0 : address of the buff
//    r1 : size of the buff	
//    
// OUTPUTS:
//    none
//    
// TRASHED REGISTERS:
// 
//    Called buffer routines also trash:
//    r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.zero_buff:
	// Push rLink onto stack
	$push_rLink_macro;
	
	r10 = r1;
	I0 = r0;
	L0 = 0;
	r0 = 0;
	do vee_zero_buff;
		M[I0, 1] = r0;
	vee_zero_buff:

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

	
//// *******************************************************************************************
// MODULE:
//    vee.heartrate.test_output
//
// DESCRIPTION:
//	
// INPUTS:
//    r0 : 源数据地址
//    r1 : 源数据长度	
//    
// OUTPUTS:
//    none
//    
// TRASHED REGISTERS:
// 
//    Called buffer routines also trash:
//    r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.test_output:
	// Push rLink onto stack
	$push_rLink_macro;

	// send test value to vm
	r3 = $DEBUG_HB_ORIGIN_VALUE_ID;
	r4 = r1;
	r5 = r0;
	call $message.send_long;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.heartrate.output
//
// DESCRIPTION:
//   send the heartrate value to vm
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.heartrate.output:
   $push_rLink_macro;	
   
   r3 = M[heartrate];	
   r1 = M[$vee.accelerate.acc_sample_enable];
   NULL = r1 - 1;
   if Z jump sport_data_output;
   
   r2 = HEARTRATE_VALUE_TO_VM_ID;
   call $message.send_short;

   jump $pop_rLink_and_rts;   
   
sport_data_output:

   r2 = SPORT_DATA_VALUE_TO_VM_ID; 
   r4 = M[$vee.accelerate.step_value];
   call $interrupt.block;
   r6 = 0;
   M[$vee.accelerate.step_value] = r6;
   call $interrupt.unblock;
   r5 = M[time_value];
   call $message.send_short;

   jump $pop_rLink_and_rts;

.ENDMODULE;

//// *******************************************************************************************
// MODULE:
//    vee.heartrate.hb_calc
//
// DESCRIPTION:
//	calc the heartratevalue 
//	HB_MIN_POINT     心率周期最小数据点个数
//	HB_MAX_POINT     心率周期最大数据点个数
// INPUTS:
//    r0 : 源数据地址
//    r1 : 源数据长度	（r1 > 1）
// 
//
// OUTPUTS:
//   r0 : 心率值
//    
// TRASHED REGISTERS:
// 
//    Called buffer routines also trash:
//    r0, r1, r2, r3, r4, r5, r6, r7, r8, r10, I0,L0,DO LOOP
//
// *******************************************************************************************
.MODULE $vee.heartrate.hb_calc;
	.CODESEGMENT PM;
	.DATASEGMENT DM;

#ifdef HZ_40
	.CONST HB_MIN_POINT          16;
	.CONST HB_MAX_POINT          50;
	.CONST HB_MAX_MIN_LEN		 8;
	.CONST HB_MIN_MAX_LEN		 4;
	.CONST HB_POINT_THRESHOLD	 8;	
#else
	.CONST HB_MIN_POINT          8;
	.CONST HB_MAX_POINT          25;
	.CONST HB_MAX_MIN_LEN		 4;
	.CONST HB_MIN_MAX_LEN		 2;
	.CONST HB_POINT_THRESHOLD	 4;	
#endif
	.CONST HB_VALIDE_POINT		 5;

	.VAR status = 0;		// 0:表示上升沿； 1:表示下降沿	
	.VAR max_flag = 1;
	.VAR min_flag = 1;	
	.VAR max_index = 0;
	.VAR min_index = 0;
	.VAR max_value = 0;
	.VAR min_value = 2000;

$vee.heartrate.hb_calc:
	$push_rLink_macro;
	
	/*初始化变量*/
	r3 = 0;
	M[status] = r3;
	M[max_value] = r3;
	r3 = 1;
	M[max_flag] = r3;
	M[min_flag] = r3;
	r3 = 2000;
	M[min_value] = r3;
	r3 = HB_MIN_POINT;
	r3 = -r3;
	M[max_index] = r3;
	M[min_index] = r3;	

	I0 = r0;
	L0 = 0;
	r10 = r1 - 1;
	
	r0 = 0;			// r0: 存取当前值
	r1 = M[I0, 1];	// r1: 存储前一个值
	r2 = 0;			// r2: 存取当前值的下标
	r3 = 0;			// r3: temp
	r6 = 0;       	// r6: temp
	
	r4 = 0;			// r4: temp
	r5 = 0;			// r5: 存取当前极大值个数

hb_calc_loop:
	r0 = M[I0, 1];
	r2 = r2 + 1;
	
	NULL = r0 - r1;
	if NEG jump hb_check_max_value;
	
	NULL = r1 - r0;
	if NEG jump hb_check_min_value;
	
	jump hb_continue_check;
	
hb_check_max_value:

	r3 = M[status];
	NULL = r3;
	if NZ jump hb_continue_check;
	
	r3 = 1;
	M[status] = r3;		// 设置为下降沿
	
	r3 = M[min_flag];
	NULL = r3;
	if Z jump hb_update_max_value;	
		
	r3 = M[min_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MIN_MAX_LEN;
	if NEG jump hb_continue_check;
	
	r3 = M[max_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MIN_POINT;
	if NEG jump hb_continue_check;
	
	r3 = 0;
	M[min_flag] = r3;
	r3 = 1;
	M[max_flag] = r3;
	r4 = M[max_index];
	r4 = r2 - r4;
	M[&$vee.heartrate.hb_addr_min + r5] = r4;	// 存取周期时间
#ifdef METHOD_TEST
	M[&$vee.heartrate.hb_temp_buff + r5] = r4;
#endif	
	M[max_index] = r2;
	M[max_value] = r1;
	r5 = r5 + 1;
	
	jump hb_continue_check;
	
hb_update_max_value:

	r3 = M[max_value];
	NULL = r3 - r1;
	if POS jump hb_continue_check;
	
	r3 = M[max_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MIN_POINT;
	if POS jump hb_continue_check;	
	
	r5 = r5 - 1;
	r4 = M[&$vee.heartrate.hb_addr_min + r5];
	r4 = r4 + r3;
	M[&$vee.heartrate.hb_addr_min + r5] = r4;	// 更新周期时间	
#ifdef METHOD_TEST
	M[&$vee.heartrate.hb_temp_buff + r5] = r4;
#endif		
	r5 = r5 + 1;
	M[max_index] = r2;
	M[max_value] = r1;
	
	jump hb_continue_check;
	
hb_check_min_value:

	r3 = M[status];
	NULL = r3;
	if Z jump hb_continue_check;
	
	r3 = 0;
	M[status] = r3;		// 设置为上升沿
	
	r3 = M[max_flag];
	NULL = r3;
	if Z jump hb_update_min_value;		
	
	r3 = M[max_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MAX_MIN_LEN;
	if NEG jump hb_continue_check;
	
	r3 = M[min_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MIN_POINT;
	if NEG jump hb_continue_check;	
	
	r3 = 0;
	M[max_flag] = r3;
	r3 = 1;
	M[min_flag] = r3;
	M[min_value] = r1;
	M[min_index] = r2;
	
	jump hb_continue_check;	
	
hb_update_min_value:

	r3 = M[min_value];
	NULL = r1 - r3;
	if POS jump hb_continue_check;
	
	r3 = M[min_index];
	r3 = r2 - r3;
	NULL = r3 - HB_MIN_POINT;
	if POS jump hb_continue_check;	
	
	M[min_index] = r2;
	M[min_value] = r1;	
	
hb_continue_check:
	r1 = r0;			// 存取当前值
	r10 = r10 - 1;	
	NULL = r10;
	if NZ jump hb_calc_loop;
	
	/* 计算心率周期个数以及总时长 */
	r0 = 0;
	NULL = r5 - HB_VALIDE_POINT;	// r5 :原始统计周期个数
	if NEG jump invalide_exit;
	
	
	/****** 密集区查找 *****************/
	r1 = 0;		// 外循环自增量
	r2 = 0;		// 内循环自增量
	r3 = 0;		// 外循环当前值
	r4 = 0;		// 内循环当前值
	r6 = 0;		// temp 

hb_outside_loop:
	r0 = 0;		// 统计个数
	r2 = 0;
	r3 = M[&$vee.heartrate.hb_addr_min + r1];
	
hb_inside_loop:
	r4 = M[&$vee.heartrate.hb_addr_min + r2];
	NULL = r3 - r4;
	if POS r6 = r3 - r4;
	NULL = r3 - r4;
	if NEG r6 = r4 - r3;
	NULL = r6 - HB_POINT_THRESHOLD;
	if NEG r0 = r0 + 1;
	r2 = r2 + 1;
	NULL = r2 - r5;
	if NEG jump hb_inside_loop;
	
	M[&$vee.heartrate.hb_addr_max + r1] = r0;
#ifdef DEBUG_TEST
	M[&$vee.heartrate.hb_debug_buff + r1] = r0;
#endif	
	r1 = r1 + 1;
	NULL = r1 - r5;
	if NEG jump hb_outside_loop;
	
	/************* 密集点有效性判断 ****************/
	r1 = 2;
	r3 = r5;
	rMAC = r3 ASHIFT 0 (LO);
	Div = rMAC / r1;
	r0 = DivResult;	
	r1 = DivRemainder;
	NULL = r1;
	if NZ r0 = r0 + 1;	// r0:最小密集点个数
	
	r1 = 0;		// 地址偏量
	r2 = 0;		// 密集值个数
	r3 = 0;		// 统计有效周期个数
	r4 = 0;		// 统计有效周期总时间
	r6 = 0;		// 当前点个数
	r10 = r5;	// r10 :原始统计周期个数
	r9 = 0;		// temp 

calc_valid_period:

	/* 判断是否在密集区内 */
	r2 = M[&$vee.heartrate.hb_addr_max + r1];	 
	r6 = M[&$vee.heartrate.hb_addr_min + r1];
	NULL = r2 - r0;
	if POS jump check_valid_period;
	jump continue_check_period;
		
check_valid_period:

	r9 = $HR_SAMPLE_POINT_NUM;
	rMAC = r9 ASHIFT 0 (LO);
	Div = rMAC / r6;
	r9 = DivResult;		// 密集点对应的周期数	
	
	r7 = 0;
	NULL = r9 - 10;
	if POS r7 = r7 + 1;
	NULL = r5 - r9;
	if NEG r7 = r7 + 1;
	NULL = r7 - 2;
	if Z jump continue_check_period;
	
	r3 = r3 + 1;
	r4 = r4 + r6;
	
continue_check_period:
	
	r1 = r1 + 1;
	r10 = r10 - 1;
	NULL = r10;
	if NZ jump calc_valid_period;

	/* 计算平均心率值 */	
	NULL = r3;
	if Z jump invalide_exit;
	
	r0 = r4;
	r1 = r3;
	call $vee.heartrate.average_hb;
	
	jump $pop_rLink_and_rts;

invalide_exit:
	r0 = 0;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
.ENDMODULE;


//// *******************************************************************************************
// MODULE:
//    vee.heartrate.hb_smooth
//
// DESCRIPTION:
//	smooth the heartrate value 
//	HB_SMOOTH_MAX    : 最大平滑阈值
//	HB_SMOOTH_MIN    ：最小平滑阈值
//
// INPUTS:
//    r0 : 原心率值
//    
// OUTPUTS:
//    none
//    
// TRASHED REGISTERS:
//     
//     
//    Called buffer routines also trash:
//    r0, r1, r2, r3, r4, r10, DO LOOP
//
// *******************************************************************************************
.MODULE $vee.heartrate.hb_smooth;
	.CODESEGMENT PM;
	.DATASEGMENT DM;

	.CONST HB_SMOOTH_THRESHOLD		4;
	.CONST HB_SMOOTH_BUFF_SIZE		10;
	.CONST HB_YZERO_THRESHOLD		5;
	.CONST HB_SMOOTH_BEFORE			0;
	.CONST HB_SMOOTH_MIDDLE			1;
	.CONST HB_SMOOTH_AFTER			2;
	.CONST HB_SMOOTH_COUNT			3;
	.CONST HB_SMOOTH_FILTER			12;
	.CONST HB_SMOOTH_ORIGIN_SIZE	6;
	
	.VAR hb_smooth_buff[HB_SMOOTH_BUFF_SIZE];
	.VAR hb_origin_data[HB_SMOOTH_ORIGIN_SIZE];
	.VAR p_smooth_value = 0;			// 前一个心率平滑值
	.VAR p_hb_value = 0;				// 前一个心率原始值
	.VAR p_yzero = 0;					// 缓冲区中连续0值个数
	.VAR hb_smooth_status = 0;			// 平滑算法的阶段


$vee.heartrate.hb_smooth:
	$push_rLink_macro;
	
	// 原始数据缓存
	r10 = HB_SMOOTH_ORIGIN_SIZE - 1;
	r4 = &$vee.heartrate.hb_smooth.hb_origin_data;
	r1 = r10;
	do add_value_to_origin_buff;
		r5 = r1 - 1;
		r2 = M[r4 + r5];
		M[r4 + r1] = r2;
		r1 = r1 - 1;
	add_value_to_origin_buff:
	M[r4] = r0;			// 原始值	
	
	r1 = M[r4 + 1];
	M[p_hb_value] = r1;
	
	r1 = M[hb_smooth_status];			//	r1 :temp 
	NULL = r1;
	if Z jump hb_smooth_before;
	
	NULL = r1 - HB_SMOOTH_MIDDLE;
	if Z jump hb_smooth_middle;
	
	NULL = r1 - HB_SMOOTH_AFTER;
	if Z jump hb_smooth_after;
	
	jump smooth_hb_value;
	
hb_smooth_before:
	
	r10 = HB_SMOOTH_COUNT ;
	I0 = &$vee.heartrate.hb_smooth.hb_origin_data;
	L0 = 0;
	r1 = 0;	
	r2 = 0;
	r3 = 0;		// sum
	r4 = 0;		// 统计合理偏差个数
	
	do desity_filter_add;
		r1 = M[I0, 1];
		r3 = r3 + r1;
		NULL = r1 - r0;
		if NEG r2 = r0 - r1;
		NULL = r1 - r0;
		if POS r2 = r1 - r0;
		NULL = r2 - HB_SMOOTH_THRESHOLD;
		if NEG r4 = r4 + 1;
	desity_filter_add:
	
	r1 = HB_SMOOTH_COUNT;
	rMAC = r3 ASHIFT 0 (LO);
	Div = rMAC / r1;
	r1 = DivResult;	
	
	NULL = r1 - HB_MIN_VALUE;
	if NEG r0 = 0;
	
	NULL = r1 - HB_MIN_VALUE;
	if NEG jump smooth_hb_value;
	
	NULL = r4 - HB_SMOOTH_COUNT;
	if NEG jump smooth_hb_value;	
	
	r2 = HB_SMOOTH_MIDDLE;	
	M[hb_smooth_status] = r2;	

	r2 = r0;
	
	r0 = &$vee.heartrate.hb_smooth.hb_smooth_buff;
	r1 = HB_SMOOTH_BUFF_SIZE;
	call $vee.heartrate.zero_buff;
	r0 = r2;
	
	jump smooth_hb_value;
	
hb_smooth_middle:
	
	/* 0值检测 */
	NULL = r0;
	if Z jump hb_smooth_zero_check;
	
hb_smooth_filter:
	
	r1 = M[p_smooth_value];
	
	r3 = r1 + HB_SMOOTH_THRESHOLD;	
	r2 = r1 + HB_SMOOTH_FILTER;

	NULL = r0 - r2;
	if POS jump calc_smooth_hb;
	
	r3 = r1 - HB_SMOOTH_THRESHOLD;	
	r2 = r1 - HB_SMOOTH_FILTER;

	NULL = r2 - r0;
	if POS jump calc_smooth_hb;	

	r3 = r1;	
	r2 = r1 + HB_SMOOTH_THRESHOLD;
	
	NULL = r0 - r2;
	if POS jump calc_smooth_hb;

	r3 = r1;	
	r2 = r1 - HB_SMOOTH_THRESHOLD;
	
	NULL = r2 - r0;
	if POS jump calc_smooth_hb;
	
	r3 = r0;

	jump calc_smooth_hb;
	
hb_smooth_zero_check:
	
	r2 = 0;
	r3 = M[p_yzero];
	r3 = r3 + 1;
	r1 = M[p_hb_value];
	NULL = r1;
	if NZ r3 = r2;
	M[p_yzero] = r3;
	
	/* 检测是否进入结束阶段 */
	r1 = M[hb_smooth_status];
	r2 = HB_SMOOTH_AFTER;
	NULL = r3 - HB_YZERO_THRESHOLD;
	if POS r1 = r2;
	M[hb_smooth_status] = r1;
	
	jump hb_smooth_filter;
	
hb_smooth_after:
	r0 = 0;
	M[hb_smooth_status] = r0;
	M[p_yzero] = r0;
	
	r0 = &$vee.heartrate.hb_smooth.hb_smooth_buff;
	r1 = HB_SMOOTH_BUFF_SIZE;
	call $vee.heartrate.zero_buff;
	
	r0 = &$vee.heartrate.hb_smooth.hb_origin_data;
	r1 = HB_SMOOTH_ORIGIN_SIZE;
	call $vee.heartrate.zero_buff;	
	
	r0 = 0;
	jump smooth_hb_value;	

calc_smooth_hb:

	r10 = HB_SMOOTH_BUFF_SIZE - 1;
	r4 = &$vee.heartrate.hb_smooth.hb_smooth_buff;
	r1 = r10;
	do add_value_to_fifo;
		r5 = r1 - 1;
		r2 = M[r4 + r5];
		M[r4 + r1] = r2;
		r1 = r1 - 1;
	add_value_to_fifo:
	M[r4] = r3;			// 平滑值

	/* 求平均值 */
	r10 = HB_SMOOTH_BUFF_SIZE;
	r0 = 0;		// 统计非零值的和
	r1 = 0;		// 统计非零值的个数
	r2 = 0;		// hb_smooth_buff的下标
	r3 = 0;		// hb_smooth_buff中的值
	do smooth_average;
		r3 = M[r4 + r2];
		NULL = r3;
		if NZ r0 = r0 + r3;
		NULL = r3;
		if NZ r1 = r1 + 1;
		r2 = r2 + 1;
	smooth_average:

	NULL = r0;
	if Z jump smooth_hb_value;

	rMAC = r0 ASHIFT 0 (LO);
	Div = rMAC / r1;
	r0 = DivResult;
	
smooth_hb_value:

    M[$vee.heartrate.heartrate] = r0;
	M[p_smooth_value] = r0;
    
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
//// *******************************************************************************************
// MODULE:
//    vee.heartrate.hb_recover
//
// DESCRIPTION:
//	
//
// INPUTS:
//    r1 : 历史心率值
//    
// OUTPUTS:
//    none
//    
// TRASHED REGISTERS:
//     
//     
//    Called buffer routines also trash:
//    r0, r10, I0, L0,DO LOOP
//
// *******************************************************************************************
$vee.heartrate.hb_recover:
	// Push rLink onto stack
	$push_rLink_macro;	
	
	I0 = &$vee.heartrate.hb_smooth.hb_smooth_buff;
	L0 = 0;
	r10 = HB_SMOOTH_BUFF_SIZE;
	do hb_value_recover;
		M[I0, 1] = r1;
	hb_value_recover:
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

.ENDMODULE;


//******************** LIS3DH accelerate sensor *********************************

#define ACC_ORIGIN_VALUEx
#define ACC_VALUE_DATA
#define $DATA_SIZE 	3

#define $ACC_SAMPLE_PERIOD	40*1000 // accelerate sample timer peroid 40ms
#define $ACC_READ_SIZE		6
#define $ACC_ANGLE_SIZE		6	
#define $ACC_LSD		0.00024414	// 10HZ LSD (1/4096)
//#define $ACC_LSD		0.00012207	// 1HZ LSD  (1/8192)
//#define $ACC_LSD      0.0009765625   // 25HZ LSD (1/1024)

#define	$I2C_AUTO_INCREMENT	0x80

#define $I2C_LIS3DH_WD		0x30	// accelerate write device address
#define $I2C_LIS3DH_RD		0x31	// accelerate read device address

// WHO AM I
#define $WHO_AM_I		0x0f

// STATUS REGISTER
#define $LIS3DH_STATUS_REG	0x27

// TEMP_CFG_REG 
#define $LIS3DH_TEMP_CFG_REG	0x1f

#define	$LIS3DH_FIFO_CTRL_REG	0x2E	/*	FiFo control reg	*/
#define	$LIS3DH_INT_CFG1	0x30	/*	interrupt 1 config	*/
#define	$LIS3DH_INT_THS1	0x32	/*	interrupt 1 threshold	*/
#define	$LIS3DH_INT_DUR1	0x33	/*	interrupt 1 duration	*/


#define	$LIS3DH_TT_CFG		0x38	/*	tap config		*/
#define	$LIS3DH_TT_THS		0x3A	/*	tap threshold		*/
#define	$LIS3DH_TT_LIM		0x3B	/*	tap time limit		*/
#define	$LIS3DH_TT_TLAT		0x3C	/*	tap time latency	*/
#define	$LIS3DH_TT_TW		0x3D	/*	tap time window		*/

// CONTROL REGISTER 1
#define $LIS3DH_CTRL_REG1	0x20
#define $LIS3DH_CTRL_REG1_VALUE 0x27	

#define $LIS3DH_CTRL_REG2	0x21
#define $LIS3DH_CTRL_REG3	0x22
#define $LIS3DH_CTRL_REG5	0x24
#define $LIS3DH_CTRL_REG6	0x25

//CONTROL REGISTER 4
#define $LIS3DH_CTRL_REG4	0x23
#define $LIS3DH_CTRL_REG4_VALUE 0x84

//OUTPUT REGISTER
#define $LIS3DH_ADC_L		0x0c
#define $LIS3DH_ADC_H		0x0d


#define $LIS3DH_OUT_MUL		0xA8	// multiple read address
#define $LIS3DH_OUT_X_L		0x28
#define $LIS3DH_OUT_X_H		0x29
#define $LIS3DH_OUT_Y_L		0x2A
#define $LIS3DH_OUT_Y_H		0x2B
#define $LIS3DH_OUT_Z_L		0x2C
#define $LIS3DH_OUT_Z_H		0x2D

// *****************************************************************************
// MODULE:
//    vee.accelerate
//
// DESCRIPTION:
//    sample the accelerate value and calcurate the angle and step
//
// *****************************************************************************
.MODULE $vee.accelerate;
	.CODESEGMENT PM;
	.DATASEGMENT DM;  

	.VAR  acc_sample_enable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  acc_sample_disable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  step_value_enable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  neck_protect_enable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  safe_driver_enable_from_vm_message_struc[$message.STRUC_SIZE];
	.VAR  const_seat_enable_from_vm_message_struc[$message.STRUC_SIZE];
	

   	.VAR acc_timer_struc[$timer.STRUC_SIZE];
   	.VAR acc_read_buf[$ACC_READ_SIZE];
   	.VAR acc_calc_buf[$ACC_READ_SIZE];
   	.VAR acc_angle_buf[$ACC_ANGLE_SIZE];

   	.VAR acc_step_buf[3];
	.VAR step_value = 0;

   	.VAR acc_sample_enable = 0;
   	.VAR step_value_enable = 0;
   	.VAR safe_driver_enable = 0;
   	.VAR neck_protect_enable = 0;
   	.VAR const_seat_enable = 0;	

 	.VAR acc_valid = 0;

   	.VAR angle_x_int = 0;
   	.VAR angle_y_int = 0;
   	.VAR angle_z_int = 0;
   	.VAR acc_axis_x = 0;
   	.VAR acc_axis_y = 0;
   	.VAR acc_axis_z = 0;
	

#ifdef ACC_VALUE_DATA
	.VAR acc_origin_data[$DATA_SIZE];
#endif

//// *******************************************************************************************
// MODULE:
//	  vee.accelerate.start
//
// DESCRIPTION:
//	 start accelerate module.
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  r8 = max 
//
// TRASHED REGISTERS:
//	  r8
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.start:
	// Push rLink onto stack
	$push_rLink_macro;	

	call $vee.accelerate.i2c_test;
	
 	// init accelerate sensor
	call $vee.accelerate.init;

	// register message handler for VM_ACCELERATE_SAMPLE_START_ID message
	r1 = &acc_sample_enable_from_vm_message_struc;
	r2 = VM_ACCELERATE_SAMPLE_START_ID;
	r3 = &$acc_sample_enable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_ACCELERATE_SAMPLE_STOP_ID message
	r1 = &acc_sample_disable_from_vm_message_struc;
	r2 = VM_ACCELERATE_SAMPLE_STOP_ID;
	r3 = &$acc_sample_disable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_ENABLE_STEP_VALUE_ID message
	r1 = &step_value_enable_from_vm_message_struc;
	r2 = VM_ENABLE_STEP_VALUE_ID;
	r3 = &$step_value_enable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_ENABLE_NECK_EVENT_ID message
	r1 = &neck_protect_enable_from_vm_message_struc;
	r2 = VM_ENABLE_NECK_EVENT_ID;
	r3 = &$neck_protect_enable_cmd_from_vm;
	call $message.register_handler;

	// register message handler for VM_ENABLE_DRIVER_EVENT_ID message
	r1 = &safe_driver_enable_from_vm_message_struc;
	r2 = VM_ENABLE_DRIVER_EVENT_ID;
	r3 = &$safe_driver_enable_cmd_from_vm;
	call $message.register_handler;
	
	// register message handler for VM_ENABLE_SEAT_EVENT_ID message
	r1 = &const_seat_enable_from_vm_message_struc;
	r2 = VM_ENABLE_SEAT_EVENT_ID;
	r3 = &$const_seat_enable_cmd_from_vm;
	call $message.register_handler;	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//	vee.accelerate.i2c_test
//
// DESCRIPTION:
//	 test the heartrate sensor can communication with i2c bus.
//
// INPUTS:
//	  none
//
// OUTPUTS:
//	  none
//
// TRASHED REGISTERS:
//	  
//	  Called buffer routines also trash:
//	  r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.i2c_test:
	// Push rLink onto stack
	$push_rLink_macro;

	r0 = $WHO_AM_I;
	call $vee.accelerate.read_reg;
	NULL = r0 - 0x33;
	if NZ jump acc_i2c_error;

	// send test failed result to vm
	r2 = ACC_I2C_TEST_TO_VM_ID;
	r3 = 0;		// r3 = 0, success.
	call $message.send_short;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

acc_i2c_error:

	// send test failed result to vm
	r2 = ACC_I2C_TEST_TO_VM_ID;
	r3 = 1;		// r3 = 1, failed.
	call $message.send_short;	

	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    acc_sample_enable_cmd_from_vm
//
// DESCRIPTION:
//   enable the accelerate sample
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3
//
// *******************************************************************************************
$acc_sample_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	r1 = M[acc_sample_enable];
	NULL = r1 - 1;
	if POS jump $pop_rLink_and_rts;

	call $interrupt.block;
	r1 = 1;
	M[acc_sample_enable] = r1;
	call $interrupt.unblock;
	
 	// start timer that sample the accelerate 
	r1 = &$vee.accelerate.acc_timer_struc;
	r2 = $ACC_SAMPLE_PERIOD;
	r3 = &$vee.accelerate.sample_timer_handler;
	call $timer.schedule_event_in;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

	
//// *******************************************************************************************
// MODULE:
//    acc_sample_disable_cmd_from_vm
//
// DESCRIPTION:
//   disable the accelerate sample
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1
//
// *******************************************************************************************
$acc_sample_disable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	call $interrupt.block;
	r1 = 0;
	M[acc_sample_enable] = r1;
	M[step_value_enable] = r1;	
	M[neck_protect_enable] = r1;
	M[safe_driver_enable] = r1;
	M[const_seat_enable] = r1;
	call $interrupt.unblock;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    step_value_enable_cmd_from_vm
//
// DESCRIPTION:
//   enable the send step value to vm
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1
//
// *******************************************************************************************
$step_value_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	call $interrupt.block;
	r1 = 1;
	M[step_value_enable] = r1;
	call $interrupt.unblock;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    neck_protect_enable_cmd_from_vm
//
// DESCRIPTION:
//   enable the neck protect
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1
//
// *******************************************************************************************
$neck_protect_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	call $interrupt.block;
	r1 = 1;
	M[neck_protect_enable] = r1;
	call $interrupt.unblock;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    safe_driver_enable_cmd_from_vm
//
// DESCRIPTION:
//   disable the neck protect
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1
//
// *******************************************************************************************
$safe_driver_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	call $interrupt.block;
	r1 = 1;
	M[safe_driver_enable] = r1;
	call $interrupt.unblock;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	
	
//// *******************************************************************************************
// MODULE:
//    const_seat_enable_cmd_from_vm
//
// DESCRIPTION:
//   disable the neck protect
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1
//
// *******************************************************************************************
$const_seat_enable_cmd_from_vm:
	// Push rLink onto stack
	$push_rLink_macro;

	call $interrupt.block;
	r1 = 1;
	M[const_seat_enable] = r1;
	call $interrupt.unblock;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.init
//
// DESCRIPTION:
//   init the accelerate sensor config register.
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.init:
	// Push rLink onto stack
	$push_rLink_macro;

	//set CTRL_REG1
	r0 = $LIS3DH_CTRL_REG1;
	r1 = $LIS3DH_CTRL_REG1_VALUE;
	call $vee.accelerate.write_reg;

	//set CTRL_REG2
	r0 = $LIS3DH_CTRL_REG2;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set CTRL_REG3
	r0 = $LIS3DH_CTRL_REG3;
	r1 = 0x0;
	call $vee.accelerate.write_reg;
	
	//set CTRL_REG4 
	r0 = $LIS3DH_CTRL_REG4;
	r1 = $LIS3DH_CTRL_REG4_VALUE;
	call $vee.accelerate.write_reg;

	//set CTRL_REG5
	r0 = $LIS3DH_CTRL_REG5;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set CTRL_REG6
	r0 = $LIS3DH_CTRL_REG6;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TEMP_CFG_REG
	r0 = $LIS3DH_TEMP_CFG_REG;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set FIFO_CTRL_REG
	r0 = $LIS3DH_FIFO_CTRL_REG;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set INT_CFG1
	r0 = $LIS3DH_INT_CFG1;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set INT_THS1
	r0 = $LIS3DH_INT_THS1;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set INT_DUR1
	r0 = $LIS3DH_INT_DUR1;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TT_CFG
	r0 = $LIS3DH_TT_CFG;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TT_THS
	r0 = $LIS3DH_TT_THS;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TT_LIM
	r0 = $LIS3DH_TT_LIM;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TT_TLAT
	r0 = $LIS3DH_TT_TLAT;
	r1 = 0x0;
	call $vee.accelerate.write_reg;

	//set TT_TW
	r0 = $LIS3DH_TT_TW;
	r1 = 0x0;
	call $vee.accelerate.write_reg;	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.read_reg
//
// DESCRIPTION:
//   read the value of the accelerate register.
//
// INPUTS:
//    r0 : the address of the register.
//    
// OUTPUTS:
//    r0 : the value read from the register (8 bit). if r0 = -1, read erro.
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.read_reg:
	// Push rLink onto stack
	$push_rLink_macro;

	r7 = r0; // store the register address

	// step1 send i2c start bit
	call $i2c.start_bit;	

	// step2 send the device address to i2c bus
	r0 =  $I2C_LIS3DH_WD;    
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump read_reg_err;

	// step3 send register address to i2c bus
	r0 = r7;
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump read_reg_err;

	// step4 send i2c start bit
	call $i2c.start_bit;	
	
	// step5 send the device address to i2c bus(read)
	r0 =  $I2C_LIS3DH_RD;    
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump read_reg_err;

	// ste6 receive the data from the i2c bus
	r0 = $I2C_NMAK;
	call $i2c.receive_byte;
	r8 = r0;

	// step7 send i2c stop bit
	call $i2c.stop_bit;
	r0 = r8;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

read_reg_err:

	r0 = -1;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;


//// *******************************************************************************************
// MODULE:
//    vee.accelerate.write_reg
//
// DESCRIPTION:
//   write the value of the accelerate register.
//
// INPUTS:
//    r0 : the address of the register.
//    r1 : the value write to the register.
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r0,r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.write_reg:
	// Push rLink onto stack
	$push_rLink_macro;

	r7 = r0; // store the register address
	r8 = r1; // store the value write to the register

	// step1 send i2c start bit
	call $i2c.start_bit;	

	// step2 send the device address to i2c bus
	r0 =  $I2C_LIS3DH_WD;    
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump write_reg_err;

	// step3 send register address to i2c bus
	r0 = r7;
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump write_reg_err;

	// step4 send the value to set the register
	r0 = r8;
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump write_reg_err;

	// step5 send i2c stop bit
	call $i2c.stop_bit;

write_reg_err:

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.sample_timer_handler
//
// DESCRIPTION:
//   read accelerate value from the i2c bus
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.sample_timer_handler:
	// Push rLink onto stack
	$push_rLink_macro;

	r0 = M[acc_sample_enable];
	NULL = r0;
	if Z jump $pop_rLink_and_rts;

	r0 = $LIS3DH_STATUS_REG;
	call $vee.accelerate.read_reg;
	
	NULL = r0 ;
	if NEG jump no_validle_data;
	NULL = r0 ;
	if Z jump no_validle_data;
 
	// lock the critical zone
	call $interrupt.block;

	// read data from i2c bus
	r7 = &$vee.accelerate.acc_read_buf;
	r8 = $ACC_READ_SIZE;
	call $vee.accelerate.read;

	r0 = 1;
	M[acc_valid] = r0;	

	call $interrupt.unblock;
	
#ifdef ACC_ORIGIN_VALUE

	r3 = $DEBUG_ACC_ORIGIN_VALUE_ID;
	r4 = $ACC_READ_SIZE;
	r5 = &$vee.accelerate.acc_read_buf;
	call $message.send_long;
	
#endif

no_validle_data:

	// post another timer event
	r1 = &$vee.accelerate.acc_timer_struc;
	r2 = $ACC_SAMPLE_PERIOD;
	r3 = &$vee.accelerate.sample_timer_handler;
	call $timer.schedule_event_in_period;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.read
//
// DESCRIPTION:
//   read accelerate value from the i2c bus
//
// INPUTS:
//    r7 :the receive buff address of payload
//    r8 :the length of receive data
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.read:
	// Push rLink onto stack
	$push_rLink_macro;

	// step1 send i2c start bit
	call $i2c.start_bit;	

	// step2 send the device address to i2c bus
	r0 =  $I2C_LIS3DH_WD;    
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump acc_read_err;

	// step3 send the read data address
	r0 = $LIS3DH_OUT_MUL;
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump acc_read_err;

	// step4 send i2c start bit
	call $i2c.start_bit;	
	
	// step5 send the device address to i2c bus(read)
	r0 =  $I2C_LIS3DH_RD;    
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump acc_read_err;

	// step6 multiple read data from i2c bus
	r0 = r7;
	r1 = r8;
	call $vee.accelerate.multiple_receive;

	// step7 send i2c stop bit
	call $i2c.stop_bit;

acc_read_err:

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//   vee.accelerate.multiple_send
//
// DESCRIPTION:
//   send multiple byte to i2c bus.
//
// INPUTS:
//    r0 :the multiple data address of payload
//    r1 :the size of the multiple data
//
// OUTPUTS:
//    r0 : if r0 = -1, multiple send error.
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************   
$vee.accelerate.multiple_send:
	$push_rLink_macro;
	I0 = r0;
	L0 = 0;

	r8 = r1;
	send_multiple_byte:
	
	r0 = M[I0,1];
	call $i2c.send_byte;
	NULL = r0;
	if NZ jump multiple_send_err;
	r8 = r8 - 1;
	NULL = r8;
	if NZ jump send_multiple_byte;

	r0 = 0;

	jump $pop_rLink_and_rts;

multiple_send_err:
	r0 = -1;

	jump $pop_rLink_and_rts;
	

//// *******************************************************************************************
// MODULE:
//   vee.accelerate.multiple_receive
//
// DESCRIPTION:
//   receive multiple byte from i2c bus.
//
// INPUTS:
//    r0 :the receive buff address of payload
//    r1 :the length of receive data
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************   
$vee.accelerate.multiple_receive:
	$push_rLink_macro;
	I0 = r0;
	L0 = 0;
	r8 = r1 - 1;  
	
	receive_multiple_byte:
	
	r0 = 1;
	call $i2c.receive_byte;  
	M[I0, 1] = r0;
	r8 = r8 - 1;
	NULL = r8;
	if NZ jump receive_multiple_byte;

	r0 = 0;
	call $i2c.receive_byte;
	M[I0, 1] = r0;

	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.arccos
//
// DESCRIPTION:
//   arccos(x) = 2 arctan(sqrt((1-x)/(1+x)))
//
// INPUTS:
//    r5 : input value between 0 and 2^23-1 (ie. 0 and 1.0 fractional)
//
//
// OUTPUTS:
//    r0 : the integer of angle 0 - 89 degree 
//    r1 : the fraction of angle 
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4,r5, r6, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.arccos:

	// Push rLink onto stack
	$push_rLink_macro;

	NULL = r5;
	if Z jump cos_vertical_angle;
	
	r5 = r5 LSHIFT -1;	// r5 / 2
	rMAC = 0.5 + r5;
	r0 = rMAC1;

	rMAC = 0.5 - r5;
	rMAC = rMAC ASHIFT -1;

	Div = rMAC / r0;
	r0 = DivResult;
	call $math.sqrt;
	r6 = DivResult;            
	
	r5 = 0x7fffff;		// 0.9999
	call $math.atan;		// arctan(x) = atan(1.0, x)
	rMAC = r5*360;
	r0 = rMAC1;		// integer of the angle
	r1 = rMAC0;		// fraction of the angle
	r1 = r1 LSHIFT -1;	// 去除符号位

	NULL = r9;
	if NZ r0 = - r0;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

cos_vertical_angle:

    r2 = M[angle_z_int];
    r3 = 90;
    r4 = -90;
    NULL = r2;
    if NEG r0 = r4;

    NULL = r2;
    if POS r0 = r3;
    
	r1 = 0;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

	
//// *******************************************************************************************
// MODULE:
//    vee.accelerate.arcsin
//
// DESCRIPTION:
//   arcsin(x) = 2 arctan((1 - sqrt(1-x^2))/x)
//
// INPUTS:
//    r5 : input value between 0 and 2^23-1 (ie. 0 and +/- 1.0 fractional)
//
//
// OUTPUTS:
//    r0 : the integer of angle - 89 ~ +89 degree 
//    r1 : the fraction of angle 
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4,r5, r6, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.arcsin:

	// Push rLink onto stack
	$push_rLink_macro;

	NULL = r5;
	if Z jump sin_zero_angle;
	
	r1 = r5*r5 (frac);
	rMAC = 1.0 - r1;
	r0 = rMAC1;
	call $math.sqrt;
	
	rMAC = 1.0 - r1;		// r1 = sqrt(1 - x^2)
	rMAC = rMAC ASHIFT -1;	// 符号位	
	Div = rMAC / r5;
	r6 = DivResult;            	
	
	r5 = 0x7fffff;		// 0.9999
	call $math.atan;		// arctan(x) = atan(1.0, x)
	rMAC = r5*360;
	r0 = rMAC1;		// integer of the angle
	r1 = rMAC0;		// fraction of the angle
	r1 = r1 LSHIFT -1;	// 去除符号位

	// Pop rLink from stack
	jump $pop_rLink_and_rts;	

sin_zero_angle:
	r0 = 0;
	r1 = 0;
	 
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.angle_calc
//
// DESCRIPTION:
//   calculate the three axis angle
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
$vee.accelerate.angle_calc:

	// Push rLink onto stack
	$push_rLink_macro;

	r0 = &$vee.accelerate.acc_read_buf;
	r1 = $ACC_READ_SIZE;

	// copy the acc_read_buf to acc_calc_buf
	I0 = r0;
	L0 = 0;
	I4 = &$vee.accelerate.acc_calc_buf;
	L4 = 0;
	r10 = r1;

	// 判断是否有值进行计算
	r0 = M[acc_valid];
	NULL = r0;
	if Z jump $pop_rLink_and_rts; 
	
	call $interrupt.block;

	do copy_acc_data;
		r1 = M[I0, 1];
		M[I4, 1] = r1;
	copy_acc_data:
	
	call $interrupt.unblock;

	// coversion the origion value to accelerate value
	r1 = M[&$vee.accelerate.acc_calc_buf + 0];
	r2 = M[&$vee.accelerate.acc_calc_buf + 1];
	r2 = r2 LSHIFT 8;
	r0 = r1 + r2;

	// get the accelerate axis acceleration
	call $vee.accelerate.get_acc;
	M[acc_axis_x] = r0;
	
	r1 = M[&$vee.accelerate.acc_calc_buf + 2];
	r2 = M[&$vee.accelerate.acc_calc_buf + 3];
	r2 = r2 LSHIFT 8;
	r0 = r1 + r2;

	// get the accelerate axis acceleration
	call $vee.accelerate.get_acc;
	M[acc_axis_y] = r0;

	r1 = M[&$vee.accelerate.acc_calc_buf + 4];
	r2 = M[&$vee.accelerate.acc_calc_buf + 5];
	r2 = r2 LSHIFT 8;
	r0 = r1 + r2;

	// get the accelerate axis acceleration
	call $vee.accelerate.get_acc;
	M[acc_axis_z] = r0;
	
	call $interrupt.block;
	r0 = 0;
	M[acc_valid] = r0;
	call $interrupt.unblock;
	
#ifdef ACC_VALUE_DATA
	call $vee.accelerate.acc_output;
#endif

    r0 = M[step_value_enable];
    NULL = r0;
    if Z jump dont_output_step_value;
    /* 统计步数 */
	call $vee.accelerate.step_count;
dont_output_step_value:

    r0 = M[safe_driver_enable];
    NULL = r0;
    if Z jump dont_output_safe_driver_event;    
	call $vee.accelerate.safe_driver;
dont_output_safe_driver_event:


    r0 = M[neck_protect_enable];
    NULL = r0;
    if Z jump dont_output_neck_prompt_event;
    call $vee.accelerate.neck_protect;
dont_output_neck_prompt_event:   

	r0 = M[const_seat_enable];
	NULL = r0;
	if Z jump dont_output_seat_prompt_event;
	call $vee.accelerate.const_seat;
dont_output_seat_prompt_event:

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

#ifdef 	ACC_VALUE_DATA
//// *******************************************************************************************
// MODULE:
//    vee.accelerate.acc_output
//
// DESCRIPTION:
//   send the acceleratevalue data to vm
//
// INPUTS:
//    
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r0, r1, r2, r3, r4, r5, I0, L0
//
// *******************************************************************************************
$vee.accelerate.acc_output:
	// Push rLink onto stack
	$push_rLink_macro;
	
	I0 = &$vee.accelerate.acc_origin_data;
	L0 = 0;

	r5 = M[acc_axis_x];
	M[I0, 1] = r5;
	r5 = M[acc_axis_y];
	M[I0, 1] = r5;
	r5 = M[acc_axis_z];
	M[I0, 1] = r5;

	r3 = $DEBUG_ACC_ORIGIN_VALUE_ID;
	r4 = $DATA_SIZE;
	r5 = &$vee.accelerate.acc_origin_data;
	call $message.send_long;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;

#endif

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.get_acc
//
// DESCRIPTION:
//   get the acceleration of the accelerate axis
//
// INPUTS:
//    r0 : the DAC value of the accelerate 
//    
//
// OUTPUTS:
//    r0 : 实际加速度与g常量的比值 乘以10取整
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r0,r1, r2
//
// *******************************************************************************************
$vee.accelerate.get_acc:
	// Push rLink onto stack
	$push_rLink_macro;

	r1 = 0xff0000;
	r2 = r0 AND 0x8000;

	NULL = r2;
	if NZ r0 = r0 OR r1;
	r0 = r0*10 (int);
	rMAC = r0*$ACC_LSD;
	r0 = rMAC1;
		
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
.ENDMODULE;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.const_seat
//
// DESCRIPTION:
//   
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
.MODULE $vee.accelerate.const_seat;
	.CODESEGMENT PM;
	.DATASEGMENT DM;
	
	.CONST SEAT_TIME_PERIOD			1000000;	// 1000 ms
	.CONST SEAT_TIME_THRESHOLD		30;		// 5min
	.CONST STABLE_HIGH_VALUE		130;
	.CONST STABLE_LOW_VALUE			70;
	.CONST STABLE_STATE_COUNTER		60;	// 6s
	.CONST GET_UP_MAX_THRESHOLD		140;
	.CONST GET_UP_MIN_THRESHOLD		60;
	.CONST GET_UP_VALUE_THRESHOLD	100;
	.CONST MOVE_STATE_COUNTER		10;
	.CONST MOVE_STATE_HALF_COUNT	5;
	
	.VAR seat_timer_enable = 0;
	.VAR seat_timer_count = 0;	
	.VAR move_timer_count = 0;
	.VAR stable_count = 0;
	
	.VAR p_value = 0;		// 前一个加速度矢量和
	.VAR status = 0;		// 0:表示上升沿； 1:表示下降沿	
	.VAR max_flag = 0;
	.VAR max_value = 0;	
	.VAR stable_status = 0;	// 0:表示非稳态；1:表示稳态

   	.VAR seat_timer_struc[$timer.STRUC_SIZE];		
	
$vee.accelerate.const_seat:
	$push_rLink_macro;
	
	/* 计算x轴和y轴加速度矢量和 */
	r3 = 0;
	r1 = M[$vee.accelerate.acc_axis_x];
	r1 = r1*r1 (int);
	r3 = r3 + r1;
	r1 = M[$vee.accelerate.acc_axis_y];
	r1 = r1*r1 (int);
	r3 = r3 + r1;		// r3:当前加速度矢量和	
	
	r0 = 0;				// r0 : temp
	r1 = M[p_value];	// r1 :前一个加速度矢量和
	r2 = 0; 			// r2 : temp 
	
	// 平稳态判断	
	r0 = M[stable_status];
	NULL = r0;
	if NZ jump get_up_state_check;
	
stable_state_check:
	
	NULL = r3 - STABLE_HIGH_VALUE;
	if POS jump continue_stable_state_check;
	
	NULL = r3 - STABLE_LOW_VALUE;
	if NEG jump continue_stable_state_check;
		
	r2 = M[stable_count];
	r2 = r2 + 1;
	NULL = r2 - STABLE_STATE_COUNTER;
	if POS jump start_seat_timer;
	M[stable_count] = r2;
	M[p_value] = r3;
	
	jump $pop_rLink_and_rts;		
	
continue_stable_state_check:
	r0 = 0;
	M[stable_count] = r0;
	M[p_value] = r3;
	
	jump $pop_rLink_and_rts;	
	
get_up_state_check:
	
	r0 = M[stable_status];		// 在出现稳态前不检测起身动作
	NULL = r0;
	if Z jump get_up_continue_check;
	
	NULL = r3 - r1;
	if POS jump seat_check_max_value;
	
seat_check_min_value:
	r0 = M[status];
	NULL = r0;
	if NZ jump get_up_continue_check;
	
	r0 = 1;
	M[status] = r0;		// 设置为下降沿
	
	NULL = r1 - GET_UP_MAX_THRESHOLD;
	if NEG jump get_up_continue_check;
		
	r0 = 1;
	M[max_flag] = r0;
	M[max_value] = r1;

	jump get_up_continue_check;

	
seat_check_max_value:
	r0 = M[status];
	NULL = r0;
	if Z jump get_up_continue_check;

	r0 = 0;
	M[status] = r0;		// 设置为上升沿
	
	NULL = r1 - STABLE_LOW_VALUE;
	if POS jump get_up_continue_check;
	
	call $interrupt.block;		
	r0 = 0;
	M[stable_count] = r0;
	M[stable_status] = r0;
	M[seat_timer_enable] = r0;
	r0 = M[move_timer_count];
	r0 = r0 + 1;
	M[move_timer_count] = r0;	
	call $interrupt.unblock;	
			
	NULL = r1 - GET_UP_MIN_THRESHOLD;
	if POS jump get_up_continue_check;

	r0 = M[max_flag];
	NULL = r0;
	if Z jump get_up_continue_check;
	
	r0 = M[max_value];
	r0 = r0 - r1;
	NULL = r0 - GET_UP_VALUE_THRESHOLD;
	if NEG jump get_up_continue_check;

	// 检测到起身动作，关闭定时器，归零时间累加器	
	call $interrupt.block;	
	r0 = 0;
	M[max_flag] = r0;
	M[seat_timer_count] = r0;
	call $interrupt.unblock;
	
#if 1
	r2 = DRIVER_PROMPT_EVENT_TO_VM_ID;
	call $message.send_short;	
#endif	
		
get_up_continue_check:
	M[p_value] = r3;

	jump $pop_rLink_and_rts;	
	
start_seat_timer:
	
	r0 = M[seat_timer_enable];
	NULL = r0;
	if NZ jump $pop_rLink_and_rts;	

 	// start timer that count the seat times
	r1 = &$vee.accelerate.const_seat.seat_timer_struc;
	r2 = SEAT_TIME_PERIOD;
	r3 = &$vee.accelerate.const_seat.seat_timer_handler;
	call $timer.schedule_event_in;
	
	r0 = 1;
	M[seat_timer_enable] = r0;	
	M[stable_status] = r0;	// 标识稳态	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	
$vee.accelerate.const_seat.seat_timer_handler:
	$push_rLink_macro;
	
	r0 = M[seat_timer_enable];
	NULL = r0;
	if Z jump $pop_rLink_and_rts;
	
	r0 = M[seat_timer_count];
	r0 = r0 + 1;		
	NULL = r0 - SEAT_TIME_THRESHOLD;
	if POS jump send_const_seat_event;
	M[seat_timer_count] = r0;

another_timer_event:

	// post another timer event	
	r1 = &$vee.accelerate.const_seat.seat_timer_struc;
	r2 = SEAT_TIME_PERIOD;
	r3 = &$vee.accelerate.const_seat.seat_timer_handler;
	call $timer.schedule_event_in_period;
	
	jump $pop_rLink_and_rts;		
	
send_const_seat_event:

	r0 = M[stable_status];
	NULL = r0;
	if Z jump dont_send_seat_event;

	r2 = SEAT_PROMPT_EVENT_TO_VM_ID;
	call $message.send_short;
	
	r0 = 0;
	M[seat_timer_count] = r0;
	M[seat_timer_enable] = r0;
	M[stable_status] = r0;
	M[stable_count] = r0;	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;		
	
dont_send_seat_event:

	r0 = M[seat_timer_count];
	r0 = r0 + 1;	
	M[seat_timer_count] = r0;
	
	r2 = r0 - SEAT_TIME_THRESHOLD;
	NULL = r2 - MOVE_STATE_COUNTER;
	if NEG jump another_timer_event;
	
	r0 = SEAT_TIME_THRESHOLD;
	M[seat_timer_count] = r0;	
	
	r0 = M[move_timer_count];
	r1 = 0;
	M[move_timer_count] = r1;
	
	NULL = r0 - MOVE_STATE_HALF_COUNT;
	if NEG jump another_timer_event;
	
	r0 = 0;
	M[seat_timer_count] = r0;
	
	jump another_timer_event;
	
.ENDMODULE;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.safe_driver
//
// DESCRIPTION:
//   
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
.MODULE $vee.accelerate.safe_driver;
	.CODESEGMENT PM;
	.DATASEGMENT DM;
	
	.CONST X_AXIS_THRESHOLD		25;
	.CONST Y_AXIS_THRESHOLD		64;
	.CONST Z_AXIS_THRESHOLD		25;
	
	.VAR nod_action_flag = 0;		// 点头动作标识
	.VAR shake_action_flag = 0;		// 摇头动作标识
	
$vee.accelerate.safe_driver:
	$push_rLink_macro;

	jump $pop_rLink_and_rts;		

	/* 计算三轴加速度矢量和 */
	r1 = 0;		// x轴矢量和
	r2 = 0;		// y轴矢量和
	r3 = 0;		// z轴矢量和
	r4 = 0;		// temp 
	r5 = 0;

	r1 = M[$vee.accelerate.acc_axis_x];
	r1 = r1*r1 (int);
	
	r2 = M[$vee.accelerate.acc_axis_y];
	r2 = r2*r2 (int);
	
	r3 = M[$vee.accelerate.acc_axis_z];
	r3 = r3*r3 (int);
	
	NULL = r2 - Y_AXIS_THRESHOLD;
	if NEG jump shake_action_check;
	
	r4 = M[nod_action_flag];
	NULL = r4;
	if NZ jump $pop_rLink_and_rts;
	
	NULL = r1 - X_AXIS_THRESHOLD;
	if POS jump $pop_rLink_and_rts;
	
	r4 = 1;
	M[nod_action_flag] = r4;
	
	// 发送消息到VM,检测到点头动作
	r2 = NOD_ACTION_EVENT_TO_VM_ID;
	call $message.send_short;	
	
	jump $pop_rLink_and_rts;	
	
shake_action_check:

	r4 = M[shake_action_flag];
	NULL = r4;
	if NZ jump $pop_rLink_and_rts;
	
	NULL = r3 - Z_AXIS_THRESHOLD;
	if NEG jump $pop_rLink_and_rts;
	
	r4 = 1;
	M[shake_action_flag] = r4;
	
	// 发送消息到VM，检测到摇头动作
	r2 = SHAKE_ACTION_EVENT_TO_VM_ID;
	call $message.send_short;		

	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	
.ENDMODULE;
	
//// *******************************************************************************************
// MODULE:
//    vee.accelerate.neck_protect
//
// DESCRIPTION:
//   
//
// INPUTS:
//    none
//
// OUTPUTS:
//    none
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************
.MODULE $vee.accelerate.neck_protect;
	.CODESEGMENT PM;
	.DATASEGMENT DM;

	.CONST ANGLE_VALUE_THRESHOLD	40;
	.CONST ANGLE_INIT_SIZE			10;
	.CONST NECK_TIME_PERIOD			1000000;	// 1000 ms
	.CONST NECK_TIME_THRESHOLD		30;			// 0.5min
	
   	.VAR neck_timer_struc[$timer.STRUC_SIZE];		

	.VAR acc_count = 0;
	.VAR angle_count = 0;
	.VAR angle_x_init = 0;
	.VAR angle_y_init = 0;
	.VAR angle_z_init = 0;
	.VAR angle_x_init_buf[ANGLE_INIT_SIZE];
	.VAR angle_y_init_buf[ANGLE_INIT_SIZE];
	.VAR angle_z_init_buf[ANGLE_INIT_SIZE];	
	.VAR acc_x_value_buf[ANGLE_INIT_SIZE];
	.VAR acc_y_value_buf[ANGLE_INIT_SIZE];
	.VAR acc_z_value_buf[ANGLE_INIT_SIZE];
	
	.VAR neck_timer_enable = 0;
	.VAR neck_timer_count = 0;
	
$vee.accelerate.neck_protect:
	$push_rLink_macro;
	
	r1 = M[acc_count];
	
	NULL = r1 - ANGLE_INIT_SIZE;
	if NEG jump store_acc_value;
	
	NULL = r1 - ANGLE_INIT_SIZE;
	if Z jump calc_angle_value;
	
	jump $pop_rLink_and_rts;
	
calc_angle_value:

	r0 = &$vee.accelerate.neck_protect.acc_x_value_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.acc_average;	
	r5 = r0;
	call $vee.accelerate.arcsin;	// 计算角度
	M[$vee.accelerate.angle_x_int] = r0;	
	
	r0 = &$vee.accelerate.neck_protect.acc_y_value_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.acc_average;	
	r5 = r0;
	call $vee.accelerate.arcsin;	// 计算角度
	M[$vee.accelerate.angle_y_int] = r0;

	r0 = &$vee.accelerate.neck_protect.acc_z_value_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.acc_average;
	r5 = r0;
	call $vee.accelerate.arccos;	// 计算角度
	M[$vee.accelerate.angle_z_int] = r0;	
	
#ifdef ACC_VALUE_DATA
	call $vee.accelerate.acc_output;
#endif	
	
	r1 = 0;
	M[acc_count] = r1;	
	
	r1 = M[angle_count];
	
	NULL = r1 - ANGLE_INIT_SIZE;
	if NEG jump store_angle_value;
	
	NULL = r1 - ANGLE_INIT_SIZE;
	if Z jump calc_init_angle;
	
	// 角度与阈值比较
	r1 = M[$vee.accelerate.angle_x_int];
	r2 = M[angle_x_init];
	NULL = r1 - r2;
	if POS r3 = r1 - r2;
	
	NULL = r1 - r2;
	if NEG r3 = r2 - r1;
	
	NULL = r3 - ANGLE_VALUE_THRESHOLD;
	if POS jump start_timer_count;
	
	r1 = M[$vee.accelerate.angle_y_int];
	r2 = M[angle_y_init];
	NULL = r1 - r2;
	if POS r3 = r1 - r2;
	
	NULL = r1 - r2;
	if NEG r3 = r2 - r1;
	
	NULL = r3 - ANGLE_VALUE_THRESHOLD;
	if POS jump start_timer_count;

	r1 = M[$vee.accelerate.angle_z_int];
	r2 = M[angle_z_init];
	NULL = r1 - r2;
	if POS r3 = r1 - r2;
	
	NULL = r1 - r2;
	if NEG r3 = r2 - r1;
	
	NULL = r3 - ANGLE_VALUE_THRESHOLD;
	if POS jump start_timer_count;	
	
	// 关闭定时器
	r0 = 0;
	M[neck_timer_enable] = r0;
	
	jump $pop_rLink_and_rts;
	
start_timer_count:	

	r0 = M[neck_timer_enable];
	NULL = r0;
	if NZ jump $pop_rLink_and_rts;

 	// start timer that count the neck times
	r1 = &$vee.accelerate.neck_protect.neck_timer_struc;
	r2 = NECK_TIME_PERIOD;
	r3 = &$vee.accelerate.neck_protect.neck_timer_handler;
	call $timer.schedule_event_in;
	
	r0 = 1;
	M[neck_timer_enable] = r0;

	jump $pop_rLink_and_rts;
	
calc_init_angle:

	r1 = r1 + 1;	
	M[angle_count] = r1;	
	// 求正常坐姿的各轴角度
	r0 = &$vee.accelerate.neck_protect.angle_x_init_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.average_method;
	M[angle_x_init] = r0;
	
	r0 = &$vee.accelerate.neck_protect.angle_y_init_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.average_method;
	M[angle_y_init] = r0;

	r0 = &$vee.accelerate.neck_protect.angle_z_init_buf;
	r1 = ANGLE_INIT_SIZE;
	call $vee.accelerate.necke_protect.average_method;
	M[angle_z_init] = r0;
		
	jump $pop_rLink_and_rts;
	
store_acc_value: 

	// 存储各轴加速度
	r0 = M[$vee.accelerate.acc_axis_x];
	M[acc_x_value_buf + r1] = r0;
	
	r0 = M[$vee.accelerate.acc_axis_y];
	M[acc_y_value_buf + r1] = r0;
	
	r0 = M[$vee.accelerate.acc_axis_z];
	M[acc_z_value_buf + r1] = r0;
	
	r1 = r1 + 1;
	M[acc_count] = r1;	

	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
store_angle_value:

	// 存储各轴的角度
	r0 = M[$vee.accelerate.angle_x_int];
	M[angle_x_init_buf + r1] = r0;
	r0 = M[$vee.accelerate.angle_y_int];
	M[angle_y_init_buf + r1] = r0;
	r0 = M[$vee.accelerate.angle_z_int];
	M[angle_z_init_buf + r1] = r0;
	
	r1 = r1 + 1;
	M[angle_count] = r1;	
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
//// *******************************************************************************************
// MODULE:
//    vee.accelerate.necke_protect.average_method
//
// DESCRIPTION:
//   
//
// INPUTS:
//    r0 : angle peload
//	  r1 : size of the peload
//
// OUTPUTS:
//    r0 : the average of the angle
//
// TRASHED REGISTERS:
//   
//    Called buffer routines also trash:
//    r1, r2, r3, r4, I0, L0, I1, L1, r10, DO LOOP
//
// *******************************************************************************************	
$vee.accelerate.necke_protect.average_method:
	$push_rLink_macro;
	
	I0 = r0;
	L0 = 0;
	r10 = r1;
	r3 = 0;			// 统计和
	
	// 采样求平均法
	do angle_init_loop;
		r2 = M[I0, 1];
		r3 = r3 + r2;
	angle_init_loop:
	
	rMAC = r3 ASHIFT 0 (LO); 
	Div = rMAC / r1;
	r0 = DivResult;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
$vee.accelerate.necke_protect.acc_average:
	$push_rLink_macro;	
	
	I0 = r0;
	L0 = 0;
	r10 = r1;
	r3 = 0;			// 统计和
	r4 = 99;
	r5 = -99;
	
	// 采样求平均法
	do acc_average_loop;
		r2 = M[I0, 1];
		r3 = r3 + r2;
	acc_average_loop:
	
	r1 = r3;
	NULL = r1 - 100;
	if POS r3 = r4;
	
	NULL = r1 + 100;
	if NEG r3 = r5;
	
	rMAC = r3 * 0.01 ;
	r0 = rMAC0;		// fraction of the angle
	r0 = r0 LSHIFT -1;	// 去除符号位	
		
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	
$vee.accelerate.neck_protect.neck_timer_handler:
	$push_rLink_macro;	
	
	// 时间累加
	r0 = M[neck_timer_count];
	r0 = r0 + 1;
	M[neck_timer_count] = r0;
	
	// 时间阈值比较
	NULL = r0 - NECK_TIME_THRESHOLD;
	if POS jump send_neck_prompt_event;
	
	r0 = M[neck_timer_enable];
	NULL = r0;
	if Z jump $pop_rLink_and_rts;

	// post another timer event
	r1 = &$vee.accelerate.neck_protect.neck_timer_struc;
	r2 = NECK_TIME_PERIOD;
	r3 = &$vee.accelerate.neck_protect.neck_timer_handler;
	call $timer.schedule_event_in_period;
	
	// Pop rLink from stack
	jump $pop_rLink_and_rts;	
	
send_neck_prompt_event:
	
	r0 = 0;
	M[neck_timer_count] = r0;
	M[neck_timer_enable] = r0;
	M[acc_count] = r0;
	M[angle_count] = r0;		// 重新计算初始角度
	
	r2 = NECT_PROMPT_EVENT_TO_VM_ID;
	call $message.send_short;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;
	
.ENDMODULE;

//// *******************************************************************************************
// MODULE:
//    vee.accelerate.step_count
//
// DESCRIPTION:
//	count the user step
//	
// INPUTS:
//    none
//    
// OUTPUTS:
//    none
//    
// TRASHED REGISTERS:
//     
//     
//    Called buffer routines also trash:
//    r0, r1, r2, r3, r4, r5
//
// *******************************************************************************************
.MODULE $vee.accelerate.step_count;
	.CODESEGMENT PM;
	.DATASEGMENT DM;

	.CONST MAX_VALUE_THRESHOLD	90;
	.CONST MIN_VALUE_THRESHOLD	70;
	.CONST TIME_THRESHOLD		3;
	.CONST VALUE_THRESHOLD		40;

	.VAR p_value = 0;		// 前一个加速度矢量和
	.VAR index_curr = 0;
	.VAR status = 0;		// 0:表示上升沿； 1:表示下降沿	
	.VAR index_max  = 0;
	.VAR max_flag = 1;
	.VAR min_flag = 1;
	.VAR min_value = 0;

$vee.accelerate.step_count:
	$push_rLink_macro;

	/* 计算x轴和y轴加速度矢量和 */
	r3 = 0;
	r1 = M[$vee.accelerate.acc_axis_x];
	r1 = r1*r1 (int);
	r3 = r3 + r1;
	r1 = M[$vee.accelerate.acc_axis_y];
	r1 = r1*r1 (int);
	r3 = r3 + r1;		// r3:当前加速度矢量和

	r1 = M[p_value];	// r1 :前一个加速度矢量和
	r2 = M[index_curr];	// r2: 当前下标
	r0 = 0;				// r0 : temp
	r5 = M[index_max];	// r5: 前一个极大值下标

	NULL = r3 - r1;
	if POS jump check_max_value;
	
check_min_value:
	r0 = M[status];
	NULL = r0;
	if NZ jump continue_check;
	
	r0 = 1;
	M[status] = r0;		// 设置为下降沿
	
	NULL = r1 - MAX_VALUE_THRESHOLD;
	if NEG jump continue_check;
		
	r0 = r2 - r5;
	NULL = r0 - TIME_THRESHOLD;
	if NEG jump continue_check;
	
	r0 = M[min_flag];
	NULL = r0;
	if Z jump continue_check;
	
	r0 = M[min_value];
	r0 = r1 - r0;
	NULL = r0 - VALUE_THRESHOLD;
	if NEG jump continue_check;

	call $interrupt.block;
	r0 = M[$vee.accelerate.step_value];
	r0 = r0 + 1;
	M[$vee.accelerate.step_value] = r0;		// 计步加1
    call $interrupt.unblock;

	r0 = 0;
	M[min_flag] = r0;
	r0 = 1;
	M[max_flag] = r0;
	M[index_max] = r2;			// 存取极大值下标 
	
	jump continue_check;
	

check_max_value:
	r0 = M[status];
	NULL = r0;
	if Z jump continue_check;

	r0 = 0;
	M[status] = r0;		// 设置为上升沿
	
	NULL = r1 - MIN_VALUE_THRESHOLD;
	if POS jump continue_check;
	
	r0 = M[max_flag];
	NULL = r0;
	if Z jump continue_check;
	
	r0 = 0;
	M[max_flag] = r0;
	r0 = 1;
	M[min_flag] = r0;
	M[min_value] = r1;
	
continue_check:

	M[p_value] = r3;
	r3 = M[index_curr];
	r3 = r3 + 1;
	M[index_curr] = r3;

	// Pop rLink from stack
	jump $pop_rLink_and_rts;

.ENDMODULE;




