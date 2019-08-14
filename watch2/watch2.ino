

////////////////////////////////////////
// includes
////////////////////////////////////////

#include <SPI.h>                    // SPI library
#include <Adafruit_GFX.h>           // Used for drawing graphics to the OLED
#include <Adafruit_SSD1351.h>       // used to interface with the OLED
#include <JC_Button.h>              // button object
#include <WiFi.h>                   // wifi library
#include <Preferences.h>
#include <tinyexpr.h>
#include <map>                      // map object for storing states
#include <string>                   // std::string
#include <functional>               // std::function thing
#include <time.h>                   // used for system-level time keeping
#include <sys/time.h>               // see above
#include <TimeLib.h>                // used for managing time (see code note 1)
#include <TimeAlarms.h>
#include <stdio.h>                  // i don't actually know...
#include <algorithm>                // used for std::find and std::min and std::max

#include "watch2.h"                 // defines and function prototypes

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

////////////////////////////////////////
// global stuff
////////////////////////////////////////

// type definitions
typedef void (*func)(void);         // function pointer type

// object creation
SPIClass *vspi = new SPIClass(VSPI);        // VSPI object
Adafruit_SSD1351 oled = Adafruit_SSD1351(128, 96, vspi, cs, dc, rst);               //hw spi (use vspi or &SPI)
//Adafruit_SSD1351 oled = Adafruit_SSD1351(128, 96, _cs, _dc, _mosi, _sclk, _rst);  //sw spi
Preferences preferences;

//button objects
Button btn_dpad_up(dpad_up, 25, false, false);
Button btn_dpad_down(dpad_down, 25, false, false);
Button btn_dpad_left(dpad_left, 25, false, false);
Button btn_dpad_right(dpad_right, 25, false, false);
Button btn_dpad_enter(dpad_enter, 25, false, false);

//map of watch states
std::map<int, stateMeta> states;

//map of icons
std::map<std::string, std::vector<unsigned short int>> icons;   //large colour icons for things like the state menu
std::map<std::string, std::vector<unsigned char>> small_icons;  //smaller monochrome icons for use within GUIs

// global variables
int state = 0;                                                  //currently selected state
int state_init = 0;                                             //0 if the state is being executed for the first time (after swtiching from another stat)
std::map<int, stateMeta>::iterator selected_menu_icon;          //iterator to the currently selected state
RTC_DATA_ATTR int selected_state = 0;                           //numerical representation of currently selected state that persists accross sleep mode
//boot count is used to keep track of what state was selected in the menu before
//entering deep sleep.  This variable is only used when going in to or waking up
//from sleep.  During active mode operation, selected_menu_icon is used,
RTC_DATA_ATTR int boot_count = 0;                               //no of times watch has woken up (including initial boot)
int trans_mode = 0;                                             //pretty colour scheme
int short_timeout = 5000;                                       //timeout when looking at watch face
int long_timeout = 30000;                                       //timeout (almost) everywhere else
bool timeout = true;                                            //whether or not to go to sleep after timeout time has elapsed
int themecolour = BLUE;                                         //colour of the system accent

int RTC_DATA_ATTR stopwatch_timing = 0;                         //stopwatch state
                                                                //0 - stopped
                                                                //1 - running
                                                                //2 - paused
uint32_t RTC_DATA_ATTR stopwatch_epoch = 0;                     //time when the stopwatch was started
uint32_t RTC_DATA_ATTR stopwatch_paused_diff = 0;               //when the stopwatch is off or paused, stopwatch_epoch is set to the current time minus this value
                                                                //when the stopwatch is paused, this value is set to the difference between stopwatch_epoch and the current time (keeps the difference constant)
                                                                //when the stopwatch is off, this value is set to 0
uint32_t stopwatch_time_diff = 0;                               //difference between epoch and current time (equivalent to elapsed time)
uint32_t stopwatch_last_time_diff = 0;                          //last time diff (used for checking when to redraw elapsed time)
uint32_t stopwatch_ms = 0, stopwatch_last_ms = 0;
uint32_t stopwatch_s = 0, stopwatch_last_s = 0;
uint32_t stopwatch_min = 0, stopwatch_last_min = 0;
uint32_t stopwatch_hour = 0, stopwatch_last_hour = 0;

