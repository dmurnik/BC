#ifndef MODBUS_H
#define MODBUS_H

#define length 9
#define modbusPin 18     // Пин управления DE/RE для MAX485
#define MODBUS_RX_PIN 16 // Подключить к RO микросхемы MAX485
#define MODBUS_TX_PIN 17 // Подключить к DI микросхемы MAX485

extern uint16_t au16data[length];
extern int modbusBaudrateListIndex;
extern const int modbusBaudrateListIndexLimit;
extern uint32_t modbusBaudrateList[];
extern bool modbusBaudrateValueFound;
extern bool modbusSettingsNeedRestart;

extern byte modbusSlaveAddressValueCurrent;
extern uint32_t modbusBaudrateValueCurrent;

void modbusInitialize();

void modbusHandlerReloader();

void modbusHandlerListener();

void modbusHandlerResponse();

void modbusUpdaterRequest();

void modbusUpdater();

#endif