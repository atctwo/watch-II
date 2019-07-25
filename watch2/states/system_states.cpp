void registerSystemStates()
{

    //state -1 (no state loaded)
    registerState("Limbo", "empty", [](){

        //dim screen
        uint8_t contrast = 0x0F;
        oled.sendCommand(0xC7, &contrast, 1);

        digitalWrite(12, HIGH);

        oled.setCursor(5, 10);
        oled.setTextColor(WHITE, BLACK);
        oled.print("no state has been\nloaded");
        oled.drawRGBBitmap(0, 29, coolcrab, 128, 55);
        if (dpad_any_active()) switchState(0);

    }, true);

    // state 0
    registerState("Initial State", "init", [](){

        //if variant is 0, this state just switches to state 1 (watch face) without
        //doing much.  if the down button is held, this state switches to variant 1,
        //which will allow the user to wipe their settings

        static bool selected_option = false;

        if (states[state].variant == 0)
        {
            //clear screen
            oled.fillScreen(0);

            //dim screen
            uint8_t contrast = 0x00;
            oled.sendCommand(0xC7, &contrast, 1);
        }
        else if (states[state].variant == 1)
        {
            //settings clear mode
            if (!state_init)
            {
                oled.setCursor(0,10);
                oled.setFont(&SourceSansPro_Regular6pt7b);
                oled.setTextColor(WHITE);
                oled.print("Do you want to clear\nall saved settings?");
            }

            if (dpad_up_active() || dpad_down_active())
            {
                selected_option = !selected_option;
            }

            if (dpad_any_active() || !state_init)
            {
                drawMenu(2, 37, SCREEN_WIDTH - 4, SCREEN_HEIGHT - 37, {"No", "Yes"}, selected_option, RED);
            }

            if (dpad_enter_active())
            {
                if (selected_option == 1) preferences.clear();
                switchState(1);
            }
        }

        //check down button for settings clearing thing
        if (states[state].variant == 0)
        {
            if (digitalRead(dpad_down))
            {
                switchState(state, 1);
            }
            else
            {
                //switch state
                switchState(1);
            }
        }
    }, true);

    //state 1
    registerState("Watch Face", "watch", [](){

        //this state is responsible for drawing the watch face
        //it is split up into several "elements", which include the hour and minute,
        //the second, the meridian (am or pm), and the date.
        //elements are only drawn to the screen if they have changed.
        //for example, the second is only drawn if the current second is different
        //to the second of the last clock cycle
        //this reduces flickering and is probably more efficient(?).
        //when an element is redrawn, a black square is drawn over the text (clearing it),
        //then the new text is drawn over it

        //current time
        time_t tim = now();
        //struct tm *current_time = gmtime(&tim);

        //state-specific variables
        //the last_x variables are used to keep track of when an element has changed
        //the last_x variables are initilised to -1 so that they change when the state is first loaded
        static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH-1, 42);            //time canvas (used to draw time w/ a gradient)
        static char text_aaaa[150];                                                     //buffer for sprintf
        String meridian = "owo";                                                        //default value for meridian
        static int time_x = 0;                                                          //x pos of time canvas (and seconds element)
        static int16_t x1, y1;                                                          //x and y positions for the black square that clears text
        static uint16_t w_min_hour=0, w_sec=0, w_meridian=0, w_datestring=0, h=0;       //width and height of the black square
        static int last_second = second(tim)-1;
        static int last_minute = minute(tim)-1;
        static int last_hour = hour(tim)-1;
        static int last_day = day(tim)-1;
        static int last_meridian = -1; //1 - am, 0 - pm

        //draw introduction message
        oled.setCursor(2, 20);
        oled.setTextColor(WHITE, BLACK);

        oled.setTextSize(1);
        //oled.print("The time right now is");

        //draw minutes and hours
        if ((last_minute != minute(tim)) || !state_init)
        {
            canvas_time->setFont(&SourceSansPro_Regular24pt7b);
            canvas_time->setCursor(0, 32);

            sprintf(text_aaaa, "%02d:%02d", last_hour, last_minute);
            canvas_time->getTextBounds(text_aaaa, 0, 32, &x1, &y1, &w_min_hour, &h);
            canvas_time->fillRect(x1, y1, w_min_hour, h, BLACK);
            //oled.fillRect(x1, y1, w, h, BLACK);
            time_x = (int)((SCREEN_WIDTH - w_min_hour) / 2);

            canvas_time->printf("%02d:%02d", hour(tim), minute(tim));

            last_minute = minute(tim);
            last_hour = hour(tim);
        }
        oled.drawRainbowBitmap(time_x - 4, 6, canvas_time->getBuffer(), SCREEN_WIDTH, 38, BLACK, ((millis() % 60000) * 6)/1000);

        //draw seconds
        oled.setFont(&SourceSansPro_Light16pt7b);
        oled.setCursor(time_x, 64);

        if ((last_second != second(tim)) || !state_init)
        {
            sprintf(text_aaaa, "%02d", last_second);
            oled.getTextBounds(text_aaaa, time_x, 64, &x1, &y1, &w_sec, &h);
            oled.fillRect(x1, y1, w_sec, h, BLACK);

            oled.printf("%02d", second(tim));
            last_second = second(tim);
        }

        //draw meridian
        if ((last_meridian != isAM(tim)) || !state_init)
        {
            String meridian = isAM(tim) ? "am" : "pm";
            sprintf(text_aaaa, "%s", meridian.c_str());

            //because the x pos of this element is dependant on its width,
            //we have to use getTextBounds to first calculate the width of the text,
            //then we have to use getTextBounds again to calculate the actual x and y
            //coordinates of the text
            oled.getTextBounds(text_aaaa, time_x, 64, &x1, &y1, &w_meridian, &h);
            int meridian_x = (time_x + w_min_hour) - w_meridian;
            oled.getTextBounds(text_aaaa, meridian_x, 64, &x1, &y1, &w_meridian, &h);

            oled.fillRect(x1, y1, w_meridian, h, BLACK);

            oled.setCursor(meridian_x, 64);
            oled.print(meridian);

            last_meridian = isAM(tim);
        }

        //draw date
        oled.setFont(&SourceSansPro_Light8pt7b);

        if ((last_day != day(tim)) || !state_init)
        {
            String datestring = String(day(tim)) + String(" ") + String(monthStr(month(tim))) + String(" ") + String(year(tim));
            sprintf(text_aaaa, "%s", datestring.c_str());
            oled.getTextBounds(text_aaaa, time_x, 64, &x1, &y1, &w_datestring, &h);
            oled.fillRect(0, 90, SCREEN_WIDTH, 6, BLACK);

            oled.setCursor((SCREEN_WIDTH - w_datestring) / 2, 90);
            oled.print(datestring);

            last_day = day(tim);
        }

        //finish drawing state
        //drawTopThing();
        oled.setFont(&SourceSansPro_Regular6pt7b);

        //check buttons
        if (dpad_enter_active())
        {
            deepSleep(31);
        }

        if (dpad_right_active())
        {
            switchState(2);
        }

    });

    //state 2
    registerState("State Menu", "menu", [](){

        static int columns = 4;
        static int icon_size = 27;
        static int icon_spacing = 4;
        static int no_icons = 0;
        static std::vector<std::map<int, stateMeta>::iterator> menu_positions;

        int icon_xpos = icon_spacing;
        int icon_ypos = icon_spacing;

        if (!state_init)
        {
            no_icons = 0;
            menu_positions.clear();
        }

        if (dpad_left_active())
        {
            if (selected_menu_icon == states.begin())
            {
                selected_menu_icon = states.end();
                digitalWrite(12, HIGH);
            }
            while(1)
            {
                selected_menu_icon--;
                if (!selected_menu_icon->second.hidden) break;
            }
        }

        if (dpad_right_active())
        {
            while(1)
            {
                selected_menu_icon++;
                if (selected_menu_icon == states.end())
                selected_menu_icon = states.begin();
                if (!selected_menu_icon->second.hidden) break;
            }
        }

        if (dpad_up_active())
        {
            //get selected icon number
            int loop_limit = columns;
            std::vector<std::map<int, stateMeta>::iterator>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), selected_menu_icon);
            if (selected_pos != menu_positions.end())
            {
                int index = std::distance(menu_positions.begin(), selected_pos) ;
                int last = no_icons % columns;
                if (index < columns)
                {
                    if (index < last) loop_limit = last;
                    else loop_limit = last + columns;
                }
                //oled.printf("%d", loop_limit);
            }

            for (int i=0; i < loop_limit; i++)
            {
                if (selected_menu_icon == states.begin())
                {
                    selected_menu_icon = states.end();
                    digitalWrite(12, HIGH);
                }
                while(1)
                {
                    selected_menu_icon--;
                    if (!selected_menu_icon->second.hidden) break;
                }
            }
        }

        if (dpad_down_active())
        {
            //get selected icon number
            int loop_limit = columns;
            std::vector<std::map<int, stateMeta>::iterator>::iterator selected_pos = std::find(menu_positions.begin(), menu_positions.end(), selected_menu_icon);
            if (selected_pos != menu_positions.end())
            {
                int index = std::distance(menu_positions.begin(), selected_pos) ;
                int last = no_icons % columns;
                if (index + 1 > ( no_icons - 4 ))
                {
                    if (index + 1 > ( no_icons - last) ) loop_limit = last;
                    else loop_limit = last + columns;
                }
                //oled.printf("%d", loop_limit);
            }

            for (int i=0; i < loop_limit; i++)
            {
                while(1)
                {
                    selected_menu_icon++;
                    if (selected_menu_icon == states.end())
                    selected_menu_icon = states.begin();
                    if (!selected_menu_icon->second.hidden) break;
                }
            }
        }



        if (dpad_up_active() || dpad_down_active() || dpad_left_active() ||
            dpad_right_active() || dpad_enter_active() || !state_init)
        {
            oled.fillRect(0, 86, SCREEN_WIDTH, 10, BLACK); //clear state name text
            icon_ypos += 10;                               //add space for top bar thing
            //draw state icons
            for (std::map<int, stateMeta>::iterator it = states.begin(); it != states.end(); it++)
            {
                stateMeta stateinfo = it->second;
                if (!stateinfo.hidden)
                {
                    oled.drawRGBBitmap(icon_xpos, icon_ypos, icons[stateinfo.stateIcon].data(),
                                       icon_size, icon_size);

                    if (selected_menu_icon == it)
                    {
                        oled.drawRect(icon_xpos-1, icon_ypos-1, icon_size+1, icon_size+1, themecolour);
                        oled.setCursor(2, 94);
                        oled.setTextColor(WHITE);
                        oled.print(stateinfo.stateName.c_str());
                    }
                    else oled.drawRect(icon_xpos-1, icon_ypos-1, icon_size+1, icon_size+1, BLACK);

                    icon_xpos += icon_size + icon_spacing;
                    if ((icon_xpos+icon_size) > SCREEN_WIDTH)
                    {
                        icon_xpos = icon_spacing;
                        icon_ypos += icon_size + icon_spacing;
                    }

                    if (!state_init)
                    {
                        menu_positions.push_back(it);
                    }

                    if (!state_init) no_icons++;
                }
            }
        }

        drawTopThing();

        if (dpad_enter_active())
        {
            switchState(selected_menu_icon->first);
        }

    }, true);

    //state 3
    registerState("Settings", "settings", [](){

        static std::vector<String> panels = {"Time and Date", "Timeout", "Colour", "About"};
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

    });



    //state 4
    registerState("Regret", "what", [](){

        if (!state_init) oled.drawRGBBitmap(0, 0, regret, 128, 96);
        if (dpad_any_active()) switchState(2);

    });

}
