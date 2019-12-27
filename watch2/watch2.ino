
#define SPI_SPEED SD_SCK_MHZ(4)

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
#include <algorithm>                // used for std::find and std::min and std::max
#include <stack>

#include "watch2.h"                 // defines and function prototypes
#include "globals.h"                // declatations for global variables

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

//declatations for each variable can be found in globals.h

// object creation
SPIClass *vspi = new SPIClass(VSPI);
Adafruit_SSD1351 oled = Adafruit_SSD1351(128, 96, vspi, cs, dc, rst);
Preferences preferences;
SdFat SD(&*vspi);
Adafruit_ImageReader reader(SD);

//button objects
Button btn_dpad_up(dpad_up, 25, false, false);
Button btn_dpad_down(dpad_down, 25, false, false);
Button btn_dpad_left(dpad_left, 25, false, false);
Button btn_dpad_right(dpad_right, 25, false, false);
Button btn_dpad_enter(dpad_enter, 25, false, false);
Button btn_zero(0);

std::map<std::string, std::vector<unsigned short int>> icons;
std::map<std::string, std::vector<unsigned char>> small_icons;

// global variables
int state = 0;
int state_init = 0;
RTC_DATA_ATTR int selected_menu_icon;
RTC_DATA_ATTR int boot_count = 0;
int trans_mode = 0;
int short_timeout = 5000;
int long_timeout = 30000;
bool timeout = true;
int themecolour = BLUE;
time_t alarm_snooze_time = 5*60;
uint8_t screen_brightness = 15;
uint8_t speaker_volume = 10;
uint8_t torch_brightness = 0;
int sd_state = 0;
int RTC_DATA_ATTR stopwatch_timing = 0;
uint32_t RTC_DATA_ATTR stopwatch_epoch = 0;
uint32_t RTC_DATA_ATTR stopwatch_paused_diff = 0;
uint32_t stopwatch_time_diff = 0;
uint32_t stopwatch_last_time_diff = 0;
uint32_t stopwatch_ms = 0, stopwatch_last_ms = 0;
uint32_t stopwatch_s = 0, stopwatch_last_s = 0;
uint32_t stopwatch_min = 0, stopwatch_last_min = 0;
uint32_t stopwatch_hour = 0, stopwatch_last_hour = 0;

std::vector<timerData> timers;
std::vector<alarmData> alarms;
int timer_trigger_status = 0;
int timer_trigger_id = 255;
int alarm_trigger_status = 0;
int alarm_trigger_id = 0;

int file_select_status = 0;
std::string file_path = "/";
bool file_select_dir_list_init = false;

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

#include "states/states.h"

////////////////////////////////////////
// setup function
////////////////////////////////////////

