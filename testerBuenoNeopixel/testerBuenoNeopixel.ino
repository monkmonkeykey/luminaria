#include <Adafruit_NeoPixel.h>

// ==== Neopixel ====
#define PIN_NEOPIXEL 12
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

// ==== Setup ====
void setup() {
  strip.begin();
  strip.show();
}

// ==== Loop ====
void loop() {
  respiraNeopixel();
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
