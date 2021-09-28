/**
 * @file watch2.h
 * @author atctwo
 * @brief the header file for each of the watch2_*.cpp files
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

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
#include <TFT_eSPI.h>               // TFT library
#include <FS.h>                     // Arduino filesystem abstraction
#include <SD.h>                     // for talking to the SD card
#include <JC_Button.h>              // button object
#include <WiFi.h>                   // wifi library
#include <WiFiClientSecure.h>       // https client library
#include <HTTPClient.h>             // http client library
#include <Audio.h>                  // for audio playback
#include <Preferences.h>            // for storing settings in nvs (allowing for persistance over power cycles)
#include <tinyexpr.h>               // expression evaluator for calculator
#include <cJSON.h>                  // JSON parser
#include <esp_log.h>                // ESP-IDF logging
#include <Wire.h>                   // Arduino I2C library
#include <Adafruit_MCP23008.h>      // MCP23008 I2C IO expander
#include <Adafruit_MCP9808.h>       // MCP9808 I2C temperature sensor
#include <Adafruit_LC709203F.h>     // LC709203F I2C fuel gauge
#include <DS1337.h>                 // DS1337 I2C RTC

#include <NimBLEDevice.h>
#include <NimBLEUtils.h>
#include <NimBLEServer.h>
#include <NimBLEHIDDevice.h>
#include <HIDKeyboardTypes.h>

#include <sys/cdefs.h>
#include <time.h>                   // used for system-level time keeping
#include <sys/time.h>               // see above
#include <TimeLib.h>                // used for managing time (see code note 1)
#include <TimeAlarms.h>             //used for creating and managing alarms

#include "watch2.h"                 // defines and function prototypes
#include "coolcrab.h"
#include "regret.h"

//pin declarations
#define cs   5         // goes to TFT CS
#define sdcs 13          //sd card chip select
#define spi_dc   22     // goes to TFT DC
#define spi_mosi 23     // goes to TFT MOSI
#define spi_sclk 18     // goes to TFT SCK/CLK
#define spi_rst  21     // ESP RST to TFT RESET
#define spi_miso 19     // Not connected
//       3.3V           // Goes to TFT LED
//       5v             // Goes to TFT Vcc
//       Gnd            // Goes to TFT Gnd
#define tftbl 33        //tft backlight
#define tftbl_resolution 8 //resolution of backlight pwm in bits

#define dpad_up             1
#define dpad_down           3
#define dpad_left           4
#define dpad_right          0
#define dpad_enter          2
#define BATTERY_DIVIDER_PIN 34
#define TORCH_PIN           14
#define IR_SEND_PIN         4
#define IR_REC_PIN          12
#define I2S_DOUT            25
#define I2S_DIN             35
#define I2S_BCLK            27
#define I2S_LRC             26
#define I2C_SDA             32
#define I2C_SCL             15
#define SHUTDOWN_PIN        7 // mcp23008

//ledc channels
#define TORCH_PWM_CHANNEL 2
#define TFTBL_PWM_CHANNEL 1
#define IR_PWM_CHANNEL 0

//button active macros
#define KEY_REPEAT_DELAY    700     //time for key repeat to start, in ms [DAS]
#define KEY_REPEAT_PERIOD   50      //time between key repeats, in ms     [ARR]

#define dpad_up_active()    watch2::dpad_active(dpad_up)
#define dpad_down_active()  watch2::dpad_active(dpad_down)
#define dpad_left_active()  watch2::dpad_active(dpad_left)
#define dpad_right_active() watch2::dpad_active(dpad_right)
#define dpad_enter_active() watch2::dpad_active(dpad_enter)
#define dpad_any_active()   ( dpad_up_active() || dpad_down_active() || dpad_left_active() || dpad_right_active() || dpad_enter_active() )

#define draw(conditions, ...)                                       \      
    if (!watch2::state_init || watch2::forceRedraw || conditions)   \
    {                                                               \
        __VA_ARGS__                                                 \
        watch2::forceRedraw = false;                                \
    }

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

#define MPS_TO_MPH 2.23694                              // multiply a value in metres per second to get the value in miles per hour

// font declarations
#define MAIN_FONT               "HelvetiHand20"
#define SLIGHTLY_BIGGER_FONT    "HelvetiHand24"
#define LARGE_FONT              "HelvetiHand36"
#define REALLY_BIG_FONT         "HelvetiHand60"
#define REALLY_REALLY_BIG_FONT  "HelvetiHand90"

// i2c addresses
#define I2C_ADDRESS_MCP9098     0x18    // temperature sensor
#define I2C_ADDRESS_MCP23008    0x20    // io expander
#define I2C_ADDRESS_MAX17043    0x36    // lipo fuel gauge (not used)
#define I2C_ADDRESS_LC709203F   0x0B    // lipo fuel gauge
#define I2C_ADDRESS_SHTC3       0x70    // temperature + humidity
#define I2C_ADDRESS_DS1337      0x68    // real time clock

// device info
#define SCREEN_WIDTH            240                     // the width of the screen in pixels
#define SCREEN_HEIGHT           240                     // the height of the screen in pixels
#define WATCH_VER               "0.1"                   // the version of the watch2 firmware
#define BATTERY_VOLTAGE_MAX     3.3                     // max voltage of battery (when fully charged)
#define BATTERY_VOLTAGE_SCALE   2                       // scale factor of voltage divider
                                                        // make sure the battery voltage (when scaled) is not above 3.3v.
                                                        // a LiPo that maxes out at 4.7v will be scaled to 2.35v, which is
                                                        // fine for the ESP32.
#define NTP_SERVER              "pool.ntp.org"          // the server to use for NTP
#define PORT_HTTP               80                      // the tcp port used for HTTP
#define PORT_HTTPS              443                     // the tcp port used for HTTPS
#define WIFI_PROFILES_FILENAME  "/wifi_profiles.json"   //!< the name of the file that stores wifi profiles
#define API_KEYS_FILENAME       "/api_keys.json"        //!< the name of the file that is used to store API keys for lots of REST APIs
#define RADIO_FILENAME          "/radio.txt"            //!< the name of the file that contains radio HTTP streams
#define WATCH2_TAG              "watch2"                // esp-idf logging tag

//Calculator definitions
#define CALC_CELL_WIDTH   25//27 for 4 chrs
#define CALC_CELL_HEIGHT  12

// root ca certs
extern const char *root_ca_reddit;
extern const char *root_ca_wikipedia;
extern const char *root_ca_jigsaw;
extern const char *root_ca_open_trivia_db;

/**
 * functions and variables that relate to the running of the watch ii system.
 */
