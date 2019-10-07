/*

    Model for the HD6309 machine
    Copyright N.A. Moseley 2019.
    All rights reserved

    * Paged memory is from 0x0000 - 0x8000
    * SC16C550 UART lives at 0xE000.
    * ROM starts at 0xF000.
    * Memory map register at 0xE800.
    * 8-bit expansion port at 0xE010.
    * Disk I/O is at 0xE020.
*/

#ifndef machine_h
#define machine_h

#include <stdint.h>
#include <unistd.h>
#include <mutex>

#include "mc6809.h"
#include "uart.h"
#include "diskio.h"

class Machine : public mc6809
{
public:
    Machine();
    virtual ~Machine();

    /** load a 4k binary ROM file into 0xF000 .. 0xFFFF */
    bool loadRom(const char *filename);
    
    /** load a HEX file into RAM */
    bool loadHex(const char *filename);

    /** returns true if the UART will accept
        data via submitSerialChar()
    */
    bool clearToSend()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return m_uart.clearToSend();
    }

    /** place a character in the UARTs receive buffer
        so the 6809 system will read it.
    */
    void submitSerialChar(uint8_t c)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_uart.submitSerialChar(c);
    }    

    virtual void execute() override
    {
        std::unique_lock<std::mutex> locker(m_mutex);

        if (static_cast<int32_t>(pc) == m_breakpoint)
        {
            //halt();
            m_debug = true;
        }
        
        if (m_debug)
        {
            printf("PC: %04X -> %02X\n\tSP: %04X\tA: %02X\tB: %02X\n", pc, read(pc), s, (int32_t)a, (int32_t)b);
            printf("\tDD: %04X\tX : %04X\tY: %04X\n", d, x, y);
            //printf("\tHEX: %02X%02X\n", read(0xDEFC+1), read(0xDEFC));
            mc6809::execute();
            usleep(1000*250);
        }
        else
        {
            mc6809::execute();
        }
    }

    uint32_t getPC()
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        return pc;
    }

    void setBreakpoint(int32_t breakpoint)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_breakpoint = breakpoint;
    }

    void debug(bool state)
    {
        std::unique_lock<std::mutex> locker(m_mutex);
        m_debug = state;
    }

protected:
    std::mutex m_mutex;

    bool    m_debug;
    bool    m_trace;
    uint8_t m_pagereg;

    int32_t m_breakpoint;

	virtual Byte read(Word) override;
	virtual void write(Word, Byte) override;
    virtual void status() override;

    Byte *m_memory;
    Byte m_rom[4096];

    UART m_uart;
    DiskIO m_diskio;
};

#endif