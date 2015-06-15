.linefile 1 "vee_heartrate.asm"
.linefile 1 "<command-line>"
.linefile 1 "vee_heartrate.asm"
.linefile 88 "vee_heartrate.asm"
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 1
.linefile 9 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h"
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/stack.h" 1
.linefile 10 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/timer.h" 1
.linefile 14 "C:/ADK3.5/kalimba/lib_sets/sdk/include/timer.h"
   .CONST $timer.MAX_TIMER_HANDLERS 50;

   .CONST $timer.LAST_ENTRY -1;

   .CONST $timer.NEXT_ADDR_FIELD 0;
   .CONST $timer.TIME_FIELD 1;
   .CONST $timer.HANDLER_ADDR_FIELD 2;
   .CONST $timer.ID_FIELD 3;
   .CONST $timer.STRUC_SIZE 4;

   .CONST $timer.n_us_delay.SHORT_DELAY 10;
   .CONST $timer.n_us_delay.MEDIUM_DELAY 150;
.linefile 11 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/message.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/message.h"
.linefile 1 "C:/ADK3.5/kalimba/architecture/architecture.h" 1
.linefile 20 "C:/ADK3.5/kalimba/architecture/architecture.h"
.linefile 1 "C:/ADK3.5/kalimba/architecture/gordon.h" 1
.linefile 9 "C:/ADK3.5/kalimba/architecture/gordon.h"
   .CONST $FLASHWIN1_START 0xFFB000;
   .CONST $FLASHWIN1_SIZE 0x001000;
   .CONST $FLASHWIN2_START 0xFFC000;
   .CONST $FLASHWIN2_SIZE 0x001000;
   .CONST $FLASHWIN3_START 0xFFD000;
   .CONST $FLASHWIN3_SIZE 0x001000;
   .CONST $MCUWIN1_START 0xFFE000;
   .CONST $MCUWIN1_SIZE 0x001000;
   .CONST $MCUWIN2_START 0xFFF000;
   .CONST $MCUWIN2_SIZE 0x000E00;

   .CONST $PMWIN_HI_START 0x020000;
   .CONST $PMWIN_LO_START 0x030000;
   .CONST $PMWIN_24_START 0x040000;
   .CONST $PMWIN_SIZE 0x003000;

   .CONST $FLASHWIN1_LARGE_START 0xD00000;
   .CONST $FLASHWIN1_LARGE_SIZE 0x100000;
   .CONST $FLASHWIN2_LARGE_START 0xE00000;
   .CONST $FLASHWIN2_LARGE_SIZE 0x100000;
   .CONST $FLASHWIN3_LARGE_START 0xF00000;
   .CONST $FLASHWIN3_LARGE_SIZE 0x0D0000;





   .CONST $INT_LOAD_INFO_CLR_REQ_MASK 16384;

   .CONST $INT_SOURCE_TIMER1_POSN 0;
   .CONST $INT_SOURCE_TIMER2_POSN 1;
   .CONST $INT_SOURCE_MCU_POSN 2;
   .CONST $INT_SOURCE_PIO_POSN 3;
   .CONST $INT_SOURCE_MMU_UNMAPPED_POSN 4;
   .CONST $INT_SOURCE_SW0_POSN 5;
   .CONST $INT_SOURCE_SW1_POSN 6;
   .CONST $INT_SOURCE_SW2_POSN 7;
   .CONST $INT_SOURCE_SW3_POSN 8;

   .CONST $INT_SOURCE_TIMER1_MASK 1;
   .CONST $INT_SOURCE_TIMER2_MASK 2;
   .CONST $INT_SOURCE_MCU_MASK 4;
   .CONST $INT_SOURCE_PIO_MASK 8;
   .CONST $INT_SOURCE_MMU_UNMAPPED_MASK 16;
   .CONST $INT_SOURCE_SW0_MASK 32;
   .CONST $INT_SOURCE_SW1_MASK 64;
   .CONST $INT_SOURCE_SW2_MASK 128;
   .CONST $INT_SOURCE_SW3_MASK 256;

   .CONST $INT_SOURCE_TIMER1_EVENT 0;
   .CONST $INT_SOURCE_TIMER2_EVENT 1;
   .CONST $INT_SOURCE_MCU_EVENT 2;
   .CONST $INT_SOURCE_PIO_EVENT 3;
   .CONST $INT_SOURCE_MMU_UNMAPPED_EVENT 4;
   .CONST $INT_SOURCE_SW0_EVENT 5;
   .CONST $INT_SOURCE_SW1_EVENT 6;
   .CONST $INT_SOURCE_SW2_EVENT 7;
   .CONST $INT_SOURCE_SW3_EVENT 8;





   .CONST $CLK_DIV_1 0;
   .CONST $CLK_DIV_2 1;
   .CONST $CLK_DIV_4 3;
   .CONST $CLK_DIV_8 7;
   .CONST $CLK_DIV_16 15;
   .CONST $CLK_DIV_32 31;
   .CONST $CLK_DIV_64 63;
   .CONST $CLK_DIV_128 127;
   .CONST $CLK_DIV_256 255;
   .CONST $CLK_DIV_512 511;
   .CONST $CLK_DIV_1024 1023;
   .CONST $CLK_DIV_2048 2047;
   .CONST $CLK_DIV_4096 4095;
   .CONST $CLK_DIV_8192 8191;
   .CONST $CLK_DIV_16384 16383;


   .CONST $CLK_DIV_MAX $CLK_DIV_64;



   .CONST $N_FLAG 1;
   .CONST $Z_FLAG 2;
   .CONST $C_FLAG 4;
   .CONST $V_FLAG 8;
   .CONST $UD_FLAG 16;
   .CONST $SV_FLAG 32;
   .CONST $BR_FLAG 64;
   .CONST $UM_FLAG 128;

   .CONST $NOT_N_FLAG (65535-$N_FLAG);
   .CONST $NOT_Z_FLAG (65535-$Z_FLAG);
   .CONST $NOT_C_FLAG (65535-$C_FLAG);
   .CONST $NOT_V_FLAG (65535-$V_FLAG);
   .CONST $NOT_UD_FLAG (65535-$UD_FLAG);
   .CONST $NOT_SV_FLAG (65535-$SV_FLAG);
   .CONST $NOT_BR_FLAG (65535-$BR_FLAG);
   .CONST $NOT_UM_FLAG (65535-$UM_FLAG);
.linefile 21 "C:/ADK3.5/kalimba/architecture/architecture.h" 2
.linefile 1 "C:/ADK3.5/kalimba/architecture/gordon_io_defs.h" 1




   .CONST $FLASH_CACHE_SIZE_1K_ENUM 0x000000;
   .CONST $FLASH_CACHE_SIZE_512_ENUM 0x000001;
   .CONST $ADDSUB_SATURATE_ON_OVERFLOW_POSN 0x000000;
   .CONST $ADDSUB_SATURATE_ON_OVERFLOW_MASK 0x000001;
   .CONST $ARITHMETIC_16BIT_MODE_POSN 0x000001;
   .CONST $ARITHMETIC_16BIT_MODE_MASK 0x000002;
   .CONST $DISABLE_UNBIASED_ROUNDING_POSN 0x000002;
   .CONST $DISABLE_UNBIASED_ROUNDING_MASK 0x000004;
   .CONST $DISABLE_FRAC_MULT_ROUNDING_POSN 0x000003;
   .CONST $DISABLE_FRAC_MULT_ROUNDING_MASK 0x000008;
   .CONST $DISABLE_RMAC_STORE_ROUNDING_POSN 0x000004;
   .CONST $DISABLE_RMAC_STORE_ROUNDING_MASK 0x000010;
   .CONST $FLASHWIN_CONFIG_NOSIGNX_POSN 0x000000;
   .CONST $FLASHWIN_CONFIG_NOSIGNX_MASK 0x000001;
   .CONST $FLASHWIN_CONFIG_24BIT_POSN 0x000001;
   .CONST $FLASHWIN_CONFIG_24BIT_MASK 0x000002;
   .CONST $INT_EVENT_TIMER1_POSN 0x000000;
   .CONST $INT_EVENT_TIMER1_MASK 0x000001;
   .CONST $INT_EVENT_TIMER2_POSN 0x000001;
   .CONST $INT_EVENT_TIMER2_MASK 0x000002;
   .CONST $INT_EVENT_XAP_POSN 0x000002;
   .CONST $INT_EVENT_XAP_MASK 0x000004;
   .CONST $INT_EVENT_PIO_POSN 0x000003;
   .CONST $INT_EVENT_PIO_MASK 0x000008;
   .CONST $INT_EVENT_MMU_UNMAPPED_POSN 0x000004;
   .CONST $INT_EVENT_MMU_UNMAPPED_MASK 0x000010;
   .CONST $INT_EVENT_SW0_POSN 0x000005;
   .CONST $INT_EVENT_SW0_MASK 0x000020;
   .CONST $INT_EVENT_SW1_POSN 0x000006;
   .CONST $INT_EVENT_SW1_MASK 0x000040;
   .CONST $INT_EVENT_SW2_POSN 0x000007;
   .CONST $INT_EVENT_SW2_MASK 0x000080;
   .CONST $INT_EVENT_SW3_POSN 0x000008;
   .CONST $INT_EVENT_SW3_MASK 0x000100;
   .CONST $INT_EVENT_GPS_POSN 0x000009;
   .CONST $INT_EVENT_GPS_MASK 0x000200;
   .CONST $BITMODE_POSN 0x000000;
   .CONST $BITMODE_MASK 0x000003;
   .CONST $BITMODE_8BIT_ENUM 0x000000;
   .CONST $BITMODE_16BIT_ENUM 0x000001;
   .CONST $BITMODE_24BIT_ENUM 0x000002;
   .CONST $BYTESWAP_POSN 0x000002;
   .CONST $BYTESWAP_MASK 0x000004;
   .CONST $SATURATE_POSN 0x000003;
   .CONST $SATURATE_MASK 0x000008;
   .CONST $NOSIGNEXT_POSN 0x000003;
   .CONST $NOSIGNEXT_MASK 0x000008;
.linefile 22 "C:/ADK3.5/kalimba/architecture/architecture.h" 2
.linefile 1 "C:/ADK3.5/kalimba/architecture/gordon_io_map.h" 1




   .CONST $INT_SW_ERROR_EVENT_TRIGGER 0xFFFE00;
   .CONST $INT_GBL_ENABLE 0xFFFE11;
   .CONST $INT_ENABLE 0xFFFE12;
   .CONST $INT_CLK_SWITCH_EN 0xFFFE13;
   .CONST $INT_SOURCES_EN 0xFFFE14;
   .CONST $INT_PRIORITIES 0xFFFE15;
   .CONST $INT_LOAD_INFO 0xFFFE16;
   .CONST $INT_ACK 0xFFFE17;
   .CONST $INT_SOURCE 0xFFFE18;
   .CONST $INT_SAVE_INFO 0xFFFE19;
   .CONST $INT_ADDR 0xFFFE1A;
   .CONST $DSP2MCU_EVENT_DATA 0xFFFE1B;
   .CONST $PC_STATUS 0xFFFE1C;
   .CONST $MCU2DSP_EVENT_DATA 0xFFFE1D;
   .CONST $DOLOOP_CACHE_EN 0xFFFE1E;
   .CONST $TIMER1_EN 0xFFFE1F;
   .CONST $TIMER2_EN 0xFFFE20;
   .CONST $TIMER1_TRIGGER 0xFFFE21;
   .CONST $TIMER2_TRIGGER 0xFFFE22;
   .CONST $WRITE_PORT0_DATA 0xFFFE23;
   .CONST $WRITE_PORT1_DATA 0xFFFE24;
   .CONST $WRITE_PORT2_DATA 0xFFFE25;
   .CONST $WRITE_PORT3_DATA 0xFFFE26;
   .CONST $WRITE_PORT4_DATA 0xFFFE27;
   .CONST $WRITE_PORT5_DATA 0xFFFE28;
   .CONST $WRITE_PORT6_DATA 0xFFFE29;
   .CONST $WRITE_PORT7_DATA 0xFFFE2A;
   .CONST $READ_PORT0_DATA 0xFFFE2B;
   .CONST $READ_PORT1_DATA 0xFFFE2C;
   .CONST $READ_PORT2_DATA 0xFFFE2D;
   .CONST $READ_PORT3_DATA 0xFFFE2E;
   .CONST $READ_PORT4_DATA 0xFFFE2F;
   .CONST $READ_PORT5_DATA 0xFFFE30;
   .CONST $READ_PORT6_DATA 0xFFFE31;
   .CONST $READ_PORT7_DATA 0xFFFE32;
   .CONST $PORT_BUFFER_SET 0xFFFE33;
   .CONST $WRITE_PORT8_DATA 0xFFFE34;
   .CONST $WRITE_PORT9_DATA 0xFFFE35;
   .CONST $WRITE_PORT10_DATA 0xFFFE36;
   .CONST $READ_PORT8_DATA 0xFFFE38;
   .CONST $READ_PORT9_DATA 0xFFFE39;
   .CONST $READ_PORT10_DATA 0xFFFE3A;
   .CONST $MM_DOLOOP_START 0xFFFE40;
   .CONST $MM_DOLOOP_END 0xFFFE41;
   .CONST $MM_QUOTIENT 0xFFFE42;
   .CONST $MM_REM 0xFFFE43;
   .CONST $MM_RINTLINK 0xFFFE44;
   .CONST $CLOCK_DIVIDE_RATE 0xFFFE4D;
   .CONST $INT_CLOCK_DIVIDE_RATE 0xFFFE4E;
   .CONST $PIO_IN 0xFFFE4F;
   .CONST $PIO2_IN 0xFFFE50;
   .CONST $PIO_OUT 0xFFFE51;
   .CONST $PIO2_OUT 0xFFFE52;
   .CONST $PIO_DIR 0xFFFE53;
   .CONST $PIO2_DIR 0xFFFE54;
   .CONST $PIO_EVENT_EN_MASK 0xFFFE55;
   .CONST $PIO2_EVENT_EN_MASK 0xFFFE56;
   .CONST $INT_SW0_EVENT 0xFFFE57;
   .CONST $INT_SW1_EVENT 0xFFFE58;
   .CONST $INT_SW2_EVENT 0xFFFE59;
   .CONST $INT_SW3_EVENT 0xFFFE5A;
   .CONST $FLASH_WINDOW1_START_ADDR 0xFFFE5B;
   .CONST $FLASH_WINDOW2_START_ADDR 0xFFFE5C;
   .CONST $FLASH_WINDOW3_START_ADDR 0xFFFE5D;
   .CONST $NOSIGNX_MCUWIN1 0xFFFE5F;
   .CONST $NOSIGNX_MCUWIN2 0xFFFE60;
   .CONST $FLASHWIN1_CONFIG 0xFFFE61;
   .CONST $FLASHWIN2_CONFIG 0xFFFE62;
   .CONST $FLASHWIN3_CONFIG 0xFFFE63;
   .CONST $NOSIGNX_PMWIN 0xFFFE64;
   .CONST $PM_WIN_ENABLE 0xFFFE65;
   .CONST $STACK_START_ADDR 0xFFFE66;
   .CONST $STACK_END_ADDR 0xFFFE67;
   .CONST $STACK_POINTER 0xFFFE68;
   .CONST $STACK_OVERFLOW_PC 0xFFFE69;
   .CONST $FRAME_POINTER 0xFFFE6A;
   .CONST $NUM_RUN_CLKS_MS 0xFFFE6B;
   .CONST $NUM_RUN_CLKS_LS 0xFFFE6C;
   .CONST $NUM_INSTRS_MS 0xFFFE6D;
   .CONST $NUM_INSTRS_LS 0xFFFE6E;
   .CONST $NUM_STALLS_MS 0xFFFE6F;
   .CONST $NUM_STALLS_LS 0xFFFE70;
   .CONST $TIMER_TIME 0xFFFE71;
   .CONST $TIMER_TIME_MS 0xFFFE72;
   .CONST $WRITE_PORT0_CONFIG 0xFFFE73;
   .CONST $WRITE_PORT1_CONFIG 0xFFFE74;
   .CONST $WRITE_PORT2_CONFIG 0xFFFE75;
   .CONST $WRITE_PORT3_CONFIG 0xFFFE76;
   .CONST $WRITE_PORT4_CONFIG 0xFFFE77;
   .CONST $WRITE_PORT5_CONFIG 0xFFFE78;
   .CONST $WRITE_PORT6_CONFIG 0xFFFE79;
   .CONST $WRITE_PORT7_CONFIG 0xFFFE7A;
   .CONST $READ_PORT0_CONFIG 0xFFFE7B;
   .CONST $READ_PORT1_CONFIG 0xFFFE7C;
   .CONST $READ_PORT2_CONFIG 0xFFFE7D;
   .CONST $READ_PORT3_CONFIG 0xFFFE7E;
   .CONST $READ_PORT4_CONFIG 0xFFFE7F;
   .CONST $READ_PORT5_CONFIG 0xFFFE80;
   .CONST $READ_PORT6_CONFIG 0xFFFE81;
   .CONST $READ_PORT7_CONFIG 0xFFFE82;
   .CONST $PM_FLASHWIN_START_ADDR 0xFFFE83;
   .CONST $PM_FLASHWIN_SIZE 0xFFFE84;
   .CONST $BITREVERSE_VAL 0xFFFE89;
   .CONST $BITREVERSE_DATA 0xFFFE8A;
   .CONST $BITREVERSE_DATA16 0xFFFE8B;
   .CONST $BITREVERSE_ADDR 0xFFFE8C;
   .CONST $ARITHMETIC_MODE 0xFFFE93;
   .CONST $FORCE_FAST_MMU 0xFFFE94;
   .CONST $DBG_COUNTERS_EN 0xFFFE9F;
   .CONST $PM_FLASHWIN_CACHE_SIZE 0xFFFEE0;
   .CONST $WRITE_PORT8_CONFIG 0xFFFEE1;
   .CONST $WRITE_PORT9_CONFIG 0xFFFEE2;
   .CONST $WRITE_PORT10_CONFIG 0xFFFEE3;
   .CONST $READ_PORT8_CONFIG 0xFFFEE5;
   .CONST $READ_PORT9_CONFIG 0xFFFEE6;
   .CONST $READ_PORT10_CONFIG 0xFFFEE7;

   .CONST $READ_CONFIG_GAP $READ_PORT8_CONFIG - $READ_PORT7_CONFIG;
   .CONST $READ_DATA_GAP $READ_PORT8_DATA - $READ_PORT7_DATA;
   .CONST $WRITE_CONFIG_GAP $WRITE_PORT8_CONFIG - $WRITE_PORT7_CONFIG;
   .CONST $WRITE_DATA_GAP $WRITE_PORT8_DATA - $WRITE_PORT7_DATA;


   .CONST $INT_UNBLOCK $INT_ENABLE;
