// I2C NodeMCU Master

#include <Wire.h>

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
  // put your setup code here, to run once:
  Serial.begin(9600);

  Wire.begin(D1, D2);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // Device address 8
  Wire.beginTransmission(8);

  SPacketReadAllSensors readRequest;
  sendI2C(readRequest);

  Wire.endTransmission();

  Wire.requestFrom(8, sizeof(SPacketAllSensors));
  SPacketAllSensors sensorData;
  Serial.println("[NODE] Receiving sensor packet");
  readI2C(sensorData);

  if(sensorData.error == 0)
  {
    Serial.println("[NODE] Received sensor data");
    Serial.print("[NODE] Gas1 : ");
    Serial.println(sensorData.gas1);
    Serial.print("[NODE] Gas2 : ");
    Serial.println(sensorData.gas2);
    Serial.print("[NODE] Light : ");
    Serial.println(sensorData.lightSensor);
  }
  else
  {
    Serial.print("[NODE] Sensor error ");
    Serial.println(sensorData.error);
  }

  while(Wire.available())
  {
    Serial.print((uint8_t)Wire.read());
    Serial.print(" ");
  }

  Serial.println();
  delay(2000);
}
