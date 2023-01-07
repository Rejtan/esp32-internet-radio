#include <ESP32Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"  //ESP32-audioI2S
#include <EEPROM.h>
#include <WiFiManager.h>  //WiFiManager by tzapu
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>


WiFiUDP ntpUDP;                                                  //needed for clock
NTPClient timeClient(ntpUDP, "tempus1.gum.gov.pl", 3600, 1000);  //defining time zone

char Time[] = "00:00:00";      // time format
char Date[] = "00/00/2000r.";  // date format
byte last_second, second_, minute_, hour_, day_, month_;
int year_;


#define TFT_CS 5
#define TFT_RST 4  //TFT pins (clear, reset, and backlight?)
#define TFT_DC 2
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);  // defining the screen with adafruit lib

#define I2S_DOUT 25
#define I2S_BCLK 27  //seting up pins for external DAC
#define I2S_LRC 26

Audio audio;  //for audio service

#define EEPROM_SIZE 3  //Set up how much eeprom to use (3x8bit int)


string station[4] = { "http://ic1.smcdn.pl/2300-1.mp3", "https://zt03.cdn.eurozet.pl/zet-tun.mp3", "http://195.150.20.7/rmf_fm", "http://us3.internet-radio.com:8313/;" };  //table with radio stations



#define SW 34  //encoder switch

ESP32Encoder encoder;  // defining encoder with ESP32Encoder lib


int enkoderold = 0;  //to know when ecoder has moved

int wybormenu = 0;  // decides which menu is chosen

int powrot = 0;  // remembers which menu was selected and comes back to it

int raz = 0;  // to do stuff in submenus once

int volume = 10;  // volume

int btnState = digitalRead(SW);  // state of button

unsigned long lastButtonPress = 0;  // time since the last button press

int stacja = 0;  //station selected

int stacjaold = EEPROM.read(1);  //used for knowing when to change the station

string tmp;  //temporary string for converting the station


#define CYAN 0x253B
#define BLUE 0x22D4
#define LIGHT_CYAN 0x06DE
//////////////////////////////////////////////////////////////////////////////////////////////////////
WiFiManager wm;  //WiFiManager, Local intialization

void setup() {


  EEPROM.begin(EEPROM_SIZE);
  volume = EEPROM.read(0);  //start eeprom and set volume and station from it
  stacja = EEPROM.read(1);

  if (stacja == 255 || volume == 255) {

    volume = 0;  //debunking if eeprom is fresh or has been reset
    stacja = 0;
  }

  WiFi.mode(WIFI_STA);  // explicitly set wifi mode


  wm.setClass("invert");

  if (EEPROM.read(2) == 1) {

    wm.resetSettings();
    EEPROM.write(2, 0);
    EEPROM.commit();
  }

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "Esp32-radio"),
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("Esp32-radio");  // anonymous ap


  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);  //setting DAC up

  audio.setVolume(volume);  //setting volume from eeprom

  tmp = station[stacja];  //setting the station from eeprom
  audio.connecttohost(tmp.c_str());


  pinMode(34, INPUT_PULLUP);  // pulling switch up with internal resistor

  //virtual rotary encoder
  ESP32Encoder::useInternalWeakPullResistors = UP;
  encoder.attachSingleEdge(35, 32);
  encoder.setCount(1);
  encoder.setFilter(1023);
  

  // initiate the screen
  tft.initR(INITR_BLACKTAB);

  // show logo
  bootlogo();


  //Task on second core for only the audio
  TaskHandle_t mp3stream;

  xTaskCreatePinnedToCore(
    radiostream,        /* Task function. */
    "RadioStreamCore1", /* name of task. */
    10000,              /* Stack size of task */
    NULL,               /* parameter of the task */
    1,                  /* priority of the task */
    &mp3stream,         /* Task handle to keep track of created task */
    0);                 /* pin task to core 0 */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {




  if (wybormenu == 0) {
    menuglowne();  //shows main menu
  }

  if (wybormenu != 0) {
    menu1();
    menu2();
    menu3();  // shows the selected submenu
    menu4();
    menu5();
  }

  zegar();  //updates the clock once every while
}
///////////////////////////////////////////////////////////////////////////////////////
void bootlogo() {
  tft.fillScreen(BLUE);
  tft.setCursor(20, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);  // logo
  tft.println("ESP32");
  tft.setCursor(20, 90);
  tft.println("Radio");
  delay(3000);
  tft.fillScreen(BLUE);
}
//////////////////////////////////////////////////////////////////////////////////////////
void menuglowne() {




  if ((int32_t)encoder.getCount() < 1) {
    encoder.setCount(1);  // making the pointer stay inside menu
  }
  if ((int32_t)encoder.getCount() > 5) {
    encoder.setCount(5);
  }

  ////////////////////////////////////////////////////////////////////////////////////////////

  if (enkoderold != (int32_t)encoder.getCount()) {
    switch ((int32_t)encoder.getCount()) {
      case 1:

        rstmenu();
        tft.setCursor(0, 0);
        tft.setTextWrap(false);
        tft.setTextColor(ST77XX_WHITE, BLUE);
        tft.setTextSize(2);
        tft.println("-> Glosnosc  ");  // switch selecting the menus
        enkoderold = (int32_t)encoder.getCount();



        break;
      case 2:

        rstmenu();
        tft.setCursor(0, 20);
        tft.setTextWrap(false);
        tft.setTextColor(ST77XX_WHITE, BLUE);
        tft.setTextSize(2);
        tft.println("-> Stacja  ");
        enkoderold = (int32_t)encoder.getCount();


        break;
      case 3:

        rstmenu();
        tft.setCursor(0, 40);
        tft.setTextWrap(false);
        tft.setTextColor(ST77XX_WHITE, BLUE);
        tft.setTextSize(2);
        tft.println("-> Data  ");
        enkoderold = (int32_t)encoder.getCount();


        break;
      case 4:

        rstmenu();
        tft.setCursor(0, 60);
        tft.setTextWrap(false);
        tft.setTextColor(ST77XX_WHITE, BLUE);
        tft.setTextSize(2);
        tft.println("-> WiFinfo  ");
        enkoderold = (int32_t)encoder.getCount();


        break;
      case 5:

        rstmenu();
        tft.setCursor(0, 80);
        tft.setTextWrap(false);
        tft.setTextColor(ST77XX_WHITE, BLUE);
        tft.setTextSize(2);
        tft.println("-> RST WiFi  ");
        enkoderold = (int32_t)encoder.getCount();


        break;
    }
  }
  ////////////////////////////////////////////////////////////////////////////////////////
  podmienmenu();
}





