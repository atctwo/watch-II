#include "watch2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "libraries/stb/stb_image_resize.h"

#include "esp_bt.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Arduino.h"

namespace watch2
{
    // object creation
    SPIClass *vspi = new SPIClass(VSPI);
    TFT_eSPI oled = TFT_eSPI();
    Preferences preferences;
    SdFat SD(&*vspi);
    //Adafruit_ImageReader reader(SD);
    TFT_eSprite top_thing = TFT_eSprite(&oled);
    TFT_eSprite framebuffer = TFT_eSprite(&oled);
    WiFiClient wifi_client;
    WiFiClientSecure wifi_client_secure;
    BleKeyboard ble_keyboard("watch2", "atctwo");

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
    uint8_t top_thing_height = oled.fontHeight() + 20;
    bool forceRedraw = false;
    bool forceRedrawLooped = false;
    bool showingControlCentre = false;
    uint16_t trans_mode = 0;
    bool animate_watch_face = false;
    int short_timeout = 5000;
    int long_timeout = 30000;
    bool timeout = true;
    int themecolour = BLUE;
    time_t alarm_snooze_time = 5*60;
    uint16_t screen_brightness = 2^tftbl_resolution;
    uint8_t speaker_volume = 10;
    uint8_t torch_brightness = 0;
    int8_t timezone = 0;
    bool ntp_wakeup_connect = true;
    bool ntp_boot_connect = true;
    bool ntp_boot_connected = true;
    bool wifi_wakeup_reconnect = true;
    bool wifi_boot_reconnect = true;
    bool wifi_enabled = false;
    String weather_location = "";
    wifi_auth_mode_t wifi_encryption = WIFI_AUTH_MAX;
    uint8_t bluetooth_state = 0;
    bool ble_set_up = false;
    int sd_state = 0;
    bool spiffs_state = 0;
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

    uint8_t wifi_state = 0;
    uint8_t wifi_reconnect_attempts = 0;
    uint8_t initial_wifi_reconnect_attempts = 0;
    uint32_t wifi_connect_timeout = 15000;
    uint32_t wifi_connect_timeout_start = 0;

    bool dpad_up_lock = false;
    bool dpad_down_lock = false;
    bool dpad_left_lock = false;
    bool dpad_right_lock = false;
    bool dpad_enter_lock = false;

    void startLoop()
    {
        //initial button setup
        btn_dpad_up.read();
        btn_dpad_down.read();
        btn_dpad_left.read();
        btn_dpad_right.read();
        btn_dpad_enter.read();
        btn_zero.read();

        //check timers and alarms
        Alarm.delay(0);

        //if the current state uses a framebuffer, clear it
        if (states[state].framebuffer) framebuffer.fillScreen(BLUE);
    }

    void endLoop()
    {
        // Serial.printf("internal RAM: %2.4f%%\n", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);
        // Serial.printf("external RAM: %2.4f%%\n", ((float)(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize()) * 100);

        //wip screenshot tool
        //this doesn't work yet
        // if (btn_zero.pressedFor(1000))
        // {
        //     oled.drawPixel(0,0,BLUE);
            
        //     for (int y = 0; y < SCREEN_HEIGHT; y++)
        //     {
        //         for (int x = 0; x < SCREEN_WIDTH; x++)
        //         {
        //             uint16_t colour[1];
        //             oled.readRect(x, y, 1, 1, colour);
        //             Serial.printf("%d ", colour[0]);
        //             //Serial.printf("%3d %3d\n", x, y);
        //         }
        //         Serial.println();
        //     }
            
        // }

        if (btn_zero.wasPressed())
        {
            watch2::controlCentreDialogue();
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

        // wifi
        // check the wifi connection status
        if (wifi_state == 4) // pls connect asap
        {
            if (WiFi.status() != WL_CONNECTED) 
            {
                if (wifi_reconnect_attempts > 0) connectToWifiAP();
                else wifi_state = 1;
            }
            else wifi_state = 3;
        }

        if (wifi_state == 2) // connecting
        {
            // wifi connection timeout
            if ((millis() - wifi_connect_timeout_start) > wifi_connect_timeout)
            {
                Serial.println("[Wifi] connection timed out");
                WiFi._setStatus(WL_CONNECT_FAILED);
                Serial.println(millis());
            }

            if (WiFi.status() == WL_CONNECTED)
            {

                // update wifi profiles list
                cJSON *profiles = getWifiProfiles();
                if (profiles)
                {
                    // Serial.println("profile list before adding new profile");
                    // Serial.println(cJSON_Print(profiles));

                    // is ssid in profile list?
                    bool help = false;
                    cJSON *profile;
                    cJSON *profile_array = cJSON_GetObjectItem(profiles, "profiles");
                    for (int i = 0; i < cJSON_GetArraySize(profile_array); i++)
                    {
                        profile = cJSON_GetArrayItem(profile_array, i);
                        const char* ssid = cJSON_GetObjectItem(profile, "ssid")->valuestring;
                        if (strcmp(ssid, WiFi.SSID().c_str()) == 0) // if profile ssid matches connected ssid
                        {
                            help = true;
                            break;
                        }
                    }

                    // if there wasn't a match (the ap doesn't have a profile in the profile list)
                    if (!help)
                    {
                        // create ap profile
                        profile = cJSON_CreateObject();
                        cJSON_AddStringToObject(profile, "ssid", WiFi.SSID().c_str());
                        cJSON_AddStringToObject(profile, "password", WiFi.psk().c_str());
                        cJSON_AddNumberToObject(profile, "encryption", wifi_encryption);

                        // Serial.println("new profile");
                        // Serial.println(cJSON_Print(profile));

                        // add profile to profile list
                        cJSON_AddItemToArray(profile_array, profile);
                    }

                    // Serial.println("profile list after adding new profile");
                    // Serial.println(cJSON_Print(profiles));

                    // update access index
                    cJSON *access_index = cJSON_GetObjectItem(profiles, "access_index");
                    if (access_index)
                    {
                        // linear search for ssid index
                        int index = -1;
                        for (int i = 0; i < cJSON_GetArraySize(access_index); i++)
                        {
                            const char *profile_ssid = cJSON_GetArrayItem(access_index, i)->valuestring;
                            if (strcmp(profile_ssid, WiFi.SSID().c_str()) == 0)
                            {
                                index = i;
                                break;
                            }
                        }

                        // if the ssid is in the list
                        if (index > -1)
                        {
                            // pop ssid from list
                            cJSON_DeleteItemFromArray(access_index, index);
                        }

                        // push ssid to start of list
                        cJSON_InsertItemInArray(access_index, 0, cJSON_CreateString(WiFi.SSID().c_str()));
                    }
                    else Serial.println("could not access access index");

                    // Serial.println("profile list after updating access index");
                    // Serial.println(cJSON_Print(profiles));

                    // write update profile list to file
                    setWifiProfiles(profiles);

                    // clear up memory
                    Serial.println("[Wifi] freeing memory for profiles");
                    cJSON_Delete(profiles);
                }
                else Serial.println("[Wifi] couldn't access profile list");

                // set wifi state
                wifi_state = 3;
                Serial.printf("[Wifi] connected to %s\n", WiFi.SSID().c_str());
            }
            
            if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_NO_SSID_AVAIL)
            {
                if (wifi_reconnect_attempts == 0) // out of reconnect attempts
                {
                    WiFi.disconnect();
                    wifi_state = 1;
                    Serial.println("[WiFi] could not connect to AP");
                }
                else
                {
                    wifi_reconnect_attempts--;
                    Serial.printf("[Wifi] failed to connect to AP, %d attempts remaining\n", wifi_reconnect_attempts);
                    connectToWifiAP();
                }
            }
        }

        if (wifi_state == 3) // connected
        {
            // set ntp time
            if (!watch2::ntp_boot_connected)
            {
                getTimeFromNTP();
                watch2::ntp_boot_connected = true;
            }

            // if wifi disconnected
            if (WiFi.status() == WL_DISCONNECTED || WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_NO_SSID_AVAIL)
            {
                wifi_state = 1;
                Serial.println("[WiFi] disconnected from AP");
            }
        }

        // bluetooth
        if (bluetooth_state == 2 || bluetooth_state == 3)
        {
            if (ble_keyboard.isConnected())
            {
                Serial.println("[Bluetooth] connected");
                bluetooth_state = 3;
            }
            else
            {
                //Serial.println("[Bluetooth] disconnected");
                bluetooth_state = 2;
            }
        }

        // redraw stuff
        if (forceRedraw)
        {
            if (forceRedrawLooped)
            {
                forceRedraw = false;
                forceRedrawLooped = false;
            }
            else
            {
                forceRedrawLooped = true;
            }
        }

        // if the current state uses a framebuffer, draw it to the tft
        if (states[state].framebuffer) framebuffer.pushSprite(0, 0);
    }


