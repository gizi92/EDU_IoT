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
        Peripheral()
        {
            m_pin = -1; // N/A
        }
        const int GetPin() const
        {
            return m_pin;
        }
    private:
        int m_pin;
};
template<class T>
class AnalogSensor : public Peripheral
{
    public:
        AnalogSensor(const int pin) : Peripheral(pin) {};
        const T GetLastReadValue() const
        {
            return m_value;
        }

        void Read()
        {
            m_value = analogRead(Peripheral::GetPin());
        }
    private:
        T m_value;
};

template<class T>
class GenericSensor : public Peripheral
{
    public:
        GenericSensor(const T init_value) : Peripheral() { m_value = init_value; };
        const T GetLastReadValue() const
        {
            return m_value;
        }
        void SetValue(const T new_value)
        {
            m_value = new_value;
        }
    private:
        T m_value;
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

class DigitalInput : public Peripheral
{
    public:
        DigitalInput(const int pin) : Peripheral(pin)
        {
            pinMode(Peripheral::GetPin(), INPUT);
        }

        void ReadState()
        {
            m_state = digitalRead(Peripheral::GetPin());
        }

        const int GetLastReadValue() const
        {
            return m_state;
        }
    private:
        uint8_t m_state;
};
#endif