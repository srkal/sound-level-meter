#include "MICROPHONE.h"
#include "PANEL.h"

#define DECODE_NEC  // IR Protocol
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

#include <IRremote.hpp> // infrared receiver

PANEL panel = PANEL();
//MICROPHONE mic = MICROPHONE();

void setup() {
  setCpuFrequencyMhz(160);
  Serial.begin(115200);
  delay(100);
  panel.begin();
  IrReceiver.begin(34, DISABLE_LED_FEEDBACK); // PIN 34 is input only
  samples_queue = xQueueCreate(8, sizeof(sum_queue_t));
  xTaskCreate(mic_i2s_reader_task, "Mic I2S Reader", I2S_TASK_STACK, NULL, I2S_TASK_PRI, NULL);
}


uint32_t Leq_samples = 0;
double Leq_sum_sqr = 0;
double Leq_dB = 0;

void loop() {
  sum_queue_t q;
  //sound level handling
  if (panel.isAnimationActive()) {
    panel.redraw(Leq_dB);
  }else {
    if (xQueueReceive(samples_queue, &q, portMAX_DELAY) > 0) {
      double short_RMS = sqrt(double(q.sum_sqr_SPL) / SAMPLES_SHORT);
      double short_SPL_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(short_RMS / MIC_REF_AMPL);

      Leq_sum_sqr += q.sum_sqr_weighted;
      Leq_samples += SAMPLES_SHORT;

      if (Leq_samples >= SAMPLE_RATE * LEQ_PERIOD) {
        double Leq_RMS = sqrt(Leq_sum_sqr / Leq_samples);
        Leq_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(Leq_RMS / MIC_REF_AMPL);
        Leq_sum_sqr = 0;
        Leq_samples = 0;
        
        Serial.printf("%.1f\n", Leq_dB);
        panel.redraw(Leq_dB);
        //Serial.printf("%u processing ticks\n", q.proc_ticks);
      }
    }
  }
  //Serial.printf("--- %.1f\n", mic.evaluateNoise());
 

  //IR decoder handling
  if (IrReceiver.decode()) {
      //IrReceiver.printIRResultShort(&Serial);
      IrReceiver.resume(); // Enable receiving of the next value

      if (IrReceiver.decodedIRData.command == 0x10) {
          panel.rotate();
      } else if (IrReceiver.decodedIRData.command == 0x11) {
          panel.setDisplayMode(1);
      } else if (IrReceiver.decodedIRData.command == 0x12) {
          panel.setDisplayMode(2);
      } else if (IrReceiver.decodedIRData.command == 0x14) {
          panel.setDisplayMode(4);
      } else if (IrReceiver.decodedIRData.command == 0x16) {
          panel.upBrightness();
      } else if (IrReceiver.decodedIRData.command == 0x19) {
          panel.downBrightNess();
      } else if (IrReceiver.decodedIRData.command == 0x13) {
          panel.startAnimation(0, 100, 3);
      }

      if (panel.isNoiseLimitActive()) {
        if (IrReceiver.decodedIRData.command == 0x00) {
          panel.upNoiseLimit();
        } else if (IrReceiver.decodedIRData.command == 0x01) {
          panel.downNoiseLimit();
        }
      }
  }
}