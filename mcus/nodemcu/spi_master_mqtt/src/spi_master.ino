// Import required libraries
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <DataPacket.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>

volatile SPacketAllSensors clientDataPacket;
SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

// Replace with your network credentials
const char* ssid = "gizitest"; //Replace with your own SSID
const char* password = "Kuba5859!"; //Replace with your own password
// MQTT
const char* mqtt_server = "192.168.0.136";
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

void mqtt_reconnect()
{
  Serial.print("Attempting MQTT connection...");
  if (mqtt_client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      //mqtt_client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
}
const int ledPin = 2;
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

void init_clientDataPacket()
{
  clientDataPacket.error = 0;
  clientDataPacket.humidiySensor = 0;
  clientDataPacket.lightSensor = 0;
  clientDataPacket.roomLightSwitchState = 0;
  clientDataPacket.temperatureSensor = 0;
  clientDataPacket.rtcDateTime.year = 0;
  clientDataPacket.rtcDateTime.month = 0;
  clientDataPacket.rtcDateTime.day = 0;
  clientDataPacket.rtcDateTime.dayOfWeek = 0;
  clientDataPacket.rtcDateTime.unixtime = 0;
  clientDataPacket.rtcDateTime.hour = 0;
  clientDataPacket.rtcDateTime.minute = 0;
  clientDataPacket.rtcDateTime.second = 0;
}

String rtc_datetime_to_str()
{
  String result = String();
  result.concat(clientDataPacket.rtcDateTime.year);
  result.concat("-");
  result.concat(clientDataPacket.rtcDateTime.month);
  result.concat("-");
  result.concat(clientDataPacket.rtcDateTime.day);
  result.concat("           ");
  result.concat(clientDataPacket.rtcDateTime.hour);
  result.concat(":");
  result.concat(clientDataPacket.rtcDateTime.minute);
  result.concat(":");
  result.concat(clientDataPacket.rtcDateTime.second);
  //result.concat("1Z");
  return result;
}

void send_spi_cmd(const EMasterPacketTypes cmd)
{
  if(cmd == EMasterPacketTypes::None)
  {
    Serial.println("[NODE] Invalid SPI command!");
    return;
  }

  SPI.beginTransaction(spi_settings);

  if(cmd == EMasterPacketTypes::ReadAllSensors)
  {
    //init_clientDataPacket();
    SPI.transfer((uint8_t)cmd);
    delay(1);
    uint8_t* dataPacketIndex = (uint8_t*)&clientDataPacket;
    for( int i=0; i<sizeof(clientDataPacket);  i++)
    {
      *dataPacketIndex = SPI.transfer('.');
      dataPacketIndex++;
      delay(1);
    }
    //DEBUG
    // Serial.println("[NODE] packet received:");
    // Serial.println(clientDataPacket.temperatureSensor);
    // Serial.println(clientDataPacket.humidiySensor);
    // Serial.println(clientDataPacket.lightSensor);
    // Serial.println("[NODE] packet end...");
  }
  else
  {
    SPI.transfer((uint8_t)cmd);
    delay(1);
  }
  SPI.endTransaction();
  return;
}

// Replaces placeholder with LED state value
void spi_cmd_TurnLightsON(){
  Serial.println("[NODE] send SPI Command: Turn Lights ON");
  send_spi_cmd(EMasterPacketTypes::TurnLightsON);
}

void spi_cmd_TurnLightsOFF(){
  Serial.println("[NODE] send SPI Command: Turn Lights OFF");
  send_spi_cmd(EMasterPacketTypes::TurnLightsOFF);
}

void spi_cmd_TurnStepperMotorCW()
{
  Serial.println("[NODE] send SPI Command: Turn Stepper Motor CW");
  send_spi_cmd(EMasterPacketTypes::TurnStepperMotorCW);
}

void spi_cmd_TurnStepperMotorCCW()
{
  Serial.println("[NODE] send SPI Command: Turn Stepper Motor CCW");
  send_spi_cmd(EMasterPacketTypes::TurnStepperMotorCCW);
}

void spi_cmd_ToggleAirConditioning()
{
  Serial.println("[NODE] send SPI Command: Toggle Airconditioning");
  send_spi_cmd(EMasterPacketTypes::ToggleAirConditioning);
}

String spi_cmd_ReadAllSensors()
{
  Serial.println("[NODE] update sensor data on webpage");
  if(clientDataPacket.error)
  {
    return "";
  }
  // Json Variable to hold data sent from the Arduino
  JSONVar packet_from_client;
  packet_from_client["temperature"] = String(clientDataPacket.temperatureSensor);
  packet_from_client["humidity"] = String(clientDataPacket.humidiySensor);
  packet_from_client["luminosity"] = String(clientDataPacket.lightSensor);
  packet_from_client["manualswitch"] = (clientDataPacket.roomLightSwitchState == 1);
  packet_from_client["rtc_datetime"] = rtc_datetime_to_str();
  String jsonString = JSON.stringify(packet_from_client);
  return jsonString;
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void setup(){

  Serial.begin(115200);

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

  //MQTT client
  mqtt_client.setServer(mqtt_server, 1883);

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  //Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html");
  });

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = spi_cmd_ReadAllSensors();
    if(!json.isEmpty())
      request->send(200, "application/json", json);
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
  });
  server.addHandler(&events);

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });

  // Route to load script.js file
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js", "text/js");
  });

  // Route to set Relay to HIGH
  server.on("/relayOn", HTTP_GET, [](AsyncWebServerRequest *request){
    spi_cmd_TurnLightsON();
    request->send(LittleFS, "/index.html");
  });

  // Route to set Relay to LOW
  server.on("/relayOff", HTTP_GET, [](AsyncWebServerRequest *request){
    spi_cmd_TurnLightsOFF();
    request->send(LittleFS, "/index.html");
  });

  server.on("/motorCw", HTTP_GET, [](AsyncWebServerRequest *request){
    spi_cmd_TurnStepperMotorCW();
    request->send(LittleFS, "/index.html");
  });

  server.on("/motorCcw", HTTP_GET, [](AsyncWebServerRequest *request){
    spi_cmd_TurnStepperMotorCCW();
    request->send(LittleFS, "/index.html");
  });

  server.on("/airC", HTTP_GET, [](AsyncWebServerRequest *request){
    spi_cmd_ToggleAirConditioning();
    request->send(LittleFS, "/index.html");
  });
  server.onNotFound(notFound);
}

volatile static char luxTemp[8];

void loop(){
  SPI.begin();
  //MQTT demo
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  if(!mqtt_client.loop())
    mqtt_client.connect("ESP8266_SmartOffice");
  //==========================================
  // Start server
  server.begin();
  if ((millis() - lastTime) > timerDelay) {
    send_spi_cmd(EMasterPacketTypes::ReadAllSensors);
    // Send Events to the client with the Sensor Readings Every 5 seconds
    events.send("ping",NULL,millis());
    events.send(spi_cmd_ReadAllSensors().c_str(),"new_readings" ,millis());
    lastTime = millis();

    //MQTT client demo
    std::string tmp = std::to_string(clientDataPacket.lightSensor);
    mqtt_client.publish("smart_office/luminosity", tmp.c_str());
    tmp.clear();
    tmp = std::to_string(clientDataPacket.temperatureSensor);
    mqtt_client.publish("smart_office/temperature", tmp.c_str());
    tmp.clear();
    tmp = std::to_string(clientDataPacket.humidiySensor);
    mqtt_client.publish("smart_office/humidity", tmp.c_str());
    //====================
  }
}
