/*
 * Connects to WiFi and subscribes to MQTT topic.
 * 
 * Assumes that the switches are of Type B, with id of Group-Switch ([1-4](I-IV)-[1-4])
 * This would be for instance the Luxorpart series that you can buy at Kjell & co
 * When ON/OFF commands are sent for topic, then turn switch on/off  
 * 
 * Example config from Home Assistant configuration.yaml: * 
# Turn on Mosquitto
mqtt:

switch:
  - platform: mqtt
    name: "Utomhusbelysning"
    command_topic: "node/luxorparts/utomhus/set"
 * 
 */
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = "....";				  // WiFi SSID
const char* password = "....";			  // WiFi password
const char* mqtt_server = "192.168.x.x";  // IP address to mqtt server
const int ledPin = BUILTIN_LED;
const int txPin = 15;                      // GPIO pin for RF 433 tx device
const int groupNumber = 1;                 // 1-4
const int switchNumber = 1;                // 1-4
const char* subscriptionTopic = "node/luxorparts/utomhus/set";

void setup() {
  Serial.begin(9600);
  mySwitch.enableTransmit(txPin);  
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();
}

void setup_wifi() {
  delay(10);
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char receivedStr[length+1];
  receivedStr[length] = '\0';
  memcpy(receivedStr, payload, length);

  Serial.print(receivedStr);

  if (strcmp(receivedStr, "ON") == 0) {
    digitalWrite(ledPin, LOW);
    mySwitch.switchOn(groupNumber, switchNumber);
  }
  if (strcmp(receivedStr, "OFF") == 0) {
    digitalWrite(ledPin, HIGH);
    mySwitch.switchOff(groupNumber, switchNumber);
  }

  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("wemosClient")) {
      Serial.println("connected");
      client.subscribe(subscriptionTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
