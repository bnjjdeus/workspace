#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include "DHT.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"

// --- CONFIGURACIÓN DE RED ---
const char* ssid = "nashe";
const char* password = "weonklxd";
const char* urlAPI_recepcion = "http://10.228.89.38:8000/recepcion";
const char* urlAPI_clima = "http://10.228.89.38:8000/clima";

// --- CONFIGURACIÓN DE PINES Y SENSORES ---
// Pantalla OLED (SH1106, 128x64)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Sensor de Temperatura y Humedad DHT22
#define DHT_PIN 14
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Pines de los Relés
#define RELAY_WINDOW_PIN 26 // Asignado a una ventana, por ejemplo
#define RELAY_FAN_PIN    27 // Asignado a un ventilador

// --- NUEVO: Umbral de temperatura para encender el ventilador ---
const float TEMP_UMBRAL = 28.0; // El ventilador se encenderá sobre esta temperatura

// --- VARIABLES GLOBALES ---
StaticJsonDocument<512> recepcion_clima;
StaticJsonDocument<200> envio;

// --- NUEVA FUNCIÓN: Para asegurar la conexión WiFi ---
void ensureWifiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Conexión WiFi perdida. Reintentando...");
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.setCursor(0, 10);
        u8g2.print("Reconectando WiFi...");
        u8g2.sendBuffer();

        WiFi.begin(ssid, password);

        int retries = 0;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            retries++;
            if (retries > 30) { // Si después de 15 segundos no conecta, reinicia el ESP32
                 Serial.println("\nNo se pudo reconectar. Reiniciando...");
                 ESP.restart();
            }
        }

        Serial.println("\n¡Reconectado a la red WiFi!");
        Serial.print("Nueva Dirección IP: ");
        Serial.println(WiFi.localIP());

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.setCursor(0, 10);
        u8g2.print("Reconectado!");
        u8g2.setCursor(0, 25);
        u8g2.print(WiFi.localIP());
        u8g2.sendBuffer();
        delay(200);
    }
}


void setup() {
    Serial.begin(115200);
    dht.begin();
    u8g2.begin();

    // --- NUEVO: Inicialización de los pines de los relés ---
    pinMode(RELAY_WINDOW_PIN, OUTPUT);
    pinMode(RELAY_FAN_PIN, OUTPUT);
    // Aseguramos que los relés comiencen apagados
    // NOTA: La mayoría de módulos de relé se apagan con HIGH (ALTO) y se encienden con LOW (BAJO).
    // Si tu relé funciona al revés, cambia HIGH por LOW y viceversa.
    digitalWrite(RELAY_WINDOW_PIN, LOW);
    digitalWrite(RELAY_FAN_PIN, LOW);
    
    // Inicia la conexión WiFi por primera vez
    ensureWifiConnection(); 
}

void loop() {
    // 1. --- NUEVO: Al inicio del loop, nos aseguramos de tener conexión ---
    ensureWifiConnection();

    // 2. Lee los datos del sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    bool fan_status = false; // Para saber si el ventilador está encendido

    // Validar lectura del sensor DHT
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Error al leer del sensor DHT!");
        delay(200);
        return; // Salta esta iteración del loop si hay error
    }

    // 3. --- NUEVO: Lógica de control del relé del ventilador ---
    if (temperature > TEMP_UMBRAL) {
        Serial.println("Temperatura alta. Encendiendo ventilador.");
        digitalWrite(RELAY_FAN_PIN, LOW); // Enciende el ventilador
        fan_status = true;
    } else {
        Serial.println("Temperatura normal. Apagando ventilador.");
        digitalWrite(RELAY_FAN_PIN, HIGH); // Apaga el ventilador
        fan_status = false;
    }

    // 4. Envía los datos del sensor a la API
    HTTPClient client;
    client.begin(urlAPI_recepcion);
    client.addHeader("Content-Type", "application/json");

    envio["origen"] = "ESP32";
    envio["temperatura"] = temperature;
    envio["humedad"] = humidity;
    envio["ventana"] = (digitalRead(RELAY_WINDOW_PIN) == LOW); // Enviará true si está encendido (LOW), sino false
    envio["ventilador"] = fan_status; // fan_status ya es un booleano (true/false)

    String jsonToSend;
    serializeJson(envio, jsonToSend);

    int httpCode = client.POST(jsonToSend);
    if (httpCode > 0) {
        Serial.printf("Datos enviados, código de respuesta: %d\n", httpCode);
    } else {
        Serial.printf("Error al enviar datos: %s\n", client.errorToString(httpCode).c_str());
    }
    client.end();

    delay(200); // Pequeña pausa

    // 5. Obtiene los datos del clima y los muestra en la pantalla
    HTTPClient clientClima;
    clientClima.begin(urlAPI_clima);

    int httpCodeClima = clientClima.GET();
    if (httpCodeClima > 0) {
        String responseClima = clientClima.getString();
        deserializeJson(recepcion_clima, responseClima);

        const char* temp_clima = recepcion_clima["temperatura"];
        const char* estado_dia = recepcion_clima["estado_dia"];

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        u8g2.setCursor(0, 10);
        u8g2.print("Temp Ext: ");
        u8g2.print(temp_clima);
        
        u8g2.setCursor(0, 22);
        u8g2.print("Temp Int: ");
        u8g2.print(temperature, 1); // Muestra con 1 decimal
        u8g2.print("C");
        
        u8g2.setCursor(0, 34);
        u8g2.print("Hum Int: ");
        u8g2.print(humidity, 1);
        u8g2.print("%");
        
        u8g2.setCursor(0, 46);
        u8g2.print("Estado: ");
        u8g2.print(estado_dia);

        // --- NUEVO: Mostrar estado del ventilador en la pantalla ---
        u8g2.setCursor(0, 58);
        u8g2.print("Ventilador: ");
        u8g2.print(fan_status ? "ON" : "OFF"); // Operador ternario: si fan_status es true, imprime "ON", si no, "OFF"
        
        u8g2.sendBuffer();
    } else {
        Serial.printf("Error al obtener el clima: %s\n", clientClima.errorToString(httpCodeClima).c_str());
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.setCursor(0, 10);
        u8g2.print("Error API Clima");
        u8g2.sendBuffer();
    }
    clientClima.end();


    delay(100); // Espera 10 segundos antes del siguiente ciclo
}