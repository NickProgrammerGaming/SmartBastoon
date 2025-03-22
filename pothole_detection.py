import paho.mqtt.client as mqtt
import numpy as np
import cv2
import io
import os

mqtt_broker = "localhost"
mqtt_port = 1883
mqtt_topic = "esp32cam/image"
mqtt_response_topic = "esp32cam/response"

image_save_path = "/home/redpassion/Bastoon/"
if not os.path.exists(image_save_path):
    os.makedirs(image_save_path)

def on_message(client, userdata, msg):
    print("Image received on topic: " + msg.topic)
    nparr = np.frombuffer(msg.payload, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    save_image(img)
    processed_image = pothole_detection(img)
    if processed_image is not None:
        print("Pothole detected, sending response to ESP32-CAM")
        client.publish(mqtt_response_topic, "Pothole Detected!")
    else:
        print("No pothole detected")

def save_image(image):
    image_filename = os.path.join(image_save_path, "received_image.jpg")
    cv2.imwrite(image_filename, image)
    print(f"Image saved as {image_filename}")

def pothole_detection(image):
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, thresh = cv2.threshold(blurred, 100, 255, cv2.THRESH_BINARY)
    edges = cv2.Canny(thresh, 100, 200)
    contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    pothole_contours = [cnt for cnt in contours if cv2.contourArea(cnt) > 500]
    if pothole_contours:
        cv2.drawContours(image, pothole_contours, -1, (0, 255, 0), 3)
        return image
    return None

client = mqtt.Client()
client.on_message = on_message
client.connect(mqtt_broker, mqtt_port, 60)
client.subscribe(mqtt_topic)
print("Waiting for image...")
client.loop_forever()
