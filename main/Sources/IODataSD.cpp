#include "../Headers/Headers.h"

deviceData deviceDataObject;
deviceDataChar deviceDataCharObject;
deviceDataShadow deviceDataShadowObject;

volatile bool deviceDataObjectIsBusy = true;

volatile uint32_t autSvdTim = 0;  // Timer for autosave settings.
unsigned long intervalSD = 1000;  // Timer for write data FRAM.

const unsigned long watchdogTimeout = 5000;
volatile unsigned long watchdogTimeSignalRefresh = 0;

void IODataSDFileWrite(const unsigned long &value, const unsigned int offset) {
    Wire.beginTransmission(0x50);
    Wire.write((uint8_t)(offset >> 8));
    Wire.write((uint8_t)(offset & 0xFF));
    // For big-endian FRAM:
    Wire.write((uint8_t)(value >> 24));
    Wire.write((uint8_t)(value >> 16));
    Wire.write((uint8_t)(value >> 8));
    Wire.write((uint8_t)(value & 0xFF));
    Wire.endTransmission();
    delay(5);
}

void IODataSDFileRead(unsigned long &value, const unsigned int offset) {
  Wire.beginTransmission(0x50);
  Wire.write((uint8_t)(offset >> 8));
  Wire.write((uint8_t)(offset & 0xFF));
  Wire.endTransmission();
  delay(5);
  Wire.requestFrom((uint8_t)0x50, (uint8_t)4);
  if (Wire.available() >= 4) {
    unsigned long readValue = 0;
    // For big-endian FRAM:
    readValue = ((uint32_t)Wire.read()) << 24;
    readValue |= ((uint32_t)Wire.read()) << 16;
    readValue |= ((uint32_t)Wire.read()) << 8;
    readValue |= ((uint32_t)Wire.read());
    value = readValue;
  }
  else {
    value = 0;
  }
}

void IODataSensorStateWrite(volatile uint8_t &value, const unsigned int offset) {
  Wire.beginTransmission(0x50);
  Wire.write((uint8_t)(offset >> 8));
  Wire.write((uint8_t)(offset & 0xFF));
  Wire.write((uint8_t)(value));
  Wire.endTransmission();
  delay(5);
}

void IODataSensorStateRead(volatile uint8_t &value, const unsigned int offset) {
  Wire.beginTransmission(0x50);
  Wire.write((uint8_t)(offset >> 8));
  Wire.write((uint8_t)(offset & 0xFF));
  Wire.endTransmission();
  delay(5);

  uint8_t readValue = 0;
  Wire.requestFrom((uint8_t)0x50, (uint8_t)1);
  if (Wire.available() >= 1) {
    readValue = ((uint8_t)Wire.read());
    value = readValue;
  }
  else {
    value = 0;
  }
}

char* totalizerCommonReturn() {
  snprintf(deviceDataCharObject.totalizerCommon, sizeof(deviceDataCharObject.totalizerCommon), "%lu", deviceDataObject.totalizerCommon);
  return deviceDataCharObject.totalizerCommon;
}

char* totalizerDirectReturn() {
  snprintf(deviceDataCharObject.totalizerDirect, sizeof(deviceDataCharObject.totalizerDirect), "%lu", deviceDataObject.totalizerDirect);
  return deviceDataCharObject.totalizerDirect;
}

char* totalizerReverseReturn() {
  snprintf(deviceDataCharObject.totalizerReverse, sizeof(deviceDataCharObject.totalizerReverse), "%lu", deviceDataObject.totalizerReverse);
  return deviceDataCharObject.totalizerReverse;
}

void IRAM_ATTR totalizerCommonValueAdd() {
  if (!deviceDataObjectIsBusy) deviceDataObjectIsBusy = true;
  deviceDataObject.totalizerCommon++;
  watchdogTimeSignalRefresh = millis();
}

void totalizerCommonValueReset() {
  // if (!deviceDataObjectIsBusy) {
  //   deviceDataObject.totalizerCommon = 0;
  // }
}

void IRAM_ATTR totalizerDirectValueAdd() {
  if (!deviceDataObjectIsBusy) deviceDataObjectIsBusy = true;
  deviceDataObject.totalizerDirect++;
  watchdogTimeSignalRefresh = millis();
}

void totalizerDirectValueReset() {
  if (!deviceDataObjectIsBusy) {
    deviceDataShadowObject.resetRequestExternal = false;
    deviceDataShadowObject.totalizerDirect = 0;
  }
}

