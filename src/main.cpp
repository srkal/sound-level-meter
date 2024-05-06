#include "MICROPHONE.h"
#include "PANEL.h"
#include "REMOTECONTROL.h"
#include <WiFiManager.h>
#include "NETWORKTIME.h"
#include "OTAUPDATE.h"

PANEL panel = PANEL();
double noise = 0;
WiFiManager wifiManager;

void setup() {
  setCpuFrequencyMhz(160);
  Serial.begin(115200);
  delay(100);
  panel.begin();
  IrReceiver.begin(34, DISABLE_LED_FEEDBACK); // PIN 34 is input only
  samples_queue = xQueueCreate(8, sizeof(sum_queue_t));
  xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect(); // auto generated AP name from chipid, no password
  ntpInit();
  otaInit(panel);
}

void loop() {
  if (panel.isFirmwareUpdateActive()) {
    return;
  }

  wifiManager.process();
  ntpTimeUpdate();
  otaUpdate();

  if (panel.isAnimationActive()) {
    panel.redraw(noise);
  } else {
    noise = getNoiseValue();
    if (noise > 0.0){ 
      panel.redraw(noise);
    }
  }

  //IR decoder handling
  if (IrReceiver.decode()) {
    //IrReceiver.printIRResultShort(&Serial);
    IrReceiver.resume(); // Enable receiving of the next value

    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      if (checkIRCommand(SAMSUNG, IR_SAMSUNG_PRE_CH) || checkIRCommand(NEC, IR_NEC_NUMBER_0)) {
        panel.rotate();
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_NUMBER_1) || checkIRCommand(NEC, IR_NEC_NUMBER_1)) {
        panel.setDisplayMode(DISPLAY_MODE_BARS);
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_NUMBER_2) || checkIRCommand(NEC, IR_NEC_NUMBER_2)) {
        panel.setDisplayMode(DISPLAY_MODE_NUMBERS);
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_SETTINGS) || checkIRCommand(NEC, IR_NEC_POUND)) {
        panel.setDisplayMode(DISPLAY_MODE_SETTINGS);  //limit settings
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_CHANNEL_UP) || checkIRCommand(NEC, IR_NEC_UP)) {
        panel.upBrightness();
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_CHANNEL_DOWN) || checkIRCommand(NEC, IR_NEC_DOWN)) {
        panel.downBrightNess();
      } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_PRIME_VIDEO) || checkIRCommand(NEC, IR_NEC_STAR)) {
        panel.startAnimation(0, 70, 6);  //test animation
      }

      if (panel.isNoiseLimitActive()) {
        if (checkIRCommand(SAMSUNG, IR_SAMSUNG_ARROW_UP) || checkIRCommand(NEC, IR_NEC_RIGHT)) {
          panel.upNoiseLimit();
        } else if (checkIRCommand(SAMSUNG, IR_SAMSUNG_ARROW_DOWN) || checkIRCommand(NEC, IR_NEC_LEFT)) {
          panel.downNoiseLimit();
        }
      }
    }
  }
}
