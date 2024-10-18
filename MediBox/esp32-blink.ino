// Include libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>

// Define OLED parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3c

// Define pin assignments
#define BUZZER 5
#define LED_1 15
#define LED_2 2
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12

// NTP server information
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_DST 0

// Global variables

//time zone variables
int UTC_OFFSET = 0;
int offset_hours = 0;
int offset_mins = 0;

// Global variables for time and alarms
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

bool alarm_enabled = false;
int n_alarms = 3;
int alarm_hours[] = {0, 1, 0};
int alarm_minutes[] = {1, 10, 0};
bool alarm_triggered[] = {false, false,false};

// Musical notes for the buzzer
int n_notes = 8;
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

// Current mode and available modes for the menu
int current_mode = 0;
int max_modes = 5;
String modes[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2","4 - Set Alarm 3", "5 - Disable Alarm"};

// Declare objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

void setup() {
  // Initialization code, runs once
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  Serial.begin(9600);
 

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(500);
  
  // Connecting to Wi-Fi
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WIFI", 0, 0, 2);
  }

  display.clearDisplay();
  print_line("Connected to WIFI", 0, 0, 2);

  display.clearDisplay();   //Clear the buffer
  print_line("Welcome to Medibox!", 10, 20, 2);  //display the welcome message
  delay(500);
  display.clearDisplay();

}

void loop() {
  // Main code running repeatedly
  update_time_with_check_alarm();
  if (digitalRead(PB_OK) == LOW){
    delay(200);
    go_to_menu();
  }
 
  check_temp();

}

// Function to print text lines on the OLED display
void print_line(String text, int column, int row, int text_size){
  display.setTextSize(text_size);           // Set the text size
  display.setTextColor(SSD1306_WHITE);      // Set the text color to white
  display.setCursor(column, row);           // Set the cursor position on the OLED display (column, row)
  display.println(text);                    // Print the specified text on the OLED display

  display.display();                        // Update the OLED display to show the printed tex

}

// Function to print the current time on the OLED display
void print_time_now(void){
  display.clearDisplay();
  print_line(String(days), 0, 0, 2);        // Display the current day
  print_line(":", 20, 0, 2);
  print_line(String(hours), 30, 0, 2);      // Display the current hour
  print_line(":", 50, 0, 2);
  print_line(String(minutes), 60, 0, 2);    // Display the current minute
  print_line(":", 80, 0, 2);
  print_line(String(seconds), 90, 0, 2);    // Display the current second
}

// Function to update the current time using NTP and check for alarms
void update_time_with_check_alarm(void){
  // Update the current time
  update_time();

  // Print the updated time on the OLED display
  print_time_now();
  
  // Check if alarms are enabled
  if (alarm_enabled == true){
    for (int i = 0; i < n_alarms; i++){
      // Check if the alarm is not triggered and the current time matches the alarm time
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
        ring_alarm();   // call the ringing function
        alarm_triggered[i] = true;
      }
    }
  }
}

//Function to update time
void update_time(void){
  // Configure the time settings using NTP
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
  
  struct tm timeinfo;  // Structure to store time information
  
  getLocalTime(&timeinfo); // Get the local time from the system
  
  // Extract and convert days, hours, minutes and seconds
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  hours = atoi(timeHour);

  char timeMinutes[3];
  strftime(timeMinutes,3, "%M", &timeinfo);
  minutes = atoi(timeMinutes);

  char timeSecond[3];
  strftime(timeSecond,3, "%S", &timeinfo);
  seconds = atoi(timeSecond);
  
  char timeDay[3];
  strftime(timeDay,3, "%d", &timeinfo);
  days = atoi(timeDay);

}

// Funtion to ring alarm
void ring_alarm(){
  // Show message on display
  display.clearDisplay();
  print_line("MEDICINE TIME!", 0, 0, 2);
  
  digitalWrite(LED_1, HIGH);  // Turn on the LED to indicate the alarm

  bool break_happened = false;

  // Ring the buzzer
  while (break_happened == false && digitalRead(PB_CANCEL) == HIGH){
    for (int i = 0; i < n_notes; i++){
      
      // Check if the cancel button is pressed to interrupt the alarm
      if (digitalRead(PB_CANCEL) == LOW){
        delay(200);
        break_happened = true;
        break;
      }
      
      // Play the musical note on the buzzer
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
    
    // Turn off the LED and clear the display after playing the notes
    digitalWrite(LED_1, LOW);
    display.clearDisplay();
  }
  
}  

// Function to a button press and return the pressed button
int wait_for_button_press(){
  while (true){
    if (digitalRead(PB_UP) == LOW){
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW){
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW){
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_CANCEL) == LOW){
      delay(200);
      return PB_CANCEL;
    }

    // Update the time while waiting for button press
    update_time();
  }
}

