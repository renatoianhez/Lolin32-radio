#include <Arduino.h>
#include <TFT_eSPI.h> // Biblioteca da tela
#include <SPI.h> // SPI
#include <WiFi.h> // Pra funcionar o WiFi
#include "Audio.h" // Biblioteca do audio
#include "Free_Fonts.h" // Fontes

#define canalPraCima  16  // Botoes para mudar a estacao
#define canalPraBaixo 17
#define I2S_DOUT      25  // Pinos do audio I2S
#define I2S_BCLK      27
#define I2S_LRC       26

bool mudouEstacao;
int radioTocando;

TFT_eSPI tela = TFT_eSPI(); // Os pinos da tela sao definidos na biblioteca
Audio audio;

//Radios e enderecos
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
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT); // Configura o audio
  audio.setVolume(12); // volume vai de zero a 21
  audio.setConnectionTimeout(2000, 7200); // Garante a conexao as estacoes
  audio.setTone(0, -4, -20); // Equalizador grave - medio - agudo (-40 a 6)
  // audio.forceMono(true); // Força o mono
  // audio.setBalance(0); // Balanco estereo direito - esquerdo (-16 a 16)
}

void loop() {

  if (digitalRead(canalPraCima) == LOW) {  // Se apertar o botao pra subir uma estacao:
    radioTocando = radioTocando + 1;          // incrementa o indice
    tela.fillScreen(TFT_BLACK);
    tela.setTextColor(TFT_RED, TFT_WHITE);
    tela.setCursor(0, 0, 4);
    tela.println("Sintonizando!");
    delay(1000); // uma pausa pra largar o botao
    if (radioTocando > 29) {     // Testa se passou da ultima estacao da lista
      radioTocando = 0;         // se sim vai pra primeira da playlist
    }
    mudouEstacao = true;       // Avisa la que mudou de estacao!
  }

  if (digitalRead(canalPraBaixo) == LOW) { //Se apertar o botao para descer a estacao:
    radioTocando = radioTocando - 1;        // decrementa o indice
    tela.fillScreen(TFT_BLACK);
    tela.setCursor(0, 0, 4);
    tela.setTextColor(TFT_RED, TFT_WHITE);
    tela.println("Sintonizando!");
    delay(1000); // uma pausa pra largar o botao
    if (radioTocando < 0) {     // Testa se passou pra baixo da primeira da lista
      radioTocando = 29;      // se sim vai pra ultima da playlist
    }
    mudouEstacao = true;    //Avisa la que mudou de estacao tambem!
  }
  if (mudouEstacao) {             //Se a estacao mudou, atualiza o LCD e o terminal
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
    mudouEstacao = false; // Mantem a estacao que esta tocando
  }

  audio.loop();
}
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_showstreamtitle(const char *titulo) { // Pra mostrar o nome da musica e de quem
  tela.setCursor(0, 60, 2);
  tela.setTextColor(TFT_CYAN, TFT_BLACK);
  tela.println("Tocando agora:");
  tela.setCursor(0, 75, 2);
  tela.println("                                                                        "); //Pra limpar
  tela.setCursor(0, 75, 2);
  tela.println(titulo);
}
