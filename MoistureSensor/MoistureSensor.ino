#include <string.h>
#include <Wire.h>

const int moistureSensorPin = 0;
const int MIN_READING = 240;
const int MAX_READING = 600;
const double MAX_ADJUSTED_READING = MAX_READING - MIN_READING;

byte lastLeaderCommand;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4);
  Wire.onReceive(receiveCommand);
  Wire.onRequest(followerRespond);
  Serial.begin(9600);
}

void loop() {
  //Serial.println(toStandardScale(getSensorReading()));
  delay(1);
}

void receiveCommand(int howMany){
  lastLeaderCommand = Wire.read();
  Serial.print("In receiveCommand -- ");
  Serial.println(lastLeaderCommand);
}

void doRequest(){
  long value = toStandardScale(getSensorReading());
  Serial.println(value);
  unsigned char bytesToSend[4];
  copyValueToByteArray(value,bytesToSend);
  Wire.write(value); 
}

void followerRespond() {
  long valueToSend = 0;
  switch(lastLeaderCommand)
  {
    case 0x00: valueToSend = toStandardScale(getSensorReading());
            break;

    case 0x01: valueToSend = getSensorReading();
            break;

    case 0x02: valueToSend = isSensorWorking(getSensorReading()) ? 1 : 0;
            break;
  }
  byte bytesToSend[4];
  copyValueToByteArray(valueToSend,bytesToSend);
  for (int i = 0; i != 4; ++i)
    Serial.println(bytesToSend[i]);
  Serial.println(valueToSend);
  Wire.write(bytesToSend,4);  
}

long getSensorReading(){
  return analogRead(moistureSensorPin);
}

bool isSensorWorking(int reading){
  return reading >= MIN_READING && reading <= MAX_READING;
}

void copyValueToByteArray(long value, byte bytesToSend[]){
  long intValue = value;
  bytesToSend[3] = intValue >> 0 & 0xFF;
  bytesToSend[2] = intValue >> 8 & 0xFF;
  bytesToSend[1] = intValue >> 16 & 0xFF;
  bytesToSend[0] = intValue >> 24 & 0xFF;
}

int toStandardScale(int reading){
 return 100 - (((reading - MIN_READING) / MAX_ADJUSTED_READING) * 100);
}
