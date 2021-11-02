#include<SPI.h>
#include<NodeWebServer.h>
#include<DataPacket.h>


volatile SPacketAllSensors clientDataPacket;
bool isLedOn = false;

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0); 
//100 kHz legyen a sebesseg, a Node tud 80MHzt de az Arduino csak 16MHzt

const char* ssid = "BEAvatLAK";
const char* pwd = "nekunkolyanNincs!";

NodeWebServer webServer(80);

void setup() 
{
  Serial.begin(9600);

  Serial.print("[NODE] Connecting to");
  Serial.println(ssid);

  WiFi.begin(ssid, pwd);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("[Node] Connected to WiFi with IP ");
  Serial.println(WiFi.localIP());

  webServer.AddHandler("/", [&webServer] {
    webServer.SendHTML(200, webServer.GetHTML());
    Serial.println("[NODE] Sent values");
  });

  webServer.AddHandler("/refresh_sensors", [&webServer] {
    webServer.SendHTML(200, webServer.GetHTML());
    Serial.println("[NODE] Sent values");
    //TEST READ ALL SENSOR DATA
    SPI.beginTransaction(spi_settings);
    //send master packet to slave
    EMasterPacketTypes slaveRequest = EMasterPacketTypes::ReadAllSensors;
    SPI.transfer((uint8_t)slaveRequest);
    delay(1);

    uint8_t* dataPacketIndex = (uint8_t*)&clientDataPacket;
    for( int i=0; i<sizeof(clientDataPacket);  i++)
    {
      *dataPacketIndex = SPI.transfer('.');
      dataPacketIndex++;
      delay(1);
    }
    SPI.endTransaction();
  
    if(clientDataPacket.error == 0)
    {
      Serial.println("[NODE] Received sensor data");
      Serial.print("[NODE] Gas1 : ");
      Serial.println(clientDataPacket.gas1);
      Serial.print("[NODE] Gas2 : ");
      Serial.println(clientDataPacket.gas2);
      Serial.print("[NODE] Light: ");
      Serial.println(clientDataPacket.lightSensor);
      webServer.SetValue("Gas: ", clientDataPacket.gas1);
      webServer.SetValue("Light: ", clientDataPacket.lightSensor);
    }
    else
    {
      Serial.print("[NODE] Sensor error ");
      Serial.println(clientDataPacket.error);
    }
  });

  webServer.AddHandler("/toggleled1", [&webServer,&isLedOn] {
    webServer.SendHTML(200, webServer.GetHTML());
    Serial.println("[NODE] Toggling LED");
    SPI.beginTransaction(spi_settings);
    //send master packet to slave
    EMasterPacketTypes slaveRequest = isLedOn ? EMasterPacketTypes::TurnLedON : EMasterPacketTypes::TurnLedOFF;
    SPI.transfer((uint8_t)slaveRequest);
    delay(1);
    SPI.endTransaction();
    isLedOn = !isLedOn; 
  });

  webServer.AddNotFoundHandler([&webServer] {
    webServer.SendJson(404, "{\"response\":\"Not found\"}");
    Serial.println("[NODE] 404 not found");
  });
  
  webServer.Start();
    
   SPI.begin();
}

void loop() 
{
  Serial.println();
  Serial.println();
  webServer.Loop();
  delay(100); 
}
