#include "WiFiEsp.h"
#include <TimerOne.h>
#include "SoftwareSerial.h"

SoftwareSerial Serial1(2,3);
char ssid[] = "mynetwork"; //set to name of network
char password[] = "12345678";
int status = WL_IDLE_STATUS;
WiFiEspServer server(69);

const int ledPin = 11;
volatile int output;
unsigned long slow   = 1000000;
unsigned long medium = 500000;
unsigned long fast   = 100000;
unsigned long blinkSpeed = fast;
int high = 255;
int dim = 32;
bool isBlinking = false;
volatile int brightness = high;

void setup() {
  pinMode(ledPin, OUTPUT);
  Timer1.initialize(blinkSpeed);
  Timer1.attachInterrupt(toggleOutput);
  Timer1.stop();
  
  Serial1.begin(9600);      //Software serial connection to the wifi shield (ESP-12E)
  Serial.begin(115200);     //Hardware serial connection to the computer for debugging
  WiFi.init(&Serial1);
  
  while( status != WL_CONNECTED)
  {
    Serial.print("Connecting to: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
  }
  printWifiStatus();

  server.begin();
}


/*
 * read characters from the buffer until it is empty
 * if character is 0-8; process as command, ignore otherwise
*/
void loop() {
  WiFiEspClient client = server.available();
  if(client) {
    while(client.connected()){
      if(client.available()){
        char c = client.read();
        if(c >= '0' && c <= '7'){
          Serial.print("Received command: ");
          Serial.println(c);
          processCmd(c);
        }
      }
      else break;
    }
  }    
}

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void processCmd(char c)
{
  /*
   * 0 = LED OFF
   * 1 = LED ON STEADY
   * 2 = LED BLINK
   * 3 = BRIGHTNESS LOW
   * 4 = BRIGHTNESS HIGH
   * 5 = BLINKSPEED SLOW
   * 6 = BLINKSPEED MEDIUM
   * 7 = BLINKSPEED FAST
   */
  
  switch(c)
  {
    case ('0'): 
      Timer1.stop();
      isBlinking = false;
      output = LOW;
      analogWrite(ledPin, output);
      break;
    case ('1'): 
      Timer1.stop();
      isBlinking = false;
      output = brightness;
      analogWrite(ledPin, output);
      break;
    case ('2'): 
      Timer1.setPeriod(blinkSpeed);
      isBlinking = true;
      break;
    case ('3'): 
      brightness = dim;
      if (output) analogWrite(ledPin, brightness);
      break;
    case ('4'): 
      brightness = high;
      if (output) analogWrite(ledPin, brightness);
      break;
    case ('5'): 
      blinkSpeed = slow;
      if (isBlinking) Timer1.setPeriod(blinkSpeed);
      break;
    case ('6'): 
      blinkSpeed = medium;
      if (isBlinking) Timer1.setPeriod(blinkSpeed);
      break;
    case ('7'):
      blinkSpeed = fast;
      if (isBlinking) Timer1.setPeriod(blinkSpeed);
      break;
  }
}

/*
 * ISR FOR TIMER
*/
void toggleOutput()
{
  if(output == LOW)
    output = brightness;
  else
    output = LOW;
  analogWrite(ledPin, output);
}
