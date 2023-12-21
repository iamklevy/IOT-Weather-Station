#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>

#define DHTPIN 18     
#define DHTTYPE DHT22  

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "###";
const char* password = "######";

// To send Emails using Gmail on port 465 (SSL)
#define emailSenderAccount "########"
#define emailSenderPassword "mycclrqisaitxigu"
#define smtpServer "smtp.gmail.com"
#define smtpServerPort 465
#define emailSubject "[ALERT] ESP32 Temperature"

#define ALTITUDE 15.0


// Default Recipient Email Address
String inputMessage = "#######";
String enableEmailChecked = "checked";
String inputMessage2 = "true";
// Default Threshold Temperature Value
String inputMessage3 = "25.0";

String lastTemperature;
String lastHumidity;
String lastPressure;
String lastAltitude;


// HTML web page to handle 3 input fields (email_input, enable_email_input, threshold_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>

<head>
    <title>Weather Station</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css"
        crossorigin="anonymous">
        <style>
        *{
             margin: 10px;
        }
         h2 {
      font-size: 3.0rem;
      text-align: center; 
    }
      p {
      font-size: 2.0rem;
      text-align: center;
    }
        form {
      max-width: 400px;
      margin: auto;
      font-family: 'Arial', sans-serif;
      padding: 20px;
      background-color: #dedcdc;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
       }
          .units {
      font-size: 1.2rem;
    }
       .dht-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }
     .bmp-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }
      input[type="submit"] {
            display: block;
            margin: 0 auto;
        }
    body {
    background-color: #c2c0c0;
}
    </style>
        </head>
        <body>
    <h2>ESP32 Weather Station</h2>
    <form action="/get">
        Email Address <input type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
        Enable Email Notification <input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%><br>
        Temperature Threshold <input type="number" step="0.1" name="threshold_input" value="%THRESHOLD%" required><br>
        <input type="submit" value="Submit">
    </form>

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
      <i class="fas fa-parking" style="color:#030b0b;"></i>
      <span class="bmp-labels">Pressure</span>
      <span id="pressure">%PRESSURE%</span>
      <sup class="units">hPa</sup>
    </p>


    </body>
</body>

</html>
)rawliteral";

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

AsyncWebServer server(80);

String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  float humidity = dht.readHumidity();
  if(isnan(humidity)){
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else{
    return String(humidity);
  }
}
String readPressure() {
  float pressure = bmp.readPressure();
  if (isnan(pressure)) {    
    Serial.println("Failed to read from BMP sensor!");
    return "--";
  }
  return String(pressure);
}

String readAltitude() {
  float altitude = bmp.readAltitude();
  if (isnan(altitude)) {    
    Serial.println("Failed to read from BMP sensor!");
    return "--";
  }
  return String(altitude);
}

String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return lastTemperature;
  } else if (var == "HUMIDITY") {
    return lastHumidity;
  } else if (var == "EMAIL_INPUT") {
    return inputMessage;
  } else if (var == "ALTITUDE") {
    return lastAltitude;
  } else if (var == "PRESSURE") {
    return lastPressure;
  } else if (var == "ENABLE_EMAIL") {
    return enableEmailChecked;
  } else if (var == "THRESHOLD") {
    return inputMessage3;
  }
  return String();
}

bool emailSent = false;

const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";
const char* PARAM_INPUT_3 = "threshold_input";

// Interval between sensor readings. Learn more about timers: https://RandomNerdTutorials.com/esp32-pir-motion-sensor-interrupts-timers/
unsigned long previousMillis = 0;
const long interval = 5000;

// The Email Sending data object contains config and data to send
SMTPData smtpData;

// Callback function to get the Email sending status
void sendCallback(SendStatus msg) {
  // Print the current status
  Serial.println(msg.info());

  // Do something when complete
  if (msg.success()) {
    Serial.println("----------------");
  }
}

bool sendEmailNotification(String emailMessage) {
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);

  // For library version 1.2.0 and later which STARTTLS protocol was supported,the STARTTLS will be
  // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
  //smtpData.setSTARTTLS(true);

  // Set the sender name and Email
  smtpData.setSender("ESP32", emailSenderAccount);

  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  // Set the subject
  smtpData.setSubject(emailSubject);

  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);

  // Add recipients
  smtpData.addRecipient(inputMessage);

  smtpData.setSendCallback(sendCallback);

  // Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(smtpData)) {
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
  // Clear all data from Email object to free memory
  smtpData.empty();
  return true;
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());

  // Start the DHT11 sensor
  dht.begin();

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
  }

  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readAltitude().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readPressure().c_str());
  });

  // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      } else {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3)) {
        inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      }
    } else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {

   String temp, humidity, pressure, altitude;
  temp = readDHTTemperature();
  humidity = readDHTHumidity();
  pressure = readPressure();
  altitude = readAltitude();
  
  Serial.println("Temperature:");
  Serial.println(temp + "c");
  Serial.println("Humidity:");
  Serial.println(humidity + "%");
  Serial.println("Pressure:");
  Serial.println(pressure + "hPa");
  Serial.println("Altitude:");
  Serial.println(altitude + "meters");

  delay(3000);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    lastTemperature = readDHTTemperature();
    lastHumidity = readDHTHumidity();
    lastPressure = readPressure();
    lastAltitude = readAltitude();

    float temperature = lastTemperature.toFloat();
    // Check if temperature is above threshold and if it needs to send the Email alert
    if (temperature > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent) {
      String emailMessage = String("Temperature above threshold. Current temperature: ") + String(temperature) + String("C");
      if (sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = true;
      } else {
        Serial.println("Email failed to send");
      }
    }
    // Check if temperature is below threshold and if it needs to send the Email alert
    else if ((temperature < inputMessage3.toFloat()) && inputMessage2 == "true" && emailSent) {
      String emailMessage = String("Temperature below threshold. Current temperature: ") + String(temperature) + String(" C");
      if (sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = false;
      } else {
        Serial.println("Email failed to send");
      }
    }
  }
}