void setup() {

    //set cpu Frequency
    //setCpuFrequencyMhz(80);

    // configure gpio data direction registers
    pinMode(12, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(sdcs, OUTPUT);
    pinMode(sdcd, INPUT);
    pinMode(BATTERY_DIVIDER_PIN, INPUT);
    pinMode(IR_PIN, OUTPUT);

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);

    //begin serial
    Serial.begin(115200);


    //test putting sdcs High
    //format sd using sda tool
    //flowchart - ask


    //set up oled
    oled.begin(18000000);
    oled.fillScreen(0);
    //oled.setFont(&SourceSansPro_Regular6pt7b);
    uint8_t contrast = 0x0f;
    oled.sendCommand(0xC7, &contrast, 1);

    //set up SD card
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);
    
    initSD();

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);
    

    //set up preferences
    preferences.begin("watch2", false);      //open watch II preferences in RW mode
    timeout = preferences.getBool("timeout", false);
    short_timeout = preferences.getInt("short_timeout", 5000);
    long_timeout = preferences.getInt("long_timeout", 30000);
    themecolour = preferences.getInt("themecolour", BLUE);
    trans_mode = preferences.getBool("trans_mode", false);
    screen_brightness = preferences.getUChar("brightness", 15);
    preferences.end();

    //set up buttons
    btn_dpad_up.begin();
    btn_dpad_down.begin();
    btn_dpad_left.begin();
    btn_dpad_right.begin();
    btn_dpad_enter.begin();

    //set up torch
    ledcAttachPin(TORCH_PIN, 0);
    ledcSetup(0, 4000, 8);
    ledcWrite(0, 0);

    //set up time
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (boot_count == 0)
    {
        setTime(23, 58, 00, 28, 6, 2019);
        WiFi.mode(WIFI_OFF);
        btStop();

    }

    //set up icons
    registerAppIcons();
    registerSmallIcons();

    if (boot_count == 0)
    {
        //set selected menu icon to first non-hidden state
        selected_menu_icon = 0;
        while(1)
        {
            if (states[selected_menu_icon].hidden)
            {
                //check next state
                selected_menu_icon++;
            }
            else break;
        }
    }
    //else selected_menu_icon = states.find(selected_state);

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
    btn_zero.read();

    //check timers and alarms
    Alarm.delay(0);

    //run current state
    if (timer_trigger_status == 0 && alarm_trigger_status == 0  && file_select_status == false)
    {
        if ( state >= states.size() )
        {
            //dim screen
            uint8_t contrast = 0x0F;
            oled.sendCommand(0xC7, &contrast, 1);

            digitalWrite(12, HIGH);

            oled.setCursor(5, 10);
            oled.setTextColor(WHITE, BLACK);
            oled.print("no state has been\nloaded");
            oled.drawRGBBitmap(0, 29, coolcrab, 128, 55);
            if (dpad_any_active()) switchState(0);
        }
        else states[state].stateFunc();

    }
    else if (timer_trigger_status != 0) //handle timers
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
    else if (alarm_trigger_status != 0) //handle alarms
    {

        static bool selected_alarm_action = 0;
        static bool last_time = 0;
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH, 20);
        //0 - dismiss
        //anything else - sleep

        if (alarm_trigger_status == 1)
        {
            //dim screen
            dimScreen(0, 10);
            oled.fillScreen(BLACK);

            //brighten screen
            dimScreen(1, 10);

            //set timer trigger status
            alarm_trigger_status = 2;
        }

        if (dpad_left_active() || dpad_right_active())
        {
            if (selected_alarm_action == 0) selected_alarm_action = 1;
            else if (selected_alarm_action == 1) selected_alarm_action = 0;
        }

        if (now() != last_time)
        {
            canvas_time->setFont(&SourceSansPro_Light12pt7b);
            //canvas_time->setTextColor(WHITE);
            canvas_time->fillScreen(BLACK);
            canvas_time->setCursor(2, 16);
            canvas_time->printf("%02d:%02d:%02d", hour(), minute(), second());
            oled.drawBitmap(2, 16, canvas_time->getBuffer(), SCREEN_WIDTH, 20, themecolour, BLACK);
            last_time = now();
        }

        //draw buttons
        if (dpad_any_active() || alarm_trigger_status == 2)
        {
            //draw dismiss button
            oled.drawRoundRect(24, 42, 28, 28, 4, (!selected_alarm_action) ? themecolour : BLACK);
            oled.drawRGBBitmap(26, 44, icons["dismiss"].data(), 24, 24);

            //draw snooze button
            oled.drawRoundRect(SCREEN_WIDTH - 26 - 26, 42, 28, 28, 4, (selected_alarm_action) ? themecolour : BLACK);
            oled.drawRGBBitmap(SCREEN_WIDTH - 26 - 24, 44, icons["bed"].data(), 24, 24);

            //draw button text
            oled.fillRect(0, 71, SCREEN_WIDTH, SCREEN_HEIGHT - 71, BLACK);
            oled.setFont(&SourceSansPro_Regular6pt7b);
            String button = (selected_alarm_action) ? "Snooze" : "Dismiss";
            oled.getTextBounds(button, 24, 80, &x1, &y1, &w, &h);
            oled.setCursor(
                ( SCREEN_WIDTH / 2 ) - ( w / 2 ),
                80
            );
            oled.print(button);

            alarm_trigger_status = 3;  //hack hack hack hack
        }

        drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_alarm_action == 1)  //snooze alarm
            {
                time_t time_alarm = Alarm.read(alarms[alarm_trigger_id].alarm_id);
                time_alarm += alarm_snooze_time;
                Alarm.write(alarms[alarm_trigger_id].alarm_id, time_alarm);
            }
            else
            {
                Serial.print("i wanna be a girl and i can't type");
                Alarm.write(alarms[alarm_trigger_id].alarm_id, alarms[alarm_trigger_id].initial_time);
            }

            alarm_trigger_status = 0;
            switchState(state);
        }

    }
    else if (file_select_status) //show the file select dialogue
    {
        //i was watching tiktoks while writing this, so it might be pretty awful

        static int selected_icon = 0; //currently selected file
        static char filename[255]; //buffer to hold filename

        static std::vector<std::string> files2;      //vector of Strings representing names of files in directory
        static std::stack<int> selected_icon_stack; //stack for storing selected file indecies when navigating through subdirectories

        //handle dpad up press
        if (dpad_up_active()) 
        {
            selected_icon--;
            if (selected_icon < 0) selected_icon = files2.size() - 1;
        }

        //handle dpad down press
        if (dpad_down_active())
        {
            selected_icon++;
            if (selected_icon > files2.size() - 1) selected_icon = 0;
        }

        //handle dpad enter press
        if (dpad_enter_active())
        {
            //if cancel was pressed
            if (files2[selected_icon] == "Cancel")
            {
                //set the filename
                file_path = "canceled";

                //stop the file select menu being active
                file_select_status = false;

                //return to the calling state
                switchState(state);
            }
            else if (files2[selected_icon] == "..") //if parent directory was selected
            {
                char path[file_path.length()];
                strcpy(path, file_path.c_str());
                char *pch;
                file_path = "/";
                
                //get number of occurances of / character
                int occurances = 0;
                for (int i = 0; i < sizeof(path) / sizeof(char); i++) if (path[i] == '/') occurances++;
                
                //split the string
                pch = strtok(path, "/");
                
                for (int i = 0; i < occurances - 2; i++)
                {
                    file_path += pch;
                    file_path += "/";
                    pch = strtok(NULL, "/");
                }

                //reset the file select dialogue
                file_select_dir_list_init = false;

                //load selected icon index from the selected index stack
                selected_icon = selected_icon_stack.top();
                selected_icon_stack.pop();
            }
            else
            {
                //determine whether selected path is a directory
                File selected_file;
                selected_file = SD.open(files2[selected_icon].c_str());
                
                //if the path points to a directory
                if (selected_file.isDirectory())
                {
                    file_path += files2[selected_icon] + "/"; //set file select dialogue to subdirectory
                    file_select_dir_list_init = false;
                    selected_icon_stack.push(selected_icon);
                    selected_icon = 0; //reset selected icon

                }
                else //otherwise, assume the path points to a file
                {

                    //set the file path
                    file_path = file_path + files2[selected_icon];

                    //reset selected icon
                    selected_icon = 0;

                    //clear the selected icon stack
                    for (int i = 0; i < selected_icon_stack.size(); i++) selected_icon_stack.pop();

                    //stop the file select menu being active
                    file_select_status = false;

                    //return to the calling state
                    switchState(state);

                }
            }
            
        }

        //if the file select list hasn't been initalised
        if (!file_select_dir_list_init)
        {
            Serial.print("opening file dialogue for ");
            Serial.println(file_path.c_str());

            //dim screen
            dimScreen(0, 10);
            oled.fillScreen(BLACK);

            //populate files2 with the contents of the selected directory
            files2.clear();
            files2 = getDirFiles(file_path);

            //if card isn't initalised, notify the user
            if (sd_state != 1)
            {
                oled.setCursor(2, 36);
                oled.print("SD card not mounted");
            }
            else
            {
                //if there are no files, notify the user
                if (files2.size() == 0)
                {
                    oled.setCursor(2, 36);
                    oled.print("This directory is empty");
                }
                //add back button if in a non-root directory
                if (file_path != "/") files2.emplace(files2.begin(), "..");                
            }

            //add cancel option
            files2.emplace(files2.begin(), "Cancel");

            dimScreen(1, 10);
        }

        //if file select list hasn't been initliased, or any button is pressed, redraw the menu
        if (!file_select_dir_list_init || dpad_any_active())
        drawMenu(2, 12, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 12, files2, selected_icon, themecolour);

        //finish file select list initilisation
        if (!file_select_dir_list_init) file_select_dir_list_init = true;

        drawTopThing();
    }

    //wip screenshot tool
    //this doesn't work yet
    if (btn_zero.pressedFor(1000))
    {
        oled.drawPixel(0,0,BLUE);
        oled.startWrite();

        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                oled.setAddrWindow(x, y, 1, 1);
                oled.writeCommand(SSD1351_CMD_READRAM);
                vspi->transfer(0x12);
                uint8_t colour = vspi->transfer(0x12);
                Serial.printf("%02x", colour);
                colour = vspi->transfer(0x12);
                Serial.printf("%02x ", colour);
            }
            Serial.println();
        }
        


        oled.endWrite();
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


