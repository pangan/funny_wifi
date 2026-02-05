#pragma once
#include <Arduino.h>
#include <functional>

#define DEBUG_SERIAL 1

#if DEBUG_SERIAL
  #define DBG(x)    do { if (Serial) Serial.println(x); } while(0)
#else
  #define DBG(x)
#endif



// Call repeatedly from loop()
void wifiLoop();



void startPortal(int lflash, int rflash, int lights, int interior);

typedef std::function<void(String, String)> AddUserCallback;
void setAddUserCallback(AddUserCallback cb);
