#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <PubSubClient.h>

// MQTT Parameters
#define mqttUser "admin"
#define mqttPassword "redborder"
#define mqttClientID  "ivanlabArduinoClient"
#define mqttTopic "rb_iot"

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte ip[]     = { 10, 0, 30, 131 };
IPAddress server (10, 0, 150, 38); // Adress of the MQTT server

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

void setup() {
  
  // initialize serial:
  Serial.begin(9600);
  Ethernet.begin(mac, ip);


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
  

  // Method 1:
  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then 
  // use those addresses and manually assign them (see above) once you know 
  // the devices on your bus (and assuming they don't change).
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 

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

void loop(void) { 
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

//function to handle incoming MQTT messeges
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  //serial.println("kk");
}

// MQTT client provisioning
 EthernetClient ethClient;
 PubSubClient client(server, 1883, callback, ethClient);

// function to publish the temperature in a MQTT topic
void publishTemperature (DeviceAddress deviceAddress) {

  float tempC = sensors.getTempC(deviceAddress);
  dtostrf(sensors.getTempC(deviceAddress),5,2,charTemperature) ;
  char jsonOut[100];
  sprintf( jsonOut, "{\"iot_id\": \"Sensor_Ivanlab_1\", \"value\": \"%s\", \"monitor\": \"temperature\"}", charTemperature );
  
  if (client.connect(mqttClientID)) {
    client.publish(mqttTopic,jsonOut);
    Serial.println("Published to MQTT broker");
    Serial.println(jsonOut);
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




