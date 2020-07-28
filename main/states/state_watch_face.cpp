#include "../watch2.h"

void state_func_watch_face()
{

    static int last_minute = -1;
    static int last_second = -1;
    static int last_day = -1;
    static TFT_eSprite time_sprite = TFT_eSprite(&watch2::oled);
    static TFT_eSprite second_sprite = TFT_eSprite(&watch2::oled);
    static TFT_eSprite day_sprite = TFT_eSprite(&watch2::oled);
    static char buffer[20];
    uint16_t row_colours[SCREEN_WIDTH];
    uint16_t row_colour = CYAN;
    float r = 0, g = 0, b = 0;
    uint16_t phase_difference = 0;

    if (watch2::states[watch2::state].variant == 0) //actual watch face
    {

        /*
        //draw introduction message
        watch2::oled.setCursor(2, watch2::top_thing_height);
        watch2::oled.setTextColor(WHITE, BLACK);

        watch2::oled.setTextSize(1);
        watch2::oled.print("The time right now is");
        */

        if (!watch2::state_init)
        {
            // set up time sprite
            watch2::setFont(REALLY_REALLY_BIG_FONT);
            time_sprite.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight());
            time_sprite.fillScreen(BLACK);
            watch2::setFont(REALLY_REALLY_BIG_FONT, time_sprite);

            // set up seconds buffer
            watch2::setFont(REALLY_BIG_FONT);
            second_sprite.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight());
            second_sprite.fillScreen(BLACK);
            watch2::setFont(REALLY_BIG_FONT, second_sprite);

            // set up date buffer
            watch2::setFont(SLIGHTLY_BIGGER_FONT);
            day_sprite.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight());
            day_sprite.fillScreen(BLACK);
            watch2::setFont(SLIGHTLY_BIGGER_FONT, day_sprite);
            watch2::setFont(MAIN_FONT);
        }

        // set phase difference
        if (watch2::animate_watch_face) phase_difference = ((millis() % 60000) * 6)/1000;

        // set up time sprite colour things
        for (int i = 0; i < SCREEN_WIDTH; i++)
        {
            switch(watch2::trans_mode)
                {
                    case 0: // random
                        //todo

                    case 1: // Gay™
                        watch2::HSVtoRGB(&r, &g, &b, fmodf(((((float) i) * (360 / SCREEN_WIDTH)) + phase_difference), (float)360.0), 1.0, 1.0);
                        row_colour = watch2::oled.color565(r * 255, g * 255, b * 255);
                        break;

                    case 2: // trans
                        watch2::getHeatMapColor( fmod(((float)i/(float)SCREEN_WIDTH) + (phase_difference/360.0), (float)1.0) , &r, &g, &b);
                        row_colour = watch2::oled.color565(r * 255, g * 255, b * 255);
                        break;

                    default:
                        row_colour = CYAN;
                        break;
                }

            row_colours[i] = row_colour;
        }

        // draw time
        draw(minute() != last_minute, {
            time_sprite.fillScreen(BLACK);
            time_sprite.setTextDatum(MC_DATUM);
            sprintf(buffer, "%02d:%02d", hour(), minute());

            // if watch face colour is 3 (seconds)
            if (watch2::trans_mode == 3) 
            {
                //float seconds = second() + (millis() / 1000);
                uint8_t seconds = second();
                watch2::HSVtoRGB(&r, &g, &b, fmodf(((((float) seconds) * (360 / 60.0))), (float)360.0), 1.0, 1.0);
                time_sprite.setTextColor( watch2::oled.color565(r * 255, g * 255, b * 255) );
            }

            // otherwise, just use the themecolour
            else time_sprite.setTextColor(watch2::themecolour);
            
            time_sprite.drawString(buffer, time_sprite.width() / 2, time_sprite.height() / 2);

            // if watch face colour is 4 (theme), push the sprite using oled::pushSprite()
            if (watch2::trans_mode == 4 || watch2::trans_mode == 3)
            {
                time_sprite.pushSprite(0, watch2::top_thing_height);
            }

            // otherwise, draw the time using a gradient
            else
            {
                uint16_t *time_sprite_data = (uint16_t*) time_sprite.frameBuffer(0);
                int16_t w = time_sprite.width();
                int16_t h = time_sprite.height();

                watch2::oled.startWrite();
                for (int y = 0; y < h; y++)
                {
                    for (int x = 0; x < w; x++)
                    {
                        if (*time_sprite_data++) row_colour = row_colours[x];
                        else row_colour = BLACK;

                        watch2::oled.setWindow(x, watch2::top_thing_height + y, 1, 1);
                        watch2::oled.pushBlock(row_colour, 1);
                    }
                }
                watch2::oled.endWrite();
            }

            last_minute = now();
        });

        //draw seconds
        draw(second() != last_second, {
            second_sprite.fillScreen(BLACK);
            second_sprite.setTextDatum(TL_DATUM);
            sprintf(buffer, " %02d", second());
            second_sprite.drawString(buffer, 0, 0);

            second_sprite.setTextDatum(TR_DATUM);
            second_sprite.drawString("pm ", second_sprite.width(), 0);

            second_sprite.pushSprite(0, watch2::top_thing_height + time_sprite.height());
            last_second = second();
        });

        // draw date
        draw(day() != last_day, {
            day_sprite.fillScreen(BLACK);
            day_sprite.setTextDatum(TC_DATUM);
            sprintf(buffer, "%s %02d.%02d.%04d", dayShortStr(dayOfWeek(day())), day(), month(), year());
            day_sprite.drawString(buffer, day_sprite.width() / 2, 0);
            uint16_t start_height = watch2::top_thing_height + time_sprite.height() + second_sprite.height();
            day_sprite.pushSprite(0, start_height + ((SCREEN_HEIGHT - start_height) / 2));
            last_day = day();
        });

        //draw minimal top thing
        watch2::drawTopThing(true);

        //check buttons
        if (dpad_enter_active() || dpad_right_active() || dpad_up_active())
        {
            // free up the memory used by the sprites
            time_sprite.deleteSprite();
            second_sprite.deleteSprite();
            day_sprite.deleteSprite();
        }

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

    }
    else if (watch2::states[watch2::state].variant == 1) //ripoff control centre
    {

        static int selected_widget = 3;
        static int last_button_widget = 3;
        static bool go_back_to_watch_face = false;
        static uint8_t last_bt_state = watch2::bluetooth_state;
        static uint8_t last_wifi_state = watch2::wifi_state;

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
                ledcWrite(TFTBL_PWM_CHANNEL, watch2::screen_brightness);
                watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                watch2::preferences.end();
            }
            else if (selected_widget == 2) // torch
            {
                watch2::torch_brightness = std::max(watch2::torch_brightness - 10, 0);
                ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
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
                ledcWrite(TFTBL_PWM_CHANNEL, watch2::screen_brightness);
                watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
                watch2::preferences.putUInt("brightness", watch2::screen_brightness);
                watch2::preferences.end();
            }
            else if (selected_widget == 2) // torch
            {
                watch2::torch_brightness = std::min(watch2::torch_brightness + 10, 255);
                ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
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
            if (selected_widget == 3)
            {
                if (watch2::wifi_state == 0) watch2::enable_wifi();
                else watch2::disable_wifi();
            }
            if (selected_widget == 4) 
            {
                if (watch2::bluetooth_state == 0) watch2::enable_bluetooth();
                else watch2::disable_bluetooth();
            }
        }

        draw(dpad_any_active() || (watch2::wifi_state != last_wifi_state) || (watch2::bluetooth_state != last_bt_state), {
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
            if (watch2::wifi_state == 3) background_colour = 0x867d; // enabled + connected
            if (watch2::wifi_state == 2) background_colour = 0x6E5A; // enabled + connecting
            if (watch2::wifi_state == 1) background_colour = 0x6E5A; // enabled + disconnected
            if (watch2::wifi_state == 0) background_colour = BLACK;  // disabled
            watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
            outline_colour = ( !(selected_widget == 3) != !(watch2::wifi_state == 3) ) ? 0x867D : WHITE;
            watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
            watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["wifi"].data(), button_size, button_size, (watch2::wifi_state != 0) ? WHITE : 0x867D);
            button_x += spacing + button_size;
            if (watch2::wifi_state != last_wifi_state) last_wifi_state = watch2::wifi_state;

            //draw bluetooth button  bluetooth blue
            if (watch2::bluetooth_state == 3) background_colour = 0x041F; // enabled + connected
            if (watch2::bluetooth_state == 2) background_colour = 0x743E; // enabled + disconnected
            if (watch2::bluetooth_state == 1) background_colour = 0xA55C; // enabling
            if (watch2::bluetooth_state == 0) background_colour = BLACK;  // disabled
            watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
            outline_colour = ( !(selected_widget == 4) != !(watch2::bluetooth_state == 3) ) ? 0x041F : WHITE;
            watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
            watch2::oled.drawBitmap(button_x, button_y, watch2::small_icons["bluetooth"].data(), button_size, button_size, (watch2::bluetooth_state != 0) ? WHITE : 0x041F);
            button_x += spacing + button_size;
            if (watch2::bluetooth_state != last_bt_state) last_bt_state = watch2::bluetooth_state;


        });

        if (dpad_down_active())
        {
            if (go_back_to_watch_face) watch2::switchState(watch2::state, 0);
        }

    }
}