#include<SPI.h>

enum class EMasterPacketTypes : uint8_t
{
  None = 0,
  ReadAllSensors,
  TurnLedON,
  TurnLedOFF
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

volatile SPacketAllSensors clientDataPacket;
// volatile uint8_t* dataPacketIndex;

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
  }
  else
  {
    Serial.print("[NODE] Sensor error ");
    Serial.println(clientDataPacket.error);
  }
   Serial.println();
   Serial.println();
   delay(1000); 

   //TEST LED OFF 
   SPI.beginTransaction(spi_settings);
   slaveRequest = EMasterPacketTypes::TurnLedOFF;
   SPI.transfer((uint8_t)slaveRequest);
   delay(1);
   SPI.endTransaction();
   delay(1000);

   //TEST LED ON
   SPI.beginTransaction(spi_settings);
   slaveRequest = EMasterPacketTypes::TurnLedON;
   SPI.transfer((uint8_t)slaveRequest);
   delay(1);
   SPI.endTransaction();
   delay(1000);  
}
