#include <string.h>
#include <Wire.h>

long byteArrayToInt(byte byteArray[]);

const int MIN_READING = 240;
const int MAX_READING = 600;
const double MAX_ADJUSTED_READING = MAX_READING - MIN_READING;

volatile long sensorPin;

void setup() {
  // put your setup code here, to run once:
  Wire.begin(4);
  Wire.onReceive(receiveCommand);
  Wire.onRequest(doRequest);
  Serial.begin(9600);
}

void loop() {
  delay(1);
}

void receiveCommand(int howMany){
  byte byteArray[4];
  if (howMany != 4){
    dumpBytes(howMany);
  } else {
    for (int i = 0; i != 4; ++i){
      byteArray[i] = Wire.read();
    }
  }
  sensorPin = byteArrayToInt(byteArray);
}

void doRequest(){
  long value = toStandardScale(getSensorReading(sensorPin));
  unsigned char bytesToSend[4];
  copyValueToByteArray(value,bytesToSend);
  Wire.write(bytesToSend,4); 
}

long getSensorReading(long pin){
  return analogRead(pin);
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

long byteArrayToInt(byte byteArray[]){
  int toInt = (uint32_t) byteArray[0] << 24;
  toInt |=  (uint32_t) byteArray[1] << 16;
  toInt |= (uint32_t) byteArray[2] << 8;
  toInt |= (uint32_t) byteArray[3];
  return toInt;
}

void dumpBytes(int howMany){
  for (int i = 0; i != howMany; ++i){
       Wire.read();
    }
}
