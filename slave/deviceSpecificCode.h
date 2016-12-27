const byte myID = B001;

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
