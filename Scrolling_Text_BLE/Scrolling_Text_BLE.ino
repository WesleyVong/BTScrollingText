#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <EEPROM.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   13
#define DATA_PIN  6
#define CS_PIN    5

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  //Serial.println(err);
  while (1);
}

// LED Setup
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 1000; // in milliseconds

// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  75
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "Type Message on Phone" };
bool newMessageAvailable = true;

// Determines if the chip should save state between power cycles.
#define SAVE_STATE true
// Char array location in EEPROM
#define STRING_START 1

void setup()
{
  //Serial.begin(115200);

  // Setup BLE
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

//  Serial.println("Requesting Bluefruit info:");
//  /* Print Bluefruit information */
//  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!
  if (SAVE_STATE){
    for (int i = 0; i < BUF_SIZE; i++){
      newMessage[i] = EEPROM[ i + STRING_START ];
    }
  }
  // Setup LED
  P.begin();
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void loop()
{
  // Check for incoming characters from Bluefruit

  if (P.displayAnimate())
  {
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    P.displayReset();
  }
  readBle();
}

void readBle(void)
{
  ble.println("AT+BLEUARTRX");
  ble.readline();
  if (strcmp(ble.buffer, "OK") != 0) {
    strcpy(newMessage,ble.buffer);
    if (SAVE_STATE){
      for (int i = 0; i < BUF_SIZE; i++){
        EEPROM[ i + STRING_START ] = newMessage[i];
      }
    }
    //Serial.println(newMessage);
    ble.waitForOK();
    newMessageAvailable = true;
  }
  
}
