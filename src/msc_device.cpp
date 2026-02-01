#include "msc_device.h"
#include "disk_config.h"
#include <string.h>

MscDevice *MscDevice::_instance = nullptr;

MscDevice::MscDevice(FatDisk &disk)
    : _disk(disk)
{
    _instance = this;
}

void MscDevice::begin()
{
    _msc.onRead(onRead);
    _msc.onWrite(onWrite);
    _msc.onStartStop(onStartStop);

    _msc.begin(SECTOR_COUNT, SECTOR_SIZE);
    _msc.mediaPresent(true);

    USB.begin();
}

void MscDevice::eject()
{
    _msc.mediaPresent(false);
}

void MscDevice::reconnect()
{
    _msc.mediaPresent(true);
}

int32_t MscDevice::onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t size)
{
    memcpy(buffer, _instance->_disk.sector(lba) + offset, size);
    return size;
}

int32_t MscDevice::onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t size)
{
    memcpy(_instance->_disk.sector(lba) + offset, buffer, size);

    if (_instance->_disk.detectGoFile(lba, buffer, size))
        _instance->goFlag = true;

    return size;
}

bool MscDevice::onStartStop(uint8_t, bool, bool)
{
    return true;
}