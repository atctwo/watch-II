void registerUtilStates()
{

    registerState("Calculator", "calculator", [](){

        static std::vector<String> calc_buttons = {"7",   "8",    "9",    "clr",  "exit",
                                                   "4",   "5",    "6",    "+",    "-",
                                                   "1",   "2",    "3",    "-",    "/",
                                                   "0",   ".",    "(-)",  "<-",   "="};
        static String calculator_expression_thing = "";
        static int columns = 5;
        static int icon_spacing = 3;
        static int icon_width = 22;
        static int icon_height = (2 * icon_spacing) + 8;
        static int radius = 2;
        static int no_icons = 0;
        static int input_colour = WHITE;
        static int selected_calc_button = 4;
        static int16_t x1, y1;
        static uint16_t w=0, h=0;

        int icon_xpos = icon_spacing;
        int icon_ypos = icon_spacing;

        if (dpad_left_active())
        {
            input_colour = WHITE;
            if (selected_calc_button == 0)
            {
                selected_calc_button = calc_buttons.size();
            }
            selected_calc_button--;
        }

        if (dpad_right_active())
        {
            input_colour = WHITE;
            selected_calc_button++;
            if (selected_calc_button == calc_buttons.size())
            selected_calc_button = 0;
        }

        if (dpad_up_active())
        {
            input_colour = WHITE;

            //get selected icon number
            int loop_limit = columns;
            int last = no_icons % columns;
            if (selected_calc_button < 4)
            {
                if (selected_calc_button < last) loop_limit = last;
                else loop_limit = columns + last;
            }

            for (int i=0; i < loop_limit; i++)
            {
                if (selected_calc_button == 0)
                {
                    selected_calc_button = calc_buttons.size();
                }
                selected_calc_button--;
            }
        }

        if (dpad_down_active())
        {
            input_colour = WHITE;

            //get selected icon number
            int loop_limit = columns;
            int last = no_icons % columns;
            if (selected_calc_button >= (no_icons - columns))
            {
                if (selected_calc_button >= (no_icons - last)) loop_limit = last;
                else loop_limit = columns + last;
            }

            for (int i=0; i < loop_limit; i++)
            {
                selected_calc_button++;
                if (selected_calc_button == calc_buttons.size())
                selected_calc_button = 0;
            }
        }

        if (dpad_enter_active())
        {
            input_colour = WHITE;
            if (calc_buttons[selected_calc_button] == "clr") calculator_expression_thing = "";
            else if (calc_buttons[selected_calc_button] == "<-")
            {
                calculator_expression_thing = calculator_expression_thing.substring(0, calculator_expression_thing.length() - 1);
            }
            else if (calc_buttons[selected_calc_button] == "=")
            {
                int error = 0;
                double result = te_interp(calculator_expression_thing.c_str(), &error);
                if (error != 0)
                {
                    input_colour = RED;
                }
                else
                {
                    calculator_expression_thing = String( result );
                }
            }
            else if (calc_buttons[selected_calc_button] != "exit")
            {
                calculator_expression_thing += calc_buttons[selected_calc_button];
            }
        }



        if (dpad_up_active() || dpad_down_active() || dpad_left_active() ||
            dpad_right_active() || dpad_enter_active() || !state_init)
        {
            //draw current input
            oled.drawRoundRect(2, 12, SCREEN_WIDTH - 4, 8 + (2 * icon_spacing), radius, themecolour);
            oled.setCursor(2 + icon_spacing, 12 + icon_spacing + 8);
            oled.setTextColor(input_colour);
            oled.fillRect(2 + icon_spacing, 12 + icon_spacing, SCREEN_WIDTH - 4 - (2 * icon_spacing), 10, BLACK);
            oled.print(calculator_expression_thing);

            //add space for top bar thing (10) and input display (8 + (3 * icon_spacing))
            icon_ypos += 10 + 8 + (3 * icon_spacing);
            //draw state icons
            for (int i = 0; i < calc_buttons.size(); i++)
            {
                oled.setCursor(icon_xpos + (int)(icon_spacing / 2), icon_ypos + 8 + (int)(icon_spacing / 2));
                oled.setTextColor(WHITE);
                oled.print(calc_buttons[i]);

                if (selected_calc_button == i)
                {
                    oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, themecolour);
                }
                else oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, 0x4A49);

                icon_xpos += icon_width + icon_spacing;
                if ((icon_xpos + icon_width) > SCREEN_WIDTH)
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_height + icon_spacing;
                }

                if (!state_init) no_icons++;
            }
        }

        drawTopThing();

        if (dpad_enter_active() && selected_calc_button == 4)
        {
            //exit state
            switchState(2);
        }


    });







}
