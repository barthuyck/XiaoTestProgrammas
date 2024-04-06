// Versie 06/04/2024
// Bart
//*********************************************************************************************
// uitlezen AHT20:  temp + vochtigheid + stockage op SD kaart met bestandnaan de huidige datum.
// uitlezen SGP30: tVOC en CO2eq uitlezen
//*********************************************************************************************

#include <Arduino.h>
#include <U8x8lib.h>
#include <PCF8563.h>
PCF8563 pcf;
#include <Wire.h>
#include "AHT20.h"
#include <SPI.h>
#include "SdFat.h"
SdFat SD;

#include <i2c.h>
#include <sensirion_common.h>
#include <sensirion_configuration.h>
#include <sgp30.h>
#include <sgp_featureset.h>


#define SD_CS_PIN D2
#define RELAY_PIN D7

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
AHT20 AHT;
File myFile;

/*
 * SD card attached to SPI bus as follows: (SPI hardware default for Xiao)
 ** MOSI - pin 11
 ** MISO - pin 10
 ** CLK - pin 9
 ** CS - depends on your SD card shield or module.
 * Pin 2 used here for the Xiao Expansion board
 * 
 */
 
void setup() {
  //Serial.begin(115200);
  // initialiseerd het scherm
  u8x8.begin();
  u8x8.setFlipMode(1);

  // initialiseerd de RTC
  Wire.begin();
  pcf.init();//initialize the clock
  // volgende lijnen zijn enkel om de klok een nieuwe waarde te geven
  //pcf.stopClock();//stop the clock
  //pcf.setYear(24);//set year
  //pcf.setMonth(4);//set month
  //pcf.setDay(5);//set dat
  //pcf.setHour(14);//set hour
  //pcf.setMinut(52);//set minut
  //pcf.setSecond(0);//set second
  pcf.startClock();//start the clock
  
  // initialiseerd de Temperatuur sensor ATH20 - https://wiki.seeedstudio.com/Grove-AHT20-I2C-Industrial-Grade-Temperature&Humidity-Sensor/
  AHT.begin();
  
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);   // choose a suitable font
  u8x8.setCursor(0, 0); // startpunt opgeven om te schrijven: eerst cijfer is de x-as, tweede het karakter op de lijn
  u8x8.println("init SD card!");
  pinMode(D2, OUTPUT);          // Modify the pins here to fit the CS pins of the SD card you are using.
  pinMode(D7, OUTPUT); // voor relay
  if (!SD.begin(SD_CS_PIN)) {
     u8x8.println("init failed!");
     //Serial.println("initialization failed!");
     while (1);
  }
  
  u8x8.println("init done.");
  delay(1000);
  u8x8.clear();


  // info: https://wiki.seeedstudio.com/Grove-VOC_and_eCO2_Gas_Sensor-SGP30/
  // voor bibliotheek, zie https://github.com/Seeed-Studio/SGP30_Gas_Sensor 
  // installeer de bibliotheel (download de zip van github, dan sketch, include library, zip library)
  // de voorbeelden zijn uitvoer naar de seriele interface, wij willen via het scherm. Verander daarom de "serial."" door "u8x8."
  

  s16 err;
  u16 scaled_ethanol_signal, scaled_h2_signal;

  u8x8.setCursor(0, 3);

  /*  Init module,Reset all baseline,The initialization takes up to around 15 seconds, during which
        all APIs measuring IAQ(Indoor air quality ) output will not change.Default value is 400(ppm) for co2,0(ppb) for tvoc*/
    while (sgp_probe() != STATUS_OK) {
        u8x8.println("SGP failed");
        while (1);
    }
    /*Read H2 and Ethanol signal in the way of blocking*/
    err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal, &scaled_h2_signal);

    u8x8.setCursor(0, 4);

    if (err == STATUS_OK) {
        u8x8.println("get ram signal!");
    } else {
        u8x8.println("error reading signals");
    }
    err = sgp_iaq_init();
    delay(1000);

    
}
 
