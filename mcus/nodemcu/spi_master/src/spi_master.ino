// Import required libraries
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <DataPacket.h>

volatile SPacketAllSensors clientDataPacket;
SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

// Replace with your network credentials
const char* ssid = "gizitest"; //Replace with your own SSID
const char* password = "Kuba5859!"; //Replace with your own password

const int ledPin = 2;
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
void processor_TurnLightsON(){
  Serial.println("[NODE] LED ON");
  SPI.beginTransaction(spi_settings);
  //send master packet to slave
  EMasterPacketTypes slaveRequest = EMasterPacketTypes::TurnLightsON;
  SPI.transfer((uint8_t)slaveRequest);
  delay(1);
  SPI.endTransaction();
}

void processor_TurnLightsOFF(){
  Serial.println("[NODE] LED OFF");
  SPI.beginTransaction(spi_settings);
  //send master packet to slave
  EMasterPacketTypes slaveRequest = EMasterPacketTypes::TurnLightsOFF;
  SPI.transfer((uint8_t)slaveRequest);
  delay(1);
  SPI.endTransaction();
}

void setup(){

  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // Initialize LittleFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  //Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html");
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/relayOn", HTTP_GET, [](AsyncWebServerRequest *request){
    processor_TurnLightsOFF();
  });

  // Route to set GPIO to LOW
  server.on("/relayOff", HTTP_GET, [](AsyncWebServerRequest *request){
    processor_TurnLightsON();
  });
}

void loop(){
  SPI.begin();
  // Start server
  server.begin();
}