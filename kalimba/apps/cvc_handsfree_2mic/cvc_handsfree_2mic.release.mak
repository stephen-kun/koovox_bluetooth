###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: cvc_handsfree_2mic
# Configuration: Release
# Generated: ���� 6�� 10 19:56:38 2015
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=cvc_handsfree_2mic
OUTDIR=C:/ADK3.5/apps/sink
DEFS=-DKALASM3 -DFRAME_SYNC_DEBUGx -DBASE_REGISTER_MODEx -Duses_96kx -Duses_SSRx -DBLD_WBx 
BOOTSTRAP=1
LIBS=security core frame_sync cbops_multirate spi_comm math audio_proc plc100 cvc_modules cvc_system sbc_wbs ssr_system 
ASMS=\
      cvc_handsfree_2mic_main.asm\
      cvc_handsfree_2mic_config.asm\
      cvc_FrontEnd.asm\
      cvc_BackEnd.asm
DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0

# Project-specific options
LIB_SET=sdk
debugtransport=[SPITRANS=USB SPIPORT=0]
mksymbols=0
separator_flags=0

-include cvc_handsfree_2mic.mak
include $(BLUELAB)/Makefile.dsp
