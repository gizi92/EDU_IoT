#include <SPI.h>
#include <Wire.h> //RTC
#include <DS3231.h> //RTC
#include <LiquidCrystal_I2C.h> //LCD
#include <Adafruit_Sensor.h> //TSL2561 Light sensor
#include <Adafruit_TSL2561_U.h> //TSL2561 Light sensor
#include <SimpleDHT.h> // DHT 11 humidity and temperature
#include <Stepper.h> // Stepper motor
#include <IRremote.hpp> // IR remote
#include "ac_LG.hpp" // IR remote
#include <DataPacket.h>
#include <Peripheral.h>
#include <EduIotErrorHandler.h>

//For LG Air Conditioning IR remote
#define IR_SEND_PIN               7
#define SIZE_OF_RECEIVE_BUFFER   10
char sRequestString[SIZE_OF_RECEIVE_BUFFER];
Aircondition_LG MyLG_Aircondition;
volatile bool isAirConditioningON;
//====================================

//Define Constants
//================
//Pins
// const uint8_t pin_IR_LED = 7;
const uint8_t pin_Relay = 9; // works only if 12V supplied on Arduino VIN
const uint8_t pin_ManualSwitch = 14;
const uint8_t pin_dht11 = 17;
const uint8_t pin_stepper_In1 = 8;
const uint8_t pin_stepper_In2 = 6;
const uint8_t pin_stepper_In3 = 5;
const uint8_t pin_stepper_In4 = 4;

//I2C Addresses
const uint8_t i2c_addr_LCD = 0x27;
const uint8_t i2c_addr_TSL2561 = 0x39;
const uint8_t i2c_addr_RTC = 0x68; //TODO find out how to use it
//Settings

// Stepper motor
// Number of steps per internal motor revolution
const float stepper_steps_per_rev = 32;
// Amount of Gear Reduction
const float stepper_gear_red = 64;
// Number of steps per geared output rotation
const float stepper_steps_per_out_rev = stepper_steps_per_rev * stepper_gear_red;
// Speed - TODO - get this param remotely
const float stepper_speed = 100;

volatile bool stepper_spi_cmd_cw;
volatile bool stepper_spi_cmd_ccw;

//================
// Define Variables

//OUTPUTS
DigitalOutput relaySwitch(pin_Relay);
LiquidCrystal_I2C lcd(i2c_addr_LCD,20,4);
Stepper stepperMotor(stepper_steps_per_rev, pin_stepper_In1, pin_stepper_In2, pin_stepper_In3, pin_stepper_In4);

//INPUTS
DS3231 rtc; // I2C ADDRESS 0x68
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(i2c_addr_TSL2561, 10);
SimpleDHT11 dht11(pin_dht11);
DigitalInput ManualSwitch(pin_ManualSwitch);
GenericSensor<int32_t> LightSensor(0);
GenericSensor<int16_t> HumiditySensor(0);
GenericSensor<int16_t> TemperatureSensor(0);

// Helper Variables
// Peripherals
volatile SRealTimeClockDateTime *rtcPtr;
volatile RTCDateTime rtcDateTime;
volatile sensors_event_t tsl_event;
volatile byte temperature;
volatile byte humidity;
volatile int stepper_stepsRequired;
volatile int manualSwitchOldState;
// SPI
volatile byte spi_index_received;
volatile byte spi_index_sent;
volatile bool spi_receive;
volatile bool spi_send;
volatile bool spi_end_transmission;
SPISettings spi_settings(100000, MSBFIRST, SPI_MODE0);
// DATA PACKETS
volatile SPacketAllSensors clientDataPacket;
volatile uint8_t* dataPacketIndex;
volatile EMasterPacketTypes masterPacketType;

//Error Handling
EDU_IOT_ErrorHandler errorHandler;

bool init_peripherals(void)
{
  Serial.begin(115200);
  /*=========================================
    Initialize Real Time Clock
    ========================================= */
  if(!rtc.begin())
  {
    errorHandler.HandleCode(IOT_ERR_RTC_CONN);
  }
  // rtc.setDateTime(__DATE__, __TIME__); //Uncomment, if you want to reset the time and date.

  /*=========================================
    Initialize  Light Sensor: Configure the gain and integration time for the TSL2561
    ========================================= */
  //use tsl.begin() to default to Wire,
  //tsl.begin(&Wire2) directs api to use Wire2, etc.
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    errorHandler.HandleCode(IOT_ERR_TSL2561_CONN);
    return false;
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
  /*=========================================
    Initialize LCD
    ========================================= */
  lcd.init();
  lcd.backlight();

  // SPI Setup
  SPCR |= bit(SPE);         //bekapcsolja az SPIt
  pinMode(MISO, OUTPUT);    //MISOn valaszol
  spi_index_received = 0;
  spi_index_sent = 0;
  spi_receive = true;
  spi_send = false;
  spi_end_transmission = false;
  dataPacketIndex = (uint8_t*)&clientDataPacket;

  //CMD LightSwitch
  ManualSwitch.ReadState();
  manualSwitchOldState = ManualSwitch.GetLastReadValue();
  //CMD Stepper
  stepper_spi_cmd_cw = false;
  stepper_spi_cmd_ccw = false;

  //IR remote to LG Air  Conditioning
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
#if defined(IR_SEND_PIN)
    IrSender.begin(IR_SEND_PIN); // Start with IR_SEND_PIN as send pin and enable feedback LED at default feedback LED pin
#else
    IrSender.begin(IR_SEND_PIN, ENABLE_LED_FEEDBACK); // Specify send pin and enable feedback LED at default feedback LED pin
#endif
  Serial.print(F("Ready to send IR signals at pin "));
  Serial.println(IR_SEND_PIN);
  Serial.println();
  MyLG_Aircondition.setType(LG_IS_WALL_TYPE);
  isAirConditioningON = false;
  return true;
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
  Serial.begin(115200);
    if(!init_peripherals())
    {
      errorHandler.HandleCode(IOT_ERR_SYS_INIT_FAIL);
    }
    else
    {
      SPI.attachInterrupt();   //ha jon az SPIn  valami beugrik a fuggvenybe
    }
}


