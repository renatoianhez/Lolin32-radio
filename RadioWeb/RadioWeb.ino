#include <ESP_I2S.h> // I2S library for bluetooth
#include <wav_header.h> //Required for the above library
#include <Arduino.h>
#include <TFT_eSPI.h> // Screen library
#include <SPI.h> // SPI
#include <WiFiMulti.h> // For more than one WiFi location
#include "Audio.h" // Audio lib
#include "examples/320 x 240/Free_Font_Demo/Free_Fonts.h" // Fonts
#include "BluetoothA2DPSink.h" // For Bluetooth
#include <HTTPClient.h> // To access the radio station information link

#define canalPraCima  0   // 0 for Lolin32 - 15 for Wrover - Buttons to change the radio station
#define canalPraBaixo 4
#define I2S_DOUT      25  // Audio I2S pins
#define I2S_BCLK      27
#define I2S_LRCK      26
#define modoAparelho  34  // Radio-Bluetooth selector switch

bool mudouEstacao;
int radioTocando;
char* tituloBT;
char* artistaBT;
char* albumBT;

TFT_eSPI tela = TFT_eSPI(); // ATTENTION The screen pins are defined in the library
Audio audio;
I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);
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
String endTudo = "https://gist.githubusercontent.com/renatoianhez/555bf033ddc5831d16e2602b5c48a909/raw";
int indice = 0;

void baixaDados(const String& urlDado, int maxTokens, String* tokens) {
  http.begin(urlDado);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    conteudoBaixado = http.getString();
    Serial.println("Radio data file downloaded successfully");
  }
  else {
    Serial.printf("Failed to download file. HTTP error code: %d\n", httpCode);
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
  // If you press the button to go up one station:
  radioTocando = radioTocando + 1;          // increase the index
  tela.fillScreen(TFT_BLACK);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.setCursor(0, 0, 4);
  tela.println("Sintonizando!");
  delay(1000); // a break to release the button
  if (radioTocando > 29) {     // Test if you have passed the last station on the list
    radioTocando = 0;         // if yes, go to the first station of the playlist
  }
}

void desceCanal() {
  radioTocando = radioTocando - 1;        // decrements the index
  tela.fillScreen(TFT_BLACK);
  tela.setCursor(0, 0, 4);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.println("Sintonizando!");
  delay(1000); // uma pausa pra largar o botao
  if (radioTocando < 0) {     // Test if it passed below the first on the list
    radioTocando = 29;      // If yes, go to the last one on the playlist.
  }
}

void mudouEstacaoMesmo() {
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  int centro = 77 - (11 * ((nomeRadio[radioTocando].length())) / 2);
  tela.setCursor(centro, 18);
  tela.setTextColor(TFT_BLUE); //blue text
  tela.println(nomeRadio[radioTocando]); // centralized radio name
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

void audio_showstreamtitle(const char *titulo) { // To show the name of the song and singer (some stations do not provide this)
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Playing now:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                                    "); //To clean names
  tela.setCursor(0, 75, 2);
  tela.println(titulo);
}

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {  // Routine to show music data in bluetooth mode
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
  tela.println("Conectado a: ");
  tela.println(a2dp_sink.get_peer_name());
  tela.setTextColor(TFT_PINK);
  tela.println(tituloBT);
  tela.setTextColor(TFT_YELLOW);
  tela.println(artistaBT);
  tela.setTextColor(TFT_LIGHTGREY);
  tela.println(albumBT);
}

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP); // Button mode
  pinMode(canalPraBaixo, INPUT_PULLUP);
  pinMode(modoAparelho, INPUT);
  Serial.begin( 115200 ); // start the serial
  tela.init(); // Start display
  tela.setRotation(1); // Landscape mode
  tela.invertDisplay(0); // Necessary to get the colors right. If the colors are inverted, try 1

  if (digitalRead(modoAparelho)) {  // Radio setups
    radioTocando = random(0, 30); // The first station is drawn
    mudouEstacao = true;
    WiFi.disconnect(); // Check if the WiFi is turned off
    WiFi.mode(WIFI_STA);
    outrosWiFi.addAP("SSID", "password"); // Local WiFi data
    //outrosWiFi.addAP("xxx", "xxx");
    //outrosWiFi.addAP("xxx","xxx");
    while (outrosWiFi.run() != WL_CONNECTED) delay(1500); // Try connecting to networks
    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT); // Audio setup
    audio.setVolume(16); // volume goes from zero to 21
    audio.setTone(0, 6, 4); // Bass - mid - treble equalizer (-40 a 6)
    // audio.setBufsize(0, 4000000);
    // audio.forceMono(true); // Force mono
    // audio.setBalance(0); // Right - Left Stereo Balance (-16 a 16)
    baixaDados(endTudo, 120, tudo); // Download radio stations data
    for (int j = 0; j < 120; j = j + 4) { // Separate the data
      nomeRadio[indice] = tudo[j];
      tipoRadio[indice] = tudo[(j + 1)];
      cidadeRadio[indice] = tudo[(j + 2)];
      urlRadio[indice] = tudo[(j + 3)];
      indice++;
    }
  }
  else {  // Bluetooth setups
   i2s.setPins(I2S_BCLK, I2S_LRCK, I2S_DOUT);
    if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
      Serial.println("Failed to initialize I2S!");
      while (1); // do nothing
    }
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_auto_reconnect(true);
    a2dp_sink.start("BTRadio");
  }
}

void loop() {
  if (digitalRead(modoAparelho)) {
    audio.loop();
    if (digitalRead(canalPraCima) == LOW) {// If you press the button to go up the station
      sobeCanal();
      mudouEstacao = true;   // Let them know that the station has changed!
    }
    if (digitalRead(canalPraBaixo) == LOW) {// If you press the button to go down the station:
      desceCanal();
      mudouEstacao = true;  // Let them know that you've changed station too!
    }
    if (mudouEstacao) {     // If the station has changed, update the LCD and the terminal
      mudouEstacaoMesmo();
      mudouEstacao = false; // Keep the station that is playing
    }
  }
}
