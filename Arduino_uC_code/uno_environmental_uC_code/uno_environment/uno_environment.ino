/*
* AUTHOR: Loredan E. Bucur & Maria Mitoi 
 *loredan.bucur@radio.pub.ro / maria.mitoi@radio.pub.ro
 * UPB
 * eWall FP7 project
 * Environmental sensors for Arduino Uno
 * LAST_EDIT: Dec. 2015
 *
 */

//DEFINE
#define DEBUG 1
#define DHTTYPE DHT22 //only one type of DHT is possible on the same arduino?

//INCLUDE
#include <XBee.h> //update MAX_FRAME_DATA_SIZE 50
#include <SoftwareSerial.h> //comm with xbee
#include <DHT.h> //supports both DHT11 and DHT22
#include <string.h> //for memcpy
#include <stdbool.h> //standard boolean
#include <Debug.h> //must be placed after #def DEBUG
#include <Wire.h> //I2C


//MQ2====================
//VARIABLES
float Ro_MQ2 = 3300.0;   //sensor resistance at 1000ppm of H2 in the clean air
int val_MQ2 = 0;        
float Vrl_MQ2 = 0.0;
float Rs_MQ2 = 0.0;
float ratio_MQ2 = 0.0;
//============================
//MQ4====================
//VARIABLES
float Ro_MQ4 = 15000.0;  
int val_MQ4 = 0;        
float Vrl_MQ4 = 0.0;
float Rs_MQ4 = 0.0;
float ratio_MQ4 = 0.0;
//============================
//MQ4====================
//VARIABLES
float Ro_MQ7 = 5000.0;  
int val_MQ7 = 0;        
float Vrl_MQ7 = 0.0;
float Rs_MQ7 = 0.0;
float ratio_MQ7 = 0.0;
//============================



//CONSTANTS
const uint32_t update_int = 5000; //ms
const uint8_t payload_size = 1 + 2 * sizeof(float) + sizeof(uint32_t) + 4 * sizeof(uint8_t); //max is 84 Bytes
const float thresold_factor = 1.2; //presence of gas

struct
{
  static const uint8_t led = 13;
  static const uint8_t mq2 = A0;
  static const uint8_t mq4 = A1;
  static const uint8_t mq7 = A2;
  static const uint8_t mq7_vcc = 6; //PWM
  static const uint8_t dht = 7; //1-Wire
  static const uint8_t pir = 3; //INT1
} 
Pins;

//GLOBALS

//GAS
uint32_t last_logged, last_mq7_switch, heating_int;
bool is_heating_high; //still takes 1 Byte
uint16_t aval_mq2, aval_mq4, aval_mq7; //0..1023
uint16_t mean_mq2, mean_mq4, mean_mq7;
uint16_t cal_MQ2, cal_MQ4, cal_MQ7; //ppm
uint8_t scaled_MQ2, scaled_MQ4, scaled_MQ7; //0-10

//DHT
DHT dht(Pins.dht, DHTTYPE);
float temperature, humidity;

//PIR
volatile bool is_movement; //volatile because is changed in ISR

//LIGHT
double ambient_light = 770; //0..120000 lux
unsigned int ls_data0, ls_data1; //light sensor
bool ls_gain; 
uint8_t ls_integration_time;
uint16_t ls_integration_time_ms;

//Door
int Door_Sensor_Pin = 2; // choose the Door_Sensor_Pin
uint8_t door_open = 0;

unsigned char payload[payload_size], prev_payload[payload_size];
bool is_changed; //if we have new, different data from sensors
SoftwareSerial ss(4, 5); //use HW ser?
XBee xbee = XBee();
XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x00000000); //0x00 for coord and 0x0F for broadcast
ZBTxRequest zbtxreq = ZBTxRequest(destAddr64, payload, sizeof(payload)); //= frame

