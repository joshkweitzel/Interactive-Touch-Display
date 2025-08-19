//Interactive Display Header
//Joshua Weitzel

#ifndef HEADER_H
#define HEADER_H

//Libraries
#include <Arduino.h>
#include <TFT_config.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <map>

//Definitions
#define SCREEN_W 320
#define SCREEN_H 240
#define T_IRQ 23
#define T_OUT 22
#define T_DIN 32
#define T_CS 33
#define T_CLK 25
#define FONT_SIZE 2

//Display components
TFT_eSPI tft = TFT_eSPI();
SPIClass tSPI = SPIClass(VSPI);
XPT2046_Touchscreen ts(T_CS, T_IRQ);
//Preferences
Preferences systemPrefs;

//Struct declarations
/**
 * Holds the timing data for the update functions in milliseconds
 */
struct {
    unsigned long fiveK = 0;
    unsigned long thirtyK = 0;
    unsigned long ten = 0;
} checkTimes;

/**
 * Holds the colour codes used when drawing graphics, coded in rgb565
 * Each theme is made from this struct
 */
struct theme{
    int back;
    int fore;
    int high;
    int text;
    String themeClassName;
    bool dark;
};

/**
 * Used to check screen tap interactions
 */
struct boundingBox {
    int x;
    int y;
    int w;
    int h;
};

//Function declarations
/**
 * Run several update checks for seperate time intervals
 * Different functionalities of the program will run at different rates to save processing power and reduce flickering
 */
void updates();

/**
 * Call functions every 5 seconds
 */
void update5_000();

/**
 * Call functions every 30 seconds
 */
void update30_000();

/**
 * Connect to wifi network and continue
 */
void wifiConnect();

/**
 * Draws a grid on the screen dividing into 10x10 sections. Used for placing elements on the screen
 */
void drawGrid();

/**
 * Draws out the menu layout 
 */
void drawMenuLayout();

/**
 * Draw the clock which is ever-present in the left hand region of the display
 */
void drawClock();

/**
 * Display WiFi signal strength
 */
void drawSignalStrength();

/**
 * Draws application menu
 */
void startApplications();

/**
 * Draws the device layout, signal strength, and clock face
 */
void startSystem();

/**
 * Draws a rounded rectangular border given dimensions and colour, thickness and radius will be consistent through program
 * Also fills in with a given internal colour, defaulted to the background
 */
void drawBorder(int x, int y, int w, int h, int cBorder, int cInternal);

/**
 * Returns the local time from an ntp server in a printable format
 */
struct tm getTime(); 

/**
 * Loads system preferences
 * Includes: current theme, current application
 */
void loadSettings();

/**
 * Switches the app given to active, and turns all other apps to inactive
 */
void appSwitch(String appName);

#endif