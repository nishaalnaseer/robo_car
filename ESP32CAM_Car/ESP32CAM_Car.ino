
/*
 * @Date: 2023-8-16 
 * @Description: ESP32 Camera Surveillance Car
 * @FilePath: 
 */

#include "esp_camera.h"
#include <WiFi.h>
#include "config.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;


// GPIO Setting
extern int gpLb =  2; // Left 1
extern int gpLf = 14; // Left 2
extern int gpRb = 15; // Right 1
extern int gpRf = 13; // Right 2
extern int gpLed =  4; // Light
extern String WiFiAddr ="";

void startCameraServer();
void initWebSocket();

void setupPWM() {
  ledcAttach(gpLf, 5000, 8);
  ledcAttach(gpLb, 5000, 8);
  ledcAttach(gpRf, 5000, 8);
  ledcAttach(gpRb, 5000, 8);
  ledcAttach(gpLed, 5000, 8);
}


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  setupPWM();
  
  ledcWrite(gpLed, 0);
  ledcWrite(gpLf, 10);
}

void loop() 
{
  
}
