#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "thingsboard.cloud"; // Sesuaikan server Anda
const char* mqtt_token = "YOUR_THINGSBOARD_ACCESS_TOKEN";

WiFiClient espClient;
PubSubClient client(espClient);

const int sensorPin = 34; // Pin potensiometer
const int ledHijau = 25;  // Pin LED Hijau  (Aman: di bawah 750)
const int ledKuning = 26; // Pin LED Kuning (Sedang: 750 - 2499)
const int ledMerah = 27;  // Pin LED Merah  (Tinggi: 2500 ke atas)

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_token, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledHijau, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

unsigned long lastMsg = 0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    int sensorValue = analogRead(sensorPin); // Nilai berkisar antara 0 - 4095

    // Matikan semua LED terlebih dahulu
    digitalWrite(ledHijau, LOW);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledMerah, LOW);

    String statusUdara = "Aman";

    // Logika ambang batas
    if (sensorValue >= 2500) {
      digitalWrite(ledMerah, HIGH);   // LED Merah menyala (Kadar Tinggi)
      statusUdara = "Tinggi";
    }
    else if (sensorValue >= 750) {
      digitalWrite(ledKuning, HIGH);  // LED Kuning menyala (Kadar Sedang)
      statusUdara = "Sedang";
    }
    else {
      digitalWrite(ledHijau, HIGH);   // LED Hijau menyala (Aman)
      statusUdara = "Aman";
    }

    Serial.println("Nilai: " + String(sensorValue) + " | Status: " + statusUdara);

    // Kirim data telemetri ke ThingsBoard
    String payload = "{\"kadar_gas\": " + String(sensorValue) + ", \"status\": \"" + statusUdara + "\"}";
    client.publish("v1/devices/me/telemetry", payload.c_str());
  }
}
