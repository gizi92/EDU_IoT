#include <SPI.h>
#include <Wire.h> //RTC
#include <DS3231.h> //RTC
#include <LiquidCrystal_I2C.h> //LCD
#include <Adafruit_Sensor.h> //TSL2561 Light sensor
#include <Adafruit_TSL2561_U.h> //TSL2561 Light sensor
#include <SimpleDHT.h> // DHT 11 humidity and temperature
#include <DataPacket.h>
#include <Peripheral.h>

// INPUTS
DS3231 rtc; // I2C ADDRESS 0x68
LiquidCrystal_I2C lcd(0x27,20,4); // I2C ADDRESS 0x27
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); // I2C ADDRESS 0x39
SimpleDHT11 dht11(A3);

//OUTPUTS
DigitalOutput LightSwitch(15);

//SPI
volatile byte index_received;
volatile byte index_sent;
volatile bool spi_receive;
volatile bool spi_send;
volatile bool spi_end_transmission;
SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);

// Data sampling
GenericSensor<float> LightSensor(0);
GenericSensor<int16_t> HumiditySensor(0);
GenericSensor<int16_t> TemperatureSensor(0);
DigitalInput RoomLightSwitch(A0);
//GenericSensor<SRealTimeClockDateTime> rtc_data();

//DATA PACKETS
volatile SPacketAllSensors clientDataPacket;
volatile uint8_t* dataPacketIndex;
volatile EMasterPacketTypes masterPacketType;

//DEBUG vars
volatile SRealTimeClockDateTime *rtcPtr;
volatile RTCDateTime rtcDateTime;
volatile sensors_event_t tsl_event;
volatile byte temperature;
volatile byte humidity;

void init_peripherals(void)
{
  Serial.begin(115200);
  /*
    Initialize Real Time Clock
  */
  rtc.begin();
  // rtc.setDateTime(__DATE__, __TIME__); //Uncomment, if you want to reset the time and date.
  /*
    Initialize  Light Sensor: Configure the gain and integration time for the TSL2561
  */
  //use tsl.begin() to default to Wire, 
  //tsl.begin(&Wire2) directs api to use Wire2, etc.
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */  
  // Serial.println("------------------------------------");
  // Serial.print  ("Gain:         "); Serial.println("Auto");
  // Serial.print  ("Timing:       "); Serial.println("101 ms");
  // Serial.println("------------------------------------");
  
  /*
    Initialize LCD
  */
  lcd.init();
  lcd.backlight();
}

// Print all data to the LCD
void lcd_print_all(void)
{
  lcd.clear();
  //row 1
  lcd.setCursor(0,0);
  lcd.print("Date:");
  lcd.print(rtcDateTime.year, 10);
  lcd.print("-");
  lcd.print(rtcDateTime.month, 10);
  lcd.print("-");
  lcd.print(rtcDateTime.day, 10);
  // row 2
  lcd.setCursor(0,1);
  lcd.print("Time:");
  lcd.print(rtcDateTime.hour, 10);
  lcd.print(":");
  lcd.print(rtcDateTime.minute, 10);
  lcd.print(":");
  lcd.print(rtcDateTime.second, 10);
  // row 3
  lcd.setCursor(0,2);
  lcd.print("Light: "); lcd.print(LightSensor.GetLastReadValue()); lcd.print(" lux");
  // row 4
  lcd.setCursor(0,3);
  lcd.print("DHT11: "); lcd.print(TemperatureSensor.GetLastReadValue()); lcd.print(" *C, "); lcd.print(HumiditySensor.GetLastReadValue()); lcd.print(" H");
}


void setup (void)
{
    init_peripherals();
    //=====================

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
  // Get RTC data and set data packet
  
  // rtcPtr = (SRealTimeClockDateTime *)&rtc.getDateTime();
  // rtcDataPacket.rtcDateTime = (SRealTimeClockDateTime)*rtcPtr;
  rtcDateTime = rtc.getDateTime();

  tsl.getEvent(&tsl_event);
  
  
  /* Display the results (light is measured in lux) */
  if (tsl_event.light)
  {
    LightSensor.SetValue(tsl_event.light);
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    lcd.println("Sensor overload");
  }
  
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) 
  {
    lcd.print("Read DHT11 failed, err="); lcd.println(err);
    return;
  }
  else
  {
    HumiditySensor.SetValue((int16_t)humidity);
    TemperatureSensor.SetValue((int16_t)temperature);
  }
  
  RoomLightSwitch.ReadState();
  lcd_print_all();
  if (spi_end_transmission)
  {
    spi_end_transmission = false;
    spi_receive = true;
    spi_send = false;

    dataPacketIndex = (uint8_t*)&clientDataPacket;
    index_sent = 0;
  }
  delay(1000);
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
        clientDataPacket.roomLightSwitchState = RoomLightSwitch.GetLastReadValue();
        clientDataPacket.humidiySensor = HumiditySensor.GetLastReadValue();
        clientDataPacket.temperatureSensor = TemperatureSensor.GetLastReadValue();
        clientDataPacket.lightSensor = LightSensor.GetLastReadValue();
        // TODO simplify
        clientDataPacket.rtcDateTime.year = rtcDateTime.year;
        clientDataPacket.rtcDateTime.month = rtcDateTime.month;
        clientDataPacket.rtcDateTime.day = rtcDateTime.day;
        clientDataPacket.rtcDateTime.hour = rtcDateTime.hour;
        clientDataPacket.rtcDateTime.minute = rtcDateTime.minute;
        clientDataPacket.rtcDateTime.second = rtcDateTime.second;
        clientDataPacket.rtcDateTime.dayOfWeek = rtcDateTime.dayOfWeek;
        clientDataPacket.rtcDateTime.unixtime = rtcDateTime.unixtime;
        break;
      }
    case EMasterPacketTypes::TurnLightsON:
      {
        Serial.println("[UNO] Received master packet TurnLightsON");
        Serial.println();
        LightSwitch.TurnON();
        break;
      }
    case EMasterPacketTypes::TurnLightsOFF:
      {
        Serial.println("[UNO] Received master packet TurnLightsOFF");
        Serial.println();
        LightSwitch.TurnOFF();
        break;
      }
    case EMasterPacketTypes::TurnAirConditioningON:
      {
        Serial.println("[UNO] Received master packet TurnAirConditioningON");
        // TODO IR LED command
        break;
      }
    case EMasterPacketTypes::TurnAirConditioningOFF:
      {
        Serial.println("[UNO] Received master packet TurnAirConditioningOFF");
        // TODO IR LED command
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
