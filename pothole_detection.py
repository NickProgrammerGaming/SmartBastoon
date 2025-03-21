import paho.mqtt.client as mqtt
import numpy as np
import cv2
import io
import os

# MQTT configuration
mqtt_broker = "localhost"  # IP address of Raspberry Pi
mqtt_port = 1883
mqtt_topic = "esp32cam/image"  # Topic where the image is published
mqtt_response_topic = "esp32cam/response"  # Topic to send response to ESP32-CAM

# Path where images will be saved
image_save_path = "/home/redpassion/Bastoon/"
if not os.path.exists(image_save_path):
    os.makedirs(image_save_path)

# Callback function when message is received
def on_message(client, userdata, msg):
    print("Image received on topic: " + msg.topic)
    
    # Convert byte payload to image
    nparr = np.frombuffer(msg.payload, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
    
    # Save the image to a file
    save_image(img)
    
    # Process the image to detect potholes (improved method)
    processed_image = pothole_detection(img)
    
    # If pothole is detected, send a response back to the ESP32-CAM
    if processed_image is not None:
        print("Pothole detected, sending response to ESP32-CAM")
        client.publish(mqtt_response_topic, "Pothole Detected!")
    else:
        print("No pothole detected")

# Save image to file
def save_image(image):
    # Generate a filename based on the current time or a counter
    image_filename = os.path.join(image_save_path, "received_image.jpg")
    
    # Save the image using OpenCV
    cv2.imwrite(image_filename, image)
    print(f"Image saved as {image_filename}")

# Improved pothole detection function using preprocessing and contour filtering
def pothole_detection(image):
    # Convert to grayscale
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Apply Gaussian blur to reduce noise and smooth the image
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    
    # Apply adaptive thresholding to get a binary image (enhances features)
    _, thresh = cv2.threshold(blurred, 100, 255, cv2.THRESH_BINARY)
    
    # Apply edge detection (Canny edge detector)
    edges = cv2.Canny(thresh, 100, 200)

    # Find contours in the edge-detected image
    contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    # Filter contours by area: Only keep large enough contours (likely potholes)
    pothole_contours = [cnt for cnt in contours if cv2.contourArea(cnt) > 500]

    # If any large enough contours are detected, assume it's a pothole
    if pothole_contours:
        # Optionally, draw contours on the image for visualization
        cv2.drawContours(image, pothole_contours, -1, (0, 255, 0), 3)  # Green color for contour
        return image  # Return the processed image
    return None  # No pothole detected

# Setup MQTT client
client = mqtt.Client()

# Set up the callback for when a message is received
client.on_message = on_message

# Connect to MQTT broker
client.connect(mqtt_broker, mqtt_port, 60)

# Subscribe to the image topic
client.subscribe(mqtt_topic)

# Start the MQTT loop to keep it running
print("Waiting for image...")
client.loop_forever()
