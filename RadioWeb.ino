// Versao WiFI Multi e Manager com correcao BT 14-10-26 
// Mudar setup do TFT - pino 2 (Lolin) ou 19 (Wrover)
// Sucesso usando ESP32  3.0.7 e Audio 3.4.2
#include <ESP_I2S.h>
#include <wav_header.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <WiFiMulti.h>
#include "Audio.h"
#include "examples/320 x 240/Free_Font_Demo/Free_Fonts.h"
#include "BluetoothA2DPSink.h"
#include <HTTPClient.h>
#include "esp_task_wdt.h"  // controle de Watchdog do BT
#include <Preferences.h>

#define canalPraBaixo 4
#define canalPraCima 0  // pino 0 (Lolin) ou 15 (Wrover)
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRCK 26
#define modoAparelho 34
#define AP_NAME "RadioWiFi_Config"
#define AP_PASSWORD ""
#define WIFI_TIMEOUT 120
#define MAX_WIFI_NETWORKS 3

bool mudouEstacao;
int radioTocando;
char* tituloBT = nullptr;
char* artistaBT = nullptr;
char* albumBT = nullptr;

TFT_eSPI tela = TFT_eSPI();
Audio audio;
WiFiManager wifiManager;
WiFiMulti wifiMulti;
HTTPClient http;
I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);
Preferences preferences;

bool connected = true;
unsigned long ultimoCliqueCima = 0;
unsigned long ultimoCliqueBaixo = 0;
const unsigned long debounceDelay = 300;

String conteudoBaixado;
String URLTudo;
String tudo[120];
String nomeRadio[30];
String tipoRadio[30];
String cidadeRadio[30];
String urlRadio[30];
String endTudo = "https://gist.githubusercontent.com/renatoianhez/555bf033ddc5831d16e2602b5c48a909/raw";
int indice = 0;
char textoConvertido[256];  // Buffer para texto convertido

// Estrutura para armazenar redes WiFi
struct WiFiCredentials {
  String ssid;
  String password;
};

WiFiCredentials savedNetworks[MAX_WIFI_NETWORKS];
int networkCount = 0;

// ==================== CALLBACK BLUETOOTH ====================

// Callback chamado pelo A2DP com cada pacote PCM decodificado.
// Escrevemos manualmente no I2S para contornar a distorcao
// causada pelo gerenciamento interno do A2DP.
void bt_audio_data(const uint8_t *data, uint32_t len) {
  i2s.write((uint8_t*)data, (size_t)len);
}

// ==================== FUNÇÕES AUXILIARES ====================

