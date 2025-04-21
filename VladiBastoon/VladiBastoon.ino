#include "WiFi.h"
#include "PubSubClient.h"
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "Naiki";
const char* password = "fortinayt";

// MQTT Broker details
const char* mqtt_server = "192.168.236.174";
const int mqtt_port = 1883;
const char* mqtt_topic = "esp32cam/image";  // Topic to publish the image
const char* confirmation_topic = "esp32cam/confirmation"; // Topic to send confirmation

WiFiClient espClient;
PubSubClient client(espClient);

// Camera pin configuration for AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup() {
  Serial.begin(115200);

  connectWifi();
  client.setServer(mqtt_server, mqtt_port);

  client.setBufferSize(4096);

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // Reduce resolution to QVGA (320x240)
  config.jpeg_quality = 40;           // Increase the quality value (less quality, smaller size)
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    delay(1000);
    ESP.restart();
  }
  Serial.println("Camera initialized successfully");

  // Send one byte confirmation message to MQTT
  sendConfirmation();
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  sendImageMQTT();
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
    
    if (client.connect("ESP32CamClient")) {
      Serial.println("connected");
      sendConfirmation();  // Send confirmation message once connected
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

void sendConfirmation() {
  // Send a single byte of data to confirm connection
  byte confirmationData = 1;  // Sending a byte with value '1' for confirmation
  boolean success = client.publish(confirmation_topic, &confirmationData, 1);
  
  if (success) {
    Serial.println("Confirmation sent successfully");
  } else {
    Serial.println("Failed to send confirmation");
  }
}

void sendImageMQTT() {
  camera_fb_t* fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  Serial.printf("Captured image of size: %u bytes\n", fb->len);

  // Publish image to MQTT
  boolean success = client.publish(mqtt_topic, fb->buf, fb->len);
  
  if (success) {
    Serial.println("Image published successfully");
  } else {
    Serial.println("Failed to publish image");
  }

  // Return frame buffer back to driver
  esp_camera_fb_return(fb);
}
