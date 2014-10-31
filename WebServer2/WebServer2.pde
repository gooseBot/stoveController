/*
  Web  Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 4 Sep 2010
 by Tom Igoe
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h> 
#define maxLength 30


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1, 177 };
String responseString = String(maxLength);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
Server server(80);

// Setup Servo and pot
Servo myServo;  // create servo object to control a servo 
int potPin = 3;  // analog pin used to connect the potentiometer
int servoPin = 3;  //digital pin used to control the servo
int potVal;    // variable to read the value from the analog pin 
int webServoVal;
String pword;
String stove;

void setup()
{
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  myServo.attach(servoPin);   // start servo
  setServo(mapPotVal(analogRead(servoPin)));   
}

void loop()
{
  //read the pot and remember the setting
  potVal = analogRead(potPin);
  
  // listen for incoming clients
  Client client = server.available();
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
            
            stove = responseString.substring((stovePos+7), (ampersand));
            
            int End = responseString.indexOf("H");
            pword = responseString.substring((ampersand+7), (End-1));
            if (pword=="jebg" && stove=="on"){
              setServo(15);
            }              
            if (pword=="jebg" && stove=="off"){
              setServo(180);
            }          
          }  
  
          int stoveOn=1;
          if (myServo.read() > 100)
            stoveOn=0;
                  
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html><head><meta name='viewport' content='width=320' /></head><body>");
          client.println("<h1>Stove Control</h1>");
          client.println("<form method=get>");
          client.print("<input type='radio' name='stove' value='on'");
          if (stoveOn==1) {
            client.println("checked />on");
          } else {
            client.println(" />on");
          }
          client.print("&nbsp;<input type='radio' name='stove' value='off'");
          if (stoveOn==0) {
            client.println("checked />off");
          } else {
            client.println(" />off");
          }
          client.println("&nbsp;code:<input type=text size=4 name=pword>");
          client.println(" <input type=submit value=submit></form></body></html>"); 

          client.print("potentiometer input is ");
          potVal = analogRead(potPin);
          client.print(potVal);        
          client.println("<br />");

          client.print("Servo is at position ");
          client.print(myServo.read());
          client.println("<br />"); 
                    
          client.print("stove ");
          client.print(stove);
          client.println("<br />"); 
          
          client.print("code ");
          client.print(pword);
          client.println("<br />");           
 
          client.print("response ");
          client.print(responseString);
          client.println("<br />");      
           
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
  
  //if pot has changed by more than x since last loop then assume manual overwride and set at new pot value
  delay(5);
  int curPotVal=analogRead(potPin);
  if (abs(curPotVal - potVal) > 5) { 
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
  delay(15);
}

int mapPotVal(int potReading)
{
  return map(potReading, 0, 1023, 1, 179);
}
