
#ifndef IR_h
#define IR_h

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

#ifndef Arduino_h
#include "Arduino.h"
#endif
#ifndef Team_h
#include <Team.h>
#endif


#define IR_PIN 2

void setupIR();
void loopIR();

void hit(int team_id, int damage);

#endif
