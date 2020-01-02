#ifndef WATCH2_H
#define WATCH2_H

////////////////////////////////////////
// includes
////////////////////////////////////////
#include <SPI.h>                    // SPI library
#include <SdFat.h>                     // sd card access library
#include <sdios.h>
#include <Adafruit_GFX.h>           // Used for drawing graphics to the OLED
#include <Adafruit_SSD1351.h>       // used to interface with the OLED
#include <Adafruit_ImageReader.h>
#include <JC_Button.h>              // button object
#include <WiFi.h>                   // wifi library
#include <Preferences.h>            // for storing settings in nvs (allowing for persistance over power cycles)
#include <tinyexpr.h>               // expression evaluator for calculator
#include <map>                      // map object for storing states
#include <unordered_map>
#include <string>                   // std::string
#include <functional>               // std::function thing
#include <time.h>                   // used for system-level time keeping
#include <sys/time.h>               // see above
#include <TimeLib.h>                // used for managing time (see code note 1)
#include <TimeAlarms.h>             //used for creating and managing alarms
#include <stdio.h>                  // i don't actually know...
#include <stdint.h>
#include <algorithm>                // used for std::find and std::min and std::max
#include <stack>

#include "watch2.h"                 // defines and function prototypes
//#include "globals.h"                // declatations for global variables

// custom fonts
#include "custom_fonts/sourcesanspro-regular-6.h"
#include "custom_fonts/sourcesanspro-light-8.h"
#include "custom_fonts/sourcesanspro-light-12.h"
#include "custom_fonts/sourcesanspro-regular-16.h"
#include "custom_fonts/sourcesanspro-light-16.h"
#include "custom_fonts/sourcesanspro-regular-24.h"
#include "custom_fonts/sourcesanspro-light-24.h"

#include "coolcrab.h"
#include "regret.h"

//pin declarations
#define cs   5      // goes to TFT CS
#define sdcs 4      //sd card chip select
#define sdcd 17     //sd card detect
#define dc   22     // goes to TFT DC
#define mosi 23     // goes to TFT MOSI
#define sclk 18     // goes to TFT SCK/CLK
#define rst  21     // ESP RST to TFT RESET
#define miso 19     // Not connected
//       3.3V       // Goes to TFT LED
//       5v         // Goes to TFT Vcc
//       Gnd        // Goes to TFT Gnd

#define dpad_up     33
#define dpad_down   27
#define dpad_left   32
#define dpad_right  25
#define dpad_enter  26
#define BATTERY_DIVIDER_PIN 34
#define TORCH_PIN 13
#define IR_PIN 12

//button active macros
#define KEY_REPEAT_DELAY    550     //time for key repeat to start, in ms [DAS]
#define KEY_REPEAT_PERIOD   24      //time between key repeats, in ms     [ARR]

#define dpad_up_active()    ( !watch2::dpad_up_lock &&    (watch2::btn_dpad_up.wasPressed() || ( watch2::btn_dpad_up.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 5 ) ) ) )
#define dpad_down_active()  ( !watch2::dpad_down_lock &&  (watch2::btn_dpad_down.wasPressed() || ( watch2::btn_dpad_down.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 5 ) ) ) )
#define dpad_left_active()  ( !watch2::dpad_left_lock &&  (watch2::btn_dpad_left.wasPressed() || ( watch2::btn_dpad_left.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 5 ) ) ) )
#define dpad_right_active() ( !watch2::dpad_right_lock && (watch2::btn_dpad_right.wasPressed() || ( watch2::btn_dpad_right.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 5 ) ) ) )
#define dpad_enter_active() ( !watch2::dpad_enter_lock && (watch2::btn_dpad_enter.wasPressed() || ( watch2::btn_dpad_enter.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 5 ) ) ) )
#define dpad_any_active()   ( dpad_up_active() || dpad_down_active() || dpad_left_active() || dpad_right_active() || dpad_enter_active() )

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define ORANGE          0xFC00
#define PURPLE          0x8010
#define WHITE           0xFFFF

// device info
#define SCREEN_WIDTH            128
#define SCREEN_HEIGHT           96
#define WATCH_VER               "0.1"
#define BATTERY_VOLTAGE_MAX     3.3     //max voltage of battery (when fully charged)
#define BATTERY_VOLTAGE_SCALE   2       //scale factor of voltage divider
                                        //make sure the battery voltage (when scaled) is not aboe 3.3v.
                                        //a LiPo that maxes out at 4.7v will be scaled to 2.35v, which is
                                        //fine for the ESP32.
                                        //Calculator definitions
#define CALC_CELL_WIDTH   25//27 for 4 chrs
#define CALC_CELL_HEIGHT  12

namespace watch2
{
    // struct definitions

