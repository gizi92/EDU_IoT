#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <Arduino.h>

class Peripheral
{
    public:
        Peripheral(const int pin)
        {
            m_pin = pin;
        }

        const int GetPin() const
        {
            return m_pin;
        }
    private:
        int m_pin;
};

class AnalogSensor : public Peripheral
{
    public:
        AnalogSensor(const int pin) : Peripheral(pin) {};
        const int GetLastReadValue() const
        {
            return m_value;
        }

        void Read()
        {
            m_value = analogRead(Peripheral::GetPin());
        }
    private:
        volatile int m_value;
};

class DigitalOutput : public Peripheral
{
    public:
        DigitalOutput(const int pin) : Peripheral(pin)
        {
            pinMode(Peripheral::GetPin(), OUTPUT);
            TurnOFF();
        }

        void TurnON()
        {
            digitalWrite(Peripheral::GetPin(), LOW);
            m_state = 0;
        }

        void TurnOFF()
        {
            digitalWrite(Peripheral::GetPin(), HIGH);
            m_state = 1;
        }

        const int GetState() const
        {
            return m_state;
        }
    private:
        int m_state;
};

#endif