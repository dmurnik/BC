#include "../Libs/ModbusRtu.h"
#include "../Headers/Headers.h"

portMUX_TYPE modbusMux = portMUX_INITIALIZER_UNLOCKED;

/*
Element (9):
[0] - slave address;, [2] - baudrate;, [4] - totalize 0 - GENERAL;, [6] - totalize 1 - FORWARD;, [8] - totalize 2 - REVERSE;
*/
uint16_t au16data[length] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int modbusBaudrateListIndex = 3;
const int modbusBaudrateListIndexLimit = 9;
uint32_t modbusBaudrateList[modbusBaudrateListIndexLimit] = { 0, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
bool modbusBaudrateValueFound = false;
bool modbusSettingsNeedRestart = true;

byte modbusSlaveAddressValueCurrent = 0;
unsigned long modbusBaudrateValueCurrent = 0;

// Используем актуальный конструктор с передачей аппаратного Serial1
Modbus slave(modbusSlaveAddressValueCurrent, Serial1, modbusPin);

void modbusInitialize() {
  deviceDataObjectIsBusy = false;

  for (int i = 0; i < modbusBaudrateListIndexLimit; i++ ) {
    if (deviceDataObject.modbusBaudrate == modbusBaudrateList[i]) {
      modbusBaudrateListIndex = i;
      modbusBaudrateValueFound = true;
      break;
    }
  }

  modbusHandlerReloader();
  modbusHandlerResponse();
}

void modbusHandlerReloader() {
  if(!deviceDataObjectIsBusy && modbusSettingsNeedRestart) {
    deviceDataObjectIsBusy = true;

    modbusSlaveAddressValueCurrent = deviceDataObject.modbusSlaveAddress;
    modbusBaudrateValueCurrent = deviceDataObject.modbusBaudrate;

    Serial1.end();
    delay(50);
    uint32_t parityConfig;
    switch (deviceDataObject.modbusParity) {
      case 0: parityConfig = SERIAL_8N1; break;
      case 1: parityConfig = SERIAL_8E1; break;
      case 2: parityConfig = SERIAL_8O1; break;
      default: parityConfig = SERIAL_8E1; break;
    }
    Serial1.begin(modbusBaudrateValueCurrent, parityConfig, MODBUS_RX_PIN, MODBUS_TX_PIN);
    
    uint16_t t35;
    if      (modbusBaudrateValueCurrent <= 1200) t35 = 40;
    else if (modbusBaudrateValueCurrent <= 2400) t35 = 20;
    else if (modbusBaudrateValueCurrent <= 4800) t35 = 10;
    else                                          t35 = 5;
    
    slave = Modbus(modbusSlaveAddressValueCurrent, Serial1, modbusPin);
    slave.setT35(t35);
    slave.start();

    au16data[0] = deviceDataObject.modbusSlaveAddress;
    au16data[1] = (deviceDataObject.modbusBaudrate >> 16) & 0xFFFF;
    au16data[2] = deviceDataObject.modbusBaudrate & 0xFFFF;

    if (UIDisplaySectionListObject == sectionDefault) UIDisplayNeedClear = true;
    deviceDataObjectIsBusy = false;
    modbusSettingsNeedRestart = false;
  }
}

void modbusHandlerListener() {
  slave.poll(au16data, length);
}

void modbusHandlerResponse() {
  au16data[3] = (deviceDataObject.totalizerCommon >> 16) & 0xFFFF;
  au16data[4] = deviceDataObject.totalizerCommon & 0xFFFF;
  au16data[5] = (deviceDataObject.totalizerDirect >> 16) & 0xFFFF;
  au16data[6] = deviceDataObject.totalizerDirect & 0xFFFF;
  au16data[7] = (deviceDataObject.totalizerReverse >> 16) & 0xFFFF;
  au16data[8] = deviceDataObject.totalizerReverse & 0xFFFF;
}

void modbusUpdater() {
  if (au16data[0] != deviceDataObject.modbusSlaveAddress || (((uint32_t)au16data[1] << 16) | au16data[2]) != deviceDataObject.modbusBaudrate) {
    if (au16data[0] >= 1 && au16data[0] <= 247) {
      deviceDataObject.modbusSlaveAddress = au16data[0];
    }

    unsigned long modbusBaudrateNewBuffer = ((uint32_t)au16data[1] << 16) | au16data[2];
    if (modbusBaudrateNewBuffer != deviceDataObject.modbusBaudrate) {
      deviceDataObject.modbusBaudrate = modbusBaudrateNewBuffer;
    }

    modbusSettingsNeedRestart = true;
  }

  modbusHandlerReloader();

  if (deviceDataShadowObject.resetRequestExternal) {
    deviceDataShadowObject.totalizerDirect = ((uint32_t)au16data[5] << 16) | au16data[6];
    deviceDataShadowObject.totalizerReverse = ((uint32_t)au16data[7] << 16) | au16data[8];
  }

  unsigned long totalizerDirectCurrentBuffer = deviceDataObject.totalizerDirect;
  unsigned long totalizerReverseCurrentBuffer = deviceDataObject.totalizerReverse;

  if (deviceDataShadowObject.totalizerDirect != totalizerDirectCurrentBuffer ||
      deviceDataShadowObject.totalizerReverse != totalizerReverseCurrentBuffer) {

    if (!deviceDataObjectIsBusy) {
      deviceDataObjectIsBusy = true;

      portENTER_CRITICAL(&modbusMux);
      if (deviceDataShadowObject.totalizerDirect == 0) deviceDataObject.totalizerDirect = deviceDataShadowObject.totalizerDirect;
      if (deviceDataShadowObject.totalizerReverse == 0) deviceDataObject.totalizerReverse = deviceDataShadowObject.totalizerReverse;
      portEXIT_CRITICAL(&modbusMux);

      if (UIDisplaySectionListObject == sectionDefault) UIDisplayNeedClear = true;
      deviceDataShadowObject.resetRequestExternal = true;
    }
  }

  modbusHandlerResponse();
}
