#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <DFRobot_BMI160.h>
#include <Adafruit_NeoPixel.h>

// ==== WiFi ====
const char* ssid = "Taller";
const char* password = "AscoSpock1@";

// ==== OSC ====
const char* outIp = "192.168.15.24";
const unsigned int outPort = 8000;
WiFiUDP Udp;

// ==== Sensor ====
DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

// ==== Neopixel ====
#define PIN_NEOPIXEL 10
#define NUM_PIXELS 64
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ==== Valores iniciales ====
float minMag = 9999.0;
float maxMag = -9999.0;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: ");
  Serial.println(WiFi.localIP());

  // Inicialización del sensor
  if (bmi160.softReset() != BMI160_OK) {
    Serial.println("Reset falló");
    while (1);
  }

  if (bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("Init falló");
    while (1);
  }

  // Inicializar Neopixel
  strip.begin();
  strip.show(); // Apaga todos los píxeles
}

// === Determinar nivel de movimiento ===
String nivelMovimiento(float mag) {
  if (mag < 1.2) return "bajo";
  else if (mag < 3.0) return "medio";
  else return "alto";
}

// === Mostrar color en la matriz según el nivel ===
void mostrarColorNivel(String nivel) {
  uint32_t color;

  if (nivel == "bajo") {
    color = strip.Color(0, 0, 255); // Azul
  } else if (nivel == "medio") {
    color = strip.Color(255, 255, 0); // Amarillo
  } else {
    color = strip.Color(255, 0, 0); // Rojo
  }

  for (int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void loop() {
  int16_t accelGyro[6] = {0};

  if (bmi160.getAccelGyroData(accelGyro) == 0) {
    // Datos de giroscopio y acelerómetro
    float gx = accelGyro[0] * 3.14 / 180.0;
    float gy = accelGyro[1] * 3.14 / 180.0;
    float gz = accelGyro[2] * 3.14 / 180.0;
    float ax = accelGyro[3] / 16384.0;
    float ay = accelGyro[4] / 16384.0;
    float az = accelGyro[5] / 16384.0;

    // Magnitud de aceleración
    float accelMag = sqrt(ax * ax + ay * ay + az * az);

    // Actualizar mínimos y máximos
    if (accelMag < minMag) minMag = accelMag;
    if (accelMag > maxMag) maxMag = accelMag;

    String nivel = nivelMovimiento(accelMag);

    // === OSC: Datos de movimiento ===
    OSCMessage msg("/imu/data");
    msg.add(gx); msg.add(gy); msg.add(gz);
    msg.add(ax); msg.add(ay); msg.add(az);
    msg.add(accelMag);

    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();

    // === OSC: Nivel de movimiento ===
    OSCMessage estado("/imu/nivel");
    estado.add(nivel.c_str());
    Udp.beginPacket(outIp, outPort);
    estado.send(Udp);
    Udp.endPacket();
    estado.empty();

    // === Mostrar en Neopixel ===
    mostrarColorNivel(nivel);

    // === Serial Debug ===
    Serial.printf("Gyro: %.2f %.2f %.2f | Accel: %.2f %.2f %.2f | Mag: %.3f | Nivel: %s | Min: %.3f | Max: %.3f\n",
      gx, gy, gz, ax, ay, az, accelMag, nivel.c_str(), minMag, maxMag);
  } else {
    Serial.println("Error al leer sensor");
  }

  delay(100); // Ajustable
}
