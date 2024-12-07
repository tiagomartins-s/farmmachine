#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ctime>

#define BUTTON_PHOSPHORUS 23    // Pino do botão de Fósforo
#define BUTTON_POTASSIUM 22     // Pino do botão de Potássio
#define LDR_PIN 34              // Pino analógico do LDR
#define DHT_PIN 19              // Pino do sensor DHT22
#define RELAY_PIN 12            // Pino de controle do relé
#define COLETA_INTERVALO 5000   // Intervalo de coleta em milissegundos (5 segundos)

// Inicializa o sensor DHT
DHT dht(DHT_PIN, DHT22);  // Pino e tipo de sensor DHT (DHT22)

// Inicializa o display LCD (endereço 0x27, tamanho 16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimoTempoColeta = 0;  // Variável para armazenar o último tempo de coleta (unsigned long é eficiente para tempos)
byte ultimaLeituraLDR = 0;            // Variável para armazenar a última leitura do LDR (byte porque o valor é pequeno)
int pH = 7;                          // Valor inicial do pH (não precisa de float, int é suficiente)
bool reléStatus = false;             // Status do relé (false = desligado, true = ligado)
char motivoAcionamento[50] = "";     // Motivo do acionamento do relé (usando array de char em vez de String para economizar memória)
byte id_coleta = 1;                   // Identificador de coleta (byte porque o número de coletas não vai ultrapassar 255)

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PHOSPHORUS, INPUT_PULLUP);
  pinMode(BUTTON_POTASSIUM, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT); // Configura o pino do relé como saída

  dht.begin();  // Inicializa o sensor DHT

  // Inicializa a comunicação I2C no ESP32
  Wire.begin(5, 18); // SDA no pino 5, SCL no pino 18

  // Configura o LCD
  lcd.begin(16, 2);  // Inicializa o LCD com 16 colunas e 2 linhas
  lcd.setBacklight(1);  // Liga o retroiluminado do LCD

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
  Serial.println("ID_Coleta   Item_Coletado          Valor_Coleta   Data/Hora_Coleta        Status_Rele  Motivo_Acionamento");
  Serial.println("--------    -------------          ---------------   ---------------------      -----------  ------------------");
}

void loop() {
  byte phosphorusButtonState = digitalRead(BUTTON_PHOSPHORUS);  // Usando byte para economizar memória
  byte potassiumButtonState = digitalRead(BUTTON_POTASSIUM);     // Usando byte para economizar memória

  if (phosphorusButtonState == LOW) {
    byte phosphorusValue = random(10, 101);  // Usando byte para o valor de fósforo
    printData("Fósforo", phosphorusValue);
    displayLCD("Fósforo", phosphorusValue);  // Exibe no LCD
    delay(1000); // Pausa para evitar leituras múltiplas
  }

  if (potassiumButtonState == LOW) {
    byte potassiumValue = random(10, 101);  // Usando byte para o valor de potássio
    printData("Potássio", potassiumValue);
    displayLCD("Potássio", potassiumValue);  // Exibe no LCD
    delay(1000); // Pausa para evitar leituras múltiplas
  }

  // Verifica se já passaram 5 segundos desde a última coleta
  unsigned long tempoAtual = millis();
  if (tempoAtual - ultimoTempoColeta >= COLETA_INTERVALO) {
    ultimoTempoColeta = tempoAtual;  // Atualiza o tempo da última coleta

    // Leitura do sensor LDR
    int ldrValue = analogRead(LDR_PIN);
    int lightIntensity = map(ldrValue, 0, 4095, 14, 0); // Usando int ao invés de float para economizar memória

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
    displayLCD("pH", lightIntensity);  // Exibe no LCD

    // Exibe os dados de Temperatura e Umidade, incluindo data e hora da coleta
    printData("Temperatura", temperatura);
    displayLCD("Temperatura", temperatura);  // Exibe no LCD
    printData("Umidade", umidade);
    displayLCD("Umidade", umidade);  // Exibe no LCD

    // Verifica as condições para acionar o relé (todas as 3 condições devem ser atendidas)
    motivoAcionamento[0] = '\0';  // Deixa o motivo vazio quando o relé não for acionado
    if (lightIntensity > 10 && temperatura > 35 && umidade < 50) {
      reléStatus = true;
      strncpy(motivoAcionamento, "Temp>35-Umidade<50", sizeof(motivoAcionamento) - 1); // Usando strncpy para evitar overflow
      digitalWrite(RELAY_PIN, HIGH); // Aciona o relé
    } else {
      reléStatus = false;
      motivoAcionamento[0] = '\0';  // Deixa o motivo vazio quando o relé não for acionado
      digitalWrite(RELAY_PIN, LOW); // Desliga o relé
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

  // Formata a data e hora no formato "YYYY-MM-DD-HH:MM:SS"
  char dataHora[20];
  snprintf(dataHora, sizeof(dataHora), "%04d-%02d-%02d-%02d:%02d:%02d",
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  // Exibe a informação no formato de colunas no monitor serial, com o ID de coleta
  Serial.printf("%-10d%-20s%-20.2f%-20s%-12s%-20s\n",
                id_coleta, nutriente, valor, dataHora,
                reléStatus ? "Ligado" : "Desligado", motivoAcionamento);
  id_coleta++;  // Incrementa o ID de coleta para o próximo
}

void displayLCD(const char* nutriente, float valor) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(nutriente);
  lcd.setCursor(0, 1);
  lcd.print(valor);
}