.linefile 23 "C:/ADK3.5/kalimba/architecture/architecture.h" 2
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/message.h" 2






   .CONST $message.MAX_LONG_MESSAGE_TX_PAYLOAD_SIZE 80;
   .CONST $message.MAX_LONG_MESSAGE_RX_PAYLOAD_SIZE 80;



   .CONST $message.MAX_LONG_MESSAGE_TX_SIZE ($message.MAX_LONG_MESSAGE_TX_PAYLOAD_SIZE + 2);
   .CONST $message.MAX_LONG_MESSAGE_RX_SIZE ($message.MAX_LONG_MESSAGE_RX_PAYLOAD_SIZE + 2);


   .CONST $message.QUEUE_SIZE_IN_MSGS (($message.MAX_LONG_MESSAGE_TX_SIZE+3)>>2)+1;


   .CONST $message.QUEUE_SIZE_IN_WORDS ($message.QUEUE_SIZE_IN_MSGS * (1+4));
   .CONST $message.LONG_MESSAGE_BUFFER_SIZE (((($message.MAX_LONG_MESSAGE_RX_SIZE+3)>>2)+1) * 4);



   .CONST $message.MAX_MESSAGE_HANDLERS 50;





   .CONST $message.REATTEMPT_SEND_PERIOD 1000;


   .CONST $message.TO_DSP_SHARED_WIN_SIZE 4;
   .CONST $message.TO_MCU_SHARED_WIN_SIZE 4;
   .CONST $message.ACK_FROM_MCU ($MCUWIN1_START + 0);
   .CONST $message.ACK_FROM_DSP ($MCUWIN1_START + 1);
   .CONST $message.DATA_TO_MCU ($MCUWIN1_START + 2);
   .CONST $message.DATA_TO_DSP ($MCUWIN1_START + 2 + $message.TO_DSP_SHARED_WIN_SIZE);


   .CONST $message.LAST_ENTRY -1;


   .CONST $message.LONG_MESSAGE_MODE_ID -2;


   .CONST $message.NEXT_ADDR_FIELD 0;
   .CONST $message.ID_FIELD 1;
   .CONST $message.HANDLER_ADDR_FIELD 2;
   .CONST $message.MASK_FIELD 3;
   .CONST $message.STRUC_SIZE 4;


   .CONST $message.QUEUE_WORDS_PER_MSG (1+4);
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbuffer.h" 1
.linefile 11 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbuffer.h"
   .CONST $cbuffer.SIZE_FIELD 0;
   .CONST $cbuffer.READ_ADDR_FIELD 1;
   .CONST $cbuffer.WRITE_ADDR_FIELD 2;




      .CONST $cbuffer.STRUC_SIZE 3;




 .CONST $frmbuffer.CBUFFER_PTR_FIELD 0;
 .CONST $frmbuffer.FRAME_PTR_FIELD 1;
 .CONST $frmbuffer.FRAME_SIZE_FIELD 2;
 .CONST $frmbuffer.STRUC_SIZE 3;
.linefile 42 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbuffer.h"
      .CONST $cbuffer.NUM_PORTS 12;
      .CONST $cbuffer.WRITE_PORT_OFFSET 0x00000C;
      .CONST $cbuffer.PORT_NUMBER_MASK 0x00000F;
      .CONST $cbuffer.TOTAL_PORT_NUMBER_MASK 0x00001F;
      .CONST $cbuffer.TOTAL_CONTINUOUS_PORTS 8;







   .CONST $cbuffer.MMU_PAGE_SIZE 64;


   .CONST $cbuffer.READ_PORT_MASK 0x800000;
   .CONST $cbuffer.WRITE_PORT_MASK $cbuffer.READ_PORT_MASK + $cbuffer.WRITE_PORT_OFFSET;




   .CONST $cbuffer.FORCE_ENDIAN_MASK 0x300000;
   .CONST $cbuffer.FORCE_ENDIAN_SHIFT_AMOUNT -21;
   .CONST $cbuffer.FORCE_LITTLE_ENDIAN 0x100000;
   .CONST $cbuffer.FORCE_BIG_ENDIAN 0x300000;


   .CONST $cbuffer.FORCE_SIGN_EXTEND_MASK 0x0C0000;
   .CONST $cbuffer.FORCE_SIGN_EXTEND_SHIFT_AMOUNT -19;
   .CONST $cbuffer.FORCE_SIGN_EXTEND 0x040000;
   .CONST $cbuffer.FORCE_NO_SIGN_EXTEND 0x0C0000;


   .CONST $cbuffer.FORCE_BITWIDTH_MASK 0x038000;
   .CONST $cbuffer.FORCE_BITWIDTH_SHIFT_AMOUNT -16;
   .CONST $cbuffer.FORCE_8BIT_WORD 0x008000;
   .CONST $cbuffer.FORCE_16BIT_WORD 0x018000;
   .CONST $cbuffer.FORCE_24BIT_WORD 0x028000;
   .CONST $cbuffer.FORCE_32BIT_WORD 0x038000;


   .CONST $cbuffer.FORCE_SATURATE_MASK 0x006000;
   .CONST $cbuffer.FORCE_SATURATE_SHIFT_AMOUNT -14;
   .CONST $cbuffer.FORCE_NO_SATURATE 0x002000;
   .CONST $cbuffer.FORCE_SATURATE 0x006000;


   .CONST $cbuffer.FORCE_PADDING_MASK 0x001C00;
   .CONST $cbuffer.FORCE_PADDING_SHIFT_AMOUNT -11;
   .CONST $cbuffer.FORCE_PADDING_NONE 0x000400;
   .CONST $cbuffer.FORCE_PADDING_LS_BYTE 0x000C00;
   .CONST $cbuffer.FORCE_PADDING_MS_BYTE 0x001400;


   .CONST $cbuffer.FORCE_PCM_AUDIO $cbuffer.FORCE_LITTLE_ENDIAN +
                                                      $cbuffer.FORCE_SIGN_EXTEND +
                                                      $cbuffer.FORCE_SATURATE;
   .CONST $cbuffer.FORCE_24B_PCM_AUDIO $cbuffer.FORCE_LITTLE_ENDIAN +
                                                      $cbuffer.FORCE_32BIT_WORD +
                                                      $cbuffer.FORCE_PADDING_MS_BYTE +
                                                      $cbuffer.FORCE_NO_SATURATE;

   .CONST $cbuffer.FORCE_16BIT_DATA_STREAM $cbuffer.FORCE_BIG_ENDIAN +
                                                      $cbuffer.FORCE_NO_SIGN_EXTEND +
                                                      $cbuffer.FORCE_NO_SATURATE;
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/interrupt.h" 1
.linefile 27 "C:/ADK3.5/kalimba/lib_sets/sdk/include/interrupt.h"
   .CONST $INTERRUPT_STORE_STATE_SIZE 48;
.linefile 14 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/pskey.h" 1
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/pskey.h"
   .CONST $pskey.NEXT_ENTRY_FIELD 0;
   .CONST $pskey.KEY_NUM_FIELD 1;
   .CONST $pskey.HANDLER_ADDR_FIELD 2;
   .CONST $pskey.STRUC_SIZE 3;



   .CONST $pskey.MAX_HANDLERS 50;

   .CONST $pskey.LAST_ENTRY -1;
   .CONST $pskey.REATTEMPT_TIME_PERIOD 10000;

   .CONST $pskey.FAILED_READ_LENGTH -1;
.linefile 15 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/flash.h" 1
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/flash.h"
   .CONST $PM_FLASHWIN_SIZE_MAX 0x40000;




   .CONST $flash.get_file_address.MAX_HANDLERS 10;


   .CONST $flash.get_file_address.NEXT_ENTRY_FIELD 0;
   .CONST $flash.get_file_address.FILE_ID_FIELD 1;
   .CONST $flash.get_file_address.HANDLER_ADDR_FIELD 2;
   .CONST $flash.get_file_address.STRUC_SIZE 3;

   .CONST $flash.get_file_address.LAST_ENTRY -1;
   .CONST $flash.get_file_address.REATTEMPT_TIME_PERIOD 10000;

   .CONST $flash.get_file_address.MESSAGE_HANDLER_UNINITIALISED -1;
.linefile 16 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/wall_clock.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/wall_clock.h"
   .CONST $wall_clock.FIRMWARE_WALL_CLOCK_PERIOD_VALUE 625;
   .CONST $wall_clock.FIRMWARE_WALL_CLOCK_PERIOD_SHIFT -1;

   .CONST $wall_clock.UPDATE_TIMER_PERIOD 100000;





   .CONST $wall_clock.MAX_WALL_CLOCKS 7;

   .CONST $wall_clock.LAST_ENTRY -1;

   .CONST $wall_clock.NEXT_ADDR_FIELD 0;
   .CONST $wall_clock.BT_ADDR_TYPE_FIELD 1;
   .CONST $wall_clock.BT_ADDR_WORD0_FIELD 2;
   .CONST $wall_clock.BT_ADDR_WORD1_FIELD 3;
   .CONST $wall_clock.BT_ADDR_WORD2_FIELD 4;
   .CONST $wall_clock.ADJUSTMENT_VALUE_FIELD 5;
   .CONST $wall_clock.CALLBACK_FIELD 6;
   .CONST $wall_clock.TIMER_STRUC_FIELD 7;
   .CONST $wall_clock.STRUC_SIZE 8 + $timer.STRUC_SIZE;

   .CONST $wall_clock.BT_TICKS_IN_7500_US 24;
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/pio.h" 1
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/pio.h"
   .CONST $pio.NEXT_ADDR_FIELD 0;
   .CONST $pio.PIO_BITMASK_FIELD 1;
   .CONST $pio.HANDLER_ADDR_FIELD 2;
   .CONST $pio.STRUC_SIZE 3;



   .CONST $pio.MAX_HANDLERS 20;

   .CONST $pio.LAST_ENTRY -1;
