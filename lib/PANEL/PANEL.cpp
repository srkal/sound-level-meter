#include "PANEL.h"

PANEL::PANEL() {
}

PANEL::~PANEL() {
	stop();
}

void PANEL::begin()
{
    optionalInit();
    FastLED.addLeds<WS2812B, LED_STRIP_PIN, GRB>(leds, NUM_LEDS);
    nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
    rotationCount = nvs_flash_preferences.getUShort(NVS_KEY_ROTATION, 1);
    displayMode = nvs_flash_preferences.getUShort(NVS_KEY_MODE, DISPLAY_MODE_BARS);
    brightness = nvs_flash_preferences.getUShort(NVS_KEY_BRIGHTNESS, 1);
    noiseLimit = nvs_flash_preferences.getUShort(NVS_KEY_LIMIT, 70);
    chipId = String(WIFI_getChipId(),HEX);
    for (int i=0; i<8; i++) history[i] = 0;
}


void PANEL::stop() {
}

void PANEL::redraw(double value) {
  double averageFromHistory = historyAverage(VALUES_COUNT_FOR_AVERAGE, value);
  if (!isAnimationActive() && !isNoiseLimitActive() && (averageFromHistory > noiseLimit)) {
    startAnimation(0, 70, 6);
    sendDataToGoogleSheet(averageFromHistory, nvs_flash_preferences.getUShort(NVS_KEY_FW_VERSION, 1));
  }
  switch (displayMode) {
    case DISPLAY_MODE_BARS:
      showHistoryOnCanvas(value);
      break;
    case DISPLAY_MODE_NUMBERS:
      showDecibelOnCanvas(value);
      break;
    case DISPLAY_MODE_ANIMATION:
      showAnimationOnCanvas();
      break;
    case DISPLAY_MODE_SETTINGS:
      showNoiseLimitOnCanvas();
      break;
    case DISPLAY_MODE_FW_UPDATE:
      showFirmwareUpdateProgressOnCanvas();
      break;
    default:
      showHistoryOnCanvas(value);
  }
  showWifiErrorOnCanvas();
  rotateCanvas(rotationCount);
  mapCanvasToLeds();
  shiftLedBrightness();
  FastLED.show();
}

void PANEL::setFirmwareUpdateProgress(uint8_t percent) {
  firmwareUpdateProgress = percent;
}

void PANEL::startAnimation(uint8_t type, uint8_t delay, uint8_t repeat) {
  animationSelectedType = type;
  animationDelay = delay;
  animationRepeatCount = repeat;
  animationCurrentFrame = 0;
  animationTotalFrames = 8;
  animationStartedMiliseconds = millis();
  lastDisplayMode = displayMode;
  if (displayMode != DISPLAY_MODE_ANIMATION) lastDisplayMode = displayMode;
  displayMode = DISPLAY_MODE_ANIMATION;
  showAnimationOnCanvas();
}

uint8_t PANEL::isAnimationActive() {
  return (displayMode == DISPLAY_MODE_ANIMATION);
}

uint8_t PANEL::isNoiseLimitActive() {
  return (displayMode == DISPLAY_MODE_SETTINGS);
}

uint8_t PANEL::isFirmwareUpdateActive() {
  return (displayMode == DISPLAY_MODE_FW_UPDATE);
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

void PANEL::showFirmwareUpdateProgressOnCanvas() {
  uint8_t intValue = firmwareUpdateProgress;
  if (intValue < 0) intValue = 0;
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
  
void PANEL::showWifiErrorOnCanvas() {
  if (WiFi.status() != WL_CONNECTED) {
    leds[XY(0,0)] = paleta[9];
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
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_ROTATION, rotationCount);
}

void PANEL::upBrightness() {
  if (brightness < 3) brightness++;
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_BRIGHTNESS, brightness);
}

void PANEL::downBrightNess() {
  if (brightness > 0) brightness--;
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_BRIGHTNESS, brightness);
}

void PANEL::upNoiseLimit() {
  if (noiseLimit < 90) noiseLimit++;
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_LIMIT, noiseLimit);
}

void PANEL::downNoiseLimit() {
  if (noiseLimit > 50) noiseLimit--;
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_LIMIT, noiseLimit);
}

void PANEL::setDisplayMode(uint8_t mode) {
  if ((mode == DISPLAY_MODE_PREVIOUS) and (lastDisplayMode > DISPLAY_MODE_PREVIOUS)) {
    displayMode = lastDisplayMode;
  } else {
    lastDisplayMode = displayMode;
    displayMode = mode;
  }
  nvs_flash_preferences.end();
  nvs_flash_preferences.begin(NVS_GROUP_NAME, false);
  nvs_flash_preferences.putUShort(NVS_KEY_MODE, displayMode);
}

const uint16_t PANEL::XY( uint8_t x, uint8_t y) {
  if( y & 0x01) {
    return (y * 8) + x;
  } else {
    uint8_t reverseX = 7 - x;
    return (y * 8) + reverseX;
  }
}

Preferences PANEL::getPreferences() {
  return nvs_flash_preferences;
}

void PANEL::sendDataToGoogleSheet(float decibels, uint8_t version) {
  HTTPClient http;
  http.setTimeout(15000);

  if (WiFi.status() == WL_CONNECTED) {
    String serverPath = String(webAppUrl) + "?cpuid=" + String(WIFI_getChipId(),HEX) 
      + "&decibels=" + String(decibels) + "&fwversion=" + String(version) + "&localtime=" + ntp->formattedTime("%d.%m.%Y+%H:%M:%S");

    if (http.begin(serverPath.c_str())) {
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.print("Server response code: ");
        Serial.println(httpCode);
      } else {
        Serial.print("HTTP GET request failed with error code: ");
        Serial.println(httpCode);
      }
      http.end();
    } else {
      Serial.println("Unable to connect to the server");
    }
  }
}

void PANEL::setNtp(NTP *ntpObject) {
  ntp = ntpObject;
}

double PANEL::historyAverage(int count, double newValue) {
  double sum = newValue;
  for (int i=7; i>7-count; i--) {
    sum += history[i];
  }
  return sum / (count+1);
}

void PANEL::optionalInit() {
  #ifdef CLEAN_NVS
  nvs_flash_erase();
  nvs_flash_init();
  #endif
}

