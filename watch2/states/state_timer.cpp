#include "../globals.h"

void state_func_timer()
{
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
}