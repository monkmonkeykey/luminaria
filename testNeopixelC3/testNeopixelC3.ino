#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN        2       // Pin conectado a los NeoPixels
#define NUMPIXELS  64       // Número de LEDs en el anillo
#define MAX_BRIGHTNESS 200 // Valor máximo de brillo (0-255)
#define BREATH_SPEED 0.05  // Velocidad del efecto de respiro

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

float breathAngle = 0;     // Ángulo para la función seno (efecto de respiro)
int hue = 0;               // Hue base para cambiar colores

void setup() {
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  pixels.begin();
}

void loop() {
  float brightness = (sin(breathAngle) + 1.0) / 2.0; // 0.0 - 1.0
  uint8_t scaledBrightness = brightness * MAX_BRIGHTNESS;

  for (int i = 0; i < NUMPIXELS; i++) {
    // Desfase de color por pixel
    int pixelHue = (hue + (i * 10)) % 256;
    pixels.setPixelColor(i, Wheel(pixelHue, scaledBrightness));
  }

  pixels.show();
  delay(20); // Controla la fluidez

  breathAngle += BREATH_SPEED;
  if (breathAngle > TWO_PI) {
    breathAngle -= TWO_PI;
    hue = (hue + 1) % 256; // Cambia ligeramente el color al terminar un ciclo
  }
}

// Convierte HSB (simulado) a RGB
uint32_t Wheel(byte wheelPos, uint8_t brightness) {
  wheelPos = 255 - wheelPos;
  if (wheelPos < 85) {
    return pixels.Color((255 - wheelPos * 3) * brightness / 255, 0, (wheelPos * 3) * brightness / 255);
  }
  if (wheelPos < 170) {
    wheelPos -= 85;
    return pixels.Color(0, (wheelPos * 3) * brightness / 255, (255 - wheelPos * 3) * brightness / 255);
  }
  wheelPos -= 170;
  return pixels.Color((wheelPos * 3) * brightness / 255, (255 - wheelPos * 3) * brightness / 255, 0);
}
