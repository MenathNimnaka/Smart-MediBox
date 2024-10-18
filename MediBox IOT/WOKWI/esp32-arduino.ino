#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

#define DHT_PIN 15
#define BUZZER 12
#define left_LDR 32
#define right_LDR 33
#define servoPin 18

char tempAr[6];
char lightAr[6];

WiFiClient espClient;
PubSubClient mqttClient(espClient);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

float theta_off = 30;
float Gamma = 0.75;
float I = 0.0;
float D;
float angle;
Servo servo;


DHTesp dhtSensor;

bool isScheduledON = false;
unsigned long scheduledOnTime;

void setup() {
  Serial.begin(115200);
  setupWifi();

  setupMqtt();

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  timeClient.begin();
  timeClient.setTimeOffset(5.5*3600);
  pinMode(BUZZER, OUTPUT);
  pinMode(left_LDR, INPUT);
  pinMode(right_LDR, INPUT);
  pinMode(servoPin, OUTPUT);
  digitalWrite(BUZZER, LOW);
  servo.attach(servoPin, 500, 2400);

}

void loop() {

  if(!mqttClient.connected()){
    connectToBroker();
  }
  mqttClient.loop();
  
  updateTemperature();
  Serial.println(tempAr);
  mqttClient.publish("AADMIN-TEMP", tempAr);

  highIntensity();

  angle = calculateMotorAngle();
  servo.write(angle);
  
  checkSchedule();
  delay(1000);
  
}

void setupWifi(){ 
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println("Wokwi-GUEST");
  WiFi.begin("Wokwi-GUEST","");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void setupMqtt(){
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

void recieveCallback(char* topic, byte* payload, unsigned int length){
  float angle;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]");

  char payloadCharAr[length];
  for(int i = 0; i<length; i++){
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }

  Serial.println();
  
  if(strcmp(topic, "AADMIN-MAIN-ON-OFF")==0){
    buzzerOn(payloadCharAr[0]=='1');  
  }else if(strcmp(topic, "AADMIN-SCH-ON")==0){
    if(payloadCharAr[0]=='N'){
      isScheduledON = false;
    }else{
      isScheduledON = true;
      scheduledOnTime = atol(payloadCharAr);
    }
  }else if (strcmp(topic, "AADMIN-SHADE-CONTROL") == 0) {
    Gamma = atof(payloadCharAr);
      
  }else if (strcmp(topic, "AADMIN-SHADE-ANGLE") == 0) {
    theta_off = atof(payloadCharAr);
  
  }


}

void connectToBroker(){
  while(!mqttClient.connected()){
    Serial.print("Attempting MQTT connection...");
    if(mqttClient.connect("ESP32-21553263")){
      Serial.println("connected");
      mqttClient.subscribe("AADMIN-MAIN-ON-OFF");
      mqttClient.subscribe("AADMIN-SCH-ON");
      mqttClient.subscribe("AADMIN-SHADE-ANGLE");
      mqttClient.subscribe("AADMIN-SHADE-CONTROL");
      }else{
      Serial.print("failed");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void buzzerOn(bool on){
  if (on){
    tone(BUZZER,256);
  }else{
    noTone(BUZZER);
  }
}


unsigned long getTime(){
  timeClient.update();
  return timeClient.getEpochTime();
}

void checkSchedule(){
  if(isScheduledON){
    unsigned long currentTime = getTime();
    if(currentTime>scheduledOnTime){
      buzzerOn(true);
      isScheduledON = false;
      mqttClient.publish("AADMIN-MAIN-ON-OFF-ESP", "1");
      mqttClient.publish("AADMIN-SCH-ESP-ON", "0");
      Serial.println("Scheduled ON");
    }
  }
}

float calculateMotorAngle() {
  float theta = (theta_off * D) + (180 - theta_off) * I * Gamma;
  return min(theta, 180.0f);
}

void highIntensity(){

  int Left_val = analogRead(left_LDR);
  int Right_val = analogRead(right_LDR);


  float leftIntensity = 1.0 - (map(Left_val,32,4063,0,1023) / 1023.0);
  float rightIntensity = 1.0 - (map(Right_val,32,4063,0,1023)/ 1023.0);

  
  // Determine which LDR has higher value
  bool isLeftIntensityHigher = leftIntensity > rightIntensity;

  if (isLeftIntensityHigher) {
    String(leftIntensity,2).toCharArray(lightAr,6);
    mqttClient.publish("AADMIN-LIGHT", lightAr);
    mqttClient.publish("AADMIN-LIGHT-SIDE", "Left side");
    I = leftIntensity;
    D = 1.5;
  } else {
    String(rightIntensity,2).toCharArray(lightAr,6);
    mqttClient.publish("AADMIN-LIGHT", lightAr);
    mqttClient.publish("AADMIN-LIGHT-SIDE", "Right side");
    I = rightIntensity;
    D = 0.5;
  }


}

void updateTemperature(){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  String(data.temperature,2).toCharArray(tempAr,6);
}