// Função para converter UTF-8 para ASCII exibível (remove/substitui acentos)
void converteTextoParaTela(const char* origem, char* destino, size_t maxLen) {
  if (origem == nullptr) {
    destino[0] = '\0';
    return;
  }
  size_t j = 0;
  for (size_t i = 0; origem[i] != '\0' && j < maxLen - 1; i++) {
    uint8_t c = (uint8_t)origem[i];
    // ASCII normal (32-126)
    if (c >= 32 && c <= 126) {
      destino[j++] = origem[i];
    }
    // UTF-8 de 2 bytes (caracteres latinos acentuados)
    else if (c == 0xC3 && origem[i + 1] != '\0') {
      i++;
      uint8_t c2 = (uint8_t)origem[i];
      // Minúsculas
      if (c2 == 0xA0 || c2 == 0xA1 || c2 == 0xA2 || c2 == 0xA3 || c2 == 0xA4 || c2 == 0xA5)
        destino[j++] = 'a';                     // à á â ã ä å
      else if (c2 == 0xA7) destino[j++] = 'c';  // ç
      else if (c2 == 0xA8 || c2 == 0xA9 || c2 == 0xAA || c2 == 0xAB)
        destino[j++] = 'e';  // è é ê ë
      else if (c2 == 0xAC || c2 == 0xAD || c2 == 0xAE || c2 == 0xAF)
        destino[j++] = 'i';                     // ì í î ï
      else if (c2 == 0xB1) destino[j++] = 'n';  // ñ
      else if (c2 == 0xB2 || c2 == 0xB3 || c2 == 0xB4 || c2 == 0xB5 || c2 == 0xB6)
        destino[j++] = 'o';  // ò ó ô õ ö
      else if (c2 == 0xB9 || c2 == 0xBA || c2 == 0xBB || c2 == 0xBC)
        destino[j++] = 'u';                     // ù ú û ü
      else if (c2 == 0xBD) destino[j++] = 'y';  // ý
      // Maiúsculas
      else if (c2 == 0x80 || c2 == 0x81 || c2 == 0x82 || c2 == 0x83 || c2 == 0x84 || c2 == 0x85)
        destino[j++] = 'A';                     // À Á Â Ã Ä Å
      else if (c2 == 0x87) destino[j++] = 'C';  // Ç
      else if (c2 == 0x88 || c2 == 0x89 || c2 == 0x8A || c2 == 0x8B)
        destino[j++] = 'E';  // È É Ê Ë
      else if (c2 == 0x8C || c2 == 0x8D || c2 == 0x8E || c2 == 0x8F)
        destino[j++] = 'I';                     // Ì Í Î Ï
      else if (c2 == 0x91) destino[j++] = 'N';  // Ñ
      else if (c2 == 0x92 || c2 == 0x93 || c2 == 0x94 || c2 == 0x95 || c2 == 0x96)
        destino[j++] = 'O';  // Ò Ó Ô Õ Ö
      else if (c2 == 0x99 || c2 == 0x9A || c2 == 0x9B || c2 == 0x9C)
        destino[j++] = 'U';                     // Ù Ú Û Ü
      else if (c2 == 0x9D) destino[j++] = 'Y';  // Ý
    }
    // Outros UTF-8 de 2 bytes
    else if (c == 0xC2 && origem[i + 1] != '\0') {
      i++;
      uint8_t c2 = (uint8_t)origem[i];
      if (c2 == 0xBA) destino[j++] = 'o';       // º
      else if (c2 == 0xAA) destino[j++] = 'a';  // ª
      else if (c2 == 0xA1) destino[j++] = '!';  // ¡
      else if (c2 == 0xBF) destino[j++] = '?';  // ¿
    }
    // Pula outros caracteres multi-byte
    else if (c >= 0xC0 && c <= 0xDF)
      i++;                                    // 2 bytes
    else if (c >= 0xE0 && c <= 0xEF) i += 2;  // 3 bytes
    else if (c >= 0xF0 && c <= 0xF7) i += 3;  // 4 bytes
  }
  destino[j] = '\0';
}

// Versão para String (conveniência)
String converteTexto(const String& texto) {
  converteTextoParaTela(texto.c_str(), textoConvertido, sizeof(textoConvertido));
  return String(textoConvertido);
}

// Função auxiliar para definir fonte padrão
void setFontePadrao() {
  tela.setFreeFont(NULL);
  tela.setTextFont(2);
}

// Função auxiliar para definir fonte de título
void setFonteTitulo() {
  tela.setFreeFont(FF5);
}

// ==================== FUNÇÕES WiFi MULTI ====================

// Salva as credenciais WiFi na memória flash
void salvarCredenciaisWiFi() {
  preferences.begin("wifi", false);
  preferences.putInt("count", networkCount);
  for (int i = 0; i < networkCount; i++) {
    String ssidKey = "ssid" + String(i);
    String passKey = "pass" + String(i);
    preferences.putString(ssidKey.c_str(), savedNetworks[i].ssid);
    preferences.putString(passKey.c_str(), savedNetworks[i].password);
    Serial.printf("Salva rede %d: %s\n", i, savedNetworks[i].ssid.c_str());
  }
  preferences.end();
}

