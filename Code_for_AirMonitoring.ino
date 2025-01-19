#include <Wire.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <DHT.h>

#define MQ2_PIN 36
#define DHT_PIN 4
#define DHT_TYPE DHT11


const char* ssid = "*****";         
const char* password = "******";  


DHT dht(DHT_PIN, DHT_TYPE);

LiquidCrystal lcd(14, 27, 16, 27, 18, 19);  


WiFiServer server(80);


float coLevel = 0.0;
float temperature = 0.0;
float humidity = 0.0;

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Air Quality Monitor</title>
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
</head>
<body>
  <div class="container mt-5">
    <h1 class="text-center">Air Quality Monitor</h1>
    <div class="card mt-4">
      <div class="card-body">
        <h3>Sensor Readings</h3>
        <p><strong>CO Level:</strong> <span id="co-level">--</span> ppm</p>
        <p><strong>Temperature:</strong> <span id="temperature">--</span> °C</p>
        <p><strong>Humidity:</strong> <span id="humidity">--</span> %</p>
      </div>
    </div>
  </div>
  <script>
    async function fetchData() {
      try {
        const response = await fetch('/data');
        const data = await response.json();
        document.getElementById('co-level').textContent = data.co;
        document.getElementById('temperature').textContent = data.temperature;
        document.getElementById('humidity').textContent = data.humidity;
      } catch (error) {
        console.error('Error fetching data:', error);
      }
    }

    setInterval(fetchData, 1000); // Fetch data every second
  </script>
</body>
</html>
)rawliteral";

void setup() {

  Serial.begin(115200);

 
  dht.begin();
  lcd.begin(16, 2);  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Air Quality");
  lcd.setCursor(0, 1);
  lcd.print("Monitor Ready");
  delay(2000);
  lcd.clear();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  
  server.begin();
}

void loop() {
  
  coLevel = analogRead(MQ2_PIN) * (5.0 / 1023.0) * 10; 
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

 
  lcd.setCursor(0, 0);
  lcd.print("CO: ");
  lcd.print(coLevel);
  lcd.print(" ppm");
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C H:");
  lcd.print(humidity);
  lcd.print("%");

  
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();


    if (request.indexOf("GET /data") >= 0) {
      String jsonResponse = "{\"co\":" + String(coLevel) +
                            ",\"temperature\":" + String(temperature) +
                            ",\"humidity\":" + String(humidity) + "}";
      
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type: application/json");
      client.println();
      client.println(jsonResponse);
    }

    else {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type: text/html");
      client.println();
      client.println(webpage);
    }
    client.stop();
  }
}
