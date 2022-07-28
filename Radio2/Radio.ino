#include <Arduino.h>
#include <TFT_eSPI.h> // Biblioteca da tela
#include <SPI.h> // Biblioteca do SPI
#include <WiFi.h> // Pra funcionar o WiFi
#include "Audio.h" // Biblioteca do audio
#include "Free_Fonts.h" // Fontes

#define canalPraCima  0   // Botoes para mudar a estacao
#define canalPraBaixo 4
#define I2S_DOUT      25  // Pinos do audio I2S
#define I2S_BCLK      27
#define I2S_LRCK      26

bool mudouEstacao;
int radioTocando;

TFT_eSPI tela = TFT_eSPI(); // Os pinos da tela sao definidos na biblioteca
Audio audio;

//Radios e enderecos
const char* nomeRadio[30] = {
  "Antena 1",   // 0
  "Kiss",       // 1
  "Eldorado FM",   // 2
  "Cultura", // 3
  "Itatiaia",   // 4
  "Dominicana",     // 5
  "BFF.fm",   // 6
  "Saudade",    // 7
  "Radio USP",     // 8
  "CBN BH",
  "Radio UFMG",
  "Tribuna",
  "Smooth Jazz",
  "ABC Lounge",
  "JB FM",
  "Chante France",
  "Guerrilla",
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
  "Web - Light/Lounge",
  "FM - Hits/Flash Back",
  "Web - Chanson Francaise",
  "Web - Alternative/Rock",
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
  "Web - Ska/Reggae"
};

const char* cidadeRadio[30] = {
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Sao Paulo SP",
  "Belo Horizonte MG",
  "Santo Domingo - Rep Dom",
  "San Francisco CA - USA",
  "Santos SP",
  "Sao Paulo SP",
  "Belo Horizonte MG",
  "Belo Horizonte MG",
  "Vitoria ES",
  "New York - USA",
  "Londres - UK",
  "Rio de Janeiro RJ",
  "Paris - Franca",
  "Sarasota FL - USA",
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
  "https://eldoradolive02.akamaized.net/hls/live/2043453/eldorado/master.m3u8", // Eldorado
  "http://radiocultura2.jmvstream.com/stream", // Cultura OK
  "http://8903.brasilstream.com.br:8903/stream?1616578451161", // Itatiaia OK
  "http://radio.domiplay.net:2002/;", // Dominicana OK
  "https://ais-sa2.cdnstream1.com/2053_128.mp3", // BFF.FM
  "http://22073.live.streamtheworld.com/SAUDADE_FMAAC.aac", // Saudade OK
  "http://flow.emm.usp.br:8007/radiousp-128.mp3", // USP OK
  "http://20323.live.streamtheworld.com/CBN_BH_ADP/HLS/playlist.m3u8", // CBN OK
  "http://150.164.63.210:8000/aovivo.mp3", // UFMG OK no tags
  "http://cast.midiaip.com.br:6060/stream?1506557548760", // Tribuna GL+
  "https://hestia2.cdnstream.com/1078_128?cb=696226.mp3",  // Smooth Jazz
  "http://str1.openstream.co/589", // ABC Louge GL++
  "https://24083.live.streamtheworld.com/JBFMAAC.aac", // JB GL+
  "https://chantefranceemotion.ice.infomaniak.ch/chantefranceemotion-96.aac", // Chante France
  "https://s2.stationplaylist.com:9460/guerrilla", // Guerrilla
  "http://s9-webradio.webradio.de/oldies-but-goldies", // Antenne Oldies GL+++
  "http://20323.live.streamtheworld.com/RADIO_ALVORADAAAC_SC", // Alvorada OK
  "http://webradio.dr.ufu.br:8000/live", // UFU OK
  "http://hestia2.cdnstream.com/1301_128?1582209939912", // Jamaica OK
  "https://player-ssl.kshost.com.br:11616/live", // Som das Gerais OK
  "http://euroticast5.euroti.com.br:8070/;", // Axe BA OK
  "http://50.3.30.111:8042/stream", // Seresta ao Luar OK
  "http://media-ice.musicradio.com/RadioXLondon", // XFM GL+
  "http://relay.181.fm:8066/", // 181 Super 70 OK
  "http://relay.181.fm:8132/", // 181 Classic OK
  "http://ice7.securenetsystems.net/WROD", // WROD GL+
  "http://8642.brasilstream.com.br/stream?identifica=9725815160273912000&rand=9201410188400340000", //UFES OK
  "https://eu10.fastcast4u.com/tonicska" // Tonic Ska GL++
};

String nomedaRede = ("Renato"); // Dados do WiFi local
String senhadaRede = ("abacatequecaiudope");

void setup() {
  pinMode(canalPraCima, INPUT_PULLUP); // Modo dos botoes
  pinMode(canalPraBaixo, INPUT_PULLUP);
  Serial.begin( 115200 ); // inicia o serial
  tela.init(); // Inicia o display
  tela.setRotation(1); // Modo paisagem
  tela.invertDisplay(1); // Necessario para acertar as cores
  radioTocando = random(0, 30); // Sorteia e primeira estacao
  mudouEstacao = true;
  WiFi.disconnect(); // Conecta ao WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(nomedaRede.c_str(), senhadaRede.c_str());
  while (WiFi.status() != WL_CONNECTED) delay(1500); // Insiste em conectar

  audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT); // Configura o audio
  audio.setVolume(12); // volume vai de zero a 21
  audio.setConnectionTimeout(2000, 7200); // Garante a conexao as estacoes
  audio.setTone(-2, -4, -8); // Equalizador grave - medio - agudo (-40 a 6)
  audio.setBufsize(0,4194252);
  // audio.forceMono(true); // Força o mono
  // audio.setBalance(0); // Balanco estereo direito - esquerdo (-16 a 16)
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
  int centro = 77 - (11 * ((strlen(nomeRadio[radioTocando])) / 2));
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
  Serial.print("info        "); Serial.println(info);
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

void loop() {
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