// Carrega as credenciais WiFi da memória flash
void carregarCredenciaisWiFi() {
  preferences.begin("wifi", true);
  networkCount = preferences.getInt("count", 0);
  if (networkCount > MAX_WIFI_NETWORKS) {
    networkCount = MAX_WIFI_NETWORKS;
  }
  for (int i = 0; i < networkCount; i++) {
    String ssidKey = "ssid" + String(i);
    String passKey = "pass" + String(i);
    savedNetworks[i].ssid = preferences.getString(ssidKey.c_str(), "");
    savedNetworks[i].password = preferences.getString(passKey.c_str(), "");
    if (savedNetworks[i].ssid.length() > 0) {
      Serial.printf("Carregada rede %d: %s\n", i, savedNetworks[i].ssid.c_str());
    }
  }
  preferences.end();
}

// Adiciona uma nova rede WiFi
void adicionarRedeWiFi(String ssid, String password) {
  // Verifica se a rede já existe
  for (int i = 0; i < networkCount; i++) {
    if (savedNetworks[i].ssid == ssid) {
      savedNetworks[i].password = password;
      salvarCredenciaisWiFi();
      Serial.printf("Rede atualizada: %s\n", ssid.c_str());
      return;
    }
  }
  // Adiciona nova rede
  if (networkCount < MAX_WIFI_NETWORKS) {
    savedNetworks[networkCount].ssid = ssid;
    savedNetworks[networkCount].password = password;
    networkCount++;
    salvarCredenciaisWiFi();
    Serial.printf("Nova rede adicionada: %s (Total: %d)\n", ssid.c_str(), networkCount);
  } else {
    // Se já tem 3 redes, substitui a mais antiga
    for (int i = 0; i < MAX_WIFI_NETWORKS - 1; i++) {
      savedNetworks[i] = savedNetworks[i + 1];
    }
    savedNetworks[MAX_WIFI_NETWORKS - 1].ssid = ssid;
    savedNetworks[MAX_WIFI_NETWORKS - 1].password = password;
    salvarCredenciaisWiFi();
    Serial.printf("Rede substituída: %s\n", ssid.c_str());
  }
}

// Limpa todas as redes salvas
void limparRedesWiFi() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  networkCount = 0;
  Serial.println("Todas as redes WiFi foram apagadas");
}

// Tenta conectar usando WiFiMulti
bool tentarConexaoMulti() {
  if (networkCount == 0) {
    Serial.println("Nenhuma rede WiFi salva");
    return false;
  }
  // Adiciona todas as redes salvas ao WiFiMulti
  for (int i = 0; i < networkCount; i++) {
    if (savedNetworks[i].ssid.length() > 0) {
      wifiMulti.addAP(savedNetworks[i].ssid.c_str(), savedNetworks[i].password.c_str());
      Serial.printf("Adicionada ao WiFiMulti: %s\n", savedNetworks[i].ssid.c_str());
    }
  }
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_YELLOW, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.println(" Modo Radio");
  tela.printf("Tentando %d rede(s)\n", networkCount);
  // Tenta conectar por até 15 segundos
  Serial.println("Tentando conectar com WiFiMulti...");
  unsigned long startTime = millis();
  while (wifiMulti.run() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    Serial.print(".");
    tela.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Conectado a: %s\n", WiFi.SSID().c_str());
    tela.fillScreen(TFT_BLACK);
    setFontePadrao();
    tela.setTextColor(TFT_GREEN, TFT_BLACK);
    tela.setCursor(0, 0);
    tela.println("WiFi Conectado!");
    tela.setTextColor(TFT_WHITE, TFT_BLACK);
    tela.print("Rede: ");
    tela.println(WiFi.SSID());
    tela.print("IP: ");
    tela.println(WiFi.localIP());
    tela.setTextColor(TFT_CYAN, TFT_BLACK);
    tela.printf("(%d/%d redes)\n", obterIndiceRedeAtual() + 1, networkCount);
    delay(2000);
    return true;
  }
  return false;
}

// Obtém o índice da rede atual conectada
int obterIndiceRedeAtual() {
  String currentSSID = WiFi.SSID();
  for (int i = 0; i < networkCount; i++) {
    if (savedNetworks[i].ssid == currentSSID) {
      return i;
    }
  }
  return -1;
}

