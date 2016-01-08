#include <WiFi.h>

char ssid[] = "ivanlab";     //  your network SSID (name) 
char pass[] = "ComputoErgoSum";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
IPAddress ip;                    // the IP address of your shield


void setup() {
  // initialize serial:
  Serial.begin(9600);

  // attempt to connect using WPA2 encryption:
  Serial.println("Attempting to connect to WPA network...");
  status = WiFi.begin(ssid, pass);

  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 
  // if you are connected, print out info about the connection:
  else {
    Serial.println("Connected to network");
    //print the local IP address
  ip = WiFi.localIP();
  Serial.println(ip);

  }

}

void loop() {
  // do nothing
}
 
