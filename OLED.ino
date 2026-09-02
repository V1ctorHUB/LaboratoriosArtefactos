#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

const int pinLM35 = A0;

#define i2c_Address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

void setup() {
  Serial.begin(9600);
  Wire.begin();

  delay(250);

  if (!display.begin(i2c_Address, true)) {
    Serial.println("ERROR: OLED no encontrada");
    while (1);
  }

  Serial.println("OLED funcionando correctamente");
}

void loop() {
  // 1. Lectura del pin analógico
  int lectura = analogRead(pinLM35);

  // 2. Conversión a grados Celsius (resolución ADC de 10 bits: 5V / 1024 pasos * 100)
  float temperatura = (lectura * (5.0 / 1024.0)) * 100.0;

  // Monitor Serial
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  // 3. Renderizado en pantalla OLED
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // Título
  display.setTextSize(1);
  display.setCursor(15, 8);
  display.println("TERMOMETRO LM35");

  // Valor de temperatura
  display.setTextSize(2);
  display.setCursor(15, 30);
  display.print(temperatura, 1);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(248);            // Símbolo de grados (°)
  display.setTextSize(2);
  display.print("C");

  display.display();

  delay(1000);
}