void configModeCallback(WiFiManager* myWiFiManager) {
  Serial.println("Entrou no modo de configuração do WiFi");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_YELLOW, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.println("  CONFIG WiFi");
  if (networkCount > 0) {
    tela.setTextColor(TFT_CYAN, TFT_BLACK);
    tela.printf("Redes salvas: %d/%d\n", networkCount, MAX_WIFI_NETWORKS);
    tela.setTextColor(TFT_WHITE, TFT_BLACK);
  }
  tela.println("Conecte na rede:");
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println(AP_NAME);
  tela.setTextColor(TFT_WHITE, TFT_BLACK);
  if (strlen(AP_PASSWORD) > 0) {
    tela.print("Senha: ");
    tela.setTextColor(TFT_CYAN, TFT_BLACK);
    tela.println(AP_PASSWORD);
  } else {
    tela.setTextColor(TFT_GREEN, TFT_BLACK);
    tela.println("(Sem senha)");
  }
  tela.setTextColor(TFT_GREEN, TFT_BLACK);
  tela.println("Acesse:");
  tela.println("192.168.4.1");
}

void saveConfigCallback() {
  Serial.println("Configuração salva!");
  // Adiciona a nova rede às credenciais salvas
  String newSSID = WiFi.SSID();
  String newPass = WiFi.psk();
  if (newSSID.length() > 0) {
    adicionarRedeWiFi(newSSID, newPass);
  }
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_GREEN, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.println("WiFi Configurado!");
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.printf("Rede %d/%d salva\n", networkCount, MAX_WIFI_NETWORKS);
  delay(2000);
}

void baixaDados(const String& urlDado, int maxTokens, String* tokens) {
  http.begin(urlDado);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    conteudoBaixado = http.getString();
    Serial.println("Arquivo de dados das radios baixado com sucesso");
  } else {
    Serial.printf("Falha ao baixar o arquivo. Código de erro HTTP: %d\n", httpCode);
  }
  http.end();
  int startIndex = 0;
  int endIndex = 0;
  int tokenIndex = 0;
  while (endIndex >= 0 && tokenIndex < maxTokens) {
    endIndex = conteudoBaixado.indexOf('\n', startIndex);
    if (endIndex >= 0) {
      tokens[tokenIndex] = conteudoBaixado.substring(startIndex, endIndex);
      startIndex = endIndex + 1;
    } else {
      tokens[tokenIndex] = conteudoBaixado.substring(startIndex);
    }
    tokenIndex++;
  }
}

void sobeCanal() {
  radioTocando = radioTocando + 1;
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_RED, TFT_BLACK);
  tela.setCursor(25, 55);
  tela.println("Sintonizando!");
  delay(500);
  if (radioTocando > 29) {
    radioTocando = 0;
  }
}

void desceCanal() {
  radioTocando = radioTocando - 1;
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_RED, TFT_BLACK);
  tela.setCursor(25, 55);
  tela.println("Sintonizando!");
  delay(500);
  if (radioTocando < 0) {
    radioTocando = 29;
  }
}

void mudouEstacaoMesmo() {
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  // Título com FreeFont FF5 (converte acentos)
  setFonteTitulo();
  String nomeConvertido = converteTexto(nomeRadio[radioTocando]);
  int centro = 80 - (tela.textWidth(nomeConvertido) / 2);
  tela.setCursor(centro, 20);
  tela.setTextColor(TFT_BLUE);
  tela.print(nomeConvertido);
  // Demais textos com fonte padrão (converte acentos)
  setFontePadrao();
  tela.setCursor(0, 32);
  tela.setTextColor(TFT_ORANGE);
  tela.println(converteTexto(cidadeRadio[radioTocando]));
  tela.setTextColor(TFT_YELLOW);
  tela.println(converteTexto(tipoRadio[radioTocando]));
  audio.connecttohost(urlRadio[radioTocando].c_str());
  delay(500);
}

