###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: vee_heartrate
# Configuration: Release
# Generated: ��һ 6�� 15 19:36:37 2015
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=vee_heartrate
OUTDIR=C:/ADK3.5/apps/sink
DEFS=-DKALASM3 -Duses_SSR -DFRAME_SYNC_DEBUGx -DBASE_REGISTER_MODEx -Duses_ANCx 
BOOTSTRAP=1
LIBS=security core_big_stack cbops frame_sync cvc_modules spi_comm math plc100 audio_proc cvc_system ssr_system sbc_wbs i2c 
ASMS=\
      cvc_headset_main.asm\
      vee_heartrate.asm\
      cvc_headset_config.asm
DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0

# Project-specific options
LIB_SET=sdk
debugtransport=[SPITRANS=USB SPIPORT=0]
separator_flags=0

-include vee_heartrate.mak
include $(BLUELAB)/Makefile.dsp
