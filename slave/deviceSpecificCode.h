#include "Arduino.h"

const byte myID = B001;
const int pinEnable = 2;
const int ID_STRING_NBytes = 60;
const char ID_STRING[ID_STRING_NBytes] = "First test device";

void doBackground(){
}

bool interpretDeviceSpecificMessage(byte *msg, int msgLength){
  byte msgType = msg[0];
  bool isValidMessage = true;
  switch (msgType) {
    default:
    	isValidMessage = false;
      break;
  }
  return isValidMessage;
}

void deviceSpecificSetup(){
  pinMode(pinEnable, OUTPUT);
  pinMode(A0,  INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(pinEnable, LOW);
  
  Serial.flush();
}
