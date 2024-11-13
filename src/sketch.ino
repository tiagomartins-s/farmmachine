#include <DHT.h>
#include <ctime>

#define BUTTON_PHOSPHORUS 23    // Pino do botão de Fósforo
#define BUTTON_POTASSIUM 22     // Pino do botão de Potássio
#define LDR_PIN 34              // Pino analógico do LDR
#define DHT_PIN 19              // Pino do sensor DHT22
#define RELAY_PIN 12            // Pino de controle do relé
#define COLETA_INTERVALO 5000   // Intervalo de coleta em milissegundos (5 segundos)

// Inicializa o sensor DHT
DHT dht(DHT_PIN, DHT22);  // Pino e tipo de sensor DHT (DHT22)

unsigned long ultimoTempoColeta = 0; // Variável para armazenar o último tempo de coleta
int ultimaLeituraLDR = 0;            // Variável para armazenar a última leitura do LDR
float pH = 7.0;                      // Valor inicial do pH (neutro)
bool reléStatus = false;             // Status do relé (false = desligado, true = ligado)
String motivoAcionamento = "";       // Motivo do acionamento do relé (inicialmente vazio)
int id_coleta = 1;                   // Identificador de coleta

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PHOSPHORUS, INPUT_PULLUP);
  pinMode(BUTTON_POTASSIUM, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT); // Configura o pino do relé como saída

  dht.begin();  // Inicializa o sensor DHT

  // Define a data e hora inicial manualmente (Ano, Mês, Dia, Hora, Minuto, Segundo)
  struct tm timeinfo;
  timeinfo.tm_year = 2024 - 1900; // Ano - 1900
  timeinfo.tm_mon = 10 - 1;       // Mês (0 = Janeiro)
  timeinfo.tm_mday = 9;           // Dia do mês
  timeinfo.tm_hour = 9;           // Hora
  timeinfo.tm_min = 0;            // Minuto
  timeinfo.tm_sec = 0;            // Segundo

  // Configura o RTC do ESP32
  time_t t = mktime(&timeinfo);
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);  // Define o RTC para a data/hora inicial

  // Cabeçalho das colunas
  Serial.println("ID Coleta   Item Coletado          Valor da Coleta   Data/Hora da Coleta        Status Rele  Motivo Acionamento");
  Serial.println("--------    -------------          ---------------   ---------------------      -----------  ------------------");
}

void loop() {
  int phosphorusButtonState = digitalRead(BUTTON_PHOSPHORUS);
  int potassiumButtonState = digitalRead(BUTTON_POTASSIUM);

  if (phosphorusButtonState == LOW) {
    int phosphorusValue = random(10, 101);
    printData("Fósforo", phosphorusValue);
    delay(1000); // Pausa para evitar leituras múltiplas
  }

  if (potassiumButtonState == LOW) {
    int potassiumValue = random(10, 101);
    printData("Potássio", potassiumValue);
    delay(1000); // Pausa para evitar leituras múltiplas
  }

  // Verifica se já passaram 5 segundos desde a última coleta
  unsigned long tempoAtual = millis();
  if (tempoAtual - ultimoTempoColeta >= COLETA_INTERVALO) {
    ultimoTempoColeta = tempoAtual;  // Atualiza o tempo da última coleta

    // Leitura do sensor LDR
    int ldrValue = analogRead(LDR_PIN);
    float lightIntensity = map(ldrValue, 0, 4095, 14, 0); // Mapeamento de LDR para pH

    // Leitura do sensor DHT22 (Temperatura e Umidade)
    float temperatura = dht.readTemperature(); // Temperatura em Celsius
    float umidade = dht.readHumidity();        // Umidade relativa

    // Verifica se houve erro na leitura do DHT22
    if (isnan(temperatura) || isnan(umidade)) {
      Serial.println("Falha na leitura do DHT22!");
      return;
    }

    // Exibe os dados de LDR
    printData("pH", lightIntensity);

    // Exibe os dados de Temperatura e Umidade, incluindo data e hora da coleta
    printData("Temperatura", temperatura);
    printData("Umidade", umidade);

    // Verifica as condições para acionar o relé (todas as 3 condições devem ser atendidas)
    motivoAcionamento = ""; // Deixa o motivo vazio quando o relé não for acionado
    if (lightIntensity > 10 && temperatura > 35 && umidade < 50) {
      reléStatus = true;
      motivoAcionamento = "pH acima de 10, Temperatura acima de 35°C e Umidade abaixo de 50%";
      digitalWrite(RELAY_PIN, HIGH); // Aciona o relé
      Serial.println("Relé acionado: pH acima de 10, Temperatura acima de 35°C e Umidade abaixo de 50%");
    } else {
      reléStatus = false;
      motivoAcionamento = "";  // Deixa o motivo vazio quando o relé não for acionado
      digitalWrite(RELAY_PIN, LOW); // Desliga o relé
      Serial.println("Relé desligado");
    }

    // Exibe os dados com o status do relé e o motivo do acionamento
    printData("pH", lightIntensity);
    printData("Temperatura", temperatura);
    printData("Umidade", umidade);
  }
}

void printData(const char* nutriente, float valor) {
  // Obtém o tempo atual do RTC
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter a data e hora");
    return;
  }

  // Exibe a informação no formato de colunas no monitor serial, com o ID de coleta
  Serial.printf("%-10d%-20s%-20.2f%-4d-%02d-%02d %02d:%02d:%02d  %-4d%-20s\n",
                id_coleta++,
                nutriente,
                valor,
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                reléStatus, motivoAcionamento.isEmpty() ? "" : motivoAcionamento.c_str());
}
