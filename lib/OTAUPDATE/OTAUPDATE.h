#ifndef __OTA_UPDATE_H
#define __OTA_UPDATE_H

#include <esp32fota.h>
#include <PANEL.h>

#define FIRMWARE_CHECK_INTERVAL_MS 60*60*1000
#define DEVICE_NAME "sound-level-meter-hw-v1"
#define NVS_KEY_FW_VERSION "fw_version"

extern const char github_root_certs[] asm("_binary_cert_github_cer_start");

esp32FOTA esp32FOTA(DEVICE_NAME, 1);
long lastUpdateCheck = 0;
PANEL display;

void progress_callback(size_t progress, size_t size) {
  //Serial.printf("%.1f\n", (100.0*progress/size));
  display.setFirmwareUpdateProgress(100.0*progress/size);
  if (!display.isFirmwareUpdateActive()) {
    display.setDisplayMode(DISPLAY_MODE_FW_UPDATE);
  }
  display.redraw(0);
}

void finished_callback(int partition, bool restart_after) {
  //store last updated firmware version, numbering use major version only
  display.getPreferences().putUInt(NVS_KEY_FW_VERSION, esp32FOTA.getPayloadVersion());
  display.setDisplayMode(DISPLAY_MODE_PREVIOUS);
  if(restart_after) {
      ESP.restart();
  }
}

void otaInit(PANEL panelInstance) {
  display = panelInstance;
  CryptoMemAsset *MyRootCA = new CryptoMemAsset("Root CA", github_root_certs, strlen(github_root_certs) + 1);
  
  auto cfg = esp32FOTA.getConfig();
  cfg.name          = DEVICE_NAME;
  cfg.manifest_url  = "https://github.com/srkal/sound-level-meter/releases/latest/download/OTAUpdateSettings.json";
  cfg.sem           = SemverClass(display.getPreferences().getUInt(NVS_KEY_FW_VERSION, 1), 0, 0); // major, minor, patch
  cfg.check_sig     = false; // verify signed firmware with rsa public key
  cfg.root_ca       = MyRootCA;
  esp32FOTA.setConfig(cfg);
  esp32FOTA.setProgressCb(progress_callback);
  esp32FOTA.setUpdateFinishedCb(finished_callback);
  esp32FOTA.printConfig();
}

void otaUpdate() {
  if ((millis() - lastUpdateCheck >= FIRMWARE_CHECK_INTERVAL_MS) && (WiFi.status() == WL_CONNECTED)) {
    esp32FOTA.handle();
    lastUpdateCheck = millis();
  }
}

#endif