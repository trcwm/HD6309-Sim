/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Model for the SC16C550 UART

*/

#ifndef uart_h
#define uart_h

//#define DEBUGGING
#include <stdint.h>

class UART
{
public:
    UART();

    /** submit a serial character from a console */
    void submitSerialChar(uint8_t c);

    /** returns true if data can be received by the UART */
    bool clearToSend() const;

    void    write(uint8_t reg, uint8_t value);
    uint8_t read(uint8_t reg);

protected:
    uint8_t m_serialInputBuffer;
    uint8_t m_status;
    uint8_t m_lcr;
};

#endif