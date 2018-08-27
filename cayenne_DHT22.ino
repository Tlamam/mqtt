// This example shows how to connect to Cayenne using a manually specified Ethernet connection and send/receive sample data.

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneMQTTESP8266.h>
#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);
// Your network name and password.
char ssid[] = "UPCFFB3577";
char wifiPassword[] = "wmkcsyApwt5x";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "a20a6ed0-e53c-11e6-a446-0d180dc59d42";
char password[] = "a4644b018f8f2c854ade28f434d8885ddcf39733";
char clientID[] = "4993ef20-aa1f-11e8-b20b-f17b9147793f";


unsigned long lastMillis = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
}

void loop() {
   delay(2000);
   //Read the light value
   int lightValue = analogRead(A0)/10.24;
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  Cayenne.loop();

  //Publish data every 10 seconds (10000 milliseconds). Change this value to publish at a different interval.
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    //Write data to Cayenne here. This example just sends the current uptime in milliseconds.
    Cayenne.virtualWrite(0, h);
    //Some examples of other functions you can use to send data.
    Cayenne.celsiusWrite(1, t);
    Cayenne.luxWrite(2, lightValue);
    //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
    Serial.println("Going into deep sleep for 20 seconds");
    ESP.deepSleep(20e6); // 20e6 is 20 microseconds
  }
}

//Default function for processing actuator commands from the Cayenne Dashboard.
//You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT()
{
  CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}