namespace watch2
{
    // struct definitions

    /**
     * @brief describes state metadata
     * 
     * this struct stores lots of data about states.
     */
    struct stateMeta {
        
        std::string             stateName;              //!< the name of the state
        std::function<void()>   stateFunc;              //!< the function that is executed when the state is loaded
        std::string             stateIcon;              //!< the name of the icon that is used for the state
        int                     variant;                //!< the current state variant
        bool                    framebuffer;            //!< whether or not the state uses a framebuffer (this doesn't do anything yet)
        bool                    hidden;                 //!< if this is true, the state won't show up on the state menu
        
        /**
         * @brief stateMeta constructor
         * 
         * @param stateName the name of the state
         * @param stateFunc the function that is executed when the state is loaded
         * @param stateIcon the name of the icon that is used for the state
         * @param variant the current state variant
         * @param framebuffer whether or not the state uses a framebuffer (this doesn't do anything yet)
         * @param hidden if this is true, the state won't show up on the state menu
         */
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

    /**
     * @brief data for entries menus drawn using `drawSettingsMenu()`.
     * 
     * Settings entries have a title, which is shown beside a spinner that lets the user select between different options.  Each entry is represented using
     * a `settingsMenuData` struct, which is passed to `drawSettingsMenu()` in an array of `settingsMenuData`s.  The currently selected option for the entry 
     * is specified using the `setting_value` member.  `text_map` can be used to specify text that corresponds to each possible setting value.  For example, if 
     * `setting_value` is 0, and `text_map` has a 0th element, the spinner will show whatever `text_map[0]` is, instead of `0`.
     * 
     * The spinner will try to only take up as much space as needed, but you can specify a mimimum width using `min_field_width`.
     */
    struct settingsMenuData {

        String                      setting_title;          //!< the setting prompt or name or title
        int                         setting_value;          //!< the current setting value, or index of `text_map`
        std::vector<String>         text_map;               //!< an array of `String`s that can represent different spinner options
        int                         min_field_width;        //!< the mimimum width of the spinner

    };

    /**
     * @brief data for timers
     * 
     */
    struct timerData {

        /**
         * @brief the time left on a timer (in seconds).  
         * 
         * this is only updated when the timer is paused, so
         * don't rely on this for when the timer is not paused.  when the timer
         * is paused, the Alarm ID is freed, and timer (at least according to
         * the TimeAlarms lib) ceases to exist.  this duration value is used to
         * create a new timer when the timer is resumed.  this makes it look like
         * the timer is actually paused.
         */
        time_t                      duration;

        /**
         * @brief the original time that the timer was set to when it was started
         * 
         */
        time_t                      initial_duration;

        /**
         * @brief the id of the timer.
         * 
         * if the timer is not paused, then it will be associated with a TimeAlarms
         * timer.  this member stores the id of the timer.  when the timer is paused,
         * the TimeAlarms timer does not exist (see above), so this variable is set
         * to 255.  this can be used to check whether or not the timer is paused.
         */
        AlarmID_t                   alarm_id;

        /**
         * @brief the time at which the TimeAlarms timer is started.  
         * 
         * if the timer is paused and resumed, this member stored the time at which 
         * the timer was resumed.  should be 255 if the timer has not yet started, or 
         * the timer is paused.
         */
        time_t                      time_started;

        /**
         * @brief the function to execute once the timer has completed
         * 
         */
        OnTick_t                    on_tick_handler;

        /**
         * @brief the previous value of the timer.  this is used for redrawing a timer to the screen.
         */
        time_t                      last_value;

    };

    /**
     * @brief data for alarms
     * 
     */
    struct alarmData {

        AlarmID_t                   alarm_id;       //!< the alarm ID
        time_t                      initial_time;   //!< the time the alarm was originally set to when the alarm was started
        bool                        paused;         //!< whether or not the alarm is paused
        time_t                      last_value;     //!< used when drawing the alarm time

    };

    /**
     * @brief data that relates to a loaded image.
     * 
     */
    struct imageData {

        /**
         * @brief the raw image data
         * 
         * This member stores an array of pixels.  The array stores each red, green, and blue value one after another, so each pixel takes up 3 array 
         * elements.  The colour is stored in 24 bit RGB format.  Here is a terrible diagram to show this:
         * ```
         * array            [R][G][B][R][G][B][R][G][B][R][G][B][R][G][B][R][G][B][R][G][B][R][G][B][R][G][B][R][G][B]...
         * element number   0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29
         * pixel number     |   0    |    1   |    2   |    3   |    4   |    5   |    6   |    7   |    8   |    9   |
         * ``` 
         */
        imageData()=default;
        unsigned char               *data; 
        uint16_t                    width;          //!< the width of the image
        uint16_t                    height;         //!< the height of the image
        const char                  *error;         //!< if there was an error when loading the image, this member will store it

    };

    enum fs_icon {
        FS_ICON_BLANK = 0,
        FS_ICON_CANCEL,
        FS_ICON_FOLDER,
        FS_ICON_FILE_GENERIC,
        FS_ICON_FILE_IMAGE,
        FS_ICON_FILE_AUDIO,
        FS_ICON_FILE_VIDEO,
        FS_ICON_FILE_COMPRESSED,
        FS_ICON_FILE_CODE,
        FS_ICON_FILE_DOCUMENT,
        FS_ICON_FILE_FONT
    };

    // ble hid consumer control reports
    // stolen from https://github.com/T-vK/ESP32-BLE-Keyboard/blob/master/BleKeyboard.h
    typedef uint8_t MediaKeyReport[2];

    const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
    const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
    const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
    const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
    const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
    const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
    const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};
    const MediaKeyReport KEY_MEDIA_WWW_HOME = {128, 0};
    const MediaKeyReport KEY_MEDIA_LOCAL_MACHINE_BROWSER = {0, 1}; // Opens "My Computer" on Windows
    const MediaKeyReport KEY_MEDIA_CALCULATOR = {0, 2};
    const MediaKeyReport KEY_MEDIA_WWW_BOOKMARKS = {0, 4};
    const MediaKeyReport KEY_MEDIA_WWW_SEARCH = {0, 8};
    const MediaKeyReport KEY_MEDIA_WWW_STOP = {0, 16};
    const MediaKeyReport KEY_MEDIA_WWW_BACK = {0, 32};
    const MediaKeyReport KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION = {0, 64}; // Media Selection
    const MediaKeyReport KEY_MEDIA_EMAIL_READER = {0, 128};

