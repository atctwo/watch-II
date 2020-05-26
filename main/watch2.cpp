#include "watch2.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"



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
    WiFiClientSecure wifi_client;

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
    wifi_auth_mode_t wifi_encryption = WIFI_AUTH_MAX;
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
        //wip screenshot tool
        //this doesn't work yet
        if (btn_zero.pressedFor(1000))
        {
            oled.drawPixel(0,0,BLUE);
            
            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                for (int x = 0; x < SCREEN_WIDTH; x++)
                {
                    uint16_t colour[1];
                    oled.readRect(x, y, 1, 1, colour);
                    Serial.printf("%d ", colour[0]);
                    //Serial.printf("%3d %3d\n", x, y);
                }
                Serial.println();
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
                batteryVoltage = ReadVoltage(BATTERY_DIVIDER_PIN) * BATTERY_VOLTAGE_SCALE;
                batteryPercentage = ( batteryVoltage / BATTERY_VOLTAGE_MAX ) * 100.0;
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

        //turn off display
        //oled.sendCommand(0xAE);

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
        //oled.sendCommand(0xAF);

        //set up buttons
        btn_dpad_up.begin();
        btn_dpad_down.begin();
        btn_dpad_left.begin();
        btn_dpad_right.begin();
        btn_dpad_enter.begin();

        //init SD card
        initSD();

        // reconnect to wifi
        if (wifi_wakeup_reconnect)
        {
            enable_wifi();
        }

        // time??? what is it really?
        if (watch2::ntp_wakeup_connect) watch2::ntp_boot_connected = false;

        //rtc_gpio_deinit(GPIO_NUM_26);
        if (next_alarm_time > -1) switchState(0, 0, 0, 0, true);
        else switchState(0);
    }

    void drawMenu(int x, int y, int width, int height, std::vector<std::string> items, int selected, bool scroll, int colour)
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
                Serial.printf("offset: %d\n", y_offset);
                Serial.printf("items:  %d\n", onscreen_items);
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
                oled.setCursor(x + padding, y + padding - y_offset);
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
            if (!file_select_dir_list_init || dpad_any_active())
            drawMenu(2, top_thing_height, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 12, files2, selected_icon, themecolour);

            //finish file select list initilisation
            if (!file_select_dir_list_init) file_select_dir_list_init = true;

            drawTopThing();
            endLoop();

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

    const char* drawImage(imageData data, int16_t img_x, int16_t img_y)
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
            // write image data
            for (int i = 0; i < pixels; i+=3)
            {
                watch2::oled.drawPixel(x, y, watch2::oled.color565(data.data[i], data.data[i+1], data.data[i+2]));
                x++;
                if (x >= data.width + img_x)
                {
                    //watch2::oled.drawPixel(x-1, y, BLACK);
                    x = img_x;
                    y++;
                }
            }

            stbi_image_free(data.data);
            data.data = NULL;

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
                    cJSON *profile_array = cJSON_GetObjectItem(profiles, "profiles");
                    cJSON *access_index = cJSON_GetObjectItem(profiles, "access_index");

                    // if there are profiles
                    if (cJSON_GetArraySize(access_index) > 0)
                    {

                        if (access_index)
                        {

                            // if the profile index refers to a profile that doesn't exist
                            // (if the profile index is greater than the number of elements in the access index - 1)
                            if (profile_index > cJSON_GetArraySize(access_index) - 1)
                            {
                                // the profile list has been exhausted
                                WiFi.disconnect();
                                wifi_state = 1;
                                wifi_reconnect_attempts = 0;
                            }
                            else
                            {
                                // the profile index refers to an access index element that does exist, so get the information for that profile
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
                                    Serial.println("thingy profile exists");
                                    WiFi.begin(
                                        cJSON_GetObjectItem(profile, "ssid")->valuestring,
                                        cJSON_GetObjectItem(profile, "password")->valuestring
                                    );
                                    WiFi._setStatus(WL_DISCONNECTED);
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
        Serial.println("setting time using ntp");
        Serial.println(watch2::wifi_state);
        configTime(watch2::timezone * 60 * 60, 0, NTP_SERVER);
        // struct timeval timeinfo;
        // getLocalTime(&timeinfo);
        // Serial.println(&timeinfo, "retrieved time: %A, %B %d %Y %H:%M:%S");
        // setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year + 1900);
    }

    cJSON *getWifiProfiles()
    {
        Serial.println("[Wifi] getting profiles");
        if (spiffs_state == 1) // if spiffs in initalised
        {
            cJSON *profiles;

            // if profile file already exists
            if (SPIFFS.exists(WIFI_PROFILES_FILENAME))
            {
                // get handle to profiles file
                fs::File profiles_file = SPIFFS.open(WIFI_PROFILES_FILENAME);

                // get profiles list
                String profiles_list = profiles_file.readString();

                // parse profiles
                profiles = cJSON_Parse(profiles_list.c_str());

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
        else return nullptr;
    }

    void setWifiProfiles(cJSON *profiles)
    {
        Serial.println("[Wifi] setting profiles");
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