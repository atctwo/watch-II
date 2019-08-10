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


}
