#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Naiki";
const char* password = "fortinayt";

// MQTT Broker details
const char* mqtt_server = "192.168.236.174";  // Replace with your MQTT server IP
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/sensordata"; // Topic for distance data
const char* response_topic = "esp32/response"; // Topic to monitor for pothole response

WiFiClient espClient;
PubSubClient client(espClient);

// Ultrasonic sensor pins
#define TRIG_PIN1 12
#define ECHO_PIN1 14
#define TRIG_PIN2 27
#define ECHO_PIN2 26
#define TRIG_PIN3 25
#define ECHO_PIN3 33

// Buzzer pin
#define BUZZER_PIN 32

// Distance threshold for obstacle detection (in centimeters)
#define DISTANCE_THRESHOLD 20

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  connectWifi();
  
  // Setup MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback); // Set the callback function to handle incoming messages

  // Set ultrasonic sensor pins as output and input
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(TRIG_PIN3, OUTPUT);
  pinMode(ECHO_PIN3, INPUT);
  
  // Set buzzer pin as output
  pinMode(BUZZER_PIN, OUTPUT);

  // Subscribe to the response topic
  if (client.subscribe(response_topic)) {
    Serial.println("Successfully subscribed to 'esp32/response'");
  } else {
    Serial.println("Failed to subscribe to 'esp32/response'");
  }
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Read the distances from the sensors
  long distance1 = readDistance(TRIG_PIN1, ECHO_PIN1);
  long distance2 = readDistance(TRIG_PIN2, ECHO_PIN2);
  long distance3 = readDistance(TRIG_PIN3, ECHO_PIN3);

  // Print the distances to the Serial Monitor
  Serial.printf("Distances: %ld cm, %ld cm, %ld cm\n", distance1, distance2, distance3);

  // Send the distance data to the MQTT server
  sendDistanceData(distance1, distance2, distance3);

  // If any sensor detects an obstacle within the threshold, activate the buzzer
  if (distance1 < DISTANCE_THRESHOLD || distance2 < DISTANCE_THRESHOLD || distance3 < DISTANCE_THRESHOLD) {
    activateBuzzer();
  } else {
    deactivateBuzzer();
  }

  delay(2000);  // Delay for 2 seconds before the next reading
}

void connectWifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32UltrasonicClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

long readDistance(int trigPin, int echoPin) {
  // Trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure duration
  long duration = pulseIn(echoPin, HIGH);
  
  // Calculate distance
  long distance = (duration / 2) / 29.1; // Distance in cm
  return distance;
}

void sendDistanceData(long distance1, long distance2, long distance3) {
  // Create JSON-like data (you could also use a library like ArduinoJson)
  String payload = "{";
  payload += "\"sensor1\": " + String(distance1) + ",";
  payload += "\"sensor2\": " + String(distance2) + ",";
  payload += "\"sensor3\": " + String(distance3);
  payload += "}";

  // Publish the distance data to MQTT topic
  if (client.publish(mqtt_topic, payload.c_str())) {
    Serial.println("Distance data sent to MQTT server");
  } else {
    Serial.println("Failed to send data to MQTT server");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload to a string
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Check if the response indicates a pothole
  Serial.print("Message received on topic '");
  Serial.print(topic);
  Serial.print("': ");
  Serial.println(message);

  if (message.indexOf("pothole") != -1) {  // Check if "pothole" is in the message
    Serial.println("Pothole detected, activating buzzer...");
    activateBuzzer();
  }
}

void activateBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);  // Activate buzzer
  Serial.println("Buzzer Activated!");
}

void deactivateBuzzer() {
  digitalWrite(BUZZER_PIN, LOW);  // Deactivate buzzer
  Serial.println("Buzzer Deactivated!");
}