void podmienmenu() {


  btnState = digitalRead(SW);
  if (btnState == LOW) {

    if (millis() - lastButtonPress > 50) {
      if (wybormenu == 0) {
        wybormenu = (int32_t)encoder.getCount();  // this is used to open and close menus
      } else {
        EEPROM.commit();
        wybormenu = 0;
        enkoderold = 0;
        raz = 0;
        encoder.setCount(powrot);
        tft.fillScreen(BLUE);
      }
    }

    lastButtonPress = millis();
  }
}




void menu1() {
  if (wybormenu == 1) {
    if (raz == 0) {
      tft.fillScreen(BLUE);
      raz = 1;
      encoder.setCount(volume);
      tft.setCursor(20, 0);
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE, BLUE);
      tft.setTextSize(2);
      tft.println("Glosnosc");
    }

    tft.setCursor(50, 60);
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_WHITE, BLUE);
    tft.setTextSize(3);
    tft.println((int32_t)encoder.getCount());
    if ((int32_t)encoder.getCount() < 0) {
      encoder.setCount(0);  // so the volume stays in 1-100
    }
    if ((int32_t)encoder.getCount() > 100) {
      encoder.setCount(100);
    }
    volume = (int32_t)encoder.getCount();
    audio.setVolume(volume);
    EEPROM.write(0, volume);
    powrot = 1;
    podmienmenu();
  }
}

void menu2() {
  if (wybormenu == 2) {
    if (raz == 0) {
      tft.fillScreen(BLUE);
      raz = 1;
      encoder.setCount(stacja);
      tft.setCursor(30, 0);
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE, BLUE);
      tft.setTextSize(2);
      tft.println("Stacja");
    }

    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_WHITE, BLUE);
    tft.setTextSize(2);
    tft.setCursor(5, 60);
    switch ((int32_t)encoder.getCount()) {
      case 0:

        tft.println("   Eska           ");
        break;
      case 1:

        tft.println(" RadioZet         ");
        break;
      case 2:

        tft.println("  RMF FM          ");
        break;
      case 3:

        tft.println("Washington       ");
        break;
    }

    if ((int32_t)encoder.getCount() < 0) {
      encoder.setCount(3);  // so the station stays in 1-4
    }
    if ((int32_t)encoder.getCount() > 3) {
      encoder.setCount(0);
    }
    stacja = (int32_t)encoder.getCount();
    tmp = station[stacja];
    EEPROM.write(1, stacja);





    powrot = 2;
    podmienmenu();
  }
}


