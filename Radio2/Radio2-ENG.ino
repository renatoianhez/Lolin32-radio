#include <Arduino.h>
#include <TFT_eSPI.h> // Library of TFT
#include <SPI.h> // SPI
#include <WiFi.h> // WiFi lib
#include "Audio.h" // Audio lib
#include "Free_Fonts.h" // Fonts

#define canalPraCima  16  // Buttons for change stations
#define canalPraBaixo 17
#define I2S_DOUT      25  // I2S pins
#define I2S_BCLK      27
#define I2S_LRC       26

bool mudouEstacao; // Variables for stations control
int radioTocando;

TFT_eSPI tela = TFT_eSPI(); // The pins of TFT are defined in library
Audio audio;

//Radios names, types, localization and URL's
const char* nomeRadio[30] = {
  "Antena 1",   // 0
  "Kiss",       // 1
  "Univers. UFV",   // 2
  "Cultura", // 3
  "Itatiaia",   // 4
  "Dominicana",     // 5
  "RCA Ribatejo",   // 6
  "Saudade",    // 7
  "Radio USP",     // 8
  "CBN Goiania",
  "Radio UFMG",
  "Tribuna",
  "Love",
  "ABC Lounge",
  "JB FM",
  "M Radio",
  "Flower Power",
  "Antenne",
  "Alvorada",
  "Univers. UFU",
  "Radio Jamaica",
  "Som das Gerais",
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
  "FM - MPB/Varios",
  "FM - MPB",
  "AM - Noticia/Musica",
  "Web - Salsa",
  "FM - Fado/Varios",
  "FM - Flash Back",
  "FM - MPB",
  "FM - Noticias",
  "FM - MPB/Varios",
  "FM - Hits/Flash Back",
  "FM - Salsa/Reggae",
  "Web - Light/Lounge",
  "FM - Hits/Flash Back",
  "Web - Chanson Francaise",
  "Web - Flash Back/Rock",
  "Web - Flash Back/Rock",
  "FM - Hits/Flash Back",
  "FM - MPB/Varios",
  "FM - Reggae",
  "Web - MPB",
  "Web - Axe",
  "Web - Seresta",
  "FM - Rock",
  "Web - Flash Back/Rock",
  "Web - Flash Back/Rock",
  "FM - Rock",
  "FM - MPB/Samba",
  "Web - Ska"
};

const char* cidadeRadio[30] = {
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Vicosa MG",
  "Sao Paulo SP",
  "Belo Horizonte MG",
  "Santo Domingo - Rep Dom",
  "Almeirim - Portugal",
  "Santos SP",
  "Sao Paulo SP",
  "Goiania GO",
  "Belo Horizonte MG",
  "Vitoria ES",
  "Belize City - Belize",
  "Paris - França",
  "Rio de Janeiro RJ",
  "Paris - Franca",
  "Dallas TX - EUA",
  "Bayern - Alemanha",
  "Belo Horizonte MG",
  "Uberlandia MG",
  "Kingston - Jamaica",
  "Uberaba MG",
  "Salvador BA",
  "Taubate SP",
  "Londres - UK",
  "Waynesboro VA - EUA",
  "Waynesboro VA - EUA",
  "Daytona Beach FL - EUA",
  "Vitoria ES",
  "Londres - UK"
};
String urlRadio[] = {
  "https://antenaone.crossradio.com.br/stream/1;", // Antena 1 OK
  "http://cloud2.cdnseguro.com:23538/;", // Kiss FM OK
  "http://paineldj5.com.br:12330/stream", // UFV OK
  "https://radiocultura2.jmvstream.com/stream", // Cultura OK
  "http://8903.brasilstream.com.br:8903/stream?1616578451161", // Itatiaia OK
  "http://radio.domiplay.net:2002/;", // Dominicana OK
  "http://95.217.75.38:8820/stream", // RCA Ribatejo GL+++
  "https://22073.live.streamtheworld.com/SAUDADE_FMAAC.aac", // Saudade OK
  "http://flow.emm.usp.br:8007/radiousp-128.mp3", // USP OK
  "http://v3.directradios.com:8229/stream", // CBN OK
  "http://150.164.63.210:8000/aovivo.mp3", // UFMG OK no tags
  "http://cast.midiaip.com.br:6060/stream?1506557548760", // Tribuna GL+
  "https://s1.voscast.com:10243/lovefm",  // Love FM GL+
  "http://str1.openstream.co/589", // ABC Louge GL++
  "https://15123.live.streamtheworld.com/JBFMAAC.aac", // JB GL+
  "http://mfm.ice.infomaniak.ch/mfm-128", // M Radio GL+++
  "http://49.12.129.103:7016/stream/;", // Flower Power GL+++
  "https://s9-webradio.webradio.de/oldies-but-goldies", // Antenne Oldies GL+++
  "https://20323.live.streamtheworld.com/RADIO_ALVORADAAAC_SC", // Alvorada OK
  "http://webradio.dr.ufu.br:8000/live", // UFU OK
  "http://hestia2.cdnstream.com/1301_128?1582209939912", // Jamaica OK
  "https://player-ssl.kshost.com.br:11616/live", // Som das Gerais OK
  "http://euroticast5.euroti.com.br:8070/;", // Axe BA OK
  "http://50.3.30.111:8042/stream", // Seresta ao Luar OK
  "http://media-ice.musicradio.com/RadioXLondon", // XFM GL+
  "http://relay.181.fm:8066/", // 181 Super 70 OK
  "http://relay.181.fm:8132/", // 181 Classic OK
  "http://ice7.securenetsystems.net/WROD", // WROD GL+
  "https://8642.brasilstream.com.br/stream?identifica=9725815160273912000&rand=9201410188400340000", //UFES OK
  "https://eu10.fastcast4u.com/tonicska" // Tonic Ska GL++
};

