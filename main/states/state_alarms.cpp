#include "../watch2.h"

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

    if (!watch2::state_init)
    {
        //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);

        //calculate sizes of digit things
        watch2::getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
        watch2::getTextBounds(": 99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
        watch2::getTextBounds(": 999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);

    }

    if (dpad_left_active())
    {
        selected_button--;
        if (selected_button < 0) selected_button = 4;
        if (selected_alarm == -1 && selected_button < 2) selected_button = 4;
    }

    if (dpad_right_active())
    {
        selected_button++;
        if (selected_button > 4) selected_button = 0;
        if (selected_alarm == -1 && selected_button < 2) selected_button = 2;
    }

    if (dpad_down_active())
    {
        if (selected_alarm == -1 || selected_button == 3 || selected_button == 4)
        {
            selected_alarm++;
            if (selected_alarm >= watch2::alarms.size()) selected_alarm = -1;
        }
        else //decrease time
        {
            time_t alarm_time = Alarm.read(watch2::alarms[selected_alarm].alarm_id);
            if (floor(alarm_time / 3600) > 0)
            if (selected_button == 0) alarm_time -= 3600;      //hours
            if (floor(alarm_time % 3600 / 60) > 0)
            if (selected_button == 1) alarm_time -= 60;        //minutes
            if (floor(alarm_time % 3600 % 60) > 0)
            if (selected_button == 2) alarm_time -= 1;         //seconds
            Alarm.write(watch2::alarms[selected_alarm].alarm_id, alarm_time);
            if (watch2::alarms[selected_alarm].paused) Alarm.disable(watch2::alarms[selected_alarm].alarm_id);
            watch2::alarms[selected_alarm].initial_time = alarm_time;

        }
    }

    if (dpad_up_active())
    {
        if (selected_alarm == -1 || selected_button == 3 || selected_button == 4)
        {
            selected_alarm--;
            if (selected_alarm < -1) selected_alarm = watch2::alarms.size() - 1;
        }
        else //increase time
        {
            time_t alarm_time = Alarm.read(watch2::alarms[selected_alarm].alarm_id);
            if (selected_button == 0) alarm_time += 3600;      //hours
            if (selected_button == 1) alarm_time += 60;        //minutes
            if (selected_button == 2) alarm_time += 1;         //seconds
            if (alarm_time > 24*60*60) alarm_time = 0;
            Alarm.write(watch2::alarms[selected_alarm].alarm_id, alarm_time);
            if (watch2::alarms[selected_alarm].paused) Alarm.disable(watch2::alarms[selected_alarm].alarm_id);
            watch2::alarms[selected_alarm].initial_time = alarm_time;
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
                        for (int i = 0; i < watch2::alarms.size(); i++)
                        {
                            if (watch2::alarms[i].alarm_id == id)
                            {
                                alarm_id = i;
                            }
                        }
                        watch2::alarm_trigger_status = 1;
                        watch2::alarm_trigger_id = alarm_id;

                    });
                Alarm.disable(alarm_id_aaa);
                watch2::alarms.push_back({
                    alarm_id_aaa,   //alarm id
                    true,           //paused
                    -1              //last value
                });
            }
            else if (selected_button == 2)
            {
                // change timer music
                Serial.println("[alarms] changing music");
                std::string new_music = watch2::beginFileSelect();
                Serial.printf("[alarms] new music to %s\n", new_music.c_str());
                watch2::alarm_music = String(new_music.c_str());
                Serial.printf("[alarms] changed music to %s\n", watch2::alarm_music.c_str());

                watch2::preferences.begin("watch2");
                watch2::preferences.putString("alarm_music", watch2::alarm_music);
                watch2::preferences.end();
                Serial.printf("[alarms] updated preferences store");
            }
        }
        else
        {
            if (selected_button == 3) // play / pause alarm
            {
                if (watch2::alarms[selected_alarm].paused)
                {
                    Alarm.enable(watch2::alarms[selected_alarm].alarm_id);
                    watch2::alarms[selected_alarm].paused = false;
                }
                else
                {
                    Alarm.disable(watch2::alarms[selected_alarm].alarm_id);
                    watch2::alarms[selected_alarm].paused = true;
                }
            }
            if (selected_button == 4) // remove alarm
            {
                //store selected timer
                AlarmID_t whee = selected_alarm;

                //change selecte timer to previous timer
                selected_alarm--;

                //free alarm id
                Alarm.free(watch2::alarms[whee].alarm_id);

                //remove timer from timers
                watch2::alarms.erase(watch2::alarms.begin() + whee);

                //clear screen
                watch2::oled.fillScreen(BLACK);
            }
        }
    }

    //draw alarms
    draw(dpad_any_active(), {
        int alarm_x = 2;
        int alarm_y = watch2::top_thing_height;
        int alarm_w = SCREEN_WIDTH - (alarm_x * 2);
        int alarm_h = SCREEN_HEIGHT - alarm_y;
        int icon_radius = 8;
        int radius = 4;
        int icon_spacing = 3;
        int icon_size = 20;
        int alarm_edge = alarm_x + alarm_w;
        uint16_t working_button_colour = WHITE;
        uint16_t working_alarm_colour = WHITE;

        draw(dpad_any_active(), {

            //set colour of row
            working_alarm_colour = (selected_alarm == -1) ? watch2::themecolour : WHITE;

            //draw header
            watch2::oled.setTextColor(working_alarm_colour, BLACK);
            watch2::oled.setCursor(alarm_x, alarm_y);
            watch2::oled.print("Alarms");

            //set colour of music button
            working_button_colour = (selected_button <= 2 && selected_alarm == -1) ? watch2::themecolour : WHITE;

            //draw music button
            watch2::oled.drawBitmap(
                alarm_edge - (icon_size * 3) - (icon_spacing * 8),
                alarm_y,
                (*watch2::small_icons)["small_audio"].data(),
                icon_size,
                icon_size,
                working_button_colour
            );

            //set colour of back button
            working_button_colour = (selected_button == 3 && selected_alarm == -1) ? watch2::themecolour : WHITE;

            //draw back button
            watch2::oled.drawBitmap(
                alarm_edge - (icon_size * 2) - (icon_spacing * 5),
                alarm_y,
                (*watch2::small_icons)["back"].data(),
                icon_size,
                icon_size,
                working_button_colour
            );

            //set colour of add button
            working_button_colour = (selected_button == 4 && selected_alarm == -1) ? watch2::themecolour : WHITE;

            //draw add button
            watch2::oled.drawBitmap(
                alarm_edge - icon_size - (icon_spacing * 2),
                alarm_y,
                (*watch2::small_icons)["add"].data(),
                icon_size,
                icon_size,
                working_button_colour
            );
        });

        //draw alarms
        if (watch2::alarms.size() == 0)
        {
            alarm_y += watch2::oled.fontHeight() + 2;
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(alarm_x, alarm_y);
            watch2::oled.print("No alarms and ");
            alarm_y += watch2::oled.fontHeight() + 2;
            watch2::oled.setCursor(alarm_x, alarm_y);
            watch2::oled.print("no suprises");
        }

        if (watch2::alarms.size() == 1)
        {
            Serial.println("a");
            watch2::oled.fillRect(alarm_x, alarm_y + watch2::oled.fontHeight() + 2, alarm_w, (watch2::oled.fontHeight() * 2) + 4, BLACK);
        }

        for (int i = 0; i < watch2::alarms.size(); i++)
        {
            Serial.printf("alarm %d\n", i);
            alarm_y += watch2::oled.fontHeight() + (icon_spacing * 2);

            time_t alarm_time = Alarm.read(watch2::alarms[i].alarm_id);
            //if this is the state's first frame, or the time of the alarm has changed,
            //or this is the selected alarm and any button has been pressed,
            //or this is the previously selected alarm and any button has been pressed
            draw(alarm_time != watch2::alarms[i].last_value || dpad_any_active(), {

                //clear previous text
                int wheeoo = (i == 0 && watch2::alarms.size() == 1) ?
                    ( SCREEN_WIDTH - alarm_x) :
                    ( alarm_w - (icon_spacing * 6) - (icon_size * 2) );
                int other_wheoo = (i == 0 && watch2::alarms.size() == 1) ? (20 + (icon_spacing * 4)) : 9;
                watch2::oled.fillRect(alarm_x, alarm_y, wheeoo, other_wheoo , BLACK);

                //set colour of row
                working_alarm_colour = (selected_alarm == i) ? watch2::themecolour : WHITE;

                //get time to display
                time_t alarm_time = Alarm.read(watch2::alarms[i].alarm_id);
                time_t alarm_time_hrs = floor(alarm_time / 3600);
                time_t alarm_time_min = floor(alarm_time % 3600 / 60);
                time_t alarm_time_sec = floor(alarm_time % 3600 % 60);

                /*
                watch2::getTextBounds("99", 0, 0, &x1, &y1, &width_two_digits, &h);
                watch2::getTextBounds(":99", 0, 0, &x1, &y1, &width_two_digits_colon, &h);
                watch2::getTextBounds(":999", 0, 0, &x1, &y1, &width_three_digits_colon, &h);
                */

                //print hours
                watch2::oled.setCursor(alarm_x, alarm_y);
                watch2::oled.setTextColor(working_alarm_colour, BLACK);
                watch2::oled.printf("%02d", alarm_time_hrs);
                working_button_colour = (selected_button == 0 && selected_alarm == i) ? watch2::themecolour : BLACK;
                watch2::oled.drawRoundRect(alarm_x - 1, alarm_y - 1, width_two_digits + 4, watch2::oled.fontHeight(), icon_radius, working_button_colour);

                //print minutes
                watch2::oled.setCursor(alarm_x + (width_two_digits) + 4, alarm_y);
                watch2::oled.printf(": %02d", alarm_time_min);
                working_button_colour = (selected_button == 1 && selected_alarm == i) ? watch2::themecolour : BLACK;
                watch2::oled.drawRoundRect(alarm_x + (width_two_digits) + 4 - 1, alarm_y - 1, width_two_digits_colon + 5, watch2::oled.fontHeight(), icon_radius, working_button_colour);

                //print seconds
                watch2::oled.setCursor(alarm_x + width_two_digits + width_two_digits_colon + 2 + 7, alarm_y);
                watch2::oled.printf(": %02d", alarm_time_sec);
                working_button_colour = (selected_button == 2 && selected_alarm == i) ? watch2::themecolour : BLACK;
                watch2::oled.drawRoundRect(alarm_x + width_two_digits + width_two_digits_colon + 2 + 7 - 1, alarm_y - 1, width_two_digits_colon + 5, watch2::oled.fontHeight(), icon_radius, working_button_colour);

                //set colour of play / pause button
                working_button_colour = (selected_button == 3 && selected_alarm == i) ? watch2::themecolour : WHITE;

                //draw play / pause button
                watch2::oled.drawBitmap(
                    alarm_edge - (icon_size * 2) - (icon_spacing * 5),
                    alarm_y,
                    (watch2::alarms[i].paused) ? (*watch2::small_icons)["play"].data() : (*watch2::small_icons)["pause"].data(),
                    icon_size,
                    icon_size,
                    working_button_colour
                );

                //set colour of delete button
                working_button_colour = (selected_button == 4 && selected_alarm == i) ? watch2::themecolour : WHITE;

                //draw delete button
                watch2::oled.drawBitmap(
                    alarm_edge - icon_size - (icon_spacing * 2),
                    alarm_y,
                    (*watch2::small_icons)["key_cancel"].data(),
                    icon_size,
                    icon_size,
                    working_button_colour
                );

                watch2::alarms[i].last_value = alarm_time;

            });

        }

    });

    watch2::drawTopThing();

    if (dpad_enter_active())
    {
        if (selected_alarm == -1)
        {
            //go back to state menu
            if (selected_button < 4) watch2::switchState(2);
        }
    }

    last_selected_alarm = selected_alarm;
}