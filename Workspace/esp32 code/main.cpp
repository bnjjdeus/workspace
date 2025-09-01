#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "DHT.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"


const char* ssid = "nashe";
const char* password = "weonklxd";
const char* urlAPI_recepcion = "http://10.128.62.180:8000/recepcion"; 


U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#define DHT_PIN 14
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);
#define RELAY_WINDOW_PIN 26
#define RELAY_FAN_PIN    27

bool estadoVentanaFinal = false;
bool estadoVentiladorFinal = false;
unsigned long tiempoAnterior = 0;
const long intervalo = 20000;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  dht.begin();
  pinMode(RELAY_WINDOW_PIN, OUTPUT);
  pinMode(RELAY_FAN_PIN, OUTPUT);
  digitalWrite(RELAY_WINDOW_PIN, LOW);
  digitalWrite(RELAY_FAN_PIN, LOW);

  WiFi.begin(ssid, password);
}

void checkWiFiConnection() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Conexión WiFi perdida. Reconectando...");

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 32, "Reconectando...");
  u8g2.sendBuffer();

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nReconectado!");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 24, "Conectado!");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 40, WiFi.localIP().toString().c_str());
    u8g2.sendBuffer();
    delay(2000); 
  }
}

void loop() {

  checkWiFiConnection();


  if (WiFi.status() == WL_CONNECTED) {
    unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoAnterior >= intervalo) {
      tiempoAnterior = tiempoActual;

      float tempInterior = dht.readTemperature();
      float humInterior = dht.readHumidity();

      if (isnan(humInterior) || isnan(tempInterior)) {
        Serial.println("Error al leer del sensor DHT!");
        return;
      }

      HTTPClient http;
      JsonDocument doc_enviar;
      doc_enviar["origen"] = "ESP32";
      doc_enviar["temperatura"] = tempInterior;
      doc_enviar["humedad"] = humInterior;
      doc_enviar["ventana"] = estadoVentanaFinal;
      doc_enviar["ventilador"] = estadoVentiladorFinal;
      String json_payload;
      serializeJson(doc_enviar, json_payload);

      http.begin(urlAPI_recepcion);
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(json_payload);

      if (httpCode > 0) {
        String response_payload = http.getString();
        JsonDocument doc_recibir;
        deserializeJson(doc_recibir, response_payload);
        
        bool prediccion_ventana = doc_recibir["ventana_pred"];
        bool prediccion_ventilador = doc_recibir["ventilador_pred"];
        float tempExterior = doc_recibir["temp_externa"];
        bool es_dia = doc_recibir["es_dia"];
        
        estadoVentanaFinal = (tempInterior > 30) && (prediccion_ventana == true);
        estadoVentiladorFinal = (tempInterior > 33) && (prediccion_ventilador == true);
        
        digitalWrite(RELAY_WINDOW_PIN, estadoVentanaFinal);
        digitalWrite(RELAY_FAN_PIN, estadoVentiladorFinal);

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.setCursor(0, 8); u8g2.print("Temp Int: "); u8g2.print(tempInterior, 1); u8g2.print("C");
        u8g2.setCursor(0, 18); u8g2.print("Temp Ext: "); u8g2.print(tempExterior, 1); u8g2.print("C");
        u8g2.setCursor(0, 28); u8g2.print("Humedad: "); u8g2.print(humInterior, 1); u8g2.print("%");
        u8g2.setCursor(0, 38); u8g2.print("Estado: "); u8g2.print(es_dia ? "Dia" : "Noche");
        u8g2.setCursor(0, 48); u8g2.print("Ventana: "); u8g2.print(estadoVentanaFinal ? "Abierta" : "Cerrada");
        u8g2.setCursor(0, 58); u8g2.print("Ventilador: "); u8g2.print(estadoVentiladorFinal ? "ON" : "OFF");
        u8g2.sendBuffer();
      } else {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 24, "Error API");
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 40); u8g2.print("Codigo: "); u8g2.print(httpCode);
        u8g2.sendBuffer();
      }
      http.end();
    }
  }
}