#ifndef COUNTER_H
#define COUNTER_H

#include <Arduino.h>

#define SENSOR_FIRST_PINOUT  4
#define SENSOR_SECOND_PINOUT 5

#define AUTO_STATE_IDLE      0
#define AUTO_STATE_S1_FIRST  1
#define AUTO_STATE_S2_FIRST  2
#define AUTO_STATE_COUNTED   3

#define MANUAL_STATE_IDLE    0
#define MANUAL_STATE_WAIT_S1 1
#define MANUAL_STATE_WAIT_S2 2
#define MANUAL_STATE_WRONG   3

#define COUNTER_MODE_AUTO 0
#define COUNTER_MODE_MANUAL 1

#define COUNTER_DIR_FORWARD 0
#define COUNTER_DIR_BACKWARD 1

volatile extern uint8_t counterMode;
volatile extern uint8_t counterManualDirection;

void sensorsInitialize();
void sensorsAttachAuto();
void sensorsDetachAll();
void counterModeApply();

void counterSensorFirst();
void counterSensorSecondary();
void counterSensorFirstManual();
void counterSensorSecondaryManual();
void counterSensorHandleForward();
void counterSensorHandleBackward();
void counterSensorHandleManual();
#endif