    void drawTopThing(bool light)
    {
        static char text[4];
        static double batteryVoltage = 4.2;
        static double batteryPercentage = 0;
        static int last_battery_reading = millis() - 1000;
        static uint16_t icon_size = 20;
        static uint16_t icon_padding = 5;
        uint16_t icon_xpos = SCREEN_WIDTH;
        uint16_t milliseconds = 0;

        top_thing.fillRect(0, 0, top_thing.width(), top_thing.height(), BLACK);

        if (!light)
        {
            //top_thing_height = top_thing.fontHeight() + 10;
            top_thing.drawFastHLine(0, top_thing.fontHeight(), SCREEN_WIDTH, themecolour);
            top_thing.drawFastHLine(0, top_thing.fontHeight() + 1, SCREEN_WIDTH, themecolour);
            top_thing.drawFastHLine(0, top_thing.fontHeight() + 2, SCREEN_WIDTH, themecolour);

            top_thing.setCursor(1,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.setTextSize(1);
            //top_thing.setFreeFont(&SourceSansPro_Regular6pt7b);
            top_thing.printf("%02d:%02d", hour(), minute());

            if ( millis() - last_battery_reading > 1000)
            {
                //batteryVoltage = ( (ReadVoltage(BATTERY_DIVIDER_PIN) * 3.3 ) / 4095.0 ) * 2;
                //batteryVoltage = ReadVoltage(BATTERY_DIVIDER_PIN) * BATTERY_VOLTAGE_SCALE;
                batteryPercentage = 69.0; ( batteryVoltage / BATTERY_VOLTAGE_MAX ) * 100.0;
                last_battery_reading = millis();
            }

            // print battery
            sprintf(text, "%.0f", batteryPercentage);
            icon_xpos -= (watch2::oled.textWidth(text) + icon_padding);
            top_thing.setCursor(icon_xpos,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.printf(text);

            icon_xpos -= (icon_size + icon_padding);
            top_thing.drawBitmap(
                icon_xpos,
                1,
                small_icons["small_battery"].data(),
                icon_size,
                icon_size,
                WHITE
            );
            
            // print ram usage
            sprintf(text, "%2.0f", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);
            icon_xpos -= (watch2::oled.textWidth(text) + icon_padding);
            top_thing.setCursor(icon_xpos,1);
            top_thing.setTextColor(WHITE, BLACK);
            top_thing.printf(text);

            icon_xpos -= (icon_size + icon_padding);
            top_thing.drawBitmap(
                icon_xpos,
                1,
                small_icons["small_ram"].data(),
                icon_size,
                icon_size,
                WHITE
            );

        }

        //draw sd card status
        int sd_colour = ORANGE;
        switch(sd_state)
        {
            case 0: sd_colour = RED; break;    //didn't mount successfully
            case 1: sd_colour = 0x0660; break; //mounted successfully
            case 2: sd_colour = BLUE;          //sd card not present
        }

        icon_xpos -= (icon_size + icon_padding);
        top_thing.drawBitmap(
            icon_xpos,
            1,
            small_icons["small_sd"].data(),
            icon_size,
            icon_size,
            sd_colour
        );

        // draw wifi icon
        if (wifi_state > 0) // if wifi is enabled
        {
            icon_xpos -= (icon_size + icon_padding);
            switch(wifi_state)
            {
                case 1: // enabled, disconnected
                    
                    top_thing.drawBitmap(
                        icon_xpos,
                        1,
                        small_icons["small_wifi_complete"].data(),
                        icon_size,
                        icon_size,
                        0x4A69
                    );
                    break;

                case 2: // enabled, connecting
                    
                    milliseconds = millis() % 1000;

                    // bar 0
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_0"].data(), icon_size, icon_size, (0 <= milliseconds && milliseconds <= 250) ? WHITE : 0x6B4D);
                    
                    // bar 1
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_1"].data(), icon_size, icon_size, (251 <= milliseconds && milliseconds <= 500) ? WHITE : 0x6B4D);

                    // bar 2
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_2"].data(), icon_size, icon_size, (501 <= milliseconds && milliseconds <= 750) ? WHITE : 0x6B4D);

                    // bar 3
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_3"].data(), icon_size, icon_size, (751 <= milliseconds && milliseconds <= 1000) ? WHITE : 0x6B4D);

                    break;

                case 3: // enabled, connected
                    
                    // draw the wifi symbol, but hightlight each "bar" depending on the wifi signal strength

                    // bar 0
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_0"].data(), icon_size, icon_size, (WiFi.RSSI() >= -80) ? WHITE : 0x6B4D);
                    
                    // bar 1
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_1"].data(), icon_size, icon_size, (WiFi.RSSI() >= -70) ? WHITE : 0x6B4D);
                    
                    // bar 2
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_2"].data(), icon_size, icon_size, (WiFi.RSSI() >= -67) ? WHITE : 0x6B4D);
                    
                    // bar 3
                    top_thing.drawBitmap(icon_xpos, 1, small_icons["small_wifi_3"].data(), icon_size, icon_size, (WiFi.RSSI() >= -30) ? WHITE : 0x6B4D);
                    break;
            }
        }

        top_thing.pushSprite(0, 0);
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
        Serial.printf("switching to new state: %d (%s)\n", newState, watch2::states[newState].stateName.c_str());

        if (dim_pause_thing > 0)
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
        if (bright_pause_thing > 0)
        dimScreen(1, bright_pause_thing);           //make the screen brighter

    }

    void dimScreen(bool direction, int pause_thing)
    {

        // direction
        // 0 - decrease brightness
        // 1 - increase brightness
        if (direction) for (uint16_t contrast = 0; contrast < screen_brightness + 1; contrast++)
        {
            ledcWrite(TFTBL_PWM_CHANNEL, contrast);
            delay(std::max(pause_thing / screen_brightness, 1));
        }
        else for (uint16_t contrast = screen_brightness; contrast > 0; contrast--)
        {
            ledcWrite(TFTBL_PWM_CHANNEL, contrast);
            delay(std::max(pause_thing / screen_brightness, 1));
        }
    }

    void deepSleep(int pause_thing)
    {
        //fade to black
        dimScreen(0, pause_thing);
        oled.fillScreen(BLACK);

        // turn off display
        // https://www.rhydolabz.com/documents/33/ST7789.pdf, pg182
        // https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/Adafruit_ST77xx.h#L46
        oled.writecommand(0x10);

        //save selected_state
        //selected_state = selected_menu_icon->first;

        //set time
        timeval tv{
            now(),
            0
        };

        settimeofday(&tv, NULL);

        //deep sleep setup
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 1); //1 = High, 0 = Low

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
        oled.writecommand(0x11); // wake up screen

        //set up buttons
        btn_dpad_up.begin();
        btn_dpad_down.begin();
        btn_dpad_left.begin();
        btn_dpad_right.begin();
        btn_dpad_enter.begin();

        //init SD card
        initSD();

        // reconnect to wifi
        if (wifi_state != 0 && wifi_wakeup_reconnect)
        {
            enable_wifi();
        }

        // time??? what is it really?
        if (watch2::ntp_wakeup_connect) watch2::ntp_boot_connected = false;

        //rtc_gpio_deinit(GPIO_NUM_26);
        if (next_alarm_time > -1) switchState(0, 0, 0, 0, true);
        else switchState(0);
    }

    void drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, bool scroll, bool centre, int colour)
    {
        static int16_t x1, y1;
        static uint16_t w=0, w2=0, h=0, h2=0;
        static int padding = 4;
        static int radius = 4;
        
        //clear screen where menu will be drawn
        oled.fillRect(x, y, width, height, BLACK);

        x += padding;

        //get text dimensions
        w = oled.textWidth(items[0].c_str());
        h = oled.fontHeight();

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
        if (scroll) 
        {
            if (selected_y_index > ((y + height) - (ht))) 
            {
                y_offset = ht * ((selected + 2) - onscreen_items);
            }
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
                    oled.setTextColor(BLACK, colour);
                }
                else
                {
                    //draw the outline of a rounded rectangle
                    oled.drawRoundRect(x, y - y_offset, width - (padding*2), h + padding + padding, radius, colour);
                    oled.setTextColor(colour, BLACK);
                }

                //draw the item text
                String itemtext = "";

                //get the length of the text
                //watch2::getTextBounds(String(item.c_str()), x + padding, y + h + padding - y_offset, &x1, &y1, &w, &h2);
                w = watch2::oled.textWidth(item.c_str());

                //if the text is too long for the item button
                if (w > (width - (padding * 6)))
                {

                    //this is a _really_ inefficient idea
                    //iterate through each letter until the length of the button is reached

                    //find the width of the string "..." and store it in w2
                    w2 = oled.textWidth("...");
                    h = oled.fontHeight();

                    //running value of item text length
                    int text_length = 0;

                    //for each letter
                    for (int i = 0; i < item.length(); i++)
                    {
                        //get width of character
                        w = oled.textWidth(String(item[i]).c_str());

                        //add width to running value
                        //really, the character width should be added to this value,
                        //but for some reason, the character width calculated by watch2::getTextBounds()
                        //above isn't correct
                        text_length += w;

                        //if the text would be too long (idk im tired)
                        //the limit is the width - padding - the length of "..."
                        if (text_length > (width - (padding * 10) - w2))
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
                if (centre)
                {
                    oled.setTextDatum(TC_DATUM);
                    oled.drawString(itemtext, x + (width / 2), y + padding);
                    oled.setTextDatum(TL_DATUM);
                }
                else
                {
                    oled.setCursor(x + padding, y + padding - y_offset);
                    oled.print(itemtext);
                }

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
            int cursor_y_pos = y + ypos;
            int outline_width;
            int outlinecolour;

            // draw settings title
            oled.setCursor(x, cursor_y_pos);
            watch2::getTextBounds(String(items[i].setting_title), x, cursor_y_pos, &x1, &y1, &w, &h);
            oled.print(items[i].setting_title);
            text_height = h;

            // determine settings value
            String final_setting_value;
            if (items[i].setting_value < items[i].text_map.size()) final_setting_value = items[i].text_map[items[i].setting_value];
            else final_setting_value = String(items[i].setting_value);

            // clear previous settings value
            if (last_selected_element == i && last_selected_value != final_setting_value)
            {
                digitalWrite(12, HIGH);
                watch2::getTextBounds(last_selected_value, x, cursor_y_pos, &x1, &y1, &w, &h);
                outline_width = std::max(items[i]. min_field_width, w + (2 * padding));

                oled.fillRect( (x + (width - padding - outline_width)) - padding,
                                cursor_y_pos - padding,
                                outline_width,
                                h + (2 * padding),
                                BLACK
                );
            }
            else digitalWrite(12, LOW);

            // draw settings value
            watch2::getTextBounds(final_setting_value, x, cursor_y_pos, &x1, &y1, &w, &h);
            outline_width = std::max(items[i]. min_field_width, w + (2 * padding));
            outlinecolour = BLACK;

            oled.setCursor(x + (width - padding - outline_width), cursor_y_pos);
            oled.print(final_setting_value);

            //draw outline around value if element is selected
            if (selected == i) outlinecolour = colour;

            oled.drawRoundRect( (x + (width - padding - outline_width)) - padding,
                                cursor_y_pos - padding,
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

    std::string beginFileSelect(std::string path)
    {
        //i was watching tiktoks while writing this, so it might be pretty awful

        static int selected_icon = 0; //currently selected file
        static char filename[255]; //buffer to hold filename

        static std::vector<std::string> files2;      //vector of Strings representing names of files in directory
        static std::stack<int> selected_icon_stack; //stack for storing selected file indecies when navigating through subdirectories

        //call once
        file_path = path;
        file_select_dir_list_init = false;

        while(true)
        {
            startLoop();

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
                    switchState(state, states[state].variant, 10, 10, true);
                    return file_path;
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
                    std::string filename = file_path + files2[selected_icon];
                    selected_file = SD.open(filename.c_str());
                    
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

                        //dim the screen and return to the calling state
                        switchState(state, states[state].variant, 10, 10, true);
                        return file_path;

                    }
                }
                
            }

            //if the file select list hasn't been initalised
            if (!file_select_dir_list_init)
            {
                Serial.print("opening file dialogue for ");
                Serial.println(file_path.c_str());

                //dim screen
                dimScreen(0, top_thing_height);
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
            if (watch2::forceRedraw || !file_select_dir_list_init || dpad_any_active())
            {
                drawMenu(2, top_thing_height, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 12, files2, selected_icon, themecolour);
            };

            //finish file select list initilisation
            if (!file_select_dir_list_init) file_select_dir_list_init = true;

            drawTopThing();
            endLoop();

        }
    }

    std::string dir_name(std::string file_path_thing)
    {
        char path[file_path_thing.length()];
        strcpy(path, file_path_thing.c_str());
        char *pch;
        std::string file_dir = "/";
        
        //get number of occurances of / character
        int occurances = 0;
        for (int i = 0; i < sizeof(path) / sizeof(char); i++) if (path[i] == '/') occurances++;
        
        //split the string
        pch = strtok(path, "/");
        
        for (int i = 0; i < occurances - 2; i++)
        {
            file_dir += pch;
            file_dir += "/";
            pch = strtok(NULL, "/");
        }

        return file_dir;
    }

    std::string file_ext(std::string file_path_thing)
    {
        // adapted from https://stackoverflow.com/a/51992/9195285
        std::string::size_type idx;
        idx = file_path_thing.rfind('.');

        if(idx != std::string::npos)
        {
            std::string extension = file_path_thing.substr(idx+1);
            // adapted from https://stackoverflow.com/a/313990/9195285
            std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c){ return std::tolower(c); });
            return extension;
        }
        else
        {
            // No extension found
            return "";
        }
    }

