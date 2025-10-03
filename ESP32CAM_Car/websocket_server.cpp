#include "ESPAsyncWebServer.h"
#include "Arduino.h"
#include "indexhtml.h"
#include "utilsjs.h"
#include "indexjs.h"

extern int gpLb;
extern int gpLf;
extern int gpRb;
extern int gpRf;
extern int gpLed;

void AnalogAct(int nLf, int nLb, int nRf, int nRb);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


String processor(const String& var) {
  if (var == "PLACEHOLDER") {
    return "Hello ESP32";
  }
  return String();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  Serial.println("1");
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  Serial.println("2");
  
  // Safety checks
  if (data == NULL || len == 0 || len > 1024) {
    Serial.println("Invalid data received");
    return;
  }
  
  // Allocate buffer dynamically
  char* buffer = (char*)malloc(len + 1);
  if (buffer == NULL) {
    Serial.println("Memory allocation failed");
    return;
  }
  
  memcpy(buffer, data, len);
  buffer[len] = '\0';
  
  Serial.print("Received message: ");
  Serial.println(buffer);
  
  // Array to store the 6 unsigned integers
  unsigned int values[6] = {0, 0, 0, 0, 0, 0};
  
  // Parse using strtok (more memory efficient)
  char* token = strtok(buffer, ",");
  int valueIndex = 0;
  
  while (token != NULL && valueIndex < 6) {
    values[valueIndex] = (unsigned int)atoi(token);
    valueIndex++;
    token = strtok(NULL, ",");
  }
  
  // Print the parsed values
  Serial.println("Parsed values:");
  for (int i = 0; i < 6; i++) {
    Serial.print("values[");
    Serial.print(i);
    Serial.print("] = ");
    Serial.println(values[i]);
  }

  unsigned int nLf = (int)values[1];
  unsigned int nLb = (int)values[2];
  unsigned int nRf = (int)values[3];
  unsigned int nRb = (int)values[4];
  AnalogAct(nLf, nLb, nRf, nRb);
  analogWrite(gpLed, values[5]); 
  free(buffer);
  buffer = NULL;
}



void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      AnalogAct(0, 0, 0, 0);
      analogWrite(gpLed, 0);
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      AnalogAct(0, 0, 0, 0);
      analogWrite(gpLed, 0);
      break;
  }
}

void initWebServer() {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", index_js, processor);
  });
  server.on("/utils.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", utils_js, processor);
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    AnalogAct(0, 0, 0, 0);
    request->send_P(200, "text/html", "<p>success!</p>", processor);
  });

  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}

void AnalogAct(int nLf, int nLb, int nRf, int nRb) {
  analogWrite(gpLf, nLf); 
  analogWrite(gpLb, nLb);
  analogWrite(gpRf, nRf);
  analogWrite(gpRb, nRb);
}

void cleanupClients() {
  ws.cleanupClients();
}
