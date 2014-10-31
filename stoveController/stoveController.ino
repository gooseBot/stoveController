#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h> 
#define maxLength 30
#include <EEPROM.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1, 177 };
String responseString = String(maxLength);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer  server(80);

// Setup Servo and pot
Servo myServo;  // create servo object to control a servo 
int potPin = 3;  // analog pin used to connect the potentiometer
int servoPin = 3;  //digital pin used to control the servo
int potVal;    // variable to read the value from the analog pin 
int webServoVal;
boolean potMode=false;  //on startup we don't want the pot to set the stove
String pword;

void setup()
{
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  myServo.attach(servoPin);   // start servo
  
  // read and set last servo val from eeprom
  setServo(EEPROM.read(1));
  
  // read the pot now and delay, seems like it takes a bit
  //   of time before the pot settles down on a restart
  //   and gives consistent values
  potVal = analogRead(potPin);
  delay(30);
}

void loop()
{
  //read the pot and remember the setting
  potVal = analogRead(potPin);
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (responseString.length() < maxLength) {
          responseString += c;
        }
        if (c == '\n' && currentLineIsBlank) {
          if (responseString.indexOf("?")>-1) {  
            int stovePos = responseString.indexOf("?");
            int ampersand = responseString.indexOf("&");            
            String stove = responseString.substring((stovePos+7), (ampersand));
            
            int End = responseString.indexOf("H");
            pword = responseString.substring((ampersand+7), (End-1));
            if (pword=="jebg" && stove=="on"){
              setServo(15);
              potMode=false;
            }              
            if (pword=="jebg" && stove=="off"){
              setServo(155);
              potMode=false;
            }          
          }  
                    
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          
          client.println("<status>");
          if (myServo.read() > 100) {
            client.println("<state>off</state>");
          } else {
            client.println("<state>on</state>");
          }
          
          potVal = analogRead(potPin);
          client.print("<pot>");
          client.print(potVal);    
          client.println("</pot>");
          
          client.print("<servo>");
          client.print(myServo.read());
          client.println("</servo>");

          client.print("<response>");
          client.print(responseString);
          client.println("</response>");
          
          client.println("</status>");
           
          responseString="";  
          
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
  
  //if pot has changed by more than x since last loop then assume
  //   manual overwride and set at new pot value
  delay(5);
  int curPotVal=analogRead(potPin);
  if (abs(curPotVal - potVal) > 5 || potMode) { 
    potMode=true;
    setServo(mapPotVal(curPotVal));
  }
}

void setServo(int value)
{
  if (value > 170)
    value = 170;
  if (value < 10)
    value = 10;
  myServo.write(value); 
  EEPROM.write(1,value);  //save servo value for use if a reset occurs
  delay(15);
}

int mapPotVal(int potReading)
{
  return map(potReading, 0, 1023, 1, 179);
}


