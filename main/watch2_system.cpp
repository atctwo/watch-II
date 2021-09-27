/**
 * @file watch2_system.cpp
 * @author atctwo
 * @brief main core system functions that the entire system depends on
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"
#include "esp_wifi.h"

namespace watch2 {

    // system
    EXT_RAM_ATTR int state = 0;
    EXT_RAM_ATTR int state_init = 0;
    EXT_RAM_ATTR int last_variant = 0;
    RTC_DATA_ATTR int selected_menu_icon;
    RTC_DATA_ATTR int boot_count = 0;
    std::string wfs = "\x0\x0\x0\x0";

    EXT_RAM_ATTR bool dpad_lock[5] = {false};
    EXT_RAM_ATTR bool dpad_pressed[5] = {false};
    EXT_RAM_ATTR bool dpad_held[5] = {false};
    EXT_RAM_ATTR uint32_t dpad_pressed_time[5] = {0};
    EXT_RAM_ATTR uint32_t dpad_last_repeat[5] = {0};

    int short_timeout = 5000;
    int long_timeout = 30000;
    bool timeout = true;
    uint32_t last_button_press = 0;

    EXT_RAM_ATTR std::vector<timerData> timers;
    EXT_RAM_ATTR std::vector<alarmData> alarms;
    int timer_trigger_status = 0;
    int timer_trigger_id = 255;
    int alarm_trigger_status = 0;
    int alarm_trigger_id = 0;

    time_t alarm_snooze_time = 5*60;
    uint16_t screen_brightness = 2^tftbl_resolution;
    uint8_t torch_brightness = 0;
    String timer_music = "";
    String alarm_music = "";

    int RTC_DATA_ATTR stopwatch_timing = 0;
    uint32_t RTC_DATA_ATTR stopwatch_epoch = 0;
    uint32_t RTC_DATA_ATTR stopwatch_paused_diff = 0;
    uint32_t stopwatch_time_diff = 0;
    uint32_t stopwatch_last_time_diff = 0;
    uint32_t stopwatch_ms = 0, stopwatch_last_ms = 0;
    uint32_t stopwatch_s = 0, stopwatch_last_s = 0;
    uint32_t stopwatch_min = 0, stopwatch_last_min = 0;
    uint32_t stopwatch_hour = 0, stopwatch_last_hour = 0;

    EXT_RAM_ATTR SPIClass *vspi = new SPIClass(VSPI);
    EXT_RAM_ATTR Preferences preferences;
    EXT_RAM_ATTR Adafruit_MCP23008 mcp;
    EXT_RAM_ATTR Adafruit_MCP9808 temperature;
    EXT_RAM_ATTR Adafruit_LC709203F fuel_gauge;

    //button objects
    EXT_RAM_ATTR Button btn_dpad_up(dpad_up, 25, false, false);
    EXT_RAM_ATTR Button btn_dpad_down(dpad_down, 25, false, false);
    EXT_RAM_ATTR Button btn_dpad_left(dpad_left, 25, false, false);
    EXT_RAM_ATTR Button btn_dpad_right(dpad_right, 25, false, false);
    EXT_RAM_ATTR Button btn_dpad_enter(dpad_enter, 25, false, false);
    EXT_RAM_ATTR Button btn_zero(0);

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
        // ESP_LOGD(WATCH2_TAG, "internal RAM: %2.4f%%", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);
        // ESP_LOGD(WATCH2_TAG, "external RAM: %2.4f%%", ((float)(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize()) * 100);

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
        //             ESP_LOGD(WATCH2_TAG, "%d ", colour[0]);
        //             //ESP_LOGD(WATCH2_TAG, "%3d %3d", x, y);
        //         }
        //         ESP_LOGD(WATCH2_TAG, "%*", );
        //     }
            
        // }

        // redraw stuff
        // if (forceRedraw)
        // {
        //     ESP_LOGD(WATCH2_TAG, "[endLoop] forceRedraw loop check: ");
        //     if (forceRedrawLooped)
        //     {
        //         ESP_LOGD(WATCH2_TAG, "finished");
        //         forceRedraw = false;
        //         forceRedrawLooped = false;
        //     }
        //     else
        //     {
        //         ESP_LOGD(WATCH2_TAG, "looped once");
        //         forceRedrawLooped = true;
        //     }
        // }
        if (forceRedraw) ESP_LOGD(WATCH2_TAG, "[endLoop] force redraw");

        if (btn_zero.wasPressed())
        {
            watch2::controlCentreDialogue();
        }

        //for (uint i = 0; i < 8; i++) ESP_LOGD(WATCH2_TAG, "btn %d: %d;\t", i, mcp.digitalRead(i));
        //ESP_LOGD(WATCH2_TAG, "");
        //ESP_LOGD(WATCH2_TAG, "dpad right: %d;", mcp.digitalRead(dpad_right));

        for (uint8_t i = 0; i < 5; i++)
        {
            // reset button active state
            if (dpad_pressed[i]) {
                //ESP_LOGD(WATCH2_TAG, "[button locks] set button %d to not pressed and locked", i);
                dpad_pressed[i] = false;
                dpad_lock[i] = true;
            }

            // reset button held state
            if (!mcp.digitalRead(i))
            {
                dpad_held[i] = false;
            }

            // save press time
            if (!dpad_lock[i] && mcp.digitalRead(i) && !dpad_pressed[i])
            {
                dpad_pressed_time[i] = millis();
            }

            // set button active state
            if (!dpad_lock[i] && mcp.digitalRead(i)) {
                //ESP_LOGD(WATCH2_TAG, "[button locks] set button %d to pressed", i);
                dpad_pressed[i] = true;
                dpad_held[i] = true;
            }

            //reset button lock state
            if (dpad_lock[i] && !mcp.digitalRead(i)) 
            {
                //ESP_LOGD(WATCH2_TAG, "[button locks] set button %d to unlocked", i);
                dpad_lock[i] = false;
            }
        }

        //handle timeouts
        if (dpad_any_active()) last_button_press = millis();
        if (timeout && (state != 0))
        {
            if (state == 1)
            {
                if (millis() - last_button_press > short_timeout) deepSleep(31);
            }
            else
            {
                if (millis() - last_button_press > long_timeout) deepSleep(31);
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
                ESP_LOGW(WATCH2_TAG, "[Wifi] connection timed out");
                WiFi._setStatus(WL_CONNECT_FAILED);
                ESP_LOGD(WATCH2_TAG, "%lu", millis());
            }

            if (WiFi.status() == WL_CONNECTED)
            {

                // update wifi profiles list
                cJSON *profiles = getWifiProfiles();
                if (profiles)
                {
                    // ESP_LOGD(WATCH2_TAG, "profile list before adding new profile");
                    // ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profiles));

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

                        // ESP_LOGD(WATCH2_TAG, "new profile");
                        // ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profile));

                        // add profile to profile list
                        cJSON_AddItemToArray(profile_array, profile);
                    }

                    // ESP_LOGD(WATCH2_TAG, "profile list after adding new profile");
                    // ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profiles));

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
                    else ESP_LOGW(WATCH2_TAG, "could not access access index");

                    // ESP_LOGD(WATCH2_TAG, "profile list after updating access index");
                    // ESP_LOGD(WATCH2_TAG, "%*", cJSON_Print(profiles));

                    // write update profile list to file
                    setWifiProfiles(profiles);

                    // clear up memory
                    ESP_LOGD(WATCH2_TAG, "[Wifi] freeing memory for profiles");
                    cJSON_Delete(profiles);
                }
                else ESP_LOGW(WATCH2_TAG, "[Wifi] couldn't access profile list");

                // set wifi state
                wifi_state = 3;
                ESP_LOGI(WATCH2_TAG, "[Wifi] connected to %s", WiFi.SSID().c_str());
            }
            
            if (WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_NO_SSID_AVAIL)
            {
                if (wifi_reconnect_attempts == 0) // out of reconnect attempts
                {
                    WiFi.disconnect();
                    wifi_state = 1;
                    ESP_LOGW(WATCH2_TAG, "[WiFi] could not connect to AP (out of attempts)");
                }
                else
                {
                    wifi_reconnect_attempts--;
                    ESP_LOGW(WATCH2_TAG, "[Wifi] failed to connect to AP, %d attempts remaining", wifi_reconnect_attempts);
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
                ESP_LOGI(WATCH2_TAG, "[WiFi] disconnected from AP");
            }
        }

        // bluetooth
        if (bluetooth_state == 2 || bluetooth_state == 3)
        {
            // if (ble_keyboard.isConnected())
            // {
            //     //ESP_LOGD(WATCH2_TAG, "[Bluetooth] connected");
            //     bluetooth_state = 3;
            // }
            // else
            // {
            //     //ESP_LOGD(WATCH2_TAG, "[Bluetooth] disconnected");
            //     bluetooth_state = 2;
            // }
        }

        // if the current state uses a framebuffer, draw it to the tft
        if (states[state].framebuffer) framebuffer.pushSprite(0, 0);
    }

    void switchState(int newState, int variant, int dim_pause_thing, int bright_pause_thing, bool dont_draw_first_frame)
    {
        ESP_LOGD(WATCH2_TAG, "switching to new state: %d (%s)", newState, watch2::states[newState].stateName.c_str());

        if (dim_pause_thing > 0)
        dimScreen(0, dim_pause_thing);              // dim the screen
        oled.fillScreen(BLACK);                     // clear screen

        // lock dpad
        for (uint16_t i = 0; i < 5; i++) 
        {
            //ESP_LOGD(WATCH2_TAG, "[button locks] set button %d to locked and not pressed", i);
            dpad_lock[i] = true;
            dpad_pressed[i] = false;
            dpad_held[i] = false;
        }
        last_button_press = millis();

        state_init = 0;                             // reset first execution flag
        state = newState;                           // switch state variable
        last_variant = states[state].variant;       // save last variant
        states[state].variant = variant;            // set state variant

        if (!dont_draw_first_frame)
        states[state].stateFunc();                  // run the state function once

        state_init = 1;                             // set first execution flag
        if (bright_pause_thing > 0)
        dimScreen(1, bright_pause_thing);           // make the screen brighter

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

        // shutdown temperature sensor
        temperature.shutdown();

        //deep sleep setup
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 1); //1 = High, 0 = Low

        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);

        //determine when to wake up (if a timer or alarm is set)
        time_t next_alarm_time = -1;

        for (int i = 0; i < timers.size(); i++)
        {
            if (timers[i].alarm_id != 255)
            {
                time_t time_left = ( timers[i].time_started + Alarm.read(timers[i].alarm_id ) ) - now();
                ESP_LOGD(WATCH2_TAG, "[sleep] timer %d: %d seconds left", i, time_left);
                if (time_left < next_alarm_time || next_alarm_time == -1) 
                {
                    next_alarm_time = time_left;
                    ESP_LOGD(WATCH2_TAG, "\tset next alarm");
                }
            }
        }
        for (int i = 0; i < alarms.size(); i++)
        {
            tmElements_t current_unix_time_without_the_time = {
                0, 0, 0, weekday(), day(), month(), year() - 1970
            };
            if (alarms[i].paused == false &&                                                                    // alarm isn't paused
                ( ( now() - makeTime(current_unix_time_without_the_time) ) < Alarm.read(alarms[i].alarm_id) )   // alarm's H:M:S is later than the current H:M:S
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
                ESP_LOGD(WATCH2_TAG, "[sleep] alarm %d: %d seconds left", i, time_left);
                if (time_left < next_alarm_time || next_alarm_time == -1) 
                {
                    next_alarm_time = time_left;
                    ESP_LOGD(WATCH2_TAG, "\tset next alarm");
                }
            }
        }

        ESP_LOGD(WATCH2_TAG, "[sleep] sleep wakeup timer: %d", next_alarm_time);

        //if an timer or an alarm has been set, set the device to wake up just before the alarm triggers
        if (next_alarm_time > -1)
        {
            esp_sleep_enable_timer_wakeup((((uint64_t)next_alarm_time) * 1000 * 1000) - 500);
            ESP_LOGD(WATCH2_TAG, "[sleep] timer in us (d): %d", next_alarm_time * 1000 * 1000 - 500);
            ESP_LOGD(WATCH2_TAG, "[sleep] timer in us (u): %u", next_alarm_time * 1000 * 1000 - 500);
        }
        else //if no alarm or timer has been set, then disable the timer wake up source
        {
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
        }

        // stop wifi
        WiFi.disconnect();
        esp_wifi_disconnect();
        esp_wifi_stop();
        wifi_state = 1;
        delay(1000);

        //begin sleep
        ESP_LOGI(WATCH2_TAG, "[sleep] entering sleep mode");
        Serial.flush();
        esp_light_sleep_start();



        //when the esp is woken up, it will resume execution at this point



        ESP_LOGI(WATCH2_TAG, "awake");
        oled.writecommand(0x11); // wake up screen

        //set up buttons
        btn_dpad_up.begin();
        btn_dpad_down.begin();
        btn_dpad_left.begin();
        btn_dpad_right.begin();
        btn_dpad_enter.begin();

        //init SD card
        initSD();
        delay(50);

        // get time from RTC
        uint8_t status;
        struct ds1337_time_t time_thing;

        if (!ds1337_get_status(&status))
        {

            if (status & DS1337_OSC_STOP_FLAG) {
                // Oscillator has stopped; time is invalid
                ESP_LOGW(WATCH2_TAG, "DS1337 oscillator stopped; time is invalid");
                ds1337_clear_status();
            } else {

                if (ds1337_read_time(&time_thing)) ESP_LOGW(WATCH2_TAG, "unable to read time from DS1337");
                else {

                    ESP_LOGI(WATCH2_TAG, "setting time from DS1337: %02d:%02d:%02d %02d.%02d.%04d", time_thing.hour, time_thing.minute, time_thing.second, time_thing.day, time_thing.month, time_thing.year);
                    setTime(time_thing.hour, time_thing.minute, time_thing.second, time_thing.day, time_thing.month, time_thing.year + 2000);

                }

            }

        }

        // reconnect to wifi
        if (wifi_state != 0 && wifi_wakeup_reconnect)
        {
            enable_wifi();
        }

        // set bluetooth state
        ESP_LOGD(WATCH2_TAG, "%d", bluetooth_state);
        if (bluetooth_state == 3) bluetooth_state = 2;

        // time??? what is it really?
        if (watch2::ntp_wakeup_connect) watch2::ntp_boot_connected = false;

        // wakeup temperature sensor
        temperature.wake();

        //rtc_gpio_deinit(GPIO_NUM_26);
        switchState(0, 0, 0, 0, true);
    }






    bool dpad_active(int key) {
    
        if (millis() - watch2::dpad_pressed_time[key] < KEY_REPEAT_DELAY) return (!watch2::dpad_lock[key] && watch2::dpad_pressed[key]);
        else
        {
            if (dpad_held[key]) 
            {
                // if (millis() - dpad_last_repeat[key] > KEY_REPEAT_PERIOD)
                // {
                //     dpad_last_repeat[key] = millis();
                //     return true;
                // }
                return true;
            }
            
        }

        return false;

    }

    

}