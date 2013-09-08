#include "drv8835.h"
#include <WProgram.h>
#include <cmath>
#include "Hardware.h"



Motor::Motor(int speedPin, int phasePin)
{
    this->speedPin = speedPin;
    this->phasePin = phasePin;
    
    pinMode(speedPin, OUTPUT);
    pinMode(phasePin, OUTPUT);
    
    // Set PWM frequency to 25kHz
    analogWriteFrequency(speedPin, 25000);
    pwmWrite(speedPin, 0.0);
}



void Motor::write(float value)
{
    // Set direction
    if (value < 0.0f)
    {
        digitalWrite(phasePin, 1);
    }
    else
    {
        digitalWrite(phasePin, 0);
    }
    
    // Set speed
    pwmWrite(speedPin, fabs(value));
}



Motor& Motor::operator=(float value)
{
    write(value);
    return *this;
}
