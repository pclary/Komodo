#include "Servo.h"
#include <WProgram.h>



Servo::Servo(int pin, bool start)
{
    pwmPin = pin;
    enabled = start;
    
    // Set up pwm
    pinMode(pwmPin, OUTPUT);
    analogWriteFrequency(pwmPin, 50);
    analogWriteResolution(16);
    
    // Set default calibration
    calibrate(2000, 1000, 60.f, -60.f);
    
    // Start at center position
    pulseWidth = center;
    analogWrite(pwmPin, (enabled ? (65535*pulseWidth)/20000 : 0));
}



bool Servo::calibrate(unsigned int plus45, unsigned int minus45, float upperLimit, float lowerLimit)
{
    // Check if given parameters are valid
    if (upperLimit > lowerLimit)
    {
        center = (plus45 + minus45) / 2;
        usPerDegree = ((int)plus45 - (int)center) / 45.0f;
        this->upperLimit = upperLimit;
        this->lowerLimit = lowerLimit;   
        return true;
    }
    else
    {
        return false;
    }
}



void Servo::write(float degrees)
{
    // Limit to the valid angle range
    degrees = (degrees > upperLimit ? upperLimit : (degrees < lowerLimit ? lowerLimit : degrees));
    
    pulseWidth = center + (int)(degrees * usPerDegree);
    analogWrite(pwmPin, (enabled ? (65535*pulseWidth)/20000 : 0));
}



float Servo::read()
{
    return ((int)pulseWidth - (int)center) / usPerDegree;
}



void Servo::disable()
{
    enabled = false;
    analogWrite(pwmPin, 0);
}



void Servo::enable()
{
    enabled = true;
    analogWrite(pwmPin, (65535*pulseWidth)/20000);
}



void Servo::operator=(float degrees)
{
    write(degrees);
}



Servo::operator float()
{
    return read();
}