void loop() {
  Time nowTime = pcf.getTime();//get current time
  //u8x8.setFont(u8x8_font_chroma48medium8_r);   // choose a suitable font
  
  // opbouw uitvoer uur:minuut:seconde
  char strTijd[9];        
  strTijd[0] = '0' + (nowTime.hour / 10);
  strTijd[1] = '0' + (nowTime.hour % 10);
  strTijd[2] = ':';
  strTijd[3] = '0' + (nowTime.minute / 10);
  strTijd[4] = '0' + (nowTime.minute % 10);
  strTijd[5] = ':';
  strTijd[6] = '0' + (nowTime.second / 10);
  strTijd[7] = '0' + (nowTime.second % 10);
  strTijd[8] = '\0';
  
  // opbouw uitvoer dag-maand-jaar
  char strDag[11];
  strDag[0] = '0' + (nowTime.day / 10);
  strDag[1] = '0' + (nowTime.day % 10);
  strDag[2] = '-';
  strDag[3] = '0' + (nowTime.month / 10);
  strDag[4] = '0' + (nowTime.month % 10);
  strDag[5] = '-';
  strDag[6] = '2';
  strDag[7] = '0';
  strDag[8] = '0' + (nowTime.year / 10 );
  strDag[9] = '0' + (nowTime.year % 10);
  strDag[10] = '\0';

  // opbouw bestandsnaam dag-maand-jaar
  char strBestandsnaam[] = "/20000000log.txt";  
  strBestandsnaam[3] = '0' + (nowTime.year / 10 );
  strBestandsnaam[4] = '0' + (nowTime.year % 10);  
  strBestandsnaam[5] = '0' + (nowTime.month / 10);
  strBestandsnaam[6] = '0' + (nowTime.month % 10);  
  strBestandsnaam[7] = '0' + (nowTime.day / 10);
  strBestandsnaam[8] = '0' + (nowTime.day % 10);

  // dag en uur op LCD scherm zetten
  u8x8.setCursor(0, 0);
  u8x8.print(strDag);
  u8x8.setCursor(0, 1);
  u8x8.print(strTijd);

  // AHT20
  float humi, temp; 
  int ret = AHT.getSensor(&humi, &temp); // ezen data
   
  if(ret)     // GET DATA OK --> data op scherm zetten
  {
      u8x8.setCursor(0, 3);
      u8x8.print("Vocht.:");
      u8x8.setCursor(8, 3);
      u8x8.print(humi*100);
      u8x8.print("%");
      u8x8.setCursor(0, 4);
      u8x8.print("Temp. : ");
      u8x8.setCursor(8, 4);
      u8x8.print(temp);      
      u8x8.print((char)0xB0);
      u8x8.print("C");
      u8x8.setCursor(14, 2);
      u8x8.print("- ");   
 

      if (nowTime.second % 20 == 0) // elke 20 seconden data wegschrijven naar de SD kaart
      {
        //myFile = SD.open("/templog.txt", FILE_WRITE);
        if (!SD.exists(strBestandsnaam))
        {
          myFile = SD.open(strBestandsnaam, FILE_WRITE);
          myFile.println("Dag,Uur,Vochtigheid,Temperatuur");
          myFile.close();
        }
        myFile = SD.open(strBestandsnaam, FILE_WRITE);
        u8x8.setCursor(14, 2);
        u8x8.print("+");         
        myFile.print(strDag);
        myFile.print(",");
        myFile.print(strTijd);
        myFile.print(",");
        myFile.print(humi*100);
        myFile.print(",");
        myFile.print(temp);      
        myFile.println(",");
        myFile.close();
         
      }      
  }
  else        // GET DATA FAIL
  {
      u8x8.println("GET DATA FROM AHT20 FAIL");
  }

  // voor de SGP30
  s16 err = 0;
  u16 tvoc_ppb, co2_eq_ppm;
  err = sgp_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
  u8x8.setCursor(0, 5);
  if (err == STATUS_OK) {
      
      u8x8.print("tVOC :");
      u8x8.setCursor(8, 5);
      u8x8.print(tvoc_ppb);
      u8x8.print("ppb");

      u8x8.setCursor(0, 6);
      u8x8.print("CO2eq :");
      u8x8.setCursor(8, 6);
      u8x8.print(co2_eq_ppm);
      u8x8.println("ppm");
  } else {
      u8x8.println("error reading IAQ values\n");
  }

  // relay doen schakelen --> kies geschikte parameter.
  if (temp>20){
    digitalWrite(D7, HIGH);
  }
  else {
    digitalWrite(D7, LOW);
  }
  delay(1000);
}
