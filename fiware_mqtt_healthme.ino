#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "46.17.108.113"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/healthme002/cmd"; // Tópico MQTT de escuta
const char* default_TOPICO_PUBLISH_2 = "/TEF/healthme002/attrs/t"; // Tópico MQTT de envio de informações para Broker
const char* default_ID_MQTT = "healthme_002"; // ID MQTT
const int DHTPIN = 2; // Pino de dados do DHT22
const int DHTTYPE = DHT22; // Tipo de sensor DHT (DHT22 neste exemplo)

// Declaração da variável para o prefixo do tópico
const char* topicPrefix = "healthme002";

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);

WiFiClient espClient;
PubSubClient MQTT(espClient);
DHT dht(DHTPIN, DHTTYPE);

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    // MQTT.setCallback(mqtt_callback); // Se necessário, adicione uma função de callback
}

void setup() {
    initSerial();
    initWiFi();
    initMQTT();
    delay(5000); // Atraso para garantir a inicialização do ESP32
}

void loop() {
    VerificaConexoesWiFIEMQTT();
    handleTemperature();
    MQTT.loop();
    delay(3000); // Atraso de 5 segundos entre as leituras
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            //MQTT.subscribe(TOPICO_SUBSCRIBE); // Se necessário, subscreva a um tópico
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleTemperature() {
    float temperatura = dht.readTemperature();
    if (!isnan(temperatura)) {
        // Envie a temperatura para o tópico MQTT correspondente
        String mensagem = String(temperatura);
        Serial.print("Valor da temperatura: ");
        Serial.println(mensagem.c_str());
        MQTT.publish(TOPICO_PUBLISH_2, mensagem.c_str());
    } else {
        Serial.println("Erro ao ler a temperatura do sensor DHT22.");
    }
}
