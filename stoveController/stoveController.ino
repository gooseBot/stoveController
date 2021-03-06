// Include the ESP8266 WiFi library. (Works a lot like the Arduino WiFi library.)
#include <ESP8266WiFi.h>
#include <Servo.h> 
#define maxLength 30

const char WiFiSSID[] = "34er54";
const char WiFiPSK[] = "jeep~fish*63";

const int LED_PIN = 5; // Thing's, green LED
const int buttonPin = 12; // Digital pin to be read
const int servoPin = 2;  //digital pin used to control the servo

int stoveOn = 15;
int stoveOff = 155;

byte ip[] = { 192,168,1, 177 };
String responseString = String(maxLength);

Servo myServo;  

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP); // Set pin 12 as an input w/ pull-up

  connectWiFi();
 
  server.begin();
  
  setStove(false);
}

void loop()
{
  //look for button push and toggle state of stove
  if (digitalRead(buttonPin) == LOW){
    if (myServo.read() > 100) {
      setStove(true);
    }
    else {
      setStove(false);
    }
    delay(400);
  }

  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1; 
  if (req.indexOf("/stove/0") != -1) {
    val = 0; // Will write stove off
    setStove(false);
  }
  else if (req.indexOf("/stove/1") != -1) {
    val = 1; // Will write stove on
    setStove(true);
  }                  
  else if (req.indexOf("/stove/status") != -1) {
    myServo.attach(servoPin);
    delay(50);
    int servoVal = myServo.read();                
    myServo.detach();
    if (servoVal <= (stoveOn + 5))
      val = 1;
    if (servoVal >= (stoveOff - 5))
      val = 0;
  } 
  client.flush();

  // Prepare the response. Start with the common header:
  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  // If we're setting the stove, print out a message saying we did
  if (val >= 0)
  {
    s += (val) ? "on" : "off";
  }
  else
  {
    s += "Invalid Request";
  }
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
}

void setStove(bool turnStoveOn)
{
  myServo.attach(servoPin);
  delay(50);
  if (turnStoveOn)
    myServo.write(stoveOn);
  else
    myServo.write(stoveOff);
  delay(500);
  myServo.detach();
}

//void turnStoveOff()
//{
//  myServo.attach(servoPin);
//  delay(50);
//  myServo.write(stoveOff);                  
//  delay(500);
//  myServo.detach();
//}
//
//void turnStoveOn() 
//{
//  myServo.attach(servoPin);
//  delay(50);
//  myServo.write(stoveOn);                  
//  delay(500);
//  myServo.detach();
//}

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
  // try to use a static address that I'm using with the router with ddns
  WiFi.config(ip,WiFi.gatewayIP(), WiFi.subnetMask());

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
