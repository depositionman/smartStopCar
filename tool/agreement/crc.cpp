#include "crc.h"
#include <string.h>

CRC::CRC()
{
    memset(crc32_table,0,sizeof (crc32_table));
    generate_crc32_table();
}

CRC* CRC::crc = nullptr;
CRC *CRC::getinterface()
{
    if(CRC::crc == nullptr){
        CRC::crc = new CRC;
    }
    return CRC::crc;
}

void CRC::generate_crc32_table()
{
    uint32_t poly = 0xEDB88320; // 逆序多项式
    for (uint32_t i = 0; i < 256; i++) {  // 针对所有可能的字节
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {  // 对每个字节的8个位进行处理
            if (crc & 1)
                crc = (crc >> 1) ^ poly;  // 如果最低位是1，则与多项式异或
            else
                crc >>= 1;  // 否则仅右移
        }
        crc32_table[i] = crc;  // 将结果存入查找表
    }
}

// 计算给定数据的CRC32校验值
uint32_t CRC::CRC32(const char* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF; // 初始值

    for (size_t i = 0; i < len; ++i) {
        uint8_t byte = (uint8_t)data[i];
        uint8_t table_index = (crc ^ byte) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[table_index];
    }

    return crc ^ 0xFFFFFFFF; // 最终的CRC32值
}
