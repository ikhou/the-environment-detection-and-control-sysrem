#include <Wire.h>
#include <math.h>
#include <rgb_lcd.h>
#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <Servo.h>

int soilMoistureValue;
float temperatureValue;
int lightSensorValue;
int status = WL_IDLE_STATUS;  // Declare 'status' before using it
// Replace network credentials
char ssid[] = "weidong";     // Your network SSID
char pass[] = "12345678"; // Your Internet password

// ThingSpeak Settings
const unsigned long channel_id = 2392540;
const char *write_api_key = "36A4ZJ86OE08LHVQ";

// Initializes the sensor and output
rgb_lcd lcd;
const int temperatureSensorPin = A0;        //Temperature sensor 
const int soilMoistureSensorPin = A1;  //Soil moisture sensor
const int lightSensorPin = A2; //Light sensor
const int buzzerPin = 2;              // buzzer
const int fanPin = 3;                 // fan
const int motorPin = 4;               // servo
const int ledPin = 5;                 //led
Servo myServo;// Create a Servo object

WiFiClient client;

// Read temperature 
float readTemperature(int sensorPin) {
//Read the value of the temperature sensor and calculate the temperature  
int sensorValue = analogRead(sensorPin);
float resistance = (1023.0 / sensorValue - 1.0) * 10000.0; // Calculate resistance
float temperature = 1.0 / (log(resistance / 10000.0) / 3950.0 + 1 / 298.15) - 273.15; // Calculate temperature
return temperature; 
}

void setup() {
  //Initialize serial communication
  Serial.begin(9600); 
  //Initialize LCD screen
  lcd.begin(16, 2);
  lcd.setRGB(255, 255, 255); //  Set backlight color
  //Initialize output pin modes
  pinMode(fanPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  myServo.attach(motorPin); // Attach the servo to the pin
  //Connect to WiFi
  Serial.println("Attempting to connect to WiFi...");
  //Check return value
  status = WiFi.begin(ssid, pass);
  delay(5000);  // Wait for 5 seconds for the connection to establish
  while (status != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    status = WiFi.status();
  }

if (status == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("IP address:");
    Serial.println(WiFi.localIP());
  } 
else {
    Serial.println("WiFi connection failed");
  }

  //Initialize ThingSpeak
  ThingSpeak.begin(client);

}

void loop() {

  // Read temperature, soil moisture, and light sensor values
  temperatureValue = readTemperature(temperatureSensorPin);
  int soilMoistureValue = analogRead(soilMoistureSensorPin);
  int lightSensorValue = analogRead(lightSensorPin);
  
  // Print sensor values to Serial Monitor  
  Serial.print("Temperature: ");
  Serial.print(temperatureValue);
  Serial.print(" °C\t");

  Serial.print("Soil Moisture: ");
  Serial.print(soilMoistureValue);
  Serial.print(" \t");

  Serial.print("Light Sensor: ");
  Serial.print(lightSensorValue);
  Serial.print(" \t\n");

  // Update LCD display
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperatureValue);
  lcd.print((char)223); // ° symbol
  lcd.print("C");
      
  lcd.setCursor(0, 1);
  lcd.print("SM: ");
  lcd.print(soilMoistureValue);
  lcd.print("  ");
  lcd.print("L: ");

  // Determine if the light is on or off based on the threshold
  if (lightSensorValue > 250) {
    lcd.print("On");
    Serial.println("Light is ON");
    digitalWrite(ledPin, LOW);
    // Do something when the light is on
  } else {
    lcd.print("Off");
    Serial.println("Light is OFF");
    digitalWrite(ledPin, HIGH);
    // Do something when the light is off
  }
    
  // Control hardware based on conditions
  if (temperatureValue > 30) {
      digitalWrite(fanPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
  } else {
      digitalWrite(fanPin, LOW);
     digitalWrite(buzzerPin, LOW);
  }
  if (soilMoistureValue > 400) {
      myServo.write(90); // Rotate the servo to 90 degrees
      digitalWrite(buzzerPin, HIGH);
  } else {
      myServo.write(0); // Rotate the servo back to 0 degrees
      digitalWrite(buzzerPin, LOW);
  }
  // Control hardware based on soil moisture and temperature
  if (soilMoistureValue > 400 || temperatureValue > 30) {
        tone(buzzerPin, 1000);// Set buzzer frequency to 1000Hz
        lcd.setRGB(255, 0, 0);// Set LCD screen background to red
  } else {
        digitalWrite(fanPin, LOW);
        noTone(buzzerPin);
        lcd.setRGB(0, 255, 0);/// Set LCD screen background to green
  }
// Send data to ThingSpeak
  ThingSpeak.setField(1, temperatureValue);
  ThingSpeak.setField(2, soilMoistureValue);
  ThingSpeak.setField(3, lightSensorValue);
  //Check return value for writing to ThingSpeak
  int writeResult = ThingSpeak.writeFields(channel_id, write_api_key);
  if (writeResult == 200) {
    Serial.println("ThingSpeak update successful");
  } 
// Delay, ensuring ThingSpeak update frequency is not less than 15 seconds
delay(15000);
}
