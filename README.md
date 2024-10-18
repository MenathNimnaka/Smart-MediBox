# ESP32 Smart MediBox with IoT

## Overview
The **Smart MediBox** is an intelligent pharmaceutical storage system designed to streamline medication management using IoT technology. This project was developed as part of the *Embedded Systems and Applications (EN2853)* module during Semester 4. The system uses an ESP32 microcontroller to manage medication schedules, monitor storage conditions, and send reminders, ensuring proper storage and timely medication intake. Real-time monitoring and remote control are achieved through MQTT communication, making it a comprehensive solution for medication management.

## Features

- **Medication Schedule Management**: Allows users to set reminders for medication intake.
- **Remote Monitoring**: Real-time data on storage conditions like temperature and light levels can be viewed via an IoT dashboard.
- **Automated Storage Control**: Monitors the internal environment and adjusts based on sensor inputs to maintain optimal storage conditions.
- **Alarms and Notifications**: Provides audio alerts and notifications for reminders or if storage conditions deviate from predefined thresholds.
- **IoT Integration**: Uses MQTT for seamless data communication and remote control.

## Hardware Components

- **ESP32 Microcontroller**: The central controller for the MediBox system.
- **DHT22 Sensor**: Measures temperature and humidity inside the MediBox.
- **LDR Sensors**: Monitors light intensity to ensure medications are stored in proper lighting conditions.
- **Servo Motor**: Controls the MediBox lid for automated opening and closing.
- **Buzzer**: Audio alerts for reminders and warnings.
- **Wi-Fi Module**: ESP32’s in-built Wi-Fi for IoT communication.
- **OLED Display (Optional)**: Displays reminders and system information on the MediBox itself.

## Software Components

- **Arduino IDE**: Used for coding and uploading the program to the ESP32.
- **MQTT Protocol**: Enables communication between the MediBox and remote server or mobile applications.
- **NTP Client**: Synchronizes time for managing medication schedules.
- **PubSubClient Library**: Manages MQTT communication between the MediBox and remote servers.

## Hardware Setup

1. **ESP32**: Central controller for all components.
2. **DHT22 Sensor**: Connected to the ESP32 to monitor temperature and humidity.
3. **LDR Sensors**: Detects light levels to ensure the environment is suitable for medication storage.
4. **Servo Motor**: Controls the MediBox lid for opening and closing.
5. **Buzzer**: Provides audio alerts for medication reminders.
6. **OLED Display (Optional)**: Displays relevant system information like temperature and reminders.

### Wiring Overview:
- **DHT22 Sensor**: Connected to the ESP32's GPIO pins.
- **LDR Sensors**: Connected to the ESP32 for light detection.
- **Servo Motor**: Attached to a GPIO pin for lid control.
- **Buzzer**: Connected to a digital output pin for audio alerts.

## How It Works

1. **Monitoring**: The MediBox continuously monitors temperature, humidity, and light levels using DHT22 and LDR sensors.
2. **Notifications**: When it’s time to take medication, the system sends alerts via a buzzer and an MQTT message.
3. **Environmental Adjustments**: If storage conditions deviate (e.g., too much light or improper temperature), the system triggers a response to alert the user.
4. **Lid Control**: The servo motor opens and closes the lid when needed, allowing automated access to medications.
5. **Real-Time Data**: All data is accessible via an IoT dashboard, which can be remotely monitored using MQTT.