    // type definitions
    typedef void (*func)(void);                                                                 //!< function pointer type

    // object creation
    extern SPIClass *vspi;                                                                      //!< VSPI object
    extern TFT_eSPI oled;                                                                       //!< hw spi (use vspi or &SPI)
    extern Preferences preferences;                                                             //!< wrapper for esp32 nvs used to store system settings
    //extern SdFat sdcard;                                                                            //!< instance of SdFat used to access the sd card
    //extern Adafruit_ImageReader reader;                                                     
    //extern Adafruit_ImageReader flash_reader;
    extern TFT_eSprite top_thing;                                                               //!< the framebuffer that the top thing is drawn onto
    extern TFT_eSprite framebuffer;                                                             //!< another framebuffer that isn't used
    extern WiFiClient wifi_client;
    extern WiFiClientSecure wifi_client_secure;                                                        //!< the wifi client used for HTTP and HTTPS requests
    //extern BleKeyboard ble_keyboard;                                                            //!< a thing that handles BLE HID Keyboard stuff
    extern Audio audio;                                                                         //!< audio playback!
    extern Adafruit_MCP23008 mcp;                                                               //!< MCP23008 IO expander for user input
    extern Adafruit_MCP9808 temperature;                                                        //!< MCP9808 temperature sensor
    extern Adafruit_LC709203F fuel_gauge;                                                       //!< LC709203F fuel gauge

    //button objects
    extern Button btn_dpad_up;                                                         //!< the object that handles the dpad up button
    extern Button btn_dpad_down;                                                       //!< the object that handles the dpad down button
    extern Button btn_dpad_left;                                                       //!< the object that handles the dpad left button
    extern Button btn_dpad_right;                                                      //!< the object that handles the dpad right button
    extern Button btn_dpad_enter;                                                      //!< the object that handles the dpad enter button
    extern Button btn_zero;                                                                     //!< the object that handles GPIO0 input

    extern std::vector<stateMeta> states;                                                       //<! vector of watch states

    //map of icons
    extern std::map<std::string, imageData> *icons;                                             //!< large colour icons for things like the state menu (16 bit 565 colour)
    extern std::map<std::string, std::vector<unsigned char>> *small_icons;                      //!< smaller monochrome icons for use within GUIs (8 bit colour)
    
    // fs icon maps
    extern std::unordered_map<std::string, fs_icon> fs_icon_ext_map;                            //!< a map of file extensions to fs icon identifiers
    extern std::unordered_map<fs_icon, std::string> fs_icon_name_map;                           //!< a map of fs icon identifiers to icon names

    // ble characteristics and services
    extern NimBLEService *batteryService;
    extern NimBLEHIDDevice *ble_hid;

    // global variables
    extern int state;                                                                           //!< currently selected state
    extern int state_init;                                                                      //!< 0 if the state is being executed for the first time (after swtiching from another state)
    extern int last_variant;                                                                    //!< the variant before the most recent call to switchState()
    extern RTC_DATA_ATTR int selected_menu_icon;                                                //!< index of currently selected state
    extern RTC_DATA_ATTR int boot_count;                                                        //!< no of times watch has woken up (including initial boot)
                                                                                                //!< boot count is used to keep track of what state was selected in the menu before
                                                                                                //!< entering deep sleep.  This variable is only used when going in to or waking up
                                                                                                //!< from sleep.  During active mode operation, selected_menu_icon is used,
    extern bool is_fuel_gauge_present;
    extern uint8_t top_thing_height;                                                            //!< the height of the top thing (plus a small buffer) in pixels
    extern bool forceRedraw;
    extern bool forceRedrawLooped;
    extern bool showingControlCentre;                                                           //!< whether or not the ripoff control centre is being shown
    // local stores of system preferences
    extern uint16_t trans_mode;                                                                 //!< pretty colour scheme
    extern bool animate_watch_face;                                                             //!< whether or not animate the watch face
    extern int short_timeout;                                                                   //!< timeout when looking at watch face
    extern int long_timeout;                                                                    //!< timeout (almost) everywhere else
    extern bool timeout;                                                                        //!< whether or not to go to sleep after timeout time has elapsed
    extern uint32_t last_button_press;                                                          //!< the time of the most recent button press
    extern int themecolour;                                                                     //!< colour of the system accent
    extern time_t alarm_snooze_time;                                                            //!< time to add to alarm when snoozing
    extern uint16_t screen_brightness;                                                          //!< brightness of screen, ranges from 0 (backlight off) to 15 (full brightness)
    extern uint8_t speaker_volume;                                                              //!< speaker volume, as controlled by audioI2S library.  ranges from 0 (no sound) to 21 (loudest)
    extern uint8_t torch_brightness;                                                            //!< brightness of the torch LED (pwm controlled, ranges from 0 (off) to 255 (fill brightnesss))
    extern int8_t timezone;                                                                     //!< the user's timezone, in the format UTC+n
    extern bool ntp_wakeup_connect;                                                             //!< whether or not to attempt to get the time on wakeup
    extern bool ntp_boot_connect;                                                               //!< whether or not to attempt to get the time on boot
    extern bool ntp_boot_connected;                                                             //!< whether or not the time has been retrieved on boot
    extern bool wifi_wakeup_reconnect;                                                          //!< whether or not to attempt to reconnect to wifi on wake up (if not already connected)
    extern bool wifi_boot_reconnect;                                                            //!< whether or not to attemp to connect to wifi on boot
    extern bool wifi_enabled;                                                                   //!< whether or not wifi is enabled.  don't use this if you actually want to know if the system is
                                                                                                //!< connected to wifi, for that use wifi_state.  this is only used to decide whether or not to enable
                                                                                                //!< wifi automatically on boot
    extern String weather_location;                                                             //!< the city name to use when getting the weather
    extern String timer_music;                                                                  //!< the music played when a timer elapses
    extern String alarm_music;                                                                  //!< the music played when an alarm elapses

