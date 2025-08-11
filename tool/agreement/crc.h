#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stdio.h>

class CRC
{
public: 
    static CRC* getinterface();
    void generate_crc32_table();
    uint32_t CRC32(const char* data, size_t len);
private:
    CRC();
    static CRC* crc;
    uint32_t crc32_table[256];
};

#endif // CRC_H
