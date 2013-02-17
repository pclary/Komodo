#ifndef _DRV8835_H
#define _DRV8835_H



class Motor
{
public:
    Motor(int speedPin, int phasePin);
    void write(float value);
    Motor& operator=(float value);
    
private:
    int speedPin;
    int phasePin;
};

#endif // _DRV8835_H