////////////////////////functuions initialisations/////////////////////
MCP2515 can1(10); //for In Out messages - to push control impact to module and recieve resulting feedback form it
MCP2515 can2(9);//for velocity messages
GyverPID regulator(p,i,d,dt);

/////////////////////////pins/////////////////////////////
const int relay1 = 5; // relay 1 - according to principal scheme  - it connects "neutral -" line to +24V - in case of HIGH mode, or disconnets with any line in case of LOW mode
const int relay2 = 6; // relay 2 - according to principal scheme - it connecnts "signal" and "neutral -" lines to handle or to digital potentiometer and to relay 1 accordingly
const int neutral = 7; // using this pin one could get nutral line state in inversed logic  - "neutral -":+24 V ==> 7 pin is LOW or "neutral -": 0 V ==> 7 pin is HIGH
const int signal_pin = A0; // using this pin one could get analog value of the handle position. +0.5V - moving forward with max velocity; +2.5V - neutral position, no moving; +4.5V - moving backwards with max velocity
const int wiper = A5; // wiper of the digital potentiometer. The usage of this pin helps to check if sent reference speed to digital potentiometer is implemented or not.
const int interruptPinCan1 = 2;
const int interruptPinCan2 = 3;
//////////////////timers////////////////////////
unsigned long curr_time=0;
unsigned long prevSendTime = 0;
unsigned long prevPotValue=0;
//////////////////variables////////////////////////
double speedFbCAN = 0.0; //speed obtained from canVelocity message
float handle=0.0; //  speed handle position
//int currentSpeed = 0; delete in future release
//String incoming_mes=""; delete in future release
float incoming_ref_speed = 0.0; //in km/h
//String incoming_mode = "m"; delete in future release
//String current_mode="manual"; delete in future release
float pot_value=127.0; //in points from 0 to 255, 147== 0 km/h
float p=0.1;
float i=0.0;
float d=0.0;
float pid=0.0;
int16_t dt=100;
int cur_pot_value=127;
volatile byte interruptCan1 = 0;
volatile byte interruptCan2 = 0;

///////////////////////structs&enums/////////////////////////
struct can_frame canVelocity;
struct can_frame canInMes;
struct can_frame canOutMes;
typedef enum {
    normalMode,
    incomingMesCan1,
    incomingMesCan2,
    digiPot,
}errCode_tag;
 errCode_tag errCode = normalMode;
typedef enum {
    SET_STATE_MANUAL,
    SET_STATE_ROBOT,
    SET_STATE_CLEAR,
    SET_STATE_STOP,
} setState_tag;
setState_tag setState = SET_STATE_MANUAL;
typedef enum{
    STATE_ROBOT,
    STATE_MANUAL,
    STATE_STOP,
    STATE_ERROR
} state_tag;
state_tag state = STATE_MANUAL;