void my_audio_info(Audio::msg_t m) {
  switch (m.e) {
    case Audio::evt_info:
      Serial.printf("info: ....... %s\n", m.msg);
      break;
    case Audio::evt_bitrate:
      Serial.printf("bitrate: .... %s\n", m.msg);
      break;
    case Audio::evt_name:
      Serial.printf("station name: %s\n", m.msg);
      break;
    case Audio::evt_streamtitle:
      Serial.printf("stream title: %s\n", m.msg);
      setFontePadrao();
      tela.fillRect(0, 62, 160, 68, TFT_BLACK);
      tela.setCursor(0, 62);
      tela.setTextColor(TFT_CYAN, TFT_BLACK);
      tela.println("Tocando agora:");
      tela.setCursor(0, 78);
      // Converte acentos do título da música
      converteTextoParaTela(m.msg, textoConvertido, sizeof(textoConvertido));
      tela.println(textoConvertido);
      break;
    default:
      Serial.printf("message:..... %s\n", m.msg);
      break;
  }
}

void avrc_metadata_callback(uint8_t id, const uint8_t* text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      tituloBT = (char*)text;
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      artistaBT = (char*)text;
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      albumBT = (char*)text;
      break;
  }
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  // Título "BLUETOOTH" com FreeFont FF5
  setFonteTitulo();
  int centro = 80 - (tela.textWidth("BLUETOOTH") / 2);
  tela.setCursor(centro, 20);
  tela.setTextColor(TFT_BLUE);
  tela.print("BLUETOOTH");
  // Demais textos com fonte padrão (converte acentos)
  setFontePadrao();
  tela.setCursor(0, 32);
  tela.setTextColor(TFT_ORANGE);
  tela.println("Conectado a: ");
  // Converte nome do dispositivo
  const char* peerName = a2dp_sink.get_peer_name();
  if (peerName != nullptr) {
    converteTextoParaTela(peerName, textoConvertido, sizeof(textoConvertido));
    tela.println(textoConvertido);
  }
  // Converte título
  tela.setTextColor(TFT_PINK);
  if (tituloBT != nullptr) {
    converteTextoParaTela(tituloBT, textoConvertido, sizeof(textoConvertido));
    tela.println(textoConvertido);
  }
  // Converte artista
  tela.setTextColor(TFT_YELLOW);
  if (artistaBT != nullptr) {
    converteTextoParaTela(artistaBT, textoConvertido, sizeof(textoConvertido));
    tela.println(textoConvertido);
  }
  // Converte álbum
  tela.setTextColor(TFT_LIGHTGREY);
  if (albumBT != nullptr) {
    converteTextoParaTela(albumBT, textoConvertido, sizeof(textoConvertido));
    tela.println(textoConvertido);
  }
}

void conectaWiFi() {
  // Carrega as credenciais salvas
  carregarCredenciaisWiFi();
  // Tenta conectar usando WiFiMulti primeiro
  if (tentarConexaoMulti()) {
    return;  // Conectou com sucesso
  }
  // Se não conectou, abre o portal de configuração
  Serial.println("Não conectou com redes salvas. Abrindo portal...");
  WiFi.disconnect();
  delay(100);
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_YELLOW, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.println("Abrindo portal");
  tela.println("de configuracao...");
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(WIFI_TIMEOUT);
  if (!wifiManager.autoConnect(AP_NAME, AP_PASSWORD)) {
    Serial.println("Falha ao conectar e timeout atingido");
    tela.fillScreen(TFT_BLACK);
    setFontePadrao();
    tela.setTextColor(TFT_RED, TFT_BLACK);
    tela.setCursor(0, 0);
    tela.println("WiFi nao");
    tela.println("configurado!");
    tela.println("Reiniciando...");
    delay(3000);
    ESP.restart();
  }
  // Conexão bem sucedida via portal
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_GREEN, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.println("WiFi Conectado!");
  tela.setTextColor(TFT_WHITE, TFT_BLACK);
  tela.print("Rede: ");
  tela.println(WiFi.SSID());
  tela.print("IP: ");
  tela.println(WiFi.localIP());
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.printf("Total: %d/%d redes\n", networkCount, MAX_WIFI_NETWORKS);
  delay(2000);
}

