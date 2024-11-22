#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Configurações Wi-Fi
const char* ssid = "Wokwi-GUEST"; 
const char* password = "";

// Configurações do HiveMQ Cloud
const char* mqtt_server = "f72bdc9f32234a82bb5a023b126e85a1.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883;                         
const char* mqtt_user = "EnergyUser";               
const char* mqtt_password = "Admin123";             
const char* mqttTopic = "EnergySaver/Movement";

WiFiClientSecure espClient; // Cliente seguro para TLS
PubSubClient client(espClient);

#define POT_PIN   34   
#define PIR_PIN   27   
#define LED_PIN   18   

void setup() {
  pinMode(LED_PIN, OUTPUT);        
  pinMode(PIR_PIN, INPUT);         
  Serial.begin(115200);            

  // Configura cliente TLS
  espClient.setInsecure(); 

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("WiFi conectado!");

  // Conecta ao MQTT
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    Serial.println("Conectando ao broker MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado ao MQTT!");
    } else {
      Serial.print("Falha ao conectar. Código de erro: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void loop() {
  // Lê o valor do potenciômetro
  int potValue = analogRead(POT_PIN);

  // Lê o estado do sensor de movimento
  int motionDetected = digitalRead(PIR_PIN);

  // Mapeia o valor do potenciômetro para o intervalo de 0 a 255 (PWM)
  int brightness = map(potValue, 0, 4095, 0, 255);

  // Ajusta o LED
  int ledBrightness;
  if (motionDetected == HIGH) {
    ledBrightness = 255;  // 100% de brilho
    Serial.println("Movimento detectado! LED em 100% de brilho.");
  } else {
    ledBrightness = (int)(brightness * 0.6);  
    Serial.println("Sem movimento. LED em 60% de brilho.");
  }
  analogWrite(LED_PIN, ledBrightness);

  // Publica os dados no tópico MQTT
  if (client.connected()) {
    // Cria uma string JSON com os dados
    String payload = "{";
    payload += "\"motionDetected\":" + String(motionDetected) + ",";
    payload += "\"brightness\":" + String(ledBrightness);
    payload += "}";

    // Publica no tópico
    client.publish(mqttTopic, payload.c_str());
    Serial.println("Dados publicados no MQTT: " + payload);
  } else {
    Serial.println("Desconectado do MQTT. Tentando reconectar...");
    while (!client.connected()) {
      if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
        Serial.println("Reconectado ao MQTT!");
      } else {
        Serial.print("Falha ao reconectar. Código de erro: ");
        Serial.println(client.state());
        delay(2000);
      }
    }
  }

  delay(1000);  
}
