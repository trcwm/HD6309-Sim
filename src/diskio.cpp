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

#pragma pack(push, 1)
struct SIR_t
{
  uint8_t volumeLabel[11];
  uint8_t volumeNum[2];       // Big endian!!
  uint8_t firstFreeTrack;
  uint8_t firstFreeSector;
  uint8_t lastFreeTrack;
  uint8_t lastFreeSector;
  uint8_t numFreeSectors[2];  // Big endian!!
  uint8_t month;
  uint8_t day;
  uint8_t year;
  uint8_t endTrack;
  uint8_t endSector;
};

struct DIR_t
{
  uint8_t name[8];
  uint8_t ext[3];
  uint16_t dummy;
  uint8_t startTrack;
  uint8_t startSector;
  uint8_t endTrack;
  uint8_t endSector;
  uint8_t totalSectorsHi;
  uint8_t totalSectorsLo;
  uint8_t flags;
  uint8_t dummy2;
  uint8_t month;
  uint8_t day;
  uint8_t year;
};

#pragma pack(pop)


DiskIO::DiskIO()
{
    // allocate the number of drives
    m_drives.resize(4);

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
    loadImage(0, "flex9.dsk");
    loadImage(1, "test.dsk");
    loadImage(2, "test.dsk");
    loadImage(3, "test.dsk");
#endif
}

bool DiskIO::loadImage(uint8_t drive, const std::string &filename)
{
    if (drive >= m_drives.size())
    { 
        // drive does not exist!
        return false;
    }

    FILE *fin = fopen(filename.c_str(),"rb");
    if (fin != 0)
    {
        fseek(fin, 0, SEEK_END);
        size_t bytes = ftell(fin);
        rewind(fin);

        if (bytes > 1024*1024*16)
        {
            // FLEX does not support more than 16MB!
            return false;
        }

        m_drives[drive].resize(bytes);
        fread(&m_drives[drive][0], 1, bytes, fin);
        fclose(fin);
        return true;
    } 
    return false;
}

bool DiskIO::getGeometry(uint8_t drive, uint8_t &tracks, uint8_t &sectors)
{
    if (drive < m_drives.size())
    {
        size_t ofs = 256*2 + 16;  // SIR record
        SIR_t *sir = (SIR_t*)&m_drives[drive][ofs];

        tracks  = sir->endTrack + 1;
        sectors = sir->endSector;

        return true;
    }
    return false;
}

void DiskIO::setByte(uint32_t drive, 
    uint32_t track, 
    uint32_t sector, 
    uint32_t byteofs,
    uint8_t value)
{
    uint8_t tracks, sectors;
    bool ok = getGeometry(drive, tracks, sectors);

    if (!ok)
    {
        return;
    }

    sector--;   // sectors start at 1
    size_t ofs = 256*(sector + sectors*track);
    
    m_drives[drive][ofs+byteofs] = value;
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

            uint8_t tracks, sectors;
            bool ok = getGeometry(m_drive, tracks, sectors);

            if (!ok) 
            {
                m_stat = 0xFF;
                return 0;
            }

            size_t ofs = 256*((uint32_t)(m_sector-1) + sectors*(uint32_t)m_track);
            uint8_t v = m_drives[m_drive][ofs+m_byteIdx];
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
            uint8_t tracks, sectors;
            bool ok = getGeometry(m_drive, tracks, sectors);

            if (!ok)
            {
                m_stat = 0xFF;
                return;
            }

            size_t ofs = 256*((m_sector-1) + sectors*m_track);
            m_drives[m_drive][ofs+m_byteIdx++] = value;
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
        break;
    case 3: // track reg
#ifdef DEBUGPRINT
    printf("DISKIO: write track # <- %d\n", value);
#endif            
        m_track = value;
        m_byteIdx = 0;
        break;
    case 4: // sector reg
#ifdef DEBUGPRINT
    printf("DISKIO: write sector # <- %d\n", value);
#endif            
        m_sector = value;
        m_byteIdx = 0;
        break;
    default:
#ifdef DEBUGPRINT
    printf("DISKIO: ?? <- %d\n", value);
#endif            
        m_stat = 0xFF;
        break;
    }
}
