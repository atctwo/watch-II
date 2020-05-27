#ifndef WATCH2_H
#define WATCH2_H

////////////////////////////////////////
// includes
////////////////////////////////////////


#include <stdio.h>                  // i don't actually know...
#include <stdint.h>
#include <algorithm>                // used for std::find and std::min and std::max
#include <stack>
#include <map>                      // map object for storing states
#include <unordered_map>
#include <string>                   // std::string
#include <functional>               // std::function thing

#include <Arduino.h>                // this will kind of be included anyway, it's just here to shut vscode up
#include <SPI.h>                    // SPI library
#include <SdFat.h>                  // sd card access library
#include <sdios.h>
#include <TFT_eSPI.h>
#include <FS.h>
//#include <Adafruit_ImageReader.h>
#include <JC_Button.h>              // button object
#include <WiFi.h>                   // wifi library
#include <WiFiClientSecure.h>       // https client library
#include <HTTPClient.h>
#include <BleKeyboard.h>            // for connecting to a device over BLE, and sending keyboard keys
#include <Preferences.h>            // for storing settings in nvs (allowing for persistance over power cycles)
#include <tinyexpr.h>               // expression evaluator for calculator
#include <cJSON.h>

#include <sys/cdefs.h>
#include <time.h>                   // used for system-level time keeping
#include <sys/time.h>               // see above
#include <TimeLib.h>                // used for managing time (see code note 1)
#include <TimeAlarms.h>             //used for creating and managing alarms

#include "watch2.h"                 // defines and function prototypes

// custom fonts
/*
#include "custom_fonts/sourcesanspro-regular-6.h"
#include "custom_fonts/sourcesanspro-light-8.h"
#include "custom_fonts/sourcesanspro-light-12.h"
#include "custom_fonts/sourcesanspro-regular-16.h"
#include "custom_fonts/sourcesanspro-light-16.h"
#include "custom_fonts/sourcesanspro-regular-24.h"
#include "custom_fonts/sourcesanspro-light-24.h"
*/

#include "coolcrab.h"
#include "regret.h"

//pin declarations
#define cs   5          // goes to TFT CS
#define sdcs 4          //sd card chip select
#define spi_dc   22     // goes to TFT DC
#define spi_mosi 23     // goes to TFT MOSI
#define spi_sclk 18     // goes to TFT SCK/CLK
#define spi_rst  21     // ESP RST to TFT RESET
#define spi_miso 19     // Not connected
//       3.3V           // Goes to TFT LED
//       5v             // Goes to TFT Vcc
//       Gnd            // Goes to TFT Gnd
#define tftbl 14        //tft backlight
#define tftbl_resolution 8 //resolution of backlight pwm in bits

#define dpad_up     33
#define dpad_down   27
#define dpad_left   32
#define dpad_right  25
#define dpad_enter  26
#define BATTERY_DIVIDER_PIN 34
#define TORCH_PIN 13
#define IR_PIN 12
#define IR_REC_PIN 16

//ledc channels
#define TORCH_PWM_CHANNEL 0
#define TFTBL_PWM_CHANNEL 1
#define IR_PWM_CHANNEL 2

//button active macros
#define KEY_REPEAT_DELAY    550     //time for key repeat to start, in ms [DAS]
#define KEY_REPEAT_PERIOD   24      //time between key repeats, in ms     [ARR]

#define dpad_up_active()    ( !watch2::dpad_up_lock &&    (watch2::btn_dpad_up.wasPressed() || ( watch2::btn_dpad_up.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 10 ) ) ) )
#define dpad_down_active()  ( !watch2::dpad_down_lock &&  (watch2::btn_dpad_down.wasPressed() || ( watch2::btn_dpad_down.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 10 ) ) ) )
#define dpad_left_active()  ( !watch2::dpad_left_lock &&  (watch2::btn_dpad_left.wasPressed() || ( watch2::btn_dpad_left.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 10 ) ) ) )
#define dpad_right_active() ( !watch2::dpad_right_lock && (watch2::btn_dpad_right.wasPressed() || ( watch2::btn_dpad_right.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 10 ) ) ) )
#define dpad_enter_active() ( !watch2::dpad_enter_lock && (watch2::btn_dpad_enter.wasPressed() || ( watch2::btn_dpad_enter.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD < 10 ) ) ) )
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

