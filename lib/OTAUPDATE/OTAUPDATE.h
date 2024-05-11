#ifndef __OTA_UPDATE_H
#define __OTA_UPDATE_H

#include <esp32fota.h>
#include <PANEL.h>

#define FIRMWARE_CHECK_INTERVAL_MS 60*60*1000

extern const char github_root_certs[] asm("_binary_cert_github_cer_start");
char* settingsUrl PROGMEM = "https://github.com/srkal/sound-level-meter/releases/latest/download/OTAUpdateSettings.json";
char* deviceName PROGMEM = "sound-level-meter-hw-v1";


esp32FOTA ota(deviceName, 1);
long lastUpdateCheck = 0;

void progress_callback(size_t progress, size_t size) {
  //Serial.printf("%.1f\n", (100.0*progress/size));
  panel.setFirmwareUpdateProgress(100.0*progress/size);
  if (!panel.isFirmwareUpdateActive()) {
    panel.setDisplayMode(DISPLAY_MODE_FW_UPDATE);
  }
  panel.redraw(0);
}

void finished_callback(int partition, bool restart_after) {
  //store last updated firmware version, numbering use major version only
  panel.getPreferences().putUInt(NVS_KEY_FW_VERSION, ota.getPayloadVersion());
  panel.setDisplayMode(DISPLAY_MODE_PREVIOUS);
  if(restart_after) {
      ESP.restart();
  }
}

void otaInit() {
  CryptoMemAsset *MyRootCA = new CryptoMemAsset("Root CA", github_root_certs, strlen(github_root_certs) + 1);
  
  auto cfg = ota.getConfig();
  cfg.name          = deviceName;
  cfg.manifest_url  = settingsUrl;
  cfg.sem           = SemverClass(panel.getPreferences().getUInt(NVS_KEY_FW_VERSION, 1), 0, 0); // major, minor, patch
  cfg.check_sig     = false; // verify signed firmware with rsa public key
  cfg.root_ca       = MyRootCA;
  ota.setConfig(cfg);
  ota.setStreamTimeout(15000);
  ota.setProgressCb(progress_callback);
  ota.setUpdateFinishedCb(finished_callback);
  ota.printConfig();
}

void otaUpdate() {
  if ((millis() - lastUpdateCheck >= FIRMWARE_CHECK_INTERVAL_MS) && (WiFi.status() == WL_CONNECTED)) {
    ota.handle();
    lastUpdateCheck = millis();
  }
}

#endif