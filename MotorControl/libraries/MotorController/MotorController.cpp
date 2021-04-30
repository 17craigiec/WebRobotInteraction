#include "MotorController.h"

MotorController::MotorController(int ena_pin, int in1_pin, int in2_pin)
{
    pinMode(ena_pin, OUTPUT);
    pinMode(in1_pin, OUTPUT);
    pinMode(in2_pin, OUTPUT);

    m_ena_pin = ena_pin;
    m_in1_pin = in1_pin;
    m_in2_pin = in2_pin;
}

MotorController::~MotorController()
{
}

void MotorController::drive(double spd)
{
    // Bound input
    if(spd > 1)
    {
        spd = 1;
    }
    if(spd < -1)
    {
        spd = -1;
    }
    
    if(spd > 0)
    {
        digitalWrite(m_in1_pin, HIGH);
        digitalWrite(m_in2_pin, LOW);
    }
    else
    {
        digitalWrite(m_in1_pin, LOW);
        digitalWrite(m_in2_pin, HIGH);
    }

    double written_val = 255*abs(spd);
    Serial.println(written_val);
    analogWite(m_ena_pin, int(val));
}