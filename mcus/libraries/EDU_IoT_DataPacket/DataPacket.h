#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <Arduino.h>

struct SRealTimeClockDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
    uint32_t unixtime;
};

enum class EMasterPacketTypes : uint8_t
{
    None = 0,
    ReadAllSensors,
    TurnLightsON,
    TurnLightsOFF,
    ToggleAirConditioning,
    TurnStepperMotorCW, //TODO, see if you can transmit also new settings, like speed and step size.
    TurnStepperMotorCCW
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
    uint8_t roomLightSwitchState; // ON/OFF  1 = OFF, 0 = ON
    int16_t humidiySensor; // H
    int16_t temperatureSensor; // Celsius
    int32_t lightSensor; // Lux
    SRealTimeClockDateTime rtcDateTime;

    SPacketAllSensors()
    {
        type = EClientPacketTypes::AllSensorData;
    }
};

#endif