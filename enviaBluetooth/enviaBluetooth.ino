#include <bluefruit.h>

BLEUart bleuart;

void setup() {
  Serial.begin(115200);
  Bluefruit.begin();
  Bluefruit.setName("XIAO-Test");
  bleuart.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(bleuart);
  Bluefruit.Advertising.start(0);

  Serial.println("Anunciando BLE UART...");
}

void loop() {
  bleuart.println("Prueba 123");
  Serial.println("Enviado: Prueba 123");
  delay(1000);
}