void drawTopThing(bool light)
{
    static double batteryVoltage = 4.2;
    static double batteryPercentage = 0;
    static int last_battery_reading = millis() - 1000;

    if (!light)
    {

        oled.drawFastHLine(0, 10, SCREEN_WIDTH, themecolour);
        oled.setCursor(1,8);
        oled.setTextColor(WHITE);
        oled.setTextSize(1);
        oled.setFont(&SourceSansPro_Regular6pt7b);
        oled.print("watch II");

        //oled.printf(" %d ", preferences.getBool("timeout", true));

        if (dpad_any_active())
        {
            oled.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK);
        }

        if ( millis() - last_battery_reading > 1000)
        {
            //batteryVoltage = ( (ReadVoltage(BATTERY_DIVIDER_PIN) * 3.3 ) / 4095.0 ) * 2;
            batteryVoltage = ReadVoltage(BATTERY_DIVIDER_PIN) * BATTERY_VOLTAGE_SCALE;
            batteryPercentage = ( batteryVoltage / BATTERY_VOLTAGE_MAX ) * 100.0;
            last_battery_reading = millis();

            //todo: clear last value
            oled.printf(" %.0f%%", batteryPercentage);
        }

    }

    //draw sd card status
    int sd_colour = ORANGE;
    switch(sd_state)
    {
        case 0: sd_colour = RED; break;    //didn't mount successfully
        case 1: sd_colour = 0x0660; break; //mounted successfully
        case 2: sd_colour = BLUE;          //sd card not present
    }

    oled.drawBitmap(
        SCREEN_WIDTH - 13,
        1,
        small_icons["small_sd"].data(),
        11,
        8,
        sd_colour,
        BLACK
    );
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
    if (direction) for (uint8_t contrast = 0; contrast < screen_brightness + 1; contrast++)
    {
        oled.sendCommand(0xC7, &contrast, 1);
        delay(pause_thing);
    }
    else for (uint8_t contrast = screen_brightness; contrast > 0; contrast--)
    {
        uint8_t contrast_step = contrast;
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
    //selected_state = selected_menu_icon->first;

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
            if (time_left < next_alarm_time || next_alarm_time == -1) next_alarm_time = time_left;
        }
    }
    for (int i = 0; i < alarms.size(); i++)
    {
        tmElements_t current_unix_time_without_the_time = {
            0, 0, 0, weekday(), day(), month(), year() - 1970
        };
        if (alarms[i].paused == false &&
            ( ( now() - makeTime(current_unix_time_without_the_time) ) < Alarm.read(alarms[i].alarm_id) )
        )
        {
            //to calculate the time until an alarm goes off, the current time is subtracted
            //from the alarm time.  the current time is stored in a unix timestamp format, that is,
            //the number of seconds since 1 Jan 1970.  the alarm time is also stored as a unix
            //timestamp, but it doesn't consider days.  the current time timestamp considers
            //the hours, minutes, seconds, days, months, and years, expressed as seconds, while the
            //alarm timestamp only considers the hours, minutes, and seconds.
            //for this reason, the current day in unix format (where h=0, m=0, s=0) is added
            //to the alarm time.  the current time is subtracted from the calculated alarm time
            //(which now represents the actual unix time when the alarm will go off), giving the
            //time until the alarm goes off in seconds
            time_t time_left = ( Alarm.read(alarms[i].alarm_id) + makeTime(current_unix_time_without_the_time) ) - now();
            if (time_left < next_alarm_time || next_alarm_time == -1) next_alarm_time = time_left;
        }
    }

    //if an timer or an alarm has been set, set the device to wake up just before the alarm triggers
    if (next_alarm_time > -1)
    {
        esp_sleep_enable_timer_wakeup(next_alarm_time * 1000 * 1000 - 500);
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

    //init SD card
    initSD();

    //rtc_gpio_deinit(GPIO_NUM_26);
    if (next_alarm_time > -1) switchState(0, 0, 0, 0, true);
    else switchState(0);
}

void drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, int colour)
{
    static int16_t x1, y1;
    static uint16_t w=0, w2=0, h=0, h2=0;
    static int padding = 4;
    static int radius = 4;
    
    //clear screen where menu will be drawn
    oled.fillRect(x, y, width, height, BLACK);

    x += padding;

    //get text dimensions
    oled.getTextBounds(String(items[0].c_str()), x, y, &x1, &y1, &w, &h);

    //get total height of button (incl. padding)
    int ht = h + padding + padding + padding;
    
    //calculate how many items are onscreen (not considering y offset)
    int onscreen_items = height / ht;

    //calculate how many items are onscreen after the offset threshold but before the height of the menu
    int threshold_items = onscreen_items - ( (height - ht) / ht );

    //calculate y index of selected item
    int selected_y_index = y + ((selected + 1) * ht);

    //calculate offset threshold based on selected item
    //if the selected item has a y index greater than this value, the offset will be non-zero
    int y_offset = 0;
    if (selected_y_index > ((y + height) - (ht))) 
    {
        y_offset = ht * (selected - threshold_items - 1);
        Serial.println(y_offset);
    }

    //print each menu item
    int fridgebuzz = 0;
    for (const std::string &item : items)
    {
        //calculate the item's y position
        int item_ypos = y - y_offset;

        //if item is within the area in which the menu is to be drawn
        if (item_ypos >= ( y - ht) || item_ypos < (y + height + ht))
        {

            //draw the item rounded rectangle

            //if the current item is the selected item
            if (fridgebuzz == selected)
            {
                //draw a filled in rounded rectangle
                oled.fillRoundRect(x, y - y_offset, width - (padding*2), h + padding + padding, radius, colour);
                oled.setTextColor(BLACK);
            }
            else
            {
                //draw the outline of a rounded rectangle
                oled.drawRoundRect(x, y - y_offset, width - (padding*2), h + padding + padding, radius, colour);
                oled.setTextColor(colour);
            }

            //draw the item text
            String itemtext = "";

            //get the length of the text
            oled.getTextBounds(String(item.c_str()), x + padding, y + h + padding - y_offset, &x1, &y1, &w, &h2);

            //if the text is too long for the item button
            if (w > (width - (padding * 4)))
            {

                //this is a _really_ inefficient idea
                //iterate through each letter until the length of the button is reached

                //find the width of the string "..." and store it in w2
                oled.getTextBounds("...", x + padding, y + h + padding, &x1, &y1, &w2, &h2);

                //running value of item text length
                int text_length = 0;

                //for each letter
                for (int i = 0; i < item.length(); i++)
                {
                    //get width of character
                    oled.getTextBounds(String(item[i]), x + padding, y + h + padding, &x1, &y1, &w, &h2);

                    //add width to running value
                    //really, the character width should be added to this value,
                    //but for some reason, the character width calculated by oled.getTextBounds()
                    //above isn't correct
                    text_length += 6;

                    //if the text would be too long (idk im tired)
                    //the limit is the width - padding - the length of "..."
                    if (text_length > (width - (padding * 4) - w2))
                    {
                        //add "..." to the item text, and break the loop
                        itemtext += "...";
                        break;
                    }
                    else
                    {
                        //add the character to the item text
                        itemtext += String(item[i]);
                    }
                    
                }

            }
            else itemtext = String(item.c_str());

            //print the text
            oled.setCursor(x + padding, y + h + padding - y_offset);
            oled.print(itemtext);

            y += ht;

            fridgebuzz++;

        }
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

std::vector<std::string> getDirFiles(std::string path)
{
    //vector to store files in the directory
    std::vector<std::string> files;

    //buffer to store filenames
    char filename[255];

    //enable the sd card
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);

    //initalise the sd card (without changing any CS pin)
    if( initSD(false) == 0 ){
        
        //sd could not be accessed
        Serial.println("getDirFiles failed because no");
        return files;

    }
    else
    {

        digitalWrite(cs, HIGH);
        digitalWrite(sdcs, LOW);

        //open the dir at the path
        File root = SD.open(path.c_str());

        //check that path is valid
        if (!root)
        {
            //file path is invalid
            return files;
        }

        //check that path is actually a dir
        if (!root.isDirectory())
        {
            //path is not a directory
            return files;
        }

        while(true)
        {
            Serial.print("f");

            //open next file in dir
            File f = root.openNextFile();

            //if there are no more files, break
            if (!f) break;

            //get the name of the file
            f.getName(filename, 255);

            //add the name to the vector
            files.push_back(std::string(filename));

            f.close();
        }

        root.close();

        Serial.println();

    }


    //disable the sd card
    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);

    return files;
}

void beginFileSelect(std::string path)
{
    file_select_status = true;
    file_path = path;
    file_select_dir_list_init = false;
}

int initSD(bool handleCS)
{




    //add CS stuff to quickstart example
    //enable CRC checks




    int no = 0;

    //enable the sd card
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);

    //initalise the sd card
    if(!SD.begin(sdcs, SPISettings(9000000, MSBFIRST, SPI_MODE0))){

        //card couldn't mount
        Serial.println("initSD() - Couldn't mount SD card");
        Serial.print("\tError code: ");
        Serial.printf("0x%x\n", SD.cardErrorCode());
        Serial.print("\tError data: ");
        Serial.printf("0x%x\n", SD.cardErrorData());
        no = 0;

    }
    else
    {

        //card mounted successfully
        Serial.println("initSD() - Successfully mounted SD card");
        Serial.printf("Card size: %d\n", SD.card()->cardSize());
        //SD.ls(LS_R | LS_DATE | LS_SIZE);
        no = 1;

    }

    //if there is no sd card inserted, set no to 2
    //if (!digitalRead(sdcd)) no = 2;

    //set global sd state variable, and return
    sd_state = no;
    return no;
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

