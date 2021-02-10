/**
 * @file watch2_dialogues.cpp
 * @author atctwo
 * @brief functions for popup dialogues
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // dialogues
    EXT_RAM_ATTR bool showingControlCentre = false;

    std::string textFieldDialogue(std::string prompt, const char *default_input, const char mask, bool clear_screen)
    {
        // lock dpad
        for (uint16_t i = 0; i < 5; i++) 
        {
            //Serial.printf("[button locks] set button %d to locked and not pressed\n", i);
            dpad_lock[i] = true;
            dpad_pressed[i] = false;
        }

        /*
        special keys (like caps lock, space, backspace, enter) are mapped to repurposed ASCII control codes.
        the codes are interpreted by the system, so that no control codes are actually returned as part of 
        the returned string.  i tried to pick control codes that match the function of the key as much as
        possible.  the mappings are described in this table:

        key             ascii (hex)     ascii description
        blank           0x00            null
        backspace       0x08            backspace
        clear           0x0c            form feed
        enter           0x04            end of transmission
        cancel          0x18            cancel
        cursor left     0x11            device control 1
        cursor right    0x12            device control 2
        space           0x20            space
        caps lock       0x13            device control 3
        symbols         0x0e            shift out
        letters         0x0f            shift in
        */

        // get the number of newlines in the prompt
        // adapted from https://stackoverflow.com/a/14266139/9195285
        std::string delimiter = "\n";
        size_t pos = 0;
        uint8_t no_newlines = 0;
        std::string token;
        while ((pos = prompt.find(delimiter, pos)) != std::string::npos) {
            pos++;
            no_newlines++;
        }

        uint8_t key_h = oled.fontHeight();
        uint8_t key_w = key_h;
        uint8_t no_rows = 4;
        uint8_t no_cols = 11;
        uint8_t box_radius = 10;
        uint8_t key_radius = 7;
        bool finish_input = false;

        bool caps_lock = false;
        bool last_caps_lock = false;
        uint8_t selected_row = 0;
        uint8_t selected_col = 0;
        uint8_t selected_page = 0;
        uint8_t last_page = 0;
        std::vector<std::vector<std::vector<char>>> keys = {
            {
                {'1',  '2', '3', '4', '5', '6', '7', '8', '9',  '0',  0x11},
                {'q',  'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p',  0x12},
                {0x13, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',  'l',  0x20},
                {0x0e, 'z', 'x', 'c', 'v', 'b', 'n', 'm', 0x08, 0x18, 0x04}
            },
            {
                {'!',  '"', 0,   '$',  '%',  '^', '&', '*', '(',  ')',  0x11},
                {':',  ';', '@', '\'', '~',  '#', '-', '_', '`',  0,    0x12},
                {'.',  ',', '?', '/',  '\\', '|', '+', '=', 0,    0,    0x20},
                {0x0f, '<', '>', '{',  '}',  '[', ']', 0,   0x08, 0x0c, 0x04}
            }
        };

        uint16_t keyboard_w = SCREEN_WIDTH * 0.95;
        uint16_t keyboard_a = (SCREEN_WIDTH - keyboard_w) / 2;
        uint16_t keyboard_h = (no_rows * oled.fontHeight()) + (keyboard_w - (key_w * no_cols));
        uint16_t keyboard_x = SCREEN_WIDTH - keyboard_a - keyboard_w;
        uint16_t keyboard_y = SCREEN_HEIGHT - keyboard_a - keyboard_h;

        uint16_t key_matrix_w = (key_w * no_cols);
        uint16_t key_matrix_h = (key_h * no_rows);
        uint16_t key_matrix_x = keyboard_x + ((keyboard_w - key_matrix_w) / 2);
        uint16_t key_matrix_y = keyboard_y + ((keyboard_h - key_matrix_h) / 2);

        uint16_t input_box_x = 15;
        uint16_t input_box_y = top_thing_height;
        uint16_t input_box_w = SCREEN_WIDTH - (input_box_x * 2);
        uint16_t input_box_h = SCREEN_HEIGHT - (2 * keyboard_a) - keyboard_h - top_thing_height;

        uint8_t input_field_padding = 2;
        uint16_t input_field_w = input_box_w * 0.9;
        uint16_t input_field_h = oled.fontHeight() + (input_field_padding * 2);
        uint16_t input_group_h = input_field_h + (oled.fontHeight() * (no_newlines + 1));
        uint16_t input_field_x = input_box_x + ((input_box_w - input_field_w) / 2);
        uint16_t input_group_y = input_box_y + ((input_group_h - input_field_h) / 2);
        uint16_t input_field_y = input_group_y + (oled.fontHeight() * (no_newlines + 1));

        std::string input = default_input;

        // draw input box
        oled.fillRoundRect(input_box_x, input_box_y, input_box_w, input_box_h, box_radius, BLACK);
        oled.drawRoundRect(input_box_x, input_box_y, input_box_w, input_box_h, box_radius, themecolour);

        // draw input field
        // also adapted from https://stackoverflow.com/a/14266139/9195285
        pos = 0;
        uint8_t line_num = 0;
        while ((pos = prompt.find(delimiter)) != std::string::npos) {
            token = prompt.substr(0, pos);
            oled.setCursor(input_field_x, input_group_y + (oled.fontHeight() * line_num));
            oled.setTextColor(WHITE, BLACK);
            oled.print(token.c_str());
            prompt.erase(0, pos + delimiter.length());
            line_num++;
        }
        oled.setCursor(input_field_x, input_group_y + (oled.fontHeight() * line_num));
        oled.setTextColor(WHITE, BLACK);
        oled.print(prompt.c_str());
        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);

        // draw keyboard box
        oled.fillRoundRect(keyboard_x, keyboard_y, keyboard_w, keyboard_h, box_radius, BLACK);
        oled.drawRoundRect(keyboard_x, keyboard_y, keyboard_w, keyboard_h, box_radius, themecolour);

        // draw keys
        auto draw_keys = [&]() {
            //oled.drawRect(key_matrix_x, key_matrix_y, key_matrix_w, key_matrix_h, ORANGE);
            uint8_t selected_key = selected_col + (no_cols * selected_row);
            oled.setTextDatum(MC_DATUM);
            for (int y = 0; y < no_rows; y++)
            {
                for (int x = 0; x < no_cols; x++)
                {
                    uint8_t index = x + (no_cols * y);
                    char key = keys[selected_page][y][x];

                    // clear screen
                    if (last_page != selected_page || last_caps_lock != caps_lock) oled.fillRect(key_matrix_x + (x * key_w), key_matrix_y + (y * key_h), key_w, key_h, BLACK);

                    // draw character
                    if (0x21 <= key && key <= 0x7e) // if key is printable (excl. space because there is a space symbol)
                    {
                        char key2 = key;
                        // if caps lock is on, and the key is a letter, print the key as a capital
                        if (caps_lock && (0x61 <= key && key <= 0x7a)) key2 -= 32;
                        oled.setTextColor(WHITE, BLACK);
                        oled.drawString(
                            String(key2),
                            key_matrix_x + (x * key_w) + (key_w/2),
                            key_matrix_y + (y * key_h) + (key_h/2)
                        );
                    }
                    else if (key == 0) // blank
                    {
                        // do literally nothing
                    }
                    else // if key is a control character
                    {
                        std::string icon = "";
                        switch(key)
                        {
                            case 0x08: // backspace
                                icon = "key_backspace";
                                break;

                            case 0x0c: // clear
                                icon = "key_clear";
                                break;

                            case 0x04: // enter
                                icon = "key_tick";
                                break;

                            case 0x18: // cancel;
                                icon = "key_cancel";
                                break;

                            case 0x11: // left
                                icon = "key_move_left";
                                break;

                            case 0x12: // right
                                icon = "key_move_right";
                                break;

                            case 0x20: // space
                                icon = "key_space";
                                break;

                            case 0x13: // caps lock
                                icon = (caps_lock) ? "key_caps_on" : "key_caps_off";
                                break;

                            case 0x0e: // symbols
                                icon = "key_symbols";
                                break;

                            case 0x0f: // letters
                                icon = "key_letters";
                                break;
                        }
                        oled.drawBitmap(
                            key_matrix_x + (x * key_w),
                            key_matrix_y + (y * key_h),
                            (*small_icons)[icon].data(),
                            key_w, key_h, WHITE
                        );
                    }
                    

                    // draw button outline
                    oled.drawRoundRect(
                        key_matrix_x + (x * key_w),
                        key_matrix_y + (y * key_h),
                        key_w, key_h, key_radius, (selected_key == index) ? themecolour : BLACK
                    );
                }
            }
            oled.setTextDatum(TL_DATUM);
            last_page = selected_page;
            last_caps_lock = caps_lock;
        };
        draw_keys();

        while(true)
        {
            startLoop();

            // print current input
            oled.setTextColor(WHITE, BLACK);
            oled.setCursor(input_field_x + input_field_padding, input_field_y + input_field_padding);
            if (mask) for (int i = 0; i < input.size(); i++) oled.print(mask);
            else oled.print(input.c_str());

            if (dpad_enter_active())
            {
                switch(keys[selected_page][selected_row][selected_col])
                {
                    case 0x04: // enter
                        finish_input = true;
                        break;

                    case 0x18: // cancel
                        input = "";
                        finish_input = true;
                        break;

                    case 0x0c: // clear input
                        input = "";
                        oled.fillRect(input_field_x, input_field_y, input_field_w, input_field_h, BLACK);
                        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);
                        break;

                    case 0x08: // backspace
                        oled.fillRect(input_field_x, input_field_y, input_field_w, input_field_h, BLACK);
                        oled.drawRect(input_field_x, input_field_y, input_field_w, input_field_h, WHITE);
                        input = input.substr(0, input.size()-1);
                        break;

                    case 0x13: // caps lock
                        caps_lock = !caps_lock;
                        break;

                    case 0x11: // move left
                        break;

                    case 0x12: // move right
                        break;

                    case 0x0e: // symbols
                        selected_page = 1;
                        break;

                    case 0x0f: // letters
                        selected_page = 0;
                        break;

                    case 0: // blank
                        // do nothing
                        break;

                    default:
                        char key2 = keys[selected_page][selected_row][selected_col];
                        // if caps lock is on, and the key is a letter, print the key as a capital
                        if (caps_lock && (0x61 <= key2 && key2 <= 0x7a)) key2 -= 32;
                        input += key2;
                        break;
                }
            }

            if (dpad_left_active())
            {
                if (selected_col == 0) selected_col = no_cols - 1;
                else selected_col--;
            }

            if (dpad_right_active())
            {
                if (selected_col == no_cols - 1) selected_col = 0;
                else selected_col++;
            }

            if (dpad_up_active())
            {
                if (selected_row == 0) selected_row = no_rows - 1;
                else selected_row--;
            }

            if (dpad_down_active())
            {
                if (selected_row == no_rows - 1) selected_row = 0;
                else selected_row++;
            }

            if (dpad_any_active())
            {
                draw_keys();
            }

            if (finish_input) break;
            drawTopThing();
            endLoop();
        }

        forceRedraw = true;
        if (clear_screen) oled.fillScreen(0);
        return input;
    }

    uint8_t messageBox(const char* msg, std::vector<const char*> btns, bool clear_screen, uint16_t colour)
    {
        Serial.printf("showing message box: %s\n", msg);

        // lock dpad
        for (uint16_t i = 0; i < 5; i++) 
        {
            //Serial.printf("[button locks] set button %d to locked and not pressed\n", i);
            dpad_lock[i] = true;
            dpad_pressed[i] = false;
        }

        watch2::setFont(MAIN_FONT);

        // The Numbers
        uint16_t padding = 7;
        uint8_t box_radius = 15;
        uint8_t btn_radius = 7;
        uint8_t selected_button = 0;
        int16_t msg_x = 0, msg_y = 0;
        uint16_t msg_w = 0, msg_h = 0;
        getTextBounds(msg, 0, 0, &msg_x, &msg_y, &msg_w, &msg_h);

        uint16_t dialogue_width = 195;
        uint16_t dialogue_height = (padding * 5) + oled.fontHeight() + msg_h;

        uint16_t dialogue_x = (SCREEN_WIDTH  / 2) - (dialogue_width  / 2);
        uint16_t dialogue_y = (SCREEN_HEIGHT / 2) - (dialogue_height / 2);

        uint16_t text_x = dialogue_x + (dialogue_width / 2);
        uint16_t text_y = dialogue_y + padding;
        Serial.printf("x: %d\ny: %d", text_x, text_y);

        uint16_t btn_widths[btns.size()];
        uint16_t total_btn_width = 0;
        int16_t btn_x = 0, btn_y = 0;
        uint16_t btn_w = 0, btn_h = 0;

        // calculate the size of each button
        for (uint8_t i = 0; i < btns.size(); i++)
        {
            getTextBounds(btns[i], 0, 0, &btn_x, &btn_y, &btn_w, &btn_h);
            btn_widths[i] = btn_w + (padding * 2);
            total_btn_width += btn_widths[i];
        }

        // finish calculating the size of all the buttons together
        total_btn_width += padding * (btns.size() - 1);

        // calculate the button position
        btn_x = dialogue_x + ((dialogue_width / 2) - (total_btn_width / 2));
        btn_y = (padding * 2) + msg_h + dialogue_y;
        btn_w = 0;
        btn_h = (padding * 2) + oled.fontHeight();

        // get number of newlines in msg
        // uint16_t newlines = 0;
        // for(const char* str = msg; *str; ++str) newlines += (*str == '\n');

        // draw dialogue thingy
        oled.fillRoundRect(dialogue_x, dialogue_y, dialogue_width, dialogue_height, box_radius, BLACK);
        oled.drawRoundRect(dialogue_x, dialogue_y, dialogue_width, dialogue_height, box_radius, colour);

        // print message
        // this splits the message using '\n' as a delimiter, and prints each segment of the message on a new line
        oled.setTextColor(WHITE, BLACK);
        oled.setTextDatum(TC_DATUM);
        char* str = strdup(msg);
        char* pch = strtok(str, "\n");
        uint16_t newlines = 0;
        while(pch != NULL)
        {
            oled.drawString(pch, text_x, text_y + (newlines * oled.fontHeight()));
            pch = strtok(NULL, "\n");
            newlines++;
        }
        free(str);
        oled.setTextDatum(TL_DATUM);

        // draw the buttons
        uint16_t current_btn_x = btn_x;
        for (uint8_t i = 0; i < btns.size(); i++)
        {
            uint16_t btn_colour = (i == selected_button) ? WHITE : colour;
            oled.drawRoundRect(current_btn_x, btn_y, btn_widths[i], btn_h, btn_radius, btn_colour);
            oled.drawString(btns[i], current_btn_x + padding, btn_y + padding);
            current_btn_x += btn_widths[i] + padding;
        }

        while(1)
        {
            startLoop();

            if (dpad_right_active())
            {
                if (selected_button == btns.size() - 1) selected_button = 0;
                else selected_button++;
            }

            if (dpad_left_active())
            {
                if (selected_button == 0) selected_button = btns.size() - 1;
                else selected_button--;
            }

            if (dpad_any_active())
            {
                // draw the buttons
                uint16_t current_btn_x = btn_x;
                for (uint8_t i = 0; i < btns.size(); i++)
                {
                    uint16_t btn_colour = (i == selected_button) ? WHITE : colour;
                    oled.drawRoundRect(current_btn_x, btn_y, btn_widths[i], btn_h, btn_radius, btn_colour);
                    oled.drawString(btns[i], current_btn_x + padding, btn_y + padding);
                    current_btn_x += btn_widths[i] + padding;
                }
            }

            if (dpad_enter_active())
            {
                break;
            }

            endLoop();
        }

        forceRedraw = true;
        if (clear_screen) oled.fillScreen(0);
        return selected_button;
    }

    uint16_t popup_menu(const char *title, std::vector<std::string> items, bool scroll, uint16_t colour)
    {
        // lock dpad
        for (uint16_t i = 0; i < 5; i++) 
        {
            //Serial.printf("[button locks] set button %d to locked and not pressed\n", i);
            dpad_lock[i] = true;
            dpad_pressed[i] = false;
        }

        uint16_t selected_item = 0;
        uint16_t padding = 4;
        uint16_t item_height = (3 * padding) + oled.fontHeight();

        uint16_t dialogue_w = 150;
        uint16_t dialogue_h = (2 * padding) + oled.fontHeight() + (items.size() * item_height);
        uint16_t dialogue_x = (SCREEN_WIDTH / 2) - (dialogue_w / 2);
        uint16_t dialogue_y = (SCREEN_HEIGHT / 2) - (dialogue_h / 2);
        uint16_t dialogue_r = 10;

        oled.fillRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, BLACK);
        oled.drawRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, colour);

        oled.setTextDatum(TC_DATUM);
        oled.drawString(title, dialogue_x + (dialogue_w / 2), dialogue_y + padding);

        drawMenu(
            dialogue_x + padding,
            dialogue_y + padding + oled.fontHeight(),
            dialogue_w - (padding * 2), item_height * items.size(),
            items, selected_item, {}, scroll, true, colour
        );

        while(1)
        {
            startLoop();

            if (dpad_up_active())
            {
                if (selected_item == 0) selected_item = items.size() - 1;
                else selected_item--;
                
            }

            if (dpad_down_active())
            {
                if (selected_item == items.size() - 1) selected_item = 0;
                else selected_item++;
            }

            if (dpad_any_active())
            {
                drawMenu(
                    dialogue_x + padding,
                    dialogue_y + padding + oled.fontHeight(),
                    dialogue_w - (padding * 2), item_height * items.size(),
                    items, selected_item, {}, false, true, colour
                );
            }

            if (dpad_enter_active()) break;

            endLoop();
        }

        oled.fillScreen(BLACK);
        forceRedraw = true;
        return selected_item;
    }

    void controlCentreDialogue()
    {
        // if the control centre is already being shown, don't do anything
        if (showingControlCentre) return;

        // print memory and task info
        watch2::print_memory_details();
        watch2::print_task_details();

        int selected_widget = 3;
        int last_button_widget = 3;
        bool go_back_to_watch_face = false;
        uint8_t last_bt_state = watch2::bluetooth_state;
        uint8_t last_wifi_state = watch2::wifi_state;
        int last_minute = 0;
        bool init = false;

        uint8_t dialogue_radius = 7;
        uint16_t dialogue_width = SCREEN_WIDTH * 0.9;
        uint16_t dialogue_height = SCREEN_HEIGHT * 0.95;
        uint16_t dialogue_x = (SCREEN_WIDTH - dialogue_width) / 2;
        uint16_t dialogue_y = 0;

        showingControlCentre = true;

        uint16_t weather = 0;
        time_t sunrise = 0, sunset = 0;
        getCurrentWeather(weather, sunrise, sunset);

        while(1)
        {
            startLoop();
            //Serial.println("aaaaaaaaaaaaaaaaaaaaa");

            //------------------------
            // handle value updating
            //------------------------

            if (dpad_left_active())
            {
                if (selected_widget > 2)
                {
                    selected_widget--;
                    if (selected_widget < 3) selected_widget = 6;
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
                    if (selected_widget > 6) selected_widget = 3;
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
                if (selected_widget == 2) // torch
                {
                    if (watch2::torch_brightness) watch2::torch_brightness = 0;
                    else watch2::torch_brightness = 255;
                    ledcWrite(TORCH_PWM_CHANNEL, watch2::torch_brightness);
                }
                if (selected_widget == 3) // wifi
                {
                    if (watch2::wifi_state == 0) watch2::enable_wifi();
                    else watch2::disable_wifi();
                }
                if (selected_widget == 4) // bluetooth
                {
                    if (watch2::bluetooth_state == 0) watch2::enable_bluetooth();
                    else watch2::disable_bluetooth();
                }
                if (selected_widget == 5) // ntp
                {
                    getTimeFromNTP();
                }
                if (selected_widget == 6) // shutdown
                {
                    if (messageBox("Are you sure\nyou want to\nshutdown?", {"No", "Yes"})) mcp.digitalWrite(SHUTDOWN_PIN, 0);
                }
            }

            //------------------------
            // draw stuff
            //------------------------

            if(!init || dpad_any_active() || (watch2::wifi_state != last_wifi_state) || (watch2::bluetooth_state != last_bt_state) || (minute() != last_minute) || forceRedraw) {
                
                setFont(SLIGHTLY_BIGGER_FONT);

                int spacing = 10;
                int button_size = 30;
                int weather_icon_size = 30;
                int radius = 10;
                int outline_colour = WHITE;
                int background_colour = BLACK;
                int time_y = dialogue_y + 2;
                int button_x = dialogue_x + spacing;
                int button_y = time_y + oled.fontHeight() + spacing;
                int slider_x = 0;

                if (!init || forceRedraw)
                {
                    // draw dialogue box
                    oled.fillRoundRect(dialogue_x, -dialogue_radius, dialogue_width, dialogue_height, dialogue_radius, BLACK);
                    oled.drawRoundRect(dialogue_x, -dialogue_radius, dialogue_width, dialogue_height, dialogue_radius, themecolour);
                }

                // draw time
                oled.setCursor(button_x, time_y);
                oled.setTextColor(WHITE, BLACK);
                oled.fillRect(button_x, time_y, (dialogue_width / 2), oled.fontHeight(), BLACK);  // hacky
                oled.printf("%02d:%02d", hour(), minute());
                setFont(MAIN_FONT);
                last_minute = minute();

                // draw weather
                uint16_t weather_x = (dialogue_x + dialogue_width) - weather_icon_size - spacing;
                switch(weather / 100)
                {
                    case 2: // thunder
                        drawImage((*watch2::icons)["thunder"], weather_x, time_y);
                        break;
                        
                    case 3: // drizzle
                        if (now() < sunrise || now() > sunset) /* night */
                            drawImage((*watch2::icons)["moon_rain"], weather_x, time_y);
                        else                                   /* day */
                            drawImage((*watch2::icons)["sun_rain"], weather_x, time_y);
                        break;

                    case 5: // rain
                        drawImage((*watch2::icons)["rain"], weather_x, time_y);
                        break;

                    case 6: // snow
                        drawImage((*watch2::icons)["snow"], weather_x, time_y);
                        break;

                    case 7: // atmosphere
                        drawImage((*watch2::icons)["wind"], weather_x, time_y);
                        break;

                    case 8: // clear / clouds
                        if (weather == 800) // clear
                        {
                            if (now() < sunrise || now() > sunset) /* night */
                                drawImage((*watch2::icons)["moon"], weather_x, time_y);
                            else                                   /* day */
                                drawImage((*watch2::icons)["sun"], weather_x, time_y);
                        }
                        else
                        {
                            if (now() < sunrise || now() > sunset) /* night */
                                drawImage((*watch2::icons)["moon_cloud"], weather_x, time_y);
                            else                                   /* day */
                                drawImage((*watch2::icons)["sun_cloud"], weather_x, time_y);
                        }
                        break;

                    default:
                        drawImage((*watch2::icons)["weather_unknown"], weather_x, time_y);
                        break;
                }

                //draw volume slider                                                                                                                    jungle green
                if (selected_widget == 0) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::speaker_volume * ( ( dialogue_width - button_size - spacing - 2 - button_x ) / 21.0 ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0x2D50);                               //background thing
                outline_colour = (selected_widget == 0) ? WHITE : 0x2D50;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, (*watch2::small_icons)["volume"].data(), button_size, button_size, WHITE);                     //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw brightness slider                                                                                                                yellow
                if (selected_widget == 1) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::screen_brightness * ( (float)( dialogue_width - button_size - spacing - 2 - button_x ) / ( 255.0 ) ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, 0xFEC0);                               //background thing
                outline_colour = (selected_widget == 1) ? WHITE : 0xFEC0;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, (*watch2::small_icons)["sun"].data(), button_size, button_size, WHITE);                        //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw torch slider                                                                                                                     adafruit yellow
                if (selected_widget == 2) watch2::oled.fillRect(button_x - 2, button_y - 2, dialogue_width - button_x, button_size + 4, BLACK);
                watch2::oled.drawRoundRect(button_x, button_y + (button_size / 3), dialogue_width - spacing - button_x, button_size / 3, (button_size / 3)/2, WHITE);
                slider_x = button_x + ( watch2::torch_brightness * ( ( dialogue_width - button_size - spacing - 2 - button_x ) / 255.0 ) );
                watch2::oled.fillRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, BLACK);
                watch2::oled.fillRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, YELLOW);                               //background thing
                outline_colour = (selected_widget == 2) ? WHITE : YELLOW;                                                                               //determine outline colour
                watch2::oled.drawRoundRect(slider_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);                       //draw outline
                watch2::oled.drawBitmap(slider_x, button_y, (*watch2::small_icons)["torch_but_smaller"].data(), button_size, button_size, WHITE);          //draw icon
                button_y += spacing + button_size;                                                                                                      //move cursor

                //draw wifi button
                if (watch2::wifi_state == 3) background_colour = 0x867d; // enabled + connected
                if (watch2::wifi_state == 2) background_colour = 0x6E5A; // enabled + connecting
                if (watch2::wifi_state == 1) background_colour = 0x6E5A; // enabled + disconnected
                if (watch2::wifi_state == 0) background_colour = BLACK;  // disabled
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( !(selected_widget == 3) != !(watch2::wifi_state == 3) ) ? 0x867D : WHITE;
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, (*watch2::small_icons)["wifi"].data(), button_size, button_size, (watch2::wifi_state != 0) ? WHITE : 0x867D);
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
                watch2::oled.drawBitmap(button_x, button_y, (*watch2::small_icons)["bluetooth"].data(), button_size, button_size, (watch2::bluetooth_state != 0) ? WHITE : 0x041F);
                button_x += spacing + button_size;
                if (watch2::bluetooth_state != last_bt_state) last_bt_state = watch2::bluetooth_state;

                //draw NTP button
                background_colour = TFT_OLIVE;
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( (selected_widget == 5) ? WHITE : 0x041f);
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, (*watch2::small_icons)["internet_time"].data(), button_size, button_size, WHITE);
                button_x += spacing + button_size;

                //draw shutdown button
                background_colour = TFT_PINK;
                watch2::oled.fillRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, background_colour);
                outline_colour = ( (selected_widget == 6) ? WHITE : TFT_PINK);
                watch2::oled.drawRoundRect(button_x - 2, button_y - 2, button_size + 4, button_size + 4, radius, outline_colour);
                watch2::oled.drawBitmap(button_x, button_y, (*watch2::small_icons)["shutdown"].data(), button_size, button_size, WHITE);
                button_x += spacing + button_size;

                // draw label
                uint16_t label_y = dialogue_y + (dialogue_height - oled.fontHeight() - 5);
                button_x = dialogue_x + spacing;;

                oled.setCursor(button_x, label_y);
                oled.setTextColor(WHITE, BLACK);
                oled.fillRect(button_x, label_y, dialogue_width - (button_x * 2), oled.fontHeight() - 5, BLACK);
                switch(selected_widget)
                {
                    case 0: // sound
                        oled.print("Volume");
                        break;

                    case 1: // backlight
                        oled.print("Brightness");
                        break;

                    case 2: // torch
                        oled.print("Torch");
                        break;

                    case 3: // wifi
                        switch(wifi_state)
                        {
                            case 0: // disabled
                                oled.print("Wifi Disabled");
                                break;

                            case 1: // enabled, disconnected
                                oled.print("Wifi Enabled");
                                break;

                            case 2: // connecting
                                oled.print("Connecting");
                                break;

                            case 3: // connected
                                oled.print(WiFi.SSID());
                                break;

                            case 4: // connect asap
                                oled.print("Connecting Soon");
                                break;
                        }
                        break;

                    case 4: // bluetooth
                        oled.print("Bluetooth");
                        break;

                    case 5: // ntp
                        oled.print("Internet Time");
                        break;

                    case 6: // shutdown
                        oled.print("Shutdown");
                        break;
                }

                init = true;
                forceRedraw = false;
            }

            if (dpad_down_active())
            {
                if (go_back_to_watch_face) break; //watch2::switchState(watch2::state, 0);
            }

            endLoop();

        }

        showingControlCentre = false;
        oled.fillScreen(BLACK);
        forceRedraw = true;
    }

}