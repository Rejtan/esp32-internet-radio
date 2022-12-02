#include <ESP32Encoder.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <EEPROM.h>
#include <WiFiManager.h>


  #define TFT_CS         5
  #define TFT_RST        4         //piny do wyswietlacza                                   
  #define TFT_DC         2
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); // wyswietlacz

#define I2S_DOUT     25
#define I2S_BCLK      27  //piny do DAC
#define I2S_LRC        26
Audio audio;

#define EEPROM_SIZE 2   //Rozmiar aktywowanej pamieci EEPROM


string station[4] = {"http://ic1.smcdn.pl/2300-1.mp3", "https://zt03.cdn.eurozet.pl/zet-tun.mp3", "http://195.150.20.7/rmf_fm", "http://us3.internet-radio.com:8313/;"};



  #define SW 34 //przycisk encodera

 ESP32Encoder encoder; // tworzenie ecodera


 int enkoderold=0; // wartosc poprzednia enkodera zeby kazda pozycja wyswietlala sie raz


 int wybormenu = 0; // warunkuje o tym jakie menu wyswietlic


 int powrot = 0; // do ktorego punku menu wrocic
 int raz = 0; // do wykonywania raz czynnosci w submenu


 int volume = 10; // wartosc glosnosci

 int btnState = digitalRead(SW); // stan wcisniecia przycisku
 unsigned long lastButtonPress = 0; // czas od ostatniego wcisniecia

 int stacja=0;
 string tmp;

//////////////////////////////////////////////////////////////////////////////////////////////////////


 void setup () {
   pinMode(TRIGGER_PIN, INPUT_PULLUP);

  EEPROM.begin(EEPROM_SIZE);
  volume = EEPROM.read(0);
  stacja = EEPROM.read(1);
  if(stacja == 255 || volume == 255){

    volume=0;
    stacja=0;
  }

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.
 
    // put your setup code here, to run once:
    Serial.begin(115200);
    
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
  
    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();
 
    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
 
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    res = wm.autoConnect("Esp32-radio"); // anonymous ap

 
    if(!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        
    }

 
 



  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  audio.setVolume(volume);

  tmp = station[stacja];
  


  audio.connecttohost(tmp.c_str());



   //////////////

pinMode(34, INPUT_PULLUP); // podciagniecie pinu przycisku przez rezystor pullup

  //wirtualny encoder
  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachSingleEdge(35, 32);
  encoder.setCount(1);
  encoder.setFilter(1023);


  // Inicjuje wyswietlacz
  tft.initR(INITR_BLACKTAB);

  // Wyswietlam logo
  bootlogo();
 }

 ////////////////////////////////////////////////////////////////////////////////////////////////////////

 void loop() {


   audio.loop();

if (wybormenu==0){
menuglowne();      //wywoluje glowne menu
}

if (wybormenu!=0){
  menu1();
  menu2();
  menu3();          // wywoluje wybrane submenu
  menu4();
  menu5();


}

// inaczej enkoder swiruje


 }
