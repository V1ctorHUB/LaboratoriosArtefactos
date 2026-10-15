#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

const char* WIFI_SSID = "TU_WIFI_SSID";
const char* WIFI_PASSWORD = "TU_WIFI_PASSWORD";


// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

// IP de la computadora donde está ejecutándose Mosquitto.
// En tu prueba actual de Windows es 192.168.0.5.
const char* MQTT_SERVER = "10.110.0.25";
const int MQTT_PORT = 1883;

// Topics esperados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";


// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// DHT11
#define DHT_PIN 4
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// LDR
#define LDR_PIN 32


// ======================================================
// OBJETOS WIFI Y MQTT
// ======================================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// ======================================================
// TEMPORIZACIÓN
// ======================================================

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;


// ======================================================
// CONECTAR A WIFI
// ======================================================

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}


// ======================================================
// CONECTAR A MQTT
// ======================================================

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String((uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF), HEX);

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {

  // ----------------------------------------------------
  // DHT11
  // ----------------------------------------------------

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // Verificar si el DHT11 devolvió una lectura válida.
  if (isnan(temperatura) || isnan(humedad)) {

    Serial.println("Error al leer el DHT11");

  } else {

    char temperaturaTexto[10];
    char humedadTexto[10];

    dtostrf(temperatura, 1, 1, temperaturaTexto);
    dtostrf(humedad, 1, 1, humedadTexto);

    // true = mensaje retained
    mqttClient.publish(
      TOPIC_TEMPERATURA,
      temperaturaTexto,
      true
    );

    mqttClient.publish(
      TOPIC_HUMEDAD,
      humedadTexto,
      true
    );

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");

    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");
  }


  // ----------------------------------------------------
  // LDR
  // ----------------------------------------------------

  int lecturaLDR = analogRead(LDR_PIN);

  // ESP32 ADC: aproximadamente 0 - 4095.
  int porcentajeLuz = map(
    lecturaLDR,
    0,
    4095,
    0,
    100
  );

  porcentajeLuz = constrain(porcentajeLuz, 0, 100);

  char luzTexto[10];

  itoa(porcentajeLuz, luzTexto, 10);

  mqttClient.publish(
    TOPIC_LUZ,
    luzTexto,
    true
  );

  Serial.print("LDR ADC: ");
  Serial.print(lecturaLDR);

  Serial.print(" | Luz: ");
  Serial.print(porcentajeLuz);
  Serial.println(" %");

  Serial.println("------------------------------");
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  dht.begin();

  pinMode(LDR_PIN, INPUT);

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();


  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}