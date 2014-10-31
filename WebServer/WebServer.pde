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
#define maxLength 25


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1, 177 };
String inString = String(maxLength);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
Server server(80);

// Setup Servo and pot
Servo myServo;  // create servo object to control a servo 
int potpin = 3;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin 
int curPotVal;
int s;

void setup()
{
  //Serial.begin(9600);           // set up Serial library at 9600 bps
  //Serial.println("servoTest!");  

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  myServo.attach(3);   // start servo
  int newValue = map(analogRead(3), 0, 1023, 2, 178);
  if (newValue > 170)
    newValue = 170;
  if (newValue < 10)
    newValue = 10;
  myServo.write(newValue);    
}

void loop()
{
  //read the pot and remember the setting
  val = analogRead(3);
  //Serial.println(val); 
  
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
        if (inString.length() < maxLength) {
          inString += c;
        }
        if (c == '\n' && currentLineIsBlank) {
          //Serial.println(inString);
          if (inString.indexOf("?")>-1) {  
            int Pos_servo = inString.indexOf("s");
            int End = inString.indexOf("H");
            String output = inString.substring((Pos_servo+2), (End-1));
            char charArray[output.length() + 1];
            output.toCharArray(charArray, output.length() + 1);
            s = atoi(charArray);
            if (s > 170)
              s = 170;
            if (s < 10)
              s = 10;
            myServo.write(s);                 
            delay(15);   // waits for the servo to get there          
          }          
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.println("<html><head></head><body>");
          client.println("<h1>Stove Control</h1>");
          client.print("<form method=get>Servo Position:<input type=text size=4 name=s value=");
          client.print(myServo.read());
          client.println(">&nbsp;<input type=submit value=submit></form></body></html>"); 

          client.print("analog input 3 is ");
          val = analogRead(3);
          client.print(val);        
          client.println("<br />");

          client.print("Servo is at position ");
          client.print(myServo.read());
          client.println("<br />"); 
          
          client.print("servo value from form ");
          client.print(s);
          client.println("<br />"); 
 
          client.print("instring value ");
          client.print(inString);
          client.println("<br />");      
      
          inString="";  
          
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
  curPotVal = analogRead(3);
  if (abs(curPotVal - val) > 5) { 
    int newValue = map(curPotVal, 0, 1023, 2, 178);
    if (newValue > 170)
      newValue = 170;
    if (newValue < 10)
      newValue = 10;
    myServo.write(newValue);                  // sets the servo position according to the scaled value 
    delay(15);
  }
}
