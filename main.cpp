//Interactive Display V1
//Joshua Weitzel 2025-08-19

#include "header.h"
#include "classes.h"

//Global Variables
String SSID = "";                                           //WiFi network name
String PASS = "";                                           //WiFi password
const char* TIME_ZONE = "EST5EDT,M3.2.0,M11.1.0";           //Time zone code
unsigned long currTime = 0;                                 //Track current run time

//Load themes
struct theme rustLight = {0x9a03, 0x18ca, 0xd408, 0xe56e, "rust", false};
struct theme rustDark = {0x212c, 0x9a03, 0xe56e, 0xd408, "rust", true};
struct theme indigoLight = {0xe51b, 0x9274, 0x6516, 0x4109, "indigo", false};
struct theme indigoDark = {0x4109, 0x6516, 0x9274, 0xe51b, "indigo", true};
struct theme colours = rustLight;  

//Class constructors
ThemeSelectApp ThemeSelect;

void setup(){
  //Initialize serial communication and screen, connect to WiFi sync time
  Serial.begin(115200);
  tft.begin();
  tSPI.begin(T_CLK, T_OUT, T_DIN, T_CS);
  ts.begin(tSPI);
  wifiConnect();
  configTzTime(TIME_ZONE, "pool.ntp.org", "time.nist.gov");
  systemPrefs.begin("settings");


  //Screen rotation is flipped, touchscreen is set to defualt landscape for alignment
  tft.setRotation(3);
  ts.setRotation(1);
  tft.fillScreen(colours.back);

  //Load system settings and display inital program layout
  loadSettings();
  startSystem();
  startApplications();
}

//Main program loop
void loop(){
  //Maintain internet connection before continuing
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(5000);
  }
  //Run update checks
  updates();
  ThemeSelect.runApp();
}

//Classes and methods

//Function definitions
void updates(){
  currTime = millis();
  update5_000();
  update30_000();
}

void update5_000(){
  if(currTime - checkTimes.fiveK >= 5000){
    drawClock();
    checkTimes.fiveK = millis();
  }
}

void update30_000(){
  if(currTime - checkTimes.thirtyK >= 30000){
    drawSignalStrength();
    checkTimes.thirtyK = millis();
  }
}

void wifiConnect(){
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);
    Serial.print("Connecting to WiFi");
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nConnected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void drawGrid(){
  int x = 0;
  int y = 0;
  int spacing = 10;

  for(int i = 0; i < SCREEN_W; i += spacing){
    tft.drawLine(x, 0, x, SCREEN_H, colours.high);
    x += spacing;
  }
  for(int i = 0; i <SCREEN_H; i += spacing){
    tft.drawLine(0, y, SCREEN_W, y, colours.high);
    y += spacing;
  }
}

void drawMenuLayout(){
  tft.fillRect(0, 0, SCREEN_W, SCREEN_H, colours.back);                             //Clean up edges by filling background
  drawBorder(0, 0, SCREEN_W, SCREEN_H, colours.fore, colours.back);                 //Screen wrap border
  tft.fillRect(0, 25, SCREEN_W, 5, colours.fore);                                   //Menu bar line
  drawBorder(10, 35, SCREEN_W / 2 - 13, SCREEN_H - 45, colours.fore, colours.back); //Left hand border
  tft.fillRect(170, 40, 135, 180, colours.back);                                    //Define area for right hand interface
}

void drawClock(){
  struct tm time = getTime();
  int xpos = 60;
  int ypos = 47;
  //Adjust for two digit hours
  if((time.tm_hour >= 10 && time.tm_hour <= 12) || time.tm_hour >= 22 && time.tm_hour <= 24){
    xpos = 40;
  }
  tft.setTextColor(colours.text, colours.back);
  //Draw number elements
  tft.setTextFont(8);
  tft.setCursor(xpos, ypos);
  tft.print(&time, "%l");
  xpos = 40;
  ypos = 127;
  tft.setCursor(xpos,ypos);
  tft.print(&time, "%M");

  //Print AM/PM
  xpos = 20;
  ypos = 50;
  tft.setTextFont(2);
  tft.setCursor(xpos, ypos);
  tft.print(&time, "%p");

  //Print date
  xpos += 20;
  ypos += 155;
  tft.setTextFont(2);
  tft.setCursor(xpos, ypos);
  tft.print(&time, "%d - %m - %Y");
}

void drawSignalStrength(){
  int x = SCREEN_W - 30;
  int y = 14;
  int strength = WiFi.RSSI();
  int barWidth = 3;
  int barHeight = 7;
  int xOffset = 0;
  int heightOffset = 0;
  
  //Draw bars and background
  tft.fillRect(x, y - 6, 13, 13, colours.back);
  for(int i = -90; i <= -50; i += 20){
    if(strength >= i){
      tft.fillRect(x + xOffset, y - heightOffset, barWidth, barHeight + heightOffset, colours.fore);
      xOffset += (barWidth + 2);
      heightOffset += 3;
    }
  }
}

void startApplications(){
  ThemeSelect.drawMenuIcon();
}

void startSystem(){
  drawMenuLayout();
  drawSignalStrength();
  drawClock();
}