.linefile 18 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/profiler.h" 1
.linefile 41 "C:/ADK3.5/kalimba/lib_sets/sdk/include/profiler.h"
   .CONST $profiler.MAX_PROFILER_HANDLERS 50;

   .CONST $profiler.LAST_ENTRY -1;





   .CONST $profiler.UNINITIALISED -2;

   .CONST $profiler.NEXT_ADDR_FIELD 0;
   .CONST $profiler.CPU_FRACTION_FIELD 1;
   .CONST $profiler.START_TIME_FIELD 2;
   .CONST $profiler.INT_START_TIME_FIELD 3;
   .CONST $profiler.TOTAL_TIME_FIELD 4;

      .CONST $profiler.RUN_CLKS_MS_START_FIELD 5;
      .CONST $profiler.RUN_CLKS_LS_START_FIELD 6;
      .CONST $profiler.RUN_CLKS_MS_TOTAL_FIELD 7;
      .CONST $profiler.RUN_CLKS_LS_TOTAL_FIELD 8;
      .CONST $profiler.RUN_CLKS_AVERAGE_FIELD 9;
      .CONST $profiler.RUN_CLKS_MS_MAX_FIELD 10;
      .CONST $profiler.RUN_CLKS_LS_MAX_FIELD 11;
      .CONST $profiler.INT_START_CLKS_MS_FIELD 12;
      .CONST $profiler.INT_START_CLKS_LS_FIELD 13;
      .CONST $profiler.INSTRS_MS_START_FIELD 14;
      .CONST $profiler.INSTRS_LS_START_FIELD 15;
      .CONST $profiler.INSTRS_MS_TOTAL_FIELD 16;
      .CONST $profiler.INSTRS_LS_TOTAL_FIELD 17;
      .CONST $profiler.INSTRS_AVERAGE_FIELD 18;
      .CONST $profiler.INSTRS_MS_MAX_FIELD 19;
      .CONST $profiler.INSTRS_LS_MAX_FIELD 20;
      .CONST $profiler.INT_START_INSTRS_MS_FIELD 21;
      .CONST $profiler.INT_START_INSTRS_LS_FIELD 22;
      .CONST $profiler.STALLS_MS_START_FIELD 23;
      .CONST $profiler.STALLS_LS_START_FIELD 24;
      .CONST $profiler.STALLS_MS_TOTAL_FIELD 25;
      .CONST $profiler.STALLS_LS_TOTAL_FIELD 26;
      .CONST $profiler.STALLS_AVERAGE_FIELD 27;
      .CONST $profiler.STALLS_MS_MAX_FIELD 28;
      .CONST $profiler.STALLS_LS_MAX_FIELD 29;
      .CONST $profiler.INT_START_STALLS_MS_FIELD 30;
      .CONST $profiler.INT_START_STALLS_LS_FIELD 31;
      .CONST $profiler.TEMP_COUNT_FIELD 32;
      .CONST $profiler.COUNT_FIELD 33;
      .CONST $profiler.STRUC_SIZE 34;
.linefile 19 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/fwrandom.h" 1
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/fwrandom.h"
   .CONST $fwrandom.NEXT_ENTRY_FIELD 0;
   .CONST $fwrandom.NUM_REQ_FIELD 1;
   .CONST $fwrandom.NUM_RESP_FIELD 2;
   .CONST $fwrandom.RESP_BUF_FIELD 3;
   .CONST $fwrandom.HANDLER_ADDR_FIELD 4;
   .CONST $fwrandom.STRUC_SIZE 5;



   .CONST $fwrandom.MAX_HANDLERS 50;

   .CONST $fwrandom.LAST_ENTRY -1;
   .CONST $fwrandom.REATTEMPT_TIME_PERIOD 10000;
   .CONST $fwrandom.MAX_RAND_BITS 512;

   .CONST $fwrandom.FAILED_READ_LENGTH -1;
.linefile 20 "C:/ADK3.5/kalimba/lib_sets/sdk/include/core_library.h" 2
.linefile 89 "vee_heartrate.asm" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops_library.h" 1
.linefile 9 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops_library.h"
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h"
   .CONST $cbops.MAX_NUM_CHANNELS 16;
   .CONST $cbops.NO_MORE_OPERATORS -1;
   .CONST $cbops.MAX_OPERATORS 10;
   .CONST $cbops.MAX_COPY_SIZE 512;



   .CONST $cbops.OPERATOR_STRUC_ADDR_FIELD 0;
   .CONST $cbops.NUM_INPUTS_FIELD 1;




   .CONST $cbops.NEXT_OPERATOR_ADDR_FIELD 0;
   .CONST $cbops.FUNCTION_VECTOR_FIELD 1;
   .CONST $cbops.PARAMETER_AREA_START_FIELD 2;
   .CONST $cbops.STRUC_SIZE 3;



.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops_vector_table.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops_vector_table.h"
   .CONST $cbops.function_vector.RESET_FIELD 0;
   .CONST $cbops.function_vector.AMOUNT_TO_USE_FIELD 1;
   .CONST $cbops.function_vector.MAIN_FIELD 2;
   .CONST $cbops.function_vector.STRUC_SIZE 3;

   .CONST $cbops.function_vector.NO_FUNCTION 0;
.linefile 34 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_dc_remove.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_dc_remove.h"
   .CONST $cbops.dc_remove.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.dc_remove.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.dc_remove.DC_ESTIMATE_FIELD 2;
   .CONST $cbops.dc_remove.STRUC_SIZE 3;





   .CONST $cbops.dc_remove.FILTER_COEF 0.0003;
.linefile 37 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_limited_copy.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_limited_copy.h"
   .CONST $cbops.limited_copy.READ_LIMIT_FIELD 0;
   .CONST $cbops.limited_copy.WRITE_LIMIT_FIELD 1;
   .CONST $cbops.limited_copy.STRUC_SIZE 2;

   .CONST $cbops.limited_copy.NO_READ_LIMIT -1;
   .CONST $cbops.limited_copy.NO_WRITE_LIMIT -1;
.linefile 40 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_fill_limit.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_fill_limit.h"
   .CONST $cbops.fill_limit.FILL_LIMIT_FIELD 0;
   .CONST $cbops.fill_limit.OUT_BUFFER_FIELD 1;
   .CONST $cbops.fill_limit.STRUC_SIZE 2;
   .CONST $cbops.fill_limit.NO_LIMIT -1;
.linefile 43 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_noise_gate.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_noise_gate.h"
   .CONST $cbops.noise_gate.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.noise_gate.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.noise_gate.MONOSTABLE_COUNT_FIELD 2;
   .CONST $cbops.noise_gate.DECAYATTACK_COUNT_FIELD 3;
   .CONST $cbops.noise_gate.STRUC_SIZE 4;
.linefile 46 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_shift.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_shift.h"
   .CONST $cbops.shift.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.shift.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.shift.SHIFT_AMOUNT_FIELD 2;
   .CONST $cbops.shift.STRUC_SIZE 3;
.linefile 49 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_sidetone_mix.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_sidetone_mix.h"
   .CONST $cbops.sidetone_mix.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.sidetone_mix.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD 2;
   .CONST $cbops.sidetone_mix.SIDETONE_MAX_SAMPLES_FIELD 3;
   .CONST $cbops.sidetone_mix.GAIN_FIELD 4;
   .CONST $cbops.sidetone_mix.STRUC_SIZE 5;
.linefile 52 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_silence_clip_detect.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_silence_clip_detect.h"
   .CONST $cbops.silence_clip_detect.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.silence_clip_detect.INSTANCE_NO_FIELD 1;
   .CONST $cbops.silence_clip_detect.SILENCE_LIMIT_FIELD 2;
   .CONST $cbops.silence_clip_detect.CLIP_LIMIT_FIELD 3;
   .CONST $cbops.silence_clip_detect.SILENCE_PERIOD_LSW_FIELD 4;
   .CONST $cbops.silence_clip_detect.SILENCE_PERIOD_MSW_FIELD 5;
   .CONST $cbops.silence_clip_detect.PREVIOUS_TIME_FIELD 6;
   .CONST $cbops.silence_clip_detect.SILENCE_AMOUNT_LSW_FIELD 7;
   .CONST $cbops.silence_clip_detect.SILENCE_AMOUNT_MSW_FIELD 8;
   .CONST $cbops.silence_clip_detect.STRUC_SIZE 9;

   .CONST $cbops.silence_clip_detect.LOOK_UP_SIZE 8;
.linefile 55 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_upsample_mix.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_upsample_mix.h"
   .CONST $cbops.upsample_mix.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.upsample_mix.TONE_SOURCE_FIELD 2;
   .CONST $cbops.upsample_mix.TONE_VOL_FIELD 3;
   .CONST $cbops.upsample_mix.AUDIO_VOL_FIELD 4;
   .CONST $cbops.upsample_mix.RESAMPLE_COEFS_ADDR_FIELD 5;
   .CONST $cbops.upsample_mix.RESAMPLE_COEFS_SIZE_FIELD 6;
   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_ADDR_FIELD 7;
   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_SIZE_FIELD 8;
   .CONST $cbops.upsample_mix.UPSAMPLE_RATIO_FIELD 9;
   .CONST $cbops.upsample_mix.INTERP_RATIO_FIELD 10;
   .CONST $cbops.upsample_mix.INTERP_COEF_CURRENT_FIELD 11;
   .CONST $cbops.upsample_mix.INTERP_LAST_VAL_FIELD 12;
   .CONST $cbops.upsample_mix.TONE_PLAYING_STATE_FIELD 13;
   .CONST $cbops.upsample_mix.TONE_DATA_AMOUNT_READ_FIELD 14;
   .CONST $cbops.upsample_mix.TONE_DATA_AMOUNT_FIELD 15;
   .CONST $cbops.upsample_mix.LOCATION_IN_LOOP_FIELD 16;
   .CONST $cbops.upsample_mix.STRUC_SIZE 17;



   .CONST $cbops.upsample_mix.TONE_START_LEVEL 118;





   .CONST $cbops.upsample_mix.TONE_BLOCK_SIZE 72;

   .CONST $cbops.upsample_mix.TONE_PLAYING_STATE_STOPPED 0;
   .CONST $cbops.upsample_mix.TONE_PLAYING_STATE_PLAYING 1;

   .CONST $cbops.upsample_mix.NO_BUFFER -1;



   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_LENGTH_HIGH_QUALITY 10;



   .CONST $cbops.upsample_mix.RESAMPLE_BUFFER_LENGTH_LOW_QUALITY 4;
.linefile 58 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_volume.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_volume.h"
   .CONST $cbops.volume.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.volume.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.volume.FINAL_VALUE_FIELD 2;
   .CONST $cbops.volume.CURRENT_VALUE_FIELD 3;
   .CONST $cbops.volume.SAMPLES_PER_STEP_FIELD 4;
   .CONST $cbops.volume.STEP_SHIFT_FIELD 5;
   .CONST $cbops.volume.DELTA_FIELD 6;
   .CONST $cbops.volume.CURRENT_STEP_FIELD 7;
   .CONST $cbops.volume.STRUC_SIZE 8;
.linefile 61 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_volume_basic.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_volume_basic.h"
   .CONST $cbops.volume_basic.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.volume_basic.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.volume_basic.FINAL_VALUE_FIELD 2;
   .CONST $cbops.volume_basic.CURRENT_VALUE_FIELD 3;
   .CONST $cbops.volume_basic.SMOOTHING_VALUE_FIELD 4;
   .CONST $cbops.volume_basic.DELTA_THRESHOLD_VALUE_FIELD 5;
   .CONST $cbops.volume_basic.STRUC_SIZE 6;
.linefile 64 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_warp_and_shift.h" 1
.linefile 11 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_warp_and_shift.h"
   .CONST $cbops.warp_and_shift.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD 2;
   .CONST $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD 3;
   .CONST $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD 4;




   .CONST $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD + 1;

   .CONST $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD + 1;
   .CONST $cbops.warp_and_shift.CURRENT_WARP_FIELD $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD + 1;
   .CONST $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD $cbops.warp_and_shift.CURRENT_WARP_FIELD + 1;
   .CONST $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD + 1;
   .CONST $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD + 1;
   .CONST $cbops.warp_and_shift.PREVIOUS_STATE_FIELD $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD + 1;
   .CONST $cbops.warp_and_shift.STRUC_SIZE $cbops.warp_and_shift.PREVIOUS_STATE_FIELD + 1;

   .CONST $cbops.warp_and_shift.filt_coefs.L_FIELD 0;
   .CONST $cbops.warp_and_shift.filt_coefs.R_FIELD 1;
   .CONST $cbops.warp_and_shift.filt_coefs.INV_R_FIELD 2;
   .CONST $cbops.warp_and_shift.filt_coefs.COEFS_FIELD 3;
.linefile 67 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_deinterleave.h" 1
.linefile 10 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_deinterleave.h"
   .CONST $cbops.deinterleave.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.deinterleave.OUTPUT1_START_INDEX_FIELD 1;
   .CONST $cbops.deinterleave.OUTPUT2_START_INDEX_FIELD 2;
   .CONST $cbops.deinterleave.SHIFT_AMOUNT_FIELD 3;
   .CONST $cbops.deinterleave.STRUC_SIZE 4;
.linefile 70 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_rate_adjustment_and_shift.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_rate_adjustment_and_shift.h"
   .CONST $cbops.rate_adjustment_and_shift.INPUT1_START_INDEX_FIELD 0;
   .CONST $cbops.rate_adjustment_and_shift.OUTPUT1_START_INDEX_FIELD 1;
   .CONST $cbops.rate_adjustment_and_shift.INPUT2_START_INDEX_FIELD 2;
   .CONST $cbops.rate_adjustment_and_shift.OUTPUT2_START_INDEX_FIELD 3;
   .CONST $cbops.rate_adjustment_and_shift.SHIFT_AMOUNT_FIELD 4;
   .CONST $cbops.rate_adjustment_and_shift.FILTER_COEFFS_FIELD 5;
   .CONST $cbops.rate_adjustment_and_shift.HIST1_BUF_FIELD 6;
   .CONST $cbops.rate_adjustment_and_shift.HIST2_BUF_FIELD 7;





   .CONST $cbops.rate_adjustment_and_shift.SRA_TARGET_RATE_ADDR_FIELD $cbops.rate_adjustment_and_shift.HIST2_BUF_FIELD+1;

   .CONST $cbops.rate_adjustment_and_shift.DITHER_TYPE_FIELD $cbops.rate_adjustment_and_shift.SRA_TARGET_RATE_ADDR_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.ENABLE_COMPRESSOR_FIELD $cbops.rate_adjustment_and_shift.DITHER_TYPE_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.FILTER_COEFFS_SIZE_FIELD $cbops.rate_adjustment_and_shift.ENABLE_COMPRESSOR_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD $cbops.rate_adjustment_and_shift.FILTER_COEFFS_SIZE_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.RF $cbops.rate_adjustment_and_shift.SRA_CURRENT_RATE_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.PREV_SHORT_SAMPLES_FIELD $cbops.rate_adjustment_and_shift.RF+1;
   .CONST $cbops.rate_adjustment_and_shift.WORKING_STATE_FIELD $cbops.rate_adjustment_and_shift.PREV_SHORT_SAMPLES_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.DITHER_HIST_LEFT_INDEX_FIELD $cbops.rate_adjustment_and_shift.WORKING_STATE_FIELD+1;
   .CONST $cbops.rate_adjustment_and_shift.DITHER_HIST_RIGHT_INDEX_FIELD $cbops.rate_adjustment_and_shift.DITHER_HIST_LEFT_INDEX_FIELD+1;

   .CONST $cbops.rate_adjustment_and_shift.STRUC_SIZE $cbops.rate_adjustment_and_shift.DITHER_HIST_RIGHT_INDEX_FIELD+1;


   .CONST $cbops.rate_adjustment_and_shift_complete.STRUC_SIZE 1;

   .CONST $cbops.rate_adjustment_and_shift.SRA_UPRATE 21;

   .CONST $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE 12;
   .CONST $cbops.rate_adjustment_and_shift.SRA_HD_QUALITY_COEFFS_SIZE 36;

  .CONST $sra.MOVING_STEP (0.0015*(1.0/1000.0)/10.0);
