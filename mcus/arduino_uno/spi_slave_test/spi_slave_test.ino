#include <SPI.h>

class AnalogSensor
{
  public:
    AnalogSensor(const int analogPin)
    {
      m_analogPin = analogPin;
    }

    const int GetAnalogPin() const
    {
      return m_analogPin;
    }

    const int GetLastReadValue() const
    {
      return m_value;
    }

    void Read()
    {
      m_value = analogRead(m_analogPin);
    }
  private:
    volatile int m_value;
    int m_analogPin;
};

AnalogSensor MQ135(0);
AnalogSensor PhotoResistor(1);

class OutputPeripherals
{
  public:
    OutputPeripherals(const int pin)
    {
      m_pin = pin;
      pinMode(m_pin,OUTPUT);
      TurnOFF();
    }

    const int GetPin() const
    {
      return m_pin;
    }

    void TurnON()
    {
      digitalWrite(m_pin, HIGH);
      m_value = 1;
      
    }

    void TurnOFF()
    {
      digitalWrite(m_pin, LOW);
      m_value = 0;
      
    }
    
    const int GetStatus() const
    {
      return m_value;
    }

  private:
    volatile int m_value;
    int m_pin;
};

OutputPeripherals PurpleLED(3);

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

volatile byte index_received;
volatile byte index_sent;
volatile bool spi_receive;
volatile bool spi_send;
volatile bool spi_end_transmission;

volatile SPacketAllSensors clientDataPacket;
volatile uint8_t* dataPacketIndex;

volatile EMasterPacketTypes masterPacketType;


SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

void setup (void)
{
  Serial.begin (9600);
  SPCR |= bit(SPE);         //bekapcsolja az SPIt
  //SPI.begin();
  pinMode(MISO, OUTPUT);    //MISOn valaszol

  index_received = 0;
  index_sent = 0;
  spi_receive = true;
  spi_send = false;
  spi_end_transmission = false;

  dataPacketIndex = (uint8_t*)&clientDataPacket;
  SPI.attachInterrupt();   //ha jon az SPIn  valami beugrik a fuggvenybe
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
  delay(500);
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
