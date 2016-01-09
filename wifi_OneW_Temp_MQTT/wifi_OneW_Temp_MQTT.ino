#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <PubSubClient.h>

// MQTT Parameters
#define mqttUser "admin"
#define mqttPassword "redborder"
#define mqttClientID  "ivanlabArduinoClient"
#define mqttTopic "rBtemp"

// WiFi Parameters
char ssid[] = "ivanlab";           // your network SSID (name) 
char pass[] = "ComputoErgoSum";    // your network password
int status = WL_IDLE_STATUS;       // the Wifi radio's status
IPAddress ip;                      // the IP address of your WiFi shield
IPAddress server(212, 72, 74, 21); // Adress of the MQTT server

// Bus parameters
#define ONE_WIRE_BUS 2   // Data wire is plugged into port 2 on the Arduino

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Sensor Parameters
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

//Temperature in character format for publishing
char charTemperature [20];

//function to handle incoming MQTT messeges
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  //serial.println("kk");
}

// MQTT client provisioning
 WiFiClient WiClient;
 PubSubClient client(server, 1883, callback, WiClient);

// function to publish the temperature in a MQTT topic
void publishTemperature (DeviceAddress deviceAddress) {

  float tempC = sensors.getTempC(deviceAddress);
  dtostrf(sensors.getTempC(deviceAddress),5,2,charTemperature) ;
  
  if (client.connect(mqttClientID, mqttUser, mqttPassword)) {
    client.publish(mqttTopic,charTemperature);
    // at the same time we can subscribe to get parameters from inTopic here:
    // client.subscribe("inTopic");
  }
  else {
    Serial.println("Cannot reach MQTT broker");
  }
} 

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress) {
  // method 2 - faster
  float tempC = sensors.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
}

// function to print a OneWire device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void setup() {
  
  // initialize serial:
  Serial.begin(9600);

  // Initialize WiFi
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
  
  // Connect to the MQTT broker


  // Initialize OneWire bus
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  
  // Assign address manually. The addresses below will beed to be changed
  // to valid device addresses on your bus. Device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  // Note that you will need to use your specific address here
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };

  // Method 1:
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then 
  // use those addresses and manually assign them (see above) once you know 
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  
  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices, 
  // or you have already retrieved all of them. It might be a good idea to 
  // check the CRC to make sure you didn't get garbage. The order is 
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");

  // show the addresses we found on the bus
  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}

void loop(void)
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  // It responds almost immediately. Let's print out the data
  printTemperature(insideThermometer); // Use a simple function to print out the data

  // Write temperature to MQTT queue
  publishTemperature(insideThermometer);

  delay(60000);
}