    // state metadata struct
    struct stateMeta {
        
        //stateMeta members
        std::string             stateName;
        std::function<void()>   stateFunc;
        std::string             stateIcon;
        int                     variant;
        bool                    hidden;
        
        //stateMeta constructor
        stateMeta(
            std::string stateName, 
            std::function<void()> stateFunc,
            std::string stateIcon = "what",
            int variant = 0,
            bool hidden = false
        ) : 
            stateName(stateName),
            stateFunc(stateFunc),
            stateIcon(stateIcon),
            variant(variant),
            hidden(hidden)
        {}
    };

    // data for entries in settings menus
    struct settingsMenuData {

        String                      setting_title;
        int                         setting_value;
        std::vector<String>         text_map;
        int                         min_field_width;

    };

    // data for timers
    struct timerData {

        //the time left on a timer (in seconds).  only updated when the timer is paused, so
        //don't rely on this for when the timer is not paused.  when the timer
        //is paused, the Alarm ID is freed, and timer (at least according to
        //the TimeAlarms lib) ceases to exist.  this duration value is used to
        //create a new timer when the timer is resumed.  this makes it look like
        //the timer is actually paused.
        time_t                      duration;

        time_t                      initial_duration;

        //if the timer is not paused, then it will be associated with a TimeAlarms
        //timer.  this member stores the id of the timer.  when the timer is paused,
        //the TimeAlarms timer does not exist (see above), so this variable is set
        //to 255.  this can be used to check whether or not the timer is paused.
        AlarmID_t                   alarm_id;

        //the time at which the TimeAlarms timer is started.  if the timer is
        //paused and resumed, this member stored the time at which the timer was
        //resumed.  should be 255 if the timer has not yet started, or the timer
        //is paused.
        time_t                      time_started;

        //the function to execute once the timer has completed
        OnTick_t                    on_tick_handler;

        time_t                      last_value;

    };

    struct alarmData {

        AlarmID_t                   alarm_id;
        time_t                      initial_time;
        bool                        paused;
        time_t                      last_value;

    };

    // type definitions
    typedef void (*func)(void);                                                         // function pointer type

    // object creation
    extern SPIClass *vspi;                                                                     // VSPI object
    extern Adafruit_SSD1351 oled;                                                              //hw spi (use vspi or &SPI)
    //Adafruit_SSD1351 oled = Adafruit_SSD1351(128, 96, cs, dc, mosi, sclk, rst);       //sw spi
    extern Preferences preferences;                                                            //wrapper for esp32 nvs used to store system settings
    extern SdFat SD;    
    extern Adafruit_ImageReader reader;                                                                       //sdfat instance used for accessing sd card

    //button objects
    extern Button btn_dpad_up;
    extern Button btn_dpad_down;
    extern Button btn_dpad_left;
    extern Button btn_dpad_right;
    extern Button btn_dpad_enter;
    extern Button btn_zero;

    //vector of watch states
    extern std::vector<stateMeta> states;

    //map of icons
    extern std::map<std::string, std::vector<unsigned short int>> icons;                       //large colour icons for things like the state menu
    extern std::map<std::string, std::vector<unsigned char>> small_icons;                      //smaller monochrome icons for use within GUIs

    // global variables
    extern int state;                                                                          //currently selected state
    extern int state_init;                                                                     //0 if the state is being executed for the first time (after swtiching from another stat)
    extern RTC_DATA_ATTR int selected_menu_icon;                                               //index of currently selected state

