# Lolin32-radio
This is another project like others out there: a web radio with ESP32.
This one uses Lolin32 Lite a 1.8'' SPI screen and I2S as audio output for two options:
1) PCM5102 + PAM8403

![image](https://user-images.githubusercontent.com/34423009/176981125-c3595077-6971-41bd-9329-ddba71c24424.png)

2) 02 MAX98356

![image](https://user-images.githubusercontent.com/34423009/176981142-baa64990-6087-4eee-bf87-1f89c40a2a42.png)

The libraries used are:

ESP32-audioI2S: https://github.com/schreibfaul1/ESP32-audioI2S

TFT_eSPI: https://github.com/Bodmer/TFT_eSPI

I'm sorry if in the code presented the explanations are in my native language, Brazilian Portuguese, and the radio stations are mostly from my country.

I have had enough difficulties to run some radios. Some just didn't work, notably those with an HLS stream. These (some I liked a lot) I removed. But others are still there, playing with a lot of glitches. In the code, I indicate with the letters "GL" and symbols "+" those that give more or less glitches.
