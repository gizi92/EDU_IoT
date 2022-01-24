#ifndef EDUIOTERRORHANDLER_H_
#define EDUIOTERRORHANDLER_H_

#include <Arduino.h>

// I/O Peripheral Errors
#define IOT_ERR_TSL2561_CONN -10
#define IOT_ERR_TSL2561_OVERLOAD -11
#define IOT_ERR_RTC_CONN -20
#define IOT_ERR_DHT11_GENERAL -30
#define IOT_ERR_DATAPACKET_UNKNOWN_MSTR_PACKET -40
#define IOT_ERR_SYS_INIT_FAIL -50

class EDU_IOT_ErrorHandler
{
  public:
    EDU_IOT_ErrorHandler() {};
    // Print Error Code to Serial Interface
    bool HandleCode(uint8_t code)
    {
      switch (code)
      {
      case IOT_ERR_TSL2561_CONN:
        Serial.println("IOT_ERROR ("); Serial.print(code); Serial.print("): No TSL2561 detected! Check your wiring or I2C ADDR!");
        break;
      
      case IOT_ERR_TSL2561_OVERLOAD:
        Serial.println("IOT_ERROR ("); Serial.print(code); Serial.print("): TSL2561 Sensor overload!");
        break;
      case IOT_ERR_RTC_CONN:
        Serial.println("IOT_ERROR ("); Serial.print(code); Serial.print("): RTC initialization failed!");
        break;
      case IOT_ERR_SYS_INIT_FAIL:
        Serial.println("IOT_ERROR ("); Serial.print(code); Serial.print("): IOT System Init failed!");
        break;
      case IOT_ERR_DHT11_GENERAL:
        Serial.println("IOT_ERROR ("); Serial.print(code); Serial.print("): DHT11 General Error!");
        break;
      default:
        break;
      }
    }
};
#endif