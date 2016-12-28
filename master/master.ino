// NETWORK SPECS
#define baudrate 9600

// DEVICE ID'S
const byte ID_current = B001; 

// COMMAND ID'S
const byte cmd_ping         = B00000001; // blink LED and respond
const byte cmd_read         = B00000010; // respond with measurement
const byte cmd_identify     = B00000011; // respond with description of device
const byte cmd_flash_eeprom = B10101010; // flash eeprom and reset device



const int pinEnable = 2;
bool alt = false;
unsigned long timeOut = 100;
unsigned long startTime = 0;

void setup(){
  Serial.begin(baudrate); // put your main code here, to run repeatedly:
  pinMode(pinEnable,   OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop(){
  sendMessage(ID_current, cmd_ping);
  listen(timeOut);
}

void sendMessage(byte ID, byte msg){
  byte message[1];
  message[0] = B00000001;
  

  Serial.flush(); // Wait for print to serial to complete
  digitalWrite(pinEnable, HIGH); // Take control of line
  Serial.write(B10101010);
  Serial.write((byte)2);
  Serial.write(ID);
  Serial.write(calcChecksum(message, 1) + ID);
  Serial.write(message, 1);
  Serial.flush();
  digitalWrite(pinEnable, LOW); // Let go of line
}

void listen(int timeOut){
 byte incomingBytes[4]; 
 startTime = millis();

 while (millis() - startTime <= timeOut){
  if (Serial.available() > 2) {
    Serial.readBytes((char *)incomingBytes, 4); // APPENDS A 0 EACH TIME THE BUFFER IS READ

    for(int i=1; i<4; i++){   
     Serial.print(incomingBytes[i]);
     Serial.print("\t");
    }
    Serial.print("\n");
}
 }
}

byte calcChecksum(byte *byteArray, int msgLength){
  byte msgSum = 0;
  for (int i = 0; i < msgLength; ++i)
  {
    msgSum += byteArray[i];
  }
  return msgSum;
}

