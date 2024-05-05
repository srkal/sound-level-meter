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

      if (IrReceiver.decodedIRData.command == BUTTON_PRE_CH) {
          panel.rotate();
      } else if (IrReceiver.decodedIRData.command == BUTTON_NUMBER_1) { 
          panel.setDisplayMode(DISPLAY_MODE_BARS);
      } else if (IrReceiver.decodedIRData.command == BUTTON_NUMBER_2) {
          panel.setDisplayMode(DISPLAY_MODE_NUMBERS);
      } else if (IrReceiver.decodedIRData.command == BUTTON_SETTINGS) {
          panel.setDisplayMode(DISPLAY_MODE_SETTINGS);  //limit settings
      } else if (IrReceiver.decodedIRData.command == BUTTON_CHANNEL_UP) {
          panel.upBrightness();
      } else if (IrReceiver.decodedIRData.command == BUTTON_CHANNEL_DOWN) {
          panel.downBrightNess();
      } else if (IrReceiver.decodedIRData.command == BUTTON_PRIME_VIDEO) {
          panel.startAnimation(0, 70, 6);  //test animation
      }

      if (panel.isNoiseLimitActive()) {
        if (IrReceiver.decodedIRData.command == BUTTON_ARROW_UP) {
          panel.upNoiseLimit();
        } else if (IrReceiver.decodedIRData.command == BUTTON_ARROW_DOWN) {
          panel.downNoiseLimit();
        }
      }
  }
}

