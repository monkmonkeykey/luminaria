#include <Arduino.h>
#include <NimBLEDevice.h>
#include <math.h>

// === BLE UUIDs ===
#define SERVICE_UUID "DEAD"
#define CHARACTERISTIC_UUID "BEEF"

NimBLECharacteristic* pCharacteristic;

// ==== Movimiento simulado ====
float accelMag = 0.0;
float baseMag = 1.0;    // valor base en reposo
float smoothMag = 1.0;  // promedio móvil
float alpha = 0.05;     // factor de suavizado

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("🔧 Iniciando EMISOR BLE con datos simulados");

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
  // Generar valores aleatorios simulados en el rango ~0.9 a ~2.0
  float ax = 0.9 + random(-10, 100) / 100.0;  // Entre 0.8 y 1.9
  float ay = 0.9 + random(-10, 100) / 100.0;
  float az = 1.0 + random(-10, 100) / 100.0;

  accelMag = sqrt(ax * ax + ay * ay + az * az);
  smoothMag = alpha * accelMag + (1 - alpha) * smoothMag;

  float delta = fabs(accelMag - smoothMag);

  // Clasificación por delta
  String nivel = "quieto";
  if (delta > 0.05 && delta <= 0.15) nivel = "leve";
  else if (delta > 0.15 && delta <= 0.4) nivel = "moderado";
  else if (delta > 0.4) nivel = "intenso";

  // Mostrar en consola
  Serial.println("──── MOVIMIENTO SIMULADO ────");
  Serial.printf("ax=%.2f ay=%.2f az=%.2f | mag=%.3f | Δ=%.3f | nivel=%s\n",
                ax, ay, az, accelMag, delta, nivel.c_str());

  // Enviar por BLE
  pCharacteristic->setValue((uint8_t*)&accelMag, sizeof(accelMag));
  pCharacteristic->notify();

  delay(3000);
}
