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
// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount    "example_sender_account@gmail.com"
#define emailSenderPassword   "email_sender_password"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "[ALERT] Weather Alert!"

// Default Recipient Email Address
String Email_Input = "your_email_recipient@gmail.com";
String enableEmailChecked = "checked";
String Email_Enabeled = "true";
// Default Threshold Temperature Value
String temp_threshold = "25.0";
String lastTemperature;

String humidity_threshold = "";
String lastHumidity;

String pressure_threshold = "";
String lastpressure;

String altitude_threshold = "";
String lastaltitude;



#pragma region HTML codes
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Weather Smart Monitoring</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        form {
            background-color: gainsboro;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            text-align: center;
        }

        h1 {
            color: black;
        }

        label {
            display: block;
            margin-top: 10px;
            font-size: 14px;
            color: #3f3d3d;
        }

        input {
            width: 100%;
            padding: 10px;
            margin-top: 5px;
            margin-bottom: 15px;
            box-sizing: border-box;
            border: 1px solid #cccccc;
            border-radius: 4px;
            font-size: 14px;
        }

        input[type="submit"] {
            background-color: #338f36;
            color: #ffffff;
            cursor: pointer;
        }

        input[type="submit"]:hover {
            background-color: #23c52b;
        }
    </style>
</head>
<body>

    <form action="/submit" method="get">
        <h1>Temperature Monitoring System</h1>
    
        <label for="email">Email:</label>
        <input type="email" id="email" name="email" required %EMAIL_INPUT%>
        Enable Email Notification<input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%>
    
        <label for="threshold">Temperature Threshold:</label>
        <input type="number" id="temp" name="threshold" required %TEMPERATURE_THRESHOLD%>
    
        <label for="Humidity">Humidity Threshold:</label>
        <input type="number" id="Humidity" name="Humidity" required %HUMIDITY_THRESHOLD%>
    
        <label for="Pressure">Pressure Threshold:</label>
        <input type="number" id="Pressure" name="Pressure" required %PRESSURE_THRESHOLD%>
    
        <label for="Altitude">Altitude Threshold:</label>
        <input type="number" id="Altitude" name="Altitude" required %ALTITUDE_THRESHOLD%>
    
        <input type="submit" value="Submit">
    </form>
    

</body>
</html>
)rawliteral";
const char index2_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Weather Station</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .weather-container {
            background-color: #ffffff;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
            text-align: center;
        }

        h1 {
            color: #333333;
        }

        .weather-info {
            display: flex;
            flex-wrap: wrap;
            justify-content: space-around;
            margin-top: 20px;
        }

        .weather-item {
            width: 45%;
            padding: 10px;
            margin: 10px;
            box-sizing: border-box;
            border: 1px solid #cccccc;
            border-radius: 4px;
            font-size: 16px;
        }

        .weather-icon {
            width: 36px;
            height: 36px;
            margin-bottom: 10px;
        }
    </style>
</head>
<body>

    <div class="weather-container">
        <h1>Weather Information</h1>

        <div class="weather-info">
            <div class="weather-item">
                <img class="weather-icon" src="" alt="Weather Icon" id="Main_Weather_Icon">
                
            </div>
            <div class="weather-item">
                <img class="weather-icon" src="Icons/Temp.png" alt="Temperature Icon ">
                <div id="temperature">Temperature: %TEMPERATURE%&deg;C</div>
            </div>

            <div class="weather-item">
                <img class="weather-icon" src="Icons/rain.png" alt="Humidity Icon">
                <div id="humidity">Humidity: %HUMIDITY% %</div>
            </div>

            <div class="weather-item">
                <img class="weather-icon" src="Icons\pressure.png" alt="Pressure Icon">
                <div id="pressure">Pressure: %PRESSURE% hPa</div>
            </div>

            <div class="weather-item">
                <img class="weather-icon" src="Icons/altitude.png" alt="Altitude Icon">
                <div id="altitude">Altitude: %ALTITUDE% meters</div>
            </div>
        </div>
    </div>

    <script>
        // Fetch weather data from ESP32 API endpoint
        function fetchWeatherData() {
            fetch('/weather-data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('Main_Weather_Icon').innerHTML = `<img class="weather-icon" src="" alt="Main Weather Icon">`;
                    
                    // Set temperature icon dynamically based on temperature range
                    const weather_Icon = document.getElementById('Main_Weather_Icon');
                    if (data.temperature < 10) {
                        weather_Icon.src = 'Icons\snowflake.png';
                    } else if (data.temperature < 30) {
                        weather_Icon.src = 'Icons\sunny.png';
                    } else {
                        weather_Icon.src = 'Icons\Cloudy';
                    }
                    document.getElementById('temperature').innerHTML = `<img class="weather-icon" src="Icons/Temp.png" alt="Temperature Icon"> Temperature: ${data.temperature} &deg;C`;
                    document.getElementById('humidity').innerHTML = `<img class="weather-icon" src="humidity.png" alt="Humidity Icon"> Humidity: ${data.humidity} %`;
                    document.getElementById('pressure').innerHTML = `<img class="weather-icon" src="pressure.png" alt="Pressure Icon"> Pressure: ${data.pressure} hPa`;
                    document.getElementById('altitude').innerHTML = `<img class="weather-icon" src="altitude.png" alt="Altitude Icon"> Altitude: ${data.altitude} meters`;
                })
                .catch(error => console.error('Error fetching weather data:', error));
        }

        // Fetch weather data on page load
        document.addEventListener('DOMContentLoaded', function () {
            fetchWeatherData();
        });

        // Refresh weather data every 2 minutes
        setInterval(fetchWeatherData, 120000);
    </script>