    // fs states
    extern int sd_state;                                                                        //!< state of the sd card
                                                                                                //!< 0 - not initalised (red)
                                                                                                //!< 1 - initalised with no errors (green)
                                                                                                //!< 2 - card not present (blue)
    extern bool spiffs_state;                                                                   //!<  the state of the spiffs
                                                                                                //!<  -1 - failed to initalise
                                                                                                //!<  0 - not initalised
                                                                                                //!<  1 - initalised successfully
    extern int RTC_DATA_ATTR stopwatch_timing;                                                  //!< stopwatch state
                                                                                                //!< 0 - stopped
                                                                                                //!< 1 - running
                                                                                                //!< 2 - paused
    extern uint32_t RTC_DATA_ATTR stopwatch_epoch;                                              //!< time when the stopwatch was started
    extern uint32_t RTC_DATA_ATTR stopwatch_paused_diff;                                        //!< when the stopwatch is off or paused, stopwatch_epoch is set to the current time minus this value
                                                                                                //!< when the stopwatch is paused, this value is set to the difference between stopwatch_epoch and the current time (keeps the difference constant)
                                                                                                //!< when the stopwatch is off, this value is set to 0
    extern uint32_t stopwatch_time_diff;                                                        //!< difference between epoch and current time (equivalent to elapsed time)
    extern uint32_t stopwatch_last_time_diff ;                                                  //!< last time diff (used for checking when to redraw elapsed time)
    extern uint32_t stopwatch_ms, stopwatch_last_ms;
    extern uint32_t stopwatch_s, stopwatch_last_s;
    extern uint32_t stopwatch_min, stopwatch_last_min;
    extern uint32_t stopwatch_hour, stopwatch_last_hour;

    extern std::vector<timerData> timers;                                                       //!< vector to store timers
    extern std::vector<alarmData> alarms;                                                       //!< vector to store alarms
    extern int timer_trigger_status;                                                            //!< the state of a timer
                                                                                                //!< 0 - timer not going off → normal state execution
                                                                                                //!< 1 - timer going off → suspend state execution and draw alarm message
                                                                                                //!< 2 - timer gone off → wait for user input before resuming state execution
    extern int timer_trigger_id;
    extern int alarm_trigger_status;
    extern int alarm_trigger_id;

    extern int file_select_status;                                                              //!< whether or not to show the file select menu
                                                                                                //!< 0 - don't show the menu.  the current state (or alarm or timer dialogue) will show instead
                                                                                                //!< 1 - do show the dialog
    extern std::string file_path;                                                               //!< if the file select menu is active, this is used to keep track of the current directory.
                                                                                                //!< otherwise, it is used to store the path of the file after file selection (or "canceled" if no file was selected)
    extern bool file_select_dir_list_init;                                                      //!< keeps track of whether or not the file list has been initalised for the current directory
    
    extern uint8_t wifi_state;                                                                  //!< keeps track of whether or not the user has enabled the wifi
                                                                                                //!< 0 - disabled by user
                                                                                                //!< 1 - enabled by user, idle / disconnected
                                                                                                //!< 2 - enabled by user, connecting
                                                                                                //!< 3 - enabled by user, connected
                                                                                                //!< 4 - enabled by user, pls connect asap (used when connecting to an AP after booting)
    extern uint8_t wifi_reconnect_attempts;                                                     //!< how many more times to attempt to connect to the next most recently connected AP.
                                                                                                //!< if this isn't zero, and wifi state is set to 4, the system will try to connect to an AP, starting with
                                                                                                //!< the one what was connected to most recently.  if the system can't connect to that AP, it will decrement
                                                                                                //!< this number, and try to connect to the next most recently connected AP, until this number reaches zero,
                                                                                                //!< of the system runs out of saved profiles.  if you set this variable, make sure to set
                                                                                                //!< initial_wifi_reconnect_attempts to the same value.
    extern uint8_t initial_wifi_reconnect_attempts;                                             //!< this variable should reflect that value that wifi_reconnect_attempts started off as.
    extern uint32_t wifi_connect_timeout;                                                       //!< how long to wait before giving up on connecting to a wifi network, in milliseconds
    extern uint32_t wifi_connect_timeout_start;                                                 //!< used to keep track of time when connecting to a wifi network

    extern uint8_t bluetooth_state;                                                             //!< keeps track of the state of the bluetooth subsystem
                                                                                                //!< 0 - disabled
                                                                                                //!< 1 - enabling
                                                                                                //!< 2 - enabled, disconnected
                                                                                                //!< 3 - enabled, connected
    extern bool ble_set_up;                                                                     //!< has the ble server been created?

    extern wifi_auth_mode_t wifi_encryption;                                                    //!< hack hack hack hack pls replace with a way to get the encryption type of the current AP
    extern TaskHandle_t audio_task_handle;                                                      //!< handle to the audio task
    extern bool audio_repeat;                                                                   //!< if this is true, audio files will repeat once they have ended
    extern std::string audio_filename;                                                          //!< the filename of the currently playing audio file
    extern fs::FS *audio_fs;                                                                    //!< the FS from which the currently playing audio is playing
    extern std::string wfs;
    extern bool updated_track_info;                                                             //!< true if any track info has just been updated; pls clear this after processing new metadata

    extern std::string track_name;                                                              //!< name of the track (from id3 tags)
    extern std::string track_album;                                                             //!< name of the album (from id3 tags)
    extern std::string track_artist;                                                            //!< name of the artist (from id3 tags)
    extern std::string track_station;                                                           //!< name of the radio station
    extern std::string track_streamtitle;                                                       //!< title of the currently playing track (from a radio station)

    // these variables stop button presses affecting new states when switching from a previous state.  when a user presses a button to go from the watch face 
    // to the menu, if the button is held down for long enough, the button press can affect the next state. these lock variables are set to true when switching 
    // states, then set to false when each button is released.
    extern bool btn0_lock;
    extern bool dpad_lock[5];                   // whether or not a button is locked
    extern bool dpad_pressed[5];                // whether or not a button was pressed (once a button is pressed, this will only be true for one main loop iteration)
    extern bool dpad_held[5];                   // whether or not a button is being held down
    extern uint32_t dpad_pressed_time[5];       // the time of the most recent button press
    extern uint32_t dpad_last_repeat[5];        // if the button is repeating, the time since the last repeat







    //--------------------------------------
    // system function prototypes
    //--------------------------------------


    /**
     * @brief does all the things that need to be done at the start of each loop
     */
    void startLoop();