.linefile 73 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_one_to_two_chan_copy.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_one_to_two_chan_copy.h"
   .CONST $cbops.one_to_two_chan_copy.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.one_to_two_chan_copy.OUTPUT_A_START_INDEX_FIELD 1;
   .CONST $cbops.one_to_two_chan_copy.OUTPUT_B_START_INDEX_FIELD 2;
   .CONST $cbops.one_to_two_chan_copy.STRUC_SIZE 3;
.linefile 76 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_copy_op.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_copy_op.h"
   .CONST $cbops.copy_op.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.copy_op.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.copy_op.STRUC_SIZE 2;
.linefile 79 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_compress_copy_op.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_compress_copy_op.h"
   .CONST $cbops.compress_copy_op.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.compress_copy_op.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.compress_copy_op.SHIFT_AMOUNT 2;
   .CONST $cbops.compress_copy_op.STRUC_SIZE 3;

   .CONST $COMPRESS_RANGE 0.1087;
.linefile 82 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_mix.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_mix.h"
   .CONST $cbops.mix.MIX_SOURCE_FIELD 0;
   .CONST $cbops.mix.MIX_VOL_FIELD 1;
   .CONST $cbops.mix.AUDIO_VOL_FIELD 2;
   .CONST $cbops.mix.MIXING_STATE_FIELD 3;
   .CONST $cbops.mix.MIXING_START_LEVEL_FIELD 4;
   .CONST $cbops.mix.NUMBER_OF_INPUTS_FIELD 5;
   .CONST $cbops.mix.INPUT_START_INDEX_FIELD 6;




   .CONST $cbops.mix.MIX_INPUT_START_LEVEL 118;

   .CONST $cbops.mix.MIXING_STATE_STOPPED 0;
   .CONST $cbops.mix.MIXING_STATE_MIXING 1;
.linefile 85 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_mono_to_stereo.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_mono_to_stereo.h"
   .CONST $cbops.mono_to_stereo.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.mono_to_stereo.OUTPUT_1_START_INDEX_FIELD 1;
   .CONST $cbops.mono_to_stereo.OUTPUT_2_START_INDEX_FIELD 2;
   .CONST $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD 3;
   .CONST $cbops.mono_to_stereo.RATIO 4;
   .CONST $cbops.mono_to_stereo.STRUC_SIZE 5;
.linefile 88 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_stereo_3d_enhance_op.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_stereo_3d_enhance_op.h"
   .CONST $cbops.stereo_3d_enhance_op.INPUT_1_START_INDEX_FIELD 0;
   .CONST $cbops.stereo_3d_enhance_op.INPUT_2_START_INDEX_FIELD 1;
   .CONST $cbops.stereo_3d_enhance_op.OUTPUT_1_START_INDEX_FIELD 2;
   .CONST $cbops.stereo_3d_enhance_op.OUTPUT_2_START_INDEX_FIELD 3;
   .CONST $cbops.stereo_3d_enhance_op.DELAY_1_STRUC_FIELD 4;
   .CONST $cbops.stereo_3d_enhance_op.DELAY_2_STRUC_FIELD 5;
   .CONST $cbops.stereo_3d_enhance_op.COEF_STRUC_FIELD 6;
   .CONST $cbops.stereo_3d_enhance_op.REFLECTION_DELAY_SAMPLES_FIELD 7;
   .CONST $cbops.stereo_3d_enhance_op.STRUC_SIZE 8;

   .CONST $cbops.stereo_3d_enhance_op.REFLECTION_DELAY 618;
.linefile 91 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_status_check_gain.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_status_check_gain.h"
   .CONST $cbops.status_check_gain.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.status_check_gain.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.status_check_gain.GAIN_ADDRESS_FIELD 2;
   .CONST $cbops.status_check_gain.PORT_ADDRESS_FIELD 3;
   .CONST $cbops.status_check_gain.STRUC_SIZE 4;
.linefile 94 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_scale.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_scale.h"
   .CONST $cbops.scale.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.scale.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.scale.PRE_INT_AMOUNT_FIELD 2;
   .CONST $cbops.scale.FRAC_AMOUNT_FIELD 3;
   .CONST $cbops.scale.POST_INT_AMOUNT_FIELD 4;
   .CONST $cbops.scale.STRUC_SIZE 5;
.linefile 97 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_two_to_one_chan_copy.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_two_to_one_chan_copy.h"
   .CONST $cbops.two_to_one_chan_copy.INPUT_A_START_INDEX_FIELD 0;
   .CONST $cbops.two_to_one_chan_copy.INPUT_B_START_INDEX_FIELD 1;
   .CONST $cbops.two_to_one_chan_copy.OUTPUT_B_START_INDEX_FIELD 2;
   .CONST $cbops.two_to_one_chan_copy.INPUT_A_GAIN_FIELD 3;
   .CONST $cbops.two_to_one_chan_copy.INPUT_B_GAIN_FIELD 4;

   .CONST $cbops.two_to_one_chan_copy.STRUC_SIZE 5;
.linefile 100 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_eq.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_eq.h"
   .CONST $cbops.eq.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.eq.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.eq.PTR_DELAY_LINE_FIELD 2;
   .CONST $cbops.eq.PTR_COEFS_BUFF_FIELD 3;
   .CONST $cbops.eq.NUM_STAGES_FIELD 4;
   .CONST $cbops.eq.DELAY_BUF_SIZE 5;
   .CONST $cbops.eq.COEFF_BUF_SIZE 6;
   .CONST $cbops.eq.BLOCK_SIZE_FIELD 7;
   .CONST $cbops.eq.PTR_SCALE_BUFF_FIELD 8;
   .CONST $cbops.eq.INPUT_GAIN_EXPONENT_PTR 9;
   .CONST $cbops.eq.INPUT_GAIN_MANTISA_PTR 10;
   .CONST $cbops.eq.STRUC_SIZE 11;
.linefile 103 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2

.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/resample/resample_header.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/resample/resample_header.h"
   .CONST $cbops.resample.INPUT_1_START_INDEX_FIELD 0;
   .CONST $cbops.resample.INPUT_2_START_INDEX_FIELD 1;
   .CONST $cbops.resample.OUTPUT_1_START_INDEX_FIELD 2;
   .CONST $cbops.resample.OUTPUT_2_START_INDEX_FIELD 3;
   .CONST $cbops.resample.COEF_BUF_INDEX_FIELD 4;
   .CONST $cbops.resample.CONVERT_RATIO_INT_FIELD 5;
   .CONST $cbops.resample.CONVERT_RATIO_FRAC_FIELD 6;
   .CONST $cbops.resample.INV_CONVERT_RATIO_FIELD 7;
   .CONST $cbops.resample.RATIO_IN_FIELD 8;
   .CONST $cbops.resample.RATIO_OUT_FIELD 9;
   .CONST $cbops.resample.STRUC_SIZE 10;


   .CONST $cbops.auto_resample_mix.IO_LEFT_INDEX_FIELD 0;
   .CONST $cbops.auto_resample_mix.IO_RIGHT_INDEX_FIELD 1;
   .CONST $cbops.auto_resample_mix.TONE_CBUFFER_FIELD 2;
   .CONST $cbops.auto_resample_mix.COEF_BUF_INDEX_FIELD 3;
   .CONST $cbops.auto_resample_mix.OUTPUT_RATE_ADDR_FIELD 4;
   .CONST $cbops.auto_resample_mix.HIST_BUF_FIELD 5;
   .CONST $cbops.auto_resample_mix.INPUT_RATE_ADDR_FIELD 6;
   .CONST $cbops.auto_resample_mix.TONE_MIXING_RATIO_FIELD 7;
   .CONST $cbops.auto_resample_mix.AUDIO_MIXING_RATIO_FIELD 8;
   .CONST $cbops.auto_resample_mix.CONVERT_RATIO_FRAC_FIELD 9;
   .CONST $cbops.auto_resample_mix.CURRENT_OUTPUT_RATE_FIELD 10;
   .CONST $cbops.auto_resample_mix.CURRENT_INPUT_RATE_FIELD 11;
   .CONST $cbops.auto_resample_mix.CONVERT_RATIO_INT_FIELD 12;
   .CONST $cbops.auto_resample_mix.IR_RATIO_FIELD 13;
   .CONST $cbops.auto_resample_mix.SOFT_MOVE_GAIN_FIELD 14;
   .CONST $cbops.auto_resample_mix.INPUT_STATE_FIELD 15;
   .CONST $cbops.auto_resample_mix.INPUT_COUNTER_FIELD 16;
   .CONST $cbops.auto_resample_mix.OPERATION_MODE_FIELD 17;
   .CONST $cbops.auto_resample_mix.STRUC_SIZE 18;


   .CONST $cbops.auto_resample_mix.TONE_MIXING_NOTONE_STATE 0;
   .CONST $cbops.auto_resample_mix.TONE_MIXING_NORMAL_STATE 1;


   .CONST $cbops.auto_resample_mix.TONE_MIXING_RESAMPLE_ACTION 0;
   .CONST $cbops.auto_resample_mix.TONE_MIXING_IGNORE_ACTION 1;
   .CONST $cbops.auto_resample_mix.TONE_MIXING_JUSTMIX_ACTION 2;


   .CONST $cbops.auto_resample_mix.TONE_FILTER_HIST_LENGTH $cbops.rate_adjustment_and_shift.SRA_COEFFS_SIZE;
   .CONST $cbops.auto_resample_mix.TONE_FILTER_UPRATE $cbops.rate_adjustment_and_shift.SRA_UPRATE;
.linefile 105 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2

.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_dither_and_shift.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_dither_and_shift.h"
   .CONST $cbops.dither_and_shift.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.dither_and_shift.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.dither_and_shift.SHIFT_AMOUNT_FIELD 2;
   .CONST $cbops.dither_and_shift.DITHER_TYPE_FIELD 3;
   .CONST $cbops.dither_and_shift.DITHER_FILTER_HIST_FIELD 4;
   .CONST $cbops.dither_and_shift.ENABLE_COMPRESSOR_FIELD 5;
   .CONST $cbops.dither_and_shift.STRUC_SIZE 6;


   .CONST $cbops.dither_and_shift.DITHER_TYPE_NONE 0;
   .CONST $cbops.dither_and_shift.DITHER_TYPE_TRIANGULAR 1;
   .CONST $cbops.dither_and_shift.DITHER_TYPE_SHAPED 2;




      .CONST $cbops.dither_and_shift.FILTER_COEFF_SIZE 5;
.linefile 107 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_univ_mix_op.h" 1
.linefile 15 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_univ_mix_op.h"
   .CONST $cbops.univ_mix_op.INPUT_START_INDEX_FIELD 0;
   .CONST $cbops.univ_mix_op.OUTPUT_START_INDEX_FIELD 1;
   .CONST $cbops.univ_mix_op.MIXER_PRIMARY_COPY_STRUCT_ADDR_FIELD 2;
   .CONST $cbops.univ_mix_op.MIXER_SECONDARY_COPY_STRUCT_ADDR_FIELD 3;
   .CONST $cbops.univ_mix_op.COMMON_PARAM_STRUCT_ADDR_FIELD 4;
   .CONST $cbops.univ_mix_op.PRIMARY_UPSAMPLER_STRUCT_ADDR_FIELD 5;
   .CONST $cbops.univ_mix_op.SECONDARY_UPSAMPLER_STRUCT_ADDR_FIELD 6;
   .CONST $cbops.univ_mix_op.OUTPUT_UPSAMPLER_STRUCT_ADDR_FIELD 7;

   .CONST $cbops.univ_mix_op.STRUC_SIZE 8;





   .CONST $cbops.univ_mix_op.common.CHANNELS_ACTIVITY_FIELD 0;

   .CONST $cbops.univ_mix_op.common.STRUC_SIZE 1;
.linefile 44 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_univ_mix_op.h"
   .CONST $cbops.univ_mix_op.params.INPUT_GAIN_FACTOR_FIELD 0;
   .CONST $cbops.univ_mix_op.params.INPUT_GAIN_SHIFT_FIELD 1;


   .CONST $cbops.univ_mix_op.params.RAMP_GAIN_WHEN_MIXING_FIELD 2;
   .CONST $cbops.univ_mix_op.params.TARGET_RAMP_GAIN_ADJUST_FIELD 3;
   .CONST $cbops.univ_mix_op.params.NUM_RAMP_SAMPLES_FIELD 4;
   .CONST $cbops.univ_mix_op.params.RAMP_STEP_SHIFT_FIELD 5;
   .CONST $cbops.univ_mix_op.params.RAMP_DELTA_FIELD 6;


   .CONST $cbops.univ_mix_op.params.UPSAMPLING_FACTOR_FIELD 7;
   .CONST $cbops.univ_mix_op.params.INPUT_RATE_FIELD 8;
   .CONST $cbops.univ_mix_op.params.INVERSE_INPUT_RATE_FIELD 9;
   .CONST $cbops.univ_mix_op.params.OUTPUT_RATE_FIELD 10;
   .CONST $cbops.univ_mix_op.params.INTERP_PHASE_STEP_FIELD 11;


   .CONST $cbops.univ_mix_op.params.RESAMPLE_COEFS_ADDR_FIELD 12;
   .CONST $cbops.univ_mix_op.params.RESAMPLE_COEFS_SIZE_FIELD 13;
   .CONST $cbops.univ_mix_op.params.RESAMPLE_BUFFER_SIZE_FIELD 14;

   .CONST $cbops.univ_mix_op.params.STRUC_SIZE 15;
