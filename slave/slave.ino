// THIS FILE SHOULD NOT BE MODIFIED ONCE PROVEN CORRECT. ALL DEVICE SPECIFIC PROGRAMMING 
// MUST BE SPECIFIED WITHIN THE DEVICE SPECIFIC FILE, INCORPORATED BY THE INCLUDE 
// COMMAND BELOW.
#include "deviceSpecificCode.h"

////////////////////////////////////////////////
// PROTOCOL:                                  //
// byte #   | description                     //
// ------------------------                   //
//   1.     | START BYTE                      //
//   2.     | CHECKSUM                =       //
//   3.     | SLAVE ID                +       //
//   4.     | MESSAGE LENGTH          +       //
//   5-63   | MESSAGE                 +       //
//                                            //
////////////////////////////////////////////////



///////////////////
// NETWORK SPECS //
///////////////////

unsigned long numBitsPerSerialByte = 8;
unsigned long baudrate = 9600;
unsigned long bitDurationMicros = (long) (1E6 / (float)baudrate) + 1; // Plus one for roundoff error
unsigned long byteDurationMicros = numBitsPerSerialByte * bitDurationMicros;


//////////////////////////////////
// STANDARD COMMANDS & MESSAGES //
//////////////////////////////////

const byte startByte                    = B10101010; // indicates start of communication
                                      
const byte cmd_ping                     = B00000000; // blink LED and respond
const byte cmd_identify                 = B00000001; // respond with description of device
const byte cmd_authorizeCommunication   = B00000010; // enable device to speak on line
const byte cmd_deauthorizeCommunication = B00000011; // disable device to speak on line

const byte msgInvalid                   = B01010101; // indicates checksum does not match
const byte msgUnknownCmd                = B00000001; // indicates command is unknown to device


//////////////////////
// GLOBAL VARIABLES //
//////////////////////

bool isAllowedToSpeak  = true; // Controls whether device is allow to grab hold of com line


/////////////////////////
// SETUP AND MAIN LOOP //
/////////////////////////

void setup(){
  loadEEPROM();
  Serial.begin(baudrate);
  deviceSpecificSetup();
}

void loop(){
  //:::::::::::::::::::::::::
  // A: STARTBYTE DETECTED ::
  //:::::::::::::::::::::::::
    
    // @1. START BYTE
          if (Serial.read() == startByte){         
      //---- WAIT ----//
            yieldUntilNbytes(3);
    // @2. CHECKSUM
            byte checkSum = Serial.read();          
    // @3. TARGET ID
            byte targetID = Serial.read();     
    // @4. MESSAGE LENGTH     
            int msgLength = (int)Serial.read();
            
      //---- WAIT ----//
            yieldUntilNbytes(msgLength);

    // @5+. MESSAGE
          // IF TARGETED DEVICE:
              if (targetID == myID){ 
                      byte msg[msgLength];
                      bool isValidMessage = readMessage(msg, msgLength, checkSum);
                  
                      if (isValidMessage){ // checksum correct -> interpret message
                        interpretMessage(msg, msgLength);
                      }
                      else{ // checksum invalid -> interpret message
                        sendMessage(msgInvalid); 
                      }
              }
            
          // NOT TARGETED DEVICE:
              else{
                purgeBuffer();
              }
        }  

  //::::::::::::::::::::::
  // B: NO STARTBYTE    ::
  //::::::::::::::::::::::
          else {
            doBackground();
          }
}


//////////////////////////
// FUNCTIONS & ROUTINES //
//////////////////////////

bool readMessage(byte *msg, int nBytes, byte checkSum){
  Serial.readBytes((char*)msg, nBytes);
  
  byte msgSum = calcChecksum(msg, nBytes);
  return (msgSum == checkSum);
}

void sendMessage(byte *msg, int nBytes){
  byte msgSum = calcChecksum(msg, nBytes);
  
  Serial.flush(); // Wait for print to serial to complete
  grabComLine();

  Serial.write(startByte);        // @1. STARTBYTE
  Serial.write(msgSum);           // @2. CHECKSUM
  Serial.write(myID);             // @3. SLAVE ID
  Serial.write((byte) nBytes);    // @4. MESSAGE LENGTH
  Serial.write(msg, nBytes);      // @5+ MESSAGE

  Serial.flush(); // Wait till message is sent, before releasing line
  releaseComLine();
}

void sendMessage(byte msgType){
  sendMessage((byte*) msgType, 1); 
}

void yieldUntilNbytes(int msgLength){
  unsigned long startTime = micros();
  unsigned long msgTimeout = byteDurationMicros * msgLength;
  while((micros() - startTime) < msgTimeout){
    doBackground();
    if (Serial.available() >= msgLength){
      break;
    }
  }
}

void interpretMessage(byte *msg, int msgLength){
  byte msgType = msg[0];

  switch (msgType) {
    case cmd_deauthorizeCommunication: // If device is screwing things up, one may force it into listen only mode
      isAllowedToSpeak = false;
      break;
    case cmd_authorizeCommunication: // Re-authorize device to communicate
      isAllowedToSpeak = true;
      break;  
    case cmd_identify: // Make device send its description
      sendMessage((byte *) ID_STRING, ID_STRING_NBytes);
      break;
    case cmd_ping: // Blink LED and confirm reception with message
      ping();
      break;
    default: // Either msg is device specific or it is an unknown command.
      bool isKnownCommand = interpretDeviceSpecificMessage(msg, msgLength);
      if (isKnownCommand){
        break;
      }
      else{
        sendMessage(msgUnknownCmd);
        break;
      }
  }
}

void releaseComLine() {
  digitalWrite(pinEnable, LOW);
}

void grabComLine(){
  if(isAllowedToSpeak){
    digitalWrite(pinEnable, HIGH);
  }
}

byte calcChecksum(byte *byteArray, int msgLength){
  byte msgSum = myID;
  msgSum += (byte) msgLength;

  for (int i = 0; i < msgLength; ++i)
  {
    msgSum += byteArray[i];
  }
  return msgSum;
}

void purgeBuffer(){
  while (Serial.available() > 0){
    Serial.read();
  }
}

void ping(){
  sendMessage(cmd_ping, (byte*)myID, 1);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}