#include <NimBLEDevice.h>

class ClienteCB : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) {
    Serial.println("Conectado al periférico.");
  }

  void onDisconnect(NimBLEClient* pClient) {
    Serial.println("Desconectado.");
  }
};

class NotificacionCB : public NimBLERemoteCharacteristicCallbacks {
  void onNotify(NimBLERemoteCharacteristic* pCharacteristic, uint8_t* data, size_t length, bool isNotify) {
    String msg = "";
    for (size_t i = 0; i < length; i++) {
      msg += (char)data[i];
    }
    Serial.print("Recibido: ");
    Serial.println(msg);
  }
};

void setup() {
  Serial.begin(115200);
  NimBLEDevice::init("");

  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setActiveScan(true);
  pScan->setInterval(45);
  pScan->setWindow(15);

  Serial.println("Escaneando...");

  NimBLEScanResults results = pScan->start(5, false);
  for (int i = 0; i < results.getCount(); i++) {
    NimBLEAdvertisedDevice device = results.getDevice(i);
    if (device.haveServiceUUID() && device.getServiceUUID().equals(NimBLEUUID("12345678-1234-1234-1234-1234567890ab"))) {
      Serial.println("Dispositivo encontrado. Conectando...");

      NimBLEClient* pClient = NimBLEDevice::createClient();
      pClient->setClientCallbacks(new ClienteCB());

      if (pClient->connect(&device)) {
        NimBLERemoteService* pService = pClient->getService("12345678-1234-1234-1234-1234567890ab");
        if (pService) {
          NimBLERemoteCharacteristic* pChar = pService->getCharacteristic("abcdefab-1234-1234-1234-abcdefabcdef");
          if (pChar && pChar->canNotify()) {
            pChar->subscribe(true, new NotificacionCB());
            Serial.println("Suscrito a notificaciones.");
            return;
          }
        }
      }

      Serial.println("Error al conectar o descubrir característica.");
    }
  }

  Serial.println("Dispositivo no encontrado.");
}

void loop() {
  delay(1000); // mantener vivo el loop
}
