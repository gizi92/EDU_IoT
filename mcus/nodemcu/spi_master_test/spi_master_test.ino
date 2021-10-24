#include<SPI.h>

enum class EMasterPacketTypes : uint8_t
{
  None = 0,
  ReadAllSensors,
  TestEntry
};

enum class EClientPacketTypes : uint8_t
{
  None = 0,
  PreparePayload,
  AllSensorData,
  Error
};

enum class EErrorType : uint8_t
{
  UnknownError = 0,
  UnknownPacket
};

typedef struct __attribute__((packed)) SPacketReadAllSensors
{
  EMasterPacketTypes type;

  SPacketReadAllSensors()
  {
    type = EMasterPacketTypes::ReadAllSensors;
  }
};

typedef struct __attribute__((packed)) SPacketAllSensors
{
  EClientPacketTypes type;

  uint8_t error;
  uint16_t gas1;
  uint16_t gas2;
  uint16_t lightSensor;

  SPacketAllSensors()
  {
    type = EClientPacketTypes::AllSensorData;
  }
};

template <typename T> size_t sendSPI(const T& data)
{
  SPI.transfer((uint8_t*)&data, sizeof(data));
  return sizeof(data);
}

template <typename T> size_t readSPI(T& data)
{
  uint8_t* pData = (uint8_t*)&data;
  size_t count = 0;

  for(count = 0; count < sizeof(data); count++)
  {
    *pData++ = SPI.transfer('.');
  }
  return count;
}

char buff[]="Uzenet a Nodetol\n";
char kuldes_tesztje[20];
char returnbuff[100];

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0); 
//100 kHz legyen a sebesseg, a Node tud 80MHzt de az Arduino csak 16MHzt

void setup() 
{
   Serial.begin(9600);  
   SPI.begin();
}

void loop() 
{
   SPI.beginTransaction(spi_settings);
   //elkuldi az uzenetet
   EMasterPacketTypes readRequest = EMasterPacketTypes::ReadAllSensors;
   SPI.transfer((uint8_t)readRequest);
   
//   for(int i=0; i<sizeof(buff); i++)
//   {  
//      kuldes_tesztje[i] = SPI.transfer(buff[i]);
//      delay(1);
//   }
   SPacketAllSensors sensorData;
   Serial.println("[NODE] Receiving sensor packet");
//   readSPI(sensorData);
   //kuld meg 100 pontot,ezeket az Arduino felulirja
   //ami visszajon azt kiolvassuk betesszuk a returnbuffbe
//   for(int i=0; i<100; i++)
//   {  
//      returnbuff[i] = SPI.transfer('.');
//      delay(1);
//   }
    
   SPI.endTransaction();

   //itt kiirjuk. Az SPI sokkal gyorsabb volt mint a soros port
   //ezert ott kozben nem probaljuk meg kiirni, a soros kiiratas lassu
//   for(int i=0; i<20; i++)  
//      Serial.print(kuldes_tesztje[i]);
//
//   for(int i=0; i<100; i++)  
//      Serial.print(returnbuff[i]);
  if(sensorData.error == 0)
  {
    Serial.println("[NODE] Received sensor data");
    Serial.print("[NODE] Gas1 : ");
    Serial.println(sensorData.gas1);
    Serial.print("[NODE] Gas2 : ");
    Serial.println(sensorData.gas2);
    Serial.print("[NODE] Light: ");
    Serial.println(sensorData.lightSensor);
  }
  else
  {
    Serial.print("[NODE] Sensor error ");
    Serial.println(sensorData.error);
  }
   Serial.println();
   delay(1000);  
}