.linefile 78 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_univ_mix_op.h"
   .CONST $cbops.univ_mix_op.data.PARAMETER_ADDR_FIELD 0;


   .CONST $cbops.univ_mix_op.data.INPUT_BUFFER_ADDR_FIELD 1;
   .CONST $cbops.univ_mix_op.data.INPUT_BUFFER_LENGTH_FIELD 2;
   .CONST $cbops.univ_mix_op.data.OUTPUT_BUFFER_ADDR_FIELD 3;
   .CONST $cbops.univ_mix_op.data.OUTPUT_BUFFER_LENGTH_FIELD 4;


   .CONST $cbops.univ_mix_op.data.INPUT_SAMPLES_REQUESTED_FIELD 5;
   .CONST $cbops.univ_mix_op.data.OUTPUT_SAMPLES_REQUESTED_FIELD 6;


   .CONST $cbops.univ_mix_op.data.INPUT_SAMPLES_READ_FIELD 7;
   .CONST $cbops.univ_mix_op.data.OUTPUT_SAMPLES_WRITTEN_FIELD 8;


   .CONST $cbops.univ_mix_op.data.RAMP_ACTIVE_FIELD 9;
   .CONST $cbops.univ_mix_op.data.CURRENT_RAMP_GAIN_ADJUST_FIELD 10;
   .CONST $cbops.univ_mix_op.data.CURRENT_RAMP_SAMPLE_COUNT_FIELD 11;
   .CONST $cbops.univ_mix_op.data.RAMP_CALLBACK_FIELD 12;


   .CONST $cbops.univ_mix_op.data.RESAMPLE_BUFFER_ADDR_FIELD 13;


   .CONST $cbops.univ_mix_op.data.INTERP_CURRENT_PHASE_FIELD 14;
   .CONST $cbops.univ_mix_op.data.INTERP_LAST_VAL_FIELD 15;
   .CONST $cbops.univ_mix_op.data.LOCATION_IN_LOOP_FIELD 16;

   .CONST $cbops.univ_mix_op.data.STRUC_SIZE 17;
.linefile 117 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_univ_mix_op.h"
   .CONST $cbops.univ_mix_op.common.NO_CHANNELS_ACTIVE (0x000000);
   .CONST $cbops.univ_mix_op.common.PRIMARY_CHANNEL_ACTIVE (0x000001);
   .CONST $cbops.univ_mix_op.common.SECONDARY_CHANNEL_ACTIVE (0x000002);
   .CONST $cbops.univ_mix_op.common.PRIMARY_AND_SECONDARY_CHANNEL_ACTIVE (0x000003);

   .CONST $cbops.univ_mix_op.common.DONT_MIX_PRIMARY_AND_SECONDARY_OUTPUTS (0x000000);
   .CONST $cbops.univ_mix_op.common.MIX_PRIMARY_AND_SECONDARY_OUTPUTS (0x000001);

   .CONST $cbops.univ_mix_op.UNITY_PHASE 0.125;
   .CONST $cbops.univ_mix_op.UNITY_PHASE_SHIFT_NORMALIZE 3;
   .CONST $cbops.univ_mix_op.PHASE_FRACTIONAL_PART_MASK (0x0fffff);
.linefile 110 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_s_to_m_op.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_s_to_m_op.h"
   .CONST $cbops.s_to_m_op.INPUT_LEFT_INDEX_FIELD 0;
   .CONST $cbops.s_to_m_op.INPUT_RIGHT_INDEX_FIELD 1;
   .CONST $cbops.s_to_m_op.OUTPUT_MONO_INDEX_FIELD 2;
   .CONST $cbops.s_to_m_op.STRUC_SIZE 3;
.linefile 113 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_cross_mix.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_cross_mix.h"
   .CONST $cbops.cross_mix.INPUT1_START_INDEX_FIELD 0;
   .CONST $cbops.cross_mix.INPUT2_START_INDEX_FIELD 1;
   .CONST $cbops.cross_mix.OUTPUT1_START_INDEX_FIELD 2;
   .CONST $cbops.cross_mix.OUTPUT2_START_INDEX_FIELD 3;
   .CONST $cbops.cross_mix.COEFF11_FIELD 4;
   .CONST $cbops.cross_mix.COEFF12_FIELD 5;
   .CONST $cbops.cross_mix.COEFF21_FIELD 6;
   .CONST $cbops.cross_mix.COEFF22_FIELD 7;

   .CONST $cbops.cross_mix.STRUC_SIZE 8;
.linefile 116 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_user_filter.h" 1
.linefile 13 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_user_filter.h"
   .CONST $cbops.user_filter.FUNCTION_PTR_PTR ($cbops.shift.STRUC_SIZE);
   .CONST $cbops.user_filter.STRUC_SIZE ($cbops.shift.STRUC_SIZE + 1);
.linefile 119 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2

.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/iir_resample/iir_resample_header.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/iir_resample/iir_resample_header.h"
   .CONST $cbops.mono.iir_resample.INPUT_1_START_INDEX_FIELD 0;
   .CONST $cbops.mono.iir_resample.OUTPUT_1_START_INDEX_FIELD 1;
   .CONST $cbops.mono.iir_resample.FILTER_DEFINITION_PTR_FIELD 2;
   .CONST $cbops.mono.iir_resample.INPUT_SCALE_FIELD 3;
   .CONST $cbops.mono.iir_resample.OUTPUT_SCALE_FIELD 4;
   .CONST $cbops.mono.iir_resample.SAMPLE_COUNT_FIELD 5;
   .CONST $cbops.mono.iir_resample.IIR_HISTORY_BUF_PTR_FIELD 6;
   .CONST $cbops.mono.iir_resample.FIR_HISTORY_BUF_PTR_FIELD 7;
   .CONST $cbops.mono.iir_resample.RESET_FLAG_FIELD 8;
   .CONST $cbops.mono.iir_resample.STRUC_SIZE 9;

   .CONST $cbops.stereo.iir_resample.INPUT_2_START_INDEX_FIELD 0;
   .CONST $cbops.stereo.iir_resample.OUTPUT_2_START_INDEX_FIELD 1;
   .CONST $cbops.stereo.iir_resample.INPUT_1_START_INDEX_FIELD 2;
   .CONST $cbops.stereo.iir_resample.OUTPUT_1_START_INDEX_FIELD 3;
   .CONST $cbops.stereo.iir_resample.FILTER_DEFINITION_PTR_FIELD 4;
   .CONST $cbops.stereo.iir_resample.INPUT_SCALE_FIELD 5;
   .CONST $cbops.stereo.iir_resample.OUTPUT_SCALE_FIELD 6;
   .CONST $cbops.stereo.iir_resample.CH1_SAMPLE_COUNT_FIELD 7;
   .CONST $cbops.stereo.iir_resample.CH1_IIR_HISTORY_BUF_PTR_FIELD 8;
   .CONST $cbops.stereo.iir_resample.CH1_FIR_HISTORY_BUF_PTR_FIELD 9;
   .CONST $cbops.stereo.iir_resample.CH2_SAMPLE_COUNT_FIELD 10;
   .CONST $cbops.stereo.iir_resample.CH2_IIR_HISTORY_BUF_PTR_FIELD 11;
   .CONST $cbops.stereo.iir_resample.CH2_FIR_HISTORY_BUF_PTR_FIELD 12;
   .CONST $cbops.stereo.iir_resample.RESET_FLAG_FIELD 13;
   .CONST $cbops.stereo.iir_resample.STRUC_SIZE 14;

   .CONST $cbops.iir_resample_complete.STRUC_SIZE 0;



   .CONST $cbops.frame.resample.CONVERSION_OBJECT_PTR_FIELD 0;
   .CONST $cbops.frame.resample.INPUT_PTR_FIELD 1;
   .CONST $cbops.frame.resample.INPUT_LENGTH_FIELD 2;
   .CONST $cbops.frame.resample.OUTPUT_PTR_FIELD 3;
   .CONST $cbops.frame.resample.OUTPUT_LENGTH_FIELD 4;
   .CONST $cbops.frame.resample.NUM_SAMPLES_FIELD 5;
   .CONST $cbops.frame.resample.SAMPLE_COUNT_FIELD 6;
   .CONST $cbops.frame.resample.IIR_HISTORY_BUF_PTR_FIELD 7;
   .CONST $cbops.frame.resample.FIR_HISTORY_BUF_PTR_FIELD 8;
   .CONST $cbops.frame.resample.DM1_OBJECT_SIZE_FIELD 9;


   .CONST $cbops.IIR_RESAMPLE_IIR_BUFFER_SIZE 9;
   .CONST $cbops.IIR_DOWNSAMPLE_FIR_BUFFER_SIZE 10;
   .CONST $cbops.IIR_UPSAMPLE_FIR_BUFFER_SIZE 7;
.linefile 121 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2

.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/iir_resamplev2/iir_resamplev2_header.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/iir_resamplev2/iir_resamplev2_header.h"
   .CONST $iir_resamplev2.INPUT_1_START_INDEX_FIELD 0;
   .CONST $iir_resamplev2.OUTPUT_1_START_INDEX_FIELD 1;

   .CONST $iir_resamplev2.FILTER_DEFINITION_PTR_FIELD 2;
   .CONST $iir_resamplev2.INPUT_SCALE_FIELD 3;
   .CONST $iir_resamplev2.OUTPUT_SCALE_FIELD 4;

   .CONST $iir_resamplev2.INTERMEDIATE_CBUF_PTR_FIELD 5;
   .CONST $iir_resamplev2.INTERMEDIATE_CBUF_LEN_FIELD 6;

   .CONST $iir_resamplev2.PARTIAL1_FIELD 7;
   .CONST $iir_resamplev2.SAMPLE_COUNT1_FIELD 8;
   .CONST $iir_resamplev2.FIR_HISTORY_BUF1_PTR_FIELD 9;
   .CONST $iir_resamplev2.IIR_HISTORY_BUF1_PTR_FIELD 10;

   .CONST $iir_resamplev2.PARTIAL2_FIELD 11;
   .CONST $iir_resamplev2.SAMPLE_COUNT2_FIELD 12;
   .CONST $iir_resamplev2.FIR_HISTORY_BUF2_PTR_FIELD 13;
   .CONST $iir_resamplev2.IIR_HISTORY_BUF2_PTR_FIELD 14;

   .CONST $iir_resamplev2.RESET_FLAG_FIELD 15;
   .CONST $iir_resamplev2.STRUC_SIZE 16;



   .CONST $cbops.complete.iir_resamplev2.STRUC_SIZE 1;




   .CONST $IIR_RESAMPLEV2_IIR_BUFFER_SIZE 19;
   .CONST $IIR_RESAMPLEV2_FIR_BUFFER_SIZE 10;
   .CONST $IIR_RESAMPLEV2_APPENDED_BUFFER_SIZE ($IIR_RESAMPLEV2_IIR_BUFFER_SIZE+$IIR_RESAMPLEV2_FIR_BUFFER_SIZE);
   .CONST $iir_resamplev2.OBJECT_SIZE $iir_resamplev2.STRUC_SIZE + 2*$IIR_RESAMPLEV2_APPENDED_BUFFER_SIZE;

   .CONST $iir_resamplev2.OBJECT_SIZE_SNGL_STAGE $iir_resamplev2.STRUC_SIZE + $IIR_RESAMPLEV2_APPENDED_BUFFER_SIZE;



   .CONST $cbops.frame.resamplev2.INPUT_PTR_FIELD 0;
   .CONST $cbops.frame.resamplev2.INPUT_LENGTH_FIELD 1;
   .CONST $cbops.frame.resamplev2.OUTPUT_PTR_FIELD 2;
   .CONST $cbops.frame.resamplev2.OUTPUT_LENGTH_FIELD 3;
   .CONST $cbops.frame.resamplev2.NUM_SAMPLES_FIELD 4;

   .CONST $cbops.frame.resamplev2.FILTER_DEFINITION_PTR_FIELD 5;
   .CONST $cbops.frame.resamplev2.INPUT_SCALE_FIELD 6;
   .CONST $cbops.frame.resamplev2.OUTPUT_SCALE_FIELD 7;

   .CONST $cbops.frame.resamplev2.INTERMEDIATE_CBUF_PTR_FIELD 8;
   .CONST $cbops.frame.resamplev2.INTERMEDIATE_CBUF_LEN_FIELD 9;

   .CONST $cbops.frame.resamplev2.PARTIAL1_FIELD 10;
   .CONST $cbops.frame.resamplev2.SAMPLE_COUNT1_FIELD 11;
   .CONST $cbops.frame.resamplev2.FIR_HISTORY_BUF1_PTR_FIELD 12;
   .CONST $cbops.frame.resamplev2.IIR_HISTORY_BUF1_PTR_FIELD 13;

   .CONST $cbops.frame.resamplev2.PARTIAL2_FIELD 14;
   .CONST $cbops.frame.resamplev2.SAMPLE_COUNT2_FIELD 15;
   .CONST $cbops.frame.resamplev2.FIR_HISTORY_BUF2_PTR_FIELD 16;
   .CONST $cbops.frame.resamplev2.IIR_HISTORY_BUF2_PTR_FIELD 17;

   .CONST $cbops.frame.resamplev2.RESET_FLAG_FIELD 18;
   .CONST $cbops.frame.resamplev2.STRUC_SIZE 19;

   .CONST $cbops.frame.resamplev2.OBJECT_SIZE $cbops.frame.resamplev2.STRUC_SIZE + 2*$IIR_RESAMPLEV2_APPENDED_BUFFER_SIZE;

   .CONST $cbops.frame.resamplev2.OBJECT_SIZE_SNGL_STAGE $cbops.frame.resamplev2.STRUC_SIZE + $IIR_RESAMPLEV2_APPENDED_BUFFER_SIZE;
.linefile 123 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2


.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_fixed_amount.h" 1
.linefile 11 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_fixed_amount.h"
   .CONST $cbops.fixed_amount.AMOUNT_FIELD 0;
   .CONST $cbops.fixed_amount.STRUC_SIZE 1;

   .CONST $cbops.fixed_amount.NO_AMOUNT -1;
.linefile 126 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2

.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_signal_detect.h" 1
.linefile 17 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_signal_detect.h"
    .const $cbops.signal_detect_op.COEFS_PTR 0;
    .const $cbops.signal_detect_op.NUM_CHANNELS 1;
    .const $cbops.signal_detect_op.FIRST_CHANNEL_INDEX 2;

    .const $cbops.signal_detect_op.STRUC_SIZE_MONO 3;
    .const $cbops.signal_detect_op.STRUC_SIZE_STEREO 4;
    .const $cbops.signal_detect_op.STRUC_SIZE_3_CHANNEL 5;




    .const $cbops.signal_detect_op_coef.LINEAR_THRESHOLD_VALUE 0;
    .const $cbops.signal_detect_op_coef.NO_SIGNAL_TRIGGER_TIME 1;
    .const $cbops.signal_detect_op_coef.CURRENT_MAX_VALUE 2;
    .const $cbops.signal_detect_op_coef.SECOND_TIMER 3;
    .const $cbops.signal_detect_op_coef.SIGNAL_STATUS 4;
    .const $cbops.signal_detect_op_coef.SIGNAL_STATUS_MSG_ID 5;

    .const $cbops.signal_detect_op_coef.STRUC_SIZE 6;
