#include <string.h>
#include <Wire.h>

long byteArrayToInt(byte byteArray[]);
volatile long sensorPin;
volatile long relayPin;
volatile bool startWatering;
volatile bool isWatering;
volatile long waterOnCount;

const int MIN_READING = 240;
const int MAX_READING = 600;
const int NUM_CYCLES = 1000;

const double MAX_ADJUSTED_READING = MAX_READING - MIN_READING;
void setup() {
  Wire.begin(4);
  Wire.onReceive(receiveCommand);
  Wire.onRequest(doRequest);
  Serial.begin(9600);
  startWatering = false;
  isWatering = false;
  waterOnCount = 0;
}

void loop() {
  if (startWatering){
    Serial.print("Start watering ");
    Serial.println(relayPin);
    noInterrupts();
    digitalWrite(relayPin, HIGH);
    startWatering = false;
    isWatering = true;
    waterOnCount = 0;
  } else if(isWatering){
     Serial.print("In Is watering ");
     Serial.println(waterOnCount);
    if (waterOnCount >= NUM_CYCLES){
      digitalWrite(relayPin,LOW);
      isWatering = false;
      waterOnCount = 0;
      interrupts();
    } else {
      ++waterOnCount;
    }
  }
}

void receiveCommand(int howMany){
  byte byteArray[4];
  if (howMany != 5){
    Serial.print("Dumping bytes ");
    Serial.println(howMany);
    dumpBytes(howMany);
  } else {
    byte first = Wire.read();
    for (int i = 0; i != 4; ++i){
      byteArray[i] = Wire.read();
    }
    if (first == 0){
      Serial.println("In sensor");
      sensorPin = byteArrayToInt(byteArray);
    } else {
       Serial.println("In relay");
       startWatering = true;
      relayPin = byteArrayToInt(byteArray);
      Serial.println(relayPin);
    }
  }
}

void doRequest(){
  long value = toStandardScale(getSensorReading(sensorPin));
  unsigned char bytesToSend[4];
  copyValueToByteArray(value,bytesToSend);
  Wire.write(bytesToSend,4); 
}

int toStandardScale(int reading){
 return 100 - (((reading - MIN_READING) / MAX_ADJUSTED_READING) * 100);
}


long getSensorReading(long pin){
  return analogRead(pin);
}

long getLongFromWire(){
   byte byteArray[4];
   for (int i = 0; i != 4; ++i){
      byteArray[i] = Wire.read();
   }
   return byteArrayToInt(byteArray);
}

void copyValueToByteArray(long value, byte bytesToSend[]){
  long intValue = value;
  bytesToSend[3] = intValue >> 0 & 0xFF;
  bytesToSend[2] = intValue >> 8 & 0xFF;
  bytesToSend[1] = intValue >> 16 & 0xFF;
  bytesToSend[0] = intValue >> 24 & 0xFF;
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