std::vector<timerData> timers;
int timer_trigger_status = 0;                                   //the state of a timer
                                                                //0 - timer not going off → normal state execution
                                                                //1 - timer going off → suspend state execution and draw alarm message
                                                                //2 - timer gone off → wait for user input before resuming state execution
int timer_trigger_id = 255;

//these variables stop button presses affecting new states
//when switching from a previous state.
//when a user presses a button to go from the watch face to the menu,
//if the button is held down for long enough, the button press can affect
//the next state.
//these lock variables are set to true when switching states, then set to false
//when each button is released.
bool dpad_up_lock = false;
bool dpad_down_lock = false;
bool dpad_left_lock = false;
bool dpad_right_lock = false;
bool dpad_enter_lock = false;

////////////////////////////////////////
// include state file things and icon files
////////////////////////////////////////
#include "icons/app_icons.cpp"
#include "icons/small_icons.cpp"

#include "states/system_states.cpp"
#include "states/util_states.cpp"
#include "states/time_states.cpp"

////////////////////////////////////////
// setup function
////////////////////////////////////////

void setup() {

    //set cpu Frequency
    //setCpuFrequencyMhz(80);

    // configure gpio data direction registers
    pinMode(12, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(BATTERY_DIVIDER_PIN, INPUT);

    //begin serial
    Serial.begin(115200);

    //set up oled
    oled.begin(18000000);
    oled.fillScreen(0);
    //oled.setFont(&SourceSansPro_Regular6pt7b);
    uint8_t contrast = 0x0f;
    oled.sendCommand(0xC7, &contrast, 1);

    //set up preferences
    preferences.begin("watch2", false);      //open watch II preferences in RW mode
    timeout = preferences.getBool("timeout", false);
    short_timeout = preferences.getInt("short_timeout", 5000);
    long_timeout = preferences.getInt("long_timeout", 30000);
    themecolour = preferences.getInt("themecolour", BLUE);
    trans_mode = preferences.getBool("trans_mode", false);
    preferences.end();

    //set up buttons
    btn_dpad_up.begin();
    btn_dpad_down.begin();
    btn_dpad_left.begin();
    btn_dpad_right.begin();
    btn_dpad_enter.begin();

    //set up time
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (boot_count == 0)
    {
        setTime(22, 15, 00, 28, 6, 2019);
        WiFi.mode(WIFI_OFF);
        btStop();

    }

    //set up icons
    registerAppIcons();
    registerSmallIcons();

    //set up states
    registerSystemStates();
    registerUtilStates();
    registerTimeStates();

    if (boot_count == 0)
    {
        //set selected menu icon to first non-hidden state
        selected_menu_icon = states.begin();
        while(1)
        {
            if (selected_menu_icon->second.hidden)
            {
                //check next state
                std::advance(selected_menu_icon, 1);
            }
            else break;
        }
    }
    else selected_menu_icon = states.find(selected_state);

    //finish up
    boot_count++;

}

////////////////////////////////////////
// loop function
////////////////////////////////////////

void loop() {

    //initial button setup
    btn_dpad_up.read();
    btn_dpad_down.read();
    btn_dpad_left.read();
    btn_dpad_right.read();
    btn_dpad_enter.read();

    //check timers and alarms
    Alarm.delay(0);

    //run current state
    if (timer_trigger_status == 0)
    {
        if ( states.find(state) == states.end() )
        states[-1].stateFunc();
        else states[state].stateFunc();
    }
    else
    {
        if (timer_trigger_status == 1)
        {
            //dim screen
            dimScreen(0, 10);
            oled.fillScreen(BLACK);

            //get time to display
            time_t duration = timers[timer_trigger_id].initial_duration;
            time_t time_left_hrs = floor(duration / 3600);
            time_t time_left_min = floor(duration % 3600 / 60);
            time_t time_left_sec = floor(duration % 3600 % 60);

            //draw message
            oled.setFont(&SourceSansPro_Light8pt7b);
            oled.setCursor(0, 20);
            oled.printf("%02d hours,\n%02d minutes, and\n%02d seconds\n", time_left_hrs, time_left_min, time_left_sec);
            oled.setFont(&SourceSansPro_Regular6pt7b);
            oled.print("have elapsed.  press any\nkey to continue");

            //brighten screen
            dimScreen(1, 10);

            //set timer trigger status
            timer_trigger_status = 2;
        }

        drawTopThing();
        if (dpad_any_active())
        {
            timer_trigger_status = 0;
            switchState(state);
        }
    }

    //reset button lock state
    if (btn_dpad_up.isReleased())       dpad_up_lock = false;
    if (btn_dpad_down.isReleased())     dpad_down_lock = false;
    if (btn_dpad_left.isReleased())     dpad_left_lock = false;
    if (btn_dpad_right.isReleased())    dpad_right_lock = false;
    if (btn_dpad_enter.isReleased())    dpad_enter_lock = false;

    //handle timeouts
    if (timeout && (state != 0))
    {
        if (state == 1)
        {
            if (btn_dpad_up.releasedFor(short_timeout) &&
                btn_dpad_down.releasedFor(short_timeout) &&
                btn_dpad_left.releasedFor(short_timeout) &&
                btn_dpad_right.releasedFor(short_timeout) &&
                btn_dpad_enter.releasedFor(short_timeout))
                deepSleep(31);
        }
        else
        {
            if (btn_dpad_up.releasedFor(long_timeout) &&
                btn_dpad_down.releasedFor(long_timeout) &&
                btn_dpad_left.releasedFor(long_timeout) &&
                btn_dpad_right.releasedFor(long_timeout) &&
                btn_dpad_enter.releasedFor(long_timeout))
                deepSleep(31);
        }
    }

}


////////////////////////////////////////
// system functions
////////////////////////////////////////


void drawTopThing()
{
    static double batteryVoltage = 4.2;
    static double batteryPercentage = 0;
    static int last_battery_reading = millis() - 1000;

    oled.drawFastHLine(0, 10, SCREEN_WIDTH, themecolour);
    oled.setCursor(1,8);
    oled.setTextColor(WHITE);
    oled.setTextSize(1);
    oled.setFont(&SourceSansPro_Regular6pt7b);
    oled.print("watch II");

    //oled.printf(" %d ", preferences.getBool("timeout", true));

    if ( millis() - last_battery_reading > 1000)
    {
        //batteryVoltage = ( (ReadVoltage(BATTERY_DIVIDER_PIN) * 3.3 ) / 4095.0 ) * 2;
        batteryVoltage = ReadVoltage(BATTERY_DIVIDER_PIN) * BATTERY_VOLTAGE_SCALE;
        batteryPercentage = ( batteryVoltage / BATTERY_VOLTAGE_MAX ) * 100.0;
        last_battery_reading = millis();

        oled.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK);  //update this to only clear status portion
        oled.printf(" %.0f%%", batteryPercentage);
    }
}

