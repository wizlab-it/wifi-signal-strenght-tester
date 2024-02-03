/**
 * @package WiFi Signal Strenght Tester with OLED display
 * @author WizLab.it
 * @version 20240203.007
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "WiFiSignalStrengthTester.h"


/**
 * System Parameters
 */

//OLED
#define OLED_ADDRESS 0x3C // OLED display I2C Address
#define OLED_WIDTH 128    // OLED display width, in pixels
#define OLED_HEIGHT 32    // OLED display height, in pixels

//HTTP
#define HTTP_PUBLICIP_ENDPOINT "http://ip1.dynupdate.no-ip.com"


/**
 * Global Variables
 */
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);


//===================================================================================


/**
 * Setup
 */
void setup() {
  Serial.begin(115200);
  Serial.print("\n\n\n============================================================================================================\n");

  //Init OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 init failed");
    for(;;);
  }
  display.clearDisplay();
  delay(250);
  display.cp437(true); // Use full 256 char 'Code Page 437'
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 8);
  display.print("Wait...");
  display.display();
  display.setTextSize(1);
}


/**
 * Loop
 */
void loop() {
  Serial.print("------------------------------------------------------------------------------------------------------------\n");

  //WiFi connection
  bool wifiStatus = wifiConnect();

  //If connected to WiFi, show signal strenght data
  if(wifiStatus) {
    uint8_t signalLevel = getSignalLevel();
    Serial.printf("RSSI: %d (%ld dBm)\n", signalLevel, WiFi.RSSI());

    //Print RSSI and Signal Bar on OLED
    display.fillRect(0, 24, OLED_WIDTH, OLED_HEIGHT, BLACK);
    display.setCursor(0, 24);
    display.printf("RSSI: %ld dBm", WiFi.RSSI());
    for(uint8_t i=0; i<signalLevel; i++) {
      display.fillRect((OLED_WIDTH - (4 * (i + 1))), 24, 3, 8, WHITE);
    }
    display.display();
  }

  delay(1000); //1 second delay
}


//===================================================================================


/**
 * WiFi Connection
 */
bool wifiConnect() {
  //Check if already connected to WiFi
  if(WiFi.status() == WL_CONNECTED) return true;

  //If not connected, then try to connect
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ACCESSPOINT_SSID, WIFI_ACCESSPOINT_PASSWORD);
  Serial.printf("Connecting to %s: ", WIFI_ACCESSPOINT_SSID);
  uint8_t i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if(i++ > WIFI_CONNECTION_TIMEOUT) break;
    delay(1000);
  }

  //Check if connection was successful
  if(WiFi.status() == WL_CONNECTED) {
    Serial.printf(" OK\n");
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("Local IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Public IP Address: %s\n", getPublicIPAddress().c_str());
    Serial.print("============================================================================================================\n");

    //Print network info on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("%s", WiFi.SSID().c_str());
    display.fillRect(0, 8, OLED_WIDTH, OLED_HEIGHT, BLACK);
    display.setCursor(0, 8);
    display.printf("Pri: %s", WiFi.localIP().toString().c_str());
    display.fillRect(0, 16, OLED_WIDTH, OLED_HEIGHT, BLACK);
    display.setCursor(0, 16);
    display.printf("Pub: %s", getPublicIPAddress().c_str());
    display.fillRect(0, 24, OLED_WIDTH, OLED_HEIGHT, BLACK);
    display.display();

    return true;
  } else {
    Serial.println(" FAILED");
    delay(1000);
  }

  //If here, connection failed
  return false;
}


/**
 * Get Public IP Address
 */
String getPublicIPAddress() {
  WiFiClient wifiClient;
  HTTPClient httpClient;
  String ipAddress = "n.a";

  httpClient.begin(wifiClient, HTTP_PUBLICIP_ENDPOINT);
  int httpCode = httpClient.GET();
  if(httpCode == HTTP_CODE_OK) {
    ipAddress = httpClient.getString();
  }
  httpClient.end();

  return ipAddress;
}

uint8_t getSignalLevel() {
  if(WiFi.status() != WL_CONNECTED) return 0;
  long rssi = WiFi.RSSI();
  if(rssi > -50) return 5;
  if(rssi > -60) return 4;
  if(rssi > -70) return 3;
  if(rssi > -80) return 2;
  if(rssi > -90) return 1;
  return 0;
}