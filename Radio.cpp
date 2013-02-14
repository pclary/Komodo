#include <WProgram.h>
#include "Radio.h"
#include "nRF24L01P_defs.h"
#include "PITimer.h"
#include "SPI.h"



namespace Radio
{

    // Private declarations
    int getRegister(int address);
    int getStatus();
    void setRegister(int address, int data);
    void receive();
    void clear();

    int _csnPin;
    int _cePin;
    int _irqPin;
    int _controller;
    unsigned rx_robot_pos;
    uint32_t rx_controller;
    uint32_t rx_robot[RX_BUFFER_SIZE];



    void initialize(int csnPin, int cePin, int irqPin, int controller)
    {
        // Set up pins
        _csnPin = csnPin;
        _cePin = cePin;
        _irqPin = irqPin;
        pinMode(_csnPin, OUTPUT);
        pinMode(_cePin, OUTPUT);
        pinMode(_csnPin, INPUT);

        // Disable nRF24L01+
        digitalWrite(_cePin, 0);
        
        // Disable chip select
        digitalWrite(_csnPin, 1);
        
        // Set up SPI
        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV16);
        
        _controller = controller;
        
        // Set up IRQ
        attachInterrupt(irqPin, receive, FALLING);
        
        // Set up clear timer
        PITimer0.period(0.5f);
    }



    void reset()
    {
        // Wait for power on reset
        delayMicroseconds(TIMING_Tpor);

        // Put into standby
        digitalWrite(_cePin, 0);
        
        // Configure registers
        setRegister(CONFIG, CONFIG_MASK_TX_DS | CONFIG_MASK_MAX_RT | CONFIG_EN_CRC | CONFIG_PWR_UP | CONFIG_PRIM_RX);
        setRegister(EN_AA, 0x00);
        setRegister(EN_RXADDR, ERX_P0 | ERX_P1);
        setRegister(SETUP_AW, SETUP_AW_3BYTES);
        setRegister(SETUP_RETR, 0x00);
        setRegister(RF_CH, RF_CHANNEL);
        setRegister(RF_SETUP, RF_SETUP_RF_DR_HIGH | RF_SETUP_RF_PWR_0);
        setRegister(STATUS, STATUS_RX_DR | STATUS_TX_DS | STATUS_MAX_RT);
        setRegister(RX_PW_P0, 4);
        setRegister(RX_PW_P1, 4);
        setRegister(DYNPD, 0x00);
        setRegister(FEATURE, 0x00);
        
        // Set addresses
        digitalWrite(_csnPin, 0);
        SPI.transfer(W_REGISTER | RX_ADDR_P0);
        SPI.transfer(CTRL_BASE_ADDRESS_1 + (_controller & 0xf));
        SPI.transfer(CTRL_BASE_ADDRESS_2);
        SPI.transfer(CTRL_BASE_ADDRESS_3);
        digitalWrite(_csnPin, 1);
        digitalWrite(_csnPin, 0);
        SPI.transfer(W_REGISTER | RX_ADDR_P1);
        SPI.transfer(ROBOT_ADDRESS_1);
        SPI.transfer(ROBOT_ADDRESS_2);
        SPI.transfer(ROBOT_ADDRESS_3);
        digitalWrite(_csnPin, 1);
        digitalWrite(_csnPin, 0);
        SPI.transfer(W_REGISTER | TX_ADDR);
        SPI.transfer(ROBOT_ADDRESS_1);
        SPI.transfer(ROBOT_ADDRESS_2);
        SPI.transfer(ROBOT_ADDRESS_3);
        digitalWrite(_csnPin, 1);
        
        // Put into PRX
        digitalWrite(_cePin, 1);
        delayMicroseconds(TIMING_Tstby2a);
        
        // Flush FIFOs
        digitalWrite(_csnPin, 0);
        SPI.transfer(FLUSH_TX);
        digitalWrite(_csnPin, 1);
        digitalWrite(_csnPin, 0);
        SPI.transfer(FLUSH_RX);
        digitalWrite(_csnPin, 1);
    }



    void transmit(uint32_t data)
    {
        // Put into standby
        digitalWrite(_cePin, 0);
        
        // Configure for PTX
        int config = getRegister(CONFIG);
        config &= ~CONFIG_PRIM_RX;
        setRegister(CONFIG, config);
        
        // Write packet data
        digitalWrite(_csnPin, 0);
        SPI.transfer(W_TX_PAYLOAD);
        SPI.transfer( (data>>0) & 0xff );
        SPI.transfer( (data>>8) & 0xff );
        SPI.transfer( (data>>16) & 0xff );
        SPI.transfer( (data>>24) & 0xff );
        digitalWrite(_csnPin, 1);
        
        // Put into PTX
        digitalWrite(_cePin, 1);
        delayMicroseconds(TIMING_Tstby2a);
        digitalWrite(_cePin, 0);
        
        // Wait for message transmission and put into PRX
        delayMicroseconds(TIMING_Toa);
        config = getRegister(CONFIG);
        config |= CONFIG_PRIM_RX;
        setRegister(CONFIG, config);
        setRegister(STATUS, STATUS_TX_DS);
        digitalWrite(_cePin, 1);
    }



    int getRegister(int address)
    {
        digitalWrite(_csnPin, 0);
        int rc = R_REGISTER | (address & REGISTER_ADDRESS_MASK);
        SPI.transfer(rc);
        int data = SPI.transfer(NOP);
        digitalWrite(_csnPin, 1);
        return data;
    }



    int getStatus()
    {
        digitalWrite(_csnPin, 0);
        int status = SPI.transfer(NOP);
        digitalWrite(_csnPin, 1);
        return status;
    }



    void setRegister(int address, int data)
    {
        bool enabled = false;
        if (digitalRead(_cePin) == 1)
        {
            enabled = true;
            digitalWrite(_cePin, 0);
        }
        
        digitalWrite(_csnPin, 0);
        int rc = W_REGISTER | (address & REGISTER_ADDRESS_MASK);
        SPI.transfer(rc);
        SPI.transfer(data & 0xff);
        digitalWrite(_csnPin, 1);
        
        if (enabled)
        {
            digitalWrite(_cePin, 1);
            delayMicroseconds(TIMING_Tpece2csn);
        }
        
    }



    void receive()
    {
        uint32_t data = 0;
        int pipe;
        
        while (!(getRegister(FIFO_STATUS) & FIFO_STATUS_RX_EMPTY))
        {
            // Check data pipe
            pipe = getStatus() & STATUS_RN_P_MASK;
            
            // Read data
            digitalWrite(_csnPin, 0);
            SPI.transfer(R_RX_PAYLOAD);
            data |= SPI.transfer(NOP)<<0;
            data |= SPI.transfer(NOP)<<8;
            data |= SPI.transfer(NOP)<<16;
            data |= SPI.transfer(NOP)<<24;
            digitalWrite(_csnPin, 1);
            
            // Sort into recieve buffer
            switch(pipe)
            {
            case STATUS_RN_P_NO_P0:
                rx_controller = data;
                PITimer0.start(&clear);
                break;
                
            case STATUS_RN_P_NO_P1:
                rx_robot[rx_robot_pos++ % RX_BUFFER_SIZE] = data;
                break;
                
            default:
                break;
            }
        }
        
        // Reset IRQ pin
        setRegister(STATUS, STATUS_RX_DR);
    }



    void clear()
    {
        rx_controller = 0;
        PITimer0.stop();
    }
    
}
