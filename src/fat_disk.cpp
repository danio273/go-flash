#include "fat_disk.h"
#include <string.h>

FatDisk::FatDisk()
{
    memset(_disk, 0, sizeof(_disk));
}

uint8_t *FatDisk::sector(uint32_t lba)
{
    return _disk[lba];
}

void FatDisk::initFat12()
{
    memset(_disk, 0, sizeof(_disk));

    /* ===== BOOT SECTOR ===== */
    uint8_t *b = _disk[0];
    b[0x00] = 0xEB;
    b[0x01] = 0x3C;
    b[0x02] = 0x90;
    memcpy(&b[0x03], "MSDOS5.0", 8);

    b[0x0B] = 0x00;
    b[0x0C] = 0x02; // 512 bytes/sector
    b[0x0D] = 0x01; // sectors/cluster
    b[0x0E] = 0x01;
    b[0x0F] = 0x00; // reserved

    b[0x10] = 0x02; // FAT count
    b[0x11] = 0x10;
    b[0x12] = 0x00; // root entries = 16

    b[0x13] = SECTOR_COUNT & 0xFF;
    b[0x14] = (SECTOR_COUNT >> 8) & 0xFF;

    b[0x15] = 0xF8;
    b[0x16] = FAT_SIZE;
    b[0x17] = 0x00;

    b[0x1FE] = 0x55;
    b[0x1FF] = 0xAA;

    /* ===== FAT TABLE ===== */
    _disk[FAT_START][0] = 0xF8;
    _disk[FAT_START][1] = 0xFF;
    _disk[FAT_START][2] = 0xFF;

    memcpy(_disk[FAT_START + FAT_SIZE],
           _disk[FAT_START],
           SECTOR_SIZE);

    /* ===== ROOT DIR ===== */
    uint8_t *r = _disk[ROOT_START];

    memcpy(&r[0], "INPUT   TXT", 11);
    r[11] = 0x20;
    r[26] = 0x02; // cluster 2
    r[27] = 0x00;

    /* FAT cluster for INPUT */
    _disk[FAT_START][3] = 0xFF;
    _disk[FAT_START][4] = 0x0F;
}

bool FatDisk::detectGoFile(uint32_t lba, uint8_t *buffer, uint32_t size)
{
    if (lba == ROOT_START)
    {
        if (memmem(buffer, size, "GO      TXT", 11))
        {
            return true;
        }
    }
    return false;
}

size_t FatDisk::readInputFile(char *out, size_t maxLen)
{
    uint8_t *data = _disk[DATA_START]; // cluster 2
    size_t len = 0;

    for (size_t i = 0; i < maxLen; i++)
    {
        char c = data[i];
        if (c == 0x00)
            break;
        out[len++] = c;
    }
    return len;
}

void FatDisk::writeOutputFile(const char *data, size_t len)
{
    uint8_t *r = _disk[ROOT_START];

    memcpy(&r[32], "OUTPUT  TXT", 11);
    r[32 + 11] = 0x20;
    r[32 + 26] = 0x03;
    r[32 + 27] = 0x00;

    memcpy(_disk[DATA_START + 1], data, len);

    r[32 + 28] = len & 0xFF;
    r[32 + 29] = (len >> 8) & 0xFF;
}

void FatDisk::deleteGoFile()
{
    uint8_t *r = _disk[ROOT_START];

    for (int i = 0; i < 16 * 32; i += 32)
    {
        if (!memcmp(&r[i], "GO      TXT", 11))
        {
            r[i] = 0xE5;
        }
    }
}