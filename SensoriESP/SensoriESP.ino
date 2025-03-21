#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

const char* ssid = "Naiki";
const char* password = "fortinayt";
const char* mqtt_server = "192.168.236.174";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32/sensordata";
const char* response_topic = "esp32/response";
const char* emergency_topic = "esp32/emergency";

WiFiClient espClient;
PubSubClient client(espClient);

#define TRIG_PIN1 12
#define ECHO_PIN1 14
#define TRIG_PIN2 27
#define ECHO_PIN2 26
#define TRIG_PIN3 25
#define ECHO_PIN3 33
#define BUZZER_PIN 32
#define MOTOR 19
#define DISTANCE_THRESHOLD 20

#define MPU_ADDR 0x68
#define ACCEL_X_REG 0x3B
#define ACCEL_Y_REG 0x3D
#define ACCEL_Z_REG 0x3F

bool caneFallen = false;
unsigned long fallTime = 0;
const unsigned long fallThreshold = 30000;

void setup() {
  Serial.begin(115200);
  connectWifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);
  pinMode(TRIG_PIN3, OUTPUT);
  pinMode(ECHO_PIN3, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);


  Wire.begin();
  initMPU();

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

  long distance1 = readDistance(TRIG_PIN1, ECHO_PIN1);
  long distance2 = readDistance(TRIG_PIN2, ECHO_PIN2);
  long distance3 = readDistance(TRIG_PIN3, ECHO_PIN3);

  Serial.printf("Distances: %ld cm, %ld cm, %ld cm\n", distance1, distance2, distance3);
  sendDistanceData(distance1, distance2, distance3);

  if (distance1 < DISTANCE_THRESHOLD || distance2 < DISTANCE_THRESHOLD || distance3 < DISTANCE_THRESHOLD) {
    activateBuzzer();
  } else {
    deactivateBuzzer();
  }

  checkCaneFall();

  if (caneFallen && (millis() - fallTime > fallThreshold)) {
    sendEmergencyAlert();
  }

  delay(2000);
}

void connectWifi() {
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
    if (client.connect("ESP32UltrasonicClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  
  long distance = (duration / 2) / 29.1;
  return distance;
}

void sendDistanceData(long distance1, long distance2, long distance3) {
  String payload = "{";
  payload += "\"sensor1\": " + String(distance1) + ",";
  payload += "\"sensor2\": " + String(distance2) + ",";
  payload += "\"sensor3\": " + String(distance3);
  payload += "}";

  if (client.publish(mqtt_topic, payload.c_str())) {
    Serial.println("Distance data sent to MQTT server");
  } else {
    Serial.println("Failed to send data to MQTT server");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received on topic '");
  Serial.print(topic);
  Serial.print("': ");
  Serial.println(message);

  if (message.indexOf("pothole") != -1) {
    Serial.println("Pothole detected, activating buzzer...");
    activateBuzzer();
  }
}

void initMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
}

void readAccelData(int& ax, int& ay, int& az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_X_REG);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = (Wire.read() << 8) | Wire.read();
  ay = (Wire.read() << 8) | Wire.read();
  az = (Wire.read() << 8) | Wire.read();
}

void checkCaneFall() {
  int ax, ay, az;
  readAccelData(ax, ay, az);

  Serial.printf("Ax: %d, Ay: %d, Az: %d\n", ax, ay, az);

  if (abs(az) > 16000) {
    if (!caneFallen) {
      caneFallen = true;
      fallTime = millis();
      Serial.println("Cane has fallen.");
    }
  } else {
    caneFallen = false;
    Serial.println("Cane is upright.");
  }
}

void sendEmergencyAlert() {
  String alertMessage = "Cane has been on the ground for more than 30 seconds!";
  if (client.publish(emergency_topic, alertMessage.c_str())) {
    Serial.println("Emergency alert sent to MQTT server");
  } else {
    Serial.println("Failed to send emergency alert");
  }
}

void activateBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  activateMotor();
  Serial.println("Buzzer Activated!");
}

void deactivateBuzzer() {
  digitalWrite(BUZZER_PIN, LOW);
  deactivateMotor();
  Serial.println("Buzzer Deactivated!");
}

void activateMotor() {
  digitalWrite(MOTOR, HIGH);
  Serial.println("Vibration Motor Activated!");
}

void deactivateMotor() {
  digitalWrite(MOTOR, LOW);
  Serial.println("Vibration Motor Deactivated!");
}

