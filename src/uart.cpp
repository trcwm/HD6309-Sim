/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Model for the SC16C550 UART

*/

#include <stdio.h>
#include "uart.h"

UART::UART()
{
    m_lcr = 0;
    m_status = 0;
    m_status |= 32; // transmit holding empty
    m_status |= 64; // transmit empty
}


void UART::write(uint8_t reg, uint8_t value)
{
#ifdef DEBUGGING
    printf("UART reg %02X <- %02X\n", (int32_t)reg, (int32_t)value);
#endif
    switch(reg)
    {
    case 0: // transmit hold register or DLL register
        if (m_lcr & 0x80)
        {
            // DLL register
        }
        else
        {
            printf("%c", value);
            fflush(stdout);
        }
        break;
    case 3: // LCR register
        m_lcr = value;
        break;
    case 5: // LSR register
        //m_status = value;
        break;
    default:
        break;
    }
}

bool UART::clearToSend() const
{
    return (m_status & 1) == 0;
}

uint8_t UART::read(uint8_t reg)
{
#ifdef DEBUGGING
    printf("UART reg %02X read\n", (int32_t)reg);
#endif    

    switch(reg)
    {
    case 0: // receive holding register
        m_status &= ~1; // set serial data ready to zero.
        return m_serialInputBuffer;
    case 3: // LCR register
        return m_lcr;
    case 5: // LSR register
        return m_status;
    default:
        return 0;
    }
}

void UART::submitSerialChar(uint8_t c)
{
    //TODO: implement FIFO
    if (m_status & 1)
    {
        // error: buffer overrun
        printf("Serial buffer overrun!\n");
    }
    else
    {
        m_serialInputBuffer = c;
        m_status |= 1; // receive data ready.
    }
}

