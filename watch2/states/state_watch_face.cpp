#include "../src/watch2.h"

void state_func_watch_face()
{

    if (watch2::states[watch2::state].variant == 0) //actual watch face
    {
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
        //static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH-1, 42);            //time canvas (used to draw time w/ a gradient)
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
        watch2::oled.setCursor(2, 20);
        watch2::oled.setTextColor(WHITE, BLACK);

        watch2::oled.setTextSize(1);
        watch2::oled.print("The time right now is");

        //draw minimal top thing
        watch2::drawTopThing(true);

        //check buttons
        if (dpad_enter_active())
        {
            watch2::deepSleep(31);
        }

        if (dpad_right_active())
        {
            watch2::switchState(2);
        }

        if (dpad_up_active())
        {
            watch2::switchState(watch2::state, 1);
        }

        if (dpad_left_active())
        {
            digitalWrite(cs, HIGH);
            digitalWrite(sdcs, LOW);
            
            watch2::initSD();

            digitalWrite(cs, LOW);
            digitalWrite(sdcs, HIGH);

        }

        //check buttons
        if (dpad_enter_active())
        {
            watch2::deepSleep(31);
        }

        if (dpad_right_active())
        {
            watch2::switchState(2);
        }

        if (dpad_up_active())
        {
            watch2::switchState(watch2::state, 1);
        }

    }
    else if (watch2::states[watch2::state].variant == 1) //ripoff control centre
    {

        static int selected_widget = 3;
        static int last_button_widget = 3;
        static bool go_back_to_watch_face = false;
        static bool bt_test = false;

        if (!watch2::state_init) go_back_to_watch_face = false;

        if (dpad_left_active())
        {
            if (selected_widget > 2)
            {
                selected_widget--;
                if (selected_widget < 3) selected_widget = 4;
            }
            else if (selected_widget == 0) // volume
            {
                watch2::speaker_volume = std::max(watch2::speaker_volume - 1, 0);
            }
            else if (selected_widget == 1) // brightness
            {
                watch2::screen_brightness = std::max(watch2::screen_brightness - 10, 0);
                ledcWrite(1, watch2::screen_brightness);
                watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                watch2::preferences.end();
            }
            else if (selected_widget == 2) // torch
            {
                watch2::torch_brightness = std::max(watch2::torch_brightness - 10, 0);
                ledcWrite(0, watch2::torch_brightness);
            }
        }
        if (dpad_right_active())
        {
            if (selected_widget > 2)
            {
                selected_widget++;
                if (selected_widget > 4) selected_widget = 3;
            }
            else if (selected_widget == 0) // volume
            {
                watch2::speaker_volume = std::min(watch2::speaker_volume + 1, 21);
            }
            else if (selected_widget == 1) // brightness
            {
                watch2::screen_brightness = std::min(watch2::screen_brightness + 10, 255);
                ledcWrite(1, watch2::screen_brightness);
                watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                watch2::preferences.end();
            }
            else if (selected_widget == 2) // torch
            {
                watch2::torch_brightness = std::min(watch2::torch_brightness + 10, 255);
                ledcWrite(0, watch2::torch_brightness);
            }
        }
        if (dpad_up_active())
        {
            if (selected_widget > 2)
            {
                last_button_widget = selected_widget;
                selected_widget = 2;
            }
            else
            {
                selected_widget--;
                if (selected_widget < 0) selected_widget = last_button_widget;
            }
        }
        if (dpad_down_active())
        {
            if (selected_widget > 2)
            {
                //last_button_widget = selected_widget;
                //selected_widget = 0;
                go_back_to_watch_face = true;
            }
            else
            {
                selected_widget++;
                if (selected_widget > 2) selected_widget = last_button_widget;
            }
        }
        if (dpad_enter_active())
        {
            bt_test = !bt_test;
        }

        if (!watch2::state_init || dpad_any_active())
        {
            int spacing = 10;
            int button_size = 30;
            int radius = 10;
            int outline_colour = WHITE;
            int background_colour = BLACK;
            int button_x = spacing;
            int button_y = spacing;
            int slider_x = 0;

            //draw volume slider                                                                                                                    jungle green
            if (selected_widget == 0) watch2::oled.fillRect(button_x - 2, button_y - 2, SCREEN_WIDTH - button_x, button_size + 4, BLACK);
            watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), SCREEN_WIDTH - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
            slider_x = button_x + ( watch2::speaker_volume * ( ( SCREEN_WIDTH - button_size - spacing - 2 - button_x ) / 21.0 ) );
            watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
            watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0x2D50);                               //background thing
            outline_colour = (selected_widget == 0) ? WHITE : 0x2D50;                                                                               //determine outline colour
            watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
            watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["volume"].data(), button_size, button_size, WHITE);                     //draw icon
            button_y += spacing + button_size;                                                                                                      //move cursor

            //draw brightness slider                                                                                                                yellow
            if (selected_widget == 1) watch2::oled.fillRect(button_x - 2, button_y - 2, SCREEN_WIDTH - button_x, button_size + 4, BLACK);
            watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), SCREEN_WIDTH - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
            slider_x = button_x + ( watch2::screen_brightness * ( (float)( SCREEN_WIDTH - button_size - spacing - 2 - button_x ) / ( 255.0 ) ) );
            watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
            watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0xFEC0);                               //background thing
            outline_colour = (selected_widget == 1) ? WHITE : 0xFEC0;                                                                               //determine outline colour
            watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
            watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["sun"].data(), button_size, button_size, WHITE);                        //draw icon
            button_y += spacing + button_size;                                                                                                      //move cursor

            //draw torch slider                                                                                                                     adafruit yellow
            if (selected_widget == 2) watch2::oled.fillRect(button_x - 2, button_y - 2, SCREEN_WIDTH - button_x, button_size + 4, BLACK);
            watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), SCREEN_WIDTH - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
            slider_x = button_x + ( watch2::torch_brightness * ( ( SCREEN_WIDTH - button_size - spacing - 2 - button_x ) / 255.0 ) );
            watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
            watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, YELLOW);                               //background thing
            outline_colour = (selected_widget == 2) ? WHITE : YELLOW;                                                                               //determine outline colour
            watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
            watch2::oled.drawBitmap(slider_x, button_y, watch2::small_icons["torch_but_smaller"].data(), button_size, button_size, WHITE);          //draw icon
            button_y += spacing + button_size;                                                                                                      //move cursor

            //draw wifi button
            outline_colour = (selected_widget == 3) ? 0x867D : WHITE;
            watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
            watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["wifi"].data(), button_size, button_size, 0x867D);
            button_x += spacing + button_size;

            //draw bluetooth button  bluetooth blue
            background_colour = (bt_test) ? 0x041F : BLACK;
            watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
            outline_colour = ( !(selected_widget == 4) != !(bt_test) ) ? 0x041F : WHITE;
            watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
            watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["bluetooth"].data(), button_size, button_size, (bt_test) ? WHITE : 0x041F);
            button_x += spacing + button_size;


        }

        if (dpad_down_active())
        {
            if (go_back_to_watch_face) watch2::switchState(watch2::state, 0);
        }

    }
}