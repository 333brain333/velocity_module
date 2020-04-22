struct can_frame canMsg;
MCP2515 mcp2515(10);
MCP2515 mcp2515_1(9);

double speedFbCAN = 0.0;
double speedFbCAN_1 = 0.0;
unsigned long prevSendTime = 0;
const int relay1 = 5; // relay 1 - according to principal scheme  - it connects "neutral -" line to +24V - in case of HIGH mode, or disconnets with any line in case of LOW mode
const int relay2 = 6; // relay 2 - according to principal scheme - it connecnts "signal" and "neutral -" lines to handle or to digital potentiometer and to relay 1 accordingly
const int neutral = 7; // using this pin one could get nutral line state in inversed logic  - "neutral -":+24 V ==> 7 pin is LOW or "neutral -": 0 V ==> 7 pin is HIGH
const int signal_pin = A0; // using this pin one could get analog value of the handle position. +0.5V - moving forward with max velocity; +2.5V - neutral position, no moving; +4.5V - moving backwards with max velocity
const int wiper = A5; // wiper of the digital potentiometer. The usage of this pin helps to check if sent reference speed to digital potentiometer is implemented or not.
float fbSpeed=0.0; // speed obtaining from handle thru signal_pin
int currentSpeed;
unsigned long curr_time=0;
unsigned long prevPotValue=0;
String incoming_mes="";
float incoming_ref_speed = 0.0; //in km/h
String incoming_mode = "m";
String current_mode="manual";
float pot_value=127.0; //in points from 0 to 255, 147== 0 km/h
float p=0.1;
float i=0.0;
float d=0.0;
float pid=0.0;
int16_t dt=100;
int cur_pot_value=127;
GyverPID regulator(p,i,d,dt);
