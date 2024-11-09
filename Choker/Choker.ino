// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Choker ESP32 Sketch
// https://github.com/invpe/Choker/
// Uses NeoPixel, feel free to get rid of it if not needed.
// Compiled with ESP32 core 3.0.5, NAPT enabled from core 3.x.x++
// (c) invpe 2k24
#define ESP_LWIP 1
#define IP_FORWARD 1
#define IP_NAPT 1
#define SNAP_LEN 2500
#include <Preferences.h>
#include <WiFi.h>
#include "SPIFFS.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <lwip/lwip_napt.h>
#include <lwip/ip.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include "esp_wifi.h"
#include "lwip/netif.h"
#include "lwip/etharp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/ip4.h"
#include "lwip/udp.h"
#include "lwip/prot/dns.h"

Preferences preferences;
String strYourWiFi;
String strYourWiFiPass;
String strMyWiFi;
String strMyWiFiPass;
String strNAPT;

void SaveConfig() {
  preferences.begin("config", false);
  preferences.putString("yourap", strYourWiFi);
  preferences.putString("yourpass", strYourWiFiPass);
  preferences.putString("myap", strMyWiFi);
  preferences.putString("mypass", strMyWiFiPass);
  preferences.putString("napt", strNAPT);
  preferences.end();
}
void LoadConfig() {
  preferences.begin("config", false);
  strYourWiFi = preferences.getString("yourap", "");
  strYourWiFiPass = preferences.getString("yourpass", "");
  strMyWiFi = preferences.getString("myap", "");
  strMyWiFiPass = preferences.getString("mypass", "");
  strNAPT = preferences.getString("napt", "");
  preferences.end();
}

//
uint32_t timestamp = millis();
uint32_t microseconds = (unsigned int)(micros() - millis() * 1000);
uint32_t orig_len = 0;
uint32_t incl_len = 0;
uint32_t uiLastSTACheck = 0;

// Color the world
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 27, NEO_GRB + NEO_KHZ800);

// Store original input/output functions
netif_input_fn oldLinkInput = nullptr;
netif_linkoutput_fn oldLinkOutput = nullptr;

void sendPcapPacket(uint32_t len, uint8_t* buf) {

  orig_len = len;
  incl_len = len;
  if (incl_len > SNAP_LEN) {
    incl_len = SNAP_LEN;
    orig_len = SNAP_LEN;
  }

  uint32_t packet_size = incl_len;
  uint8_t packetBuffer[4];

  // Internal
  packetBuffer[0] = (packet_size >> 0) & 0xFF;
  packetBuffer[1] = (packet_size >> 8) & 0xFF;
  packetBuffer[2] = (packet_size >> 16) & 0xFF;
  packetBuffer[3] = (packet_size >> 24) & 0xFF;

  Serial.write(packetBuffer, 4);
  Serial.write(buf, incl_len);
}

// Intercept incoming packets
err_t myLinkInput(struct pbuf* p, struct netif* netif) {
  sendPcapPacket(p->len, (uint8_t*)p->payload);
  return oldLinkInput(p, netif);
}
// Intercept outgoing packets
err_t myLinkOutput(struct netif* netif, struct pbuf* p) {
  sendPcapPacket(p->len, (uint8_t*)p->payload);
  return oldLinkOutput(netif, p);
}

void setup() {
  pixels.begin();
  pixels.setBrightness(255);
  pixels.setPixelColor(0, 0, 0, 0);
  pixels.show();

  LoadConfig();

  // No sense if no AP configured
  if (strMyWiFi == "" || strYourWiFi == "") {

    Serial.begin(115200);
    Serial.println("Time to setup, hit ENTER to start.");

    // Wait for user readiness
    while (!Serial.available()) {
      // Wait for input
    }
    Serial.readString();

    // Wait for user to provide WiFi name
    Serial.println("Your WiFi name (repeat):");
    while (!Serial.available()) {
      // Wait for input
    }
    strYourWiFi = Serial.readString();
    strYourWiFi.trim();

    // Wait for user to provide WiFi password
    Serial.println("Your WiFi pass (repeat):");
    while (!Serial.available()) {
      // Wait for input
    }
    strYourWiFiPass = Serial.readString();
    strYourWiFiPass.trim();

    Serial.println("My WiFi name");
    while (!Serial.available()) {
      // Wait for input
    }
    strMyWiFi = Serial.readString();
    strMyWiFi.trim();

    Serial.println("My WiFi pass");
    while (!Serial.available()) {
      // Wait for input
    }
    strMyWiFiPass = Serial.readString();
    strMyWiFiPass.trim();

    Serial.println("Enable NAPT (yes/no)");
    while (!Serial.available()) {
      // Wait for input
    }
    strNAPT = Serial.readString();
    strNAPT.trim();
   
    SaveConfig();

    Serial.println("Done, rebooting");
    ESP.restart();
  }

  // Normal run
  Serial.begin(1500000);
  WiFi.mode(WIFI_AP_STA);

  // If STA selected, connect.
  if (!strYourWiFi.isEmpty()) {
    WiFi.begin(strYourWiFi.c_str(), strYourWiFiPass.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }

  // AP iface
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.softAPConfig(local_ip, gateway, subnet, (uint32_t)0, dns);
  WiFi.softAP(strMyWiFi.c_str(), strMyWiFiPass.c_str());


  // Turn on NAPT by default and hook us up
  struct netif* netif = netif_list;
  while (netif != nullptr) {

    // Corresponds to AP
    if (netif->num == 2) {
      if (strNAPT == "yes")
        ip_napt_enable_netif(netif, 1);

      oldLinkInput = netif->input;
      oldLinkOutput = netif->linkoutput;
      netif->input = myLinkInput;
      netif->linkoutput = myLinkOutput;
    }
    netif = netif->next;
  }
}

void loop() {
  if (millis() - uiLastSTACheck >= 10000) {
    int numStations = WiFi.softAPgetStationNum();

    if (numStations > 0)
      pixels.setPixelColor(0, 0, 255, 0);
    else
      pixels.setPixelColor(0, 0, 0, 0);

    pixels.show();
    uiLastSTACheck = millis();
  }
}
