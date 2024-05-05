#ifndef __NETWORK_TIME_PROTOCOL_H
#define __NETWORK_TIME_PROTOCOL_H

#include <NTP.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define TIME_REFRESH_MILISECONDS 960000L

WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

void ntpInit() {
  ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)
  ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)
  ntp.updateInterval(TIME_REFRESH_MILISECONDS);
  ntp.begin();
}

void ntpTimeUpdate() {
  if (WiFi.status() == WL_CONNECTED) {
    ntp.update();
  }
}

#endif