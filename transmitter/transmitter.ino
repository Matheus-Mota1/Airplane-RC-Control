#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//IMPORTANT: The same as in the receiver 0xE9E8F0F0E1LL
const uint64_t pipeOut = 0xE9E8F0F0E1LL;
// Set up the value to stop the motor from rotating
const int STOPMOTOR = 12;
// Set up the servos to stay in the middle position 255/2
const int SERVO_INICIAL_POSITION = 127;

const int leftVRXPin  = A0;
const int leftVRYPin  = A1;
const int rightVRXPin = A2;
const int rightVRYPin = A3;
const int throttlePin = A4;

/*
 Module // Arduino UNO,NANO
    
    GND    ->   GND
    VCC    ->   3.3V
    CE     ->   D7
    CSN    ->   D8
    SCK    ->   D13
    MOSI   ->   D11
    MISO   ->   D12
*/

RF24 radio(7, 8); 

// The sizeof this struct should not exceed 32 bytes
struct Signal {
  byte throttle; 
  byte aileron_r; //left
  byte aileron_l; //right
  byte elevator;
  byte rudder;
};
Signal data;


void initializeState() 
{
    /* Set up the initial state of the plane, the servos must be 
    in the middle position, and the motor must be turn off.
    This function is called in two situations.
    1: You are preparing to take off
    2: You lost signal */  
    
    // Turn of the motor
    data.throttle = STOPMOTOR; 

    // Set up the middle as a inicital position for the servos
    data.aileron_r = SERVO_INICIAL_POSITION;  
    data.aileron_l = SERVO_INICIAL_POSITION;   
    data.elevator  = SERVO_INICIAL_POSITION;    
    data.rudder    = SERVO_INICIAL_POSITION;    
}


int mapJoystickValues(int pin, bool reverse)
{
  // Read the value from the pin specified
  readVal = analogRead(pin)
  minimumValue = 12;
  middleValue = 514;
  maximumValue = 1024;
  /* Redefine the value of the variable to always be in the range
  Exaple: readval = 200, lower = 500, upper = 1023
  the range of the value can only be between 500-1023
  If it's lower than 500. readVal is set to be 500.
  If it's higher than 1023. readVal is set to be 1023
  If it's between the range. readVal keep it's value */
  readVal = constrain(readVal, minimumValue, maximumValue);
  /* Divide the range of value read in two:
  From the middle to the back | From the middle to the front  
  This make each movement more precise because the range of mapping
  is lower. */
  if ( readVal < middleValue )
    readVal = map(readVal, minimumValue, middle, 0, 128);
  else
    readVal = map(readVal, middleValue, maximumValue, 128, 255);
  
  if (reverse) { 
    return 255 - readVal;
  }
  else {
    return readVal;
  }
}


void setup()
{
  //Set up radio transmitter
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  //start the radio comunication for Transmitter
  radio.stopListening(); 
  initializeState(); 
}

void loop()
{
  // Setting may be required for the correct values of the control levers.
  data.throttle  = mapJoystickValues(throttlePin, false );
  data.aileron_r = mapJoystickValues(rightVRXPin, false );   
  data.aileron_l = mapJoystickValues(rightVRYPin, false );  
  data.elevator  = mapJoystickValues(leftVRXPin,  false );   
  data.rudder    = mapJoystickValues(rightVRYPin, false );     

  // Send the data to the receiver
  radio.write(&data, sizeof(Signal));
}