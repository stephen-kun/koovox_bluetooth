// *****************************************************************************
// %%fullcopyright(2007)        http://www.csr.com
// %%version
//
// $Change: 1849175 $  $DateTime: 2014/03/12 16:45:08 $
// *****************************************************************************

// *****************************************************************************
// DESCRIPTION
//    This application uses the frame_sync framework (frame_sync library) to
//    copy samples across from the SCO in buffer to the DAC buffer, and from the
//    ADC buffer to the SCO out buffer.  CVC (cvc_headset library) is called to
//    process and transfer the data between buffers
//
// NOTES
//    CVC is comprised of the following processing modules:
//     - AEC (Acoustic Echo Canceller) applied to SCO output
//     - AGC (Automatic Gain Control) applied to DAC output
//     - AGC (Automatic Gain Control) applied to SCO output
//     - NS (Noise Suppression) applied to SCO output
//     - NDVC (Noise Dependent Volume Control) applied to DAC output
//     - SCO in parametric equalizer
//     - SCO out parametric equalizer
//     - SCO in and ADC in DC-Block filters
//
//    These modules can be included/excluded from the build using define the
//    define statements in cvc_headset_config.h
//
// *****************************************************************************

#include "flash.h"
#include "core_library.h"
#include "cbops_library.h"
#include "kalimba_messages.h"
#include "i2c_library.h"




// --------------------------------------------------------------------------
// System defines
// --------------------------------------------------------------------------

   .CONST $MESSAGE.AUDIO_CONFIG                 0x2000;

   // 1ms is chosen as the interrupt rate for the audio input/output to minimise latency:
   .CONST $TIMER_PERIOD_AUDIO_COPY_MICROS       625;

   // send the persistence block to the VM @ a 3s interval
   .CONST $TIMER_PERIOD_PBLOCK_SEND_MICROS      3000*1000; //3E6

   .CONST  $NUM_SAMPLES_2MS_16K   (32);


#ifdef BUILD_MULTI_KAPS
// *****************************************************************************
// MODULE:
//    $flash.init_dmconst
//
// DESCRIPTION:
//    On a BC7 initialises flash data to be mapped into windows 2 and 3.
//    24-bit data (corresponding to DMCONST) is mapped into window 3,
// while 16-bit data is mapped into window 2.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.flash.init_dmconst;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   $flash.init_dmconst:

   // these variables are filled out with the address in flash of the
   // flash segment by the firmware upon a VM KalimbaLoad() call
   .VAR $flash.data16.address;
   .VAR $flash.data24.address;

   // set up the start address and 24-bit data mode for window 2
   r0 = M[$flash.data24.address];
   M[$FLASH_WINDOW2_START_ADDR] = r0;
   r0 = $FLASHWIN_CONFIG_24BIT_MASK;
   M[$FLASHWIN2_CONFIG] = r0;

   // set up the start address and 16-bit data mode for window 1
   r0 = M[$flash.data16.address];
   M[$FLASH_WINDOW1_START_ADDR] = r0;
   M[$FLASHWIN1_CONFIG] = Null;
   rts;

.ENDMODULE;
#endif


// CVC Security ID
.CONST  $CVC_ONEMIC_HEADSET_SECID     0x258C;


// *****************************************************************************
// MODULE:
//    $M.Main
//
// DESCRIPTION:
//    This is the main routine.  It initializes the libraries and then loops
//    "forever" in the processing loop.
//
// *****************************************************************************
#ifndef BUILD_MULTI_KAPS
.MODULE $M.main;
   .CODESEGMENT MAIN_PM;

$main:
   // initialise the stack library
   call $stack.initialise;
   // initialise DM_flash
   call $flash.init_dmconst;
   // initialise the interrupt library
   call $interrupt.initialise;
   // initialise the message library
   call $message.initialise;
   // initialise the pskey library
   call $pskey.initialise;
   // initialise the cbuffer library
   call $cbuffer.initialise;
   // initialise licensing subsystem
   call $Security.Initialize;
   // intialize SPI communications library
   call $spi_comm.initialize;
      

    call $vee.heartrate.start;
    call $vee.accelerate.start;


   // send message saying we re up and running!
   r2 = 0x1000;
   call $message.send_short;
   
#else
.MODULE $M.cvc_main;
   .CODESEGMENT PM_ENTRYPT;

    //.VAR/DMCONST16 cvc_modules_path[]= string("cvc_share_hf/cvc_share_hf.kap");    // /pack16
$app_main:

   // initialise DM_flash
   call $flash.init_dmconst;

#endif

   // tell vm we re ready and wait for the go message
   call $message.send_ready_wait_for_go;

main_loop:

#if 1

    	call $vee.accelerate.angle_calc;

    	call $vee.heartrate.calc;

	jump main_loop;

#else

	r0 = 100;
	call $timer.n_ms_delay;

#endif


   	jump main_loop;

.ENDMODULE;