    //boot count is used to keep track of what state was selected in the menu before
    //entering deep sleep.  This variable is only used when going in to or waking up
    //from sleep.  During active mode operation, selected_menu_icon is used,
    extern RTC_DATA_ATTR int boot_count;                                                        //no of times watch has woken up (including initial boot)
    extern int trans_mode;                                                                      //pretty colour scheme
    extern int short_timeout;                                                                   //timeout when looking at watch face
    extern int long_timeout;                                                                    //timeout (almost) everywhere else
    extern bool timeout;                                                                        //whether or not to go to sleep after timeout time has elapsed
    extern int themecolour;                                                                     //colour of the system accent
    extern time_t alarm_snooze_time;                                                            //time to add to alarm when snoozing
    extern uint8_t screen_brightness;                                                           //brightness of screen, ranges from 0 (backlight off) to 15 (full brightness)
    extern uint8_t speaker_volume;                                                              //speaker volume, as controlled by audioI2S library.  ranges from 0 (no sound) to 21 (loudest)
    extern uint8_t torch_brightness;                                                            //brightness of the torch LED (pwm controlled, ranges from 0 (off) to 255 (fill brightnesss))
    extern int sd_state;                                                                        //state of the sd card
                                                                                                //0 - not initalised (red)
                                                                                                //1 - initalised with no errors (green)
                                                                                                //2 - card not present (blue)
    extern int RTC_DATA_ATTR stopwatch_timing;                                                  //stopwatch state
                                                                                                //0 - stopped
                                                                                                //1 - running
                                                                                                //2 - paused
    extern uint32_t RTC_DATA_ATTR stopwatch_epoch;                                              //time when the stopwatch was started
    extern uint32_t RTC_DATA_ATTR stopwatch_paused_diff;                                        //when the stopwatch is off or paused, stopwatch_epoch is set to the current time minus this value
                                                                                                //when the stopwatch is paused, this value is set to the difference between stopwatch_epoch and the current time (keeps the difference constant)
                                                                                                //when the stopwatch is off, this value is set to 0
    extern uint32_t stopwatch_time_diff;                                                        //difference between epoch and current time (equivalent to elapsed time)
    extern uint32_t stopwatch_last_time_diff ;                                                  //last time diff (used for checking when to redraw elapsed time)
    extern uint32_t stopwatch_ms, stopwatch_last_ms;
    extern uint32_t stopwatch_s, stopwatch_last_s;
    extern uint32_t stopwatch_min, stopwatch_last_min;
    extern uint32_t stopwatch_hour, stopwatch_last_hour;

    extern std::vector<timerData> timers;                                                      //vector to store timers
    extern std::vector<alarmData> alarms;                                                      //vector to store alarms
    extern int timer_trigger_status;                                                           //the state of a timer
                                                                                        //0 - timer not going off → normal state execution
                                                                                        //1 - timer going off → suspend state execution and draw alarm message
                                                                                        //2 - timer gone off → wait for user input before resuming state execution
    extern int timer_trigger_id;
    extern int alarm_trigger_status;
    extern int alarm_trigger_id;

    extern int file_select_status;                                                             //whether or not to show the file select menu
                                                                                        //0 - don't show the menu.  the current state (or alarm or timer dialogue) will show instead
                                                                                        //1 - do show the dialog
    extern std::string file_path;                                                                   //if the file select menu is active, this is used to keep track of the current directory.
                                                                                        //otherwise, it is used to store the path of the file after file selection (or "canceled" if no file was selected)
    extern bool file_select_dir_list_init;                                                     //keeps track of whether or not the file list has been initalised for the current directory

    //these variables stop button presses affecting new states
    //when switching from a previous state.
    //when a user presses a button to go from the watch face to the menu,
    //if the button is held down for long enough, the button press can affect
    //the next state.
    //these lock variables are set to true when switching states, then set to false
    //when each button is released.
    extern bool dpad_up_lock;
    extern bool dpad_down_lock;
    extern bool dpad_left_lock;
    extern bool dpad_right_lock;
    extern bool dpad_enter_lock;


    // system function prototypes

    void startLoop();
    void endLoop();

    //draw the status bar at the top of the page
    //light - if this is true, only the status icons will be drawn.  otherwise, everything will be drawn
    void    drawTopThing(bool light = false);

    //add an icon to the list of 16 bit colour icons
    //iconName - the name of the icon that will be used to access it later
    //icon - a vector or array of unsigned short ints that represent the icon
    bool    registerIcon(std::string iconName, std::vector<unsigned short int> icon);

    //add an icon to the list of smaller 8 bit monochrome icons
    //iconName - the name of the icon that will be used to access it later
    //icon - a vector or array of unsigned chars that represent the icon
    bool    registerSmallIcon(std::string iconName, std::vector<unsigned char> icon);

    //dim or make brighter the screen
    //direction - if this is false, the screen will dim.  if this is true, the screen will increase in brightness
    //pause_thing - the time in milliseconds between each step of brightness
    void    dimScreen(bool direction, int pause_thing);
    void    switchState(int newState, int variant = 0, int dim_pause_thing = 10, int bright_pause_thing = 10, bool dont_draw_first_frame = false);
    void    deepSleep(int pause_thing=10);
    void    drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, int colour);
    void    drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour);

    //method to return all the files in a directory (non-recursively)
    //path - the path of the directory to return files in
    std::vector<std::string> getDirFiles(std::string path);

    //open the file select dialogue.  this will pause the state until a file has been selected (or the operation has been cancelled)
    //path - the path to start the file selection at
    std::string beginFileSelect(std::string path = "/");
    int     initSD(bool handleCS = true);
    void    colour888(uint16_t colour, float *r, float *g, float *b);
    void    HSVtoRGB( float *r, float *g, float *b, float h, float s, float v );
    bool    getHeatMapColor(float value, float *red, float *green, float *blue);
    double  ReadVoltage(byte pin);
    int     something(int x, int y);

}

#endif /* WATCH2_H */
