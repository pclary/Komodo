#include "drv8835.h"
#include "Radio.h"



float xaxis, yaxis, turnaxis;
Motor motor(3, 2);
int cycleCount = 0;



int deadzone(int input, int zone)
{
    if (input > zone) return input;
    else if (input < -zone) return input;
    else return 0;
} // deadzone()



void setup()
{
    Radio::initialize(10, 14, 15, 0);
    Radio::reset();
}



void loop()
{
    while (millis() < cycleCount * 100);
    
    cycleCount++;
    
    xaxis = 0.0078125f * deadzone((int8_t)((Radio::rx_controller>>0)&0xff), 8); // Convert to +/-1.0f range
    yaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>8)&0xff), 8);
    turnaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>16)&0xff), 8);
    
    motor = yaxis;
}
