###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: spdif_sink
# Configuration: Release
# Generated: ���� 8�� 6 11:53:32 2015
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=spdif_sink
OUTDIR=C:/ADK3.5/apps/sink
DEFS=-DKALASM3 -DAUDIO_CBUFFER_SIZE=1500 -DCODEC_CBUFFER_SIZE=4096 -DSPDIF_CBUFFER_SIZE=3072 -DSPDIF_ENABLE -DAC3_ENABLEx -DAAC_ENABLEx -DMP3_ENABLEx -DSUB_ENABLE -DANC_96Kx 
BOOTSTRAP=1
LIBS=core cbops spdif_sink codec frame_sync audio_proc spi_comm math cvc_modules 
ASMS=\
      spdif_sink.asm\
      music_example_system.asm\
      mips_profile.asm\
      spi_message.asm\
      vm_message.asm\
      music_example_resample.asm\
      usb_config.asm\
      music_example_config.asm\
      subwoofer.asm\
      user_eq.asm
DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0

# Project-specific options
LIB_SET=sdk
debugtransport=[SPI|LPT1|No]
goodworkingbufferlevel=0.65

-include spdif_sink.mak
include $(BLUELAB)/Makefile.dsp
