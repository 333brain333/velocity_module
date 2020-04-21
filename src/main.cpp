#include <Arduino.h>

#include <GyverPID.h>
#include <SPI.h>
#include <stdio.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10);
MCP2515 mcp2515_1(9);`

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



void SET_SPEED(void) {
  // take the SS pin low to select the chip:
  digitalWrite(SS, LOW);
  //  send in the address and value via SPI:
  SPI.transfer(0); // address
  cur_pot_value=(int)pot_value;
  if ((fbSpeed-127)*(cur_pot_value-127)<0){
    cur_pot_value=147.0;
    SPI.transfer(cur_pot_value);
  }
  else {
    if (abs(fbSpeed-127)<abs(cur_pot_value-127.0)){
    cur_pot_value=fbSpeed+20;
    SPI.transfer(cur_pot_value);
    }
    else{
    SPI.transfer(cur_pot_value+20);
    }
  }
  if (cur_pot_value+20>145 && cur_pot_value+20<149){
    digitalWrite(relay2,HIGH);
  }
  else {
    digitalWrite(relay2,LOW);
  }
  // take the SS pin high to de-select the chip:
 digitalWrite(SS, HIGH);
}



void GET_SPEED(void)
{
  fbSpeed=(int)(analogRead(signal_pin)*0.25);
}

void SET_MANUAL(void)
{
  digitalWrite(relay1,LOW);
  current_mode="manual";
}

void SET_ROBOT()
{
  digitalWrite(relay1,HIGH);
  current_mode="robot";
  
}

void SEND()
{
Serial.print((String)regulator.getResultTimer()+";"+fbSpeed+";"+pot_value+";"+current_mode+";"+String(speedFbCAN)+";"+p+";"+i+";"+d+";"+(int)regulator.getResultTimer()+";"+String(speedFbCAN_1)+"\n");
}

void SEND_TO_HUMAN()
{
Serial.print((String)"PID output: "+regulator.getResultTimer()+";"+"Handle position: "+fbSpeed+";"+"Refer. handl. pos: "+pot_value+";"+"Mode: "+current_mode+";"+"Speed from CAN1: "+String(speedFbCAN)+";"+"PID coef.: "+p+";"+i+";"+d+";"+"PID output"+(int)regulator.getResultTimer()+";"+"Speed from CAN2: "+String(speedFbCAN_1)+"\n");
}


void setup() {

  currentSpeed = 0 ;
  SPI.begin();
  delay(200);
  pinMode(SS,OUTPUT); // switch chip select pin to output mode
  pinMode(relay1,OUTPUT); // switch relay 1 pin to output mode
  pinMode(relay2,OUTPUT);
  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
  pinMode(neutral,INPUT);
  digitalWrite(neutral,HIGH);  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();
  mcp2515_1.reset();
  mcp2515_1.setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515_1.setNormalMode();
  regulator.setDirection(REVERSE); // направление регулирования (NORMAL/REVERSE). ПО УМОЛЧАНИЮ СТОИТ NORMAL
  regulator.setLimits(-2, 2);    // пределы (ставим для 8 битного ШИМ). ПО УМОЛЧАНИЮ СТОЯТ 0 И 255
  regulator.setpoint = 0;
  regulator.setMode(0);
   

  Serial.begin(115200);
  Serial.setTimeout(10);
  while (!Serial) {
    ;
  }

  prevSendTime = millis();  
}


void readCAN()
{
   if (interrupt)
   {
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
     { // && (canMsg.can_id == 0x98FF0102) ) {
        speedFbCAN = 0.2 * canMsg.data[5];
    }
     interrupt--;
   }
}

void loop() {

  readCAN();
  
  // Read serial input:
  if (Serial.available()>0){
      incoming_mes=Serial.readString();
      if (incoming_mes == "restart") {
      errCounter=0;
      status=NORMAL;
      }
        // incoming_ref_speed = incoming_mes.substring(0,3).toInt();
        // incoming_mode = incoming_mes.substring(3,4);
        // p=incoming_mes.substring(4,7).toFloat();
        // i=incoming_mes.substring(7,10).toFloat();
        // d=incoming_mes.substring(10,13).toFloat() ;
        regulator.Kp=p;
        regulator.Ki=i;
        regulator.Kd=d;
        if (incoming_mode=="m"){
          SET_MANUAL();
        }
        if (incoming_mode=="r"){
          SET_ROBOT();
        }
    }
  if (incoming_mode=="r"){
      regulator.setpoint = incoming_ref_speed;
      regulator.input = speedFbCAN;
      pid=regulator.getResultTimer()/10.0;
      if (millis()-prevPotValue>dt){
        prevPotValue=millis();
        pot_value=pot_value+pid;  
      }
  }
  else if (incoming_mode=="m"){
    pot_value=incoming_ref_speed;
  }
      
  if (incoming_ref_speed==0){
      pot_value=127.0;
    }
  if (pot_value>230){
    pot_value=230.0;
  }
  else if (pot_value<25){
    pot_value=25.0;
  }
  
  
  
  GET_SPEED();
  if (millis() - prevSendTime > 100){
    SEND_TO_HUMAN();
    prevSendTime = millis();
  }
  SET_SPEED();
}