    std::string textFieldDialogue(std::string prompt, const char *default_input, const char mask, bool clear_screen)
    {
        /*
        special keys (like caps lock, space, backspace, enter) are mapped to repurposed ASCII control codes.
        the codes are interpreted by the system, so that no control codes are actually returned as part of 
        the returned string.  i tried to pick control codes that match the function of the key as much as
        possible.  the mappings are described in this table:

        key             ascii (hex)     ascii description
        blank           0x00            null
        backspace       0x08            backspace
        clear           0x0c            form feed
        enter           0x04            end of transmission
        cancel          0x18            cancel
        cursor left     0x11            device control 1
        cursor right    0x12            device control 2
        space           0x20            space
        caps lock       0x13            device control 3
        symbols         0x0e            shift out
        letters         0x0f            shift in
        */

        // get the number of newlines in the prompt
        // adapted from https://stackoverflow.com/a/14266139/9195285
        std::string delimiter = "\n";
        size_t pos = 0;
        uint8_t no_newlines = 0;
        std::string token;
        while ((pos = prompt.find(delimiter, pos)) != std::string::npos) {
            pos++;
            no_newlines++;
        }

        uint8_t key_h = oled.fontHeight();
        uint8_t key_w = key_h;
        uint8_t no_rows = 4;
        uint8_t no_cols = 11;
        uint8_t box_radius = 10;
        uint8_t key_radius = 7;
        bool finish_input = false;

        bool caps_lock = false;
        bool last_caps_lock = false;
        uint8_t selected_row = 0;
        uint8_t selected_col = 0;
        uint8_t selected_page = 0;
        uint8_t last_page = 0;
        std::vector<std::vector<std::vector<char>>> keys = {
            {
                {'1',  '2', '3', '4', '5', '6', '7', '8', '9',  '0',  0x11},
                {'q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p',  0x12},
                {0x13, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',  'l',  0x20},
                {0x0e, 'z', 'x', 'c', 'v', 'b', 'n', 'm', 0x08, 0x18, 0x04}
            },
            {
                {'!',  '"', 0,   '$',  '%',  '^', '&', '*', '(',  ')',  0x11},
                {':',  ';', '@', '\'', '~',  '#', '-', '_', '`',  0,    0x12},
                {'.',  ',', '?', '/',  '\\', '|', '+', '=', 0,    0,    0x20},
                {0x0f, '<', '>', '{',  '}',  '[', ']', 0,   0x08, 0x0c, 0x04}
            }
        };

        uint16_t keyboard_w = SCREEN_WIDTH * 0.95;
        uint16_t keyboard_a = (SCREEN_WIDTH - keyboard_w) / 2;
        uint16_t keyboard_h = (no_rows * oled.fontHeight()) + (keyboard_w - (key_w * no_cols));
        uint16_t keyboard_x = SCREEN_WIDTH - keyboard_a - keyboard_w;
        uint16_t keyboard_y = SCREEN_HEIGHT - keyboard_a - keyboard_h;

        uint16_t key_matrix_w = (key_w * no_cols);
        uint16_t key_matrix_h = (key_h * no_rows);
        uint16_t key_matrix_x = keyboard_x + ((keyboard_w - key_matrix_w) / 2);
        uint16_t key_matrix_y = keyboard_y + ((keyboard_h - key_matrix_h) / 2);

        uint16_t input_box_x = 15;
        uint16_t input_box_y = top_thing_height;
        uint16_t input_box_w = SCREEN_WIDTH - (input_box_x * 2);
        uint16_t input_box_h = SCREEN_HEIGHT - (2 * keyboard_a) - keyboard_h - top_thing_height;

        uint8_t input_field_padding = 2;
        uint16_t input_field_w = input_box_w * 0.9;
        uint16_t input_field_h = oled.fontHeight() + (input_field_padding * 2);
        uint16_t input_group_h = input_field_h + (oled.fontHeight() * (no_newlines + 1));
        uint16_t input_field_x = input_box_x + ((input_box_w - input_field_w) / 2);
        uint16_t input_group_y = input_box_y + ((input_group_h - input_field_h) / 2);
        uint16_t input_field_y = input_group_y + (oled.fontHeight() * (no_newlines + 1));

        std::string input = default_input;

        // draw input box
        oled.fillRoundRect(input_box_x, input_box_y, input_box_w, input_box_h, box_radius, BLACK);
        oled.drawRoundRect(input_box_x, input_box_y, input_box_w, input_box_h, box_radius, themecolour);

        // draw input field
        // also adapted from https://stackoverflow.com/a/14266139/9195285
        pos = 0;
        uint8_t line_num = 0;
        while ((pos = prompt.find(delimiter)) != std::string::npos) {
            token = prompt.substr(0, pos);
            oled.setCursor(input_field_x, input_group_y + (oled.fontHeight() * line_num));
            oled.setTextColor(WHITE, BLACK);
            oled.print(token.c_str());
            prompt.erase(0, pos + delimiter.length());
            line_num++;
        }
        oled.setCursor(input_field_x, input_group_y + (oled.fontHeight() * line_num));
        oled.setTextColor(WHITE, BLACK);
        oled.print(prompt.c_str());
        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);

        // draw keyboard box
        oled.fillRoundRect(keyboard_x, keyboard_y, keyboard_w, keyboard_h, box_radius, BLACK);
        oled.drawRoundRect(keyboard_x, keyboard_y, keyboard_w, keyboard_h, box_radius, themecolour);

        // draw keys
        auto draw_keys = [&]() {
            //oled.drawRect(key_matrix_x, key_matrix_y, key_matrix_w, key_matrix_h, ORANGE);
            uint8_t selected_key = selected_col + (no_cols * selected_row);
            oled.setTextDatum(MC_DATUM);
            for (int y = 0; y < no_rows; y++)
            {
                for (int x = 0; x < no_cols; x++)
                {
                    uint8_t index = x + (no_cols * y);
                    char key = keys[selected_page][y][x];

                    // clear screen
                    if (last_page != selected_page || last_caps_lock != caps_lock) oled.fillRect(key_matrix_x + (x * key_w), key_matrix_y + (y * key_h), key_w, key_h, BLACK);

                    // draw character
                    if (0x21 <= key && key <= 0x7e) // if key is printable (excl. space because there is a space symbol)
                    {
                        char key2 = key;
                        // if caps lock is on, and the key is a letter, print the key as a capital
                        if (caps_lock && (0x61 <= key && key <= 0x7a)) key2 -= 32;
                        oled.setTextColor(WHITE, BLACK);
                        oled.drawString(
                            String(key2),
                            key_matrix_x + (x * key_w) + (key_w/2),
                            key_matrix_y + (y * key_h) + (key_h/2)
                        );
                    }
                    else if (key == 0) // blank
                    {
                        // do literally nothing
                    }
                    else // if key is a control character
                    {
                        std::string icon = "";
                        switch(key)
                        {
                            case 0x08: // backspace
                                icon = "key_backspace";
                                break;

                            case 0x0c: // clear
                                icon = "key_clear";
                                break;

                            case 0x04: // enter
                                icon = "key_tick";
                                break;

                            case 0x18: // cancel;
                                icon = "key_cancel";
                                break;

                            case 0x11: // left
                                icon = "key_move_left";
                                break;

                            case 0x12: // right
                                icon = "key_move_right";
                                break;

                            case 0x20: // space
                                icon = "key_space";
                                break;

                            case 0x13: // caps lock
                                icon = (caps_lock) ? "key_caps_on" : "key_caps_off";
                                break;

                            case 0x0e: // symbols
                                icon = "key_symbols";
                                break;

                            case 0x0f: // letters
                                icon = "key_letters";
                                break;
                        }
                        oled.drawBitmap(
                            key_matrix_x + (x * key_w),
                            key_matrix_y + (y * key_h),
                            small_icons[icon].data(),
                            key_w, key_h, WHITE
                        );
                    }
                    

                    // draw button outline
                    oled.drawRoundRect(
                        key_matrix_x + (x * key_w),
                        key_matrix_y + (y * key_h),
                        key_w, key_h, key_radius, (selected_key == index) ? themecolour : BLACK
                    );
                }
            }
            oled.setTextDatum(TL_DATUM);
            last_page = selected_page;
            last_caps_lock = caps_lock;
        };
        draw_keys();

        while(true)
        {
            startLoop();

            // print current input
            oled.setTextColor(WHITE, BLACK);
            oled.setCursor(input_field_x + input_field_padding, input_field_y + input_field_padding);
            if (mask) for (int i = 0; i < input.size(); i++) oled.print(mask);
            else oled.print(input.c_str());

            if (dpad_enter_active())
            {
                switch(keys[selected_page][selected_row][selected_col])
                {
                    case 0x04: // enter
                        finish_input = true;
                        break;

                    case 0x18: // cancel
                        input = "";
                        finish_input = true;
                        break;

                    case 0x0c: // clear input
                        input = "";
                        oled.fillRect(input_field_x, input_field_y, input_field_w, input_field_h, BLACK);
                        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);
                        break;

                    case 0x08: // backspace
                        oled.fillRect(input_field_x, input_field_y, input_field_w, input_field_h, BLACK);
                        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);
                        input = input.substr(0, input.size()-1);
                        break;

                    case 0x13: // caps lock
                        caps_lock = !caps_lock;
                        break;

                    case 0x11: // move left
                        break;

                    case 0x12: // move right
                        break;

                    case 0x0e: // symbols
                        selected_page = 1;
                        break;

                    case 0x0f: // letters
                        selected_page = 0;
                        break;

                    case 0: // blank
                        // do nothing
                        break;

                    default:
                        char key2 = keys[selected_page][selected_row][selected_col];
                        // if caps lock is on, and the key is a letter, print the key as a capital
                        if (caps_lock && (0x61 <= key2 && key2 <= 0x7a)) key2 -= 32;
                        input += key2;
                        break;
                }
            }

