#ifndef SERVO_H
#define SERVO_H



class Servo
{
public:
    Servo(int pin, bool start = true);
    bool calibrate(unsigned int plus45, unsigned int minus45, float upperLimit, float lowerLimit);
    void write(float degrees);
    float read();
    void disable();
    void enable();
    void operator=(float degrees);
    operator float();
    
    
    
protected:
    int pwmPin;
    unsigned int pulseWidth;
    bool enabled;
    
    unsigned int center;
    float usPerDegree;
    float upperLimit;
    float lowerLimit;
};

#endif // SERVO_H