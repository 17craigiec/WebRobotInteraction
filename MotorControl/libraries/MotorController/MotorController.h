#include "Arduino.h"

class MotorController
{
private:
    int m_ena_pin = -1;
    int m_in1_pin = -1;
    int m_in2_pin = -1;
public:
    MotorController(int ena_pin, int in1_pin, int in2_pin);
    ~MotorController();

    // Spd is a double bounded from -1 to 1
    void drive(double spd);
};