int registerState(std::string stateName, std::string stateIcon, const std::function<void()>& stateFunc, bool hidden)
{
    stateMeta meta = {
        stateName,
        stateIcon,
        stateFunc,
        0,
        hidden
    };

    states.insert( { states.size() - 1, meta } );

    return 5; //fix
}

bool registerIcon(std::string iconName, std::vector<unsigned short int> icon)
{
    icons.emplace( iconName, icon );

    return false;
}

bool registerSmallIcon(std::string iconName, std::vector<unsigned char> icon)
{
    small_icons.emplace( iconName, icon );

    return false;
}

void switchState(int newState, int variant, int dim_pause_thing, int bright_pause_thing, bool dont_draw_first_frame)
{
    dimScreen(0, dim_pause_thing);              //dim the screen
    oled.fillScreen(BLACK);                     //clear screen

    dpad_up_lock = true;                        //set button locks
    dpad_down_lock = true;
    dpad_left_lock = true;
    dpad_right_lock = true;
    dpad_enter_lock = true;

    state_init = 0;                             //reset first execution flag
    state = newState;                           //switch state variable
    states[state].variant = variant;            //set state variant

    if (!dont_draw_first_frame)
    states[state].stateFunc();                  //run the state function once

    state_init = 1;                             //set first execution flag
    dimScreen(1, bright_pause_thing);           //make the screen brighter

}

void dimScreen(bool direction, int pause_thing)
{

    // direction
    // 0 - decrease brightness
    // 1 - increase brightness
    if (direction) for (uint8_t contrast = 0; contrast < 16; contrast++)
    {
        oled.sendCommand(0xC7, &contrast, 1);
        delay(pause_thing);
    }
    else for (uint8_t contrast = 16; contrast > 0; contrast--)
    {
        uint8_t contrast_step = contrast - 1;
        oled.sendCommand(0xC7, &contrast_step, 1);
        delay(pause_thing);
    }
}