// font declarations
#define MAIN_FONT               "HelvetiHand20"
#define SLIGHTLY_BIGGER_FONT    "HelvetiHand24"
#define LARGE_FONT              "HelvetiHand36"
#define REALLY_BIG_FONT         "HelvetiHand60"
#define REALLY_REALLY_BIG_FONT  "HelvetiHand90"

// device info
#define SCREEN_WIDTH            240
#define SCREEN_HEIGHT           240
#define WATCH_VER               "0.1"
#define BATTERY_VOLTAGE_MAX     3.3     //max voltage of battery (when fully charged)
#define BATTERY_VOLTAGE_SCALE   2       //scale factor of voltage divider
                                        //make sure the battery voltage (when scaled) is not above 3.3v.
                                        //a LiPo that maxes out at 4.7v will be scaled to 2.35v, which is
                                        //fine for the ESP32.
#define NTP_SERVER              "pool.ntp.org"
#define PORT_HTTP               80
#define PORT_HTTPS              443
#define WIFI_PROFILES_FILENAME  "/wifi_profiles.json"
//Calculator definitions
#define CALC_CELL_WIDTH   25//27 for 4 chrs
#define CALC_CELL_HEIGHT  12

// root ca certs
extern const char *root_ca_wikipedia;
extern const char *root_ca_jigsaw;
extern const char *root_ca_open_trivia_db;

