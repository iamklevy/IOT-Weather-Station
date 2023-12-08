#include <DHT.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "WiFi.h"

//Network credential 
const char* ssid = "Fifi";
const char* password = "o12w11h1a8s2";

#define DHTPIN 15  // Digital pin connected to the DHT sensor
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
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("IP address of ESP32 is : ");
  Serial.println(WiFi.localIP());
  Serial.println(readDHTTemperature());
  Serial.println(readDHTHumidity());

if (!bmp.begin()) 
{
Serial.println("Not connected with BMP180/BMP085 sensor, check connections ");
 
}
}
void loop() {
  Serial.print("Temp = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");


  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");


  Serial.println();
  delay(500);

}
