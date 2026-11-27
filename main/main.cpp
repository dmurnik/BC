#include <Arduino.h>
#include <Wire.h>
#include "Headers/Headers.h"

#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

Adafruit_MCP23008 mcp;

void setup() {
  Serial.begin(115200); // Для отладки через USB

  // Обязательно запускаем I2C до инициализации MCP23008 и FRAM
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000L);

  MCPDisplayInitialize(0x26, 20, 4);

  IODataSDInitialize();

  sensorsInitialize();

  modbusInitialize();

  UIDisplayInitialize();
}

void loop() {  
  IODataWatchdogHandler();

  modbusUpdater();

  modbusHandlerListener();
  
  IODataSDFileWritePeriodically();

  UIButtonsHandler();

  UIDisplayHandler();
}
