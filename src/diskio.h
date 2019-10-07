/*

    Simulator for the HD6309 computer
    Copyright N.A. Moseley 2019
    
    www.moseleyinstruments.com
    
    namoseley.wordpress.com

    Model for the fake disk I/O.

*/

#ifndef diskio_h
#define diskio_h

#include <stdint.h>
#include <vector>


/** fake disk I/O subsystem */
class DiskIO
{
public:
    DiskIO();

    void writeReg(uint8_t reg, uint8_t value);
    uint8_t readReg(uint8_t reg);

protected:
    uint32_t m_sectors; ///< number of sector per track
    uint32_t m_tracks;  ///< number of tracks per drive
    uint32_t m_drives;  ///< number of drives

    uint8_t m_track;    ///< track register
    uint8_t m_sector;   ///< sector register
    uint8_t m_drive;    ///< drive register
    uint8_t m_cmd;      ///< command register
    uint8_t m_stat;     ///< status register

    uint8_t m_byteIdx;  ///< current byte index within sector

    std::vector<uint8_t>    m_data; ///< the drive data

    static constexpr uint8_t IDLECMD = 0;
    static constexpr uint8_t READSECTOR = 1;
    static constexpr uint8_t WRITESECTOR = 2;
    static constexpr uint8_t SEEKSECTOR = 3;
    static constexpr uint8_t MAXCMD = 4;
};

#endif
