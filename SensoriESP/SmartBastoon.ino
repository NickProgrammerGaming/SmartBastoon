#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// WiFi & MQTT
const char* ssid = "Naiki";
const char* password = "fortinayt";
const char* mqtt_server = "raspberrypi.local";
const int mqtt_port = 1883;
const char* distance_topic = "esp32/sensordata";
const char* emergency_topic = "esp32/emergency";

WiFiClient espClient;
PubSubClient client(espClient);

// Pins
#define TRIG1 12
#define ECHO1 14
#define TRIG2 27
#define ECHO2 26
#define TRIG3 25
#define ECHO3 33
#define TRIG4 15
#define ECHO4 2
#define BUZZER 32
#define MOTOR 19
#define LED_INDICATOR 13
#define DIST_THRESHOLD 20

// MPU6050
#define MPU_ADDR 0x68
#define ACCEL_X_REG 0x3B
int az_baseline = 0;
const int fall_sensitivity = 6000;

// Global state
long dist1 = 0, dist2 = 0, dist3 = 0, dist4 = 0;
bool caneFallen = false;
unsigned long fallTime = 0;
const unsigned long fallThreshold = 30000;

// === Function declarations ===
void readAccelData(int& ax, int& ay, int& az);
long readDistance(int trig, int echo);
void activateAlert();
void deactivateAlert();
void tryConnectWiFi();
void reconnectMQTT();
void sendSensorData();

// === MPU ===
void initMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  delay(100);
  int ax, ay, az;
  readAccelData(ax, ay, az);
  az_baseline = az;
  Serial.printf("Calibrated Z-axis: %d\n", az_baseline);
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

// === Distance & Alert ===
long readDistance(int trig, int echo) {
  digitalWrite(trig, LOW); delayMicroseconds(2);
  digitalWrite(trig, HIGH); delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  return (duration / 2) / 29.1;
}

void activateAlert() {
  tone(BUZZER, 2000);  // Increased frequency for louder tone  // 1kHz tone for passive buzzer
  digitalWrite(MOTOR, HIGH);
  digitalWrite(LED_INDICATOR, HIGH);
}

void deactivateAlert() {
  noTone(BUZZER);
  digitalWrite(MOTOR, LOW);
  digitalWrite(LED_INDICATOR, LOW);
}

// === WiFi ===
void tryConnectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print(".");
  }
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}
 
// === MQTT ===
void reconnectMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32SensorClient")) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("MQTT reconnect failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
} 

void sendSensorData() {
  String payload = "{";
  payload += "\"sensor1\":" + String(dist1) + ",";
  payload += "\"sensor2\":" + String(dist2) + ",";
  payload += "\"sensor3\":" + String(dist3) + ",";
  payload += "\"sensor4\":" + String(dist4);
  payload += "}";
  client.publish(distance_topic, payload.c_str());
}

// === Sensor Task ===
void sensorLoop(void* pv) {
  while (true) {
    dist1 = readDistance(TRIG1, ECHO1);
    dist2 = readDistance(TRIG2, ECHO2);
    dist3 = readDistance(TRIG3, ECHO3);
    dist4 = readDistance(TRIG4, ECHO4);

    if ((dist1 > 0 && dist1 < DIST_THRESHOLD) || (dist2 > 0 && dist2 < DIST_THRESHOLD) ||
        (dist3 > 0 && dist3 < DIST_THRESHOLD) || (dist4 > 0 && dist4 < DIST_THRESHOLD)) {
      activateAlert();
    } else {
      deactivateAlert();
    }

    int ax, ay, az;
    readAccelData(ax, ay, az);
    if (abs(az - az_baseline) > fall_sensitivity) {
      if (!caneFallen) {
        caneFallen = true;
        fallTime = millis();
      }
    } else {
      caneFallen = false;
    }

    delay(50);
  }
}

void commsLoop(void* pv) {
  
  tryConnectWiFi();
  reconnectMQTT();
  
  while (true) { 
    client.setBufferSize(5120);
    client.loop();

    if (client.connected()) {
      sendSensorData();

      if (caneFallen && (millis() - fallTime > fallThreshold)) {
        client.publish(emergency_topic, "Cane has fallen for over 30s");
      }
    }
    delay(200);
  }
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  Wire.begin();
  initMPU();

  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);
  pinMode(TRIG4, OUTPUT); pinMode(ECHO4, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(LED_INDICATOR, OUTPUT);

  xTaskCreatePinnedToCore(sensorLoop, "Sensors", 8000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(commsLoop, "MQTT+WiFi", 10000, NULL, 1, NULL, 0);
}

void loop() {
  // Nothing here
}