void setup()
{
  //async, 1start + 8data + 1stop, no parity - bit
  //disables the general use of pins 0, 1
  DB_begin(115200);
  DB_printsln("\nSETUP");

  //could move it to HW serial
  ss.begin(9600);
  xbee.setSerial(ss);

  //Ambient light

  //declare output / input_pullup pins if any
  pinMode(Pins.led, OUTPUT); //nice to have a led for debug
  pinMode(Pins.mq7_vcc, OUTPUT);
  pinMode(Pins.pir, INPUT_PULLUP); //avoid false alarms
  pinMode(Door_Sensor_Pin, INPUT);
  //set pin states
  digitalWrite(Pins.mq7_vcc, HIGH); //start @5V

  //init vars
  //DB::LOOP_COUNT = 0;
  heating_int = 90000; //90s @ 5v
  is_heating_high = true;
  mean_mq2 = 200;
  mean_mq4 = 200;
  mean_mq7 = 200;
  //calibrated gas values to ppm
  cal_MQ2 = 200;
  cal_MQ4 = 300;
  cal_MQ7 = 35;

  attachInterrupt(1, alarm_pir, CHANGE); //pin 3
}

void loop()
{
  unsigned long current_time;

  current_time = millis();
  if (current_time - last_mq7_switch > heating_int)
  {
    last_mq7_switch = current_time;
    if (is_heating_high)
    {
      DB_printsln("\nswitching to 1.4V for 60s\n");
      is_heating_high = false;
      heating_int = 60000; //60s @ 1.4V
      analogWrite(Pins.mq7_vcc, 70);
    }
    else
    {
      aval_mq7 = analogRead(Pins.mq7); //make the read
      //mean_mq7 = 0.8 * mean_mq7 + 0.2 * aval_mq7;
      val_MQ7=aval_mq7;
      Vrl_MQ7 = val_MQ7 * ( 5.00 / 1023.0  );      // V
      Rs_MQ7 = 20000 * ( 5.00 - Vrl_MQ7) / Vrl_MQ7 ;   // Ohm 
      ratio_MQ7 =  Rs_MQ2/Ro_MQ7;   
      cal_MQ7 = get_CO(ratio_MQ7);
      if (cal_MQ7 < 2) scaled_MQ7 = 0;
      else if ((cal_MQ7 > 2) & (cal_MQ7 < 32)) scaled_MQ7 = 1;
      else if ((cal_MQ7 > 32) & (cal_MQ7 < 50)) scaled_MQ7 = 2;
      else if ((cal_MQ7 > 50) & (cal_MQ7 < 80)) scaled_MQ7 = 3;
      else if ((cal_MQ7 > 80) & (cal_MQ7 < 126)) scaled_MQ7 = 4;
      else if ((cal_MQ7 > 126) & (cal_MQ7 < 200)) scaled_MQ7 = 5;
      else if ((cal_MQ7 > 200) & (cal_MQ7 < 317)) scaled_MQ7 = 6;
      else if ((cal_MQ7 > 317) & (cal_MQ7 < 502)) scaled_MQ7 = 7;
      else if ((cal_MQ7 > 502) & (cal_MQ7 < 796)) scaled_MQ7 = 8;
      else if ((cal_MQ7 > 796) & (cal_MQ7 < 1262)) scaled_MQ7 = 9;
      else if (cal_MQ7 > 1262) scaled_MQ7 = 10;
      //=========================
      DB_printsln("\nswitching to 5V for 90s\n");
      is_heating_high = true;
      heating_int = 90000; //90s @ 5v
      digitalWrite(Pins.mq7_vcc, HIGH);
    }
  }
  
  door_open = digitalRead(Door_Sensor_Pin);
  if (door_open == 1)  {
    //send data at once
      read_sensors();
      process_data();
      debug();
      xbee.send(zbtxreq);
   // if changes occure while the door is open, send them only if different
     while (digitalRead(Door_Sensor_Pin) == 1) {
        current_time = millis();
        if (current_time - last_logged > update_int) {
                    DB_print(++DB::LOOP_COUNT);
                    DB_prints(" ");
                    read_sensors();
                    last_logged = current_time;
                    process_data(); //construct payload
                    send();
         }
    }
  }
  else {
  current_time = millis();
  if (current_time - last_logged > update_int)
  {
    DB_print(++DB::LOOP_COUNT);
    DB_prints(" ");
    read_sensors();
    last_logged = current_time;
    process_data(); //construct payload
    send();
   }
}
}

