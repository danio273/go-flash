#pragma once
#include <Arduino.h>
#include "disk_config.h"

class FatDisk
{
public:
    FatDisk();

    uint8_t *sector(uint32_t lba);
    void initFat12();

    bool detectGoFile(uint32_t lba, uint8_t *buffer, uint32_t size);
    void deleteGoFile();
    void writeOutputFile(const char *data, size_t len);
    size_t readInputFile(char *out, size_t maxLen);

private:
    uint8_t _disk[SECTOR_COUNT][SECTOR_SIZE];
};