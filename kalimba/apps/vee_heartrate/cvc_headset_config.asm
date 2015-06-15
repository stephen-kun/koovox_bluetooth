// *****************************************************************************
// %%fullcopyright(2007)        http://www.csr.com
// %%version
//
// $Change: 1865344 $  $DateTime: 2014/04/01 14:36:59 $
// *****************************************************************************


// *****************************************************************************
// DESCRIPTION
//    CVC static configuration file that includes tables of function pointers
//    and their corresponding data objects.adapt_eq
//
//    Customer modification to this file is NOT SUPPORTED.
//
//    CVC configuration should be handled from within the cvc_headset_config.h
//    header file.
//
// *****************************************************************************

#include "stack.h"
#include "frame_codec.h"
#include "cvc_modules.h"
#include "cvc_headset.h"
#include "cvc_system_library.h"
#include "cbops_library.h"
#include "frame_sync_buffer.h"
#include "cbuffer.h"
#include "frame_sync_tsksched.h"
#include "frame_sync_stream_macros.h"

// SP. Needs for Sample Rate converters
#include "operators\iir_resamplev2\iir_resamplev2_header.h"

#ifndef BUILD_MULTI_KAPS   // SP. moved to dsp_core
// declare twiddle factors
#include "fft_twiddle.h"
#endif
// temporary until application linkfile can configure this segment
#define USE_FLASH_ADDR_TABLES
#ifdef USE_FLASH_ADDR_TABLES
   #if defined(KAL_ARCH3) // arch3: cannot use 16-bit addresses
      #define ADDR_TABLE_DM DMCONST
   #elif defined (KAL_ARCH2) // arch2: can use 16-bit addresses
      #define ADDR_TABLE_DM DMCONST16
   #else
      #define ADDR_TABLE_DM DM
   #endif
#else
   #define ADDR_TABLE_DM DM
#endif


// Generate build error messages if necessary.
#if uses_SND_NS == 0
#if uses_AEC
   #error AEC cannot be enabled without OMS
#endif
#if uses_NSVOLUME
   #error NDVC cannot be enabled without OMS
#endif
#endif

#define MAX_NUM_PEQ_STAGES             (5)

// System Configuration is saved in kap file.
.MODULE $M.CVC_MODULES_STAMP;
   .DATASEGMENT DM;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $M.CVC_VERSION_STAMP
//
// DESCRIPTION:
//    This data module is used to write the CVC algorithm ID and build version
//    into the kap file in a readable form.
// *****************************************************************************

.MODULE $M.CVC_VERSION_STAMP;
   .DATASEGMENT DM;

.ENDMODULE;


.MODULE $M.CVC.data;
   .DATASEGMENT DM;

 



.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $M.purge_cbuffer
//
// DESCRIPTION:
//    Purge cbuffers by writing zeros into the cbuffer.
//    Without this function the DAC port would continuously be fed stale data
//    from DSP cbuffers when switching from HFK to SSR mode.
//
// INPUTS:
//    - r7 = Pointer to cbuffer struc
//
// OUTPUTS:
//    - none
//
// *****************************************************************************
#if uses_SSR
.MODULE $M.purge_cbuffer;
   .CODESEGMENT CVC_SSR_UTILITY_PM;
   .DATASEGMENT DM;

$purge_cbuffer:
   $push_rLink_macro;


    jump $pop_rLink_and_rts;

.ENDMODULE;
#endif

// Always called for a MODE change
.MODULE $M.CVC_HEADSET.SystemReInitialize;


   .CODESEGMENT CVC_SYSTEM_REINITIALIZE_PM;
   .DATASEGMENT DM;

 func:



   // Call Module Initialize Functions
   $push_rLink_macro;

   // Configure PLC and Codecs
 


   jump $pop_rLink_and_rts;
.ENDMODULE;


.MODULE  $M.CVC.Zero_DC_Nyquist;

   .codesegment CVC_ZERO_DC_NYQUIST_PM;

func:

   rts;
.ENDMODULE;

// *****************************************************************************
//
// MODULE:
//    $M.set_mode_gains.func
//
// DESCRIPTION:
//    Sets input gains (ADC and SCO) based on the current mode.
//    (Note: this function should only be called from within standy,
//    loopback, and pass-through modes).
//
//    MODE              ADC GAIN        SCO GAIN
//    pass-through      user specified  user specified
//    standby           zero            zero
//    loopback          unity           unity
//
//
// INPUTS:
//    r7 - Pointer to the data structure
//
// *****************************************************************************

.MODULE $M.set_mode_gains;
    .CODESEGMENT SET_MODE_GAIN_PM;

$cvc_Set_LoopBackGains:


   jump setgains;

$cvc_Set_PassThroughGains:

 

   rts;
.ENDMODULE;




// *****************************************************************************
// DESCRIPTION: Response to persistence load request
// r2 = length of persistence block / zero for error[?]
// r3 = address of persistence block
// *****************************************************************************
.MODULE $M.1mic.LoadPersistResp;

   .CODESEGMENT PM;
   .DATASEGMENT DM;


func:




   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $M.pblock_send_handler
//
// DESCRIPTION:
//    This module periodically sends the persistence block to HSV5 for storage
//
//
// *****************************************************************************
.MODULE $M.pblock_send_handler;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

 

$pblock_send_handler:

   $push_rLink_macro;

 

   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $M.vad_hold.process
//
// DESCRIPTION:
//    Delay VAD transition to zero after echo event. Logic:
//    if echo_flag
//        echo_hold_counter = echo_hold_time_frames
//    else
//        echo_hold_counter = MAX(--echo_hold_counter, 0)
//    end
//    VAD = VAD && (echo_hold_counter > 0)
//
// INPUTS:
//    r7 - Pointer to the data structure
//
// *****************************************************************************
.MODULE $M.vad_hold.process;
   .CODESEGMENT PM;

func:

   rts;

.ENDMODULE;
