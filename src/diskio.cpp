/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Model for the fake disk I/O.

*/

#include <stdio.h>
#include "diskio.h"

//#define DEBUGPRINT

DiskIO::DiskIO()
{
    // 89.6K Shugart Shugart SA 400
    m_tracks  = 35;
    m_sectors = 10;
    m_drives  = 4;

    m_data.resize(256*m_tracks*m_sectors*m_drives);

#if 0
    setByte(0,0,5,16+0, 'D');
    setByte(0,0,5,16+1, 'I');
    setByte(0,0,5,16+2, 'R');
    setByte(0,0,5,16+3, 0);
    setByte(0,0,5,16+4, 0);
    setByte(0,0,5,16+5, 0);
    setByte(0,0,5,16+6, 0);
    setByte(0,0,5,16+7, 0);
    setByte(0,0,5,16+8, 'C');
    setByte(0,0,5,16+9, 'M');
    setByte(0,0,5,16+10, 'D');
    setByte(0,0,5,16+11, 0);     // not used
    setByte(0,0,5,16+12, 0);     // not used
    setByte(0,0,5,16+13, 10);    // start track
    setByte(0,0,5,16+14, 0);     // start sector
    setByte(0,0,5,16+15, 10);    // end track
    setByte(0,0,5,16+16, 1);     // end sector
    setByte(0,0,5,16+17, 0);     // total sectors
    setByte(0,0,5,16+18, 1);     // total sectors
    setByte(0,0,5,16+19, 0);     // random file flag
    setByte(0,0,5,16+20, 0);     // not used
    setByte(0,0,5,16+21, 1);     // month
    setByte(0,0,5,16+22, 1);     // day
    setByte(0,0,5,16+23, 1);     // year

    // write the first disk to disk ;)
    // skip sector 0 because it doesn't exist.. 
    FILE *fout = fopen("test.dsk","wb");
    fwrite(&m_data[256], 1, 256*m_tracks*m_sectors, fout);
    fclose(fout);

#else
    FILE *fin = fopen("flex9.dsk","rb");
    if (fin != 0)
    {
        fread(&m_data[0], 1, 256*m_tracks*m_sectors, fin);
        fclose(fin);
        printf("Flex9 disk read!\n");
    }    
#endif

}

void DiskIO::setByte(uint32_t drive, 
    uint32_t track, 
    uint32_t sector, 
    uint32_t byteofs,
    uint8_t value)
{
    sector--;   // sectors start at 1
    size_t ofs = 256*(sector + m_sectors*(track + m_tracks*drive));
    
    m_data[ofs+byteofs] = value;
    //printf("        %d\n", (uint32_t)ofs);
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
            size_t ofs = 256*((uint32_t)(m_sector-1) + m_sectors*((uint32_t)m_track + m_tracks*(uint32_t)m_drive));
            uint8_t v = m_data[ofs+m_byteIdx];
#ifdef DEBUGPRINT            
            printf("DISKIO: data[%u]: %d\n", (uint32_t)m_byteIdx, (uint32_t)v);
#endif
            m_byteIdx++;
            return v;
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
#ifdef DEBUGPRINT
    printf("DISKIO: write cmd <- %d\n", value);
#endif    
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
#ifdef DEBUGPRINT
    printf("DISKIO: write data <- %d\n", value);
#endif        
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
#ifdef DEBUGPRINT
    printf("DISKIO: write drive select <- %d\n", value);
#endif        
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
#ifdef DEBUGPRINT
    printf("DISKIO: write track # <- %d\n", value);
#endif            
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
#ifdef DEBUGPRINT
    printf("DISKIO: write sector # <- %d\n", value);
#endif            
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
#ifdef DEBUGPRINT
    printf("DISKIO: ?? <- %d\n", value);
#endif            
        m_stat = 0xFF;
        break;
    }
}
