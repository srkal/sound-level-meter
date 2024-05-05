#ifndef __OTA_UPDATE_H
#define __OTA_UPDATE_H

#include <esp32fota.h>
#include <PANEL.h>

#define FIRMWARE_CHECK_INTERVAL_MS 60*60*1000
#define DEVICE_NAME "sound-level-meter-hw-v1"
#define NVS_KEY_FW_VERSION "fw_version"

const char* root_ca = R"ROOT_CA(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)ROOT_CA";

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
  CryptoMemAsset  *MyRootCA = new CryptoMemAsset("Root CA", root_ca, strlen(root_ca)+1);
  
  auto cfg = esp32FOTA.getConfig();
  cfg.name          = DEVICE_NAME;
  cfg.manifest_url  = "https://kcd.cloud/OTAUpdateSettings.json";
  cfg.sem           = SemverClass(display.getPreferences().getUInt(NVS_KEY_FW_VERSION, 1), 0, 0); // major, minor, patch
  cfg.check_sig     = false; // verify signed firmware with rsa public key
  cfg.unsafe        = true; // disable certificate check when using TLS
  cfg.root_ca       = MyRootCA;
  //cfg.pub_key       = MyRSAKey;
  //cfg.use_device_id = false;
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