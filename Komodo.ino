#include <WProgram.h>
#include "drv8835.h"
#include "Radio.h"
#include "./Servo.h"
#include "CircularBuffer.h"



float xaxis, yaxis, turnaxis;
Motor legRight(5, 7);
Motor legLeft(6, 8);
Motor armRight(20, 18);
Motor armLeft(21, 19);
Servo waist(3);
int cycleCount = 0;
const int period = 20; // ms

enum PunchState
{
	idle,
	punching,
	out,
	retreating
};

float punchTimeRight = 0.0f;
float punchTimeLeft = 0.0f;
const float punchPeriod = 7.0f; // cycles
PunchState rightState = idle;
PunchState leftState = idle;
const float punchSpeed = -1.0f;
const float retreatSpeed = 0.5f;
bool rpunch, lpunch;


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
	
	Serial.begin(115200);
}



void loop()
{
    while (millis() < cycleCount * period);
    cycleCount++;
    
    float xlast = xaxis;
	float ylast = yaxis;
	float turnlast = turnaxis;
	
	xaxis = 0.0078125f * deadzone((int8_t)((Radio::rx_controller>>0)&0xff), 8); // Convert to +/-1.0f range
    yaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>8)&0xff), 8);
    turnaxis = -0.0078125f * deadzone((int8_t)((Radio::rx_controller>>16)&0xff), 8);
	rpunch = (Radio::rx_controller>>24)&0x1;
	lpunch = (Radio::rx_controller>>25)&0x1;
	
	xaxis = xlast * 0.7f + xaxis * 0.3f;
	yaxis = ylast * 0.7f + yaxis * 0.3f;
    if (fabs(turnaxis - turnlast) > 0.05f)
		turnaxis = turnlast * 0.85f + turnaxis * 0.15f;
	
    legRight = -0.5f * (yaxis + xaxis) * 0.7f;
    legLeft = 0.5f * (yaxis - xaxis) * 0.7f;
    waist = turnaxis * -60.0f;
	
	Serial.println(Radio::rx_controller);
    
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
		punchTimeRight += fabs(punchSpeed);
		if (punchTimeRight > punchPeriod)
		{
			rightState = out;
			armRight = 0.0f;
			punchTimeRight = punchPeriod;
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
		punchTimeRight -= fabs(retreatSpeed);
		if (rpunch)
		{
			rightState = punching;
			armRight = punchSpeed;
		}
		else if (punchTimeRight < 0.0f)
		{
			rightState = idle;
			armRight = 0.0f;
			punchTimeRight = 0.0f;
		}
		break;
	}
	
	switch (leftState)
	{
	case idle:
		if (lpunch)
		{
			leftState = punching;
			punchTimeLeft = 0;
			armLeft = punchSpeed;
		}
		break;
		
	case punching:
		punchTimeLeft += fabs(punchSpeed);
		if (punchTimeLeft > punchPeriod)
		{
			leftState = out;
			armLeft = 0.0f;
			punchTimeLeft = 0.0f;
		}
		break;
	
	case out:
		if (!lpunch)
		{
			leftState = retreating;
			punchTimeLeft = punchPeriod;
			armLeft = retreatSpeed;
		}
		break;
	
	case retreating:
		punchTimeLeft -= fabs(retreatSpeed);
		if (lpunch)
		{
			leftState = punching;
			armLeft = punchSpeed;
		}
		else if (punchTimeLeft < 0)
		{
			leftState = idle;
			armLeft = 0.0f;
			punchTimeLeft = 0.0f;
		}
		break;
	}
}
