void registerTimeStates()
{

    registerState("Stopwatch", "stopwatch", [](){

        //stopwatch variables are accessed by the main menu,
        //so they are global, and are declared in watch2.ino
        //they are duplicated below for reference
        /*
        int stopwatch_timing = 0;                                       //stopwatch state
                                                                        //0 - stopped
                                                                        //1 - running
                                                                        //2 - paused
        uint32_t stopwatch_epoch = 0;                                   //time when the stopwatch was started
        uint32_t stopwatch_paused_diff = 0;                             //when the stopwatch is off or paused, stopwatch_epoch is set to the current time minus this value
                                                                        //when the stopwatch is paused, this value is set to the difference between stopwatch_epoch and the current time (keeps the difference constant)
                                                                        //when the stopwatch is off, this value is set to 0
        uint32_t stopwatch_time_diff = 0;                               //difference between epoch and current time (equivalent to elapsed time)
        uint32_t stopwatch_last_time_diff = 0;                          //last time diff (used for checking when to redraw elapsed time)
        uint32_t stopwatch_ms = 0, stopwatch_last_ms = 0;
        uint32_t stopwatch_s = 0, stopwatch_last_s = 0;
        uint32_t stopwatch_min = 0, stopwatch_last_min = 0;
        uint32_t stopwatch_hour = 0, stopwatch_last_hour = 0;
        */
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static uint16_t width_two_digits = 0, width_two_digits_colon = 0, width_three_digits_colon = 0;

        //code to execute while in a stopwatch state
        switch(stopwatch_timing)
        {

            case 0: //stopped

                stopwatch_epoch = millis();
                stopwatch_paused_diff = 0;
                break;

            case 1: //running

                //idk
                break;

            case 2: //paused

                stopwatch_epoch = millis() - stopwatch_paused_diff;
                break;

        }

        //code to execute when entering a stopwatch state
        if (dpad_enter_active())
        {
            switch(stopwatch_timing)
            {
                case 0: //stopped to running

                    stopwatch_epoch = millis();
                    stopwatch_timing = 1;
                    break;

                case 1: //running to paused

                    stopwatch_paused_diff = millis() - stopwatch_epoch;
                    stopwatch_timing = 2;
                    break;

                case 2: //paused to running

                    stopwatch_timing = 1;
                    break;
            }
        }

        //stop timing
        if (dpad_down_active())
        {
            stopwatch_timing = 0;
        }

        //draw elapsed time
        stopwatch_time_diff = millis() - stopwatch_epoch;
        stopwatch_ms = stopwatch_time_diff % 1000;
        stopwatch_s    = (int) (stopwatch_time_diff / 1000)             % 60;
        stopwatch_min  = (int)((stopwatch_time_diff / (1000 * 60))      % 60);
        stopwatch_hour = (int)((stopwatch_time_diff / (1000 * 60 * 60)) % 24);

        oled.setFont(&SourceSansPro_Light12pt7b);
        oled.setTextColor(themecolour);

        //calculate sizes of stopwatch digit things
        if (!state_init)
        {
            oled.getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
            oled.getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
            oled.getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);
        }

        //draw hours
        if ((stopwatch_hour != stopwatch_last_hour) || !state_init)
        {
            oled.setCursor(2, 40);
            sprintf(text_aaaa, "%02d", stopwatch_last_hour);
            oled.getTextBounds(String(text_aaaa), 2, 40, &x1, &y1, &w, &h);
            oled.fillRect(x1, y1, w, h, BLACK);
            oled.printf("%02d", stopwatch_hour);
            stopwatch_last_hour = stopwatch_hour;
        }

        //draw minutes
        if ((stopwatch_min != stopwatch_last_min) || !state_init)
        {
            oled.setCursor(2 + (width_two_digits), 40);
            sprintf(text_aaaa, ":%02d", stopwatch_last_min);
            oled.getTextBounds(text_aaaa, 2 + (width_two_digits), 40, &x1, &y1, &w, &h);
            oled.fillRect(x1, y1, w, h, BLACK);
            oled.printf(":%02d", stopwatch_min);
            stopwatch_last_min = stopwatch_min;
        }

        //draw seconds
        if ((stopwatch_s != stopwatch_last_s) || !state_init)
        {
            oled.setCursor(2 + (2 * width_two_digits_colon), 40);
            sprintf(text_aaaa, ":%02d", stopwatch_last_s);
            oled.getTextBounds(text_aaaa, 2 + (2 * width_two_digits_colon), 40, &x1, &y1, &w, &h);
            oled.fillRect(x1, y1, w, h, BLACK);
            oled.printf(":%02d", stopwatch_s);
            stopwatch_last_s = stopwatch_s;
        }

        //draw milliseconds
        if ((stopwatch_ms != stopwatch_last_ms) || !state_init)
        {
            oled.setCursor(2 + (3 * width_two_digits_colon), 40);
            sprintf(text_aaaa, ":%03d", stopwatch_last_ms);
            oled.getTextBounds(text_aaaa, 2 + (3 * width_two_digits_colon), 40, &x1, &y1, &w, &h);
            oled.fillRect(x1, y1, w, h, BLACK);
            oled.printf(":%03d", stopwatch_ms);
            stopwatch_last_ms = stopwatch_ms;
        }

         // reset font
        oled.setFont(&SourceSansPro_Light8pt7b);
        oled.setTextColor(WHITE);

        //draw status text
        if (dpad_any_active() || !state_init)
        {
            oled.setFont(&SourceSansPro_Regular6pt7b);
            oled.setCursor(2, 20);
            oled.fillRect(2, 12, SCREEN_WIDTH - 2, 11, BLACK);
            if (stopwatch_timing == 0) oled.print("Stopped");
            if (stopwatch_timing == 1) oled.print("Timing");
            if (stopwatch_timing == 2) oled.print("Paused");
        }

        //draw top thing
        drawTopThing();

        //if left pressed, go back to state menu
        if (dpad_left_active())
        {
            switchState(2);
        }

    });



    registerState("Timer", "timer", [](){

        static int selected_timer = -1;
        static int selected_button = 3;
        static int last_selected_timer = -2;
        //0 - hours
        //1 - minutes
        //2 - seconds
        //3 - play / pause (or back if on timer -1)
        //4 - delete (or add if on timer -1)
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static uint16_t width_two_digits = 0, width_two_digits_colon = 0, width_three_digits_colon = 0;

        if (!state_init)
        {
            oled.setFont(&SourceSansPro_Regular6pt7b);

            //calculate sizes of digit things
            oled.getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
            oled.getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
            oled.getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);

        }

        if (dpad_left_active())
        {
            selected_button--;
            if (selected_button < 0) selected_button = 4;
            if (selected_timer == -1 && selected_button < 3) selected_button = 4;
        }

        if (dpad_right_active())
        {
            selected_button++;
            if (selected_button > 4) selected_button = 0;
            if (selected_timer == -1 && selected_button < 3) selected_button = 3;
        }

        if (dpad_down_active())
        {
            if (selected_timer == -1 || selected_button == 3 || selected_button == 4)
            {
                selected_timer++;
                if (selected_timer >= timers.size()) selected_timer = -1;
            }
            else //decrease time
            {
                if (floor(timers[selected_timer].duration / 3600) > 0)
                if (selected_button == 0) timers[selected_timer].duration -= 3600;      //hours
                if (floor(timers[selected_timer].duration % 3600 / 60) > 0)
                if (selected_button == 1) timers[selected_timer].duration -= 60;        //minutes
                if (floor(timers[selected_timer].duration % 3600 % 60) > 0)
                if (selected_button == 2) timers[selected_timer].duration -= 1;         //seconds
                if (timers[selected_timer].alarm_id != 255) Alarm.write(timers[selected_timer].alarm_id, timers[selected_timer].duration); //reset timer with new duration
                else timers[selected_timer].initial_duration = timers[selected_timer].duration;
            }
        }

        if (dpad_up_active())
        {
            if (selected_timer == -1 || selected_button == 3 || selected_button == 4)
            {
                selected_timer--;
                if (selected_timer < -1) selected_timer = timers.size() - 1;
            }
            else //increase time
            {
                if (selected_button == 0) timers[selected_timer].duration += 3600;      //hours
                if (selected_button == 1) timers[selected_timer].duration += 60;        //minutes
                if (selected_button == 2) timers[selected_timer].duration += 1;         //seconds
                if (timers[selected_timer].alarm_id != 255) Alarm.write(timers[selected_timer].alarm_id, timers[selected_timer].duration); //reset timer with new duration
                else timers[selected_timer].initial_duration = timers[selected_timer].duration;
            }
        }

        if (dpad_enter_active())
        {
            if (selected_timer == -1)
            {
                //add new timer
                if (selected_button == 4)
                {
                    timers.push_back({
                        30,     //duration in seconds
                        30,     //initial duration (used when a timer ends)
                        255,    //alarm id (255 means that no TimeAlarms timer has been created)
                        -1,     //time started
                        [](){   //function to execute once timer has ended
                            AlarmID_t id = Alarm.getTriggeredAlarmId();
                            int timer_id = -1;
                            for (int i = 0; i < timers.size(); i++)
                            {
                                if (timers[i].alarm_id == id)
                                {
                                    timer_id = i;
                                }
                            }
                            timers[timer_id].alarm_id = 255;
                            timers[timer_id].time_started = -1;
                            timers[timer_id].duration = timers[timer_id].initial_duration;
                            timer_trigger_status = 1;
                            timer_trigger_id = timer_id;
                        },
                        -1      //last value (used for redrawing remaining time)
                    });
                }
            }
            else
            {
                if (selected_button == 3) // play / pause timer
                {
                    if (timers[selected_timer].alarm_id == 255) //start timer
                    {
                        //create a TimeAlarms timer
                        AlarmID_t id = Alarm.timerOnce(
                            timers[selected_timer].duration,             //duration in seconds
                            timers[selected_timer].on_tick_handler       //function to call after time has elapsed
                        );

                        //store timer metadata
                        timers[selected_timer].alarm_id = id;
                        timers[selected_timer].time_started = now();
                    }
                    else                                      //pause timer
                    {
                        //store time remaining
                        time_t time_remaining = ( timers[selected_timer].time_started + Alarm.read(timers[selected_timer].alarm_id ) ) - now();
                        timers[selected_timer].duration = time_remaining;

                        //free alarm id (essentially destroying the TimeAlarm timer) and reset stored alarm id and time started
                        Alarm.free(timers[selected_timer].alarm_id);
                        timers[selected_timer].alarm_id = 255;
                        timers[selected_timer].time_started = -1;
                    }
                }
                if (selected_button == 4) // remove timer
                {
                    //store selected timer
                    AlarmID_t whee = selected_timer;

                    //change selecte timer to previous timer
                    selected_timer--;

                    //free alarm id
                    Alarm.free(timers[whee].alarm_id);

                    //remove timer from timers
                    timers.erase(timers.begin() + whee);

                    //clear screen
                    oled.fillScreen(BLACK);
                }
            }
        }

        //draw timers
        if (1)
        {
            int timer_x = 2;
            int timer_y = 14;
            int timer_w = SCREEN_WIDTH - (timer_x * 2);
            int timer_h = SCREEN_HEIGHT - timer_y;
            int icon_radius = 3;
            int radius = 4;
            int icon_spacing = 2;
            int icon_size = 8;
            int timer_edge = timer_x + timer_w;
            uint16_t working_button_colour = WHITE;
            uint16_t working_timer_colour = WHITE;

            if (!state_init || dpad_any_active())
            {

                //set colour of row
                working_timer_colour = (selected_timer == -1) ? themecolour : WHITE;

                //draw header
                oled.setTextColor(working_timer_colour);
                oled.setCursor(timer_x, timer_y + 8);
                oled.print("Timers");

                //set colour of back button
                working_button_colour = (selected_button <= 3 && selected_timer == -1) ? themecolour : WHITE;

                //draw back button
                oled.drawRoundRect(
                    timer_edge - (icon_size * 2) - (icon_spacing * 6),
                    timer_y - icon_spacing,
                    icon_size + (2 * icon_spacing),
                    icon_size + (2 * icon_spacing),
                    icon_radius,
                    working_button_colour
                );
                oled.drawBitmap(
                    timer_edge - (icon_size * 2) - (icon_spacing * 5),
                    timer_y,
                    small_icons["back"].data(),
                    icon_size,
                    icon_size,
                    working_timer_colour
                );

                //set colour of add button
                working_button_colour = (selected_button == 4 && selected_timer == -1) ? themecolour : WHITE;

                //draw add button
                oled.drawRoundRect(
                    timer_edge - icon_size - (icon_spacing * 3),
                    timer_y - icon_spacing,
                    icon_size + (2 * icon_spacing),
                    icon_size + (2 * icon_spacing),
                    icon_radius,
                    working_button_colour
                );
                oled.drawBitmap(
                    timer_edge - icon_size - (icon_spacing * 2),
                    timer_y,
                    small_icons["add"].data(),
                    icon_size,
                    icon_size,
                    working_timer_colour
                );
            }

            //draw timers
            for (int i = 0; i < timers.size(); i++)
            {
                timer_y += 8 + (icon_spacing * 4);

                time_t current_time = ( timers[i].time_started + Alarm.read(timers[i].alarm_id) ) - now();
                //if this is the state's first frame, or the time of the timer has changed,
                //or this is the selected timer and any button has been pressed,
                //or this is the previouskly selected timer and any button has been pressed
                if (!state_init || current_time != timers[i].last_value || (dpad_any_active() && (selected_timer == i || last_selected_timer == i)))
                {
                    //clear previous text
                    oled.fillRect(timer_x, timer_y, timer_w - (icon_spacing * 6) - (icon_size * 2), 9, BLACK);

                    //set colour of row
                    working_timer_colour = (selected_timer == i) ? themecolour : WHITE;

                    //get time to display
                    time_t time_left = (timers[i].alarm_id == 255) ? timers[i].duration : ( timers[i].time_started + Alarm.read(timers[i].alarm_id ) ) - now();
                    time_t time_left_hrs = floor(time_left / 3600);
                    time_t time_left_min = floor(time_left % 3600 / 60);
                    time_t time_left_sec = floor(time_left % 3600 % 60);

                    /*
                    oled.getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
                    oled.getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
                    oled.getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);
                    */

                    //print hours
                    oled.setCursor(timer_x, timer_y + 8);
                    oled.setTextColor(working_timer_colour);
                    oled.printf("%02d", time_left_hrs);
                    working_button_colour = (selected_button == 0 && selected_timer == i) ? themecolour : BLACK;
                    oled.drawRoundRect(timer_x - 1, timer_y - 1, width_two_digits + 4, 12, icon_radius, working_button_colour);

                    //print minutes
                    oled.setCursor(timer_x + (width_two_digits) + 4, timer_y + 8);
                    oled.printf(":%02d", time_left_min);
                    working_button_colour = (selected_button == 1 && selected_timer == i) ? themecolour : BLACK;
                    oled.drawRoundRect(timer_x + (width_two_digits) + 4 - 1, timer_y - 1, width_two_digits_colon + 5, 12, icon_radius, working_button_colour);

                    //print seconds
                    oled.setCursor(timer_x + width_two_digits + width_two_digits_colon + 2 + 7, timer_y + 8);
                    oled.printf(": %02d", time_left_sec);
                    working_button_colour = (selected_button == 2 && selected_timer == i) ? themecolour : BLACK;
                    oled.drawRoundRect(timer_x + width_two_digits + width_two_digits_colon + 2 + 7 - 1, timer_y - 1, width_two_digits_colon + 5, 12, icon_radius, working_button_colour);

                    //set colour of play / pause button
                    working_button_colour = (selected_button == 3 && selected_timer == i) ? themecolour : WHITE;

                    //draw play / pause button
                    oled.drawRoundRect(
                        timer_edge - (icon_size * 2) - (icon_spacing * 6),
                        timer_y - icon_spacing,
                        icon_size + (2 * icon_spacing),
                        icon_size + (2 * icon_spacing),
                        icon_radius,
                        working_button_colour
                    );
                    oled.fillRect(
                        timer_edge - (icon_size * 2) - (icon_spacing * 5),
                        timer_y,
                        icon_size,
                        icon_size,
                        BLACK
                    );
                    oled.drawBitmap(
                        timer_edge - (icon_size * 2) - (icon_spacing * 5),
                        timer_y,
                        (timers[i].alarm_id == 255) ? small_icons["play"].data() : small_icons["pause"].data(),
                        icon_size,
                        icon_size,
                        working_timer_colour
                    );

                    //set colour of delete button
                    working_button_colour = (selected_button == 4 && selected_timer == i) ? themecolour : WHITE;

                    //draw delete button
                    oled.drawRoundRect(
                        timer_edge - icon_size - (icon_spacing * 3),
                        timer_y - icon_spacing,
                        icon_size + (2 * icon_spacing),
                        icon_size + (2 * icon_spacing),
                        icon_radius,
                        working_button_colour
                    );
                    oled.fillRect(
                        timer_edge - icon_size - (icon_spacing * 2),
                        timer_y,
                        icon_size,
                        icon_size,
                        BLACK
                    );
                    oled.drawBitmap(
                        timer_edge - icon_size - (icon_spacing * 2),
                        timer_y,
                        small_icons["x"].data(),
                        icon_size,
                        icon_size,
                        working_timer_colour
                    );

                    timers[i].last_value = ( timers[i].time_started + Alarm.read(timers[i].alarm_id) ) - now();

                }

            }

        }

        drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_timer == -1)
            {
                //go back to state menu
                if (selected_button < 4) switchState(2);
            }
        }

        last_selected_timer = selected_timer;

    });



    registerState("Alarms", "alarms", [](){

        static int selected_alarm = -1;
        static int selected_button = 3;
        static int last_selected_alarm = -2;
        //0 - hours
        //1 - minutes
        //2 - seconds
        //3 - play / pause (or back if on timer -1)
        //4 - delete (or add if on timer -1)
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static uint16_t width_two_digits = 0, width_two_digits_colon = 0, width_three_digits_colon = 0;

        if (!state_init)
        {
            oled.setFont(&SourceSansPro_Regular6pt7b);

            //calculate sizes of digit things
            oled.getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
            oled.getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
            oled.getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);

        }

        if (dpad_left_active())
        {
            selected_button--;
            if (selected_button < 0) selected_button = 4;
            if (selected_alarm == -1 && selected_button < 3) selected_button = 4;
        }

        if (dpad_right_active())
        {
            selected_button++;
            if (selected_button > 4) selected_button = 0;
            if (selected_alarm == -1 && selected_button < 3) selected_button = 3;
        }

        if (dpad_down_active())
        {
            if (selected_alarm == -1 || selected_button == 3 || selected_button == 4)
            {
                selected_alarm++;
                if (selected_alarm >= alarms.size()) selected_alarm = -1;
            }
            else //decrease time
            {
                time_t alarm_time = Alarm.read(alarms[selected_alarm].alarm_id);
                if (floor(alarm_time / 3600) > 0)
                if (selected_button == 0) alarm_time -= 3600;      //hours
                if (floor(alarm_time % 3600 / 60) > 0)
                if (selected_button == 1) alarm_time -= 60;        //minutes
                if (floor(alarm_time % 3600 % 60) > 0)
                if (selected_button == 2) alarm_time -= 1;         //seconds
                Alarm.write(alarms[selected_alarm].alarm_id, alarm_time);
                if (alarms[selected_alarm].paused) Alarm.disable(alarms[selected_alarm].alarm_id);
                alarms[selected_alarm].initial_time = alarm_time;

            }
        }

        if (dpad_up_active())
        {
            if (selected_alarm == -1 || selected_button == 3 || selected_button == 4)
            {
                selected_alarm--;
                if (selected_alarm < -1) selected_alarm = alarms.size() - 1;
            }
            else //increase time
            {
                time_t alarm_time = Alarm.read(alarms[selected_alarm].alarm_id);
                if (selected_button == 0) alarm_time += 3600;      //hours
                if (selected_button == 1) alarm_time += 60;        //minutes
                if (selected_button == 2) alarm_time += 1;         //seconds
                Alarm.write(alarms[selected_alarm].alarm_id, alarm_time);
                if (alarms[selected_alarm].paused) Alarm.disable(alarms[selected_alarm].alarm_id);
                alarms[selected_alarm].initial_time = alarm_time;
            }
        }

        if (dpad_enter_active())
        {
            if (selected_alarm == -1)
            {
                //add new alarm
                if (selected_button == 4)
                {
                    AlarmID_t alarm_id_aaa = Alarm.alarmRepeat(
                        hour(),
                        minute(),
                        second(),
                        [](){   //function to execute once alarm time has been idk

                            AlarmID_t id = Alarm.getTriggeredAlarmId();
                            int alarm_id = -1;
                            for (int i = 0; i < alarms.size(); i++)
                            {
                                if (alarms[i].alarm_id == id)
                                {
                                    alarm_id = i;
                                }
                            }
                            alarm_trigger_status = 1;
                            alarm_trigger_id = alarm_id;

                        });
                    Alarm.disable(alarm_id_aaa);
                    alarms.push_back({
                        alarm_id_aaa,   //alarm id
                        true,           //paused
                        -1              //last value
                    });
                }
            }
            else
            {
                if (selected_button == 3) // play / pause alarm
                {
                    if (alarms[selected_alarm].paused)
                    {
                        Alarm.enable(alarms[selected_alarm].alarm_id);
                        alarms[selected_alarm].paused = false;
                    }
                    else
                    {
                        Alarm.disable(alarms[selected_alarm].alarm_id);
                        alarms[selected_alarm].paused = true;
                    }
                }
                if (selected_button == 4) // remove alarm
                {
                    //store selected timer
                    AlarmID_t whee = selected_alarm;

                    //change selecte timer to previous timer
                    selected_alarm--;

                    //free alarm id
                    Alarm.free(alarms[whee].alarm_id);

                    //remove timer from timers
                    alarms.erase(alarms.begin() + whee);

                    //clear screen
                    oled.fillScreen(BLACK);
                }
            }
        }

        //draw alarms
        if (1)
        {
            int alarm_x = 2;
            int alarm_y = 14;
            int alarm_w = SCREEN_WIDTH - (alarm_x * 2);
            int alarm_h = SCREEN_HEIGHT - alarm_y;
            int icon_radius = 3;
            int radius = 4;
            int icon_spacing = 2;
            int icon_size = 8;
            int alarm_edge = alarm_x + alarm_w;
            uint16_t working_button_colour = WHITE;
            uint16_t working_alarm_colour = WHITE;

            if (!state_init || dpad_any_active())
            {

                //set colour of row
                working_alarm_colour = (selected_alarm == -1) ? themecolour : WHITE;

                //draw header
                oled.setTextColor(working_alarm_colour);
                oled.setCursor(alarm_x, alarm_y + 8);
                oled.print("Alarms");

                //set colour of back button
                working_button_colour = (selected_button <= 3 && selected_alarm == -1) ? themecolour : WHITE;

                //draw back button
                oled.drawRoundRect(
                    alarm_edge - (icon_size * 2) - (icon_spacing * 6),
                    alarm_y - icon_spacing,
                    icon_size + (2 * icon_spacing),
                    icon_size + (2 * icon_spacing),
                    icon_radius,
                    working_button_colour
                );
                oled.drawBitmap(
                    alarm_edge - (icon_size * 2) - (icon_spacing * 5),
                    alarm_y,
                    small_icons["back"].data(),
                    icon_size,
                    icon_size,
                    working_alarm_colour
                );

                //set colour of add button
                working_button_colour = (selected_button == 4 && selected_alarm == -1) ? themecolour : WHITE;

                //draw add button
                oled.drawRoundRect(
                    alarm_edge - icon_size - (icon_spacing * 3),
                    alarm_y - icon_spacing,
                    icon_size + (2 * icon_spacing),
                    icon_size + (2 * icon_spacing),
                    icon_radius,
                    working_button_colour
                );
                oled.drawBitmap(
                    alarm_edge - icon_size - (icon_spacing * 2),
                    alarm_y,
                    small_icons["add"].data(),
                    icon_size,
                    icon_size,
                    working_alarm_colour
                );
            }

            //draw alarms
            if (alarms.size() == 0)
            {
                alarm_y += 8 + (icon_spacing * 4);
                oled.setTextColor(working_alarm_colour);
                oled.setCursor(alarm_x, alarm_y + 8);
                oled.print("No alarms and ");
                alarm_y += 8 + (icon_spacing * 4);
                oled.setCursor(alarm_x, alarm_y + 8);
                oled.print("no suprises");
            }

            for (int i = 0; i < alarms.size(); i++)
            {
                alarm_y += 8 + (icon_spacing * 4);

                time_t alarm_time = Alarm.read(alarms[i].alarm_id);
                //if this is the state's first frame, or the time of the alarm has changed,
                //or this is the selected alarm and any button has been pressed,
                //or this is the previouskly selected alarm and any button has been pressed
                if (!state_init || alarm_time != alarms[i].last_value || (dpad_any_active() && (selected_alarm == i || last_selected_alarm == i)))
                {
                    //clear previous text
                    int wheeoo = (i == 0 && alarms.size() == 1) ?
                        ( SCREEN_WIDTH - alarm_x) :
                        ( alarm_w - (icon_spacing * 6) - (icon_size * 2) );
                    int other_wheoo = (i == 0 && alarms.size() == 1) ? (20 + (icon_spacing * 4)) : 9;
                    oled.fillRect(alarm_x, alarm_y, wheeoo, other_wheoo , BLACK);

                    //set colour of row
                    working_alarm_colour = (selected_alarm == i) ? themecolour : WHITE;

                    //get time to display
                    time_t alarm_time = Alarm.read(alarms[i].alarm_id);
                    time_t alarm_time_hrs = floor(alarm_time / 3600);
                    time_t alarm_time_min = floor(alarm_time % 3600 / 60);
                    time_t alarm_time_sec = floor(alarm_time % 3600 % 60);

                    /*
                    oled.getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
                    oled.getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
                    oled.getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);
                    */

                    //print hours
                    oled.setCursor(alarm_x, alarm_y + 8);
                    oled.setTextColor(working_alarm_colour);
                    oled.printf("%02d", alarm_time_hrs);
                    working_button_colour = (selected_button == 0 && selected_alarm == i) ? themecolour : BLACK;
                    oled.drawRoundRect(alarm_x - 1, alarm_y - 1, width_two_digits + 4, 12, icon_radius, working_button_colour);

                    //print minutes
                    oled.setCursor(alarm_x + (width_two_digits) + 4, alarm_y + 8);
                    oled.printf(":%02d", alarm_time_min);
                    working_button_colour = (selected_button == 1 && selected_alarm == i) ? themecolour : BLACK;
                    oled.drawRoundRect(alarm_x + (width_two_digits) + 4 - 1, alarm_y - 1, width_two_digits_colon + 5, 12, icon_radius, working_button_colour);

                    //print seconds
                    oled.setCursor(alarm_x + width_two_digits + width_two_digits_colon + 2 + 7, alarm_y + 8);
                    oled.printf(": %02d", alarm_time_sec);
                    working_button_colour = (selected_button == 2 && selected_alarm == i) ? themecolour : BLACK;
                    oled.drawRoundRect(alarm_x + width_two_digits + width_two_digits_colon + 2 + 7 - 1, alarm_y - 1, width_two_digits_colon + 5, 12, icon_radius, working_button_colour);

                    //set colour of play / pause button
                    working_button_colour = (selected_button == 3 && selected_alarm == i) ? themecolour : WHITE;

                    //draw play / pause button
                    oled.drawRoundRect(
                        alarm_edge - (icon_size * 2) - (icon_spacing * 6),
                        alarm_y - icon_spacing,
                        icon_size + (2 * icon_spacing),
                        icon_size + (2 * icon_spacing),
                        icon_radius,
                        working_button_colour
                    );
                    oled.fillRect(
                        alarm_edge - (icon_size * 2) - (icon_spacing * 5),
                        alarm_y,
                        icon_size,
                        icon_size,
                        BLACK
                    );
                    oled.drawBitmap(
                        alarm_edge - (icon_size * 2) - (icon_spacing * 5),
                        alarm_y,
                        (alarms[i].paused) ? small_icons["play"].data() : small_icons["pause"].data(),
                        icon_size,
                        icon_size,
                        working_alarm_colour
                    );

                    //set colour of delete button
                    working_button_colour = (selected_button == 4 && selected_alarm == i) ? themecolour : WHITE;

                    //draw delete button
                    oled.drawRoundRect(
                        alarm_edge - icon_size - (icon_spacing * 3),
                        alarm_y - icon_spacing,
                        icon_size + (2 * icon_spacing),
                        icon_size + (2 * icon_spacing),
                        icon_radius,
                        working_button_colour
                    );
                    oled.fillRect(
                        alarm_edge - icon_size - (icon_spacing * 2),
                        alarm_y,
                        icon_size,
                        icon_size,
                        BLACK
                    );
                    oled.drawBitmap(
                        alarm_edge - icon_size - (icon_spacing * 2),
                        alarm_y,
                        small_icons["x"].data(),
                        icon_size,
                        icon_size,
                        working_alarm_colour
                    );

                    alarms[i].last_value = alarm_time;

                }

            }

        }

        drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_alarm == -1)
            {
                //go back to state menu
                if (selected_button < 4) switchState(2);
            }
        }

        last_selected_alarm = selected_alarm;

    });






}
