#ifndef __REMOTECONTROL_H
#define __REMOTECONTROL_H
#include <IRremote.hpp>

#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

//Constants for chinese IR Remote control BN59
#define DECODE_SAMSUNG  // IR Protocol
#define IR_SAMSUNG_STAND_BY 0x2
#define IR_SAMSUNG_SOURCE 0x1
#define IR_SAMSUNG_NUMBER_1 0x4
#define IR_SAMSUNG_NUMBER_2 0x5
#define IR_SAMSUNG_NUMBER_3 0x6
#define IR_SAMSUNG_NUMBER_4 0x8
#define IR_SAMSUNG_NUMBER_5 0x9
#define IR_SAMSUNG_NUMBER_6 0xA
#define IR_SAMSUNG_NUMBER_7 0xC
#define IR_SAMSUNG_NUMBER_8 0xD
#define IR_SAMSUNG_NUMBER_9 0xE
#define IR_SAMSUNG_NUMBER_0 0x11
#define IR_SAMSUNG_TTX_MIX 0x2C
#define IR_SAMSUNG_PRE_CH 0x13
#define IR_SAMSUNG_VOLUME_UP 0x7
#define IR_SAMSUNG_VOLUME_DOWN 0xB
#define IR_SAMSUNG_CHANNEL_UP 0x12
#define IR_SAMSUNG_CHANNEL_DOWN 0x10
#define IR_SAMSUNG_MUTE 0xF
#define IR_SAMSUNG_CH_LIST 0x6B
#define IR_SAMSUNG_HOME_HOUSE 0x79
#define IR_SAMSUNG_NETFLIX 0xF3
#define IR_SAMSUNG_ROKUTEN_TV 0xBC
#define IR_SAMSUNG_PRIME_VIDEO 0xF4
#define IR_SAMSUNG_GUIDE 0x4F
#define IR_SAMSUNG_ARROW_UP 0x60
#define IR_SAMSUNG_ARROW_LEFT 0x65
#define IR_SAMSUNG_ARROW_RIGHT 0x62
#define IR_SAMSUNG_ARROW_DOWN 0x61
#define IR_SAMSUNG_ARROW_CENTER 0x68
#define IR_SAMSUNG_RETURN 0x58
#define IR_SAMSUNG_EXIT 0x2D
#define IR_SAMSUNG_RED_A 0x6C
#define IR_SAMSUNG_GREEN_B 0x14
#define IR_SAMSUNG_YELLOW_C 0x15
#define IR_SAMSUNG_BLUE_D 0x16
#define IR_SAMSUNG_SETTINGS 0x1A
#define IR_SAMSUNG_INFO 0x1F
#define IR_SAMSUNG_AD_SUBT 0x25
#define IR_SAMSUNG_STOP_RECTANGLE 0x46
#define IR_SAMSUNG_FAST_BACKWARD 0x45
#define IR_SAMSUNG_PLAY_TRIANGLE 0x47
#define IR_SAMSUNG_PAUSE 0x4A
#define IR_SAMSUNG_FAST_FORWARD 0x48

// Constants for simple chinese remote
#define DECODE_NEC  // IR Protocol
#define IR_NEC_NUMBER_1 0x45
#define IR_NEC_NUMBER_2 0x46
#define IR_NEC_NUMBER_3 0x47
#define IR_NEC_NUMBER_4 0x44
#define IR_NEC_NUMBER_5 0x40
#define IR_NEC_NUMBER_6 0x43
#define IR_NEC_NUMBER_7 0x7
#define IR_NEC_NUMBER_8 0x15
#define IR_NEC_NUMBER_9 0x9
#define IR_NEC_NUMBER_0 0x19
#define IR_NEC_STAR 0x16
#define IR_NEC_POUND 0xD
#define IR_NEC_UP 0x18
#define IR_NEC_DOWN 0x52
#define IR_NEC_LEFT 0x8
#define IR_NEC_RIGHT 0x5A
#define IR_NEC_OK 0x1C

bool checkIRCommand(decode_type_t protocol, uint16_t command) {
  return IrReceiver.decodedIRData.protocol == protocol && IrReceiver.decodedIRData.command == command;
}

#endif