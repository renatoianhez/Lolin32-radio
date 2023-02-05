#include <Arduino.h>
#include <TFT_eSPI.h> // Screen lib
#include <SPI.h> // SPI
#include <WiFi.h> // WiFi lib
#include <WiFiMulti.h> // Multi address WiFi
#include "Audio.h" // Audio lib
#include "examples/320 x 240/Free_Font_Demo/Free_Fonts.h" // Fonts
#include "BluetoothA2DPSink.h" // Bluetooth lib

#define canalPraCima  0   // Switch stations
#define canalPraBaixo 4
#define I2S_DOUT      25  // Audio I2S pins
#define I2S_BCLK      27
#define I2S_LRCK      26
#define modoAparelho  34  // Switch radio-bluetooth

bool mudouEstacao;
int radioTocando;
char* tituloBT;
char* artistaBT;
char* albumBT;

TFT_eSPI tela = TFT_eSPI(); // Screen pins are in library
Audio audio;
BluetoothA2DPSink a2dp_sink;
WiFiMulti outrosWiFi;
bool connected = true;

//Radios and URL's
const char* nomeRadio[30] = {
  "Antena 1",
  "Kiss",
  "Eldorado FM", 
  "Cultura",
  "Itatiaia", 
  "Dominicana",  
  "BFF.fm", 
  "Saudade", 
  "Radio USP", 
  "CBN BH",
  "Radio UFMG",
  "Tribuna",
  "Smooth Jazz",
  "Louge FM",
  "JB FM",
  "Chante France",
  "Guerrilla",
  "Oldie Antenne",
  "Alvorada",
  "Sarau",
  "RadioNOS",
  "Inconfidencia",
  "Axe Bahia",
  "Seresta aoLuar",
  "XFM",
  "Super 70s",
  "Classic Hits",
  "WROD",
  "Univers. UFES",
  "Tonic Ska"
};
const char* tipoRadio[30] = {
  "FM - Hits Internacional",
  "FM - Rock",
  "FM - Noticia/Musica",
  "FM - MPB",
  "AM - Noticia/Musica",
  "Web - Salsa",
  "Web - Indie/Wolrd",
  "FM - Flash Back",
  "FM - MPB",
  "FM - Noticias",
  "FM - MPB/Varios",
  "FM - Hits/Flash Back",
  "Web - Jazz",
  "Web - Acustico/Lounge",
  "FM - Hits/Flash Back",
  "Web - Chanson Francaise",
  "Web - Alternative/Rock",
  "Web - Flash Back/Rock",
  "FM - Hits/Flash Back",
  "FM - MPB/Seresta",
  "Web - Dub/Reggae",
  "FM - MPB/Noticias",
  "Web - Axe",
  "Web - Seresta",
  "FM - Rock",
  "Web - Flash Back/Rock",
  "Web - Flash Back/Rock",
  "FM - Rock",
  "FM - MPB/Samba",
  "Web - Ska/Reggae"
};

const char* cidadeRadio[30] = {
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Belo Horizonte MG",
  "Santo Domingo - R. Dom",
  "San Francisco CA - USA",
  "Santos SP",
  "Sao Paulo SP",
  "Belo Horizonte MG",
  "Belo Horizonte MG",
  "Vitoria ES",
  "New York - USA",
  "Kiev - Ucrania",
  "Rio de Janeiro RJ",
  "Paris - Franca",
  "Sarasota FL - USA",
  "Bayern - Alemanha",
  "Belo Horizonte MG",
  "Conservatoria RJ",
  "Fortaleza CE",
  "Belo Horizonte MG",
  "Salvador BA",
  "Taubate SP",
  "Londres - UK",
  "Waynesboro VA - EUA",
  "Waynesboro VA - EUA",
  "Daytona Beach FL - EUA",
  "Vitoria ES",
  "Londres - UK"
};
String urlRadio[] = {    // GL = glitches, OK = no glitches
  "https://antenaone.crossradio.com.br/stream/1;", // Antena 1 OK no tags
  "http://cloud2.cdnseguro.com:23538/;", // Kiss FM OK
  "https://eldoradolive02.akamaized.net/hls/live/2043453/eldorado/master.m3u8", // Eldorado OK no tags
  "https://radiocultura2.jmvstream.com/stream", // Cultura OK no tags
  "http://8903.brasilstream.com.br:8903/stream?1616578451161", // Itatiaia OK
  "http://radio.domiplay.net:2002/;", // Dominicana OK
  "https://ais-sa2.cdnstream1.com/2053_128.mp3", // BFF.FM GL
  "https://24073.live.streamtheworld.com/SAUDADE_FMAAC.aac", // Saudade OK
  "http://flow.emm.usp.br:8007/radiousp-128.mp3", // USP OK no tags
  "https://26513.live.streamtheworld.com/CBN_BHAAC.aac", // CBN OK
  "http://150.164.63.210:8000/aovivo.mp3", // UFMG OK no tags
  "http://cast.midiaip.com.br:6060/stream?1506557548760", // Tribuna GL+
  "https://hestia2.cdnstream.com/1078_128?cb=696226.mp3",  // Smooth Jazz GL
  "http://cast.loungefm.com.ua/acoustic128?1410329161003.mp3", // Louge FM GL
  "https://24083.live.streamtheworld.com/JBFMAAC.aac", // JB GL+
  "https://chantefranceemotion.ice.infomaniak.ch/chantefranceemotion-96.aac", // Chante France GL
  "http://s2.stationplaylist.com:9460/guerrilla", // Guerrilla OK
  "https://s7-webradio.oldie-antenne.de/oldie-antenne", // Antenne Oldies GL+++
  "http://20323.live.streamtheworld.com/RADIO_ALVORADAAAC_SC", // Alvorada OK
  "https://s17.maxcast.com.br:8783/live", // Sarau OK
  "https://nos.radio.br/stream/8/;", // RadioNOS OK
  "https://8446.brasilstream.com.br/stream", // Inconfidencia FM OK
  "http://euroticast5.euroti.com.br:8070/;", // Axe BA OK
  "http://50.3.30.111:8042/stream", // Seresta ao Luar OK
  "http://media-ice.musicradio.com/RadioXLondon", // XFM GL+
  "http://relay.181.fm:8066/", // 181 Super 70 OK
  "http://relay.181.fm:8132/", // 181 Classic OK
  "http://ice7.securenetsystems.net/WROD", // WROD GL+
  "http://8642.brasilstream.com.br/stream?identifica=9725815160273912000&rand=9201410188400340000", //UFES OK
  "https://eu10.fastcast4u.com/tonicska" // Tonic Ska GL++
};

