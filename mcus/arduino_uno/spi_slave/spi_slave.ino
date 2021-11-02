#include <SPI.h>
#include <DataPacket.h>
#include <Peripheral.h>

// INPUTS
AnalogSensor MQ135(0);
AnalogSensor PhotoResistor(1);

//OUTPUTS
DigitalOutput PurpleLED(3);

//SPI
volatile byte index_received;
volatile byte index_sent;
volatile bool spi_receive;
volatile bool spi_send;
volatile bool spi_end_transmission;
SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

//DATA PACKETS
volatile SPacketAllSensors clientDataPacket;
volatile uint8_t* dataPacketIndex;
volatile EMasterPacketTypes masterPacketType;



void setup (void)
{
    Serial.begin (9600);
    // SPI Setup
    SPCR |= bit(SPE);         //bekapcsolja az SPIt
    pinMode(MISO, OUTPUT);    //MISOn valaszol
    index_received = 0;
    index_sent = 0;
    spi_receive = true;
    spi_send = false;
    spi_end_transmission = false;
    dataPacketIndex = (uint8_t*)&clientDataPacket;
    SPI.attachInterrupt();   //ha jon az SPIn  valami beugrik a fuggvenybe
    //
}

void loop ()
{
  MQ135.Read();
  PhotoResistor.Read();
  if (spi_end_transmission)
  {
    spi_end_transmission = false;
    spi_receive = true;
    spi_send = false;

    dataPacketIndex = (uint8_t*)&clientDataPacket;
    index_sent = 0;
  }
  delay(100);
}

// SPI interrupt routine
ISR(SPI_STC_vect)
{
  //receive and handle master request
  uint8_t receivedByte = (uint8_t)SPDR;
  
  if(spi_receive)
  {
    masterPacketType = (EMasterPacketTypes)receivedByte;
    //Prepare clientDataPackage for sending to master
    switch (masterPacketType)
    {
    case EMasterPacketTypes::ReadAllSensors:
      {
        Serial.println("[UNO] Received master packet ReadAllSensors");
        Serial.println();
        clientDataPacket.error = 0;
        clientDataPacket.gas1 = MQ135.GetLastReadValue();
        clientDataPacket.gas2 = 0;
        clientDataPacket.lightSensor = PhotoResistor.GetLastReadValue();
        break;
      }
    case EMasterPacketTypes::TurnLedON:
      {
        Serial.println("[UNO] Received master packet TurnLedON");
        Serial.println();
        PurpleLED.TurnON();
        break;
      }
    case EMasterPacketTypes::TurnLedOFF:
      {
        Serial.println("[UNO] Received master packet TurnLedOFF");
        Serial.println();
        PurpleLED.TurnOFF();
        break;
      }
    default:
      {
        Serial.print("[UNO] Unknown master packet with type ");
        Serial.println();
        Serial.println((uint8_t)masterPacketType);
        clientDataPacket.error = -1;
        break;
      }
    }
    spi_receive = false;
    spi_send = true;
  }

  if(spi_send)
  {
    if(masterPacketType == EMasterPacketTypes::ReadAllSensors)
    {
      SPDR = *dataPacketIndex++;
      index_sent++;
      if(index_sent > sizeof(clientDataPacket))
        spi_end_transmission = true;
    }
    else
    {
      spi_end_transmission = true;
    }
  }
}
