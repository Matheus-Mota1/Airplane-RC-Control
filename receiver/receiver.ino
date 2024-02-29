#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

/* IMPORTANT: The same as in the receiver 0xE9E8F0F0E1LL
   This is the address of the communication */
const uint64_t pipeIn = 0xE9E8F0F0E1LL;
// Set up the value to stop the motor from rotating
const int STOPMOTOR = 12;
// Set up the servos to stay in the middle position 255/2
const int SERVO_INICIAL_POSITION = 127;

// PWM output on pins D2, D3, D4, D5, D6
const int throttle_Pin = 2;
const int aileronr_Pin = 3;
const int aileronl_Pin = 4;
const int elevator_Pin = 5;
const int rudder_Pin = 6;

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

RF24 radio(9, 10); 

// Defining the value to ouput to the channels
int channel_1 = 0;
int channel_2 = 0;
int channel_3 = 0;
int channel_4 = 0;
int channel_5 = 0;

// Servos instance definition
Servo servo_Throttle;
Servo servo_Aileronr;
Servo servo_Aileronl;
Servo servo_Elevator;
Servo servo_Rudder;

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
    /*
    Set up the initial state of the plane, the servos must be 
    in the middle position, and the motor must be turn off.
    This function is called in two situations.
    1: You are preparing to take off
    2: You lost signal
    */  

    // Turn of the motor
    data.throttle = STOPMOTOR; 
    // Set up the middle as a inicital position for the servos
    data.aileron_r = SERVO_INICIAL_POSITION;  
    data.aileron_l = SERVO_INICIAL_POSITION;   
    data.elevator  = SERVO_INICIAL_POSITION;    
    data.rudder    = SERVO_INICIAL_POSITION;     
}

unsigned long lastRecvTime = 0;

// Receive and store the data sent in the "data" structure
void recvData()
{
    while ( radio.available() )
    {
        radio.read(&data, sizeof(Signal));
        lastRecvTime = millis();
    }
}

void setup()
{
  //Set the pins for each PWM signal
  servo_Throttle.attach(throttle_Pin);
  servo_Aileronr.attach(aileronr_Pin);
  servo_Aileronl.attach(aileronl_Pin);
  servo_Elevator.attach(elevator_Pin);
  servo_Rudder.attach(rudder_Pin);

  //Configure the NRF24 module
  initializeState();
  radio.begin();
  radio.openReadingPipe(1,pipeIn);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);

  //start the radio comunication for receiver
  radio.startListening();
  pinMode(6,OUTPUT);
}

void loop()
{
    recvData();
    unsigned long now = millis();
    // 1 second without respose means signal lost
    if ( now - lastRecvTime > 1000 ) {
      initializeState(); 
    }

    
    /*First argument:
    data.X is the value that was sent be the transmitter

    /* Second and third argument
    0, 255: These are the lower and upper bounds of the range of
    the sent value (data.X). The minimum value data.X can have is 0,
    and the maximum is 255.

    /* fouth and fifth
    1000, 2000: These are the lower and upper bounds of the new
    desired range for channel. The mapped value will be adjusted
    to be within this new range. 
    If data.throttle is 0, channel_1 will be mapped to 1000.
    If data.throttle is 255, channel_1 will be mapped to 2000.
    
    The values 1000, 2000 are the servos motors accept PWM signals
    to determine their position. The typical range for these signals is
    from 1000 to 2000 microseconds. Each microsecond within this interval
    represents a variation in the position of the servo.
    */
    
  channel_1 = map(data.throttle, 0, 255, 1000, 2000);
  channel_2 = map(data.pitch,    0, 255, 1000, 2000);
  channel_3 = map(data.roll,     0, 255, 1000, 2000);
  channel_4 = map(data.yaw,      0, 255, 1000, 2000); 
  channel_5 = map(data.rudder,   0, 255, 1000, 2000);

  // Write the PWM signal for each one of the servos
  servo_Throttle.writeMicroseconds(channel_1);
  servo_Aileronr.writeMicroseconds(channel_2);
  servo_Aileronl.writeMicroseconds(channel_3);
  servo_Elevator.writeMicroseconds(channel_4);
  servo_Rudder.writeMicroseconds(channel_5);
}