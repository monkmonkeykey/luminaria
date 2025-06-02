#include <Arduino.h>
#include <NimBLEDevice.h>
#include <DFRobot_BMI160.h>
#include <math.h>

// === BMI160 ===
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;
String nivelMovimiento = "";

// === BLE UUIDs ===
#define SERVICE_UUID "DEAD"
#define CHARACTERISTIC_UUID "BEEF"

NimBLECharacteristic* pCharacteristic;
// ==== Movimiento ====
float accelMag = 0.0;
float baseMag = 1.0;    // valor base en reposo
float smoothMag = 1.0;  // promedio móvil
float alpha = 0.05;     // factor de suavizado

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("🔧 Iniciando EMISOR BLE con sensor BMI160");

  // === Sensor BMI160 ===
  if (bmi160.softReset() != BMI160_OK || bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("❌ Error inicializando sensor BMI160");
    while (1)
      ;
  }
  Serial.println("✅ Sensor BMI160 listo");

  // === BLE Peripheral Setup ===
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
  int16_t data[6] = { 0 };
  if (bmi160.getAccelGyroData(data) == 0) {
    // Aceleración
    float ax = data[3] / 16384.0;
    float ay = data[4] / 16384.0;
    float az = data[5] / 16384.0;

    // Magnitud del vector de aceleración
    accelMag = sqrt(ax * ax + ay * ay + az * az);

    // Suavizado exponencial
    smoothMag = alpha * accelMag + (1 - alpha) * smoothMag;

    // Diferencia respecto a base
    float delta = fabs(accelMag - smoothMag);

    // Clasificación por delta
    String nivel = "quieto";
    if (delta > 0.05 && delta <= 0.15) nivel = "leve";
    else if (delta > 0.15 && delta <= 0.4) nivel = "moderado";
    else if (delta > 0.4) nivel = "intenso";

    // Serial Monitor
    Serial.println("──── MOVIMIENTO ────");
    Serial.printf("ax=%.2f ay=%.2f az=%.2f | mag=%.3f | Δ=%.3f | nivel=%s\n",
                  ax, ay, az, accelMag, delta, nivel.c_str());

    // === Notificar al cliente si está suscrito ===
    pCharacteristic->setValue((uint8_t*)&accelMag, sizeof(accelMag));
    pCharacteristic->notify();
  }

  delay(100);
}
