#include <Arduino.h>
#include <TFT_eSPI.h> // Screen lib
#include <SPI.h> // SPI
#include <WiFi.h> // WiFi lib
#include <WiFiMulti.h> // Multi WiFi
#include "Audio.h" // Audio lib
#include "examples/320 x 240/Free_Font_Demo/Free_Fonts.h" // Fonts
#include "BluetoothA2DPSink.h" // Bluetooth lib
#include <HTTPClient.h> // HTTP lib

#define canalPraCima  0   // Buttons to change stations
#define canalPraBaixo 4
#define I2S_DOUT      25  // I2S audio pins
#define I2S_BCLK      27
#define I2S_LRCK      26
#define modoAparelho  34  // Radio-bluetooth switch

bool mudouEstacao;
int radioTocando;
char* tituloBT;
char* artistaBT;
char* albumBT;

TFT_eSPI tela = TFT_eSPI(); // Screen pins is in lib
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
String endTudo = "https://gist.githubusercontent.com/XXX/XXX/raw"; // Place here the radio station data address
      // The data format is:
      //   Name of Station (max 14 caracters) ex. KFMG Radio
      //   City-State-Country of the station ex. London UK
      //   Type of station (Web or FM) and genre ex. Web - Rock/Alternative
      //   Station's stream URL ex. https://antenaone.crossradio.com.br/stream/1;
      //  Attention: you must enter the data of 30 radio stations!
      //  Example: https://gist.githubusercontent.com/renatoianhez/4b5726d5ef32ad0a24fa3db6521989d6/raw

int indice = 0;

void baixaDados(const String& urlDado, int maxTokens, String* tokens) {
  http.begin(urlDado);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    conteudoBaixado = http.getString();
    Serial.println("Data file dowloaded!");
  }
  else {
    Serial.printf("Error in file download. HTTP error code: %d\n", httpCode);
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
  radioTocando = radioTocando + 1;          // increments the index
  tela.fillScreen(TFT_BLACK);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.setCursor(0, 0, 4);
  tela.println("Sintonizando!");
  delay(1000); // uma pausa pra largar o botao
  if (radioTocando > 29) { 
    radioTocando = 0; 
  }
}

void desceCanal() {
  radioTocando = radioTocando - 1;        // decrements the index
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(0, 0, 4);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.println("Sintonizando!");
  delay(1000); // uma pausa pra largar o botao
  if (radioTocando < 0) {    
    radioTocando = 29;     
  }
}

void mudouEstacaoMesmo() {  // If change the station it will update the screen
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  int centro = 77 - (11 * ((nomeRadio[radioTocando].length())) / 2);
  tela.setCursor(centro, 18);
  tela.setTextColor(TFT_BLUE); //texto azul
  tela.println(nomeRadio[radioTocando]); 
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

void audio_showstreamtitle(const char *titulo) { // Display and update music tags
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Tocando agora:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                                    "); //To clean
  tela.setCursor(0, 75, 2);
  tela.println(titulo);
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  Serial.printf("==> AVRC metadata rsp: attribute id 0x%x, %s\n", id, text);
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  tela.setCursor(28, 18);
  tela.setTextColor(TFT_BLUE);
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
  tela.setTextColor(TFT_ORANGE);
  tela.println(a2dp_sink.get_peer_name());  // Shows the source name
  tela.setTextColor(TFT_PINK);
  tela.println(tituloBT);
  tela.setTextColor(TFT_YELLOW);
  tela.println(artistaBT);
  tela.setTextColor(TFT_LIGHTGREY);
  tela.println(albumBT);
}

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP); 
  pinMode(canalPraBaixo, INPUT_PULLUP);
  pinMode(modoAparelho, INPUT);
  Serial.begin( 115200 ); 
  tela.init(); 
  tela.setRotation(1); // Landscape mode
  tela.invertDisplay(0); // Color mode adjust. If the colors are wrong, use 1
  outrosWiFi.addAP("XXX", "XXX"); // WiFi data (user, password)
  //outrosWiFi.addAP("xxx", "xxx");
  //outrosWiFi.addAP("xxx","xxx");

  if (digitalRead(modoAparelho)) {
    radioTocando = random(0, 30); // The first station is randomized
    mudouEstacao = true;
    WiFi.disconnect(); 
    WiFi.mode(WIFI_STA);
    while (outrosWiFi.run() != WL_CONNECTED) delay(1500); 
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT); 
    audio.setVolume(16); // volume vai de zero a 21
    audio.setConnectionTimeout(2000, 7200); 
    audio.setTone(4, 0, 4); // Equalizator bass - mid - terble (-40 to 6)
    // audio.setBufsize(0, 4000000);
    // audio.forceMono(true); // Mono option
    // audio.setBalance(0); // Balance right - left (-16 to 16)
    baixaDados(endTudo, 120, tudo); // Download radio data
    for (int j = 0; j < 120; j = j + 4) { // Separates data into its types
      nomeRadio[indice] = tudo[j];
      tipoRadio[indice] = tudo[(j + 1)];
      cidadeRadio[indice] = tudo[(j + 2)];
      urlRadio[indice] = tudo[(j + 3)];
      indice++;
    }
  }
  else {                               // Bluetooth option
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
    if (digitalRead(canalPraCima) == LOW) {//Up station
      sobeCanal();
      mudouEstacao = true; 
    }
    if (digitalRead(canalPraBaixo) == LOW) {//Down station
      desceCanal();
      mudouEstacao = true;  
    }
    if (mudouEstacao) {     //If change station, update the screen
      mudouEstacaoMesmo();
      mudouEstacao = false;
    }
  }
}