void menu3() {
  if (wybormenu == 3) {
    if (raz == 0) {
      tft.fillScreen(BLUE);
      raz = 1;

      tft.setCursor(40, 0);
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE, BLUE);
      tft.setTextSize(2);
      tft.println("Data");
      tft.setCursor(0, 30);
      tft.println(Date);
      switch(weekday()) {

  case 1:
      tft.setCursor(10, 60);
      tft.println("Poniedziałek");
    break;
  case 2:
      tft.setCursor(10, 60);
      tft.println("Wtorek");
    break;
  case 3:
      tft.setCursor(10, 60);
      tft.println("Sroda");
    break;
  case 4:
      tft.setCursor(10, 60);
      tft.println("Czwartek");
    break;
  case 5:
      tft.setCursor(10, 60);
      tft.println("Piatek");
    break;
  case 6:
      tft.setCursor(10, 60);
      tft.println("Sobota");
    break;
  case 7:
      tft.setCursor(10, 60);
      tft.println("Niedziela");
    break;
    }


    }
    powrot = 3;
    podmienmenu();
  
}
}

void menu4() {
  if (wybormenu == 4) {
    if (raz == 0) {
      tft.fillScreen(BLUE);
      raz = 1;

      tft.setCursor(10, 0);
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE, BLUE);
      tft.setTextSize(2);
      tft.println("WiFi Info:");
      tft.setCursor(0, 20);
      tft.println("IP:");
      tft.setCursor(35, 25);
      tft.setTextSize(1);
      tft.println(WiFi.localIP());
      tft.setTextSize(2);
      tft.setCursor(0, 40);
      tft.println("Signal:");
      tft.setCursor(0, 60);
      tft.println("SSID:");
      tft.setCursor(60, 65);
      tft.setTextSize(1);
      tft.println(WiFi.SSID());
    }
    tft.setCursor(85, 40);
    tft.println(WiFi.RSSI());

    powrot = 4;
    podmienmenu();
  }
}

void menu5() {
  if (wybormenu == 5) {
    if (raz == 0) {
      tft.fillScreen(BLUE);
      raz = 1;

      tft.setCursor(20, 40);
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE, BLUE);
      tft.setTextSize(2);
      tft.println("Reboot to");
      tft.setCursor(20, 60);
      tft.println("apply");
      tft.setCursor(10, 80);
      tft.println("changes");
      delay(2000);
      EEPROM.write(2, 1);
      EEPROM.commit();
    }
    powrot = 5;
    podmienmenu();
  }
}

/////////////////////////////////////////////////////////
void rstmenu() {


  tft.setCursor(0, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, BLUE);
  tft.setTextSize(2);
  tft.println(" Glosnosc  ");  //madry sposob zeby nie odswiezac na okraglo ekranu przy zmianie pozycji


  tft.setCursor(0, 20);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, BLUE);
  tft.setTextSize(2);
  tft.println(" Stacja  ");


  tft.setCursor(0, 40);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, BLUE);
  tft.setTextSize(2);
  tft.println(" Data  ");

  tft.setCursor(0, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, BLUE);
  tft.setTextSize(2);
  tft.println(" WiFinfo  ");

  tft.setCursor(0, 80);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, BLUE);
  tft.setTextSize(2);
  tft.println(" RST WiFi  ");
}

void zegar() {
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();  // Get Unix epoch time from the NTP server

  second_ = second(unix_epoch);
  if (last_second != second_) {


    minute_ = minute(unix_epoch);
    hour_ = hour(unix_epoch);
    day_ = day(unix_epoch);
    month_ = month(unix_epoch);
    year_ = year(unix_epoch);



    Time[7] = second_ % 10 + 48;
    Time[6] = second_ / 10 + 48;
    Time[4] = minute_ % 10 + 48;
    Time[3] = minute_ / 10 + 48;
    Time[1] = hour_ % 10 + 48;
    Time[0] = hour_ / 10 + 48;



    Date[0] = day_ / 10 + 48;
    Date[1] = day_ % 10 + 48;
    Date[3] = month_ / 10 + 48;
    Date[4] = month_ % 10 + 48;
    Date[8] = (year_ / 10) % 10 + 48;
    Date[9] = year_ % 10 % 10 + 48;



    tft.setCursor(15, 120);
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_WHITE, BLUE);
    tft.setTextSize(2);
    tft.println(Time);
    last_second = second_;
  }
}


void radiostream(void* pvParameters) {

  for (;;) {
    if (stacjaold != stacja) {
      audio.setVolume(0);
      audio.connecttohost(tmp.c_str());
      delay(100);
      audio.setVolume(volume);
      stacjaold = stacja;
    }
    audio.loop();
  }
}
