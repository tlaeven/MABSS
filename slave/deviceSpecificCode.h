#include "Arduino.h"
const byte myID = B001;

void doBackground(){
}
void identify(){
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
