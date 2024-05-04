#include "PANEL.h"

PANEL::PANEL() {
    begin();
}

PANEL::~PANEL() {
	stop();
}

void PANEL::begin()
{
    FastLED.addLeds<WS2812B, LED_STRIP_PIN, GRB>(leds, NUM_LEDS);
    nvs_flash_preferences.begin("panel_settings", false);
    rotationCount = nvs_flash_preferences.getUShort("rotation", 1);
    displayMode = nvs_flash_preferences.getUShort("mode", 1);
    brightness = nvs_flash_preferences.getUShort("brightness", 1);
    noiseLimit = nvs_flash_preferences.getUShort("limit", 70);
    for (int i=0; i<8; i++) history[i] = 0;
}


void PANEL::stop() {
}

void PANEL::redraw(double value) {
  if (!isAnimationActive() && !isNoiseLimitActive() && (value > noiseLimit)) {
    startAnimation(0, 70, 6);
  }

  switch (displayMode) {
    case 1:
      showHistoryOnCanvas(value);
      break;
    case 2:
      showDecibelOnCanvas(value);
      break;
    case 3:
      showAnimationOnCanvas();
      break;
    case 4:
      showNoiseLimitOnCanvas();
      break;
    default:
      showHistoryOnCanvas(value);
  }
  rotateCanvas(rotationCount);
  mapCanvasToLeds();
  shiftLedBrightness();
  FastLED.show();
}


void PANEL::startAnimation(uint8_t type, uint8_t delay, uint8_t repeat) {
  animationSelectedType = type;
  animationDelay = delay;
  animationRepeatCount = repeat;
  animationCurrentFrame = 0;
  animationTotalFrames = 8;
  animationStartedMiliseconds = millis();
  lastDisplayMode = displayMode;
  if (displayMode != 3) lastDisplayMode = displayMode;
  displayMode = 3;
  showAnimationOnCanvas();
}

uint8_t PANEL::isAnimationActive() {
  return (displayMode == 3);
}

uint8_t PANEL::isNoiseLimitActive() {
  return (displayMode == 4);
}

void PANEL::showAnimationOnCanvas() {
  u_int8_t animationNewFrame = (millis() - animationStartedMiliseconds) / animationDelay % animationTotalFrames;
  if ((animationCurrentFrame == animationTotalFrames - 1) && animationNewFrame == 0) {
    animationRepeatCount--;
    if (animationRepeatCount == 0) {
      displayMode = lastDisplayMode;
    }
  }
  animationCurrentFrame = animationNewFrame;
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      canvas[x][y] = paleta[siren[animationCurrentFrame][(x%8)+8*(y%8)]];
    }
  }
}

void PANEL::showDecibelOnCanvas(double value) {
  uint8_t intValue = value;
  if (intValue < 40) intValue = 40;
  if (intValue > 99) intValue = 99;
  uint8_t vLo = intValue % 10;
  uint8_t vHi = intValue / 10;

  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      canvas[x][y] = paleta[basicNumber[vHi][(x%4)+4*(y%8)]];
    }
  }
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 4; x < 8; x++) {
      canvas[x][y] = paleta[basicNumber[vLo][(x%4)+4*(y%8)]];
    }
  }
}

void PANEL::showNoiseLimitOnCanvas() {
  uint8_t vLo = noiseLimit % 10;
  uint8_t vHi = noiseLimit / 10;

  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 4; x++) {
      canvas[x][y] = paleta[basicNumber[vHi][(x%4)+4*(y%8)]];
    }
  }
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 4; x < 8; x++) {
      canvas[x][y] = paleta[basicNumber[vLo][(x%4)+4*(y%8)]];
    }
  }
  for (uint8_t x = 0; x < 8; x++) {
    canvas[x][7] = paleta[15];
  }
}

void PANEL::showHistoryOnCanvas(double value) {
  uint8_t intValue = value;
  if (intValue < 40) intValue = 40;
  if (intValue > 104) intValue = 104;
  for (uint8_t hindex=0; hindex<7; hindex++) {
    history[hindex] = history[hindex+1];
  }
  history[7] = intValue;

  for (uint8_t hindex=0; hindex<8; hindex++) {
    for (uint8_t x = 0; x < 8; x++) {
      canvas[hindex][7-x] = paleta[barGraph[history[hindex]-40][x]];
    }

  }
}

void PANEL::rotateCanvas(u_int16_t count) {
  CRGB temp[NUM_LEDS/8][NUM_LEDS/8];
  for (int rotation = 0; rotation < count; rotation++) {
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        temp[7-y][x] = canvas[x][y];
      }
    }
    for (int x = 0; x < 8; x++) {
      for (int y = 0; y < 8; y++) {
        canvas[x][y] = temp[x][y];
      }
    }
  }
}

void PANEL::shiftLedBrightness() {
  if (brightness > 0) {
    for (uint8_t x = 0; x < NUM_LEDS; x++) {
        leds[x].red = leds[x].red << brightness;
        leds[x].green = leds[x].green << brightness;
        leds[x].blue = leds[x].blue << brightness;
    }
  }
}
  
void PANEL::mapCanvasToLeds() {
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      leds[XY(x,y)] = canvas[x][y];
    }
  }
} 

void PANEL::rotate() {
  rotationCount = (rotationCount+1) % 4;
  nvs_flash_preferences.putUShort("rotation", rotationCount);
}

void PANEL::upBrightness() {
  if (brightness < 3) brightness++;
  nvs_flash_preferences.putUShort("brightness", brightness);
}

void PANEL::downBrightNess() {
  if (brightness > 0) brightness--;
  nvs_flash_preferences.putUShort("brightness", brightness);
}

void PANEL::upNoiseLimit() {
  if (noiseLimit < 90) noiseLimit++;
  nvs_flash_preferences.putUShort("limit", noiseLimit);
}

void PANEL::downNoiseLimit() {
  if (noiseLimit > 50) noiseLimit--;
  nvs_flash_preferences.putUShort("limit", noiseLimit);
}

void PANEL::setDisplayMode(uint8_t mode) {
  displayMode = mode;
  nvs_flash_preferences.putUShort("mode", displayMode);
}

const uint16_t PANEL::XY( uint8_t x, uint8_t y) {
  if( y & 0x01) {
    return (y * 8) + x;
  } else {
    uint8_t reverseX = 7 - x;
    return (y * 8) + reverseX;
  }
}