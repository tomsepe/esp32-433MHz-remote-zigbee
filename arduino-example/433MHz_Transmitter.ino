/*
Author: Danny van den Brande, Arduinosensors.nl. BlueCore Tech.
*/
 
 
#include <VirtualWire.h>
int Button1 = 2; 
boolean val ;
 
void setup() {
    Serial.begin(9600);
    pinMode(13,OUTPUT);
    pinMode (Button1, INPUT);
    vw_set_tx_pin(12); 
    vw_setup(4000);// speed of data transfer in bps, can max out at 10000
}
 
void loop()
{
  char *senddata="0123456789abcdef";
  val = digitalRead (Button1); 
    if (val == 0){
    Serial.print(val);
    digitalWrite(13,1); 
    vw_send((uint8_t *)senddata, strlen(senddata));
    vw_wait_tx(); 
    digitalWrite(13,0); 
    delay(1000);
    }
}