</body>
</html>

)rawliteral";
#pragma endregion comment

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);
//proccesor for IOT page
String processor(const String& var){
  //Serial.println(var);
  
  if(var == "EMAIL_INPUT"){
    return Email_Input;
  }
  else if(var == "ENABLE_EMAIL"){
    return enableEmailChecked;
  }
  else if(var == "TEMPERATURE_THRESHOLD"){
    return temp_threshold;
  }
  else if(var == "HUMIDITY_THRESHOLD"){
    return humidity_threshold;
  }
  else if(var == "PRESSURE_THRESHOLD"){
    return pressure_threshold;
  }
  else if(var == "ALTITUDE_THRESHOLD"){
    return altitude_threshold;
  }

  return String();
}
//proccesor for Weather Station page
String processor2(const String& var) {
    if (var == "TEMPERATURE") {
        
        float temperature = fetchTemperatureData(); // Replace with your actual function
        return String(temperature, 1);  
    } else if (var == "HUMIDITY") {
        
        float humidity = fetchHumidityData(); // Replace with your actual function
        return String(humidity, 1);  
    } else if (var == "PRESSURE") {
         
        float pressure = fetchPressureData(); // Replace with your actual function
        return String(pressure, 1);  
    } else if (var == "ALTITUDE") {
        
        float altitude = fetchAltitudeData(); // Replace with your actual function
        return String(altitude, 1);
    } else {
        return String();
    }
}


// update the main display 
void updateWeatherHTML(temperature, humidity, pressure, altitude) {
    

    // Update HTML elements with the fetched sensor data
    String script = "<script>";
    script += "document.getElementById('temperature').innerHTML = `<img class=\"weather-icon\" src=\"Icons/Temp.png\" alt=\"Temperature Icon\"> Temperature: " + String(temperature) + " &deg;C`;";
    script += "document.getElementById('humidity').innerHTML = `<img class=\"weather-icon\" src=\"Icons/humidity.png\" alt=\"Humidity Icon\"> Humidity: " + String(humidity) + " %`;";
    script += "document.getElementById('pressure').innerHTML = `<img class=\"weather-icon\" src=\"Icons/pressure.png\" alt=\"Pressure Icon\"> Pressure: " + String(pressure) + " hPa`;";
    script += "document.getElementById('altitude').innerHTML = `<img class=\"weather-icon\" src=\"Icons/altitude.png\" alt=\"Altitude Icon\"> Altitude: " + String(altitude) + " meters`;";
    
    // Set Weather icon dynamically based on temperature range
    script += "const weatherIcon = document.getElementById('Main_Weather_Icon');";
    script += "if (" + String(temperature) + " < 10) { weatherIcon.src = 'Icons/snowflake.png'; }";
    script += "else if (" + String(temperature) + " < 30) { weatherIcon.src = 'Icons/sunny.png'; }";
    script += "else { weatherIcon.src = 'Icons/cloudy.png'; }";

    script += "</script>";

    // Send the script to the client
    server.send(200, "text/html", script);
}

bool emailSent = false;

const char* PARAM_INPUT_EMAIL = "email_input";
const char* PARAM_INPUT_ENABLE = "enable_email_input";
const char* PARAM_INPUT_TEMP = "temp_input";
const char* PARAM_INPUT_HUM = "humidity_input";
const char* PARAM_INPUT_PRES = "pressure_input";
const char* PARAM_INPUT_ALT = "altitude_input";



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

  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/page2", HTTP_GET, [](AsyncWebServerRequest *request){
   request->send_P(200, "text/html", index2_html, processor2);
  });


  // Handle the form submission
server.on("/submit", HTTP_GET, [](AsyncWebServerRequest *request){
  // Retrieve values from the request parameters
  String email = request->getParam("email")->value();
  String enableEmail = request->getParam("enable_email_input")->value();
  float temperatureThreshold = request->getParam("threshold")->value().toFloat();
  float humidityThreshold = request->getParam("Humidity")->value().toFloat();
  float pressureThreshold = request->getParam("Pressure")->value().toFloat();
  float altitudeThreshold = request->getParam("Altitude")->value().toFloat();

  // Now you can use these values to trigger email alerts or perform other actions

  // Send a response back to the client
  request->send(200, "text/html", "Form submitted successfully!");
});


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
