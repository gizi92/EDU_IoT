#include <SPI.h>

int index = 0;
uint8_t requestType = 0;

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

typedef struct __attribute__((packed)) SPacketAllSensors
{
  uint16_t error;
  uint16_t gas1;
  uint16_t gas2;
  uint16_t lightSensor;
};

template <typename T> uint8_t messageSPI(const T data)
{ 
  int sizeOfData = sizeof(data);
  uint8_t data_arr[sizeOfData];
  memcpy(data_arr, &data, sizeOfData);
  uint8_t receivedData = SPDR;
  SPDR = data_arr[index];
  index++;
  return receivedData;
}

SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

char bejovo_uzenet[100];
char kimeno_uzenet[100]="Felelet az arduinotol\n";
SPacketAllSensors packageInstance;
int structSize = sizeof(packageInstance);
SPacketAllSensors sensorData;

void setup (void)
{
  Serial.begin (9600);
  Serial.println("[UNO] Setup");
  Serial.println(structSize);
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
  Serial.print("[UNO] Reading the received message : ");
  Serial.println((uint8_t)SPDR);
  
  if (requestType == 0) 
  {
    //receive and handle master request
    requestType = (uint8_t)SPDR;
    Serial.println(requestType);
    index = 0;
    sensorData.error = 0;
    sensorData.gas1 = MQ135.GetLastReadValue();
    sensorData.gas2 = 0;
    sensorData.lightSensor = PhotoResistor.GetLastReadValue();
    Serial.print("Error:");
    Serial.println(sensorData.error);
    Serial.print("Gas1:");
    Serial.println(sensorData.gas1);
    Serial.print("Gas2:");
    Serial.println(sensorData.gas2);
    Serial.print("LightSensor:");
    Serial.println(sensorData.lightSensor);
  } else {    
    switch (requestType)
    {
    case 1:
      {
        if (index == 0) 
          Serial.println("[UNO] Received master request ReadAllSensors");
        messageSPI(sensorData);
        if (index >= structSize) {
          requestType = 0;
        }
        break;
      }
    case 2:
       { 
         if (index == 0)     
           Serial.println("[UNO] Received master request TurnLedON");
  
         PurpleLED.TurnON();
         break;  
       }
    case 3:
      {
         Serial.println("[UNO] Received master request TurnLedOFF");
  
         PurpleLED.TurnOFF();
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
}
