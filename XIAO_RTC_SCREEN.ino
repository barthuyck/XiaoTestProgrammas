// Versie 12/11/2023
// Bart
//*************************************************************************************
// uitlezen temp + vochtigheid + stockage op SD kaart met bestandnaan de huidige datum.
//*************************************************************************************

#include <Arduino.h>
#include <U8x8lib.h>
#include <PCF8563.h>
PCF8563 pcf;
#include <Wire.h>
#include "AHT20.h"
#include <SPI.h>
#include "SdFat.h"
SdFat SD;

#define SD_CS_PIN D2

//#include <SD.h>
//#include "FS.h"

//U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* clock=*/ PIN_WIRE_SCL, /* data=*/ PIN_WIRE_SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
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
  u8x8.begin();
  u8x8.setFlipMode(1);
  Wire.begin();
  pcf.init();//initialize the clock
  //pcf.stopClock();//stop the clock
  //pcf.setYear(24);//set year
  //pcf.setMonth(4);//set month
  //pcf.setDay(5);//set dat
  //pcf.setHour(14);//set hour
  //pcf.setMinut(52);//set minut
  //pcf.setSecond(0);//set second
  pcf.startClock();//start the clock
  
  AHT.begin();
  
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);   // choose a suitable font
  u8x8.setCursor(0, 0);
  u8x8.println("init SD card!");
  pinMode(D2, OUTPUT);          // Modify the pins here to fit the CS pins of the SD card you are using.
  if (!SD.begin(SD_CS_PIN)) {
     u8x8.println("init failed!");
     //Serial.println("initialization failed!");
     while (1);
  }
  // 
  // if (!SD.begin(D2)) {
  //   
  //   return;
  // }
  // Serial.println("initialization done.");
  u8x8.println("init done.");
  delay(2000);
  u8x8.clear();
    
}
 
void loop() {
  Time nowTime = pcf.getTime();//get current time
  //u8x8.setFont(u8x8_font_chroma48medium8_r);   // choose a suitable font
  
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

  char strBestandsnaam[] = "/20000000log.txt";  
  strBestandsnaam[3] = '0' + (nowTime.year / 10 );
  strBestandsnaam[4] = '0' + (nowTime.year % 10);  
  strBestandsnaam[5] = '0' + (nowTime.month / 10);
  strBestandsnaam[6] = '0' + (nowTime.month % 10);  
  strBestandsnaam[7] = '0' + (nowTime.day / 10);
  strBestandsnaam[8] = '0' + (nowTime.day % 10);

 

 
  /*u8x8.setCursor(0, 0);
  u8x8.print(nowTime.day);
  u8x8.print("/");
  u8x8.print(nowTime.month);
  u8x8.print("/");
  u8x8.print("20");
  u8x8.print(nowTime.year);
  u8x8.setCursor(0, 1);
  u8x8.print(nowTime.hour);
  u8x8.print(":");
  u8x8.print(nowTime.minute);
  u8x8.print(":");
  u8x8.println(nowTime.second);*/

  
  u8x8.setCursor(0, 0);
  u8x8.print(strDag);
  u8x8.setCursor(0, 1);
  u8x8.print(strTijd);


  float humi, temp;
 
  int ret = AHT.getSensor(&humi, &temp);
   
  if(ret)     // GET DATA OK
  {
      u8x8.setCursor(0, 3);
      u8x8.print("Vochtigheid:");
      u8x8.setCursor(0, 4);
      u8x8.print(humi*100);
      u8x8.print("%");
      u8x8.setCursor(0, 5);
      u8x8.print("Temperatuur: ");
      u8x8.setCursor(0, 6);
      u8x8.print(temp);      
      u8x8.print((char)0xB0);
      u8x8.print("C");
      u8x8.setCursor(14, 6);
      u8x8.print("- ");   
 

      if (nowTime.second % 20 == 0)
      {
        //myFile = SD.open("/templog.txt", FILE_WRITE);
        if (!SD.exists(strBestandsnaam))
        {
          myFile = SD.open(strBestandsnaam, FILE_WRITE);
          myFile.println("Dag,Uur,Vochtigheid,Temperatuur");
          myFile.close();
        }
        myFile = SD.open(strBestandsnaam, FILE_WRITE);
        u8x8.setCursor(14, 6);
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
 
  delay(1000);
}