// Function to navigate through the menu options
void go_to_menu(){
  while (digitalRead(PB_CANCEL) == HIGH){
    display.clearDisplay();

    // Display the current mode on the OLED display
    print_line(modes[current_mode], 0, 0, 2);
    
    // Wait for a button press
    int pressed = wait_for_button_press();

    // Respond based on the pressed button
    if (pressed == PB_UP){
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      current_mode -= 1;
      if (current_mode < 0){
        current_mode = max_modes - 1;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      run_mode(current_mode); //Execute the selected mode when OK button is pressed
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;  //Break out of the loop and exit the menu when CANCEL button is pressed
    }
  }
}

// Function to set a time zone
void set_time_zone(){
  // Initialize temporary variables for offset hours
  int temp_offset_hour = offset_hours;

  // Loop to set offset hours
  while (true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_offset_hour), 0, 0, 2);
    
    // Wait for a button press
    int pressed = wait_for_button_press();

    //Increment and wrap offset hours  between 14 and -12
    if (pressed == PB_UP){  // Respond based on the pressed button
      delay(200);
      temp_offset_hour += 1;
      if (temp_offset_hour > 14)
      {
        temp_offset_hour = -12 ;
      }
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_offset_hour -= 1;
      if (temp_offset_hour < -12)
      {
        temp_offset_hour = 14;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      offset_hours = temp_offset_hour; // Set the final offset hours and break out of the loop
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }
  
  // Initialize temporary variables for offset hours
  int temp_offset_minute = offset_mins;

  // Loop to set offset minutes
  while (true){
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_offset_minute), 0, 0, 2);
    
    
    //Increment and wrap offset minutes between 5 and 55
    int pressed = wait_for_button_press();   // Respond based on the pressed button
    if (pressed == PB_UP){
      delay(200);
      temp_offset_minute += 5;
      temp_offset_minute = temp_offset_minute % 55;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_offset_minute -= 5;
      if (temp_offset_minute < 0){
        temp_offset_minute = 55;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      offset_mins = temp_offset_minute;  // Set the final offset minutes and break out of the loop
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }
  
  UTC_OFFSET = offset_hours*3600 + offset_mins*60 ; // Calculate the total UTC offset in seconds

  display.clearDisplay();
  print_line("Time Zone is set", 0, 0, 2); // Display a confirmation message on the OLED screen
  delay(1000);
}

// Function to set an alarm
void set_alarm(int alarm){
  // Enable alarms
  alarm_enabled = true;
  
  // Initialize temporary variable for alarm hours with the current value
  int temp_hour = alarm_hours[alarm];

  // Loop to set alarm hours
  while (true){
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2); //display the current alarm hours
    
    // Wait for a button press
    int pressed = wait_for_button_press();

    // Increment and wrap alrm_hours between 1 and 23
    if (pressed == PB_UP){   // Respond based on the pressed button
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0){
        temp_hour = 23;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;   // Set the final alarm hours and break out of the loop
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }
  
  // Initialize temporary variable for alarm minutes with the current value
  int temp_minute = alarm_minutes[alarm];

  // Loop to set alarm minutes
  while (true){
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);  //display the current alarm minutes
    
     // Increment and wrap alrm_minutes between 1 and 59
    int pressed = wait_for_button_press();  // Respond based on the pressed button
    if (pressed == PB_UP){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }

    else if (pressed == PB_DOWN){
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0){
        temp_minute = 59;
      }
    }

    else if (pressed == PB_OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;  // Set the final alarm minutes and break out of the loop
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }
  
  // Display a confirmation message on the OLED screen
  display.clearDisplay();
  print_line("Alarm is set", 0, 0, 2);
  delay(1000);
}

void run_mode(int mode){

  // Execute different actions based on the selected mode
  if (mode == 0){  // Mode 0: Set Time Zone
    set_time_zone();
  }

  else if (mode == 1 || mode == 2 || mode == 3){ // Mode 1, 2, or 3: Set Alarm 1, Set Alarm 2, or Set Alarm 3
    set_alarm(mode - 1);
  }

  else if (mode == 4){ // Mode 4: Disable All Alarms
    disable_all_alarms();
  }
}

// Function to disable all alarms and reset triggered status
void disable_all_alarms(){
  alarm_enabled = false;

  for (int i = 0; i < n_alarms; i++) {
        alarm_triggered[i] = false;
  }

  display.clearDisplay();
  
  // Display a message indicating that all alarms are disabled
  print_line(" Disabled all alarms.", 0, 0, 2); 
  delay(1000);
}

// Function to check temperature and humidity
void check_temp() {

  // Read temperature and humidity data from the DHT sensor
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  
  // Check if the temperature is higher than 32 degrees Celsius
  if (data.temperature > 32) {
    display.clearDisplay();
    digitalWrite(LED_2, HIGH);
    print_line("TEMP HIGH", 0, 20, 2); // Display a message indicating high temperature
    delay(500);
    digitalWrite(LED_2, LOW);
  } 

  // Check if the temperature is lower than 26 degrees Celsius
  else if (data.temperature < 26) {
    display.clearDisplay();
    digitalWrite(LED_2, HIGH);
    print_line("TEMP LOW", 0, 20, 2);  // Display a message indicating low temperature
    delay(500);
    digitalWrite(LED_2, LOW);
  }
  
  // Check if the humidity is higher than 80%
  if (data.humidity > 80) {
    display.clearDisplay();
    digitalWrite(LED_2, HIGH);
    print_line("Humidity HIGH", 0, 35, 2);  // Display a message indicating high humidity
    delay(500);
    digitalWrite(LED_2, LOW);
  } 

  // Check if the humidity is lower than 60%
  else if (data.humidity < 60) {
    display.clearDisplay();
    digitalWrite(LED_2, HIGH);
    print_line("Humidity LOW", 0, 35, 2);  // Display a message indicating low humidity
    delay(500);
    digitalWrite(LED_2, LOW);
  }
}
