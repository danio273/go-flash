#pragma once
#include <Arduino.h>

#define SECTOR_SIZE 512

/*
 Boot  : 1 sector
 FAT1  : 1 sector
 FAT2  : 1 sector
 Root  : 1 sector
 Data  : 16 sectors (8 KB)
 Total : 20 sectors
*/
#define SECTOR_COUNT 20

/* FAT12 layout */
#define FAT_START 1
#define FAT_SIZE 1
#define ROOT_START 3
#define ROOT_SIZE 1
#define DATA_START 4

/* DATA.TXT = 8192 bytes */
#define DATA_BUF_SIZE 8192