void deepSleep(int pause_thing)
{
    //fade to black
    dimScreen(0, pause_thing);
    oled.fillScreen(BLACK);

    //turn off display
    oled.sendCommand(0xAE);

    //save selected_state
    selected_state = selected_menu_icon->first;

    //set time
    timeval tv{
        now(),
        0
    };

    settimeofday(&tv, NULL);

    //deep sleep setup
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 1); //1 = High, 0 = Low

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);

    //determine when to wake up (if a timer or alarm is set)
    time_t next_alarm_time = -1;
    time_t time_left = 0;

    for (int i = 0; i < timers.size(); i++)
    {
        if (timers[i].alarm_id != 255)
        {
            time_t time_left = ( timers[i].time_started + Alarm.read(timers[i].alarm_id ) ) - now();
            if (time_left > next_alarm_time) next_alarm_time = time_left;
        }
    }

    //if an timer or an alarm has been set, set the device to wake up just before the alarm triggers
    if (next_alarm_time > -1)
    {
        esp_sleep_enable_timer_wakeup(next_alarm_time * 1000 * 1000 - 100);
    }
    else //if no alarm or timer has been set, then disable the timer wake up source
    {
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    }

    //begin sleep
    Serial.println("entering sleep mode");
    esp_light_sleep_start();

    //when the esp is woken up, it will resume execution at this point

    Serial.println("awake");
    oled.sendCommand(0xAF);

    //set up buttons
    btn_dpad_up.begin();
    btn_dpad_down.begin();
    btn_dpad_left.begin();
    btn_dpad_right.begin();
    btn_dpad_enter.begin();

    //rtc_gpio_deinit(GPIO_NUM_26);
    if (next_alarm_time > -1) switchState(0, 0, 0, 0, true);
    else switchState(0);
}

void drawMenu(int x, int y, int width, int height, std::vector<String> items, int selected, int colour)
{
    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static int padding = 4;
    static int radius = 4;

    int fridgebuzz = 0;
    for (const String &item : items)
    {
        oled.setCursor(x+padding, y+padding+8);
        oled.getTextBounds(item, x+padding, y+padding+8   , &x1, &y1, &w, &h);
        if (selected == fridgebuzz)
        {
            oled.fillRoundRect(x, y, width, h + (2 * padding), radius, colour);
            oled.setTextColor(BLACK);
        }
        else
        {
            oled.fillRoundRect(x, y, width, h + (2 * padding), radius, BLACK);
            oled.drawRoundRect(x, y, width, h + (2 * padding), radius, colour);
            oled.setTextColor(colour);
        }
        //oled.setCursor(x1, y1);
        oled.print(item);
        y += h + padding + padding + padding;
        fridgebuzz++;
    }
}

void drawSettingsMenu(int x, int y, int width, int height, std::vector<settingsMenuData> items, int selected, int colour)
{
    static String last_selected_value = "";
    static int last_selected_element = -1;
    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static int padding = 4;
    static int spacing = 1;
    static int radius = 4;
    int ypos = padding;               //y position of cursor (considering multiline settings)


    for (int i = 0; i < items.size(); i++)
    {
        int text_height = 0;
        int cursor_y_pos = y + ypos + 8;
        int outline_width;
        int outlinecolour;

        // draw settings title
        oled.setCursor(x, cursor_y_pos);
        oled.getTextBounds(String(items[i].setting_title), x, cursor_y_pos, &x1, &y1, &w, &h);
        oled.print(items[i].setting_title);
        text_height = h;

        // determine settings value
        String final_setting_value;
        if (i < items[i].text_map.size()) final_setting_value = items[i].text_map[items[i].setting_value];
        else final_setting_value = String(items[i].setting_value);

        // clear previous settings value
        if (last_selected_element == i && last_selected_value != final_setting_value)
        {
            digitalWrite(12, HIGH);
            oled.getTextBounds(last_selected_value, x, cursor_y_pos, &x1, &y1, &w, &h);
            outline_width = std::max(items[i]. min_field_width, w + (2 * padding));

            oled.fillRect( (x + (width - padding - outline_width)) - padding,
                            cursor_y_pos - padding - 8,
                            outline_width,
                            h + (2 * padding),
                            BLACK
            );
        }
        else digitalWrite(12, LOW);

        // draw settings value
        oled.getTextBounds(final_setting_value, x, cursor_y_pos, &x1, &y1, &w, &h);
        outline_width = std::max(items[i]. min_field_width, w + (2 * padding));
        outlinecolour = BLACK;

        oled.setCursor(x + (width - padding - outline_width), cursor_y_pos);
        oled.print(final_setting_value);

        //draw outline around value if element is selected
        if (selected == i) outlinecolour = colour;

        oled.drawRoundRect( (x + (width - padding - outline_width)) - padding,
                            cursor_y_pos - padding - 8,
                            outline_width,
                            h + (2 * padding),
                            radius,
                            outlinecolour
        );

        if (selected == i)
        {
            last_selected_value = final_setting_value;
            last_selected_element = selected;
        }
        ypos += text_height + ( 2 * padding );
    }
}