void read_sensors()
{
  //test MQ2==============
  aval_mq2 = analogRead(Pins.mq2);
  val_MQ2=aval_mq2;
  Vrl_MQ2 = val_MQ2 * ( 5.00 / 1023.0  );      // V
  //Rload=5kOhm
  Rs_MQ2 = 5000 * ( 5.00 - Vrl_MQ2) / Vrl_MQ2 ;   // Ohm 
  ratio_MQ2 =  Rs_MQ2/Ro_MQ2;   
  cal_MQ2 = get_LPG(ratio_MQ2);
  if (cal_MQ2 < 200) scaled_MQ2 = 0;
  else if ((cal_MQ2 > 200) & (cal_MQ2 < 296)) scaled_MQ2 = 1;
  else if ((cal_MQ2 > 296) & (cal_MQ2 < 437)) scaled_MQ2 = 2;
  else if ((cal_MQ2 > 437) & (cal_MQ2 < 647)) scaled_MQ2 = 3;
  else if ((cal_MQ2 > 647) & (cal_MQ2 < 956)) scaled_MQ2 = 4;
  else if ((cal_MQ2 > 956) & (cal_MQ2 < 1414)) scaled_MQ2 = 5;
  else if ((cal_MQ2 > 1414) & (cal_MQ2 < 2091)) scaled_MQ2 = 6;
  else if ((cal_MQ2 > 2091) & (cal_MQ2 < 3092)) scaled_MQ2 = 7;
  else if ((cal_MQ2 > 3092) & (cal_MQ2 < 4573)) scaled_MQ2 = 8;
  else if ((cal_MQ2 > 4573) & (cal_MQ2 < 6762)) scaled_MQ2 = 9;
  else if (cal_MQ2 > 6762) scaled_MQ2 = 10;
  //=========================

  //test MQ4=================
   aval_mq4 = analogRead(Pins.mq4);
   val_MQ4=aval_mq4;
   Vrl_MQ4 = val_MQ4 * ( 5.00 / 1023.0  );      // V
   //Rload=20kOhm
   Rs_MQ4 = 20000 * ( 5.00 - Vrl_MQ4) / Vrl_MQ4 ;   // Ohm 
   ratio_MQ4 =  Rs_MQ4/Ro_MQ4;   
   cal_MQ4 = get_NG(ratio_MQ4);
   if (cal_MQ4 < 300) scaled_MQ4 = 0;
   else if ((cal_MQ4 > 300) & (cal_MQ4 < 500)) scaled_MQ4 = 1;
   else if ((cal_MQ4 > 500) & (cal_MQ4 < 835)) scaled_MQ4 = 2;
   else if ((cal_MQ4 > 835) & (cal_MQ4 < 1392)) scaled_MQ4 = 3;
   else if ((cal_MQ4 > 1392) & (cal_MQ4 < 2322)) scaled_MQ4 = 4;
   else if ((cal_MQ4 > 2322) & (cal_MQ4 < 3873)) scaled_MQ4 = 5;
   else if ((cal_MQ4 > 3873) & (cal_MQ4 < 6460)) scaled_MQ4 = 6;
   else if ((cal_MQ4 > 6460) & (cal_MQ4 < 10775)) scaled_MQ4 = 7;
   else if ((cal_MQ4 > 10775) & (cal_MQ4 < 17972)) scaled_MQ4 = 8;
   else if ((cal_MQ4 > 17972) & (cal_MQ4 < 29977)) scaled_MQ4 = 9;
   else if (cal_MQ4 > 29977) scaled_MQ4 = 10;
  //=========================

  //DHT
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  //door
  
}


