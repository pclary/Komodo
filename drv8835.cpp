#include "drv8835.h"
#include <cmath>



Motor::Motor(int speedPin, int phasePin)
{
    this->speedPin = speedPin;
    this->phasePin = phasePin;
    
    pinMode(speedPin, OUTPUT);
    pinMode(phasePin, OUTPUT);
    
    // Set PWM frequency to 40kHz
    analogWriteFrequency(speedPin, 40000);
    analogWrite(speedPin, 0);
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
    analogWrite(speedPin, 255 * fabs(value));
}



Motor& Motor::operator=(float value)
{
    write(value);
    return *this;
}
