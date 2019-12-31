
/*
//this file contains declarations for every global variable that the system
//uses.  the veriables are defined in watch2.ino

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

*/