#ifndef _RADIO_H
#define _RADIO_H

#include <cstdint>

#define RX_BUFFER_SIZE 4



namespace Radio
{
    void initialize(int csnPin, int cePin, int irqPin, int controller);
    void reset();
    void transmit(uint32_t data);
    
    extern volatile uint32_t rx_controller;
    extern volatile uint32_t rx_robot[RX_BUFFER_SIZE];
};



#endif // _RADIO_H