.linefile 128 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_stereo_soft_mute.h" 1
.linefile 14 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_stereo_soft_mute.h"
    .const $cbops.stereo_soft_mute_op.STRUC_SIZE 6;

    .const $cbops.stereo_soft_mute_op.INPUT_LEFT_START_INDEX_FIELD 0;
    .const $cbops.stereo_soft_mute_op.INPUT_RIGHT_START_INDEX_FIELD 1;
    .const $cbops.stereo_soft_mute_op.OUTPUT_LEFT_START_INDEX_FIELD 2;
    .const $cbops.stereo_soft_mute_op.OUTPUT_RIGHT_START_INDEX_FIELD 3;
    .const $cbops.stereo_soft_mute_op.MUTE_DIRECTION 4;
    .const $cbops.stereo_soft_mute_op.MUTE_INDEX 5;
.linefile 129 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_soft_mute.h" 1
.linefile 14 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_soft_mute.h"
    .const $cbops.soft_mute_op.STRUC_SIZE_MONO 5;
    .const $cbops.soft_mute_op.STRUC_SIZE_STEREO 7;

    .const $cbops.soft_mute_op.MUTE_DIRECTION 0;
    .const $cbops.soft_mute_op.MUTE_INDEX 1;
    .const $cbops.soft_mute_op.NUM_CHANNELS 2;
    .const $cbops.soft_mute_op.INPUT_1_START_INDEX_FIELD 3;
    .const $cbops.soft_mute_op.OUTPUT_1_START_INDEX_FIELD 4;
    .const $cbops.soft_mute_op.INPUT_2_START_INDEX_FIELD 5;
    .const $cbops.soft_mute_op.OUTPUT_2_START_INDEX_FIELD 6;
.linefile 130 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_switch.h" 1
.linefile 12 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_switch.h"
    .CONST $cbops.switch_op.SWITCH_ADDR_FIELD 0;
    .CONST $cbops.switch_op.ALT_NEXT_FIELD 1;
    .CONST $cbops.switch_op.SWITCH_MASK_FIELD 2;
    .CONST $cbops.switch_op.INVERT_CONTROL_FIELD 3;
    .CONST $cbops.switch_op.STRUC_SIZE 4;

    .CONST $cbops.switch_op.OFF 0;
    .CONST $cbops.switch_op.ON 1;

    .CONST $cbops.switch_op.INVERT_CONTROL 1;
.linefile 131 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_delay.h" 1
.linefile 15 "C:/ADK3.5/kalimba/lib_sets/sdk/include/operators/cbops_delay.h"
    .const $cbops.delay.INPUT_INDEX 0;
    .const $cbops.delay.OUTPUT_INDEX 1;
    .const $cbops.delay.DBUFF_ADDR_FIELD 2;
    .const $cbops.delay.DBUFF_SIZE_FIELD 3;
    .const $cbops.delay.DELAY_FIELD 4;

    .const $cbops.delay.STRUC_SIZE 5;
.linefile 132 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops.h" 2
.linefile 10 "C:/ADK3.5/kalimba/lib_sets/sdk/include/cbops_library.h" 2
.linefile 90 "vee_heartrate.asm" 2
.linefile 1 "C:/ADK3.5/kalimba/lib_sets/sdk/include/frame_sync_tsksched.h" 1
.linefile 11 "C:/ADK3.5/kalimba/lib_sets/sdk/include/frame_sync_tsksched.h"
.CONST $FRM_SCHEDULER.COUNT_FIELD 0;
.CONST $FRM_SCHEDULER.MAX_COUNT_FIELD 1;
.CONST $FRM_SCHEDULER.NUM_TASKS_FIELD 2;
.CONST $FRM_SCHEDULER.TOTAL_MIPS_FIELD 3;
.CONST $FRM_SCHEDULER.SEND_MIPS_FIELD 4;
.CONST $FRM_SCHEDULER.TOTALTM_FIELD 5;
.CONST $FRM_SCHEDULER.TOTALSND_FIELD 6;
.CONST $FRM_SCHEDULER.TIMER_FIELD 7;
.CONST $FRM_SCHEDULER.TRIGGER_FIELD 8;
.CONST $FRM_SCHEDULER.TASKS_FIELD 9;
.linefile 91 "vee_heartrate.asm" 2
.linefile 102 "vee_heartrate.asm"
.MODULE $vee.heartrate;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .VAR/DMCIRC sample_cbuffer[121];
 .VAR sample_cbuffer_struc[]=
  LENGTH(sample_cbuffer),
  &sample_cbuffer,
  &sample_cbuffer;

 .VAR/DMCIRC calc_cbuffer[121];
 .VAR calc_cbuffer_struc[]=
  LENGTH(calc_cbuffer),
  &calc_cbuffer,
  &calc_cbuffer;

 .VAR hb_enable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR hb_disable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR time_value_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR recover_hb_value_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR sample_timer_struc[$timer.STRUC_SIZE];
 .VAR sport_time_timer_struc[$timer.STRUC_SIZE];

 .VAR calc_buff[120];

 .VAR hb_sample_enable = 0;
 .VAR hb_time_enable = 0;

  .VAR heartrate = 0;
 .VAR time_value = 0;

 .VAR value_lsb;
 .VAR value_msb;
.linefile 144 "vee_heartrate.asm"
 .VAR hb_addr_min[18];
 .VAR hb_addr_max[18];
.linefile 171 "vee_heartrate.asm"
$vee.heartrate.start:

 push rLink;

 call $vee.heartrate.i2c_test;


 r1 = &hb_enable_from_vm_message_struc;
 r2 = 0x4f00;
 r3 = &$hb_enable_cmd_from_vm;
 call $message.register_handler;


 r1 = &hb_disable_from_vm_message_struc;
 r2 = 0x4f01;
 r3 = &$hb_disable_cmd_from_vm;
 call $message.register_handler;


 r1 = &time_value_from_vm_message_struc;
 r2 = 0x4f06;
 r3 = &$time_value_cmd_from_vm;
 call $message.register_handler;


 r1 = &recover_hb_value_from_vm_message_struc;
 r2 = 0x4f09;
 r3 = &$vee.heartrate.hb_recover;
 call $message.register_handler;


 jump $pop_rLink_and_rts;
.linefile 223 "vee_heartrate.asm"
$vee.heartrate.i2c_test:

 push rLink;


 call $i2c.start_bit;


 r0 = 0x90;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;

 r0 = 0x01;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;

 r0 = 0x85;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;

 r0 = 0x83;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;


 call $i2c.stop_bit;


 call $i2c.start_bit;


 r0 = 0x90;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;

 r0 = 0x01;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;


 call $i2c.stop_bit;


 call $i2c.start_bit;


 r0 = 0x91;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump i2c_test_err;

 r0 = 1;
 call $i2c.receive_byte;

 r0 = 1;
 call $i2c.receive_byte;
 NULL = r0 - 0x83;
 if NZ jump i2c_test_err;


 call $i2c.stop_bit;


 r2 = 0x4f24;
 r3 = 0;
 call $message.send_short;


 jump $pop_rLink_and_rts;

i2c_test_err:


 r2 = 0x4f24;
 r3 = 1;
 call $message.send_short;


 jump $pop_rLink_and_rts;
.linefile 328 "vee_heartrate.asm"
$hb_enable_cmd_from_vm:

 push rLink;

 r1 = M[hb_sample_enable];
 NULL = r1 - 1;
 if POS jump $pop_rLink_and_rts;

 r1 = 1;
 M[hb_sample_enable] = r1;


 r1 = &$vee.heartrate.sample_timer_struc;
 r2 = 50*1000;
 r3 = &$vee.heartrate.sample_timer_handler;
 call $timer.schedule_event_in;


 jump $pop_rLink_and_rts;
.linefile 368 "vee_heartrate.asm"
$hb_disable_cmd_from_vm:

 push rLink;

    call $interrupt.block;
 r1 = 0;
 M[hb_sample_enable] = r1;
 M[hb_time_enable] = r1;
    call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 401 "vee_heartrate.asm"
$time_value_cmd_from_vm:

 push rLink;

 r1 = M[hb_time_enable];
 NULL = r1 - 1;
 if POS jump $pop_rLink_and_rts;

 r1 = 1;
 M[hb_time_enable] = r1;


 r1 = &$vee.heartrate.sport_time_timer_struc;
 r2 = 1000*1000;
 r3 = &$vee.heartrate.sport_time_timer_handler;
 call $timer.schedule_event_in;


 jump $pop_rLink_and_rts;
.linefile 440 "vee_heartrate.asm"
$vee.heartrate.sport_time_timer_handler:


 push rLink;

 r0 = M[hb_time_enable];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;

    call $interrupt.block;
 r1 = M[time_value];
 r1 = r1 + 1;
 M[time_value] = r1;
    call $interrupt.unblock;


 r1 = &$vee.heartrate.sport_time_timer_struc;
 r2 = 1000*1000;
 r3 = &$vee.heartrate.sport_time_timer_handler;
 call $timer.schedule_event_in;


 jump $pop_rLink_and_rts;
.linefile 485 "vee_heartrate.asm"
$vee.heartrate.sample_timer_handler:


 push rLink;

 r0 = M[hb_sample_enable];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;


 call $vee.heartratevalue.read;
 NULL = r3;
 if NEG jump vee_heartrate_sample_ret;


 call $vee.heartrate.input;


 r0 = &$vee.heartrate.sample_cbuffer_struc;
 call $cbuffer.calc_amount_data;


 NULL = r0 - 120;
 if NEG jump vee_heartrate_sample_ret;


 call $vee.heartrate.sample_copy;

vee_heartrate_sample_ret:


 r1 = &$vee.heartrate.sample_timer_struc;
 r2 = 50*1000;
 r3 = &$vee.heartrate.sample_timer_handler;
 call $timer.schedule_event_in_period;


 jump $pop_rLink_and_rts;
.linefile 543 "vee_heartrate.asm"
$vee.heartratevalue.read:
 push rLink;



 call $i2c.start_bit;


 r0 = 0x90;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;

 r0 = 0x01;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;

 r0 = 0x85;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;

 r0 = 0x83;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;


 call $i2c.stop_bit;



 call $i2c.start_bit;


 r0 = 0x90;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;

 r0 = 0x0;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;


 call $i2c.stop_bit;



 call $i2c.start_bit;


 r0 = 0x91;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump send_err;

 r0 = 1;
 call $i2c.receive_byte;
 M[value_msb] = r0;

 r0 = 1;
 call $i2c.receive_byte;
 M[value_lsb] = r0;


 call $i2c.stop_bit;

 r1 = M[value_msb];
 r1 = r1 ASHIFT 8;
 r3 = r1 + M[value_lsb];
 r3 = r3 LSHIFT -4;

 NULL = r3 - 0x7ff;
 if POS r3 = 0;


 jump $pop_rLink_and_rts;

send_err:
 r3 = -1;


 jump $pop_rLink_and_rts;
.linefile 648 "vee_heartrate.asm"
$vee.heartrate.sample_copy:

 push rLink;


 r0 = &$vee.heartrate.sample_cbuffer_struc;





 call $cbuffer.get_read_address_and_size;

 I0 = r0;
 L0 = r1;


 r0 = &$vee.heartrate.calc_cbuffer_struc;





 call $cbuffer.get_write_address_and_size;

 I4 = r0;
 L4 = r1;


 r10 = 120;
 do cbuf_copy;
  r1 = M[I0, 1];
  M[I4, 1] = r1;
 cbuf_copy:


 r0 = &$vee.heartrate.calc_cbuffer_struc;
 r1 = I4;
 call $cbuffer.set_write_address;
 L0 = 0;


 r0 = &$vee.heartrate.sample_cbuffer_struc;





 call $cbuffer.get_read_address_and_size;

 I0 = r0;
 L0 = r1;







 r10 = 20;
 do move_read_address;
  r1 = M[I0, 1];



 move_read_address:

 r0 = &$vee.heartrate.sample_cbuffer_struc;
 r1 = I0;
 call $cbuffer.set_read_address;
 L0 = 0;


 jump $pop_rLink_and_rts;
.linefile 743 "vee_heartrate.asm"
$vee.heartrate.calc_cbuffer_copy:

 push rLink;


 r0 = &$vee.heartrate.calc_cbuffer_struc;





 call $cbuffer.get_read_address_and_size;

 I0 = r0;
 L0 = r1;

 I4 = &$vee.heartrate.calc_buff;
 L4 = 0;

    r10 = 120;
 do calc_cbuffer_copy1;
  r1 = M[I0, 1];
  M[I4, 1] = r1;
 calc_cbuffer_copy1:


 r0 = &$vee.heartrate.calc_cbuffer_struc;
    call $cbuffer.empty_buffer;


 jump $pop_rLink_and_rts;
.linefile 795 "vee_heartrate.asm"
$vee.heartrate.input:

 push rLink;


 r0 = &$vee.heartrate.sample_cbuffer_struc;





 call $cbuffer.get_write_address_and_size;

 I0 = r0;
 L0 = r1;

 r0 = r3;
 M[I0, 1] = r0;

 r0 = &$vee.heartrate.sample_cbuffer_struc;
 r1 = I0;
 call $cbuffer.set_write_address;
 L0 = 0;


 jump $pop_rLink_and_rts;
.linefile 843 "vee_heartrate.asm"
$vee.heartrate.average_hb:
 push rLink;


 r2 = 50;
 r0 = r0*r2 (int);

 rMAC = r0 ASHIFT 0 (LO);
 Div = rMAC / r1;
 r1 = DivResult;

 r2 = 60000;
  rMAC = r2 ASHIFT 0 (LO);

 Div = rMAC / r1;
 r0 = DivResult;


 NULL = r0 - 50;
 if NEG r0 = 0;
 NULL = r0 - 150;
 if POS r0 = 0;


 jump $pop_rLink_and_rts;
.linefile 888 "vee_heartrate.asm"
$vee.heartrate.calc:
 push rLink;


 call $interrupt.block;


 r0 = &$vee.heartrate.calc_cbuffer_struc;
 call $cbuffer.calc_amount_data;

 NULL = r0 - 120;
 if NEG jump do_not_calc;


 call $interrupt.unblock;


 call $vee.heartrate.calc_cbuffer_copy;


 r0 = &$vee.heartrate.calc_buff;
    r1 = 120;
 call $vee.heartrate.hb_calc;

 call $vee.heartrate.hb_smooth;

 call $vee.heartrate.output;