void drawBorder(int x, int y, int w, int h, int cBorder, int cInternal){
  tft.fillRoundRect(x, y, w, h, 20, cBorder);
  tft.fillRoundRect(x + 5, y + 5, w - 10, h - 10, 15, cInternal);
}

struct tm getTime(){
  struct tm time;
  while(!getLocalTime(&time)){
    delay(1);
  }
  return time;
}

void loadSettings(){
  //Load theme
  systemPrefs.getBytes("theme", &colours, sizeof(colours));
  
  //Load app
  appSwitch(systemPrefs.getString("app"));
}

void appSwitch(String appName){
  static std::map<String, int> currentApp = {
    {"themeSelector", 1}
  };

  switch(currentApp[appName]){
    case 1:
      ThemeSelect.active = true;
  }
}

//Method definitions
//Button
Button::Button(struct boundingBox touch){
  touchBounds = touch;
  timePressed = 0;
}

bool Button::checkPress(){
  if(ts.tirqTouched() && ts.touched()){
    //Map touch point to x and y position
    TS_Point p = ts.getPoint();
    int xT = map(p.x, 200, 3700, 1, SCREEN_W);
    int yT = map(p.y, 240, 3800, 1, SCREEN_H);

    //Check if point is within bounding box
    if(xT >= touchBounds.x &&
      yT >= touchBounds.y &&
      xT <= touchBounds.x + touchBounds.w &&
      yT <= touchBounds.y + touchBounds.h){
        return true;
      }
  }
  return false;
}

//Theme selection app
ThemeSelectApp::ThemeSelectApp()
  : menuButton({0, 0, 70, 25}), 
    rustButton({170, 40, 40, 40}), 
    indigoButton({170, 90, 40, 40}),
    darkLightToggle({225, 45, 70, 20}){
  active = false;
  switchedTo = false;
  themeSwitch = false;
  checkPressTime = 0;
}

void ThemeSelectApp::runApp(){
  //Check if app is selected and needs to be drawn
  if(active && !switchedTo){
    dark = colours.dark;
    tft.fillRect(SCREEN_W / 2 + 2, 35, SCREEN_W / 2 - 13, SCREEN_H - 45, colours.back);
    drawThemeTiles();
    drawDarkModeToggle();
    switchedTo = true;
  }

  //Check for all button presses
  if(currTime - checkPressTime >= 10){
    checkPressTime = millis();
    //App select
    if(menuButton.checkPress()){
      //Application is active, menu icon colour change
      menuButton.timePressed = millis();
      appSwitch("themeSelector");
      switchedTo = false;
      systemPrefs.putString("app", "themeSelector");
      drawMenuIcon();
    }
    if(active){
      //Theme switching
      if(darkLightToggle.checkPress()){
        if(dark){
          dark = false;
        } else {
          dark = true;
        }
      }

      if(rustButton.checkPress() || (darkLightToggle.checkPress() && colours.themeClassName == "rust")){
        if(dark){
          colours = rustDark;
        } else {
          colours = rustLight;
        }
        themeSwitch = true;
      }
      if(indigoButton.checkPress() || (darkLightToggle.checkPress() && colours.themeClassName == "indigo")){
        if(dark){
          colours = indigoDark;
        } else {
          colours = indigoLight;
        }
        themeSwitch = true;
      }

      //Redraw the screen if any of the theme change events above have been triggered
      if(themeSwitch){
        //Save theme data to settings namespace
        systemPrefs.putBytes("theme", &colours, sizeof(colours));

        //Redraw everything on screen
        startSystem();
        startApplications();
        drawMenuIcon();
        drawThemeTiles();
        drawDarkModeToggle();

        themeSwitch = false;
      }
    }
  }

  //Check button time pressed for touch feedback
  if(currTime - menuButton.timePressed >= 100 && menuButton.feedbackPlayed == true){
    drawMenuIcon();
  }
}

void ThemeSelectApp::drawMenuIcon(){
  int xpos = 15;
  int ypos = 7;
  String text = "Themes";

  if(currTime - menuButton.timePressed <= 100){
    tft.setTextColor(colours.back, colours.text);
    menuButton.feedbackPlayed = true;
  } else {
    tft.setTextColor(colours.text, colours.back);
    menuButton.feedbackPlayed = false;
  }
  tft.setTextFont(2);
  tft.setCursor(xpos,ypos);
  tft.print(text);
  tft.fillRect(65, 0, 5, 25, colours.fore);
}

void ThemeSelectApp::drawThemeTiles(){
  int xpos = 170;
  int ypos = 40;
  drawBorder(xpos, ypos, 40, 40, colours.high, rustLight.back);
  ypos += 50;
  drawBorder(xpos, ypos, 40, 40, colours.high, indigoLight.back);

  //Draw app title
  ypos = 215;
  tft.setTextFont(2);
  tft.setTextColor(colours.text, colours.back);
  tft.setCursor(xpos, ypos);
  tft.print("Theme Selector");
}

void ThemeSelectApp::drawDarkModeToggle(){
  int xpos = 225;
  int ypos = 45;

  tft.setTextFont(4);
  tft.setTextColor(colours.text, colours.back);
  tft.setCursor(xpos, ypos);
  if(colours.dark){
    tft.print("LIGHT");
  } else {
    tft.print("DARK");
  }
}