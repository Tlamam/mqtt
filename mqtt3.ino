
/*
  It connects to an MQTT server then:
  - on 0 switches off relay
  - on 1 switches on relay
  - on 2 switches the state of the relay
  - sends 0 on off relay
  - sends 1 on on relay
  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.
  The current state is stored in EEPROM and restored on bootup
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <NewPing.h>

#define TRIGGER_PIN  13  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     12  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 2000 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
const char* ssid = "Internet";
const char* password = "h3slo123";
const char* mqtt_server = "90.177.115.75";
const long interval = 1000;
unsigned long previousMillis;
const int pinCidlaDS = 0;
OneWire oneWireDS(pinCidlaDS);
const int pinCidlaVL = 4;
// vytvoření instance senzoryDS z knihovny DallasTemperature
DallasTemperature senzoryDS(&oneWireDS);
  
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const char* outTopic = "/LivingRoom/LED/1";
const char* inTopic = "/LivingRoom/LED/1";
const char* outTopic1 = "/LivingRoom/light";
const char* outTopic2 = "/LivingRoom/temp";
const char* outTopic3 = "/LivingRoom/dept";
const char* outTopic4 = "/LivingRoom/dept1";
const char* outTopic5 = "/LivingRoom/moisure";
int relay_pin = 15;
int button_pin = 5;
bool relayState = LOW;

// Instantiate a Bounce object :
Bounce debouncer = Bounce();


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    extButton();
    for (int i = 0; i < 500; i++) {
      extButton();
      delay(1);
    }
    Serial.print(".");
  }
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);
  digitalWrite(13, LOW);
  delay(500);
  digitalWrite(13, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '0') {
    digitalWrite(relay_pin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    Serial.println("relay_pin -> LOW");
    relayState = LOW;
    EEPROM.write(0, relayState);    // Write state to EEPROM
    EEPROM.commit();
  } else if ((char)payload[0] == '1') {
    digitalWrite(relay_pin, HIGH);  // Turn the LED off by making the voltage HIGH
    Serial.println("relay_pin -> HIGH");
    relayState = HIGH;
    EEPROM.write(0, relayState);    // Write state to EEPROM
    EEPROM.commit();
  } else if ((char)payload[0] == '2') {
    relayState = !relayState;
    digitalWrite(relay_pin, relayState);  // Turn the LED off by making the voltage HIGH
    Serial.print("relay_pin -> switched to ");
    Serial.println(relayState);
    EEPROM.write(0, relayState);    // Write state to EEPROM
    EEPROM.commit();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "Sonoff1 booted");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      for (int i = 0; i < 5000; i++) {
        extButton();
        delay(1);
      }
    }
  }
}

void extButton() {
  debouncer.update();

  // Call code if Bounce fell (transition from HIGH to LOW) :
  if ( debouncer.fell() ) {
    Serial.println("Debouncer fell");
    // Toggle relay state :
    relayState = !relayState;
    digitalWrite(relay_pin, relayState);
    EEPROM.write(0, relayState);    // Write state to EEPROM
    if (relayState == 1) {
      client.publish(outTopic, "1");
    }
    else if (relayState == 0) {
      client.publish(outTopic, "0");
    }
  }
}

void writeLight() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.2V):
    int light = sensorValue;
    // print out the value you read:
    light = map (light, 0, 1023, 100, 0); // scaled from 100 - 0 (lower is darker)
    Serial.print("light: ");    
    Serial.println(light);
    client.publish(outTopic1, String(light).c_str(), true);
    senzoryDS.requestTemperatures();
    int temp = senzoryDS.getTempCByIndex(0);
    Serial.print("temp: ");    
    Serial.println(temp);
    client.publish(outTopic2, String(temp).c_str(), true);
    int dept = sonar.ping_cm();
    Serial.print("dept: ");    
    Serial.println(dept);
    client.publish(outTopic3, String(dept).c_str(), true);
    int dept1 = dept;
    dept1 = map (dept1, 0, 200, 100, 0); // scaled from 100 - 0 (lower is darker)
    Serial.print("dept1: ");    
    Serial.println(dept1);
    client.publish(outTopic4, String(dept1).c_str(), true);
    
    int adc_key_in = analogRead(pinCidlaVL);
    adc_key_in = map(adc_key_in, 4095, 0, 0, 100);
    Serial.print("Mositure : ");
    Serial.println("%");
    Serial.println(adc_key_in);
    client.publish(outTopic5, String(0).c_str(), true);
  }
}

void setup() {
  EEPROM.begin(512);              // Begin eeprom to store on/off state
  pinMode(relay_pin, OUTPUT);     // Initialize the relay pin as an output
  pinMode(button_pin, INPUT);     // Initialize the relay pin as an output
  pinMode(13, OUTPUT);
  relayState = EEPROM.read(0);
  digitalWrite(relay_pin, relayState);

  debouncer.attach(button_pin);   // Use the bounce2 library to debounce the built in button
  debouncer.interval(50);         // Input must be low for 50 ms

  digitalWrite(13, LOW);          // Blink to indicate setup
  delay(500);
  digitalWrite(13, HIGH);
  delay(500);

  Serial.begin(115200);
  setup_wifi();                   // Connect to wifi
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback);
  senzoryDS.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  extButton();
  writeLight();
}