void mostrarRedesSalvas() {
  tela.fillScreen(TFT_BLACK);
  setFontePadrao();
  tela.setTextColor(TFT_YELLOW, TFT_BLACK);
  tela.setCursor(0, 0);
  tela.printf("Redes: %d/%d\n", networkCount, MAX_WIFI_NETWORKS);
  tela.setTextColor(TFT_WHITE, TFT_BLACK);
  for (int i = 0; i < networkCount && i < 5; i++) {
    tela.setTextColor(TFT_CYAN, TFT_BLACK);
    tela.printf("%d. ", i + 1);
    tela.setTextColor(TFT_WHITE, TFT_BLACK);
    tela.println(savedNetworks[i].ssid);
  }
  delay(3000);
}

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP);
  pinMode(canalPraBaixo, INPUT_PULLUP);
  pinMode(modoAparelho, INPUT);
  Serial.begin(115200);
  tela.init();
  tela.setRotation(1);
  tela.invertDisplay(0);
  // Inicializa com fonte padrão
  setFontePadrao();
  if (digitalRead(modoAparelho)) {
    // MODO RADIO
    
    // RESET: Segurar botão CIMA ao ligar = apaga TODAS as redes WiFi
    if (digitalRead(canalPraCima) == LOW) {
      tela.fillScreen(TFT_RED);
      setFontePadrao();
      tela.setTextColor(TFT_WHITE, TFT_RED);
      tela.setCursor(0, 0);
      tela.println("RESET WiFi!");
      tela.println("Apagando todas");
      tela.println("as redes...");
      limparRedesWiFi();
      wifiManager.resetSettings();
      delay(2000);
      tela.println("Reiniciando...");
      delay(1000);
      ESP.restart();
    }
    btStop();
    // MOSTRAR REDES: Segurar botão BAIXO ao ligar = mostra redes salvas
    if (digitalRead(canalPraBaixo) == LOW) {
      carregarCredenciaisWiFi();
      mostrarRedesSalvas();
    }
    // Conecta ao WiFi usando WiFiMulti + WiFiManager
    conectaWiFi();
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    radioTocando = random(0, 30);
    mudouEstacao = true;
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    audio.setVolume(16);
    audio.setConnectionTimeout(5000, 10000);
    audio.setTone(-2, 4, 2);
    Audio::audio_info_callback = my_audio_info;
    baixaDados(endTudo, 120, tudo);
    WiFi.setSleep(false);
    for (int j = 0; j < 120; j = j + 4) {
      nomeRadio[indice] = tudo[j];
      tipoRadio[indice] = tudo[(j + 1)];
      cidadeRadio[indice] = tudo[(j + 2)];
      urlRadio[indice] = tudo[(j + 3)];
      indice++;
    }
  } else {
    // MODO BLUETOOTH
    esp_task_wdt_deinit();  // Desativa watchdog para evitar timeout nos callbacks do Bluetooth
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    i2s.setPins(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
      Serial.println("Failed to initialize I2S!");
      while (1);
    }
    a2dp_sink.set_stream_reader(bt_audio_data, false); // Evita distorcao
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_auto_reconnect(true);
    a2dp_sink.start("BTRadio");
  }
}

void loop() {
  if (digitalRead(modoAparelho)) {
    audio.loop();
    vTaskDelay(10);
    unsigned long agora = millis();
    static unsigned long ultimaVerificacao = 0;
    if (agora - ultimaVerificacao >= 50) {
      ultimaVerificacao = agora;
      if (digitalRead(canalPraCima) == LOW && (agora - ultimoCliqueCima > debounceDelay)) {
        ultimoCliqueCima = agora;
        sobeCanal();
        mudouEstacao = true;
      }
      if (digitalRead(canalPraBaixo) == LOW && (agora - ultimoCliqueBaixo > debounceDelay)) {
        ultimoCliqueBaixo = agora;
        desceCanal();
        mudouEstacao = true;
      }
    }
    if (mudouEstacao) {
      mudouEstacaoMesmo();
      mudouEstacao = false;
    }
  }
  vTaskDelay(10);
}