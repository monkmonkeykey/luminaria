#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const char* ssid = "Taller";        // SSID del AP creado por el servidor
const char* password = "AscoSpock1@";

const IPAddress servidorIP(192, 168, 15, 24);  // IP por defecto de ESP Access Point
const int servidorPuerto = 8000;

WiFiUDP Udp;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando al servidor ESP...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado al AP.");
  Serial.println(WiFi.localIP());

  Udp.begin(8888);  // Puerto local del cliente
}

void loop() {
  float valor = random(100) / 10.0;

  OSCMessage msg("/valor");
  msg.add(valor);

  Udp.beginPacket(servidorIP, servidorPuerto);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();

  Serial.print("Enviado: ");
  Serial.println(valor);

  delay(1000);
}