.linefile 924 "vee_heartrate.asm"
    r0 = &$vee.heartrate.hb_addr_min;
    r1 = 18;
    call $vee.heartrate.zero_buff;

    r0 = &$vee.heartrate.hb_addr_max;
    r1 = 18;
    call $vee.heartrate.zero_buff;
.linefile 956 "vee_heartrate.asm"
 jump $pop_rLink_and_rts;

do_not_calc:


 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 985 "vee_heartrate.asm"
$vee.heartrate.zero_buff:

 push rLink;

 r10 = r1;
 I0 = r0;
 L0 = 0;
 r0 = 0;
 do vee_zero_buff;
  M[I0, 1] = r0;
 vee_zero_buff:


 jump $pop_rLink_and_rts;
.linefile 1020 "vee_heartrate.asm"
$vee.heartrate.test_output:

 push rLink;


 r3 = 0x4087;
 r4 = r1;
 r5 = r0;
 call $message.send_long;


 jump $pop_rLink_and_rts;
.linefile 1052 "vee_heartrate.asm"
$vee.heartrate.output:
   push rLink;

   r3 = M[heartrate];
   r1 = M[$vee.accelerate.acc_sample_enable];
   NULL = r1 - 1;
   if Z jump sport_data_output;

   r2 = 0x4f20;
   call $message.send_short;

   jump $pop_rLink_and_rts;

sport_data_output:

   r2 = 0x4f21;
   r4 = M[$vee.accelerate.step_value];
   call $interrupt.block;
   r6 = 0;
   M[$vee.accelerate.step_value] = r6;
   call $interrupt.unblock;
   r5 = M[time_value];
   call $message.send_short;

   jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 1102 "vee_heartrate.asm"
.MODULE $vee.heartrate.hb_calc;
 .CODESEGMENT PM;
 .DATASEGMENT DM;
.linefile 1113 "vee_heartrate.asm"
 .CONST HB_MIN_POINT 8;
 .CONST HB_MAX_POINT 25;
 .CONST HB_MAX_MIN_LEN 4;
 .CONST HB_MIN_MAX_LEN 2;
 .CONST HB_POINT_THRESHOLD 4;

 .CONST HB_VALIDE_POINT 5;

 .VAR status = 0;
 .VAR max_flag = 1;
 .VAR min_flag = 1;
 .VAR max_index = 0;
 .VAR min_index = 0;
 .VAR max_value = 0;
 .VAR min_value = 2000;

$vee.heartrate.hb_calc:
 push rLink;


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

 r0 = 0;
 r1 = M[I0, 1];
 r2 = 0;
 r3 = 0;
 r6 = 0;

 r4 = 0;
 r5 = 0;

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
 M[status] = r3;

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
 M[&$vee.heartrate.hb_addr_min + r5] = r4;



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
 M[&$vee.heartrate.hb_addr_min + r5] = r4;



 r5 = r5 + 1;
 M[max_index] = r2;
 M[max_value] = r1;

 jump hb_continue_check;

hb_check_min_value:

 r3 = M[status];
 NULL = r3;
 if Z jump hb_continue_check;

 r3 = 0;
 M[status] = r3;

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
 r1 = r0;
 r10 = r10 - 1;
 NULL = r10;
 if NZ jump hb_calc_loop;


 r0 = 0;
 NULL = r5 - HB_VALIDE_POINT;
 if NEG jump invalide_exit;



 r1 = 0;
 r2 = 0;
 r3 = 0;
 r4 = 0;
 r6 = 0;

hb_outside_loop:
 r0 = 0;
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



 r1 = r1 + 1;
 NULL = r1 - r5;
 if NEG jump hb_outside_loop;


 r1 = 2;
 r3 = r5;
 rMAC = r3 ASHIFT 0 (LO);
 Div = rMAC / r1;
 r0 = DivResult;
 r1 = DivRemainder;
 NULL = r1;
 if NZ r0 = r0 + 1;

 r1 = 0;
 r2 = 0;
 r3 = 0;
 r4 = 0;
 r6 = 0;
 r10 = r5;
 r9 = 0;

calc_valid_period:


 r2 = M[&$vee.heartrate.hb_addr_max + r1];
 r6 = M[&$vee.heartrate.hb_addr_min + r1];
 NULL = r2 - r0;
 if POS jump check_valid_period;
 jump continue_check_period;

check_valid_period:

 r9 = 120;
 rMAC = r9 ASHIFT 0 (LO);
 Div = rMAC / r6;
 r9 = DivResult;

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


 NULL = r3;
 if Z jump invalide_exit;

 r0 = r4;
 r1 = r3;
 call $vee.heartrate.average_hb;

 jump $pop_rLink_and_rts;

invalide_exit:
 r0 = 0;


 jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 1417 "vee_heartrate.asm"
.MODULE $vee.heartrate.hb_smooth;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .CONST HB_SMOOTH_THRESHOLD 4;
 .CONST HB_SMOOTH_BUFF_SIZE 10;
 .CONST HB_YZERO_THRESHOLD 5;
 .CONST HB_SMOOTH_BEFORE 0;
 .CONST HB_SMOOTH_MIDDLE 1;
 .CONST HB_SMOOTH_AFTER 2;
 .CONST HB_SMOOTH_COUNT 3;
 .CONST HB_SMOOTH_FILTER 12;
 .CONST HB_SMOOTH_ORIGIN_SIZE 6;

 .VAR hb_smooth_buff[HB_SMOOTH_BUFF_SIZE];
 .VAR hb_origin_data[HB_SMOOTH_ORIGIN_SIZE];
 .VAR p_smooth_value = 0;
 .VAR p_hb_value = 0;
 .VAR p_yzero = 0;
 .VAR hb_smooth_status = 0;


$vee.heartrate.hb_smooth:
 push rLink;


 r10 = HB_SMOOTH_ORIGIN_SIZE - 1;
 r4 = &$vee.heartrate.hb_smooth.hb_origin_data;
 r1 = r10;
 do add_value_to_origin_buff;
  r5 = r1 - 1;
  r2 = M[r4 + r5];
  M[r4 + r1] = r2;
  r1 = r1 - 1;
 add_value_to_origin_buff:
 M[r4] = r0;

 r1 = M[r4 + 1];
 M[p_hb_value] = r1;

 r1 = M[hb_smooth_status];
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
 r3 = 0;
 r4 = 0;

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

 NULL = r1 - 50;
 if NEG r0 = 0;

 NULL = r1 - 50;
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
 M[r4] = r3;


 r10 = HB_SMOOTH_BUFF_SIZE;
 r0 = 0;
 r1 = 0;
 r2 = 0;
 r3 = 0;
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


 jump $pop_rLink_and_rts;
.linefile 1652 "vee_heartrate.asm"
$vee.heartrate.hb_recover:

 push rLink;

 I0 = &$vee.heartrate.hb_smooth.hb_smooth_buff;
 L0 = 0;
 r10 = HB_SMOOTH_BUFF_SIZE;
 do hb_value_recover;
  M[I0, 1] = r1;
 hb_value_recover:


 jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 1742 "vee_heartrate.asm"
.MODULE $vee.accelerate;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .VAR acc_sample_enable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR acc_sample_disable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR step_value_enable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR neck_protect_enable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR safe_driver_enable_from_vm_message_struc[$message.STRUC_SIZE];
 .VAR const_seat_enable_from_vm_message_struc[$message.STRUC_SIZE];


    .VAR acc_timer_struc[$timer.STRUC_SIZE];
    .VAR acc_read_buf[6];
    .VAR acc_calc_buf[6];
    .VAR acc_angle_buf[6];

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



 .VAR acc_origin_data[3];
.linefile 1801 "vee_heartrate.asm"
$vee.accelerate.start:

 push rLink;

 call $vee.accelerate.i2c_test;


 call $vee.accelerate.init;


 r1 = &acc_sample_enable_from_vm_message_struc;
 r2 = 0x4f02;
 r3 = &$acc_sample_enable_cmd_from_vm;
 call $message.register_handler;


 r1 = &acc_sample_disable_from_vm_message_struc;
 r2 = 0x4f03;
 r3 = &$acc_sample_disable_cmd_from_vm;
 call $message.register_handler;


 r1 = &step_value_enable_from_vm_message_struc;
 r2 = 0x4f05;
 r3 = &$step_value_enable_cmd_from_vm;
 call $message.register_handler;


 r1 = &neck_protect_enable_from_vm_message_struc;
 r2 = 0x4f07;
 r3 = &$neck_protect_enable_cmd_from_vm;
 call $message.register_handler;


 r1 = &safe_driver_enable_from_vm_message_struc;
 r2 = 0x4f08;
 r3 = &$safe_driver_enable_cmd_from_vm;
 call $message.register_handler;


 r1 = &const_seat_enable_from_vm_message_struc;
 r2 = 0x4f0A;
 r3 = &$const_seat_enable_cmd_from_vm;
 call $message.register_handler;


 jump $pop_rLink_and_rts;
.linefile 1868 "vee_heartrate.asm"
$vee.accelerate.i2c_test:

 push rLink;

 r0 = 0x0f;
 call $vee.accelerate.read_reg;
 NULL = r0 - 0x33;
 if NZ jump acc_i2c_error;


 r2 = 0x4f25;
 r3 = 0;
 call $message.send_short;


 jump $pop_rLink_and_rts;

acc_i2c_error:


 r2 = 0x4f25;
 r3 = 1;
 call $message.send_short;


 jump $pop_rLink_and_rts;
.linefile 1915 "vee_heartrate.asm"
$acc_sample_enable_cmd_from_vm:

 push rLink;

 r1 = M[acc_sample_enable];
 NULL = r1 - 1;
 if POS jump $pop_rLink_and_rts;

 call $interrupt.block;
 r1 = 1;
 M[acc_sample_enable] = r1;
 call $interrupt.unblock;


 r1 = &$vee.accelerate.acc_timer_struc;
 r2 = 40*1000;
 r3 = &$vee.accelerate.sample_timer_handler;
 call $timer.schedule_event_in;


 jump $pop_rLink_and_rts;
.linefile 1957 "vee_heartrate.asm"
$acc_sample_disable_cmd_from_vm:

 push rLink;

 call $interrupt.block;
 r1 = 0;
 M[acc_sample_enable] = r1;
 M[step_value_enable] = r1;
 M[neck_protect_enable] = r1;
 M[safe_driver_enable] = r1;
 M[const_seat_enable] = r1;
 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 1993 "vee_heartrate.asm"
$step_value_enable_cmd_from_vm:

 push rLink;

 call $interrupt.block;
 r1 = 1;
 M[step_value_enable] = r1;
 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 2024 "vee_heartrate.asm"
$neck_protect_enable_cmd_from_vm:

 push rLink;

 call $interrupt.block;
 r1 = 1;
 M[neck_protect_enable] = r1;
 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 2056 "vee_heartrate.asm"
$safe_driver_enable_cmd_from_vm:

 push rLink;

 call $interrupt.block;
 r1 = 1;
 M[safe_driver_enable] = r1;
 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 2088 "vee_heartrate.asm"
$const_seat_enable_cmd_from_vm:

 push rLink;

 call $interrupt.block;
 r1 = 1;
 M[const_seat_enable] = r1;
 call $interrupt.unblock;


 jump $pop_rLink_and_rts;
.linefile 2119 "vee_heartrate.asm"
$vee.accelerate.init:

 push rLink;


 r0 = 0x20;
 r1 = 0x27;
 call $vee.accelerate.write_reg;


 r0 = 0x21;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x22;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x23;
 r1 = 0x84;
 call $vee.accelerate.write_reg;


 r0 = 0x24;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x25;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x1f;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x2E;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x30;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x32;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x33;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x38;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x3A;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x3B;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x3C;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 r0 = 0x3D;
 r1 = 0x0;
 call $vee.accelerate.write_reg;


 jump $pop_rLink_and_rts;
.linefile 2225 "vee_heartrate.asm"
$vee.accelerate.read_reg:

 push rLink;

 r7 = r0;


 call $i2c.start_bit;


 r0 = 0x30;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump read_reg_err;


 r0 = r7;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump read_reg_err;


 call $i2c.start_bit;


 r0 = 0x31;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump read_reg_err;


 r0 = 0;
 call $i2c.receive_byte;
 r8 = r0;


 call $i2c.stop_bit;
 r0 = r8;


 jump $pop_rLink_and_rts;

read_reg_err:

 r0 = -1;


 jump $pop_rLink_and_rts;
.linefile 2295 "vee_heartrate.asm"
$vee.accelerate.write_reg:

 push rLink;

 r7 = r0;
 r8 = r1;


 call $i2c.start_bit;


 r0 = 0x30;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump write_reg_err;


 r0 = r7;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump write_reg_err;


 r0 = r8;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump write_reg_err;


 call $i2c.stop_bit;

write_reg_err:


 jump $pop_rLink_and_rts;
.linefile 2350 "vee_heartrate.asm"
$vee.accelerate.sample_timer_handler:

 push rLink;

 r0 = M[acc_sample_enable];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;

 r0 = 0x27;
 call $vee.accelerate.read_reg;

 NULL = r0 ;
 if NEG jump no_validle_data;
 NULL = r0 ;
 if Z jump no_validle_data;


 call $interrupt.block;


 r7 = &$vee.accelerate.acc_read_buf;
 r8 = 6;
 call $vee.accelerate.read;

 r0 = 1;
 M[acc_valid] = r0;

 call $interrupt.unblock;
.linefile 2388 "vee_heartrate.asm"
no_validle_data:


 r1 = &$vee.accelerate.acc_timer_struc;
 r2 = 40*1000;
 r3 = &$vee.accelerate.sample_timer_handler;
 call $timer.schedule_event_in_period;


 jump $pop_rLink_and_rts;
.linefile 2419 "vee_heartrate.asm"
$vee.accelerate.read:

 push rLink;


 call $i2c.start_bit;


 r0 = 0x30;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump acc_read_err;


 r0 = 0xA8;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump acc_read_err;


 call $i2c.start_bit;


 r0 = 0x31;
 call $i2c.send_byte;
 NULL = r0;
 if NZ jump acc_read_err;


 r0 = r7;
 r1 = r8;
 call $vee.accelerate.multiple_receive;


 call $i2c.stop_bit;

acc_read_err:


 jump $pop_rLink_and_rts;
