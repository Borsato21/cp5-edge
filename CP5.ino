#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// CONFIG
const char* default_SSID            = "Wokwi-GUEST";
const char* default_PASSWORD        = "";
const char* default_BROKER_MQTT     = "20.48.230.121";   
const int   default_BROKER_PORT     = 1883;

const char* mqtt_user               = ""; 
const char* mqtt_pass               = ""; 

const char* default_TOPICO_SUBSCRIBE= "/TEF/lamp001/cmd";
const char* default_TOPICO_PUBLISH_1= "/TEF/lamp001/attrs";            // estado s|on / s|off
const char* default_TOPICO_PUBLISH_2= "/TEF/lamp001/attrs/l";          // luminosidade
const char* default_TOPICO_PUBLISH_3= "/TEF/lamp001/attrs/h";          // umidade
const char* default_TOPICO_PUBLISH_4= "/TEF/lamp001/attrs/t";          // temperatura
const char* default_TOPICO_PUBLISH_5= "/TEF/lamp001/attrs/alagamento"; // distância (cm)
const char* default_ID_MQTT         = "fiware_001";
const int   default_D4              = 2;    // LED onboard
const char* topicPrefix             = "lamp001";

// PINOS
#define DHTPIN   15
#define DHTTYPE  DHT22
#define LDR_PIN  34
#define TRIG_PIN 5
#define ECHO_PIN 18

// VARS
char* SSID               = (char*)default_SSID;
char* PASSWORD           = (char*)default_PASSWORD;
char* BROKER_MQTT        = (char*)default_BROKER_MQTT;
int   BROKER_PORT        = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE   = (char*)default_TOPICO_SUBSCRIBE;
char* TOPICO_PUBLISH_1   = (char*)default_TOPICO_PUBLISH_1;
char* TOPICO_PUBLISH_2   = (char*)default_TOPICO_PUBLISH_2;
char* TOPICO_PUBLISH_3   = (char*)default_TOPICO_PUBLISH_3;
char* TOPICO_PUBLISH_4   = (char*)default_TOPICO_PUBLISH_4;
char* TOPICO_PUBLISH_5   = (char*)default_TOPICO_PUBLISH_5;
char* ID_MQTT            = (char*)default_ID_MQTT;
int   D4                 = default_D4;

WiFiClient espClient;
PubSubClient MQTT(espClient);
DHT dht(DHTPIN, DHTTYPE);
char EstadoSaida = '0';
unsigned long lastPub = 0;

// HELPERS 
void initSerial(){ Serial.begin(115200); delay(50); }

void reconectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Conectando WiFi");
  WiFi.begin(SSID, PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) { 
    delay(300); Serial.print(".");
    if (millis() - start > 20000) { // timeout 20s
      Serial.println("\nTimeout conectando WiFi, tentando novamente...");
      start = millis();
    }
  }
  Serial.printf("\nWiFi OK - IP: %s\n", WiFi.localIP().toString().c_str());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg; msg.reserve(length);
  for (unsigned int i=0;i<length;i++) msg += (char)payload[i];
  Serial.printf("CMD [%s]: %s\n", topic, msg.c_str());

  String onTopic  = String(topicPrefix) + "@on|";
  String offTopic = String(topicPrefix) + "@off|";
  if (msg == onTopic)  { digitalWrite(D4, HIGH); EstadoSaida = '1'; }
  if (msg == offTopic) { digitalWrite(D4, LOW);  EstadoSaida = '0'; }
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    if (WiFi.status() != WL_CONNECTED) reconectWiFi();
    Serial.printf("Conectando MQTT %s:%d...\n", BROKER_MQTT, BROKER_PORT);
    if (strlen(mqtt_user) == 0) {
      if (MQTT.connect(ID_MQTT)) {
        Serial.println("MQTT conectado (anonimo)");
        MQTT.subscribe(TOPICO_SUBSCRIBE);
      } else {
        Serial.printf("Falha (%d). Retentando...\n", MQTT.state());
        delay(1000);
      }
    } else {
      if (MQTT.connect(ID_MQTT, mqtt_user, mqtt_pass)) {
        Serial.println("MQTT conectado (com auth)");
        MQTT.subscribe(TOPICO_SUBSCRIBE);
      } else {
        Serial.printf("Falha auth (%d). Retentando...\n", MQTT.state());
        delay(1000);
      }
    }
  }
}

void VerificaConexoesWiFIEMQTT() {
  if (WiFi.status() != WL_CONNECTED) reconectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void EnviaEstadoOutputMQTT() {
  if (!MQTT.connected()) return;
  if (EstadoSaida == '1') MQTT.publish(TOPICO_PUBLISH_1, "s|on");
  else                    MQTT.publish(TOPICO_PUBLISH_1, "s|off");
}

void InitOutput() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);
  for (int i=0;i<6;i++){ digitalWrite(D4, !digitalRead(D4)); delay(120); }
}

// SENSORES 
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5m)
  if (dur == 0) return NAN;
  return dur * 0.0343f / 2.0f; // cm
}

void publishSensors() {
  if (!MQTT.connected()) return;

  int rawLdr = analogRead(LDR_PIN);
  int lum = map(rawLdr, 0, 4095, 0, 100);
  char buf[24];
  itoa(lum, buf, 10);
  MQTT.publish(TOPICO_PUBLISH_2, buf);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) { dtostrf(h, 0, 0, buf); MQTT.publish(TOPICO_PUBLISH_3, buf); }
  if (!isnan(t)) { dtostrf(t, 0, 1, buf); MQTT.publish(TOPICO_PUBLISH_4, buf); }

  float d = readDistanceCM();                 // distância detectada
  int alag = isnan(d) ? 0 : (int)round(d);    // envia inteiro em cm
  itoa(alag, buf, 10);
  MQTT.publish(TOPICO_PUBLISH_5, buf);
}

// ------- SETUP / LOOP -------
void setup() {
  initSerial();
  InitOutput();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  dht.begin();
  reconectWiFi();
  initMQTT();
  reconnectMQTT();
  EnviaEstadoOutputMQTT();
}

void loop() {
  VerificaConexoesWiFIEMQTT();
  MQTT.loop();

  unsigned long now = millis();
  if (now - lastPub >= 2000) {
    EnviaEstadoOutputMQTT();
    publishSensors();
    lastPub = now;
  }
}
