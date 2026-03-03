/*
Author: Danny van den Brande, Arduinosensors.nl. BlueCore Tech.
*/
#include <VirtualWire.h>

int Buzzer = 6;
int Relay = 2;

// here i set up the tones, you can change them @ void loop.
int tones[] = {261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493, 523 ,554};
//              1    2    3    4    5    6    7    8    9    10   11   12   13   14
// You can add more tones but i added 14. Just fill in what tone you would like to use, @ void loop you see " tone(Buzzer, tones[12]); " below,  digitalWrite(Buzzer, HIGH);
// here you can change the tones by filling in a number between 1 and 14

void setup()
{
    Serial.begin(9600); //we wanna be able to read what we got
    vw_set_rx_pin(12);//connect the receiver data pin to pin 12
    vw_setup(4000);  // speed of data transfer in bps, maxes out at 10000
    pinMode(13, OUTPUT);
    pinMode (Buzzer, OUTPUT); 
    pinMode (Relay, OUTPUT); 
 
    vw_rx_start();       // Start the receiver PLL running
    for (int i = 0; i < Buzzer; i++)  ;
}
    void loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
 
    if (vw_get_message(buf, &buflen)) // if we get a message that we recognize on this buffer...
    {
 
    String out = "";
    
 // we have data coming in, let's aknowledge somehow that we've received it

    digitalWrite(Buzzer, HIGH);
    digitalWrite(13, HIGH);
    tone(Buzzer, tones[2]);
    delay(100);
    digitalWrite(Buzzer, LOW);
    digitalWrite(13, LOW);
    noTone(Buzzer);
    delay(100);
    digitalWrite(Buzzer, HIGH);
    digitalWrite(13, HIGH);
    tone(Buzzer, tones[14]);
    delay(100);
    digitalWrite(Buzzer, LOW);
    digitalWrite(13, LOW);
    noTone(Buzzer);
    delay(100);

    digitalWrite(Buzzer, HIGH);
    digitalWrite(13, HIGH);
    tone(Buzzer, tones[2]);
    delay(100);
    digitalWrite(Buzzer, LOW);
    digitalWrite(13, LOW);
    noTone(Buzzer);
    delay(100);
    digitalWrite(Buzzer, HIGH);
    digitalWrite(13, HIGH);
    tone(Buzzer, tones[14]);
    delay(100);
    digitalWrite(Buzzer, LOW);
    digitalWrite(13, LOW);
    noTone(Buzzer);
    delay(100);
    
    digitalWrite(Relay, HIGH);
    digitalWrite(13, HIGH);
    
    for (int i = 0; i<buflen; i++)
    {
       out +=(char)buf[i]; // fill the string with the data received 
    }
    Serial.println(out); // simple enough
//    digitalWrite(13,0); //transmission ended
    
   }
}
