#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect = false;
static uint32_t scanTimeMs = 5000;
#define PIN_NEOPIXEL 1
#define NUM_PIXELS 64
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
float valor; 
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

    // Inicializar Neopixel
  strip.begin();
  strip.show(); // Apaga todos los píxeles
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
 mostrarColorNivel();
  delay(10); // BLE maneja todo en callbacks
}


void mostrarColorNivel() {
  uint32_t color;

  if (valor > 0.05 && valor <= 0.15) {
    color = strip.Color(0, 0, 255); // Azul
  } else if (valor > 0.15 && valor <= 0.4) {
    color = strip.Color(255, 255, 0); // Amarillo
  } else if (valor > 0.4){
    color = strip.Color(255, 0, 0); // Rojo
  }

 
  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}