    /**
     * @brief does all the things that need to be done at the end of each loop
     */
    void endLoop();

    /**
     * @brief switch to a different state or app.
     * 
     * this method lets you switch from running the current state, to another state (or the current state, but using a different variant).
     * 
     * @param newState the id of the state to switch to.  the state menu will always be state 2
     * @param variant the state variant to switch to
     * @param dim_pause_thing the time that should be taken to fade out the current state
     * @param bright_pause_thing the time that should be taken to fade in the new state
     * @param dont_draw_first_frame normally, the new state function is executed once before it is faded in.  set this to true to stop this.
     */
    void    switchState(int newState, int variant = 0, int dim_pause_thing = 250, int bright_pause_thing = 250, bool dont_draw_first_frame = false);

    /**
     * @brief dim or make brighter the screen.
     * @param direction if this is false, the screen will dim.  if this is true, the screen will increase in brightness
     * @param pause_thing the time it should take to fade the screen
     */
    void    dimScreen(bool direction, int pause_thing);

    /**
     * @brief puts the system in sleep mode.
     * 
     * this turns off most of the ESP32's peripherals, and internally uses the ESP32's light sleep system.  this handles things before and after sleeping.  this
     * function will automatically handle fading out the screen.  i don't know what else to write... aaa!  i am bored!  wow!
     * 
     * @param pause_thing the time that it should take for the screen to fade out
     */
    void    deepSleep(int pause_thing=10);


    bool dpad_active(int key);



    //--------------------------------------
    // utility function prototypes
    //--------------------------------------


    /**
     * @brief gets an API key stored in the api key file in SPIFFS.
     * The filename is stored by the watch with the API_KEYS_FILENAME define.  Keys are stored in the file in a JSON format.  You can
     * also store any extra information that is needed.  You can specify what service you want to get the key for using the `service`
     * parameter, and you can specify what field you want using the `field` parameter.  Here's an example of the format used to store
     * keys:
     * ```json
     * {
     *     "service1": {
     *          "key": "abcdefg123456"
     *      },
     * "    service2: {
     *          "key": "bpwpb42",
     *          "user": "icantthinkofagoodusername"
     *      }
     * }
     * ```
     * @param service the name of the REST API to get the key of
     * @param field the field of the API object to get (this is "key" by default)
     * @return std::stringthe value of the specified field
     */
    std::string getApiKey(const char *service, const char *field="key");

    /**
     * @brief get the current weather from OpenWeather.
     * For this function to work, the system needs to be connected to a Wifi network.
     * @param weather a code representing the current weather.  see [this page](https://openweathermap.org/weather-conditions)
     * @param sunrise the time that the sun will rise (in Unix time)
     * @param sunset the time that sun will set (in Unix time)
     */
    void getCurrentWeather(uint16_t &weather, time_t &sunrise, time_t &sunset);

    /**
     * @brief attempts to get the current time and date using NTP.
     * 
     * for this to work, the system must be connected to the internet.  depending on the system settings, this can be called automatically when the system
     * boots or wakes up from sleep mode.
     */
    void getTimeFromNTP();

    /**
     * @brief performs an ADC reading, then processes it to me more accurate.
     * this was stolen from https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function.  the ESP32's ADC is non-linear, so this function takes an ADC reading, 
     * then adjusts it using a polynomial to make it more linear
     * @param pin the pin to read the voltage at
     * @return double the adjusted voltage read
     */
    double  ReadVoltage(byte pin);

    /**
     * @brief i dont know!!1!!!
     * 
     * this was a sample prototype is added when i was creating this file that I never got rid of.  it has no implementation, so don't use it.
     * 
     * @param x how many different stars there are in the sky
     * @param y the number of times you have blinked in your lifetime
     * @return int the number of bugs in this codebase
     */
    int     something(int x, int y);

    /**
     * @brief returns a file size in human readable form.
     * 
     * this will return a string that expresses a given file size in a format that makes more sense to humans.  For example, if you pass `1030` bytes, this
     * function will return "1KB".
     * 
     * @param bytes the number of bytes
     * @return const char* a human readable form
     */
    const char *humanSize(uint64_t bytes);

    /**
     * @brief prints details about each currently running task
     * @param output the Print object to print the details to
     */
    void print_task_details(Print &output=Serial);

    /**
     * @brief prints details about current memory usage
     * @param output the Print object to print the details to
     */
    void print_memory_details(Print &output=Serial);






    //--------------------------------------
    // drawing function prototypes
    //--------------------------------------


    /**
     * @brief sets up the fs_icon maps.
     * this only needs to be done once, and is already done in main.cpp.  it just
     * populates `fs_icon_ext_map` and `fs_icon_name_map` with links between file
     * extensions and fs icon identifiers, and fs icon identifiers and icon names.
     */
    void setupFsIcons();

    /**
     * @brief draw the status bar at the top of the page.
     * @param light if this is true, only the status icons will be drawn.  otherwise, everything will be drawn
     */
    void    drawTopThing(bool light = false);

    /**
     * @brief add an icon to the list of 16 bit colour icons.  this icon store is usually used to store things like app icons.
     * @param iconName the name of the icon that will be used to access it later
     * @param icon an array of unsigned short ints that represent the icon
     */
    bool    registerIcon(std::string iconName, imageData icon);

    /**
     * @brief add an icon to the list of smaller 8 bit monochrome icons.
     * @param iconName the name of the icon that will be used to access it later
     * @param icon a vector or array of unsigned chars that represent the icon
     */
    bool    registerSmallIcon(std::string iconName, std::vector<unsigned char> icon);

    /**
     * @brief sets the font used for a TFT or Sprite object from a vlw file stored on a filesystem.
     * 
     * the TFT_eSPI library allows you to use .vlw fonts.  It comes with a Processing sketch that lets you generate vlw fonts from standard font files.  You can
     * store a font file on a file system (like an SD card or SPIFFS), and use it to draw text to the TFT (or a Sprite).  This method checks if the specified font
     * exists on the specified filesystem, and if it does, it sets it as the font for the specified TFT or Sprite.
     * 
     * @param font a path to the vlw file on the specified file system.  don't include the ".vlw" extension.
     * @param tft the TFT_eSPI object to set the font for.  this is the main TFT object by default.
     * @param ffs the filesystem to load the font from.  this is SPIFFS by default.
     */
    void    setFont(const char* font, TFT_eSPI &tft = oled, fs::FS &ffs = SPIFFS);

