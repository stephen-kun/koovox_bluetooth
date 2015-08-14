###########################################################
# Makefile generated by xIDE                               
#                                                          
# Project: faststream_decoder
# Configuration: Release
# Generated: ���� 8�� 14 13:21:14 2015
#                                                          
# WARNING: Do not edit this file. Any changes will be lost 
#          when the project is rebuilt.                    
#                                                          
###########################################################

OUTPUT=faststream_decoder
OUTDIR=C:/ADK3.5/apps/sink
DEFS=-DKALASM3 -DAUDIO_CBUFFER_SIZE=720 -DCODEC_CBUFFER_SIZE=1024 -DGOOD_WORKING_BUFFER_LEVEL=0.15 -DFASTSTREAM_ENABLE -DHARDWARE_RATE_MATCH -DLATENCY_REPORTINGx 
BOOTSTRAP=1
LIBS=core cbops sbc codec faststream frame_sync audio_proc spi_comm math 
ASMS=\
      sr_adjustment_gaming.asm\
      mips_profile.asm\
      spi_message.asm\
      vm_message.asm\
      codec_decoder_faststream.asm\
      music_example_config_gaming.asm\
      music_example_system.asm\
      music_example_resample_gaming.asm\
      user_eq.asm\
      latency_reporting.asm
DEBUGTRANSPORT=SPITRANS=USB SPIPORT=0

# Project-specific options
debugtransport=[SPITRANS=LPT SPIPORT=1]

-include faststream_decoder.mak
include $(BLUELAB)/Makefile.dsp
