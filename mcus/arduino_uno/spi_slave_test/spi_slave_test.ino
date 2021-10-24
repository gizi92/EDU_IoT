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

enum class EMasterPacketTypes : uint8_t
{
  None = 0,
  ReadAllSensors
};

EMasterPacketTypes lastReceivedMasterPacket;

void handleMasterPacket(uint8_t requestType)
{
  lastReceivedMasterPacket = (EMasterPacketTypes)requestType;
}

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

template <typename T> size_t sendSPI(const T& data)
{ 
  uint8_t* pData = (uint8_t*)&data;
  size_t count = 0;
  for(count = 0; count < sizeof(data); count++)
  {
    SPDR = *pData++;
  }
  return sizeof(data);
}

template <typename T> size_t readSPI(T& data)
{
  uint8_t* pData = (uint8_t*)&data;
  uint8_t* pStart = pData;
  
  size_t count = 0;
  for(count = 0; count < sizeof(data); count++)
  {
    *pData++ = SPDR;
  }

  return count;
}

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

char bejovo_uzenet[100];
char kimeno_uzenet[100]="Felelet az arduinotol\n";

volatile byte index_received;
volatile byte index_sent;
volatile bool spi_receive;
volatile bool spi_send;
volatile bool spi_end_transmission;

void setup (void)
{
  Serial.begin (9600);
  SPCR |= bit(SPE);         //bekapcsolja az SPIt
  //SPI.begin();
  pinMode(MISO, OUTPUT);    //MISOn valaszol
  
  SPI.attachInterrupt();   //ha jon az SPIn  valami beugrik a fuggvenybe

  //DEBUG
  // Serial.println("sizeof(SPacketAllSensors):");
  // Serial.print(sizeof(SPacketAllSensors));
}

void loop ()
{
  MQ135.Read();
  PhotoResistor.Read();
  delay(500);
}

// SPI interrupt routine
ISR(SPI_STC_vect)
{
  //receive and handle master request
  uint8_t requestType = (uint8_t)SPDR;
  Serial.print("[UNO] Received packet : ");
  Serial.print(requestType);
  Serial.println();
  //handleMasterPacket(requestType);
  
  //Handle master request
  SPacketAllSensors sensorData;
  switch (requestType)
  {
  case 1:
    {
      Serial.println("[UNO] Received master request ReadAllSensors");

      sensorData.error = 0;
      sensorData.gas1 = MQ135.GetLastReadValue();
      sensorData.gas2 = 0;
      sensorData.lightSensor = PhotoResistor.GetLastReadValue();
      sendSPI(sensorData);
      break;
    }
  
  default:
    {
      Serial.print("[UNO] Unknown master request with type ");
      Serial.println((uint8_t)requestType);

      sensorData.error = 1;
      
      break;
    }
  }
}
