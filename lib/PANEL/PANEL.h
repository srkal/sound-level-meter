#ifndef __PANEL_H
#define __PANEL_H

#include <FastLED.h>
#include "picdata.h"
#include "preferences.h"

#define NUM_LEDS 64
#define LED_STRIP_PIN 2

#define DISPLAY_MODE_PREVIOUS 0
#define DISPLAY_MODE_BARS 1
#define DISPLAY_MODE_NUMBERS 2
#define DISPLAY_MODE_ANIMATION 3
#define DISPLAY_MODE_SETTINGS 4
#define DISPLAY_MODE_FW_UPDATE 5

#define NVS_GROUP_NAME "panel_settings"
#define NVS_KEY_BRIGHTNESS "brightness"
#define NVS_KEY_LIMIT "limit"
#define NVS_KEY_ROTATION "rotation"
#define NVS_KEY_MODE "mode"


class PANEL {
  public:
    PANEL();
    ~PANEL();
    void begin();
    void stop();
    void redraw(double value);
    void rotate();
    void setDisplayMode(uint8_t mode);
    void upBrightness();
    void downBrightNess();
    void upNoiseLimit();
    void downNoiseLimit();
    void startAnimation(uint8_t type, uint8_t delay, uint8_t repeat);
    void showAnimationOnCanvas();
    uint8_t isAnimationActive();
    uint8_t isNoiseLimitActive();
    uint8_t isFirmwareUpdateActive();
    void setFirmwareUpdateProgress(uint8_t percent);
    Preferences getPreferences();

  private:
    CRGBArray<NUM_LEDS> leds;
    CRGB canvas[NUM_LEDS/8][NUM_LEDS/8];
    uint8_t rotationCount;
    uint8_t displayMode;
    uint8_t lastDisplayMode;
    uint8_t history[8];
    uint8_t brightness;
    uint8_t noiseLimit;
    uint8_t animationSelectedType;
    uint8_t animationCurrentFrame;
    uint8_t animationTotalFrames;
    uint8_t animationDelay;
    uint8_t animationRepeatCount;
    uint8_t firmwareUpdateProgress;
    long animationStartedMiliseconds;
    Preferences nvs_flash_preferences;

    void showDecibelOnCanvas(double value);
    void showHistoryOnCanvas(double value);
    void showNoiseLimitOnCanvas();
    void showFirmwareUpdateProgressOnCanvas();
    void rotateCanvas(u_int16_t count);
    void shiftLedBrightness();
    void mapCanvasToLeds();
    const uint16_t XY( uint8_t x, uint8_t y);
};
#endif