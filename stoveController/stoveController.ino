// Include the ESP8266 WiFi library. (Works a lot like the Arduino WiFi library.)
#include <ESP8266WiFi.h>
#include <Servo.h> 
#define maxLength 30
#include <EEPROM.h>

//////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiSSID[] = "34er54";
const char WiFiPSK[] = "jeep~fish*63";

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int buttonPin = 15; // Digital pin to be read
const int servoPin = 2;  //digital pin used to control the servo

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte ip[] = { 192,168,1, 177 };
String responseString = String(maxLength);

// Initialize the WiFiServer server library
WiFiServer server(80);

// Setup Servo and pot
Servo myServo;  // create servo object to control a servo 

int webServoVal;
String pword;

void setup()
{
  // start the Ethernet connection and the server:
  connectWiFi();
  //Ethernet.begin(mac, ip);
  server.begin();
  
  myServo.attach(servoPin);   // start servo
  
  // read and set last servo val from eeprom
  setServo(EEPROM.read(1));  
}

void loop()
{
  
  // listen for incoming clients
  WiFiClient  client = server.available();
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
            }              
            if (pword=="jebg" && stove=="off"){
              setServo(155);
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

void connectWiFi()
{
  byte ledStatus = LOW;
  Serial.println();
  Serial.println("Connecting to: " + String(WiFiSSID));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
