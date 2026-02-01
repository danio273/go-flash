#pragma once
#include <Arduino.h>
#include "USB.h"
#include "USBMSC.h"
#include "fat_disk.h"

class MscDevice
{
public:
    MscDevice(FatDisk &disk);

    void begin();
    void eject();
    void reconnect();

    volatile bool goFlag = false;

private:
    USBMSC _msc;
    FatDisk &_disk;

    static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t size);
    static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t size);
    static bool onStartStop(uint8_t power, bool start, bool eject);

    static MscDevice *_instance;
};