void sobeCanal() {
  // Se apertar o botao pra subir uma estacao:
  radioTocando = radioTocando + 1;          // increments index
  tela.fillScreen(TFT_BLACK);
  tela.setTextColor(TFT_RED, TFT_WHITE);
  tela.setCursor(0, 0, 4);
  tela.println("Sintonizando!");
  delay(1000); // delay for button
  if (radioTocando > 29) {     // Tests if is the last station of list
    radioTocando = 0;         // if yes, go to the first
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

void mudouEstacaoMesmo() {
  tela.setFreeFont(FF5);
  tela.fillScreen(TFT_BLACK);
  tela.fillRect(0, 0, 160, 28, TFT_WHITE);
  int centro = 77 - (11 * ((strlen(nomeRadio[radioTocando])) / 2));
  tela.setCursor(centro, 18);
  tela.setTextColor(TFT_BLUE); //blue text
  tela.println(nomeRadio[radioTocando]); // centralizes radio name
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

void audio_showstreamtitle(const char *titulo) { // To show tags
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Tocando agora:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                                    "); //Clean tags
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
  Serial.begin( 115200 );
  tela.init(); // Display init
  tela.setRotation(1); // Screen mode portrait
  tela.invertDisplay(1); // Color adjust for the TFT
  outrosWiFi.addAP("xxx", "xxx"); // WiFi data
  //outrosWiFi.addAP("xxx", "xxx"); // Others WiFi
  //outrosWiFi.addAP("xxx","xxx");

  if (digitalRead(modoAparelho)) {  //If radio mode
    radioTocando = random(0, 30); // Sort the first station
    mudouEstacao = true;
    WiFi.disconnect(); // Wifi connection
    WiFi.mode(WIFI_STA);
    while (outrosWiFi.run() != WL_CONNECTED) delay(1500);

    audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT); // Radio config
    audio.setVolume(16); // volume zero to 21
    // audio.setConnectionTimeout(2000, 7200); // Grant connection
    audio.setTone(4, 0, 4); // Equalizator bass - mid - terble (-40 to 6)
    audio.setBufsize(0, 4000000);
    // audio.forceMono(true); // Mono
    // audio.setBalance(0); // Balance (-16 to 16)
  }
  else {
    i2s_pin_config_t my_pin_config = {  //If Bluetooth mode
      .bck_io_num = 27,
      .ws_io_num = 26,
      .data_out_num = 25,
      .data_in_num = I2S_PIN_NO_CHANGE
    };
    a2dp_sink.set_pin_config(my_pin_config);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_auto_reconnect(true);
    a2dp_sink.start("BTRadio1");
  }
}

void loop() {
  if (digitalRead(modoAparelho)) { // If the mode switch is radio
    audio.loop();
    if (digitalRead(canalPraCima) == LOW) {//If up station
      sobeCanal();
      mudouEstacao = true;
    }
    if (digitalRead(canalPraBaixo) == LOW) {//If down station
      desceCanal();
      mudouEstacao = true;
    }
    if (mudouEstacao) {  
      mudouEstacaoMesmo();
      mudouEstacao = false;
    }
  }
}
