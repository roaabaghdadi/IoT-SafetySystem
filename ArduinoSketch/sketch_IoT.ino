#include <WiFiS3.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Sensor pins
#define DHTPIN 8         // Digital pin connected to DHT11
#define DHTTYPE DHT11    // DHT 11
#define MQ2_PIN A0       // Analog pin connected to MQ-2 sensor

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "MagentaWLAN-NHL6";
const char* pass = "R.K.0106199018081994"; 

int status = WL_IDLE_STATUS;
char server[] = "khaledkassar.pythonanywhere.com"; 
int port = 80;  // HTTP port

WiFiClient client;

// Variables for sensors
float temperature = 0.0;
int gasValue = 0;
float gasVoltage = 0.0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
  // Initialize sensors
  dht.begin();
  pinMode(MQ2_PIN, INPUT);
  
  connectToWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    connectToWiFi();
  } else {
    // Send data every 5 seconds
    sendDataToServer();
    delay(5000);
  }
}

void connectToWiFi() {
  Serial.println("Attempting to connect to SSID: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  int attempts = 0;
  while (status != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    status = WiFi.status();
    attempts++;
  }
  
  if (status == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi!");
    printWifiStatus();
  } else {
    Serial.println("\nFailed to connect to WiFi!");
  }
}

void readSensors() {
  // Read temperature from DHT11
  temperature = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  
  // Read gas sensor from MQ-2
  gasValue = analogRead(MQ2_PIN);
  gasVoltage = (gasValue * 5.0) / 1024.0;  // For 5V systems
  
  Serial.print("Gas Sensor Value: ");
  Serial.print(gasValue);
  Serial.print(" | Voltage: ");
  Serial.print(gasVoltage);
  Serial.println("V");
}

void sendDataToServer() {
  Serial.println("\nSending data to server...");
  
  // Read all sensor data before sending
  readSensors();
  
  if (client.connect(server, port)) {
    Serial.println("Connected to server");
    
    // Create JSON data with both sensors
    StaticJsonDocument<200> doc;
    
    doc["temperature"] = temperature;
    doc["gas_level"] = gasValue;        // Raw analog value (0-1023)
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    // Send HTTP POST request
    client.println("POST /api/data HTTP/1.1");
    client.println("Host: " + String(server));
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println();
    client.println(jsonData);
    
    Serial.println("Data sent: " + jsonData);
    
    // Wait for response
    unsigned long timeout = millis();
    while (client.connected() && millis() - timeout < 5000L) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    
    client.stop();
    Serial.println("Connection closed");
    
  } else {
    Serial.println("Connection to server failed!");
  }
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}