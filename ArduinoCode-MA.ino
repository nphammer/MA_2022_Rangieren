//Railuino is added
#include <SPI.h>
#include "mcp2515_can.h"
#include "RailuinoSeeed.h" 

const int SPI_CS_PIN = 9;
const int CAN_INT_PIN = 2;

//Input pin for the sensor
const int sensorPin = 6; 

//Output pin for the uncoupling track
int Pin8 = 7; 

#define CAN_2515
mcp2515_can CAN(SPI_CS_PIN); //Set CS pin
TrackController Ctrl(0xdf24, false /* debug */);
#define MAX_DATA_SIZE 8

//Address of the locomotive is set
const word    LOCO  = ADDR_MFX + 7; 

//Address of the turnout is set
const word    TURN  = ADDR_ACC_MM2 + 2; 
const word    TIME  = 2000;
const word    SPEED  = 140;

enum Command {
  GO = 1,
  TURNSTRAIGHT = 2,
  TURNROUND = 3,
  DECOUPLE = 4,
  SETBACK = 5
};

int Decouple() {

  //Locomotive sets off
  Ctrl.setLocoSpeed(LOCO, SPEED); 

  //Wait until the wagon reaches the sensor
  while(digitalRead(sensorPin) == 1){} 

  //Wait until the wagon has passed the sensor
  while(digitalRead(sensorPin) == 0){} 

  //Uncoupling track is switched on for one second
  digitalWrite(Pin8, HIGH);

  delay(1000);

  digitalWrite(Pin8, LOW); 

  //Locomotive stops
  Ctrl.setLocoSpeed(LOCO, 0); 

  return 1;
}

int TurnStraight(){

  //Turnout is set straight
  Ctrl.setTurnout(TURN, true);
   
  return 1;
}

int TurnRound(){

  //Turnout is set to the right
  Ctrl.setTurnout(TURN, false);
    
  return 1;
}

int Go(){

  //Track voltage is switched on
  Ctrl.setPower(true); 

  //For safety reasons, the speed is set to 0
  Ctrl.setLocoSpeed(LOCO, 0); 
    
  Ctrl.setLocoFunction(LOCO, 0, 1);

  //Correct direction of the locomotive is set
  Ctrl.setLocoDirection(LOCO, DIR_FORWARD); 

  return 1;
}

int SetBack(){

  //Turnout is set straight for track 1
  Ctrl.setTurnout(TURN, true);

  //Locomotive sets off
  Ctrl.setLocoSpeed(LOCO, SPEED);

  //Wait for train to hit the bumper
  while (true) {
    word current;
    Ctrl.getSystemStatus(0x0, 0x01, &current);
    if (current > 0x90) break;
    delay(100);
  }

  Ctrl.setLocoSpeed(LOCO, 0);

  //Correct direction of the locomotive is set
  Ctrl.setLocoDirection(LOCO, DIR_REVERSE); 
}


void setup() {

    //Initialise can bus : set baudrate = 250k
    while (CAN_OK != CAN.begin(CAN_250KBPS)) {             
        
        delay(100);
    }

    //Input and output for the respective pins are set
    pinMode(Pin8, OUTPUT);
    pinMode(sensorPin, INPUT); 

    Ctrl.init(CAN);

    //Serial link is started
    Serial.begin(115200); 
    Serial.setTimeout(1);
    while (!Serial) {}

    //Readiness is sent to Python
    Serial.print("Ready"); 
}


void loop() {

  //Arduino waits for command
  while (!Serial.available());
  int cmd = Serial.readString().toInt(); 

  //Arduino executes the respective command
  int response = 0; 
  switch (cmd)
  {
    case Command::GO:
    {
      response = Go();
      break;
    }
    case Command::DECOUPLE:
    {
      response = Decouple();
      break;
    }
    case Command::TURNSTRAIGHT:
    {
      response = TurnStraight();
      break;
    }
    case Command::TURNROUND:
    {
      response = TurnRound();
      break;
    }
    case Command::SETBACK:
    {
      response = SetBack();
      break;
    }
    default:
    {
      response = -cmd;
    }
  }
  Serial.print(response, DEC);

}