void loop ()
{
  if(stepper_spi_cmd_cw)
  {
    SPI.detachInterrupt();
    stepper_stepsRequired = stepper_steps_per_out_rev/2;
    stepperMotor.setSpeed(stepper_speed);
    stepperMotor.step(stepper_stepsRequired);
    stepper_spi_cmd_cw = false;
    SPI.attachInterrupt();
  }
  else if (stepper_spi_cmd_ccw)
  {
    SPI.detachInterrupt();
    stepper_stepsRequired = - stepper_steps_per_out_rev/2;
    stepperMotor.setSpeed(stepper_speed);
    stepperMotor.step(stepper_stepsRequired);
    stepper_spi_cmd_ccw = false;
    SPI.attachInterrupt();
  }

  /*=========================================
    Get RTC data
    ========================================= */
  rtcDateTime = rtc.getDateTime();

  /*=========================================
    Get Light Sensor data (light is measured in lux)
    ========================================= */
  tsl.getEvent(&tsl_event);
  if (tsl_event.light)
  {
    LightSensor.SetValue((int32_t)tsl_event.light);
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    errorHandler.HandleCode(IOT_ERR_TSL2561_OVERLOAD);
    LightSensor.SetValue(0);
  }
  /*=========================================
    Get Temperature and Humidity Data
    ========================================= */
  int err = SimpleDHTErrSuccess;
  if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess)
  {
    errorHandler.HandleCode(IOT_ERR_DHT11_GENERAL);
    HumiditySensor.SetValue(0);
    TemperatureSensor.SetValue(0);
  }
  else
  {
    HumiditySensor.SetValue((int16_t)humidity);
    TemperatureSensor.SetValue((int16_t)temperature);
  }
  /*=========================================
    Read Manual Switch State
    ========================================= */
  ManualSwitch.ReadState();
  // If switch state changed, execute command
  if (manualSwitchOldState != ManualSwitch.GetLastReadValue())
  {
    manualSwitchOldState = ManualSwitch.GetLastReadValue();
    if(manualSwitchOldState)
      relaySwitch.TurnOFF();
    else
    {
      relaySwitch.TurnON();
    }
  }

  /*=========================================
    Print Data to LCD
    ========================================= */
  lcd_print_all();
  /*=========================================
    Handle SPI transmission end
    ========================================= */
  if (spi_end_transmission)
  {
    spi_end_transmission = false;
    spi_receive = true;
    spi_send = false;

    dataPacketIndex = (uint8_t*)&clientDataPacket;
    spi_index_sent = 0;
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
        clientDataPacket.roomLightSwitchState = relaySwitch.GetState();
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
        Serial.println("[UNO] Received master packet - TurnLightsON");
        Serial.println();
        relaySwitch.TurnON();
        break;
      }
    case EMasterPacketTypes::TurnLightsOFF:
      {
        Serial.println("[UNO] Received master packet - TurnLightsOFF");
        Serial.println();
        relaySwitch.TurnOFF();
        break;
      }

    case EMasterPacketTypes::TurnStepperMotorCW:
      {
        Serial.println("[UNO] Received master packet - TurnStepperMotorCW");
        stepper_spi_cmd_cw = true;
        break;
      }
    case EMasterPacketTypes::TurnStepperMotorCCW:
      {
        Serial.println("[UNO] Received master packet - TurnStepperMotorCCW");
        stepper_spi_cmd_ccw = true;
        break;
      }
    case EMasterPacketTypes::ToggleAirConditioning:
      {
        Serial.println("[UNO] Received master packet - ToggleAirConditioning");
        if(isAirConditioningON)
        {
          MyLG_Aircondition.sendIRCommand(LG_COMMAND_ON);
          isAirConditioningON = true;
        }
        {
          MyLG_Aircondition.sendIRCommand(LG_COMMAND_OFF);
          isAirConditioningON = false;
        }
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
      spi_index_sent++;
      if(spi_index_sent > sizeof(clientDataPacket))
        spi_end_transmission = true;
    }
    else
    {
      spi_end_transmission = true;
    }
  }
}
