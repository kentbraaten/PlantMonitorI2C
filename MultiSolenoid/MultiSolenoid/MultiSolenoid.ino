#include <Wire.h>

void resetPinArray();
long byteArrayToInt(byte byteArray[]);
void receiveRequest(int howMany);
void processRequest();
bool validNumberOfBytesSent(int howMany);
void dumpBytes(int howMany);

const int LOOPING = 0;
const int TURN_ON = 1;
const int WAITING = 2;
const int TURN_OFF = 3;
const int FIRST_PIN = 4;
const int MAX_PIN_IDX = 4;
const int NUM_CYCLES = 10000;
volatile long cyclesPerPin[4];
volatile bool startNewCycle = false;

class MultiSolenoidState{
  private:
    int state = LOOPING;
    int pinIdx = 0;
    int cycles = 0;

  public: 
    void start(volatile long cycles[]){
        pinIdx = 0;
        cycles = 0;
        state = TURN_ON;
    }

    MultiSolenoidState(){
      for (int i = 0; i != MAX_PIN_IDX; ++i){
        cyclesPerPin[i] = 0;
      }
    }
  
    int getNextState(){
      switch(state){
        case TURN_ON: state = processTurnOnState();
                      break;

        case WAITING: state = processWaitingState();
                      break;

        case TURN_OFF: state = processTurnOffState();
                        break;

        case LOOPING: state = processLoopingState();
                      break;
      }
    }

  int processTurnOnState(){
    digitalWrite(cyclesPerPin[pinIdx], HIGH);
    return WAITING;
  }

  int processWaitingState() {
    cycles++;

    if (cycles >= NUM_CYCLES){
      digitalWrite(cyclesPerPin[pinIdx],LOW);
      pinIdx++;
      cycles = 0;
      if (pinIdx == MAX_PIN_IDX || cyclesPerPin[pinIdx] == 0){
        return TURN_OFF;
      } 
      return TURN_ON;
    }
    return WAITING;
  }

  int processTurnOffState() {
    interrupts();
    return LOOPING;
  }

  int processLoopingState() {
    return LOOPING;
  }
};

MultiSolenoidState* currentState;

void setup() {
   currentState = new MultiSolenoidState();
   Wire.begin(6);
   Wire.onReceive(receiveRequest);
   Wire.onRequest(processRequest);
   Serial.begin(9600);
}

void loop() {
  if (startNewCycle){
    startNewCycle = false;
    noInterrupts();
    currentState->start(cyclesPerPin);
  }
  currentState->getNextState();
  delay(1);
}

long byteArrayToInt(byte byteArray[]){
  int toInt = (uint32_t) byteArray[0] << 24;
  toInt |=  (uint32_t) byteArray[1] << 16;
  toInt |= (uint32_t) byteArray[2] << 8;
  toInt |= (uint32_t) byteArray[3];
  return toInt;
}

void receiveRequest(int howMany){
  if (validNumberOfBytesSent(howMany)){
    byte byteArray[4];
    resetPinArray();
    for (int i = 0; i != howMany; ++i){
      byteArray[i % 4] = Wire.read();
      if (i % 4 == 3){
        cyclesPerPin[i / 4] = byteArrayToInt(byteArray);
      }
    }
    startNewCycle = true;
  } else {
    dumpBytes(howMany);
  }
}

void processRequest() {
    Wire.write(0x01);
}

bool validNumberOfBytesSent(int howMany){
  return howMany % 4 == 0 && howMany / 4 < 4;
}

void dumpBytes(int howMany){
  for (int i = 0; i != howMany; ++i){
       Wire.read();
    }
}

void resetPinArray(){
  for (int i = 0; i != 4; ++i){
    cyclesPerPin[i] = 0;
  }
}
