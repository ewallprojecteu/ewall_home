/*
* AUTHOR: Loredan E. Bucur & Maria Mitoi &Razvan Craciunescu
 *loredan.bucur@radio.pub.ro / maria.mitoi@radio.pub.ro
 * UPB
 * eWall FP7 project
 * Environmental sensors for Arduino Uno
 * LAST_EDIT: Feb. 2016
 * -------------------------------------
 * Humidity & Temperature & PIR sensors 
 * -------------------------------------
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
//float Ro_MQ2 = 3300.0;   //sensor resistance at 1000ppm of H2 in the clean air
float Ro_MQ2;
int val_MQ2 = 0;        
float Vrl_MQ2 = 0.0;
float Rs_MQ2 = 0.0;
float ratio_MQ2 = 0.0;
//==================


//CONSTANTS
const uint32_t update_int = 5000; //ms
const uint8_t payload_size = 1 + 2 * sizeof(float) + sizeof(uint32_t) + 4 * sizeof(uint8_t); //max is 84 Bytes
const float thresold_factor = 1.2; //presence of gas

struct
{
  static const uint8_t led = 13;
  static const uint8_t dht = 7; //1-Wire
  static const uint8_t mq2 = A0; //MQ2
  static const uint8_t pir = 3; //INT1
  static const uint8_t door = 2; //INT0
} 
Pins;

//GLOBALS

//Gas

uint32_t last_logged;

uint8_t aval_mq2;
uint16_t cal_MQ2;
uint8_t scaled_MQ2, scaled_MQ4=0, scaled_MQ7=0; //0-10

//DHT
DHT dht(Pins.dht, DHTTYPE);
float temperature, humidity;

//PIR
volatile bool is_movement; //volatile because is changed in ISR


//LIGHT
double ambient_light = 0;//0..120000 lux - no ambient light sensor for this stege


//Door
volatile uint8_t door_open_interupt;
uint8_t door_open;


unsigned char payload[payload_size], prev_payload[payload_size];
bool is_changed; //if we have new, different data from sensors
SoftwareSerial ss(4, 5); //use HW ser?
XBee xbee = XBee();
XBeeAddress64 destAddr64 = XBeeAddress64(0x00000000, 0x00000000); //0x00 for coord and 0x0F for broadcast
ZBTxRequest zbtxreq = ZBTxRequest(destAddr64, payload, sizeof(payload)); //= frame

void setup()
{

  DB_begin(115200);
  DB_printsln("\nSETUP");
  cal_MQ2 = 200;

  //could move it to HW serial
  ss.begin(9600);
  xbee.setSerial(ss);

  //declare output / input_pullup pins if any
  pinMode(Pins.led, OUTPUT); //nice to have a led for debug
  pinMode(Pins.pir, INPUT_PULLUP); //avoid false alarms
  pinMode(Pins.door, INPUT);
  
  //calibrate MQ2 - determine Ro
  Ro_MQ2 = MQCalibration(Pins.mq2);

  attachInterrupt(0,alarm_door, CHANGE); //pin 2
  attachInterrupt(1, alarm_pir, CHANGE); //pin 3
  //calibrated gas values to ppm
  
}

void loop()
{
  unsigned long current_time;
  current_time = millis();
  if (current_time - last_logged > update_int)
  {
    read_sensors();
    last_logged = current_time;
    process_data(); //construct payload
    send();
  }
}

void read_sensors()
{

  //test MQ2==============
  aval_mq2 = analogRead(Pins.mq2);
  val_MQ2=aval_mq2;
  Vrl_MQ2 = val_MQ2 * ( 5.00 / 1023.0  );      // V
  //Rload=10kOhm
  Rs_MQ2 = 10000 * ( 5.00 - Vrl_MQ2) / Vrl_MQ2 ;   // Ohm 
  ratio_MQ2 =  Rs_MQ2/Ro_MQ2;   
  cal_MQ2 = get_LPG(ratio_MQ2);

  if (cal_MQ2 < 180) scaled_MQ2 = 0;
  else if ((cal_MQ2 > 180) & (cal_MQ2 < 296)) scaled_MQ2 = 1;
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

  //DHT
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

//calibrate MQ2 - determine R0
float MQCalibration(int mq_pin)
{
  int i;
  float val=0;
  uint8_t raw_adc=analogRead(Pins.mq2);
 
  for (i=0;i<50;i++) {            //take multiple samples
    val += ((float)10000*(1023-raw_adc)/raw_adc);
    delay(500);
  }
  val = val/50;                   //calculate the average value
 
  val = val/9.83;                        //divided by RO_CLEAN_AIR_FACTOR yields the Ro 
                                                        //according to the chart in the datasheet 
  return val; 
}


//=====MQ2 datasheet===
float get_LPG (float ratio){
  float ppm = 0.0;
  ppm=665.18*pow(ratio,-1.73);
  return ppm;
}
//===============

void process_data()
{
  is_movement ? (payload[0] |= 8) : (payload[0] &= ~8);

  // digitalRead(3)==LOW for https://www.adafruit.com/products/189 PIR sensor
  //digitalRead(3)==HIGH for https://www.sparkfun.com/products/retired/8630 PIR sensor
  if (digitalRead(3)==LOW) {
    is_movement=false;
  }

  if (door_open_interupt==1) door_open=0;
  else door_open=1;

  //float to bytes
  memcpy(&payload[1], &humidity, sizeof(humidity));
  memcpy(&payload[1 + sizeof(float)], &temperature, sizeof(temperature));
  memcpy(&payload[1 + 2 * sizeof(float)], &ambient_light, sizeof(ambient_light));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t)], &scaled_MQ2, sizeof(scaled_MQ2));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + sizeof(uint8_t)], &scaled_MQ4, sizeof(scaled_MQ4));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + 2 * sizeof(uint8_t)], &scaled_MQ7, sizeof(scaled_MQ7));
  memcpy(&payload[1 + 2 * sizeof(float) + sizeof(uint32_t) + 3 * sizeof(uint8_t)], &door_open, sizeof(door_open));
}

void  send()
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
      (prev_payload[0] != payload[0])
      )
    {
      is_changed = true;
    }
  }

  if (is_changed)
  {
    xbee.send(zbtxreq);
    memcpy(prev_payload, payload, sizeof(payload));
    is_changed = false;
    debug();
  }
}

void alarm_pir()
{


  // if (digitalRead(Pins.pir)) for https://www.adafruit.com/products/189 PIR sensor
  // if (!digitalRead(Pins.pir)) for https://www.sparkfun.com/products/retired/8630 PIR sensor    
  if (digitalRead(Pins.pir))
  {
    is_movement = true;
    digitalWrite(Pins.led, HIGH);
  }
  else
  {
    digitalWrite(Pins.led, LOW);


  }
}

// when the door is opened or closed the data is sent to the gateway
void alarm_door(){
  door_open_interupt =   digitalRead(Pins.door);
  read_sensors();
  process_data();
  xbee.send(zbtxreq);
//  debug();
  memcpy(prev_payload, payload, sizeof(payload));

}

void debug()
{
  DB_print("================");
  DB_println();
  DB_print(scaled_MQ2); 
  DB_prints(" -------");
  DB_print(scaled_MQ4); 
  DB_prints(" -------");
  DB_print(scaled_MQ7); 
  DB_prints(" -------");
  DB_print(humidity); 
  DB_prints(" -------");
  DB_print(temperature); 
  DB_prints(" -------");
  DB_print(door_open); 
  for (uint8_t i = 0; i < payload_size; i++)
  {
    DB_prints(" -------");
    DB_printf(payload[i], HEX);

  }
  DB_println();
  DB_println();
  DB_print("================");
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










