#pragma once
#include <Arduino.h>

/* ===== DISK CONFIG ===== */
#define SECTOR_SIZE 512
#define SECTOR_COUNT 64 // 32 KB
/* ====================== */

/* FAT12 layout */
#define FAT_START 1
#define FAT_SIZE 1
#define ROOT_START 3
#define ROOT_SIZE 1
#define DATA_START 4

#define INPUT_BUF_SIZE 512