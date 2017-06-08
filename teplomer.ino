#include <OneWire.h>
#include <DallasTemperature.h>
#include "CayenneDefines.h"
#include "CayenneWiFi.h"
#include "CayenneWiFiClient.h"
#define CAYANNE_DEBUG
#define CAYANNE_PRINT Serial
#define VIRTUAL_PIN 1
#define RELAY_DIGITAL_PIN 15
// constants won't change. Used here to set a pin number :
char token[] = "g27jdklxzz";
// Your network name and password.
char ssid[] = "adsl";
char password[] = "ITGeeks123";

const int ledPin =  15;// the number of the LED pin
const int analogPin = 0;
// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
int val = 0; 
// constants won't change :
const long interval = 1000;           // interval at which to blink (milliseconds)
// nastavení čísla vstupního pinu
const int pinCidlaDS = 0;
// vytvoření instance oneWireDS z knihovny OneWire
OneWire oneWireDS(pinCidlaDS);
// vytvoření instance senzoryDS z knihovny DallasTemperature
DallasTemperature senzoryDS(&oneWireDS);

void setup(void) {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600);
  // zapnutí komunikace knihovny s teplotním čidlem
  senzoryDS.begin();
    // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  Cayenne.begin(token, ssid, password);
}
CAYENNE_IN(VIRTUAL_PIN)
{
  // get value sent from dashboard
  int currentValue = getValue.asInt(); // 0 to 1

  // assuming you wire your relay as normally open
  if (currentValue == 0) {
    digitalWrite(RELAY_DIGITAL_PIN, HIGH);
  } else {
    digitalWrite(RELAY_DIGITAL_PIN, LOW);
  }
}
void loop(void) {
  Cayenne.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  // načtení informací ze všech připojených čidel na daném pinu
  senzoryDS.requestTemperatures();
  // výpis teploty na sériovou linku, při připojení více čidel
  // na jeden pin můžeme postupně načíst všechny teploty
  // pomocí změny čísla v závorce (0) - pořadí dle unikátní adresy čidel
  Serial.print("Teplota cidla DS18B20: ");
  Serial.print(senzoryDS.getTempCByIndex(0));
  Serial.println(" stupnu Celsia");
  // pauza pro přehlednější výpis
  unsigned long currentMillis = millis();
  val = analogRead(analogPin);
  Serial.println(val);   
  }
}
