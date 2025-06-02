#include <Arduino.h>
#include <NimBLEDevice.h>

static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect = false;
static uint32_t scanTimeMs = 5000;

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override {
    Serial.println("✅ Conectado al periférico BLE");
  }

  void onDisconnect(NimBLEClient* pClient, int reason) override {
    Serial.printf("❌ Desconectado. Razón: %d. Reiniciando escaneo...\n", reason);
    NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  }
} clientCB;

class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    Serial.printf("🔍 Encontrado: %s\n", advertisedDevice->toString().c_str());

    if (advertisedDevice->isAdvertisingService(NimBLEUUID("DEAD"))) {
      Serial.println("🧠 Servicio 'DEAD' detectado");
      NimBLEDevice::getScan()->stop();
      advDevice = advertisedDevice;
      doConnect = true;
    }
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    Serial.printf("📴 Fin de escaneo (%d). Reiniciando...\n", reason);
    NimBLEDevice::getScan()->start(scanTimeMs, false, true);
  }
};

void notifyCB(NimBLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (length == sizeof(float)) {
    float valor;
    memcpy(&valor, pData, sizeof(float));
    Serial.printf("📥 Float recibido por BLE: %.3f\n", valor);
  } else {
    Serial.println("⚠️ Longitud inesperada de datos BLE.");
  }
}

bool connectToServer() {
  NimBLEClient* pClient = NimBLEDevice::createClient();
  pClient->setClientCallbacks(&clientCB);

  if (!pClient->connect(advDevice)) {
    Serial.println("❌ Falló la conexión");
    NimBLEDevice::deleteClient(pClient);
    return false;
  }

  Serial.printf("🟢 Conectado a: %s\n", pClient->getPeerAddress().toString().c_str());

  NimBLERemoteService* pService = pClient->getService("DEAD");
  if (!pService) {
    Serial.println("⚠️ Servicio 'DEAD' no encontrado");
    pClient->disconnect();
    return false;
  }

  NimBLERemoteCharacteristic* pChar = pService->getCharacteristic("BEEF");
  if (!pChar) {
    Serial.println("⚠️ Característica 'BEEF' no encontrada");
    pClient->disconnect();
    return false;
  }

  if (pChar->canNotify()) {
    pChar->subscribe(true, notifyCB);
    Serial.println("📡 Suscripción a notificaciones activada");
    return true;
  }

  Serial.println("⚠️ La característica no soporta notificaciones");
  pClient->disconnect();
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("🔧 Iniciando cliente BLE con NimBLE 2.3.0");

  NimBLEDevice::init("ESP32_Central");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);  // Máxima potencia TX
  NimBLEScan* pScan = NimBLEDevice::getScan();

  pScan->setScanCallbacks(new ScanCallbacks(), false);
  pScan->setActiveScan(true);
  pScan->start(scanTimeMs, false);
}

void loop() {
  if (doConnect) {
    doConnect = false;
    if (connectToServer()) {
      Serial.println("✅ Conexión y suscripción completa");
    } else {
      Serial.println("❌ Conexión fallida. Reintentando escaneo...");
      NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
  }

  delay(10); // BLE maneja todo en callbacks
}
