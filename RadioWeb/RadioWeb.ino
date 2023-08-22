#include <Arduino.h>
#include <TFT_eSPI.h> // Biblioteca da tela
#include <SPI.h> // SPI
#include <WiFi.h> // Pra funcionar o WiFi
#include <WiFiMulti.h> // Pra mais de um local de WiFi
#include "Audio.h" // Biblioteca do audio
#include "examples/320 x 240/Free_Font_Demo/Free_Fonts.h" // Fontes
#include "BluetoothA2DPSink.h" // Para o Bluetooth
#include <HTTPClient.h>

#define canalPraCima  0   // Botoes para mudar a estacao
#define canalPraBaixo 4
#define I2S_DOUT      25  // Pinos do audio I2S
#define I2S_BCLK      27
#define I2S_LRCK      26
#define modoAparelho  34  // Chave seletora radio-bluetooth

bool mudouEstacao;
int radioTocando;
char* tituloBT;
char* artistaBT;
char* albumBT;

TFT_eSPI tela = TFT_eSPI(); // Os pinos da tela sao definidos na biblioteca
Audio audio;
BluetoothA2DPSink a2dp_sink;
WiFiMulti outrosWiFi;
HTTPClient http;
bool connected = true;

//Radios e enderecos
String conteudoBaixado;
String tudo[120];
String nomeRadio[30];
String tipoRadio[30];
String cidadeRadio[30];
String urlRadio[30];
String endTudo = "https://gist.githubusercontent.com/renatoianhez/4b5726d5ef32ad0a24fa3db6521989d6/raw";
int indice = 0;

void baixaDados(const String& urlDado, int maxTokens, String* tokens) {
  http.begin(urlDado);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    conteudoBaixado = http.getString();
    Serial.println("Arquivo de dados das radios baixado com sucesso");
  }
  else {
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
  // Se apertar o botao pra subir uma estacao:
  radioTocando = radioTocando + 1;          // incrementa o indice
  tela.fillScreen(TFT_BLACK);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.setCursor(0, 0, 4);
  tela.println("Sintonizando!");
  delay(1000); // uma pausa pra largar o botao
  if (radioTocando > 29) {     // Testa se passou da ultima estacao da lista
    radioTocando = 0;         // se sim vai pra primeira da playlist
  }
}

void desceCanal() {
  radioTocando = radioTocando - 1;        // decrementa o indice
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(0, 0, 4);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.println("Sintonizando!");
  delay(1000); // uma pausa pra largar o botao
  if (radioTocando < 0) {     // Testa se passou pra baixo da primeira da lista
    radioTocando = 29;      // se sim vai pra ultima da playlist
  }
}

void mudouEstacaoMesmo() {
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  int centro = 77 - (11 * ((nomeRadio[radioTocando].length())) / 2);
  tela.setCursor(centro, 18);
  tela.setTextColor(TFT_BLUE); //texto azul
  tela.println(nomeRadio[radioTocando]); // nome da radio centralizado
  tela.setTextFont(2);
  tela.setCursor(0, 30);
  tela.setTextColor(TFT_ORANGE);
  tela.println(cidadeRadio[radioTocando]);
  tela.setCursor(0, 45, 2);
  tela.setTextColor(TFT_YELLOW);
  tela.println(tipoRadio[radioTocando]);
  audio.connecttohost(urlRadio[radioTocando].c_str());
  delay(500);
}
void audio_info(const char *info) {
  Serial.print("info        ");
  Serial.println(info);
}

void audio_showstreamtitle(const char *titulo) { // Pra mostrar o nome da musica e cantor
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Tocando agora:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                                    "); //Pra limpar
  tela.setCursor(0, 75, 2);
  tela.println(titulo);
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  tela.setCursor(28, 18);
  tela.setTextColor(TFT_BLUE); //texto azul
  tela.println("BLUETOOTH");
  tela.setTextFont(2);
  tela.setCursor(0, 30);
  switch (id) {
    case ESP_AVRC_MD_ATTR_TITLE:
      tituloBT = (char*) text;
      break;
    case ESP_AVRC_MD_ATTR_ARTIST:
      artistaBT = (char*) text;
      break;
    case ESP_AVRC_MD_ATTR_ALBUM:
      albumBT = (char*) text;
      break;
  }
  tela.setTextColor(TFT_PINK);
  tela.println(tituloBT);
  tela.setTextColor(TFT_YELLOW);
  tela.println(artistaBT);
  tela.setTextColor(TFT_LIGHTGREY);
  tela.println(albumBT);
}

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP); // Modo dos botoes
  pinMode(canalPraBaixo, INPUT_PULLUP);
  pinMode(modoAparelho, INPUT);
  Serial.begin( 115200 ); // inicia o serial
  tela.init(); // Inicia o display
  tela.setRotation(1); // Modo paisagem
  tela.invertDisplay(0); // Necessario para acertar as cores
  outrosWiFi.addAP("Renato", "abacatequecaiudope"); // Dados do WiFi local
  //outrosWiFi.addAP("xxx", "xxx");
  //outrosWiFi.addAP("xxx","xxx");

  if (digitalRead(modoAparelho)) {
    radioTocando = random(0, 30); // Sorteia e primeira estacao
    mudouEstacao = true;
    WiFi.disconnect(); // Confere se o WiFi ta desligado
    WiFi.mode(WIFI_STA);
    while (outrosWiFi.run() != WL_CONNECTED) delay(1500); // Tenta conectar nas redes
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT); // Configura o audio
    audio.setVolume(16); // volume vai de zero a 21
    audio.setConnectionTimeout(2000, 7200); // Garante a conexao as estacoes
    audio.setTone(4, 0, 4); // Equalizador grave - medio - agudo (-40 a 6)
    // audio.setBufsize(0, 4000000);
    // audio.forceMono(true); // Força o mono
    // audio.setBalance(0); // Balanco estereo direito - esquerdo (-16 a 16)
    baixaDados(endTudo, 120, tudo); // Baixa dados das radios
    for (int j = 0; j < 120; j = j + 4) { // Separa os dados
      nomeRadio[indice] = tudo[j];
      tipoRadio[indice] = tudo[(j + 1)];
      cidadeRadio[indice] = tudo[(j + 2)];
      urlRadio[indice] = tudo[(j + 3)];
      indice++;
    }
  }
  else {
    i2s_pin_config_t my_pin_config = {
      .bck_io_num = 27,
      .ws_io_num = 26,
      .data_out_num = 25,
      .data_in_num = I2S_PIN_NO_CHANGE
    };
    a2dp_sink.set_pin_config(my_pin_config);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_auto_reconnect(true);
    a2dp_sink.start("BTRadio");
  }
}

void loop() {
  if (digitalRead(modoAparelho)) {
    audio.loop();
    if (digitalRead(canalPraCima) == LOW) {//Se apertar o botao pra subir a estacao
      sobeCanal();
      mudouEstacao = true;   // Avisa la que mudou de estacao!
    }
    if (digitalRead(canalPraBaixo) == LOW) {//Se apertar o botao para descer a estacao:
      desceCanal();
      mudouEstacao = true;  //Avisa la que mudou de estacao tambem!
    }
    if (mudouEstacao) {     //Se a estacao mudou, atualiza o LCD e o terminal
      mudouEstacaoMesmo();
      mudouEstacao = false; // Mantem a estacao que esta tocando
    }
  }
}
