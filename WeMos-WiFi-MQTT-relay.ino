#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// i'm loading settings from this file
#include "credentials.h"

// but you can set them here instead
//const char* ssid = "";
//const char* password = "";
//const char* mqtt_server = "";
//#define CLIENT_ID "qm9"

#define RELAY_PIN D1

const char* willTopic = "$CONNECTED/"CLIENT_ID;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
float temp = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); //We don't want the ESP to act as an AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Toggle relay and LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(RELAY_PIN, HIGH);
    client.publish(CLIENT_ID"/relay", "1", true);
    delay(3000);
    digitalWrite(RELAY_PIN, LOW);
    client.publish(CLIENT_ID"/relay", "0", true);
  }
  else
  {
    Serial.print("Received non 1. Therefore I won't do anything right now.");
    Serial.println();
  }
}

void reconnect() {
  // Loop until we're reconnected
  digitalWrite(LED_BUILTIN, LOW);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CLIENT_ID, mqtt_username, mqtt_password ,willTopic, 0, true, "0")) {
      Serial.println("connected");
      client.publish(willTopic, "1", true);
      client.subscribe(CLIENT_ID"/relay/set");
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
