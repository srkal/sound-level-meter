#include "MICROPHONE.h"

MICROPHONE::MICROPHONE() {
    
}

MICROPHONE::~MICROPHONE() {
	
}

double MICROPHONE::evaluateNoise() {
  sum_queue_t q;
  //sound level handling
  if (xQueueReceive(samples_queue, &q, portMAX_DELAY) > 0) {
    double short_RMS = sqrt(double(q.sum_sqr_SPL) / SAMPLES_SHORT);
    double short_SPL_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(short_RMS / MIC_REF_AMPL);

    sum_sqr += q.sum_sqr_weighted;
    samples += SAMPLES_SHORT;

    if (samples >= SAMPLE_RATE * LEQ_PERIOD) {
      double Leq_RMS = sqrt(sum_sqr / samples);
      noise_dB = MIC_OFFSET_DB + MIC_REF_DB + 20 * log10(Leq_RMS / MIC_REF_AMPL);
      sum_sqr = 0;
      samples = 0;
      
      //Serial.printf("%.1f\n", Leq_dB);
      //Serial.printf("%u processing ticks\n", q.proc_ticks);
    }
  }
  return noise_dB;
}