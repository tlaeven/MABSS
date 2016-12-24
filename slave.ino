// THIS FILE SHOULD NOT BE MODIFIED ONCE PROVEN CORRECT. ALL DEVICE SPECIFIC PROGRAMMING 
// MUST BE SPECIFIED WITHIN THE DEVICE SPECIFIC FILE, INCORPORATED BY THE INCLUDE 
// COMMAND BELOW.
#include deviceSpecificCode.h

/////////////////////////////////////////////////////////////////////////////////////////
// PROTOCOL:
// BYTE #   | DESCRIPTION
// ------------------------
//   1.     | START BYTE
//   2.     | MESSAGE LENGTH
//   3.     | ADDRESSED ID
//   4.     | CHECKSUM
//   5-63   | MESSAGE   


// NETWORK SPECS
unsigned long numBitsPerSerialByte = 10;
unsigned long baudrate = 9600;
unsigned long bitDurationMicros = 1E6 / baudrate;
unsigned long byteDurationMicros = numBitsPerSerialByte * bitDuration;

boolean isAllowedToSpeak  = true;

// COMMAND ID'S
const byte cmd_ping                     = B00000001; // blink LED and respond
const byte cmd_read                     = B00000010; // respond with measurement
const byte cmd_identify                 = B00000011; // respond with description of device
const byte cmd_reset                    = B00000100; // reset device
const byte cmd_flash_eeprom             = B00000101; // flash eeprom and reset device
const byte cmd_authorizeCommunication   = B00000110; // enable device to speak on line
const byte cmd_deauthorizeCommunication = B00000111; // disable device to speak on line

const byte msgInvalid       = B01010101;
const byte startByte        = B10101010;

const int pinEnable = 2;

void setup(){
  loadEEPROM();

  Serial.begin(baudrate, SERIAL_8E1);
  pinMode(pinEnable,   OUTPUT);
  pinMode(A0,  INPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  
  digitalWrite(pinEnable, LOW);
  Serial.flush();
}

void loop(){
  ///////////////////////////////////
  //CHECK IF NEW MESSAGE IS AVAILABLE
  if (Serial.available() > 2) {
    //CHECK IF IT IS A VALID MESSAGE
    if (Serial.read() == startByte){
      int msgLength = (int)Serial.read();
      //LISTEN TO MESSAGE
      if (Serial.read() == myID){
        waitForMessage(msgLength);
        byte msg[msgLength];
        bool isValidMessage = readMessage(msg, msgLength);
        //INTERPRET IF CHECKSUM IS CORRECT
        if (isValidMessage){
          interpretMessage(msg, msgLength);
        }
        //REPORT INCORRECT MESSAGE IN CASE OF ERROR
        else{
          sendMessage(msgInvalid, (byte*)sizeof(msgInvalid), 1); //CHECK WHEN NOT SLEEPY    }
      }
      //IGNORE MESSAGE AND PERFORM OTHER TASKS
      else{
        waitForMessage(msgLength);
        purgeBuffer();
      }
    }

    //NOT A VALID MESSAGE: EMPTY BUFFER
    else{
      purgeBuffer();
    }
  }

  /////////////////////////////////////////
  // IF BUS IS EMPTY, PERFORM REGULAR TASKS
  else {
    doBackground();
  }
}

void interpretMessage(byte *msg, int msgLength){
  byte msgType = msg[0];

  switch (msgType) {
    case cmd_reset: // re-run setup code
      setup();
      break;
    case cmd_deauthorizeCommunication: // If device is screwing things up, one may force it into listen only mode
      isAuthorizedToSpeak = false;
      break;
    case cmd_authorizeCommunication: // Re-authorize device to communicate
      isAuthorizedToSpeak = true;
      break;  
    case cmd_identify: // Make device send its description
      identify();
      break;
    case cmd_ping: // Blink LED and confirm reception with message
      // ping();
      break;
    case cmd_flash_eeprom: // Flash eeprom to update device
      // flash_eeprom(byte *msg, int msgLength);
      break;
    default: // Either msg is device specific or it is an unknown command.
      bool isKnownCommand = interpretDeviceSpecificMessage(msg, msgLength);
      if (isKnownCommand){
        break;
      }
      else{
        sendMessage(msgUnknownCmd, byte noMsg, 0);
        break;
      }
  }
}

byte calcChecksum(byte *byteArray, int arrayLength){
  byte msgSum = 0;
  for (int i = 0; i < msgLength; ++i)
  {
    msgSum += msg[i];
  }
  return msgSum;
}

void sendMessage(byte msgType, byte *msg, int nBytes){
  byte msgSum = calcChecksum(msg, nBytes);
  
  Serial.flush(); // Wait for print to serial to complete
  grabComLine();

  Serial.write(startByte);
  Serial.write(myID);
  Serial.write(msg, nBytes);
  Serial.flush(); // Wait till message is sent, before releasing line
 
  releaseComLine();
}

void grabComLine(){
  if(isAllowedToSpeak){
    digitalWrite(pinEnable, HIGH);
  }
}

void releaseComLine(){
  digitalWrite(pinEnable, LOW);
}

bool readMessage(byte *msg, int msgLength){
  byte checkSum = Serial.read();
  Serial.readBytes((char*)msg, msgLength);
  
  byte msgSum = calcChecksum(msg, msgLength);
  return (msgSum == checkSum);
}

void waitForMessage(int msgLength){
  unsigned long startTime = micros();
  unsigned long msgTimeout = byteDurationMicros * msgLength;
  while(micros() - startTime < msgTimeout){
    doBackground();
    if (Serial.available() >= msgLength){
      break;
    }
  }
}

void purgeBuffer(){
  while (Serial.available() > 0){
    Serial.read();
  }
}

void loadEEPROM(){}
