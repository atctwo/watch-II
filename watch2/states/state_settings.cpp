#include "../src/watch2.h"

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

            if (!watch2::state_init)
            {
                temp_time[0] = hour();
                temp_time[1] = minute();
                temp_time[2] = second();
                temp_time[3] = day();
                temp_time[4] = month();
                temp_time[5] = year();
            }

            //watch2::oled.setFreeFont(&SourceSansPro_Light12pt7b);
            watch2::oled.setCursor(4, 12 + 16);

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

            if (dpad_any_active() || !watch2::state_init)
            {
                // redraw date settings thing
                for (int i = 0; i < 6; i++)
                {

                    sprintf(text_aaaa, "%02d", temp_time[i]);
                    watch2::getTextBounds(String(text_aaaa), watch2::oled.getCursorX(), watch2::oled.getCursorY(), &x1, &y1, &w, &h);
                    watch2::oled.fillRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding) + 10, h + (2 * time_element_padding), BLACK);
                    if (selected_time == i) watch2::oled.drawRoundRect(x1 - time_element_padding, y1 - time_element_padding, w + (2 * time_element_padding), h + (2 * time_element_padding), radius, watch2::themecolour);
                    //oled.fillRect(x1, y1, w, h, BLACK);
                    watch2::oled.printf("%02s", text_aaaa);

                    if ( (i == 0) || (i == 1) ) watch2::oled.print(":");
                    if ( i == 2 ) watch2::oled.print("\n ");
                    if ( (i == 3) || (i == 4) ) watch2::oled.print(".");
                }
            }

            if (dpad_enter_active())
            {
                // save time
                setTime(temp_time[0], temp_time[1], temp_time[2], temp_time[3], temp_time[4], temp_time[5]);
                watch2::switchState(watch2::state, 0);
            }
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
                drawSettingsMenu(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, timeout_data, selected_timeout, watch2::themecolour);
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

            if (!watch2::state_init)
            {
                float r=0, g=0, b=0;
                watch2::colour888(watch2::themecolour, &r, &g, &b);

                colour_data.clear();
                colour_data.push_back( (struct watch2::settingsMenuData){
                    "Trans Mode",
                    watch2::preferences.getBool("trans_mode", false),
                    {"Off", "On"},
                    24
                } );
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
                watch2::themecolour = watch2::oled.color565(colour_data[1].setting_value,
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
                watch2::themecolour = watch2::oled.color565(colour_data[1].setting_value,
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

            if (dpad_any_active() || !watch2::state_init)
            {
                //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
                watch2::oled.setTextColor(WHITE, BLACK);
                drawSettingsMenu(0, 12, SCREEN_WIDTH, SCREEN_HEIGHT - 12, colour_data, selected_colour, watch2::themecolour);
            }

            if (dpad_enter_active())
            {
                // store settings
                watch2::trans_mode = colour_data[0].setting_value;

                watch2::preferences.putBool("trans_mode", watch2::trans_mode);
                watch2::preferences.putInt("themecolour", watch2::themecolour);

                // go back to settings menu
                watch2::switchState(watch2::state, 0);
            }

            break;

        case 4: //about

            static int yoffset = 0;
            const int about_height = 200;
            static uint64_t chipid = ESP.getEfuseMac();
            /*
            static GFXcanvas16 *canvas_about = new GFXcanvas16(SCREEN_WIDTH, about_height);

            if (!watch2::state_init)
            {
                //clear text
                //oled.fillRect(0, 11, SCREEN_WIDTH, SCREEN_HEIGHT-11, BLACK);

                canvas_about->setFreeFont(&SourceSansPro_Regular6pt7b);
                canvas_about->setCursor(0, 10);  //normally, y = 20
                canvas_about->setTextColor(WHITE);

                canvas_about->print("watch II, version " + String(WATCH_VER) + "\nmade by alice (atctwo)\n\n");
                canvas_about->print("Compile date and time:\n  ");
                canvas_about->setTextColor(watch2::themecolour);
                canvas_about->print(String(__DATE__) + " " + String(__TIME__) + "\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Chip Revision: ");
                canvas_about->setTextColor(watch2::themecolour);
                canvas_about->print(String(ESP.getChipRevision()) + "\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("CPU Frequency: ");
                canvas_about->setTextColor(watch2::themecolour);
                canvas_about->print( String(ESP.getCpuFreqMHz()) + " MHz" );

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Sketch Size: ");
                canvas_about->setTextColor(watch2::themecolour);
                canvas_about->print( String(ESP.getSketchSize()) + " B\n");

                canvas_about->setTextColor(WHITE);
                canvas_about->print("Flash Size: ");
                canvas_about->setTextColor(watch2::themecolour);
                canvas_about->print( String(ESP.getFlashChipSize()) + " B\n" );

                canvas_about->setTextColor(WHITE);
                canvas_about->print("MAC Address: \n  ");
                canvas_about->setTextColor(watch2::themecolour);
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

            if (!watch2::state_init || dpad_any_active())
            {
                //clear top of screen for top bar thing
                //watch2::oled.drawRGBBitmap(0, 20 - yoffset, canvas_about->getBuffer(), SCREEN_WIDTH, about_height);
                watch2::oled.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK);
            }*/

            if (dpad_enter_active() || dpad_left_active())
            {
                watch2::switchState(watch2::state, 0);
            }

            break;
    }

    watch2::drawTopThing();
}