///////////////////////////////////////////////////////////////////////////////////////
 void bootlogo() {
  tft.fillScreen(ST77XX_BLUE);
  tft.setCursor(20, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);                // logo przy starcie
  tft.println("ESP32");
  tft.setCursor(20, 90);
  tft.println("Radio");
  delay(3000);
  tft.fillScreen(ST77XX_BLUE);

}
//////////////////////////////////////////////////////////////////////////////////////////
void menuglowne(){




    if ((int32_t)encoder.getCount()<1){
    encoder.setCount(1);                  // zeby licznik w menu glownym sie nie przekrecil
  }
  if ((int32_t)encoder.getCount()>5){
    encoder.setCount(5);
  }

////////////////////////////////////////////////////////////////////////////////////////////

if (enkoderold!=(int32_t)encoder.getCount()){
    switch ((int32_t)encoder.getCount()) {
  case 1:
  
  rstmenu();
  tft.setCursor(0, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("-> Glosnosc  ");                      // switch warunkujacy o wybranej pozycji w menu
  enkoderold=(int32_t)encoder.getCount();
  
  

    break;
  case 2:
  
  rstmenu();
  tft.setCursor(0, 20);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("-> Stacja  ");
  enkoderold=(int32_t)encoder.getCount();
  
  
    break;
  case 3:
  
  rstmenu();
  tft.setCursor(0, 40);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("-> Zegar  ");
  enkoderold=(int32_t)encoder.getCount();
  
  
    break;
  case 4:
  
  rstmenu();
  tft.setCursor(0, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("-> EQ  ");
  enkoderold=(int32_t)encoder.getCount();
  
  
    break;
  case 5:
  
  rstmenu();
  tft.setCursor(0, 80);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("-> BL/WiFi  ");
  enkoderold=(int32_t)encoder.getCount();
  
  
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
      if (wybormenu==0){
        wybormenu=(int32_t)encoder.getCount();   // to otwiera submenu po wcisnieciu i wychodzi z niego
      }else {
        EEPROM.commit();
        if (powrot==2){
          audio.connecttohost(tmp.c_str());
        }
        
        wybormenu=0;
        enkoderold=0;
        raz=0;
        encoder.setCount(powrot);
        tft.fillScreen(ST77XX_BLUE);
      }
      
	
		}

		lastButtonPress = millis();
	}

}




void menu1(){
if (wybormenu==1){
  if (raz==0){
tft.fillScreen(ST77XX_BLUE);
raz=1;
encoder.setCount(volume);
  tft.setCursor(20, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("Glosnosc");
  }
  
  tft.setCursor(50, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(3);
  tft.println((int32_t)encoder.getCount());
   if ((int32_t)encoder.getCount()<0){
    encoder.setCount(0);                  // zeby licznik w menu glownym sie nie przekrecil
  }
  if ((int32_t)encoder.getCount()>100){
    encoder.setCount(100);
  }
  volume=(int32_t)encoder.getCount();
  audio.setVolume(volume);
  EEPROM.write(0, volume);
  powrot=1;
  podmienmenu();

}
}

void menu2(){
if (wybormenu==2){
  if (raz==0){
tft.fillScreen(ST77XX_BLUE);
raz=1;
  encoder.setCount(stacja);
  tft.setCursor(20, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("Stacja");
  }
  tft.setCursor(50, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(3);
  tft.println((int32_t)encoder.getCount()+1);
      if ((int32_t)encoder.getCount()<0){
    encoder.setCount(3);                  // zeby licznik w menu glownym sie nie przekrecil
  }
  if ((int32_t)encoder.getCount()>3){
    encoder.setCount(0);
  }
  stacja=(int32_t)encoder.getCount();
  tmp = station[stacja];
  EEPROM.write(1, stacja);
  


  

  powrot=2;
  podmienmenu();

}
}


void menu3(){
if (wybormenu==3){
  if (raz==0){
tft.fillScreen(ST77XX_BLUE);
raz=1;

  tft.setCursor(20, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("Zegar");
  }
  


  powrot=3;
  podmienmenu();

}
}


void menu4(){
if (wybormenu==4){
  if (raz==0){
tft.fillScreen(ST77XX_BLUE);
raz=1;

  tft.setCursor(20, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("EQ");
  }
  


  powrot=4;
  podmienmenu();

}
}

void menu5(){
if (wybormenu==5){
  if (raz==0){
tft.fillScreen(ST77XX_BLUE);
raz=1;

  tft.setCursor(20, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println("Bluetooth");
  }
  


  powrot=5;
  podmienmenu();

}
}

/////////////////////////////////////////////////////////
void rstmenu(){


  tft.setCursor(0, 0);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println(" Glosnosc  ");                         //madry sposob zeby nie odswiezac na okraglo ekranu przy zmianie pozycji

  
  tft.setCursor(0, 20);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println(" Stacja  ");

  
  tft.setCursor(0, 40);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println(" Zegar  ");

  tft.setCursor(0, 60);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println(" EQ  ");
  
  tft.setCursor(0, 80);
  tft.setTextWrap(false);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextSize(2);
  tft.println(" BL/WiFi  ");




}