    /**
     * @brief gets the dimensions of some text.
     * 
     * this aims to be compatible with the `getTextBounds()` method from AdafruitGFX.  This function does consider the number of lines that a string will take
     * when printed, and determines the height of one line based on the current font.
     * 
     * @param string the string to find the dimensions of
     * @param x the x position at which the string will be drawn
     * @param y the y position at which the string will be drawn
     * @param x1 the x component of the upper left corner of the string's bounding box
     * @param y1 the y component of the upper left corner of the string's bounding box
     * @param w the width of the string's bounding box
     * @param h the height of the string's bounding box
     */
    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

    /**
     * @brief gets the dimensions of some text.
     * 
     * this aims to be compatible with the `getTextBounds()` method from AdafruitGFX.  This function does consider the number of lines that a string will take
     * when printed, and determines the height of one line based on the current font.
     * 
     * @param str the string to find the dimensions of
     * @param x the x position at which the string will be drawn
     * @param y the y position at which the string will be drawn
     * @param x1 the x component of the upper left corner of the string's bounding box
     * @param y1 the y component of the upper left corner of the string's bounding box
     * @param w the width of the string's bounding box
     * @param h the height of the string's bounding box
     */
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

    /**
     * @brief converts a 16 bit 565 colour to a 24 bit 888 colour.
     * @param colour a 16 bit 565 colour
     * @param r the red component of the 24 bit colour
     * @param g the green component of the 24 bit colour
     * @param b the blue component of the 24 bit colour
     */
    void    colour888(uint16_t colour, float *r, float *g, float *b);

    /**
     * @brief converts an HSV colour to an RGB colour.
     * @param r the red component of the colour
     * @param g the green component of the colour
     * @param b the blue component of the colour
     * @param h the hue component of the colour
     * @param s the saturation component of the colour
     * @param v the value component of the colour
     */
    void    HSVtoRGB( float *r, float *g, float *b, float h, float s, float v );

    /**
     * @brief gets the colour at a certain position of a gradient.
     * 
     * the gradient should be passed as a vector of arrays of 3 elements. each vector element represents a colour in the gradient, 
     * and each array element represents a colour component as a float (0 meaning none and 1.0 meaning full colour; element 0 is red,
     * element 1 is green, and element 2 is blue).  here is an example:
     * 
     * ```
     * float r, g, b;
     * watch2::getHeatMapColor(value, &r, &g, &b, {
     *      {0.333, 0.804, 0.988},
     *      {0.969, 0.659, 0.722},
     *      {0.984, 0.976, 0.961},
     *      {0.969, 0.659, 0.722},
     *      {0.333, 0.804, 0.988}    
     *  });
     * ```
     * 
     * this was adapted from http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradient.
     * 
     * @param value the position of the gradient to get the colour at
     * @param red the red component of the colour
     * @param green the green component of the colour
     * @param blue the blue component of the colour
     * @param heatmap a vector of arrays of 3 elements, that contain the gradient colours
     */
    void    getHeatMapColor(float value, float *red, float *green, float *blue, std::vector<std::array<float, 3>> heatmap);

