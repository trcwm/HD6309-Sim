/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Model for the fake disk I/O.

*/

#include <stdio.h>
#include "diskio.h"

DiskIO::DiskIO()
{
    // 256K fake disks
    m_tracks  = 32;
    m_sectors = 32;
    m_drives  = 4;

    m_data.resize(256*m_tracks*m_sectors*m_drives);
}

uint8_t DiskIO::readReg(uint8_t reg)
{
    switch(reg)
    {
    case 0: // command register
        m_stat = 0;
        return m_cmd;
    case 1: // data register
        if (m_cmd == READSECTOR)
        {            
            m_stat = 0;
            size_t ofs = 256*(m_sector + m_sectors*(m_track + m_tracks*m_drive));
            return m_data[ofs+m_byteIdx++];
        }
        m_stat = 0xFF;  // error
        return 0;
    case 2: // drive select reg
        m_stat = 0;
        return m_drive;
    case 3: // track reg
        m_stat = 0;
        return m_track;
    case 4: // sector reg
        m_stat = 0;
        return m_sector;
    case 5: // status reg        
        return m_stat;
    default:
        m_stat = 0xFF;
        return 0;
    }
}

void DiskIO::writeReg(uint8_t reg, uint8_t value)
{
    switch(reg)
    {
    case 0: // command register
        m_cmd = value;
        m_byteIdx = 0;
        if (value < MAXCMD)
        {
            m_stat = 0;
        }
        else
        {
            m_stat = 0xFF;
        }
        break;
    case 1: // data register
        if (m_cmd == WRITESECTOR)
        {            
            size_t ofs = 256*(m_sector + m_sectors*(m_track + m_tracks*m_drive));
            m_data[ofs+m_byteIdx++] = value;
            m_stat = 0x00;
        }
        else
        {
            m_stat = 0xFF;
        }
        break;
    case 2: // drive select reg
        m_drive = value;
        m_byteIdx = 0;
        if (value < m_drives)
        {
            m_stat = 0;
        }
        else
        {
            m_stat = 0xFF;
        }
        break;
    case 3: // track reg
        m_track = value;
        m_byteIdx = 0;
        if (value < m_tracks)
        {
            m_stat = 0;
        }
        else
        {
            m_stat = 0xFF;
        }        
        break;
    case 4: // sector reg
        m_sector = value;
        m_byteIdx = 0;
        if (value < m_sectors)
        {
            m_stat = 0;
        }
        else
        {
            m_stat = 0xFF;
        }        
        break;
    default:
        m_stat = 0xFF;
        break;
    }
}
