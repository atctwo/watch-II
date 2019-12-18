#include "../globals.h"

void state_func_alarms()
{
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
            if (alarm_time > 24*60*60) alarm_time = 0;
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
}