.linefile 2480 "vee_heartrate.asm"
$vee.accelerate.multiple_send:
 push rLink;
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
.linefile 2526 "vee_heartrate.asm"
$vee.accelerate.multiple_receive:
 push rLink;
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
.linefile 2568 "vee_heartrate.asm"
$vee.accelerate.arccos:


 push rLink;

 NULL = r5;
 if Z jump cos_vertical_angle;

 r5 = r5 LSHIFT -1;
 rMAC = 0.5 + r5;
 r0 = rMAC1;

 rMAC = 0.5 - r5;
 rMAC = rMAC ASHIFT -1;

 Div = rMAC / r0;
 r0 = DivResult;
 call $math.sqrt;
 r6 = DivResult;

 r5 = 0x7fffff;
 call $math.atan;
 rMAC = r5*360;
 r0 = rMAC1;
 r1 = rMAC0;
 r1 = r1 LSHIFT -1;

 NULL = r9;
 if NZ r0 = - r0;


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


 jump $pop_rLink_and_rts;
.linefile 2639 "vee_heartrate.asm"
$vee.accelerate.arcsin:


 push rLink;

 NULL = r5;
 if Z jump sin_zero_angle;

 r1 = r5*r5 (frac);
 rMAC = 1.0 - r1;
 r0 = rMAC1;
 call $math.sqrt;

 rMAC = 1.0 - r1;
 rMAC = rMAC ASHIFT -1;
 Div = rMAC / r5;
 r6 = DivResult;

 r5 = 0x7fffff;
 call $math.atan;
 rMAC = r5*360;
 r0 = rMAC1;
 r1 = rMAC0;
 r1 = r1 LSHIFT -1;


 jump $pop_rLink_and_rts;

sin_zero_angle:
 r0 = 0;
 r1 = 0;


 jump $pop_rLink_and_rts;
.linefile 2693 "vee_heartrate.asm"
$vee.accelerate.angle_calc:


 push rLink;

 r0 = &$vee.accelerate.acc_read_buf;
 r1 = 6;


 I0 = r0;
 L0 = 0;
 I4 = &$vee.accelerate.acc_calc_buf;
 L4 = 0;
 r10 = r1;


 r0 = M[acc_valid];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;

 call $interrupt.block;

 do copy_acc_data;
  r1 = M[I0, 1];
  M[I4, 1] = r1;
 copy_acc_data:

 call $interrupt.unblock;


 r1 = M[&$vee.accelerate.acc_calc_buf + 0];
 r2 = M[&$vee.accelerate.acc_calc_buf + 1];
 r2 = r2 LSHIFT 8;
 r0 = r1 + r2;


 call $vee.accelerate.get_acc;
 M[acc_axis_x] = r0;

 r1 = M[&$vee.accelerate.acc_calc_buf + 2];
 r2 = M[&$vee.accelerate.acc_calc_buf + 3];
 r2 = r2 LSHIFT 8;
 r0 = r1 + r2;


 call $vee.accelerate.get_acc;
 M[acc_axis_y] = r0;

 r1 = M[&$vee.accelerate.acc_calc_buf + 4];
 r2 = M[&$vee.accelerate.acc_calc_buf + 5];
 r2 = r2 LSHIFT 8;
 r0 = r1 + r2;


 call $vee.accelerate.get_acc;
 M[acc_axis_z] = r0;

 call $interrupt.block;
 r0 = 0;
 M[acc_valid] = r0;
 call $interrupt.unblock;


 call $vee.accelerate.acc_output;


    r0 = M[step_value_enable];
    NULL = r0;
    if Z jump dont_output_step_value;

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


 jump $pop_rLink_and_rts;
.linefile 2808 "vee_heartrate.asm"
$vee.accelerate.acc_output:

 push rLink;

 I0 = &$vee.accelerate.acc_origin_data;
 L0 = 0;

 r5 = M[acc_axis_x];
 M[I0, 1] = r5;
 r5 = M[acc_axis_y];
 M[I0, 1] = r5;
 r5 = M[acc_axis_z];
 M[I0, 1] = r5;

 r3 = 0x4088;
 r4 = 3;
 r5 = &$vee.accelerate.acc_origin_data;
 call $message.send_long;


 jump $pop_rLink_and_rts;
.linefile 2852 "vee_heartrate.asm"
$vee.accelerate.get_acc:

 push rLink;

 r1 = 0xff0000;
 r2 = r0 AND 0x8000;

 NULL = r2;
 if NZ r0 = r0 OR r1;
 r0 = r0*10 (int);
 rMAC = r0*0.00024414;
 r0 = rMAC1;


 jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 2889 "vee_heartrate.asm"
.MODULE $vee.accelerate.const_seat;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .CONST SEAT_TIME_PERIOD 1000000;
 .CONST SEAT_TIME_THRESHOLD 30;
 .CONST STABLE_HIGH_VALUE 130;
 .CONST STABLE_LOW_VALUE 70;
 .CONST STABLE_STATE_COUNTER 60;
 .CONST GET_UP_MAX_THRESHOLD 140;
 .CONST GET_UP_MIN_THRESHOLD 60;
 .CONST GET_UP_VALUE_THRESHOLD 100;
 .CONST MOVE_STATE_COUNTER 10;
 .CONST MOVE_STATE_HALF_COUNT 5;

 .VAR seat_timer_enable = 0;
 .VAR seat_timer_count = 0;
 .VAR move_timer_count = 0;
 .VAR stable_count = 0;

 .VAR p_value = 0;
 .VAR status = 0;
 .VAR max_flag = 0;
 .VAR max_value = 0;
 .VAR stable_status = 0;

    .VAR seat_timer_struc[$timer.STRUC_SIZE];

$vee.accelerate.const_seat:
 push rLink;


 r3 = 0;
 r1 = M[$vee.accelerate.acc_axis_x];
 r1 = r1*r1 (int);
 r3 = r3 + r1;
 r1 = M[$vee.accelerate.acc_axis_y];
 r1 = r1*r1 (int);
 r3 = r3 + r1;

 r0 = 0;
 r1 = M[p_value];
 r2 = 0;


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

 r0 = M[stable_status];
 NULL = r0;
 if Z jump get_up_continue_check;

 NULL = r3 - r1;
 if POS jump seat_check_max_value;

seat_check_min_value:
 r0 = M[status];
 NULL = r0;
 if NZ jump get_up_continue_check;

 r0 = 1;
 M[status] = r0;

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
 M[status] = r0;

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


 call $interrupt.block;
 r0 = 0;
 M[max_flag] = r0;
 M[seat_timer_count] = r0;
 call $interrupt.unblock;


 r2 = 0x4f22;
 call $message.send_short;


get_up_continue_check:
 M[p_value] = r3;

 jump $pop_rLink_and_rts;

start_seat_timer:

 r0 = M[seat_timer_enable];
 NULL = r0;
 if NZ jump $pop_rLink_and_rts;


 r1 = &$vee.accelerate.const_seat.seat_timer_struc;
 r2 = SEAT_TIME_PERIOD;
 r3 = &$vee.accelerate.const_seat.seat_timer_handler;
 call $timer.schedule_event_in;

 r0 = 1;
 M[seat_timer_enable] = r0;
 M[stable_status] = r0;


 jump $pop_rLink_and_rts;

$vee.accelerate.const_seat.seat_timer_handler:
 push rLink;

 r0 = M[seat_timer_enable];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;

 r0 = M[seat_timer_count];
 r0 = r0 + 1;
 NULL = r0 - SEAT_TIME_THRESHOLD;
 if POS jump send_const_seat_event;
 M[seat_timer_count] = r0;

another_timer_event:


 r1 = &$vee.accelerate.const_seat.seat_timer_struc;
 r2 = SEAT_TIME_PERIOD;
 r3 = &$vee.accelerate.const_seat.seat_timer_handler;
 call $timer.schedule_event_in_period;

 jump $pop_rLink_and_rts;

send_const_seat_event:

 r0 = M[stable_status];
 NULL = r0;
 if Z jump dont_send_seat_event;

 r2 = 0x4f26;
 call $message.send_short;

 r0 = 0;
 M[seat_timer_count] = r0;
 M[seat_timer_enable] = r0;
 M[stable_status] = r0;
 M[stable_count] = r0;


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
.linefile 3145 "vee_heartrate.asm"
.MODULE $vee.accelerate.safe_driver;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .CONST X_AXIS_THRESHOLD 25;
 .CONST Y_AXIS_THRESHOLD 64;
 .CONST Z_AXIS_THRESHOLD 25;

 .VAR nod_action_flag = 0;
 .VAR shake_action_flag = 0;

$vee.accelerate.safe_driver:
 push rLink;

 jump $pop_rLink_and_rts;


 r1 = 0;
 r2 = 0;
 r3 = 0;
 r4 = 0;
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


 r2 = 0x4f27;
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


 r2 = 0x4f28;
 call $message.send_short;


 jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 3236 "vee_heartrate.asm"
.MODULE $vee.accelerate.neck_protect;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .CONST ANGLE_VALUE_THRESHOLD 40;
 .CONST ANGLE_INIT_SIZE 10;
 .CONST NECK_TIME_PERIOD 1000000;
 .CONST NECK_TIME_THRESHOLD 30;

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
 push rLink;

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
 call $vee.accelerate.arcsin;
 M[$vee.accelerate.angle_x_int] = r0;

 r0 = &$vee.accelerate.neck_protect.acc_y_value_buf;
 r1 = ANGLE_INIT_SIZE;
 call $vee.accelerate.necke_protect.acc_average;
 r5 = r0;
 call $vee.accelerate.arcsin;
 M[$vee.accelerate.angle_y_int] = r0;

 r0 = &$vee.accelerate.neck_protect.acc_z_value_buf;
 r1 = ANGLE_INIT_SIZE;
 call $vee.accelerate.necke_protect.acc_average;
 r5 = r0;
 call $vee.accelerate.arccos;
 M[$vee.accelerate.angle_z_int] = r0;


 call $vee.accelerate.acc_output;


 r1 = 0;
 M[acc_count] = r1;

 r1 = M[angle_count];

 NULL = r1 - ANGLE_INIT_SIZE;
 if NEG jump store_angle_value;

 NULL = r1 - ANGLE_INIT_SIZE;
 if Z jump calc_init_angle;


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


 r0 = 0;
 M[neck_timer_enable] = r0;

 jump $pop_rLink_and_rts;

start_timer_count:

 r0 = M[neck_timer_enable];
 NULL = r0;
 if NZ jump $pop_rLink_and_rts;


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


 r0 = M[$vee.accelerate.acc_axis_x];
 M[acc_x_value_buf + r1] = r0;

 r0 = M[$vee.accelerate.acc_axis_y];
 M[acc_y_value_buf + r1] = r0;

 r0 = M[$vee.accelerate.acc_axis_z];
 M[acc_z_value_buf + r1] = r0;

 r1 = r1 + 1;
 M[acc_count] = r1;


 jump $pop_rLink_and_rts;

store_angle_value:


 r0 = M[$vee.accelerate.angle_x_int];
 M[angle_x_init_buf + r1] = r0;
 r0 = M[$vee.accelerate.angle_y_int];
 M[angle_y_init_buf + r1] = r0;
 r0 = M[$vee.accelerate.angle_z_int];
 M[angle_z_init_buf + r1] = r0;

 r1 = r1 + 1;
 M[angle_count] = r1;


 jump $pop_rLink_and_rts;
.linefile 3446 "vee_heartrate.asm"
$vee.accelerate.necke_protect.average_method:
 push rLink;

 I0 = r0;
 L0 = 0;
 r10 = r1;
 r3 = 0;


 do angle_init_loop;
  r2 = M[I0, 1];
  r3 = r3 + r2;
 angle_init_loop:

 rMAC = r3 ASHIFT 0 (LO);
 Div = rMAC / r1;
 r0 = DivResult;


 jump $pop_rLink_and_rts;

$vee.accelerate.necke_protect.acc_average:
 push rLink;

 I0 = r0;
 L0 = 0;
 r10 = r1;
 r3 = 0;
 r4 = 99;
 r5 = -99;


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
 r0 = rMAC0;
 r0 = r0 LSHIFT -1;


 jump $pop_rLink_and_rts;

$vee.accelerate.neck_protect.neck_timer_handler:
 push rLink;


 r0 = M[neck_timer_count];
 r0 = r0 + 1;
 M[neck_timer_count] = r0;


 NULL = r0 - NECK_TIME_THRESHOLD;
 if POS jump send_neck_prompt_event;

 r0 = M[neck_timer_enable];
 NULL = r0;
 if Z jump $pop_rLink_and_rts;


 r1 = &$vee.accelerate.neck_protect.neck_timer_struc;
 r2 = NECK_TIME_PERIOD;
 r3 = &$vee.accelerate.neck_protect.neck_timer_handler;
 call $timer.schedule_event_in_period;


 jump $pop_rLink_and_rts;

send_neck_prompt_event:

 r0 = 0;
 M[neck_timer_count] = r0;
 M[neck_timer_enable] = r0;
 M[acc_count] = r0;
 M[angle_count] = r0;

 r2 = 0x4f23;
 call $message.send_short;


 jump $pop_rLink_and_rts;

.ENDMODULE;
.linefile 3558 "vee_heartrate.asm"
.MODULE $vee.accelerate.step_count;
 .CODESEGMENT PM;
 .DATASEGMENT DM;

 .CONST MAX_VALUE_THRESHOLD 90;
 .CONST MIN_VALUE_THRESHOLD 70;
 .CONST TIME_THRESHOLD 3;
 .CONST VALUE_THRESHOLD 40;

 .VAR p_value = 0;
 .VAR index_curr = 0;
 .VAR status = 0;
 .VAR index_max = 0;
 .VAR max_flag = 1;
 .VAR min_flag = 1;
 .VAR min_value = 0;

$vee.accelerate.step_count:
 push rLink;


 r3 = 0;
 r1 = M[$vee.accelerate.acc_axis_x];
 r1 = r1*r1 (int);
 r3 = r3 + r1;
 r1 = M[$vee.accelerate.acc_axis_y];
 r1 = r1*r1 (int);
 r3 = r3 + r1;

 r1 = M[p_value];
 r2 = M[index_curr];
 r0 = 0;
 r5 = M[index_max];

 NULL = r3 - r1;
 if POS jump check_max_value;

check_min_value:
 r0 = M[status];
 NULL = r0;
 if NZ jump continue_check;

 r0 = 1;
 M[status] = r0;

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
 M[$vee.accelerate.step_value] = r0;
    call $interrupt.unblock;

 r0 = 0;
 M[min_flag] = r0;
 r0 = 1;
 M[max_flag] = r0;
 M[index_max] = r2;

 jump continue_check;


check_max_value:
 r0 = M[status];
 NULL = r0;
 if Z jump continue_check;

 r0 = 0;
 M[status] = r0;

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


 jump $pop_rLink_and_rts;

.ENDMODULE;