            if (dpad_left_active())
            {
                if (selected_col == 0) selected_col = no_cols - 1;
                else selected_col--;
            }

            if (dpad_right_active())
            {
                if (selected_col == no_cols - 1) selected_col = 0;
                else selected_col++;
            }

            if (dpad_up_active())
            {
                if (selected_row == 0) selected_row = no_rows - 1;
                else selected_row--;
            }

            if (dpad_down_active())
            {
                if (selected_row == no_rows - 1) selected_row = 0;
                else selected_row++;
            }

            if (dpad_any_active())
            {
                draw_keys();
            }

            if (finish_input) break;
            drawTopThing();
            endLoop();
        }

        forceRedraw = true;
        if (clear_screen) oled.fillScreen(0);
        return input;
    }

    uint8_t messageBox(const char* msg, std::vector<const char*> btns, bool clear_screen, uint16_t colour)
    {
        Serial.printf("showing message box: %s\n", msg);
        watch2::setFont(MAIN_FONT);

        // The Numbers
        uint16_t padding = 7;
        uint8_t box_radius = 15;
        uint8_t btn_radius = 7;
        uint8_t selected_button = 0;
        int16_t msg_x = 0, msg_y = 0;
        uint16_t msg_w = 0, msg_h = 0;
        getTextBounds(msg, 0, 0, &msg_x, &msg_y, &msg_w, &msg_h);

        uint16_t dialogue_width = 195;
        uint16_t dialogue_height = (padding * 5) + oled.fontHeight() + msg_h;

        uint16_t dialogue_x = (SCREEN_WIDTH  / 2) - (dialogue_width  / 2);
        uint16_t dialogue_y = (SCREEN_HEIGHT / 2) - (dialogue_height / 2);

        uint16_t text_x = dialogue_x + (dialogue_width / 2);
        uint16_t text_y = dialogue_y + padding;
        Serial.printf("x: %d\ny: %d", text_x, text_y);

        uint16_t btn_widths[btns.size()];
        uint16_t total_btn_width = 0;
        int16_t btn_x = 0, btn_y = 0;
        uint16_t btn_w = 0, btn_h = 0;

        // calculate the size of each button
        for (uint8_t i = 0; i < btns.size(); i++)
        {
            getTextBounds(btns[i], 0, 0, &btn_x, &btn_y, &btn_w, &btn_h);
            btn_widths[i] = btn_w + (padding * 2);
            total_btn_width += btn_widths[i];
        }

        // finish calculating the size of all the buttons together
        total_btn_width += padding * (btns.size() - 1);

        // calculate the button position
        btn_x = dialogue_x + ((dialogue_width / 2) - (total_btn_width / 2));
        btn_y = (padding * 2) + msg_h + dialogue_y;
        btn_w = 0;
        btn_h = (padding * 2) + oled.fontHeight();

        // get number of newlines in msg
        // uint16_t newlines = 0;
        // for(const char* str = msg; *str; ++str) newlines += (*str == '\n');

        // draw dialogue thingy
        oled.fillRoundRect(dialogue_x, dialogue_y, dialogue_width, dialogue_height, box_radius, BLACK);
        oled.drawRoundRect(dialogue_x, dialogue_y, dialogue_width, dialogue_height, box_radius, colour);

        // print message
        // this splits the message using '\n' as a delimiter, and prints each segment of the message on a new line
        oled.setTextColor(WHITE, BLACK);
        oled.setTextDatum(TC_DATUM);
        char* str = strdup(msg);
        char* pch = strtok(str, "\n");
        uint16_t newlines = 0;
        while(pch != NULL)
        {
            oled.drawString(pch, text_x, text_y + (newlines * oled.fontHeight()));
            pch = strtok(NULL, "\n");
            newlines++;
        }
        free(str);
        oled.setTextDatum(TL_DATUM);

        // draw the buttons
        uint16_t current_btn_x = btn_x;
        for (uint8_t i = 0; i < btns.size(); i++)
        {
            uint16_t btn_colour = (i == selected_button) ? WHITE : colour;
            oled.drawRoundRect(current_btn_x, btn_y, btn_widths[i], btn_h, btn_radius, btn_colour);
            oled.drawString(btns[i], current_btn_x + padding, btn_y + padding);
            current_btn_x += btn_widths[i] + padding;
        }

        while(1)
        {
            startLoop();

            if (dpad_right_active())
            {
                if (selected_button == btns.size() - 1) selected_button = 0;
                else selected_button++;
            }

            if (dpad_left_active())
            {
                if (selected_button == 0) selected_button = btns.size() - 1;
                else selected_button--;
            }

            if (dpad_any_active())
            {
                // draw the buttons
                uint16_t current_btn_x = btn_x;
                for (uint8_t i = 0; i < btns.size(); i++)
                {
                    uint16_t btn_colour = (i == selected_button) ? WHITE : colour;
                    oled.drawRoundRect(current_btn_x, btn_y, btn_widths[i], btn_h, btn_radius, btn_colour);
                    oled.drawString(btns[i], current_btn_x + padding, btn_y + padding);
                    current_btn_x += btn_widths[i] + padding;
                }
            }

            if (dpad_enter_active())
            {
                break;
            }

            endLoop();
        }

        forceRedraw = true;
        if (clear_screen) oled.fillScreen(0);
        return selected_button;
    }

    uint16_t popup_menu(const char *title, std::vector<std::string> items, bool scroll, uint16_t colour)
    {
        uint16_t selected_item = 0;
        uint16_t padding = 4;
        uint16_t item_height = (3 * padding) + oled.fontHeight();

        uint16_t dialogue_w = 150;
        uint16_t dialogue_h = (2 * padding) + oled.fontHeight() + (items.size() * item_height);
        uint16_t dialogue_x = (SCREEN_WIDTH / 2) - (dialogue_w / 2);
        uint16_t dialogue_y = (SCREEN_HEIGHT / 2) - (dialogue_h / 2);
        uint16_t dialogue_r = 10;

        oled.fillRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, BLACK);
        oled.drawRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, colour);

        oled.setTextDatum(TC_DATUM);
        oled.drawString(title, dialogue_x + (dialogue_w / 2), dialogue_y + padding);

        drawMenu(
            dialogue_x + padding,
            dialogue_y + padding + oled.fontHeight(),
            dialogue_w - (padding * 2), item_height * items.size(),
            items, selected_item, scroll, true, colour
        );

        while(1)
        {
            startLoop();

            if (dpad_up_active())
            {
                if (selected_item == 0) selected_item = items.size() - 1;
                else selected_item--;
                
            }

            if (dpad_down_active())
            {
                if (selected_item == items.size() - 1) selected_item = 0;
                else selected_item++;
            }

            if (dpad_any_active())
            {
                drawMenu(
                    dialogue_x + padding,
                    dialogue_y + padding + oled.fontHeight(),
                    dialogue_w - (padding * 2), item_height * items.size(),
                    items, selected_item, false, true, colour
                );
            }

            if (dpad_enter_active()) break;

            endLoop();
        }

        oled.fillScreen(BLACK);
        forceRedraw = true;
        return selected_item;
    }

    void controlCentreDialogue()
    {
        // if the control centre is already being shown, don't do anything
        if (showingControlCentre) return;

        int selected_widget = 3;
        int last_button_widget = 3;
        bool go_back_to_watch_face = false;
        uint8_t last_bt_state = watch2::bluetooth_state;
        uint8_t last_wifi_state = watch2::wifi_state;
        int last_minute = 0;
        bool init = false;

        uint8_t dialogue_radius = 7;
        uint16_t dialogue_width = SCREEN_WIDTH * 0.9;
        uint16_t dialogue_height = SCREEN_HEIGHT * 0.95;
        uint16_t dialogue_x = (SCREEN_WIDTH - dialogue_width) / 2;
        uint16_t dialogue_y = 0;

        // draw dialogue box
        oled.fillRoundRect(dialogue_x, -dialogue_radius, dialogue_width, dialogue_height, dialogue_radius, BLACK);
        oled.drawRoundRect(dialogue_x, -dialogue_radius, dialogue_width, dialogue_height, dialogue_radius, themecolour);

        showingControlCentre = true;

        uint16_t weather = 0;
        time_t sunrise = 0, sunset = 0;
        getCurrentWeather(weather, sunrise, sunset);

        while(1)
        {
            startLoop();
            //Serial.println("aaaaaaaaaaaaaaaaaaaaa");

            //------------------------
            // handle value updating
            //------------------------

            if (dpad_left_active())
            {
                if (selected_widget > 2)
                {
                    selected_widget--;
                    if (selected_widget < 3) selected_widget = 5;
                }
                else if (selected_widget == 0) // volume
                {
                    watch2::speaker_volume = std::max(watch2::speaker_volume - 1, 0);
                }
                else if (selected_widget == 1) // brightness
                {
                    watch2::screen_brightness = std::max(watch2::screen_brightness - 10, 0);
                    ledcWrite(TFTBL_PWM_CHANNEL, watch2::screen_brightness);
                    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                    watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                    watch2::preferences.end();
                }
                else if (selected_widget == 2) // torch
                {
                    watch2::torch_brightness = std::max(watch2::torch_brightness - 10, 0);
                    ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
                }
            }
            if (dpad_right_active())
            {
                if (selected_widget > 2)
                {
                    selected_widget++;
                    if (selected_widget > 5) selected_widget = 3;
                }
                else if (selected_widget == 0) // volume
                {
                    watch2::speaker_volume = std::min(watch2::speaker_volume + 1, 21);
                }
                else if (selected_widget == 1) // brightness
                {
                    watch2::screen_brightness = std::min(watch2::screen_brightness + 10, 255);
                    ledcWrite(TFTBL_PWM_CHANNEL, watch2::screen_brightness);
                    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                    watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                    watch2::preferences.end();
                }
                else if (selected_widget == 2) // torch
                {
                    watch2::torch_brightness = std::min(watch2::torch_brightness + 10, 255);
                    ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
                }
            }
            if (dpad_up_active())
            {
                if (selected_widget > 2)
                {
                    last_button_widget = selected_widget;
                    selected_widget = 2;
                }
                else
                {
                    selected_widget--;
                    if (selected_widget < 0) selected_widget = last_button_widget;
                }
            }
            if (dpad_down_active())
            {
                if (selected_widget > 2)
                {
                    //last_button_widget = selected_widget;
                    //selected_widget = 0;
                    go_back_to_watch_face = true;
                }
                else
                {
                    selected_widget++;
                    if (selected_widget > 2) selected_widget = last_button_widget;
                }
            }
            if (dpad_enter_active())
            {
                if (selected_widget == 2) // torch
                {
                    if (watch2::torch_brightness) watch2::torch_brightness = 0;
                    else watch2::torch_brightness = 255;
                    ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
                }
                if (selected_widget == 3) // wifi
                {
                    if (watch2::wifi_state == 0) watch2::enable_wifi();
                    else watch2::disable_wifi();
                }
                if (selected_widget == 4) // bluetooth
                {
                    if (watch2::bluetooth_state == 0) watch2::enable_bluetooth();
                    else watch2::disable_bluetooth();
                }
                if (selected_widget == 5) // ntp
                {
                    getTimeFromNTP();
                }
            }

            //------------------------
            // draw stuff
            //------------------------

            if(!init || dpad_any_active() || (watch2::wifi_state != last_wifi_state) || (watch2::bluetooth_state != last_bt_state) || (minute() != last_minute)) {
                
                setFont(SLIGHTLY_BIGGER_FONT);

                int spacing = 10;
                int button_size = 30;
                int weather_icon_size = 24;
                int radius = 10;
                int outline_colour = WHITE;
                int background_colour = BLACK;
                int time_y = dialogue_y + 2;
                int button_x = dialogue_x + spacing;
                int button_y = time_y + oled.fontHeight() + spacing;
                int slider_x = 0;

                // draw time
                oled.setCursor(button_x, time_y);
                oled.setTextColor(WHITE, BLACK);
                oled.fillRect(button_x, time_y, (dialogue_width / 2), oled.fontHeight(), BLACK);  // hacky
                oled.printf("%02d:%02d", hour(), minute());
                setFont(MAIN_FONT);
                last_minute = minute();

                // draw weather
                uint16_t weather_x = (dialogue_x + dialogue_width) - weather_icon_size - spacing;
                switch(weather / 100)
                {
                    case 2: // thunder
                        oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["thunder"].data());
                        break;
                        
                    case 3: // drizzle
                        if (now() < sunrise || now() > sunset) /* night */
                            oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["moon_rain"].data());
                        else                                   /* day */
                            oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["sun_rain"].data());
                        break;

                    case 5: // rain
                        oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["rain"].data());
                        break;

                    case 6: // snow
                        oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["snow"].data());
                        break;

                    case 7: // atmosphere
                        oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["wind"].data());
                        break;

                    case 8: // clear / clouds
                        if (weather == 800) // clear
                        {
                            if (now() < sunrise || now() > sunset) /* night */
                                oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["moon"].data());
                            else                                   /* day */
                                oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["sun"].data());
                        }
                        else
                        {
                            if (now() < sunrise || now() > sunset) /* night */
                                oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["moon_cloud"].data());
                            else                                   /* day */
                                oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["sun_cloud"].data());
                        }
                        break;

                    default:
                        oled.pushImage(weather_x, time_y, weather_icon_size, weather_icon_size, watch2::icons["weather_unknown"].data());
                        break;
                }

                //draw volume slider                                                                                                                    jungle green
                if (selected_widget == 0) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::speaker_volume * ( ( dialogue_width - button_size - spacing - 2 - button_x ) / 21.0 ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0x2D50);                               //background thing
                outline_colour = (selected_widget == 0) ? WHITE : 0x2D50;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["volume"].data(), button_size, button_size, WHITE);                     //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw brightness slider                                                                                                                yellow
                if (selected_widget == 1) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::screen_brightness * ( (float)( dialogue_width - button_size - spacing - 2 - button_x ) / ( 255.0 ) ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0xFEC0);                               //background thing
                outline_colour = (selected_widget == 1) ? WHITE : 0xFEC0;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["sun"].data(), button_size, button_size, WHITE);                        //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw torch slider                                                                                                                     adafruit yellow
                if (selected_widget == 2) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::torch_brightness * ( ( dialogue_width - button_size - spacing - 2 - button_x ) / 255.0 ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, YELLOW);                               //background thing
                outline_colour = (selected_widget == 2) ? WHITE : YELLOW;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["torch_but_smaller"].data(), button_size, button_size, WHITE);          //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw wifi button
                if (watch2::wifi_state == 3) background_colour = 0x867d; // enabled + connected
                if (watch2::wifi_state == 2) background_colour = 0x6E5A; // enabled + connecting
                if (watch2::wifi_state == 1) background_colour = 0x6E5A; // enabled + disconnected
                if (watch2::wifi_state == 0) background_colour = BLACK;  // disabled
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( !(selected_widget == 3) != !(watch2::wifi_state == 3) ) ? 0x867D : WHITE;
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["wifi"].data(), button_size, button_size, (watch2::wifi_state != 0) ? WHITE : 0x867D);
                button_x += spacing + button_size;
                if (watch2::wifi_state != last_wifi_state) last_wifi_state = watch2::wifi_state;

                //draw bluetooth button  bluetooth blue
                if (watch2::bluetooth_state == 3) background_colour = 0x041F; // enabled + connected
                if (watch2::bluetooth_state == 2) background_colour = 0x743E; // enabled + disconnected
                if (watch2::bluetooth_state == 1) background_colour = 0xA55C; // enabling
                if (watch2::bluetooth_state == 0) background_colour = BLACK;  // disabled
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( !(selected_widget == 4) != !(watch2::bluetooth_state == 3) ) ? 0x041F : WHITE;
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["bluetooth"].data(), button_size, button_size, (watch2::bluetooth_state != 0) ? WHITE : 0x041F);
                button_x += spacing + button_size;
                if (watch2::bluetooth_state != last_bt_state) last_bt_state = watch2::bluetooth_state;

                //draw NTP button
                background_colour = TFT_OLIVE;
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( (selected_widget == 5) ? WHITE : 0x041f);
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["internet_time"].data(), button_size, button_size, WHITE);
                button_x += spacing + button_size;

                // draw label
                uint16_t label_y = dialogue_y + (dialogue_height - oled.fontHeight() - 5);
                button_x = dialogue_x + spacing;;

                oled.setCursor(button_x, label_y);
                oled.setTextColor(WHITE, BLACK);
                oled.fillRect(button_x, label_y, dialogue_width - (button_x * 2), oled.fontHeight() - 5, BLACK);
                switch(selected_widget)
                {
                    case 0: // sound
                        oled.print("Volume");
                        break;

                    case 1: // backlight
                        oled.print("Brightness");
                        break;

                    case 2: // torch
                        oled.print("Torch");
                        break;

                    case 3: // wifi
                        switch(wifi_state)
                        {
                            case 0: // disabled
                                oled.print("Wifi Disabled");
                                break;

                            case 1: // enabled, disconnected
                                oled.print("Wifi Enabled");
                                break;

                            case 2: // connecting
                                oled.print("Connecting");
                                break;

                            case 3: // connected
                                oled.print(WiFi.SSID());
                                break;

                            case 4: // connect asap
                                oled.print("Connecting Soon");
                                break;
                        }
                        break;

                    case 4: // bluetooth
                        oled.print("Bluetooth");
                        break;

                    case 5: // ntp
                        oled.print("Internet Time");
                        break;
                }

                init = true;
            }

            if (dpad_down_active())
            {
                if (go_back_to_watch_face) break; //watch2::switchState(watch2::state, 0);
            }

            endLoop();

        }

        showingControlCentre = false;
        oled.fillScreen(BLACK);
        forceRedraw = true;
    }

    std::string getApiKey(const char *service, const char *field)
    {
        Serial.printf("[api keys] getting api key: %s:%s\n", service, field);
        std::string api_key = "";

        if (SPIFFS.exists(API_KEYS_FILENAME))
        {
            // open keys file
            fs::File api_keys_file = SPIFFS.open(API_KEYS_FILENAME);

            // parse json
            cJSON *api_keys_json = cJSON_Parse(api_keys_file.readString().c_str());

            cJSON *service_json = cJSON_GetObjectItem(api_keys_json, service);
            if (service_json)
            {
                cJSON *field_json = cJSON_GetObjectItem(service_json, field);
                if (field_json)
                {
                    const char *value = field_json->valuestring;
                    Serial.printf("[api keys] value: %s\n", value);
                    uint8_t pos = 0;
                    while(1)
                    {
                        api_key += value[pos];

                        pos++;
                        if (value[pos] == '\0') break;
                    }
                }
                else Serial.println("[api keys] field doesn't exist");
            }
            else Serial.println("[api keys] service doesn't exist");

            // free json memory
            cJSON_Delete(api_keys_json);
        }
        else
        {
            Serial.println("[api keys] file doesn't exist");
        }

        return api_key;
    }

    void getCurrentWeather(uint16_t &weather, time_t &sunrise, time_t &sunset)
    {
        weather = 0;
        sunrise = 0;
        sunset = 0;

        if (wifi_state != 3) // wifi not connected
        {
            Serial.println("[weather] not connected to wifi");
        }
        else // wifi is connected
        {
            Serial.println("[weather] getting current weather");

            HTTPClient *http = new HTTPClient();
            char server[150];
            std::string api_key = getApiKey("openweather");
            Serial.printf("[weather] key: %s\n", api_key.c_str());
            sprintf(server, "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s", weather_location.c_str(), api_key.c_str());
            Serial.printf("[weather] server request: %s\n", server);

            // connect to server
            Serial.println("[weather] connecting to server");
            http->begin(watch2::wifi_client, server);
            int http_code = http->GET();
            if (http_code)
            {
                Serial.println("[weather] connected to server");
                Serial.printf("[weather] http code: %d (%s)\n", http_code, http->errorToString(http_code));

                if (http_code > 0)
                {
                    if (http_code == HTTP_CODE_OK)
                    {
                        // get server response
                        String res = http->getString();
                        Serial.println("[weather] response:");
                        Serial.println(res);

                        // parse returned json
                        cJSON *weather_data = cJSON_Parse(res.c_str());
                        if (weather_data)
                        {
                            // get weather array
                            cJSON *weather_array = cJSON_GetObjectItem(weather_data, "weather");
                            if (weather_array)
                            {
                                // get the primary weather condition
                                cJSON *weather_status = cJSON_GetArrayItem(weather_array, 0);
                                if (weather_status)
                                {
                                    // get the weather code
                                    weather = cJSON_GetObjectItem(weather_status, "id")->valueint;
                                    Serial.printf("[weather] got primary weather code: %d\n", weather);
                                }
                                else
                                {
                                    Serial.println("[weather] failed to get primary weather condition");
                                }
                            }
                            else
                            {
                                Serial.println("[weather] failed to get weather array");
                            }

                            // get time info
                            cJSON *time_info = cJSON_GetObjectItem(weather_data, "sys");
                            if (time_info)
                            {
                                // get sunrise
                                sunrise = cJSON_GetObjectItem(time_info, "sunrise")->valueint;
                                Serial.printf("[weather] got sunrise time: %d\n", sunrise);

                                // get sunset
                                sunset = cJSON_GetObjectItem(time_info, "sunset")->valueint;
                                Serial.printf("[weather] got sunset time: %d\n", sunset);
                            }
                            else
                            {
                                Serial.println("[weather] failed to get time info");
                            }
                            
                        }
                        else
                        {
                            Serial.println("[weather] failed to parse response");
                        }
                        
                        // free memory used by parsed json
                        cJSON_Delete(weather_data);
                    }
                }
                else Serial.println("[weather] ???");
                
            }
            else
            {
                Serial.println("[weather] failed to connect");
            }

            delete http;
            Serial.println("[weather] finished");
        }
        
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
    void getHeatMapColor(float value, float *red, float *green, float *blue)
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

    void setFont(const char* font, TFT_eSPI &tft, fs::FS &ffs)
    {
        if (ffs.exists("/" + String(font) + ".vlw"))
        {
            tft.loadFont(String(font));
        }
        else 
        {
            Serial.println("[error] font " + String(font) + " doesn't exist");
            tft.setFreeFont(NULL);
        }
    }

    void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        std::string help(string);
        int newlines = std::count(help.begin(), help.end(), '\n') + 1;

        *x1 = x;
        *y1 = y;
        *w = oled.textWidth(string);
        *h = oled.fontHeight() * newlines;
    }

    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        std::string help(str.c_str());
        int newlines = std::count(help.begin(), help.end(), '\n') + 1;

        *x1 = x;
        *y1 = y;
        *w = oled.textWidth(str);
        *h = oled.fontHeight() * newlines;
    }

    // stolen from https://gist.github.com/dgoguerra/7194777
    const char *humanSize(uint64_t bytes)
    {
        char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
        char length = sizeof(suffix) / sizeof(suffix[0]);

        int i = 0;
        double dblBytes = bytes;

        if (bytes > 1024) {
            for (i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024)
                dblBytes = bytes / 1024.0;
        }

        static char output[200];
        sprintf(output, "%.02lf %s", dblBytes, suffix[i]);
        return output;
    }

    // Bodmers BMP image rendering function
    // stolen from https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino

    uint16_t read16(fs::File &f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
    }

    uint32_t read32(fs::File &f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
    }

    void drawBmp(const char *filename, int16_t x, int16_t y) {

    if ((x >= oled.width()) || (y >= oled.height())) return;

    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS)
    {
        Serial.print("File not found");
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t  r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42)
    {
        read32(bmpFS);
        read32(bmpFS);
        seekOffset = read32(bmpFS);
        read32(bmpFS);
        w = read32(bmpFS);
        h = read32(bmpFS);

        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
        {
        y += h - 1;

        bool oldSwapBytes = oled.getSwapBytes();
        oled.setSwapBytes(true);
        bmpFS.seek(seekOffset);

        uint16_t padding = (4 - ((w * 3) & 3)) & 3;
        uint8_t lineBuffer[w * 3 + padding];

        for (row = 0; row < h; row++) {
            
            bmpFS.read(lineBuffer, sizeof(lineBuffer));
            uint8_t*  bptr = lineBuffer;
            uint16_t* tptr = (uint16_t*)lineBuffer;
            // Convert 24 to 16 bit colours
            for (uint16_t col = 0; col < w; col++)
            {
            b = *bptr++;
            g = *bptr++;
            r = *bptr++;
            *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }

            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            oled.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
        }
        oled.setSwapBytes(oldSwapBytes);
        Serial.print("Loaded in "); Serial.print(millis() - startTime);
        Serial.println(" ms");
        }
        else Serial.println("BMP format not recognized.");
    }
    bmpFS.close();
    }


    imageData getImageData(const char *filename)
    {

        // this method currently uses stb_image for everything

        stbi_io_callbacks callbacks = {
            img_read,
            img_skip,
            img_eof
        };

        // open file
        File f = SD.open(filename);
        char buffer[255];
        f.getName(buffer, 255);
        Serial.println(buffer);

        // read image
        int img_w, img_h, img_n, x, y;
        unsigned char *data = stbi_load_from_callbacks(&callbacks, &f, &img_w, &img_h, &img_n, 3);

        // set up response struct
        const char *error;
        if (data == NULL) error = stbi_failure_reason();
        else error = "";
        imageData response = {
            data,
            img_w,
            img_h,
            error
        };

        // return data
        f.close();
        return response;
    }

    void freeImageData(unsigned char *data)
    {
        if (data) stbi_image_free(data);
    }

    const char* drawImage(imageData data, int16_t img_x, int16_t img_y, float scaling)
    {
        // numbers
        unsigned long pixels = data.width * data.height * 3;//sizeof(data) / sizeof(unsigned char);
        int x = img_x, y = img_y;

        if (data.data == NULL)
        {
            return stbi_failure_reason();
        }
        else
        {
            // scale image
            unsigned char *actual_data;
            uint16_t img_width = 0, img_height = 0;

            if (scaling != 1)
            {
                img_width = data.width/scaling;
                img_height = data.height/scaling;
                stbir_resize_uint8(
                    data.data, data.width, data.height, 0,
                    actual_data, img_width, img_height, 0, 3
                );
            }
            else
            {
                img_width = data.width;
                img_height = data.height;
                actual_data = data.data;
            }

            // write image data
            // for (int i = 0; i < pixels; i+=(3 * scaling))
            // {
            //     watch2::oled.drawPixel(x, y, watch2::oled.color565(data.data[i], data.data[i+1], data.data[i+2]));
            //     x++;
            //     if (x >= data.width + img_x)
            //     {
            //         //watch2::oled.drawPixel(x-1, y, BLACK);
            //         x = img_x;
            //         y++;
            //     }
            // }

            for (uint16_t y = 0; y < img_height; y+=1)
            {
                for (uint16_t x = 0; x < img_width; x+=1)
                {
                    uint32_t pixel = ( x + (img_width * y) ) * 3;
                    watch2::oled.drawPixel(img_x + x, img_y + y, watch2::oled.color565(actual_data[pixel], actual_data[pixel+1], actual_data[pixel+2]));
                }
            }

            // if (actual_data) 
            // {
            //     Serial.println("[drawImage] freeing scaled image data");
            //     stbi_image_free(actual_data);
            // }

            return NULL;

        }
    }


    void enable_wifi(bool connect)
    {
        Serial.println("[Wifi] enabling wifi");
        wifi_state = 1; // enabled, disconnected

        // tell the system to enable wifi on boot
        watch2::preferences.begin("watch2", false);
        watch2::preferences.putBool("wifi_en", true);
        watch2::preferences.end();

        // connect to an access point
        if (connect)
        {
            watch2::wifi_state = 4;
            watch2::initial_wifi_reconnect_attempts = 3;
            watch2::wifi_reconnect_attempts = watch2::initial_wifi_reconnect_attempts;
        }
    }

    void disable_wifi()
    {
        Serial.println("[Wifi] disconnecting from wifi");
        WiFi.disconnect();
        //WiFi.mode(WIFI_OFF);

        // tell the system to disable wifi on boot
        watch2::preferences.begin("watch2", false);
        watch2::preferences.putBool("wifi_en", false);
        watch2::preferences.end();
        wifi_state = 0; // disabled
    }

    void connectToWifiAP(const char *ssid, const char *password)
    {
        Serial.println("[WiFi] connecting to AP");
        WiFi.enableSTA(true);
        //WiFi.mode(WIFI_STA);
        WiFi.setSleep(true);
        WiFi.setHostname("watch ii");

        if (strcmp(ssid, "") == 0)
        {
            // automatically connect to internet
            if (wifi_reconnect_attempts == 0)
            {
                WiFi.disconnect();
                wifi_state = 1;
            }
            else
            {
                // get most recent profile
                uint8_t profile_index = initial_wifi_reconnect_attempts - wifi_reconnect_attempts;
                cJSON *profiles = getWifiProfiles();
                
                if (profiles)
                {
                    Serial.println("[Wifi] got profiles object");
                    cJSON *profile_array = cJSON_GetObjectItem(profiles, "profiles");
                    cJSON *access_index = cJSON_GetObjectItem(profiles, "access_index");

                    // if there are profiles
                    if (cJSON_GetArraySize(access_index) > 0)
                    {
                        if (access_index)
                        {
                            Serial.printf("[Wifi] found %d profiles\n", cJSON_GetArraySize(profile_array));
                            Serial.printf("[Wifi] found %d profiles in access index\n", cJSON_GetArraySize(access_index));

                            // if the profile index refers to a profile that doesn't exist
                            // (if the profile index is greater than the number of elements in the access index - 1)
                            if (profile_index > cJSON_GetArraySize(access_index) - 1)
                            {
                                // the profile list has been exhausted
                                Serial.println("[Wifi] tried to access a profile that doesn't exist");
                                WiFi.disconnect();
                                wifi_state = 1;
                                wifi_reconnect_attempts = 0;
                            }
                            else
                            {
                                // the profile index refers to an access index element that does exist, so get the information for that profile
                                Serial.printf("[Wifi] access index %d points to an existing profile\n", profile_index);
                                const char *ssid = cJSON_GetArrayItem(access_index, profile_index)->valuestring;
                                cJSON *profile;
                                bool help = false;
                                for (int i = 0; i < cJSON_GetArraySize(profile_array); i++)
                                {
                                    profile = cJSON_GetArrayItem(profile_array, i);
                                    const char* profile_ssid = cJSON_GetObjectItem(profile, "ssid")->valuestring;
                                    Serial.printf("checking ssid %s; profile ssid %s\n", ssid, profile_ssid);
                                    if (strcmp(ssid, profile_ssid) == 0) // if profile ssid matches ap ssid
                                    {
                                        help = true;
                                        break;
                                    }
                                }

                                if (help) // the profile actually exists
                                {
                                    Serial.println("[Wifi] the profile's SSID matches the access index's SSID :), connecting...");
                                    //WiFi._setStatus(WL_DISCONNECTED);
                                    WiFi.begin(
                                        cJSON_GetObjectItem(profile, "ssid")->valuestring,
                                        cJSON_GetObjectItem(profile, "password")->valuestring
                                    );
                                    wifi_connect_timeout_start = millis();
                                }
                                // otherwise, the AP name exists in the access index, but doesn't actually have a profile, so skip to the next AP
                                else 
                                {
                                    Serial.println("[Wifi] ssid was found in access index, but no matching profile was found");
                                    wifi_reconnect_attempts--;
                                }

                                wifi_state = 2;
                            }

                        }
                        else
                        {
                            Serial.println("[Wifi] couldn't access access index");
                            wifi_reconnect_attempts = 0;
                        }
                    }
                    else 
                    {
                        Serial.println("[Wifi] no profiles");
                        wifi_reconnect_attempts = 0;
                        WiFi.disconnect();
                        WiFi._setStatus(WL_CONNECT_FAILED);
                    }
                    
                    cJSON_Delete(profiles);

                }
                else
                {
                    Serial.println("[Wifi] couldn't access profile list");
                    wifi_reconnect_attempts = 0;
                }
            }
            
        }
        else
        {
            WiFi.begin(ssid, password);
            WiFi._setStatus(WL_DISCONNECTED);
            wifi_state = 2; // enabled, connecting
            wifi_connect_timeout_start = millis();
        }
        
        // the system will check if the wifi has connected to an AP in the endLoop() method.
    }

    void getTimeFromNTP()
    {
        Serial.println("[NTP] setting time using ntp");
        configTime(watch2::timezone * 60 * 60, 0, NTP_SERVER);

        struct tm timeinfo;
        getLocalTime(&timeinfo);
        Serial.println(&timeinfo, "[NTP] retrieved time: %A, %B %d %Y %H:%M:%S");
        setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
    }

    cJSON *getWifiProfiles()
    {
        Serial.println("[Wifi profiles] getting profiles");
        if (spiffs_state == 1) // if spiffs is initalised
        {
            cJSON *profiles;

            // if profile file already exists
            if (SPIFFS.exists(WIFI_PROFILES_FILENAME))
            {
                Serial.println("[Wifi profiles] profile exists");

                // get handle to profiles file
                fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME);

                // get profiles list
                String profiles_list = profiles_file.readString();

                // parse profiles
                profiles = cJSON_Parse(profiles_list.c_str());

                if (profiles)
                {
                    Serial.println("[Wifi profiles] valid profile");
                }
                else
                {
                    Serial.println("[Wifi profiles] invalid profile list, returning minimal profile object");

                    // create profiles object
                    profiles = cJSON_CreateObject();
                    cJSON *profile_array = cJSON_AddArrayToObject(profiles, "profiles");
                    cJSON *access_index = cJSON_AddArrayToObject(profiles, "access_index");
                }

                // Serial.println("help");
                // Serial.println(profiles_list);
                // Serial.println(cJSON_Print(profiles));

                // close file
                profiles_file.close();

                // return profiles
                return profiles;
            }
            //otherwise
            else
            {
                Serial.println("[Wifi profiles] profile doesn't exist, creating");

                // create profiles object
                profiles = cJSON_CreateObject();
                cJSON *profile_array = cJSON_AddArrayToObject(profiles, "profiles");
                cJSON *access_index = cJSON_AddArrayToObject(profiles, "access_index");

                // get handle to profiles file (it doesn't exist, so create it)
                fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME, "w");

                // write profiles object to file
                profiles_file.print(cJSON_Print(profiles));

                // close file
                profiles_file.close();

                // return profiles
                return profiles;
            }
        }
        else 
        {
            Serial.printf("[Wifi profiles] can't access spiffs (state: %d)\n", spiffs_state);
            return nullptr;
        }
    }

    void setWifiProfiles(cJSON *profiles)
    {
        Serial.println("[Wifi profiles] setting profiles");
        if (spiffs_state == 1) // if spiffs has been initalised
        {
            // open profiles file.  if the file doesn't exist, it will be created, and if it does exist,
            // it will be truncated to zero bytes.
            fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME, "w");

            // get profile list as a string
            const char *profile_string = cJSON_Print(profiles);

            // write the profile object to the file
            profiles_file.write((uint8_t*)profile_string, strlen(profile_string));
            
            // close file
            profiles_file.close();
        }
    }

    void enable_bluetooth()
    {
        Serial.println("[Bluetooth] enabling bluetooth");
        bluetooth_state = 1; // enabling

        // enable ble keyboard
        // Serial.println("[Bluetooth] starting BLE keyboard");
        //ble_keyboard.begin();

        Serial.println("[BLE] ble device init");
        BLEDevice::init("watch2");

        if (ble_set_up)
        {
            Serial.println("[BLE] ble already set up, restarting bt");
            btStart();
        }
        else
        {
            Serial.println("[BLE] create server");
            BLEServer *pServer = BLEDevice::createServer();

            Serial.println("[BLE] set up services");
            BLEService *pService = pServer->createService("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
            BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                                    "beb5483e-36e1-4688-b7f5-ea07361b26a8",
                                                    BLECharacteristic::PROPERTY_READ |
                                                    BLECharacteristic::PROPERTY_WRITE
                                                );

            pCharacteristic->setValue("Hello World says Neil");
            pService->start();

            Serial.println("[BLE] set up advertising");
            // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
            BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
            pAdvertising->addServiceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
            pAdvertising->setScanResponse(true);
            pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
            pAdvertising->setMinPreferred(0x12);
            ble_set_up = true;
        }

        Serial.println("[BLE] starting advertising");
        BLEDevice::startAdvertising();
        

        Serial.println("[Bluetooth] finished enabling bluetooth");
        bluetooth_state = 2;
    }

    void disable_bluetooth()
    {
        Serial.println("[Bluetooth] disabling bluetooth");

        // disable ble keyboard
        Serial.println("[Bluetooth] ending BLE keyboard");
        //ble_keyboard.end();

        Serial.println("[BLE] stopping advertising");
        BLEDevice::stopAdvertising();

        Serial.println("[BLE] de-init ble device");
        BLEDevice::deinit(false);

        Serial.println("[Bluetooth] finished disabling bluetooth");
        bluetooth_state = 0;
    }


    // These read 16- and 32-bit types from the SD card file.
    // BMP data is stored little-endian, Arduino is little-endian too.
    // May need to reverse subscript order if porting elsewhere.

    //functions for stb_image
    int img_read(void *user,  char *data, int size)
    {
        File *f = static_cast<File*>(user);
        int bytes_read = f->read(data, size);
        return bytes_read;
    }

    void img_skip(void *user, int n)
    {
        File *f = static_cast<File*>(user);
        f->seekCur(n);
    }

    int img_eof(void *user)
    {
        File *f = static_cast<File*>(user);
        uint32_t help = f->available();
        if (help == 0) return 1;
        return 0;
    }

}

////////////////////////////////////////
// include state file things and icon files
////////////////////////////////////////
//#include "../states/states.h"