void Adafruit_GFX::drawRainbowBitmap(int16_t x, int16_t y,
  const uint8_t bitmap[], int16_t w, int16_t h,
  uint16_t bg, int phase_difference) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

	float colour_parts = 360 / w;

    startWrite();
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            float R, G, B;
            if (!trans_mode)   HSVtoRGB(&R, &G, &B, fmodf(((((float) i) * colour_parts) + phase_difference), (float)360.0), 1.0, 1.0);
            else                                            getHeatMapColor(fmod(((float)i/(float)w) + (phase_difference/360.0), (float)1.0), &R, &G, &B);
            if(i & 7) byte <<= 1;
            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            writePixel(x+i, y, (byte & 0x80) ? oled.color565((int)(R*255.0), (int)(G*255.0), (int)(B*255.0)) : bg);
        }
    }
    endWrite();
}

// adapted from https://stackoverflow.com/a/9069480/9195285
void colour888(uint16_t colour, float *r, float *g, float *b)
{
    uint16_t red   = (colour & 0xf800) >> 11;
    uint16_t green = (colour & 0x07e0) >> 5;
    uint16_t blue  = (colour & 0x001f);

    *r = ( red   * 255 ) / 31;
    *g = ( green * 255 ) / 63;
    *b = ( blue  * 255 ) / 31;
}

// from https://www.cs.rit.edu/~ncs/color/t_convert.html#RGB%20to%20HSV%20&%20HSV%20to%20RGB
void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
	int i;
	float f, p, q, t;
	if( s == 0 ) {
		// achromatic (grey)
		*r = *g = *b = v;
		return;
	}
	h /= 60;			// sector 0 to 5
	i = floor( h );
	f = h - i;			// factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );
	switch( i ) {
		case 0:
			*r = v;
			*g = t;
			*b = p;
			break;
		case 1:
			*r = q;
			*g = v;
			*b = p;
			break;
		case 2:
			*r = p;
			*g = v;
			*b = t;
			break;
		case 3:
			*r = p;
			*g = q;
			*b = v;
			break;
		case 4:
			*r = t;
			*g = p;
			*b = v;
			break;
		default:		// case 5:
			*r = v;
			*g = p;
			*b = q;
			break;
	}
}

//from http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients
bool getHeatMapColor(float value, float *red, float *green, float *blue)
{
  const int NUM_COLORS = 5;
  static float color[NUM_COLORS][3] = { {0.333, 0.804, 0.988},
                                        {0.969, 0.659, 0.722},
                                        {0.984, 0.976, 0.961},
                                        {0.969, 0.659, 0.722},
                                        {0.333, 0.804, 0.988} };
    // A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

  int idx1;        // |-- Our desired color will be between these two indexes in "color".
  int idx2;        // |
  float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

  if(value <= 0)      {  idx1 = idx2 = 0;            }    // accounts for an input <=0
  else if(value >= 1)  {  idx1 = idx2 = NUM_COLORS-1; }    // accounts for an input >=0
  else
  {
    value = value * (NUM_COLORS-1);        // Will multiply value by 3.
    idx1  = floor(value);                  // Our desired color will be after this index.
    idx2  = idx1+1;                        // ... and before this index (inclusive).
    fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
  }

  *red   = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
  *green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
  *blue  = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

// https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function
double ReadVoltage(byte pin){
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if(reading < 1 || reading > 4095) return 0;
  return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  // return -0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089;
} // Added an improved polynomial, use either, comment out as required
