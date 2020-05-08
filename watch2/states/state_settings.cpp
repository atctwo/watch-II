#include "../src/watch2.h"

void state_func_settings()
{
    static std::vector<std::string> panels = {"Time and Date", "Timeout", "Colour", "About"};
    static int selected_panel = 0;
    static int last_selected_time = 0;

    static int temp_time[6] = {0};
    static int selected_time = 0;
    static int time_limits[6] = {23, 59, 59, 30, 12, 2106};
    const  int time_lower[6]  = {0,  0,  0,  1,  1,  1970};
    static int days_in_each_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
                if (selected_panel < 0) selected_panel = panels.size();
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

        case 1: //time and date

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
                watch2::switchState(watch2::state, 0);
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

        case 4: //about

            static int yoffset = 0;
            const int about_height = 2000;
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
    }

    watch2::drawTopThing();
}