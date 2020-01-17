#include "../src/watch2.h"

void state_func_stopwatch()
{
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
    static uint16_t text_height = 0;
    static uint16_t width_two_digits = 0, width_two_digits_colon = 0, width_three_digits_colon = 0;
    static uint16_t stopwatch_y = watch2::top_thing_height + ((SCREEN_HEIGHT - watch2::top_thing_height) / 3);
    static uint16_t status_y = watch2::top_thing_height + ((SCREEN_HEIGHT - watch2::top_thing_height) / 2);

    //code to execute while in a stopwatch state
    switch(watch2::stopwatch_timing)
    {

        case 0: //stopped

            watch2::stopwatch_epoch = millis();
            watch2::stopwatch_paused_diff = 0;
            break;

        case 1: //running

            //idk
            break;

        case 2: //paused

            watch2::stopwatch_epoch = millis() - watch2::stopwatch_paused_diff;
            break;

    }

    //code to execute when entering a stopwatch state
    if (dpad_enter_active())
    {
        switch(watch2::stopwatch_timing)
        {
            case 0: //stopped to running

                watch2::stopwatch_epoch = millis();
                watch2::stopwatch_timing = 1;
                break;

            case 1: //running to paused

                watch2::stopwatch_paused_diff = millis() - watch2::stopwatch_epoch;
                watch2::stopwatch_timing = 2;
                break;

            case 2: //paused to running

                watch2::stopwatch_timing = 1;
                break;
        }
    }

    //stop timing
    if (dpad_down_active())
    {
        watch2::stopwatch_timing = 0;
    }

    //draw elapsed time
    watch2::stopwatch_time_diff = millis() - watch2::stopwatch_epoch;
    watch2::stopwatch_ms = watch2::stopwatch_time_diff % 1000;
    watch2::stopwatch_s    = (int) (watch2::stopwatch_time_diff / 1000)             % 60;
    watch2::stopwatch_min  = (int)((watch2::stopwatch_time_diff / (1000 * 60))      % 60);
    watch2::stopwatch_hour = (int)((watch2::stopwatch_time_diff / (1000 * 60 * 60)) % 24);

    //watch2::oled.setFreeFont(&SourceSansPro_Light12pt7b);
    watch2::oled.setTextColor(watch2::themecolour, BLACK);

    //calculate sizes of stopwatch digit things, and calculate text height
    if (!watch2::state_init)
    {
        watch2::setFont(LARGE_FONT);
        watch2::getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
        watch2::getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
        watch2::getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);
        text_height += watch2::oled.fontHeight();
        status_y = stopwatch_y + watch2::oled.fontHeight();

        //main font
        watch2::setFont(MAIN_FONT);
        text_height += watch2::oled.fontHeight();
        watch2::setFont(LARGE_FONT);
    }

    //draw hours
    if ((watch2::stopwatch_hour != watch2::stopwatch_last_hour) || !watch2::state_init)
    {
        watch2::oled.setCursor(2, stopwatch_y);
        sprintf(text_aaaa, "%02d", watch2::stopwatch_last_hour);
        watch2::getTextBounds(String(text_aaaa), 2, stopwatch_y, &x1, &y1, &w, &h);
        watch2::oled.fillRect(x1, y1, w, h, BLACK);
        watch2::oled.printf("%02d", watch2::stopwatch_hour);
        watch2::stopwatch_last_hour = watch2::stopwatch_hour;
    }

    //draw minutes
    if ((watch2::stopwatch_min != watch2::stopwatch_last_min) || !watch2::state_init)
    {
        watch2::oled.setCursor(2 + (width_two_digits), stopwatch_y);
        sprintf(text_aaaa, ":%02d", watch2::stopwatch_last_min);
        watch2::getTextBounds(text_aaaa, 2 + (width_two_digits), stopwatch_y, &x1, &y1, &w, &h);
        watch2::oled.fillRect(x1, y1, w, h, BLACK);
        watch2::oled.printf(":%02d", watch2::stopwatch_min);
        watch2::stopwatch_last_min = watch2::stopwatch_min;
    }

    //draw seconds
    if ((watch2::stopwatch_s != watch2::stopwatch_last_s) || !watch2::state_init)
    {
        watch2::oled.setCursor(2 + (2 * width_two_digits_colon), stopwatch_y);
        sprintf(text_aaaa, ":%02d", watch2::stopwatch_last_s);
        watch2::getTextBounds(text_aaaa, 2 + (2 * width_two_digits_colon), stopwatch_y, &x1, &y1, &w, &h);
        watch2::oled.fillRect(x1, y1, w, h, BLACK);
        watch2::oled.printf(":%02d", watch2::stopwatch_s);
        watch2::stopwatch_last_s = watch2::stopwatch_s;
    }

    //draw milliseconds
    if ((watch2::stopwatch_ms != watch2::stopwatch_last_ms) || !watch2::state_init)
    {
        watch2::oled.setCursor(2 + (3 * width_two_digits_colon), stopwatch_y);
        sprintf(text_aaaa, ":%03d", watch2::stopwatch_last_ms);
        watch2::getTextBounds(text_aaaa, 2 + (3 * width_two_digits_colon), stopwatch_y, &x1, &y1, &w, &h);
        watch2::oled.fillRect(x1, y1, w, h, BLACK);
        watch2::oled.printf(":%03d", watch2::stopwatch_ms);
        watch2::stopwatch_last_ms = watch2::stopwatch_ms;
    }

        // reset font
    //watch2::oled.setFreeFont(&SourceSansPro_Light8pt7b);
    watch2::oled.setTextColor(WHITE, BLACK);

    //draw status text
    if (dpad_any_active() || !watch2::state_init)
    {
        //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
        watch2::setFont(MAIN_FONT);
        watch2::oled.setCursor(2, status_y);
        watch2::oled.fillRect(2, status_y, SCREEN_WIDTH - 2, watch2::oled.fontHeight(), BLACK);
        if (watch2::stopwatch_timing == 0) watch2::oled.print("Stopped");
        if (watch2::stopwatch_timing == 1) watch2::oled.print("Timing");
        if (watch2::stopwatch_timing == 2) watch2::oled.print("Paused");
        watch2::setFont(LARGE_FONT);
    }

    //draw top thing
    watch2::drawTopThing();

    //if left pressed, go back to state menu
    if (dpad_left_active())
    {
        watch2::setFont(MAIN_FONT);
        watch2::switchState(2);
    }
}