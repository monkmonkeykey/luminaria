#include <Arduino.h>
#include <NimBLEDevice.h>
#include <DFRobot_BMI160.h>
#include <math.h>

// === BMI160 ===
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;
String nivelAnterior = "";

// === BLE UUIDs ===
#define SERVICE_UUID "DEAD"
#define CHARACTERISTIC_UUID "BEEF"

NimBLECharacteristic* pCharacteristic;

// ==== Movimiento ====
float accelMag = 0.0;
float smoothMag = 1.0;
float alpha = 0.05;

// Control de envío
unsigned long ultimaNotificacion = 0;
const unsigned long intervaloEnvio = 300; // ms

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("🔧 Iniciando EMISOR BLE con sensor BMI160");

  if (bmi160.softReset() != BMI160_OK || bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("❌ Error inicializando sensor BMI160");
    while (1);
  }

  Serial.println("✅ Sensor BMI160 listo");

  NimBLEDevice::init("ESP32_Emisor");
  NimBLEServer* pServer = NimBLEDevice::createServer();
  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    NIMBLE_PROPERTY::NOTIFY);

  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("📡 Anunciando como periférico BLE...");
}

void loop() {
  int16_t data[6] = {0};
  if (bmi160.getAccelGyroData(data) == 0) {
    float ax = data[3] / 16384.0;
    float ay = data[4] / 16384.0;
    float az = data[5] / 16384.0;

    accelMag = sqrt(ax * ax + ay * ay + az * az);
    smoothMag = alpha * accelMag + (1 - alpha) * smoothMag;
    float delta = fabs(accelMag - smoothMag);

    String nivel = "quieto";
    if (delta > 0.05 && delta <= 0.15) nivel = "leve";
    else if (delta > 0.15 && delta <= 0.4) nivel = "moderado";
    else if (delta > 0.4) nivel = "intenso";

    Serial.println("──── MOVIMIENTO ────");
    Serial.printf("ax=%.2f ay=%.2f az=%.2f | mag=%.3f | Δ=%.3f | nivel=%s\n",
                  ax, ay, az, accelMag, delta, nivel.c_str());

    // Control de envío BLE
    unsigned long ahora = millis();
    if ((nivel != nivelAnterior || ahora - ultimaNotificacion >= intervaloEnvio)) {
      pCharacteristic->setValue((uint8_t*)&accelMag, sizeof(accelMag));
      pCharacteristic->notify();
      ultimaNotificacion = ahora;
      nivelAnterior = nivel;
    }
  }

  delay(20);  // Pequeño delay para no sobrecargar el loop
}
