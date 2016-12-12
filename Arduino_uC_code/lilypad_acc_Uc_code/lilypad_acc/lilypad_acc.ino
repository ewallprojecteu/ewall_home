/**
* Edited by Razvan Craciunescu. (rcraciunescu@radio.pub.ro) 
* UPB 
* eWall FP7 project
* LilyPad uC accelerometer code 
*/


#include <XBee.h> //copy XBee folder to this in arduino-1.0.x/libraries
#include <SoftwareSerial.h> //only for the lilypad

SoftwareSerial ss(9, 10); //Rx Tx
XBee xbee = XBee();

const int xpin = A3;                  // x-axis of the accelerometer
const int ypin = A4;                  // y-axis
const int zpin = A5;
float zero_G = 512.0;           //this is an indicative value. depends from device to device    
float scale = 102.3;        //330 mV/G × (1023 ADC units) / 3.3 V = 102.3 (ADC units)/G
float G = 9.81;             // the gravitational acceleration

const uint8_t payload_size = 1 + 3 * sizeof(float);//payload size 
unsigned char payload[payload_size];

XBeeAddress64 addr64 = XBeeAddress64(0x0, 0x0); // coordinator address
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();


void setup() {
  
  Serial.begin(9600);
  ss.begin(9600); //uC to Xbee
  xbee.setSerial(ss); //uC to Xbee
}

void loop() {
  
  
  //identification byte
  payload[0]|=32;   //for the flags. is not neccesarily 
  
  //reading acc pin data
  int x = analogRead(xpin);
  delay(1); 
  int y = analogRead(ypin);
  delay(1);  
  int z = analogRead(zpin);
  delay(1);
  
  //converting to acc - 1->1G->m/s2
  float ax=(((float)x - zero_G)/scale)*G;
  Serial.println(ax);Serial.println("\n");
  float ay=(((float)y - zero_G)/scale)*G;
  Serial.println(ay);Serial.println("\n");
  float az=(((float)z - zero_G)/scale)*G;
  Serial.println(az);Serial.println("\n");
  
  //payload
  memcpy(&payload[1], &ax, sizeof(ax));
  memcpy(&payload[1 + sizeof(float)], &ay, sizeof(ay));
  memcpy(&payload[1 + 2 * sizeof(float)], &az, sizeof(az));
  
  //sending the acc data
  xbee.send(zbTx);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!

    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {

      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
      Serial.print("succes");

      } 
    }
  } 
  else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");  
    Serial.println(xbee.getResponse().getErrorCode());
  } 

  delay(50); //50ms sending rate
}