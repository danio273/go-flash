#include <Arduino.h>

#include "fat_disk.h"
#include "msc_device.h"

FatDisk disk;
MscDevice msc(disk);

void setup()
{
  disk.initFat12();

  msc.begin();
}

void loop()
{
  if (!msc.goFlag)
    return;

  delay(50);
  msc.eject();

  msc.goFlag = false;
  disk.deleteGoFile();

  char buffer[INPUT_BUF_SIZE + 1];
  memset(buffer, 0, sizeof(buffer));
  size_t len = disk.readInputFile(buffer, INPUT_BUF_SIZE);

  disk.writeOutputFile(buffer, len);

  delay(2000);
  msc.reconnect();
}