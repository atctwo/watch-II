#ifndef WATCH2_H
#define WATCH2_H

//pin declarations
#define cs   5//17  // goes to TFT CS
#define dc   22//16  // goes to TFT DC
#define mosi 23  // goes to TFT MOSI
#define sclk 18  // goes to TFT SCK/CLK
#define rst  21//5   // ESP RST to TFT RESET
#define miso 19     // Not connected
//       3.3V     // Goes to TFT LED
//       5v       // Goes to TFT Vcc
//       Gnd      // Goes to TFT Gnd

#define dpad_up     33
#define dpad_down   27
#define dpad_left   32
#define dpad_right  25
#define dpad_enter  26
#define BATTERY_DIVIDER_PIN 34

//button active macros
#define KEY_REPEAT_DELAY    550     //time for key repeat to start, in ms
#define KEY_REPEAT_PERIOD   24      //time between key repeats, in ms

#define dpad_up_active()    ( !dpad_up_lock &&    (btn_dpad_up.wasPressed() || ( btn_dpad_up.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD == 0 ) ) ) )
#define dpad_down_active()  ( !dpad_down_lock &&  (btn_dpad_down.wasPressed() || ( btn_dpad_down.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD == 0 ) ) ) )
#define dpad_left_active()  ( !dpad_left_lock &&  (btn_dpad_left.wasPressed() || ( btn_dpad_left.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD == 0 ) ) ) )
#define dpad_right_active() ( !dpad_right_lock && (btn_dpad_right.wasPressed() || ( btn_dpad_right.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD == 0 ) ) ) )
#define dpad_enter_active() ( !dpad_enter_lock && (btn_dpad_enter.wasPressed() || ( btn_dpad_enter.pressedFor(KEY_REPEAT_DELAY) && ( millis() % KEY_REPEAT_PERIOD == 0 ) ) ) )
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


// struct definitions

// state metadata struct
struct stateMeta {
    std::string             stateName;
    std::string             stateIcon;
    std::function<void()>   stateFunc;
    int                     variant;
};

// data for entries in settings menus
struct settingsMenuData {

    String                      setting_title;
    int                         setting_value;
    std::vector<String>         text_map;
    int                         min_field_width;

};


// system function prototypes
//preferences.getInt("themecolour", default_themecolour)
void    drawTopThing();
int     registerState(std::string stateName, std::string stateIcon, const std::function<void()>& stateFunc);
bool    registerIcon(std::string iconName, std::vector<unsigned short int> icon);
void    dimScreen(bool direction, int pause_thing);
void    switchState(int newState, int variant = 0, int dim_pause_thing = 10, int bright_pause_thing = 10);
void    deepSleep(int pause_thing=10);
void    drawMenu(int x, int y, int width, int height, std::vector<String> items, int selected, int colour);
void    drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour);
void    colour888(uint16_t colour, float *r, float *g, float *b);
void    HSVtoRGB( float *r, float *g, float *b, float h, float s, float v );
bool    getHeatMapColor(float value, float *red, float *green, float *blue);
double  ReadVoltage(byte pin);
int     something(int x, int y);

#endif /* WATCH2_H */
