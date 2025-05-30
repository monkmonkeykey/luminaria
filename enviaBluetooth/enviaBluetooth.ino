#include <bluefruit.h>

BLEUart bleuart;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Bluefruit.begin();
  Bluefruit.setName("XIAO-EMISOR");
  bleuart.begin();

  // Anunciar el servicio
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.start(0); // 0 = indefinidamente

  Serial.println("Emisor BLE UART iniciado.");
}


void loop() {
  bleuart.print("Hola desde el emisor\n");
  Serial.println("Mensaje enviado");
  delay(1000);
}
