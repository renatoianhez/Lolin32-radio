# Lolin32-radio
This is another project like others out there: a web radio with ESP32.

New update: I put a bluetooth mode through an SPDT switch. Look in the RadioeBT folder.

The best update so far: it's possible to update your list of radio stations via a text file in GIST! See it on RadioWeb example. This is important because radio stations often change their URLs. In my case, as I make the radio sets as gifts to my friends, I won't need to get the sets back to update them. Although it is a good reason to visit them!


This radio uses Lolin32 Lite (with a PSRAM update, which makes it equivalent to an ESP32 WROVER), a 1.8'' SPI screen and I2S with PCM5102 + PAM8403 amp:

![image](https://github.com/renatoianhez/Lolin32-radio/assets/34423009/a30e7724-32ee-49d0-93e9-ccdccdb0904d)



The libraries used are:

ESP32-audioI2S: https://github.com/schreibfaul1/ESP32-audioI2S

TFT_eSPI: https://github.com/Bodmer/TFT_eSPI

Bluetooth: https://github.com/pschatzmann/ESP32-A2DP

Add the PSRAM:

![image](https://user-images.githubusercontent.com/34423009/181618815-bb8abaf5-4f57-484a-8c8b-20fcffd7e0ec.png)

https://blog.pagefault-limited.co.uk/lolin32-lite-esp32-8mb-psram-upgrade-mod

With PSRAM, the Lolin32 Lite works like a WROVER Kit. The Arduino IDE configuration looks like this:
![image](https://github.com/user-attachments/assets/d0b6b8d4-adf1-4a61-b563-b1994e4caae8)



After audio library updates, it is possible to play HLS streams!
I'm assembling it in a wooden box.

![Imagem do WhatsApp de 2023-02-05 à(s) 17 16 38](https://user-images.githubusercontent.com/34423009/216842651-7298a861-359e-4c44-aa80-7afa5a5a34c8.jpg)

![image](https://github.com/renatoianhez/Lolin32-radio/assets/34423009/b9fccbc7-f1a6-48f1-ba40-f3c2eb047170)