//=====MQ2 datasheet===
float get_LPG (float ratio){
  float ppm = 0.0;
  //ppm = 10131*pow(2.71,-2.504*ratio);
  ppm=665.18*pow(ratio,-1.73);
  return ppm;
}
//===============
//=====MQ4 datasheet==
float get_NG (float ratio){
  float ppm = 0.0;
  ppm=958.36*pow(ratio,-2.917);
  return ppm;
}
//===============
//=====MQ7 datasheet==
float get_CO(float ratio){
  float ppm = 0.0;
  ppm=103.16*pow(ratio,-1.498);
  return ppm;
}
//===============

void process_data()
{
  //TODO:
  //initial RL calibration
  //adjust by current temp & humidity
  //transform to ppm with datasheet graphs

  mean_mq2 = 0.95 * mean_mq2 + 0.05 * aval_mq2;
  mean_mq4 = 0.95 * mean_mq4 + 0.05 * aval_mq4;

  //combine the presence of 3 gases in a byte
  //has 8b, which means 8 different flags can be stored in it
  (aval_mq2 > thresold_factor * mean_mq2) ? (payload[0] |= 1) : (payload[0] &= ~1);
  (aval_mq4 > thresold_factor * mean_mq4) ? (payload[0] |= 2) : (payload[0] &= ~2);
  (aval_mq7 > thresold_factor * mean_mq7) ? (payload[0] |= 4) : (payload[0] &= ~4);
  is_movement ? (payload[0] |= 8) : (payload[0] &= ~8);
  //possibly add other flags here

  //float to bytes
  memcpy(&payload[1], &humidity, sizeof(humidity));
  memcpy(&payload[1 + sizeof(float)], &temperature, sizeof(temperature));
  memcpy(&payload[1 + 2 * sizeof(float)], &ambient_light, sizeof(ambient_light));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t)], &scaled_MQ2, sizeof(scaled_MQ2));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + sizeof(uint8_t)], &scaled_MQ4, sizeof(scaled_MQ4));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(uint8_t)], &scaled_MQ7, sizeof(scaled_MQ7));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + 3 * sizeof(uint8_t)], &door_open, sizeof(door_open));
}

void send()
{
  for (uint8_t i = 0; i < payload_size; i++)
  { //check if a change in the environment has occured
    if (
    (abs(prev_payload[3] - payload[3])>=4) //an increase or decrease of 0.5 %RH
    || 
    (abs(prev_payload[7] - payload[7])>=4) //an increase or decrease of 0.5 degrees
    ||
    (prev_payload[13] != payload[13]) //a change in MQ2
    ||
    (prev_payload[14] != payload[14]) // a change in MQ4
    ||
    (prev_payload[15] != payload[15]) // a change in MQ7
	||
    (prev_payload[0] != payload[0]) // PIR change
    )
    {
      is_changed = true;
    }
  }

  if (is_changed)
  {
    xbee.send(zbtxreq);
    memcpy(prev_payload, payload, sizeof(payload));
    is_movement = false;
    is_changed = false;
    debug();
  }
}

void debug()
{
  DB_println();
  DB_print(scaled_MQ2); 
  DB_prints(" ");
  DB_print(scaled_MQ4); 
  DB_prints(" ");
  DB_print(scaled_MQ7); 
  DB_prints(" ");
  DB_print(humidity); 
  DB_prints(" ");
  DB_print(temperature); 
  DB_prints(" ");
  DB_print(door_open); 
  DB_prints(" ");
  DB_println();
  for (uint8_t i = 0; i < payload_size; i++)
  {
    DB_printf(payload[i], HEX);
    DB_prints(" ");
  }
  DB_println();
  DB_println();
  DB_print("================");
}

void alarm_pir()
{
  if (!digitalRead(Pins.pir))
  {
    is_movement = true;
    digitalWrite(Pins.led, HIGH);
  }
  else
  {
    //is_movement = false; //don't do that here; might miss movements between sends
    digitalWrite(Pins.led, LOW);
  }
}

void serialEvent()
{ //called everytime when hw serial buffer is not empty
  char c = Serial.read();

  //project specific commands
  if (c == 's')
  { //send the payload anyway
    is_changed = true;
  }

  DB::serialEvent(c);
}