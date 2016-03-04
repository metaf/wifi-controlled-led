#include <ESP8266WiFi.h>
#include <Wire.h>

#pragma GCC diagnostic ignored "-Wwrite-strings"

#define SDA     0                          // ESP8266-01
#define SCL     2                          // ESP8266-01
#define LED_PIN 5                          // on Sparkfun Thing board

const char WiFiAPPSK[] = "asdf";       // Note... need to change
const int  ANALOG_PIN  = A0;               // The only analog pin on the Thing
const int  DIGITAL_PIN = 12;               // Digital pin to be read
boolean    Lflag       = true;

WiFiServer server(80);                     // create object


void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Append the last two bytes of the MAC (HEX'd) to string to make unique
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "WIFI_LED";   // + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, AP_NameString.length() + 1, 0);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar);//, WiFiAPPSK);
}


void setup() 
{
  // Serial.begin(115200);                    // diagnostic channel (RUN env)
  Serial.begin(74880);                     // native bootloader channel (DEV Only)
  delay(10);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  Serial.println("About to setup wifi");
  setupWiFi();
  Serial.println("Just setup wifi");
  Serial.println("about to start server");
  server.begin();
  Serial.println("server started");

}


void loop() 
{
  WiFiClient client = server.available();  // Check if a client has connected
  if (!client)
  {
    if (Lflag)
    {
      Serial.println(F("Waiting for a client to connect to this server..."));
      Serial.println(F("On address 192.168.4.1"));
      delay(200);
      Lflag = false;
    }
      return;
  }

  String req = client.readStringUntil('\r');// Read the first line of the request
  Serial.println(req);
  client.flush();

  // Match the request
  int val = -1;                            // 'val' to keep track of both the
                                           // request type (read/set) and value if set.
  if (req.indexOf("/led/off") != -1)
    val = 1; // Will write LED low
  else if (req.indexOf("/led/on") != -1)
    val = 0; // Will write LED high
  else if (req.indexOf("/read") != -1)
    val = -2;                              // Will print pin reads
                                           // Otherwise request is invalidt
  if (val >= 0)                            // Set GPIO5 according to input
    digitalWrite(LED_PIN, val);

  client.flush();

  String s = "HTTP/1.1 200 OK\r\n";        // the common header:
  s += "Content-Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";

  if (val >= 0)                            // Setting LED?
  {
    s += "LED is now ";
    s += (val)?"off":"on";
    //temp[0] = (char) 0;
    String temporary = (val)?"LED on":"LED off";
  } else if (val == -2)
  {
    s += "Analog Pin = ";                  // Reading pins?, print values:
    s += String(analogRead(ANALOG_PIN));
    String Stemp = String(analogRead(ANALOG_PIN));
    s += "<br>";                           // Go to the next line.
    s += "Digital Pin 12 = ";
    s += String(digitalRead(DIGITAL_PIN));
  } else
  {
    s += "Invalid Request.<br> Try /led/1, /led/0, or /read.";
  }

  s += "</html>\n";

  client.print(s);                         // Send the response to the client
  delay(1);
  Serial.println(F("Client disonnected"));

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