    /**
     * @brief draws a scrollable menu to the screen.
     * 
     * the menu looks like lots of rectangles on top of each other, and one "item" can be highlighted at a time.  you pass an array or vector of strings, and
     * each string is drawn inside one of these rectangles.  you can also pass a selected index, which determines which rectangle is highlighted.  this function
     * doesn't handle button presses, so you have to write your own logic to determine when to change the selected index, or what to do when the enter button
     * is pressed.
     * 
     * @param x the x position to draw the menu at
     * @param y the y position to draw the menu at
     * @param width the width of the menu
     * @param height the height of the menu
     * @param items a vector or array of std::strings that make up the menu items
     * @param selected the index of the selected menu item
     * @param scroll whether or not the menu should scroll when the selected item is beyond a certain threshold that i can't remember
     * @param centre if this is true, text will be drawn in the middle of the button
     * @param colour the colour the menu should be drawn.  by default, this will be the theme colour
     */
    void    drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, std::vector<fs_icon> icons={}, bool scroll=true, bool centre = false, int colour=themecolour);

    /**
     * @brief draws a settings menu.
     * 
     * this draws a menu where there are lots of prompts, and each prompt has a spinner thing beside it.  the spinner can have multiple different options, which
     * the user can choose from.  you pass an array or vector of `settingsMenuData` structs, which describe the prompt, the possible options for the spinner, and
     * some other settings.  this function doesn't handle any keypresses, so it's up to the developer to change the selected index, and selected options.
     * 
     * @param x the x position to draw the menu at
     * @param y the y position to draw the menu at
     * @param width the width of the menu
     * @param height the height of the menu
     * @param items a vector or array of `settingsMenuData` structs
     * @param selected the index of the selected `settingsMenuData` struct
     * @param colour the colour that the menu should be drawn.  by default, this will be the theme colour
     */
    void    drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour=themecolour);





    //--------------------------------------
    // image function prototypes
    //--------------------------------------


    /**
     * @brief idk what this does.  it's part of drawBmp().
     * 
     * this is from a BMP rendering function written by Bodmer, that can be found at https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino
     * i'm not entirely sure what this function actually does...
     * 
     * @param f ???
     * @return uint16_t ???
     */
    uint16_t read16(fs::File &f);

    /**
     * @brief idk what this does.  it's part of drawBmp().
     * 
     * this is from a BMP rendering function written by Bodmer, that can be found at https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino
     * i'm not entirely sure what this function actually does...
     * 
     * @param f ???
     * @return uint16_t ???
     */
    uint32_t read32(fs::File &f);

    //functions for stb_image
    int img_read(void *user,  char *data, int size);
    void img_skip(void *user, int n);
    int img_eof(void *user);
    int img_read_spiffs(void *user,  char *data, int size);
    void img_skip_spiffs(void *user, int n);
    int img_eof_spiffs(void *user);

    /**
     * @brief draws a 24 bit uncompressed BMP to the screen.
     * 
     * this is a function written by Bodmer, and can be found at https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino.
     * 
     * @param filename the path to the BMP in SPIFFS to draw
     * @param x the x position to draw the image at
     * @param y the y position to draw the image at
     */
    void drawBmp(const char *filename, int16_t x, int16_t y);

    void loadBmp(const char *filename);

    /**
     * @brief get the data of an image as an array of pixels.
     * 
     * this function relies on [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) to return an array of pixel data based on an input file.  the
     * image formats supported are dependant on what stb_image.h can handle, which are listed below:
     * - JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)
     * - PNG 1/2/4/8/16-bit-per-channel
     * - TGA (not sure what subset, if a subset)
     * - BMP non-1bpp, non-RLE
     * - PSD (composited view only, no extra channels, 8/16 bit-per-channel        
     * - GIF (*comp always reports as 4-channel, not animated)
     * - HDR (radiance rgbE format)
     * - PIC (Softimage PIC)
     * - PNM (PPM and PGM binary only)
     * This function returns an `imageData` struct, which contains the raw pixel data, and some metadata, like width and height.  Images can be drawn to the 
     * screen using `drawImage()`.  Once you have drawn the image, you can free the memory used by calling `freeImageData()` on the `data` member of the 
     * returned struct.
     * 
     * For some reason, BMPs smaller than 44x44 won't load properly.
     * 
     * @param filename the path to the image file
     * @return imageData a struct containing image data
     */
    imageData getImageData(const char *filename);
    imageData getImageDataSPIFFS(const char *filename);
    imageData getImageDataMemory(const unsigned char *buffer, int len);

    /**
     * @brief frees the memory used by an image loaded using `getImageData()`.
     * @param data the image data to free.
     */
    void freeImageData(imageData data);
    void freeImageData(unsigned char *data);

    /**
     * @brief draws an image to the screen.
     * 
     * this function can be used to draw images returned from `getImageData()`.  you can pass a value that can be used to scale the image so that it will be
     * a different size.  the value passed is used to determine how many times smaller the image will be drawn.  if the value passed is 2.0, the image will
     * be half the size.  if the value is 0.5, the image will be drawn at twice the size.  the image resizing is done by 
     * [stb_image_resize.h](https://github.com/nothings/stb/blob/master/stb_image_resize.h).
     * 
     * @param data an `imageData` containing the image data
     * @param img_x the x position to draw the image at
     * @param img_y the y position to draw the image at
     * @param scaling the scaling factor
     * @return const char* if the image couldn't be read, this will return a string describing the reason why
     */
    const char* drawImage(imageData data, int16_t img_x, int16_t img_y, float scaling=1.0, int array_offset=0, TFT_eSPI &tft=oled, bool use_dma=true);






    //--------------------------------------
    // file system function prototypes
    //--------------------------------------


    /**
     * @brief initalises the SD card.
     * 
     * this will try and initalise the SD card and set up the watch's variables that track the SD card's state.  most watch 2 functions that work with the SD card
     * call this function before doing things
     * 
     * @param handleCS whether or not the SPI chip select pins should be manually configured by the function
     * @return int the state of the sd card
     * 0 - the SD card couldn't be mounted
     * 1 - the SD card was mounted successfully
     */
    int     initSD(bool handleCS = true);
    
    /**
     * @brief method to return all the files in a directory (non-recursively).
     * @param path the path of the directory to return files in
     */
    std::vector<std::string> getDirFiles(std::string path, std::vector<fs_icon> *icons=NULL);

    /**
     * @brief open the file select dialogue.  this will pause the state until a file has been selected (or the operation has been cancelled).
     * @param path the path to start the file selection at
     */
    std::string beginFileSelect(std::string path = "/");

    /**
     * @brief gets the name part of a file path.
     * 
     * for example, if you pass 'images/ir/pause.bmp', this function will return 'pause.bmp'.  if `extension` is false, it will return `pause`.
     * 
     * @param filepath the path to the file to get the name of
     * @param extension whether or not to include the file extension
     * @return std::string the name
     */
    std::string file_name(const char* filepath, bool extension=true);

    /**
     * @brief gets the directory part of a file path.
     * 
     * for example, if you pass `/images/ir/pause.bmp`, this function will return `/images/ir/`.
     * 
     * @param file_path_thing the path to get the directory part of
     * @return std::string the path part of the directory
     */
    std::string dir_name(std::string file_path_thing);

    /**
     * @brief gets the extension of a file name.
     * 
     * for example, if you pass `/images/ir/pause.bmp`, this function will return `bmp`.
     * 
     * @param file_path_thing the filename to get the extension of
     * @return std::string the extension
     */
    std::string file_ext(std::string file_path_thing);





    //--------------------------------------
    // dialogue function prototypes
    //--------------------------------------
    
    
    /**
     * @brief gets a string from the user.
     * 
     * this will open a popup dialogue that will ask the user for a text-based input.  a tiny keyboard will also be drawn on screen.  this function blocks the calling
     * code, and automatically handles dpad input.
     * 
     * @param prompt a prompt for what should be entered
     * @param default_input the text field will be populated with this when the dialogue opens
     * @param mask if this isn't 0, all the characters in the text field will be drawn with whatever character is passed.
     * @param clear_screen if this is true, the screen will be cleared when the dialogue closes (any part of the calling state that is wrapped with the `draw` macro
     * will be redrawn after the dialgoue closes irrespective of this parameter).
     * @return std::string the string that the user entered.  if the user cancelled the input, this will return "".
     */
    std::string textFieldDialogue(std::string prompt="", const char *default_input="", const char mask=0, bool clear_screen=true);

    /**
     * @brief this will produce a popup message box.
     * 
     * this function will block the calling state, and automatically handles dpad input.  you can specify the text to draw in the message box, and what buttons should
     * be provided.  when a button is pressed, the index of whatever button was pressed will be returned.
     * 
     * @param msg the message to draw in the box
     * @param btns an array of vector of strings that will be drawn as buttons below the message
     * @param clear_screen if this is true, the screen will be cleared when the dialogue closes (any part of the calling state that is wrapped with the `draw` macro
     * will be redrawn after the dialgoue closes irrespective of this parameter).
     * @param colour the colour to draw the dialogue.  by default, this will be the current theme colour
     * @return uint8_t the index of whatever button is selected
     */
    uint8_t messageBox(const char* msg, std::vector<const char*> btns={"Ok"}, bool clear_screen=true, uint16_t colour=themecolour);

    /**
     * @brief opens a popup menu with a series of buttons
     * 
     * @param title the title that is displayed at the top of the dialogue
     * @param items an array or vector of strings that represent the menu items
     * @param colour the colour to draw the dialogue
     * @return uint16_t the index of the selected menu item
     */
    uint16_t popup_menu(const char *title, std::vector<std::string> items, bool scroll=false, uint16_t colour=watch2::themecolour);

    /**
     * @brief shows the ripoff control centre as a popup dialogue
     * 
     */
    void controlCentreDialogue();

    

    

    //--------------------------------------
    // wifi function prototypes
    //--------------------------------------    

    
    /**
     * @brief enables the wifi subsystem. 
     * @param connect if this is true, the system will connect to one of the most recently connected to access points
     */
    void enable_wifi(bool connect=true);

    /**
     * @brief disables the wifi subsystem.
     */
    void disable_wifi();

    /**
     * @brief connect to an access point.
     * 
     * if `ssid` is specified, the system will attempt to connect to that specific access point.  if `ssid` isn't specified, the system will attempt to
     * connect to one of the access points that was most recently connected to.  this works by attempting to connect to each of the `n` access points
     * that were most recently connected to.  the value of `n` is determined by `initial_wifi_reconnect_attempts`.
     * 
     * @param ssid 
     * @param password 
     */
    void connectToWifiAP(const char *ssid="", const char *password="");

    /**
     * @brief returns the wifi profiles JSON object.
     * 
     * when the system connects to an access point, the connection details are saved in SPIFFS as a "profile", so that the watch can automatically
     * reconnect to it later.  this function reads the profiles file from SPIFFS, parses it to create a cJSON struct, and returns it.  The profile
     * list can be updated using `setWifiProfiles()`.  If a profiles file doesn't exist, a blank one is created.
     * 
     * The profiles object has two keys: 
     * - `profiles`
     *   - this is an array of objects that each represent one profile.  each object has 3 keys:
     *     - `ssid`: the access point's SSID
     *     - `password`: the access point's password, in plain text...
     *     - `encryption`: an enumeration that represents the encryption used by the access point.  the possible values can be found [here](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/api-reference/network/esp_wifi.html#_CPPv416wifi_auth_mode_t)
     * - `access_list`
     *   - this is an array of strings that represent the SSIDs of profiles in the order of most recently connected.
     * 
     * @return cJSON* the wifi profiles
     */
    cJSON *getWifiProfiles();

    /**
     * @brief saves the wifi profiles list.
     * 
     * when the system connects to an access point, the connection details are saved in SPIFFS as a "profile", so that the watch can automatically
     * reconnect to it later.  this function can be used to update the profile list, by passing a profile cJSON struct.  the object that is passed should
     * follow the schema defined in the documentation of `getWifiProfiles()`. 
     * 
     * @param profiles a cJSON object containing wifi profiles
     */
    void setWifiProfiles(cJSON *profiles);





    //--------------------------------------
    // bluetooth function prototypes
    //--------------------------------------

    /**
     * @brief sends an HID Keyboard report
     * This will only do something if the watch is connected to something over BLE (bluetooth_state == 3).
     * This sends a Keyboard report, and notifies the connected device.  The report should be an array of
     * 8 bytes that make up a standard USB HID Keyboard Report (this is described really well at 
     * https://github.com/jpbrucker/BLE_HID/blob/master/doc/HID.md).  Make sure you send a blank report
     * afterwards to "release" the key.
     * 
     * Here is an example that presses and releases the 'a' key:
     * 
     * ```c++
     * uint8_t report_a[8] = {0, 0, 4, 0, 0, 0, 0, 0};
     * uint8_t report_clr[8] = {0}; // initalises each element to 0
     * 
     * watch2::ble_hid_send_keyboard_report(report_a);
     * watch2::ble_hid_send_keyboard_report(report_clr);
     * ```
     * @param report an array of bytes that make up the report
     */
    void ble_hid_send_keyboard_report(uint8_t* report);

    /**
     * @brief sends an HID Consumer Control report using a byte array
     * This will only do something if the watch is connected to something over BLE (bluetooth_state == 3).
     * Make sure to "release" the key after pressing it by sending an empty report.
     * @param report an array of bytes that make up the report
     */
    void ble_hid_send_media_key_report(uint8_t* report);

    /**
     * @brief sends an HID consumer control report from a set of predefined reports
     * This will only do something if the watch is connected to something over BLE (bluetooth_state == 3).
     * This does the same thing as `ble_hid_send_media_key_report(uint8_t *report)`, except that you don't need to pass in
     * a byte array.  Instead, you pass in a predefined `MediaKeyReport` that represens the key you want to press.
     * This function takes care of releasing the key after pressing it, so you don't need to pass an empty report.
     * @param report a report that represents the media key to press and release
     */
    void ble_hid_send_media_key_report(const MediaKeyReport report);

    /**
     * @brief enables the bluetooth subsystem.  this is kind of broken at the minute. 
     */
    void enable_bluetooth();

    /**
     * @brief disables the bluetooth subsystem.  this is kind of broken at the minute.
     */
    void disable_bluetooth();






    //--------------------------------------
    // audio function prototypes
    //--------------------------------------

    void setup_audio_for_playback();
    void setup_audio_for_input();
    void uninstall_i2s_driver();

    /**
     * @brief Play an audio file over I2S.
     * 
     * The audio playback is handled by schreibfaul1's [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) library.  The I2S
     * interface is setup, then a new FreeRTOS task with a really high priority is created to handle the audio loop.  This means that if
     * you try to do a lot of anything else at the same time, it will run really slowly, and the audio will playback very distorted.
     * 
     * You have to pass a task handle to the function.  You can use this to delete the task when you need to stop playback.  The task will
     * delete itself once playback ends, unless `repeat` is true.
     * 
     * @param task_handle a handle to the task that is created to play the music
     * @param filename the name of the audio file on the filesystem, or a url to an audio stream
     * @param repeat if this is true, the audio file will restart once it has ended, and will play forever
     * @param fs the filesystem to stream the audio from, or null to use The Internet™
     * @return true if the audio was loaded ok
     * @return false if tha audio failed to load
     */
    bool play_music(const char *filename, bool repeat=false, fs::FS *fs=&SD);

    /**
     * @brief this stops any currently playing music.
     * 
     */
    void stop_music();

    /**
     * @brief this is the function used for the audio task, so pls don't call it.
     * 
     * @param pvParameters 
     */
    void audio_task(void *pvParameters);

}

#endif /* WATCH2_H */
