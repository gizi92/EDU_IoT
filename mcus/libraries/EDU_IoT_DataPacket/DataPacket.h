#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <Arduino.h>

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

#endif