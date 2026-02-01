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

void FatDisk::setFat12Entry(uint8_t *fat, uint16_t index, uint16_t value)
{
    uint32_t byteIndex = (index * 3) / 2;
    if ((index & 1) == 0)
    {
        fat[byteIndex] = value & 0xFF;
        fat[byteIndex + 1] = (fat[byteIndex + 1] & 0xF0) | ((value >> 8) & 0x0F);
    }
    else
    {
        fat[byteIndex] = (fat[byteIndex] & 0x0F) | ((value << 4) & 0xF0);
        fat[byteIndex + 1] = (value >> 4) & 0xFF;
    }
}

void FatDisk::initFat12()
{
    memset(_disk, 0, sizeof(_disk));

    /* ===== BOOT SECTOR ===== */
    uint8_t *b = _disk[0];
    b[0] = 0xEB;
    b[1] = 0x3C;
    b[2] = 0x90;
    memcpy(&b[3], "MSDOS5.0", 8);

    b[0x0B] = 0x00;
    b[0x0C] = 0x02; // 512 bytes
    b[0x0D] = 0x01; // sectors/cluster
    b[0x0E] = 0x01;
    b[0x0F] = 0x00;

    b[0x10] = 0x02; // FATs
    b[0x11] = 0x10; // root entries = 16
    b[0x12] = 0x00;

    b[0x13] = SECTOR_COUNT & 0xFF;
    b[0x14] = (SECTOR_COUNT >> 8) & 0xFF;

    b[0x15] = 0xF8;
    b[0x16] = FAT_SIZE;
    b[0x17] = 0x00;

    b[0x1FE] = 0x55;
    b[0x1FF] = 0xAA;

    /* ===== FAT ===== */
    uint8_t *fat = _disk[FAT_START];
    memset(fat, 0, SECTOR_SIZE);

    setFat12Entry(fat, 0, 0xFF8);
    setFat12Entry(fat, 1, 0xFFF);

    uint16_t firstCluster = 2;
    for (uint16_t i = 0; i < DATA_CLUSTERS; i++)
    {
        uint16_t c = firstCluster + i;
        if (i < DATA_CLUSTERS - 1)
            setFat12Entry(fat, c, c + 1);
        else
            setFat12Entry(fat, c, 0xFFF);
    }

    memcpy(_disk[FAT_START + FAT_SIZE], fat, SECTOR_SIZE);

    /* ===== ROOT DIR ===== */
    uint8_t *r = _disk[ROOT_START];
    memset(r, 0, SECTOR_SIZE);

    memcpy(&r[0], "DATA    TXT", 11);
    r[11] = 0x20;
    r[26] = firstCluster & 0xFF;
    r[27] = (firstCluster >> 8) & 0xFF;

    uint32_t size = DATA_CLUSTERS * SECTOR_SIZE;
    r[28] = size & 0xFF;
    r[29] = (size >> 8) & 0xFF;
    r[30] = (size >> 16) & 0xFF;
    r[31] = (size >> 24) & 0xFF;

    /* ===== DATA ===== */
    for (uint16_t i = 0; i < DATA_CLUSTERS; i++)
        memset(_disk[DATA_START + i], 0, SECTOR_SIZE);
}

bool FatDisk::detectGoFile(uint32_t lba, uint8_t *buffer, uint32_t size)
{
    if (lba == ROOT_START && memmem(buffer, size, "GO      TXT", 11))
        return true;
    return false;
}

size_t FatDisk::readData(char *out, size_t maxLen)
{
    uint8_t *r = _disk[ROOT_START];
    uint32_t size = r[28] | (r[29] << 8) | (r[30] << 16) | (r[31] << 24);
    if (size > maxLen)
        size = maxLen;

    size_t pos = 0;
    for (uint16_t i = 0; i < DATA_CLUSTERS && pos < size; i++)
    {
        size_t chunk = SECTOR_SIZE;
        if (pos + chunk > size)
            chunk = size - pos;
        memcpy(out + pos, _disk[DATA_START + i], chunk);
        pos += chunk;
    }
    return pos;
}

void FatDisk::writeData(const char *data, size_t len)
{
    size_t maxSize = DATA_CLUSTERS * SECTOR_SIZE;
    if (len > maxSize)
        len = maxSize;

    for (uint16_t i = 0; i < DATA_CLUSTERS; i++)
        memset(_disk[DATA_START + i], 0, SECTOR_SIZE);

    size_t pos = 0;
    for (uint16_t i = 0; i < DATA_CLUSTERS && pos < len; i++)
    {
        size_t chunk = SECTOR_SIZE;
        if (pos + chunk > len)
            chunk = len - pos;
        memcpy(_disk[DATA_START + i], data + pos, chunk);
        pos += chunk;
    }

    uint8_t *r = _disk[ROOT_START];
    r[28] = len & 0xFF;
    r[29] = (len >> 8) & 0xFF;
    r[30] = (len >> 16) & 0xFF;
    r[31] = (len >> 24) & 0xFF;
}

void FatDisk::deleteGoFile()
{
    uint8_t *r = _disk[ROOT_START];
    for (int i = 0; i < 16 * 32; i += 32)
        if (!memcmp(&r[i], "GO      TXT", 11))
            r[i] = 0xE5;
}
