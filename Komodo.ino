#include <WProgram.h>
#include "drv8835.h"
#include "Radio.h"
#include "./Servo.h"



float xaxis, yaxis, turnaxis;
Motor legRight(5, 7);
Motor legLeft(6, 8);
Motor armRight(18, 20);
Motor armLeft(19, 21);
Servo waist(3);
int cycleCount = 0;
const int period = 20; // ms

enum PunchState
{
	idle;
	punching;
	out;
	retreating;
};

int punchTimeRight = 0;
int punchTimeLeft = 0;
const int punchPeriod; // cycles
PunchState rightState = idle;
PunchState leftState = idle;
const float punchSpeed = 1.0f;
const float retreatSpeed = -1.0f;



int deadzone(int input, int zone)
{
    if (input > zone) return input;
    else if (input < -zone) return input;
    else return 0;
}



void setup()
{
    Radio::initialize(10, 14, 15, 0);
    Radio::reset();
}



void loop()
{
    while (millis() < cycleCount * period);
    cycleCount++;
    
    xaxis = 0.0078125f * deadzone((int8_t)((Radio::rx_controller>>0)&0xff), 8); // Convert to +/-1.0f range
    yaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>8)&0xff), 8);
    turnaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>16)&0xff), 8);
	bool rpunch = (Radio::rx_controller>>24)&0x1;
	bool lpunch = (Radio::rx_controller>>25)&0x1;
    
    legRight = 0.5f * (yaxis + xaxis);
    legLeft = 0.5f * (yaxis - xaxis);
    waist = turnaxis * 60.0f;
    
    switch (rightState)
	{
	case idle:
		if (rpunch)
		{
			rightState = punching;
			punchTimeRight = 0;
			armRight = punchSpeed;
		}
		break;
		
	case punching:
		if (++punchTimeRight >= punchPeriod)
		{
			rightState = out;
			armRight = 0.0f;
		}
		break;
	
	case out:
		if (!rpunch)
		{
			rightState = retreating;
			punchTimeRight = punchPeriod;
			armRight = retreatSpeed;
		}
		break;
	
	case retreating:
		if (rpunch)
		{
			rightState = punching;
			armRight = punchSpeed;
		}
		else if (--punchTimeRight <= 0)
		{
			rightState = idle;
			armRight = 0.0f;
		}
		break;
	}
	
	switch (leftState)
	{
	case idle:
		if (lpunch)
		{
			leftState = punching;
			punchTimeleft = 0;
			armleft = punchSpeed;
		}
		break;
		
	case punching:
		if (++punchTimeleft >= punchPeriod)
		{
			leftState = out;
			armleft = 0.0f;
		}
		break;
	
	case out:
		if (!lpunch)
		{
			leftState = retreating;
			punchTimeleft = punchPeriod;
			armleft = retreatSpeed;
		}
		break;
	
	case retreating:
		if (lpunch)
		{
			leftState = punching;
			armleft = punchSpeed;
		}
		else if (--punchTimeleft <= 0)
		{
			leftState = idle;
			armleft = 0.0f;
		}
		break;
	}
}
