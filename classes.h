//Classes header file
//Joshua Weitzel

#ifndef CLASSES_H
#define CLASSES_H

#include "header.h"

/**
 * Button class is used for all interactive elements used in the applications
 */
class Button{
    public:
        Button(struct boundingBox touch);
        struct boundingBox touchBounds;     //Dimensions used to check button press
        bool checkPress();                  //Checks if button has been pressed
        unsigned long timePressed;          //Records time of button press
        bool feedbackPlayed;                //Tracks whether the button feedback has been drawn
};

/**
 * Parent class which device applications are derived from
 */
class Application{
    public:
        virtual void runApp();          //App functionality is nested inside, and defined for each application
        virtual void drawMenuIcon();    //Draws the icon in the top menu bar, reacts to touch

        bool active;                    //Tracks if application is active
        bool switchedTo;                //Tracks if we have drawn the app after active is flagged true
        unsigned long checkPressTime;   //Tracks the time between checking for button presses
};

class ThemeSelectApp: public Application{
    public:
        //Constructors and methods
        ThemeSelectApp();
        void runApp() override;
        void drawMenuIcon();
        void drawThemeTiles();      //User taps tiles to chnage theme
        void drawDarkModeToggle();  //User taps to change from dark to light mode

        //Variables
        bool dark;                  //Tracks whether we are in dark mode or not
        bool themeSwitch;           //Tracks whether we have switched themes, and need to redraw the screen
        
        //Interactive
        Button menuButton;
        Button rustButton;
        Button indigoButton;
        Button darkLightToggle;


};

#endif