/**
 * functions and variables that relate to the running of the watch ii system.
 */
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
        bool                    framebuffer;
        bool                    hidden;
        
        //stateMeta constructor
        stateMeta(
            std::string stateName, 
            std::function<void()> stateFunc,
            std::string stateIcon = "what",
            int variant = 0,
            bool framebuffer = false,
            bool hidden = false
        ) : 
            stateName(stateName),
            stateFunc(stateFunc),
            stateIcon(stateIcon),
            variant(variant),
            framebuffer(framebuffer),
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

    struct imageData {

        unsigned char               *data;
        uint16_t                    width;
        uint16_t                    height;
        const char                  *error;

    };

    // type definitions
    typedef void (*func)(void);                                                         // function pointer type

    // object creation
    extern SPIClass *vspi;                                                                     // VSPI object
    extern TFT_eSPI oled;                                                              //hw spi (use vspi or &SPI)
    extern Preferences preferences;                                                            //wrapper for esp32 nvs used to store system settings
    extern SdFat SD;    
    //extern Adafruit_ImageReader reader;                                                                       //sdfat instance used for accessing sd card
    //extern Adafruit_ImageReader flash_reader;
    extern TFT_eSprite top_thing;
    extern TFT_eSprite framebuffer;
    extern WiFiClientSecure wifi_client;
    extern BleKeyboard *ble_keyboard;

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
    extern uint8_t top_thing_height;                                                            //the height of the top thing (plus a small buffer) in pixels
    extern bool forceRedraw;

    // local stores of system preferences
    extern uint16_t trans_mode;                                                                 //pretty colour scheme
    extern bool animate_watch_face;                                                             //whether or not animate the watch face
    extern int short_timeout;                                                                   //timeout when looking at watch face
    extern int long_timeout;                                                                    //timeout (almost) everywhere else
    extern bool timeout;                                                                        //whether or not to go to sleep after timeout time has elapsed
    extern int themecolour;                                                                     //colour of the system accent
    extern time_t alarm_snooze_time;                                                            //time to add to alarm when snoozing
    extern uint16_t screen_brightness;                                                           //brightness of screen, ranges from 0 (backlight off) to 15 (full brightness)
    extern uint8_t speaker_volume;                                                              //speaker volume, as controlled by audioI2S library.  ranges from 0 (no sound) to 21 (loudest)
    extern uint8_t torch_brightness;                                                            //brightness of the torch LED (pwm controlled, ranges from 0 (off) to 255 (fill brightnesss))
    extern int8_t timezone;                                                                    // the user's timezone, in the format UTC+n
    extern bool ntp_wakeup_connect;
    extern bool ntp_boot_connect;
    extern bool ntp_boot_connected;
    extern bool wifi_wakeup_reconnect;                                                          // whether or not to attempt to reconnect to wifi on wake up (if not already connected)
    extern bool wifi_boot_reconnect;                                                            // whether or not to attemp to connect to wifi on boot
    extern bool wifi_enabled;                                                                   // whether or not wifi is enabled.  don't use this if you actually want to know if the system is
                                                                                                // connected to wifi, for that use wifi_state.  this is only used to decide whether or not to enable
                                                                                                // wifi automatically on boot
    extern wifi_auth_mode_t wifi_encryption;                                                    // hack hack hack hack pls replace with a way to get the encryption type of the current AP

    // fs states
    extern int sd_state;                                                                        //state of the sd card
                                                                                                //0 - not initalised (red)
                                                                                                //1 - initalised with no errors (green)
                                                                                                //2 - card not present (blue)
    extern bool spiffs_state;                                                                   // the state of the spiffs
                                                                                                // -1 - failed to initalise
                                                                                                // 0 - not initalised
                                                                                                // 1 - initalised successfully
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
    
    extern uint8_t wifi_state;                                                          // keeps track of whether or not the user has enabled the wifi
                                                                                        // 0 - disabled by user
                                                                                        // 1 - enabled by user, idle / disconnected
                                                                                        // 2 - enabled by user, connecting
                                                                                        // 3 - enabled by user, connected
                                                                                        // 4 - enabled by user, pls connect asap (used when connecting to an AP after booting)
    extern uint8_t wifi_reconnect_attempts;                                             // how many more times to attempt to connect to the next most recently connected AP.
                                                                                        // if this isn't zero, and wifi state is set to 4, the system will try to connect to an AP, starting with
                                                                                        // the one what was connected to most recently.  if the system can't connect to that AP, it will decrement
                                                                                        // this number, and try to connect to the next most recently connected AP, until this number reaches zero,
                                                                                        // of the system runs out of saved profiles.  if you set this variable, make sure to set
                                                                                        // initial_wifi_reconnect_attempts to the same value.
    extern uint8_t initial_wifi_reconnect_attempts;                                     // this variable should reflect that value that wifi_reconnect_attempts started off as.
    extern uint32_t wifi_connect_timeout;                                               // how long to wait before giving up on connecting to a wifi network, in milliseconds
    extern uint32_t wifi_connect_timeout_start;                                         // used to keep track of time when connecting to a wifi network

    extern uint8_t bluetooth_state;                                                     // keeps track of the state of the bluetooth subsystem
                                                                                        // 0 - disabled
                                                                                        // 1 - enabling
                                                                                        // 2 - enabled, disconnected
                                                                                        // 3 - enabled, connected

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
    void    switchState(int newState, int variant = 0, int dim_pause_thing = 250, int bright_pause_thing = 250, bool dont_draw_first_frame = false);
    void    deepSleep(int pause_thing=10);
    void    drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, bool scroll=true, int colour=themecolour);
    void    drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour=themecolour);

    //method to return all the files in a directory (non-recursively)
    //path - the path of the directory to return files in
    std::vector<std::string> getDirFiles(std::string path);

    //open the file select dialogue.  this will pause the state until a file has been selected (or the operation has been cancelled)
    //path - the path to start the file selection at
    std::string beginFileSelect(std::string path = "/");
    std::string textFieldDialogue(std::string prompt="", const char *default_input="", const char mask=0, bool clear_screen=true);
    int     initSD(bool handleCS = true);
    void    colour888(uint16_t colour, float *r, float *g, float *b);
    void    HSVtoRGB( float *r, float *g, float *b, float h, float s, float v );
    bool    getHeatMapColor(float value, float *red, float *green, float *blue);
    double  ReadVoltage(byte pin);
    void    setFont(const char* font, TFT_eSPI &tft = oled, fs::FS &ffs = SPIFFS);
    int     something(int x, int y);

    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    const char *humanSize(uint64_t bytes);
    uint16_t read16(fs::File &f);
    uint32_t read32(fs::File &f);
    void drawBmp(const char *filename, int16_t x, int16_t y);
    imageData getImageData(const char *filename);
    const char* drawImage(imageData data, int16_t img_x, int16_t img_y);

    void enable_wifi(bool connect=true);
    void disable_wifi();
    void connectToWifiAP(const char *ssid="", const char *password="");
    void getTimeFromNTP();

    cJSON *getWifiProfiles();
    void setWifiProfiles(cJSON *profiles);

    void enable_bluetooth();
    void disable_bluetooth();

    //functions for stb_image
    int img_read(void *user,  char *data, int size);
    void img_skip(void *user, int n);
    int img_eof(void *user);

}

#endif /* WATCH2_H */
