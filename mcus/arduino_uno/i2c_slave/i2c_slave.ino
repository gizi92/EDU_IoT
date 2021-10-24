// I2C Arduino UNO Slave
#include <Wire.h>

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
  uint32_t temperature;
  uint32_t humidity;
  uint16_t lightSensor;

  SPacketAllSensors()
  {
    type = EClientPacketTypes::AllSensorData;
  }
};

template <typename T> size_t sendI2C(const T& data)
{
  Wire.write((uint8_t*)&data, sizeof(data));
  return sizeof(data);
}

template <typename T> size_t readI2C(T& data)
{
  uint8_t* pData = (uint8_t*)&data;
  uint8_t* pStart = pData;
  
  size_t count = 0;
  for(count = 0; count < sizeof(data); count++)
  {
    *pData++ = Wire.read();
  }

  return count;
}

void setup() {
  // Join I2C bus with address 8
  Wire.begin(8);

  Wire.onReceive(onReceiveEvent);
  Wire.onRequest(onRequestEvent);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  MQ135.Read();
  PhotoResistor.Read();
  delay(500);
}

EMasterPacketTypes lastReceivedMasterPacket;
void handleMasterPacket(uint8_t requestType)
{
  lastReceivedMasterPacket = (EMasterPacketTypes)requestType;
}

void onReceiveEvent(int byteCount)
{
  uint8_t requestType = 0;
  while(Wire.available() > 0)
  {
    char c = Wire.read();
    Serial.print("[UNO] Received packet : ");
    Serial.print((uint8_t)c);

    requestType = (uint8_t)c;
  }
  Serial.println();

  handleMasterPacket(requestType);
}

void onRequestEvent()
{
  SPacketAllSensors sensorData;
  
  switch(lastReceivedMasterPacket)
  {
    case EMasterPacketTypes::ReadAllSensors:
    {
      Serial.println("[UNO] Received master request ReadAllSensors");

      sensorData.error = 0;
      sensorData.gas1 = MQ135.GetLastReadValue();
      sensorData.gas2 = 0;
      sensorData.temperature = 0;
      sensorData.humidity = 0;
      sensorData.lightSensor = PhotoResistor.GetLastReadValue();
      break;
    }
    default:
    {
      Serial.print("[UNO] Unknown master request with type ");
      Serial.println((uint8_t)lastReceivedMasterPacket);

      sensorData.error = 1;
      
      break;
    }
  }

  sendI2C(sensorData);
}
