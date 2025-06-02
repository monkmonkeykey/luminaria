#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

const char* ssid_ap = "ESP_OSC_AP";
const char* password_ap = "12345678";  // mínimo 8 caracteres

WiFiUDP Udp;
const int localPort = 8000;

void setup() {
  Serial.begin(115200);

  // Crear Access Point
  WiFi.softAP(ssid_ap, password_ap);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  Udp.begin(localPort);
}

void receiveOSC() {
  OSCMessage msg;
  int size;
  if ((size = Udp.parsePacket()) > 0) {
    while (size--) msg.fill(Udp.read());

    if (!msg.hasError()) {
      msg.dispatch("/valor", [](OSCMessage &msg) {
        float valor = msg.getFloat(0);
        Serial.print("Valor recibido: ");
        Serial.println(valor);
      });
    }
  }
}

void loop() {
  receiveOSC();
}
