#include "../globals.h"

void state_func_settings()
{
    static std::vector<std::string> panels = {"Time and Date", "Timeout", "Colour", "About", "when", "i", "was", "sixteen", "i", "learned", "to", "sing"};
    static int selected_panel = 0;
    static int last_selected_time = 0;

    static int temp_time[6] = {0};
    static int selected_time = 0;
    static int time_limits[6] = {23, 59, 59, 30, 12, 2106};
    const  int time_lower[6]  = {0,  0,  0,  1,  1,  1970};
    static int days_in_each_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static char text_aaaa[4];
    static int time_element_padding = 3;
    static int radius = 4;

    preferences.begin("watch2", false);      //open watch II preferences in RW mode

    switch (states[state].variant)
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
            if (dpad_any_active() || !state_init)
            {
                // draw menu
                oled.setFont(&SourceSansPro_Regular6pt7b);
                drawMenu(2, 12, SCREEN_WIDTH-4, SCREEN_HEIGHT-12, panels, selected_panel, themecolour);
            }
            if (dpad_enter_active())
            {
                // go to panel
                switchState(3, selected_panel + 1);
            }
            if (dpad_left_active())
            {
                // go back to the state menu
                preferences.end();
                switchState(2);
            }
            break;

        case 1: //time and date

            if (!state_init)
            {
                temp_time[0] = hour();
                temp_time[1] = minute();
                temp_time[2] = second();
                temp_time[3] = day();
                temp_time[4] = month();
                temp_time[5] = year();
            }

            oled.setFont(&SourceSansPro_Light12pt7b);
            oled.setCursor(4, 12 + 16);

            if (dpad_left_active())
            {
                // decrement element selection
                selected_time--;
                if (selected_time < 0) selected_time = 5;
            }
            if (dpad_right_active())
            {
                // increment element selection
                selected_time++;
                if (selected_time > 5) selected_time = 0;
            }

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

            if (dpad_any_active() || !state_init)
            {
                // redraw date settings thing
                for (int i = 0; i < 6; i++)
                {

                    sprintf(text_aaaa, "%02d", temp_time[i]);
                    oled.getTextBounds(String(text_aaaa), oled.getCursorX(), oled.getCursorY(), &x1, &y1, &w, &h);
                    oled.fillRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding) + 10, h + (2 * time_element_padding), BLACK);
                    if (selected_time == i) oled.drawRoundRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding), h + (2 * time_element_padding), radius, themecolour);
                    //oled.fillRect(x1, y1, w, h, BLACK);
                    oled.printf("%02s", text_aaaa);

                    if ( (i == 0) || (i == 1) ) oled.print(":");
                    if ( i == 2 ) oled.print("\n ");
                    if ( (i == 3) || (i == 4) ) oled.print(".");
                }
            }

            if (dpad_enter_active())
            {
                // save time
                setTime(temp_time[0], temp_time[1], temp_time[2], temp_time[3], temp_time[4], temp_time[5]);
                switchState(state, 0);
            }
            oled.setFont(&SourceSansPro_Light8pt7b); // reset font
            break;

        case 2: //timeouts

            static std::vector<settingsMenuData> timeout_data;
            static int selected_timeout = 0;

            if (!state_init)
            {
                timeout_data.clear();
                timeout_data.push_back( (struct settingsMenuData){
                    "Timeout?",
                    preferences.getBool("timeout", true),
                    {"No", "Yes"},
                    24
                } );
                timeout_data.push_back( (struct settingsMenuData){
                    "Watch face \ntimeout",
                    preferences.getInt("short_timeout", 5000),
                    {},
                    24
                } );
                timeout_data.push_back( (struct settingsMenuData){
                    "Long timeout",
                    preferences.getInt("long_timeout", 30000),
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

            if (dpad_any_active() || !state_init)
            {
                oled.setFont(&SourceSansPro_Regular6pt7b);
                oled.setTextColor(WHITE);
                drawSettingsMenu(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, timeout_data, selected_timeout, themecolour);
            }

            if (dpad_enter_active())
            {
                // store settings
                timeout = timeout_data[0].setting_value;
                short_timeout = timeout_data[1].setting_value;
                long_timeout = timeout_data[2].setting_value;

                preferences.putBool("timeout", timeout);
                preferences.putInt("short_timeout", short_timeout);
                preferences.putInt("long_timeout", long_timeout);

                // go back to settings menu
                switchState(state, 0);
            }

            break;

        case 3: //colour

            static std::vector<settingsMenuData> colour_data;
            static int selected_colour = 0;
            static int last_themecolour = themecolour;

            if (!state_init)
            {
                float r=0, g=0, b=0;
                colour888(themecolour, &r, &g, &b);

                colour_data.clear();
                colour_data.push_back( (struct settingsMenuData){
                    "Trans Mode",
                    preferences.getBool("trans_mode", false),
                    {"Off", "On"},
                    24
                } );
                colour_data.push_back( (struct settingsMenuData){
                    "Theme colour R",
                    r,
                    {},
                    24
                } );
                colour_data.push_back( (struct settingsMenuData){
                    "Theme colour G",
                    g,
                    {},
                    24
                } );
                colour_data.push_back( (struct settingsMenuData){
                    "Theme colour B",
                    b,
                    {},
                    24
                } );
            }

            if (dpad_left_active())
            {
                int& setting_value = colour_data[selected_colour].setting_value;
                if (selected_colour == 1 || selected_colour == 2 || selected_colour == 3)
                {
                    setting_value--;
                    if (setting_value < 0) setting_value = 255;
                }
                else if (selected_colour == 0)
                {
                    setting_value = !setting_value;
                }

                // calculate themecolour
                themecolour = oled.color565(colour_data[1].setting_value,
                                                colour_data[2].setting_value,
                                                colour_data[3].setting_value);

            }

            if (dpad_right_active())
            {
                int& setting_value = colour_data[selected_colour].setting_value;
                if (selected_colour == 1 || selected_colour == 2 || selected_colour == 3)
                {
                    setting_value++;
                    if (setting_value > 255) setting_value = 0;
                }
                else if (selected_colour == 0)
                {
                    setting_value = !setting_value;
                }

                // calculate themecolour
                themecolour = oled.color565(colour_data[1].setting_value,
                                                colour_data[2].setting_value,
                                                colour_data[3].setting_value);
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

            if (dpad_any_active() || !state_init)
            {
                oled.setFont(&SourceSansPro_Regular6pt7b);
                oled.setTextColor(WHITE);
                drawSettingsMenu(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, colour_data, selected_colour, themecolour);
            }

            if (dpad_enter_active())
            {
                // store settings
                trans_mode = colour_data[0].setting_value;

                preferences.putBool("trans_mode", trans_mode);
                preferences.putInt("themecolour", themecolour);

                // go back to settings menu
                switchState(state, 0);
            }

            break;

        case 4: //about

            static int yoffset = 0;
            const int about_height = 200;
            static uint64_t chipid = ESP.getEfuseMac();
            static GFXcanvas16 *canvas_about = new GFXcanvas16(SCREEN_WIDTH, about_height);

            if (!state_init)
            {
                //clear text
                //oled.fillRect(0, 11, SCREEN_WIDTH, SCREEN_HEIGHT-11, BLACK);

                canvas_about->setFont(&SourceSansPro_Regular6pt7b);
                canvas_about->setCursor(0, 10);  //normally, y = 20
                canvas_about->setTextColor(WHITE);

                canvas_about->print("watch II, version " + String(WATCH_VER) + "\nmade by alice (atctwo)\n\n");
                canvas_about->print("Compile date and time:\n  ");
                canvas_about->setTextColor(themecolour);
                canvas_about->print(String(__DATE__) + " " + String(__TIME__) + "\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Chip Revision: ");
                canvas_about->setTextColor(themecolour);
                canvas_about->print(String(ESP.getChipRevision()) + "\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("CPU Frequency: ");
                canvas_about->setTextColor(themecolour);
                canvas_about->print( String(ESP.getCpuFreqMHz()) + " MHz" );

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Sketch Size: ");
                canvas_about->setTextColor(themecolour);
                canvas_about->print( String(ESP.getSketchSize()) + " B\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Flash Size: ");
                canvas_about->setTextColor(themecolour);
                canvas_about->print( String(ESP.getFlashChipSize()) + " B\n" );

                canvas_about->setTextColor(WHITE);
                canvas_about->print("MAC Address: \n  ");
                canvas_about->setTextColor(themecolour);
                canvas_about->printf("%04X",(uint16_t)(chipid>>32));//print High 2 bytes
                canvas_about->printf("%08X",(uint32_t)chipid);//print Low 4bytes.

            }

            if (dpad_down_active())
            {
                yoffset = std::min(about_height - (SCREEN_HEIGHT-11), yoffset + 1);
            }

            if (dpad_up_active())
            {
                yoffset = std::max(0, yoffset - 1);
            }

            if (!state_init || dpad_any_active())
            {
                //clear top of screen for top bar thing
                oled.drawRGBBitmap(0, 20 - yoffset, canvas_about->getBuffer(), SCREEN_WIDTH, about_height);
                oled.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK);
            }

            if (dpad_enter_active() || dpad_left_active())
            {
                switchState(state, 0);
            }

            break;
    }

    drawTopThing();
}