#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_NeoPixel.h>
static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect = false;
static uint32_t scanTimeMs = 5000;
#define PIN_NEOPIXEL 2
#define NUM_PIXELS 64
#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ==== Centro y control ====
float t = 0;
float currentSpeed = 0.02;  // velocidad fija
float centerX = MATRIX_WIDTH / 2.0 - 0.5;
float centerY = MATRIX_HEIGHT / 2.0 - 0.5;

// ==== HSB a RGB ====
uint32_t colorHSB(float h, float s, float b) {
  float r = 0, g = 0, bl = 0;
  int i = int(h * 6);
  float f = h * 6 - i;
  float p = b * (1 - s);
  float q = b * (1 - f * s);
  float t = b * (1 - (1 - f) * s);

  switch (i % 6) {
    case 0: r = b, g = t, bl = p; break;
    case 1: r = q, g = b, bl = p; break;
    case 2: r = p, g = b, bl = t; break;
    case 3: r = p, g = q, bl = b; break;
    case 4: r = t, g = p, bl = b; break;
    case 5: r = b, g = p, bl = q; break;
  }

  return strip.Color(r * 255, g * 255, bl * 255);
}
int xyToIndex(int x, int y) {
  return y * MATRIX_WIDTH + x;
}
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
  strip.show();  // Apaga todos los píxeles
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
  delay(10);  // BLE maneja todo en callbacks
}


void mostrarColorNivel() {
  uint32_t color;

  if (valor > 0.05 && valor <= 0.15) {
    color = strip.Color(0, 0, 255);  // Azul
  } else if (valor > 0.15 && valor <= 0.4) {
    color = strip.Color(255, 255, 0);  // Amarillo
  } else if (valor > 0.4) {
    color = strip.Color(255, 0, 0);  // Rojo
  }


  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void respiraNeopixel() {
  t += currentSpeed;
  float waveBrightness = (sin(t) + 1.0) / 2.0;

  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      float dx = x - centerX;
      float dy = y - centerY;
      float dist = sqrt(dx * dx + dy * dy);
      float maxDist = sqrt(centerX * centerX + centerY * centerY);
      float normDist = dist / maxDist;

      float hue = normDist + (t * 0.1);
      if (hue > 1.0) hue -= 1.0;

      float localBrightness = waveBrightness * (1.0 - normDist);
      uint32_t color = colorHSB(hue, 1.0, localBrightness);
      int i = xyToIndex(x, y);
      strip.setPixelColor(i, color);
    }
  }

  strip.show();
  delay(20);
}