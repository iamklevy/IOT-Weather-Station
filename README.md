# IOT-Weather-Station
Natural disasters that are related to weather causes loss in both lives and properties, but it can be detected and reported to avoid loss so we designe a weather station that measures temperature, humidity, altitude and air pressure, record these readings for studying and send alert if any of the readings is abnormal.

BMP180/BMP085 Sensor:

- VCC pin of the BMP180/BMP085 sensor with the 3.3V or vin (5v) output on the ESP32.
- the GND pin of the BMP180/BMP085 sensor with the GND on the ESP32.
- the SDA pin of the BMP180/BMP085 sensor with the GPIO21 on the ESP32.
- the SCL pin of the BMP180/BMP085 sensor with the GPIO22 on the ESP32.

DHT22 Sensor:

- VCC pin of the DHT22 sensor with the 3.3V output on the ESP32.
- the GND pin of the DHT22 sensor with the GND on the ESP32.
- the data pin of the DHT22 sensor with GPIO18 on the ESP32.

SD Card Module:

- the VCC pin of the SD card module with the 5V output on the ESP32.
- the GND pin of the SD card module with the GND on the ESP32.
- the MISO, MOSI, and SCK pins of the SD card to the corresponding SPI pins on the ESP32.(GPIO19),(GPIO23), and (GPIO18).
- the CS pin of the SD card module to a digital pin GPIO5 on the ESP32.

  <img src = "https://github.com/iamklevy/Weather-Station/assets/94145850/986edd3b-9d12-4334-9671-829cdeefa709" width="2100" height="500"/>
