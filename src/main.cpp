#include <Arduino.h>

#include "fat_disk.h"
#include "msc_device.h"
#include "gemini_client.h"

FatDisk disk;
MscDevice msc(disk);

GeminiClient gemini("key");

void setup()
{
  disk.initFat12();

  msc.begin();

  WiFi.begin("ssid", "password");
  while (WiFi.status() != WL_CONNECTED)
    delay(500);
}

void loop()
{
  if (!msc.goFlag)
    return;

  delay(50);
  msc.eject();

  msc.goFlag = false;
  disk.deleteGoFile();

  static char buffer[DATA_BUF_SIZE + 1];
  memset(buffer, 0, sizeof(buffer));
  size_t len = disk.readData(buffer, DATA_BUF_SIZE);

  GeminiResult res = gemini.generate(String(buffer, len));

  if (res.success)
  {
    disk.writeData(res.text, strlen(res.text));
  }
  else
  {
    String answer = "ERROR: HTTPCode=" + String(res.httpCode) +
                    ", Message=\"" + res.errorMessage + "\"" +
                    (res.errorStatus.length() > 0 ? ", Status=" + res.errorStatus : "");

    disk.writeData(answer.c_str(), answer.length());
  }

  msc.reconnect();
}
