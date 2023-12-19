#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "ESP32_MailClient.h"
#include <DHT.h>
#include <vector>
#include <LinearRegression.h>

// Replace with your network credentials
const char* ssid = "####";
const char* password = "#####";

// DHT sensor
#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// BMP180 sensor
Adafruit_BMP085 bmp;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String readBMPTemperature() {
  // Read temperature from BMP180 sensor
  return String(bmp.readTemperature());
}

String readPressure() {
  // Read pressure from BMP180 sensor
  return String(bmp.readPressure());
}

String readAltitude() {
  // Read altitude from BMP180 sensor
  return String(bmp.readAltitude());
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .bmp-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Weather Station</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
  <p>
    <i class="fas fa-bars" style="color:#38059e;"></i> 
    <span class="bmp-labels">Altitude</span> 
    <span id="altitude">%ALTITUDE%</span>
    <sup class="units">meters</sup>
  </p>
  <p>
    <i class="fas fa-bars" style="color:#00d6c8;"></i> 
    <span class="bmp-labels">Pressure</span>
    <span id="pressure">%PRESSURE%</span>
    <sup class="units">hPa</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("altitude").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/altitude", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET

", "/pressure", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>
)rawliteral";

// Replaces placeholder with sensor values
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  else if(var == "ALTITUDE"){
    return readAltitude();
  }
  else if(var == "PRESSURE"){
    return readPressure();
  }
  return String();
}




float mean(const vector<float>& data) {
    float sum = 0.0;
    for (const float& value : data) {
        sum += value;
    }
    return sum / data.size();
}


float covariance(const vector<float>& x, const vector<float>& y) {
    float sum = 0.0;
    float xMean = mean(x);
    float yMean = mean(y);

    for (size_t i = 0; i < x.size(); ++i) {
        sum += (x[i] - xMean) * (y[i] - yMean);
    }

    return sum / x.size();
}

// Gets M and B and puts them in a pair variable
pair<float, float> linearRegression(const vector<float>& x, const vector<float>& y) {
    float m = covariance(x, y) / covariance(x, x);
    float b = mean(y) - m * mean(x);

    return make_pair(m, b);
}

//predicts the value
float predict(float x, float m, float b) {
    return m * x + b;
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  //Wire.begin (21, 22);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }
  
  vector<float> days = {1, 2, 3, 4, 5, 6};
  vector<float> highTemperatures = {24, 25, 26, 24, 23,22 , 22};
  vector<float> lowTemperatures = {15, 14, 15, 15, 15, 13, 14};

  // Calculate linear regression coefficients for high temperatures
  pair<float, float> highCoefficients = linearRegression(days, highTemperatures);

  // Calculate linear regression coefficients for low temperatures
  pair<float, float> lowCoefficients = linearRegression(days, lowTemperatures);


  
  vector<float> highPredictions;
  vector<float> lowPredictions; 
  int numOfDaysToPredict = 7;


  for (int i = 1; i <= numDaysToPredict; ++i) {
      float highPrediction = predict(i + 6, highCoefficients.first, highCoefficients.second);
      float lowPrediction = predict(i + 6, lowCoefficients.first, lowCoefficients.second);

      highPredictions.push_back(highPrediction);
      lowPredictions.push_back(lowPrediction);
      days.push_back(i+6);
  }

  for (size_t i = 0; i < highPredictions.size(); ++i) {
      Serial.print("Day ");
      Serial.print(i + days.size() + 1);
      Serial.print(" - High: ");
      Serial.print(highPredictions[i]);
      Serial.print("°C, Low: ");
      Serial.print(lowPredictions[i]);
      Serial.println("°C");
    }



  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readAltitude().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readPressure().c_str());
  });

  // Start server
  server.begin();
}

void loop(){
  
}
