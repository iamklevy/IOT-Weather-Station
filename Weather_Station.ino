#include <DHT.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "WiFi.h"

//Network credential 
const char* ssid = "Fifi";
const char* password = "o12w11h1a8s2";

#define DHTPIN 15  // Digital pin connected to the DHT sensor
#define CS_PIN 2
#define DHTTYPE DHT22 
#define ALTITUDE 200.0 //altitude in meters

DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;


File dataFile;
           
String readDHTTemperature() {

  float temperature = dht.readTemperature();

  if (isnan(temperature)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(temperature);
    return String(temperature);
  }
}

String readDHTHumidity(){

  float humidity = dht.readHumidity();

  if(isnan(humidity)){
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
   else {
    Serial.println(humidity);
    return String(humidity);
  }
}

void setup() {

  Serial.begin(115200);
  dht.begin();
  SD.begin(CS_PIN);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
     attempts++;
  }
  Serial.println("IP address of ESP32 is : ");
  Serial.println(WiFi.localIP());
  Serial.println(readDHTTemperature());
  Serial.println(readDHTHumidity());

  if (!bmp.begin()) 
  {
   Serial.println("Failed to initialize BMP180/BMP085 sensor. Check connections.");
   while(1);
  }
   if (!SD.begin())
  {
    Serial.println("Failed to initialize SD card. Check connections.");
    while (1);
  }

  dataFile = SD.open("sensor_data.txt", FILE_WRITE);
  dataFile.println("Temperature (DHT, BMP) [°C]\tHumidity [RH%]\tPressure [Pa]\tAltitude [m]");

}
void loop() {

  float dhtTemperature = dht.readTemperature();
  float dhtHumidity = dht.readHumidity();

  float bmpTemperature = bmp.readTemperature();
  float bmpPressure = bmp.readPressure();
  float bmpAltitude = bmp.readAltitude();

  Serial.println("Temperature from DHT sensor: " + String(dhtTemperature) + " °C");
  Serial.println("Humidity from DHT sensor: " + String(dhtHumidity) + " %");
  Serial.println("Temperature from BMP180/BMP085 sensor: " + String(bmpTemperature) + " °C");
  Serial.println("Pressure from BMP180/BMP085 sensor: " + String(bmpPressure) + " Pa");
  Serial.println("Altitude from BMP180/BMP085 sensor: " + String(bmpAltitude) + " meters");
  Serial.println();

  dataFile.print(dhtTemperature);
  dataFile.print("\t");
  dataFile.print(dhtHumidity);
  dataFile.print("\t");
  dataFile.print(bmpTemperature);
  dataFile.print("\t");
  dataFile.print(bmpPressure);
  dataFile.print("\t");
  dataFile.println(bmpAltitude);

  delay(500);

}