void IRAM_ATTR totalizerReverseValueAdd() {
  if (!deviceDataObjectIsBusy) deviceDataObjectIsBusy = true;
  deviceDataObject.totalizerReverse++;
  watchdogTimeSignalRefresh = millis();
}

void totalizerReverseValueReset() {
  if (!deviceDataObjectIsBusy) {
    deviceDataShadowObject.resetRequestExternal = false;
    deviceDataShadowObject.totalizerReverse = 0;
  }
}

void deviceConfigurationModbusBaudrateSet(unsigned long value) {
  deviceDataObject.modbusBaudrate = value;
}

void deviceConfigurationModbusSlaveAddressSet(byte value) {
  deviceDataObject.modbusSlaveAddress = value;
}

void modbusSlaveAddressAdd() {
    deviceDataObject.modbusSlaveAddress = (deviceDataObject.modbusSlaveAddress >= 247) ? 0 : deviceDataObject.modbusSlaveAddress + 1;
}

void modbusSlaveAddressReduce() {
    deviceDataObject.modbusSlaveAddress = (deviceDataObject.modbusSlaveAddress == 0) ? 247 : deviceDataObject.modbusSlaveAddress - 1;
}

char* deviceConfigurationModbusBaudrateGet() {
  snprintf(deviceDataCharObject.modbusBaudrate, sizeof(deviceDataCharObject.modbusBaudrate), "%lu", deviceDataObject.modbusBaudrate);
  return deviceDataCharObject.modbusBaudrate;
}

char* deviceConfigurationModbusSlaveAddressGet() {
  snprintf(deviceDataCharObject.modbusSlaveAddress, sizeof(deviceDataCharObject.modbusSlaveAddress), "%lu", deviceDataObject.modbusSlaveAddress);
  return deviceDataCharObject.modbusSlaveAddress;
}

void IODataSDInitialize() {
  // Wire.begin() теперь вызывается в main.cpp до инициализации дисплея и FRAM

  IODataSDFileRead(deviceDataObject.totalizerCommon, 0);
  IODataSDFileRead(deviceDataObject.totalizerDirect, 4);
  IODataSDFileRead(deviceDataObject.totalizerReverse, 8);
  IODataSDFileRead(deviceDataObject.modbusBaudrate, 12);
  IODataSDFileRead(deviceDataObject.modbusSlaveAddress, 16);
  IODataSensorStateRead(counterMode, 20);
  IODataSensorStateRead(counterManualDirection, 21);
  IODataSensorStateRead(deviceDataObject.modbusParity, 22);
  
  if (deviceDataObject.modbusParity > 2) deviceDataObject.modbusParity = 1; // по умолчанию Even
  if (deviceDataObject.modbusBaudrate == 0 || deviceDataObject.modbusBaudrate > 115200) deviceDataObject.modbusBaudrate = 9600; // защита при чистой FRAM
  if (deviceDataObject.modbusSlaveAddress == 0 || deviceDataObject.modbusSlaveAddress > 247) deviceDataObject.modbusSlaveAddress = 1;

  deviceDataShadowObject.resetRequestExternal = true;
}

void IODataSDFileWritePeriodically() {
  unsigned long currentMillisSaved = millis();
  if (currentMillisSaved - autSvdTim >= intervalSD) {
    IODataSDFileWrite(deviceDataObject.totalizerCommon, 0);
    IODataSDFileWrite(deviceDataObject.totalizerDirect, 4);
    IODataSDFileWrite(deviceDataObject.totalizerReverse, 8);
    IODataSDFileWrite(deviceDataObject.modbusBaudrate, 12);
    IODataSDFileWrite(deviceDataObject.modbusSlaveAddress, 16);
    IODataSensorStateWrite(counterMode, 20);
    IODataSensorStateWrite(counterManualDirection, 21);
    IODataSensorStateWrite(deviceDataObject.modbusParity, 22);

    autSvdTim = currentMillisSaved;
  }
}

void IODataWatchdogHandler() {
  if (millis() - watchdogTimeSignalRefresh >= watchdogTimeout) {
    deviceDataObjectIsBusy = false;
  }
}

const char* deviceConfigurationModbusParityGet() {
  switch (deviceDataObject.modbusParity) {
    case 0: return "None";
    case 1: return "Even";
    case 2: return "Odd";
    default: return "?";
  }
}
