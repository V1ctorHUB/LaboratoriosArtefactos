#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// Pines para los dos sensores táctiles
const int TOUCH_PIN_1 = 4;   // Sensor 1 (T0)
const int TOUCH_PIN_2 = 15;  // Sensor 2 (T3)

// Umbral de activación obtenido en tu prueba
const int UMBRAL_TOUCH = 350;

void setup() {
  Serial.begin(115200);

  // Inicialización del bus I2C en GPIO21 (SDA) y GPIO22 (SCL)
  Wire.begin(21, 22);

  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("Error al iniciar la pantalla OLED");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
}

void loop() {
  // Lectura de ambos sensores
  int valorTouch1 = touchRead(TOUCH_PIN_1);
  int valorTouch2 = touchRead(TOUCH_PIN_2);

  display.clearDisplay();

  // Título fijo superior
  display.setTextSize(1);
  display.setCursor(26, 5);
  display.println("ESP32 TOUCH");

  // Condición 1: Se toca el primer sensor
  if (valorTouch1 < UMBRAL_TOUCH && valorTouch2 >= UMBRAL_TOUCH) {
    display.setCursor(20, 28);
    display.println("Boton 1 Activado");

    // Indicador visual a la izquierda
    display.fillCircle(25, 52, 5, SH110X_WHITE);
  }
  // Condición 2: Se toca el segundo sensor
  else if (valorTouch2 < UMBRAL_TOUCH && valorTouch1 >= UMBRAL_TOUCH) {
    display.setCursor(20, 28);
    display.println("Boton 2 Activado");

    // Indicador visual a la derecha
    display.fillCircle(103, 52, 5, SH110X_WHITE);
  }
  // Condición 3: Ninguno o ambos están en reposo
  else {
    display.setCursor(32, 28);
    display.println("Esperando...");

    // Círculo en posición central de reposo
    display.drawCircle(64, 52, 5, SH110X_WHITE);
  }

  display.display();
  delay(100);
}