String nomedaRede = ("XXXXXX"); // WiFi local data
String senhadaRede = ("XXXXXX");

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP); // Buttons mode
  pinMode(canalPraBaixo, INPUT_PULLUP);
  Serial.begin( 115200 ); // Serial init
  tela.init(); // Display init
  tela.setRotation(1); // Landscape screen
  tela.invertDisplay(1); // My TFT need this for correct colors
  radioTocando = random(0, 30); // Random the first station
  mudouEstacao = true;
  WiFi.disconnect(); // Conect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(nomedaRede.c_str(), senhadaRede.c_str());
  while (WiFi.status() != WL_CONNECTED) delay(1500); 
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT); // Audio config
  audio.setVolume(12); // volume zero to 21
  audio.setConnectionTimeout(2000, 7200); // Time for connect
  audio.setTone(0, -4, -20); // Equalization bass - mid - terble (-40 to 6)
  // audio.forceMono(true); // Mono
  // audio.setBalance(0); // Balance estereo right - left (-16 to 16)
}

void loop() {

  if (digitalRead(canalPraCima) == LOW) {  // If play the button, up one station:
    radioTocando = radioTocando + 1;          // increments the index
    tela.fillScreen(TFT_BLACK);
    tela.setTextColor(TFT_RED, TFT_WHITE);
    tela.setCursor(0, 0, 4);
    tela.println("Sintonizando!");
    delay(1000); // a break to release the button
    if (radioTocando > 29) {     // Test if you passed the last station on the list
      radioTocando = 0;         // if yes go to the first of the playlist
    }
    mudouEstacao = true;       // Let it know you've changed the station!
  }

  if (digitalRead(canalPraBaixo) == LOW) { //If play the button, down one station:
    radioTocando = radioTocando - 1;        // decrements the index
    tela.fillScreen(TFT_BLACK);
    tela.setCursor(0, 0, 4);
    tela.setTextColor(TFT_RED, TFT_WHITE);
    tela.println("Sintonizando!");
    delay(1000); // a break to release the button
    if (radioTocando < 0) {     // Forehead went below the first on the list
      radioTocando = 29;      // if yes, go to the last of the playlist
    }
    mudouEstacao = true;    // Let it know you've changed the station!
  }
  if (mudouEstacao) {             // If the station has changed, update the screen
    tela.setFreeFont(FF5);
    tela.fillScreen(TFT_BLACK);
    tela.fillRect(0, 0, 160, 28, TFT_WHITE);
    int centro = 77 - (11 * ((strlen(nomeRadio[radioTocando])) / 2));
    tela.setCursor(centro, 18);
    tela.setTextColor(TFT_BLUE); 
    tela.println(nomeRadio[radioTocando]); // Centralized
    tela.setTextFont(2);
    tela.setCursor(0, 30);
    tela.setTextColor(TFT_ORANGE);
    tela.println(cidadeRadio[radioTocando]);
    tela.setCursor(0, 45, 2);
    tela.setTextColor(TFT_YELLOW);
    tela.println(tipoRadio[radioTocando]);
    audio.connecttohost(urlRadio[radioTocando].c_str());
    delay(500);
    mudouEstacao = false; // Keep the station playing
  }
  audio.loop();
}
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_showstreamtitle(const char *titulo) { // To show the name of the song and who sings it
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Tocando agora:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                        "); //To clean
  tela.setCursor(0, 75, 2);
  tela.println(titulo);
}
