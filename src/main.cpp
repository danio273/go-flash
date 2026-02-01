#include <Arduino.h>
#include "USB.h"
#include "USBMSC.h"

/* ===== DISK CONFIG ===== */
#define SECTOR_SIZE 512
#define SECTOR_COUNT 64 // 64 * 512 = 32 KB
/* ====================== */

USBMSC MSC;

/*
  FAT12 image (32 KB)
  Структура:
  - Boot sector
  - FAT x2
  - Root dir
  - Data
*/
static uint8_t disk[SECTOR_COUNT][SECTOR_SIZE];

/* FAT12 constants */
#define FAT_START 1
#define FAT_SIZE 1
#define ROOT_START 3
#define ROOT_SIZE 1
#define DATA_START 4

volatile bool go_flag = false;

#define INPUT_BUF_SIZE 512
char input_buf[INPUT_BUF_SIZE + 1];
size_t input_len = 0;

/* ===== CALLBACKS ===== */

int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
  memcpy(buffer, disk[lba] + offset, bufsize);
  return bufsize;
}

int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
  memcpy(disk[lba] + offset, buffer, bufsize);

  // дуже проста детекція GO.TXT
  if (lba == ROOT_START)
  {
    if (memmem(buffer, bufsize, "GO      TXT", 11))
    {
      go_flag = true;
    }
  }
  return bufsize;
}

bool onStartStop(uint8_t power, bool start, bool eject)
{
  return true;
}

/* ===== FAT INIT ===== */

void init_fat()
{
  memset(disk, 0, sizeof(disk));

  // --- BOOT SECTOR ---
  uint8_t *b = disk[0];
  b[0x00] = 0xEB;
  b[0x01] = 0x3C;
  b[0x02] = 0x90;
  memcpy(&b[0x03], "MSDOS5.0", 8);
  b[0x0B] = 0x00;
  b[0x0C] = 0x02; // 512 bytes/sector
  b[0x0D] = 0x01; // sectors/cluster
  b[0x0E] = 0x01;
  b[0x0F] = 0x00; // reserved
  b[0x10] = 0x02; // FATs
  b[0x11] = 0x10;
  b[0x12] = 0x00; // root entries = 16
  b[0x13] = SECTOR_COUNT & 0xFF;
  b[0x14] = (SECTOR_COUNT >> 8) & 0xFF;
  b[0x15] = 0xF8;
  b[0x16] = FAT_SIZE;
  b[0x17] = 0x00;
  b[0x1FE] = 0x55;
  b[0x1FF] = 0xAA;

  // --- FAT ---
  disk[FAT_START][0] = 0xF8;
  disk[FAT_START][1] = 0xFF;
  disk[FAT_START][2] = 0xFF;

  memcpy(disk[FAT_START + FAT_SIZE],
         disk[FAT_START],
         SECTOR_SIZE);

  // --- ROOT DIR ---
  uint8_t *r = disk[ROOT_START];

  // INPUT.TXT
  memcpy(&r[0], "INPUT   TXT", 11);
  r[11] = 0x20;
  r[26] = 0x02;
  r[27] = 0x00; // cluster 2
  r[28] = 0x00;
  r[29] = 0x00;

  // FAT cluster for INPUT
  disk[FAT_START][3] = 0xFF;
  disk[FAT_START][4] = 0x0F;
}

/* ===== SETUP ===== */

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  init_fat();

  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.onStartStop(onStartStop);

  MSC.begin(SECTOR_COUNT, SECTOR_SIZE);
  MSC.mediaPresent(true);

  USB.begin();
}

/* ===== LOOP ===== */

void loop()
{
  if (!go_flag)
    return;
  go_flag = false;

  // eject
  delay(10);
  MSC.mediaPresent(false);

  // ===== ТУТ АЛГОРИТМ (обробка тексту з інпут) =====

  memset(input_buf, 0, sizeof(input_buf));
  input_len = 0;

  uint8_t *data = disk[DATA_START]; // cluster 2

  for (int i = 0; i < INPUT_BUF_SIZE; i++)
  {
    char c = data[i];
    if (c == 0x00)
      break;
    input_buf[input_len++] = c;
  }

  Serial.println("INPUT.TXT cached:");
  Serial.write((uint8_t *)input_buf, input_len);
  Serial.println("\n----");

  delay(5000);
  // =================================

  // ===== WRITE OUTPUT.TXT =====
  uint8_t *r = disk[ROOT_START];
  memcpy(&r[32], "OUTPUT  TXT", 11);
  r[32 + 11] = 0x20;
  r[32 + 26] = 0x03;
  r[32 + 27] = 0x00;

  memcpy(disk[DATA_START + 1], input_buf, input_len);
  r[32 + 28] = input_len & 0xFF;
  r[32 + 29] = (input_len >> 8) & 0xFF;

  // ===== DELETE GO.TXT =====
  for (int i = 0; i < 16 * 32; i += 32)
  {
    if (!memcmp(&r[i], "GO      TXT", 11))
    {
      r[i] = 0xE5;
    }
  }

  // reconnect
  MSC.mediaPresent(true);
}