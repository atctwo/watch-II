#include "../src/watch2.h"

void state_func_settings()
{
    static std::vector<std::string> panels = {"Time and Date", "Timeout", "Colour", "WiFi", "About"};
    static int selected_panel = 0;
    static int last_selected_time = 0;

    static int temp_time[6] = {0};
    static int selected_time = 0;
    static int time_limits[6] = {23, 59, 59, 30, 12, 2106};
    const  int time_lower[6]  = {0,  0,  0,  1,  1,  1970};
    static int days_in_each_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    static cJSON *profiles;
    static cJSON *profile_array;
    static cJSON *profile_list;
    static uint16_t edit_profile_index = 0;

    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static char text_aaaa[6];
    static int time_element_padding = 1;
    static int radius = 4;

    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode

    switch (watch2::states[watch2::state].variant)
    {
        default:
        case 0: //menu

            if (dpad_down_active())
            {
                // increment selection
                selected_panel++;
                if (selected_panel >= panels.size()) selected_panel = 0;
            }
            if (dpad_up_active())
            {
                // decrement selection
                selected_panel--;
                if (selected_panel < 0) selected_panel = panels.size() - 1;
            }
            if (dpad_any_active() || !watch2::state_init)
            {
                // draw menu
                //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
                watch2::drawMenu(2, watch2::top_thing_height, SCREEN_WIDTH-4, SCREEN_HEIGHT-12, panels, selected_panel, watch2::themecolour);
            }
            if (dpad_enter_active())
            {
                // go to panel
                watch2::switchState(3, selected_panel + 1);
            }
            if (dpad_left_active())
            {
                // go back to the state menu
                watch2::preferences.end();
                watch2::switchState(2);
            }
            break;

        case 20: // manually set time and date

            Serial.printf("0: %d\n", selected_time);
            if (!watch2::state_init)
            {
                temp_time[0] = hour();
                temp_time[1] = minute();
                temp_time[2] = second();
                temp_time[3] = day();
                temp_time[4] = month();
                temp_time[5] = year();
            }
            //Serial.printf("1: %d\n", selected_time);
            //watch2::oled.setFreeFont(&SourceSansPro_Light12pt7b);
            
            if (dpad_left_active())
            {
                // decrement element selection
                selected_time--;
                if (selected_time < 0) selected_time = 5;
            }
            //Serial.printf("2: %d\n", selected_time);
            if (dpad_right_active())
            {
                // increment element selection
                selected_time++;
                if (selected_time > 5) selected_time = 0;
            }
            //Serial.printf("3: %d\n", selected_time);

            if (dpad_up_active())
            {
                // increment selected element
                if (temp_time[5] % 4 == 0) days_in_each_month[2] = 29;
                else days_in_each_month[2] = 28;                                                                                    // if it is a leap year, set days in Feb to 29, else set to 28
                if (selected_time == 3) time_limits[3] = days_in_each_month[temp_time[4]-1];                                        // set day limit based on month
                temp_time[selected_time]++;                                                                                         // increment value
                if (temp_time[selected_time] > time_limits[selected_time]) temp_time[selected_time] = time_lower[selected_time];    // if value is above limit, loop
                if (selected_time == 4 || selected_time == 5) temp_time[3] = std::min( temp_time[3], days_in_each_month[temp_time[4]-1] ); // limit current day value by month or year
            }
            //Serial.printf("4: %d\n", selected_time);
            if (dpad_down_active())
            {
                // increment selected element
                if (temp_time[5] % 4 == 0) days_in_each_month[1] = 29;
                else days_in_each_month[1] = 28;                                                                                    // if it is a leap year, set days in Feb to 29, else set to 28
                if (selected_time == 3) time_limits[3] = days_in_each_month[temp_time[4]-1];                                        // set day limit based on month
                temp_time[selected_time]--;                                                                                         // decrement value
                if (temp_time[selected_time] < time_lower[selected_time]) temp_time[selected_time] = time_limits[selected_time];    // if value is below limit, loop
                if (selected_time == 4 || selected_time == 4) temp_time[3] = std::min( temp_time[3], days_in_each_month[temp_time[4]-1] ); // limit current day value by month or year
            }
            //Serial.printf("5: %d\n", selected_time);

            if (dpad_any_active() || !watch2::state_init)
            {
                watch2::oled.setCursor(4, watch2::top_thing_height);
                watch2::setFont(LARGE_FONT);

                // redraw date settings thing
                for (int i = 0; i < 6; i++)
                {
                    if (i != 5) sprintf(text_aaaa, "%02d", temp_time[i]); //sets hours to zero when i = 5
                    else sprintf(text_aaaa, "%04d", temp_time[i]);
                    watch2::getTextBounds(String(text_aaaa), watch2::oled.getCursorX(), watch2::oled.getCursorY(), &x1, &y1, &w, &h);
                    watch2::oled.fillRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding) + 10, h + (2 * time_element_padding), BLACK);
                    if (selected_time == i) watch2::oled.drawRoundRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding), h, radius, watch2::themecolour);
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.printf("%02s", text_aaaa);
                    
                    if ( (i == 0) || (i == 1) ) watch2::oled.print(":");
                    if ( i == 2 ) watch2::oled.print("\n ");
                    if ( (i == 3) || (i == 4) ) watch2::oled.print(".");
                }

                watch2::setFont(MAIN_FONT);
            }

            if (dpad_enter_active())
            {
                // save time
                setTime(temp_time[0], temp_time[1], temp_time[2], temp_time[3], temp_time[4], temp_time[5]);
                watch2::switchState(watch2::state, 1);
            }
            //Serial.printf("7: %d\n", selected_time);
            //watch2::oled.setFreeFont(&SourceSansPro_Light8pt7b); // reset font
            break;

        case 2: //timeouts

            static std::vector<watch2::settingsMenuData> timeout_data;
            static int selected_timeout = 0;

            if (!watch2::state_init)
            {
                timeout_data.clear();
                timeout_data.push_back( (struct watch2::settingsMenuData){
                    "Timeout?",
                    watch2::preferences.getBool("timeout", true),
                    {"No", "Yes"},
                    24
                } );
                timeout_data.push_back( (struct watch2::settingsMenuData){
                    "Watch face \ntimeout",
                    watch2::preferences.getInt("short_timeout", 5000),
                    {},
                    24
                } );
                timeout_data.push_back( (struct watch2::settingsMenuData){
                    "Long timeout",
                    watch2::preferences.getInt("long_timeout", 30000),
                    {},
                    24
                } );
            }

            if (dpad_left_active())
            {
                int& setting_value = timeout_data[selected_timeout].setting_value;
                if (selected_timeout == 1 || selected_timeout == 2)
                {
                    setting_value -= 500;
                    if (setting_value < 0) setting_value = 0;
                }
                else if (selected_timeout == 0)
                {
                    setting_value = !setting_value;
                }
            }

            if (dpad_right_active())
            {
                int& setting_value = timeout_data[selected_timeout].setting_value;
                if (selected_timeout == 1 || selected_timeout == 2)
                {
                    setting_value += 500;
                }
                else if (selected_timeout == 0)
                {
                    setting_value = !setting_value;
                }
            }

            if (dpad_down_active())
            {
                selected_timeout++;
                if (selected_timeout > timeout_data.size() - 1) selected_timeout = 0;
            }

            if (dpad_up_active())
            {
                selected_timeout--;
                if (selected_timeout < 0 ) selected_timeout = timeout_data.size() - 1;
            }

            if (dpad_any_active() || !watch2::state_init)
            {
                //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
                watch2::oled.setTextColor(WHITE, BLACK);
                drawSettingsMenu(0, watch2::top_thing_height, SCREEN_WIDTH, SCREEN_HEIGHT - watch2::top_thing_height, timeout_data, selected_timeout, watch2::themecolour);
            }

            if (dpad_enter_active())
            {
                // store settings
                watch2::timeout = timeout_data[0].setting_value;
                watch2::short_timeout = timeout_data[1].setting_value;
                watch2::long_timeout = timeout_data[2].setting_value;

                watch2::preferences.putBool("timeout", watch2::timeout);
                watch2::preferences.putInt("short_timeout", watch2::short_timeout);
                watch2::preferences.putInt("long_timeout", watch2::long_timeout);

                // go back to settings menu
                watch2::switchState(watch2::state, 0);
            }

            break;

        case 3: //colour

            static std::vector<watch2::settingsMenuData> colour_data;
            static int selected_colour = 0;
            static int last_themecolour = watch2::themecolour;
            static uint8_t value_limit = 255;

            if (!watch2::state_init)
            {
                float r=0, g=0, b=0;
                watch2::colour888(watch2::themecolour, &r, &g, &b);

                colour_data.clear();
                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Time Colour",
                    watch2::preferences.getUInt("trans_mode", 0),
                    {"Random", "Gay™", "Trans", "Seconds", "Theme"},
                    24
                } );

                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Animate Time",
                    watch2::preferences.getBool("animate_time", true),
                    {"No", "Yes"}
                });

                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Theme colour R",
                    r,
                    {},
                    24
                } );

                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Theme colour G",
                    g,
                    {},
                    24
                } );

                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Theme colour B",
                    b,
                    {},
                    24
                } );
            }

            if (dpad_left_active())
            {
                int& setting_value = colour_data[selected_colour].setting_value;

                switch(selected_colour)
                {
                    case 0: // watch face colour scheme
                        value_limit = 4;
                        break;

                    case 1: // animate watch face
                        value_limit = 1;
                        break;
                    
                    case 2: // theme colour r
                    case 3: // theme colour g
                    case 4: // theme colour b
                        value_limit = 255;
                        break;
                }

                setting_value--;
                if (setting_value < 0) setting_value = value_limit;

                // calculate themecolour
                if (selected_colour != 0)
                {
                    watch2::themecolour = watch2::oled.color565(colour_data[2].setting_value,
                                                    colour_data[3].setting_value,
                                                    colour_data[4].setting_value);
                }

            }

            if (dpad_right_active())
            {
                int& setting_value = colour_data[selected_colour].setting_value;

                switch(selected_colour)
                {
                    case 0: // watch face colour scheme
                        value_limit = 4;
                        break;

                    case 1: // animate watch face
                        value_limit = 1;
                        break;
                    
                    case 2: // theme colour r
                    case 3: // theme colour g
                    case 4: // theme colour b
                        value_limit = 255;
                        break;
                }

                setting_value++;
                if (setting_value > value_limit) setting_value = 0;

                // calculate themecolour
                watch2::themecolour = watch2::oled.color565(colour_data[2].setting_value,
                                                colour_data[3].setting_value,
                                                colour_data[4].setting_value);
            }

            if (dpad_down_active())
            {
                selected_colour++;
                if (selected_colour > colour_data.size() - 1) selected_colour = 0;
            }

            if (dpad_up_active())
            {
                selected_colour--;
                if (selected_colour < 0 ) selected_colour = colour_data.size() - 1;
            }

            if (dpad_any_active() || !watch2::state_init)
            {
                //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
                watch2::oled.setTextColor(WHITE, BLACK);
                drawSettingsMenu(0, watch2::top_thing_height, SCREEN_WIDTH, SCREEN_HEIGHT - watch2::top_thing_height, colour_data, selected_colour, watch2::themecolour);
            }

            if (dpad_enter_active())
            {
                // store settings
                watch2::trans_mode = colour_data[0].setting_value;
                watch2::animate_watch_face = colour_data[1].setting_value;

                watch2::preferences.putUInt("trans_mode", watch2::trans_mode);
                watch2::preferences.putBool("animate_time", watch2::animate_watch_face);
                watch2::preferences.putInt("themecolour", watch2::themecolour);

                // go back to settings menu
                watch2::switchState(watch2::state, 0);
            }

            break;

        case 5: //about

            static int yoffset = 0;
            static const int about_height = 2000;
            static uint64_t chipid = ESP.getEfuseMac();
            static TFT_eSprite about_text = TFT_eSprite(&watch2::oled);
            
            if (!watch2::state_init)
            {
                // set up sprite
                // about_text.setColorDepth(1);
                // about_text.setTextWrap(false);
                // about_text.createSprite(SCREEN_WIDTH, 200);
                // about_text.setScrollRect(0, 0, SCREEN_WIDTH, 100, BLACK);

                // print info to sprite
                watch2::oled.setCursor(0, watch2::top_thing_height);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setTextWrap(false, false);
                watch2::setFont(MAIN_FONT, about_text);
                watch2::oled.printf("watch ii, ver %s\n", WATCH_VER);
                watch2::oled.printf("by alice (atctwo)\n\n");
                
                watch2::oled.printf("Compile date: %s\n", __DATE__);
                watch2::oled.printf("Compile time: %s\n", __TIME__);

                watch2::oled.printf("MAC address: %x", ESP.getEfuseMac());
                watch2::oled.printf("Chip Revision: %d\n", ESP.getChipRevision());
                watch2::oled.printf("SDK Version: %d\n", ESP.getSdkVersion());
                watch2::oled.printf("CPU Frequency: %dMHz\n", ESP.getCpuFreqMHz());
                watch2::oled.printf("Sketch Size: %d", watch2::humanSize(ESP.getSketchSize()));
                watch2::oled.printf("Flash Size: %d\n", watch2::humanSize(ESP.getFlashChipSize()));
                watch2::oled.printf("Flash Speed: %d\n", ESP.getFlashChipSpeed());
                watch2::oled.printf("Flash Mode: ");
                switch(ESP.getFlashChipMode())
                {
                    case 0x00: watch2::oled.printf("QIO\n"); break;
                    case 0x01: watch2::oled.printf("QOUT\n"); break;
                    case 0x02: watch2::oled.printf("DIO\n"); break;
                    case 0x03: watch2::oled.printf("DOUT\n"); break;
                    case 0x04: watch2::oled.printf("Fast read\n"); break;
                    case 0x05: watch2::oled.printf("Slow read\n"); break;
                    default:   watch2::oled.printf("Unknown\n"); break;
                }
                watch2::oled.printf("Total heap:  %s\n", watch2::humanSize(ESP.getHeapSize()));
                watch2::oled.printf("Free heap:   %s\n", watch2::humanSize(ESP.getFreeHeap()));
                watch2::oled.printf("Total PSRAM: %s\n", watch2::humanSize(ESP.getPsramSize()));
                watch2::oled.printf("Free PSRAM:  %s\n", watch2::humanSize(ESP.getFreePsram()));
            }

            if (dpad_any_active() || !watch2::state_init)
            {
                //about_text.pushSprite(0, watch2::top_thing_height);
            }

            if (dpad_enter_active() || dpad_left_active())
            {
                watch2::switchState(watch2::state, 0);
            }

            break;

        case 1: // time and date menu

            static std::vector<std::string> set_time_buttons = {"Set time manually", "Set time using Internet"};
            static std::vector<watch2::settingsMenuData> ntp_settings = {
                (watch2::settingsMenuData){"Timezone", watch2::timezone, {}, 24},
                (watch2::settingsMenuData){"Set Internet time\non boot", watch2::ntp_boot_connect, {"No", "Yes"}, 24},
                (watch2::settingsMenuData){"Set Internet time\non wakeup", watch2::ntp_wakeup_connect, {"No", "Yes"}, 24}
            };
            static uint8_t selected_item = 0;
            static uint16_t ypos = watch2::top_thing_height;

            if (dpad_up_active())
            {
                if (selected_item == 0) selected_item = 4;
                else selected_item--;
            }

            if (dpad_down_active())
            {
                selected_item++;
                if (selected_item > 4) selected_item = 0;
            }

            if (dpad_left_active())
            {
                switch(selected_item)
                {
                    case 2: // timezone
                        watch2::timezone--;
                        if (watch2::timezone < -12) watch2::timezone = 12;
                        ntp_settings[selected_item-2].setting_value = watch2::timezone;
                        break;
                        
                    case 4: // ntp on wakeup
                        watch2::ntp_wakeup_connect = !watch2::ntp_wakeup_connect;
                        ntp_settings[selected_item-2].setting_value = watch2::ntp_wakeup_connect;
                        break;

                    case 3: // ntp on boot
                        watch2::ntp_boot_connect = !watch2::ntp_boot_connect;
                        ntp_settings[selected_item-2].setting_value = watch2::ntp_boot_connect;
                        break;
                }
            }

            if (dpad_right_active())
            {
                switch(selected_item)
                {
                    case 2: // timezone
                        watch2::timezone++;
                        if (watch2::timezone > 12) watch2::timezone = -12;
                        ntp_settings[selected_item-2].setting_value = watch2::timezone;
                        break;
                        
                    case 4: // ntp on wakeup
                        watch2::ntp_wakeup_connect = !watch2::ntp_wakeup_connect;
                        ntp_settings[selected_item-2].setting_value = watch2::ntp_wakeup_connect;
                        break;

                    case 3: // ntp on boot
                        watch2::ntp_boot_connect = !watch2::ntp_boot_connect;
                        ntp_settings[selected_item-2].setting_value = watch2::ntp_boot_connect;
                        break;
                }
            }

            if (!watch2::state_init || dpad_any_active())
            {

                // draw "set time" buttons
                ypos = watch2::top_thing_height;
                watch2::drawMenu(
                    2, ypos, 
                    SCREEN_WIDTH-4, 24 + (watch2::oled.fontHeight() * 2), 
                    set_time_buttons, selected_item, false
                );
                ypos += 24 + (watch2::oled.fontHeight() * 2);

                // draw ntp settings
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::drawSettingsMenu(
                    2, ypos,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - ypos,
                    ntp_settings, selected_item - 2
                );

            }

            if (dpad_enter_active())
            {
                // set time manually
                if (selected_item == 0) watch2::switchState(watch2::state, 20);
                
                // set time using ntp
                else if (selected_item == 1) watch2::getTimeFromNTP();
                
                // go back to the menu
                else 
                {
                    watch2::preferences.begin("watch2");
                    watch2::preferences.putUInt("timezone", watch2::timezone);
                    watch2::preferences.putBool("ntp_wakeup", watch2::ntp_wakeup_connect);
                    watch2::preferences.putBool("ntp_boot", watch2::ntp_boot_connect);
                    watch2::preferences.end();
                    watch2::switchState(watch2::state, 0);
                }
            }

            break;

        case 4: // wifi

            static std::vector<watch2::settingsMenuData> wifi_settings = {
                (watch2::settingsMenuData){"Wifi Enabled", watch2::wifi_state != 0, {"No", "Yes"}, 24},
                (watch2::settingsMenuData){"Connect to wifi\non boot", watch2::wifi_boot_reconnect, {"No", "Yes"}, 24},
                (watch2::settingsMenuData){"Connect to wifi\non wakeup", watch2::wifi_wakeup_reconnect, {"No", "Yes"}, 24}
            };
            static std::vector<std::string> wifi_buttons = {"Connect to an AP", "Edit Profiles"};
            static uint8_t selected_item_wifi = 0;
            static uint16_t ypos_wifi = watch2::top_thing_height;

            if (dpad_up_active())
            {
                if (selected_item_wifi == 0) selected_item_wifi = 4;
                else selected_item_wifi--;
            }

            if (dpad_down_active())
            {
                selected_item_wifi++;
                if (selected_item_wifi > 4) selected_item_wifi = 0;
            }

            if (dpad_left_active() || dpad_right_active())
            {
                switch(selected_item_wifi)
                {
                    case 0: // wifi enabled
                        
                        if (watch2::wifi_state == 0)
                        {
                            watch2::enable_wifi();
                            wifi_settings[0].setting_value = true;
                        }
                        else
                        {
                            watch2::disable_wifi();
                            wifi_settings[0].setting_value = false;
                        }
                        
                        break;

                    case 2: // reconnect on wakeup

                        watch2::wifi_wakeup_reconnect = !watch2::wifi_wakeup_reconnect;
                        wifi_settings[2].setting_value = watch2::wifi_wakeup_reconnect;
                        break;

                    case 1: // reconnect on boot

                        watch2::wifi_boot_reconnect = !watch2::wifi_boot_reconnect;
                        wifi_settings[1].setting_value = watch2::wifi_boot_reconnect;
                        break;
                }
            }

            if (!watch2::state_init || dpad_any_active())
            {
                // draw wifi settings
                ypos_wifi = watch2::top_thing_height;
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::drawSettingsMenu(
                    2, ypos_wifi,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - ypos,
                    wifi_settings, selected_item_wifi
                );
                ypos_wifi += (watch2::oled.fontHeight() * 5) * 42;

                // draw wifi buttons
                watch2::drawMenu(
                    2, 150, 
                    SCREEN_WIDTH-4, 24 + (watch2::oled.fontHeight() * 2), 
                    wifi_buttons, selected_item_wifi - 3, false
                );
            }

            if (dpad_enter_active())
            {
                if (selected_item_wifi == 3)
                {
                    // go to the ssid menu
                    watch2::switchState(watch2::state, 21);
                }
                else if (selected_item_wifi == 4)
                {
                    // go to the edit profile menu
                    watch2::switchState(watch2::state, 22);
                }
                else
                {
                    // go back to the menu
                    watch2::preferences.begin("watch2");
                    watch2::preferences.putBool("wifi_wakeup", watch2::wifi_wakeup_reconnect);
                    watch2::preferences.putBool("wifi_boot", watch2::wifi_boot_reconnect);
                    watch2::preferences.end();
                    watch2::switchState(watch2::state, 0);
                }
            }

            break;

        case 21: // ssid menu

            static std::vector<std::string> ssid_names = {"Cancel", "WPS"};
            static uint8_t no_ssid = 0; // number of ssids found after scanning
            static bool scanned = false;
            static uint8_t selected_ssid = 0;

            if (!watch2::state_init)
            {
                scanned = false;
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(0, watch2::top_thing_height);

                // scan for aps
                watch2::oled.println("Scanning for APs");
                no_ssid = WiFi.scanNetworks();
                scanned = true;

                // add scanned aps to ssid list
                ssid_names.clear();
                ssid_names.push_back("Cancel");
                ssid_names.push_back("WPS");
                for (int i = 0; i < no_ssid; i++)
                {
                    ssid_names.push_back(WiFi.SSID(i).c_str());
                }

                // clear screen
                watch2::oled.fillRect(0, watch2::top_thing_height, SCREEN_WIDTH, SCREEN_HEIGHT - watch2::top_thing_height, BLACK);
            }

            if (dpad_up_active())
            {
                if (selected_ssid == 0) selected_ssid = no_ssid - 1;
                else selected_ssid--;
            }

            if (dpad_down_active())
            {
                selected_ssid++;
                if (selected_ssid >= no_ssid) no_ssid = 0;
            }

            if (!watch2::state_init || dpad_any_active() || watch2::forceRedraw)
            {
                // draw ssids
                watch2::drawMenu(
                    2, watch2::top_thing_height,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - watch2::top_thing_height,
                    ssid_names, selected_ssid
                );
                watch2::forceRedraw = false;
            }

            if (dpad_enter_active())
            {
                if (selected_ssid == 0) // cancel
                {
                    // return to wifi menu
                    watch2::switchState(watch2::state, 4);
                }
                else if (selected_ssid == 1) // wps
                {
                    // wps connection
                }
                else
                {
                    // get wifi profiles
                    cJSON *profiles = watch2::getWifiProfiles();

                    // if the ap has a profile
                    bool help = false;
                    cJSON *profile;
                    cJSON *profile_array = cJSON_GetObjectItem(profiles, "profiles");
                    for (int i = 0; i < cJSON_GetArraySize(profile_array); i++)
                    {
                        profile = cJSON_GetArrayItem(profile_array, i);
                        const char* ssid = cJSON_GetObjectItem(profile, "ssid")->valuestring;
                        Serial.printf("checking ssid %s\n", ssid);
                        if (strcmp(ssid, WiFi.SSID(selected_ssid-2).c_str()) == 0) // if profile ssid matches ap ssid
                        {
                            help = true;
                            break;
                        }
                    }

                    if (help) // the ap does have a profile
                    {
                        Serial.println("connecting using stored profile");
                        watch2::wifi_encryption = (wifi_auth_mode_t) cJSON_GetObjectItem(profile, "encryption")->valueint;
                        watch2::connectToWifiAP(
                            cJSON_GetObjectItem(profile, "ssid")->valuestring,
                            cJSON_GetObjectItem(profile, "password")->valuestring
                        );
                    }
                    else // the ap doesn't have a profile
                    {
                        // connect to selected ssid
                        Serial.println("no profile was found, so connecting based on user input");
                        wifi_auth_mode_t ap_encryption = WiFi.encryptionType(selected_ssid - 2);
                        std::string password = "none";

                        if (ap_encryption == WIFI_AUTH_WEP || ap_encryption == WIFI_AUTH_WPA_PSK || ap_encryption == WIFI_AUTH_WPA2_PSK || ap_encryption == WIFI_AUTH_WPA_WPA2_PSK)
                        {
                            // get password
                            password = watch2::textFieldDialogue(std::string("Password for\n").append(WiFi.SSID(selected_ssid-2).c_str()), "", '*');
                        }

                        if (!password.empty()) watch2::connectToWifiAP(WiFi.SSID(selected_ssid-2).c_str(), password.c_str());

                    }

                    // free memory
                    cJSON_Delete(profiles);
                }
            }

            break;

        case 22: // edit profile menu

            static std::vector<std::string> profiles_vector = {};
            static uint16_t selected_profile = 0;

            if (!watch2::state_init)
            {
                // get profiles
                profiles = watch2::getWifiProfiles();
                profile_array = cJSON_GetObjectItem(profiles, "profiles");

                // populate profiles vector with profile ssids
                profiles_vector.clear();
                profiles_vector.push_back("Cancel");
                for (int i = 0; i < cJSON_GetArraySize(profile_array); i++)
                {
                    cJSON *profile = cJSON_GetArrayItem(profile_array, i);
                    profiles_vector.push_back(cJSON_GetObjectItem(profile, "ssid")->valuestring);
                }

            }

            if (dpad_up_active())
            {
                if (selected_profile == 0) selected_profile = profiles_vector.size() - 1;
                else selected_profile--;
            }

            if (dpad_down_active())
            {
                if (selected_profile == profiles_vector.size() - 1) selected_profile = 0;
                else selected_profile++;
            }

            if (!watch2::state_init || dpad_any_active())
            {
                // draw profile menu
                watch2::drawMenu(
                    2, watch2::top_thing_height,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - watch2::top_thing_height,
                    profiles_vector, selected_profile
                );
            }

            if (dpad_enter_active())
            {
                if (selected_profile == 0) // cancel
                {
                    // go back to the wifi menu
                    cJSON_Delete(profiles);
                    watch2::switchState(watch2::state, 4);
                }
                else
                {
                    // go to the profile editor
                    if (profile_array)
                    {
                        profile_list = profile_array;
                        edit_profile_index = selected_profile - 1;
                        watch2::switchState(watch2::state, 23);
                    }
                }
            }

            break;

        case 23: // profile editor

            static std::vector<std::string> profile_actions = {"Back", "Edit SSID", "Edit Password", "Delete Profile"};
            static std::vector<watch2::settingsMenuData> profile_settings = {
                (watch2::settingsMenuData){"Encryption", 0, 
                {"Open", "WEP", "WPA", "WPA2", "WPA1/2", "WPA2E", "???"}, 24}
            };
            static uint8_t selected_profile_setting = 0;
            static std::string ssid = "";
            static std::string password = "";
            static wifi_auth_mode_t encryption = WIFI_AUTH_OPEN;
            static cJSON *edit_profile;

            if (!watch2::state_init)
            {
                if (profile_list)
                {
                    edit_profile = cJSON_GetArrayItem(profile_list, edit_profile_index);
                    ssid = cJSON_GetObjectItem(edit_profile, "ssid")->valuestring;
                    password = cJSON_GetObjectItem(edit_profile, "password")->valuestring;
                    encryption = (wifi_auth_mode_t) cJSON_GetObjectItem(edit_profile, "encryption")->valueint;

                    profile_settings[0].setting_value = encryption;
                }
            }

            if (dpad_up_active())
            {
                if (selected_profile_setting == 0) selected_profile_setting = 4;
                else selected_profile_setting--;
            }

            if (dpad_down_active())
            {
                if (selected_profile_setting == 4) selected_profile_setting = 0;
                else selected_profile_setting++;
            }

            if (!watch2::state_init || dpad_any_active() || watch2::forceRedraw)
            {
                // draw ssid
                watch2::oled.fillRect(0, watch2::top_thing_height, SCREEN_WIDTH, watch2::oled.fontHeight(), BLACK);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.print(ssid.c_str());

                // draw menu
                uint16_t button_menu_y = watch2::top_thing_height + watch2::oled.fontHeight() + 5;
                watch2::drawMenu(
                    2, button_menu_y,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - watch2::top_thing_height,
                    profile_actions, selected_profile_setting, false
                );

                // draw encryption menu
                uint16_t settings_menu_y = button_menu_y + ( (12 + watch2::oled.fontHeight()) * profile_actions.size() );
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::drawSettingsMenu(
                    2, settings_menu_y,
                    SCREEN_WIDTH - 4, SCREEN_HEIGHT - settings_menu_y,
                    profile_settings, selected_profile_setting - profile_actions.size()
                );

                watch2::forceRedraw = false;
            }

            if (dpad_enter_active())
            {
                std::string ssid2 = "", password2 = "";
                switch (selected_profile_setting)
                {
                    case 0: // back
                        // save settings
                        if (edit_profile)
                        {
                            Serial.println("updating AP profile");
                            cJSON_ReplaceItemInObject(edit_profile, "ssid", cJSON_CreateString(ssid.c_str()));
                            cJSON_ReplaceItemInObject(edit_profile, "password", cJSON_CreateString(password.c_str()));
                            cJSON_ReplaceItemInObject(edit_profile, "encryption", cJSON_CreateNumber(encryption));
                            watch2::setWifiProfiles(profiles);
                        }

                        // clear settings
                        ssid = "";
                        password = "";
                        encryption = WIFI_AUTH_MAX;

                        // go back to profile menu
                        watch2::switchState(watch2::state, 22);
                        break;

                    case 1: // ssid
                        ssid2 = watch2::textFieldDialogue("SSID", ssid.c_str());
                        if (!ssid2.empty()) ssid.assign(ssid2);
                        break;

                    case 2: // password
                        password2 = watch2::textFieldDialogue("Password", password.c_str(), '*');
                        if (!password2.empty()) password.assign(password2);
                        break;

                    case 3: // delete profile

                        // todo: get user confirmation

                        if (edit_profile)
                        {
                            Serial.println("deleting AP profile");

                            // delete profile
                            cJSON_DeleteItemFromArray(profile_array, edit_profile_index);

                            // remove ssid from access index
                            cJSON *access_index = cJSON_GetObjectItem(profiles, "access_index");
                            int profile_index = -1;
                            for (int i = 0; i < cJSON_GetArraySize(access_index); i++)
                            {
                                if (strcmp(ssid.c_str(), cJSON_GetArrayItem(access_index, i)->valuestring) == 0)
                                {
                                    profile_index = i;
                                    break;
                                }
                            }
                            if (profile_index > -1) cJSON_DeleteItemFromArray(access_index, profile_index);

                            // save changes
                            watch2::setWifiProfiles(profiles);
                            selected_profile--; // this might be a bit hacky
                        }

                        // clear settings
                        ssid = "";
                        password = "";
                        encryption = WIFI_AUTH_MAX;

                        // go back to profile menu
                        watch2::switchState(watch2::state, 22);
                        break;
                }

            }

            break;
    }

    watch2::drawTopThing();
}