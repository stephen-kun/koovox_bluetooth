###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: cvc_headset_2mic
# Configuration: Release
# Generated: ��һ 6�� 15 19:36:33 2015
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=cvc_headset_2mic
OUTDIR=C:/ADK3.5/apps/sink
DEFS=-DKALASM3 -DFRAME_SYNC_DEBUGx -DBASE_REGISTER_MODEx -Duses_96kx -Duses_SSRx 
BOOTSTRAP=1
LIBS=security core frame_sync cbops_multirate spi_comm math audio_proc plc100 cvc_modules cvc_system sbc_wbs ssr_system 
ASMS=\
      cvc_headset_2mic_main.asm\
      cvc_headset_2mic_config.asm\
      cvc_FrontEnd.asm\
      cvc_BackEnd.asm
DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0

# Project-specific options
LIB_SET=sdk
debugtransport=[SPITRANS=LPT SPIPORT=1]
mksymbols=0
separator_flags=0

-include cvc_headset_2mic.mak
include $(BLUELAB)/Makefile.dsp
