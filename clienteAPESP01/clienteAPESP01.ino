#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const char* ssid = "ESP_OSC_AP";
const char* password = "12345678";

const IPAddress servidorIP(192, 168, 4, 1);  // IP del servidor (ESP Access Point)
const int servidorPuerto = 8000;

WiFiUDP Udp;

void setup() {
  Serial.begin(9600); // Velocidad igual a la del dispositivo conectado por TX/RX
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Udp.begin(8888);
}

void loop() {
  if (Serial.available()) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    if (entrada.length() > 0) {
      float valor = entrada.toFloat();  // suponiendo que los datos son números

      OSCMessage msg("/serialdata");
      msg.add(valor);

      Udp.beginPacket(servidorIP, servidorPuerto);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
    }
  }
}
