/*

    Model for the HD6309 machine
    Copyright N.A. Moseley 2019.
    All rights reserved

    * Paged memory is from 0x0000 - 0x8000
    * SC16C550 UART lives at 0xE000.
    * ROM starts at 0xF000.
    * Memory map register at 0xE800.
    * 8-bit expansion port at 0xE010.

*/

#include <stdio.h>
#include "machine.h"

Machine::Machine()
{
    m_trace = false;
    m_debug = false;
    m_pagereg = 0;
    m_breakpoint = -1;
    m_memory = new Byte[1024*1024]; // 1 megabyte of memory!
}

Machine::~Machine()
{
    delete[] m_memory;
}

bool Machine::loadRom(const std::string &filename)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    FILE *fin = fopen(filename.c_str(), "rb");
    if (fin != 0)
    {
        size_t bytes = fread(m_rom, 1, sizeof(m_rom), fin);
        fclose(fin);
        return true;
    }
    return false;
}

Byte Machine::read(Word address)
{
    if (address < 0x8000)
    {
        // paged RAM area
        uint32_t addr = static_cast<uint32_t>(m_pagereg & 31) << 15;
        addr |= address;
        return m_memory[addr];
    }
    else if (address < 0xE000)
    {
        // non-paged RAM area
        return m_memory[address];
    }
    else if (address < 0xF000)
    {
        // peripheral area
        if ((address >= 0xE000) && (address < 0xE010))
        {
            return m_uart.read(address - 0xE000);
        }
        else if ((address >= 0xE020) && (address < 0xE030))
        {
            return m_diskio.readReg(address - 0xE020);
        }
        return 0;
    }
    else 
    {
        // ROM area
        return m_rom[address-0xF000];
    }
}

void Machine::write(Word address, Byte value)
{
    if (address < 0x8000)
    {
        // paged RAM area
        uint32_t addr = static_cast<uint32_t>(m_pagereg & 31) << 15;
        addr |= address;
        m_memory[addr] = value;
    }
    else if (address < 0xE000)
    {
        // non-paged RAM area
        m_memory[address] = value;
    }
    else if (address < 0xF000)
    {
        // peripheral area
        if ((address >= 0xE000) && (address < 0xE010))
        {
            m_uart.write(address - 0xE000, value);
        }
        else if ((address >= 0xE020) && (address < 0xE030))
        {
            m_diskio.writeReg(address - 0xE020, value);
        }        
        else if (address == 0xE800)
        {
            m_pagereg = value;
        }
    }
    else 
    {
        // ROM area, so do nothing..
    }
}

void Machine::status()
{
    //printf("PC = %04X -> %02X\n", pc, read(pc));
}


static uint32_t hexchar(uint8_t c)
{
    if ((c>='0') && (c<='9'))
    {
        return c-'0';
    }
    if ((c>='A') && (c<='F'))
    {
        return c-'A'+10;
    }
    if ((c>='a') && (c<='f'))
    {
        return c-'a'+10;
    }

    return 0;
}

bool Machine::loadHex(const std::string &filename)
{
    std::unique_lock<std::mutex> locker(m_mutex);

    int state = 0;
    uint32_t address;
    uint8_t data;

    printf("Loading %s\n", filename.c_str());

    FILE *fin = fopen(filename.c_str(), "rt");
    if (fin == 0)
    {
        return false;
    }

    while(!feof(fin))
    {
        uint8_t c = fgetc(fin);

        // skip tabs, spaces and carriage returns
        if ((c == '\t') || (c == '\r') || (c == ' '))
        {
            continue;
        }

        // if newline, reset the interpreter/state machine
        if (c == '\n')
        {
            state = 0;
            continue;
        }

        switch(state)
        {
        case 0: // read hex address
            address = hexchar(c);
            state++;
            break;
        case 1:
            address <<= 4;
            address |= hexchar(c);
            state++;
            break;
        case 2:
            address <<= 4;
            address |= hexchar(c);
            state++;
            break;
        case 3:
            address <<= 4;
            address |= hexchar(c);
            //printf("Address: 0x%04X\n", address);
            state++;
            break;
        case 4: // expect ':'
            if (c != ':')
            {
                printf("Expected ':' but got %c -- aborting.\n", c);
                return -1;
            }
            state++;
            break;
        case 5: 
            data = hexchar(c);
            state++;
            break;
        case 6:
            data <<= 4;
            data |= hexchar(c);
            if (address < 0xE000)
            {
                //if ((address >= 0xCD00) && (address <= 0xCE00))
                //{
                //    printf("  %02X\n", data);
                //}
                m_memory[address] = data;
            }
            else if (address >= 0xF000)
            {
                m_rom[address-0xF000] = data;
            }
            state++;
            address++;
            break;
        case 7:
            // expect a comma
            if (c == ',')
            {
                state = 5;
            }
            else
            {
                state = 0;
            }
            break;
        default:
            printf("Error: unknown char %c\n", c);
            fclose(fin);
            return false;
        }
    }

    fclose(fin);
    return true;
}

bool Machine::mountDisk(uint8_t drive, const std::string &filename)
{
    std::unique_lock<std::mutex> locker(m_mutex);
    return m_diskio.loadImage(drive, filename);
}
