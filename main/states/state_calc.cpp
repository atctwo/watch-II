#include "../watch2.h"

void state_func_calc()
{
    static std::vector<String> calc_buttons = {"7",   "8",    "9",    "clr",  "exit", "(",    ")",    "pi",   "e",     "ln",
                                                   "4",   "5",    "6",    "+",    "-",    "abs",  "sin",  "asin", "sqrt",  "log",
                                                   "1",   "2",    "3",    "*",    "/",    "ceil", "cos",  "acos", "^",     "10^",
                                                   "0",   ".",    "",     "<-",   "=",    "flr",  "tan",  "atan", "%",     "fac"};
    static String calculator_expression_thing = "";
    static int columns = 5;
    static int pages = 2;
    static int icon_spacing = 6;
    static int radius = 10;
    static int no_icons = 0;
    static int input_colour = WHITE;
    static int selected_calc_button = 4;
    static int16_t x1, y1;
    static uint16_t w=0, h=0;
    static int last_page_number = -1;

    static int icon_width = ( SCREEN_WIDTH - ( ( columns + 1 ) * icon_spacing ) ) / columns;
    static int icon_height = (2 * icon_spacing) + watch2::oled.fontHeight();

    int icon_xpos = icon_spacing;
    int icon_ypos = icon_spacing;
    int total_icons_per_row = columns * pages;



    

    if (dpad_up_active())
    {if (dpad_left_active())
    {
        input_colour = WHITE;
        while(1)
        {
            if (selected_calc_button == 0)
            selected_calc_button = calc_buttons.size()-1;
            else selected_calc_button--;
            if (calc_buttons[selected_calc_button] != "") break;
        }
    }

    if (dpad_right_active())
    {
        input_colour = WHITE;
        while(1)
        {
            selected_calc_button++;
            if (selected_calc_button == calc_buttons.size())
            selected_calc_button = 0;
            if (calc_buttons[selected_calc_button] != "") break;
        }
    }
        input_colour = WHITE;

        //get selected icon number
        int loop_limit = total_icons_per_row;
        int last = no_icons % total_icons_per_row;
        if (selected_calc_button < 4)
        {
            if (selected_calc_button < last) loop_limit = last;
            else loop_limit = total_icons_per_row + last;
        }

        // loop to calculated button
        for (int i=0; i < loop_limit; i++)
        {
            if (selected_calc_button == 0)
            {
                selected_calc_button = calc_buttons.size();
            }
            selected_calc_button--;
        }

        // if button has no text, find next button with text
        if (calc_buttons[selected_calc_button] == "") while(1)
        {
            if (selected_calc_button == 0)
            selected_calc_button = calc_buttons.size();
            else selected_calc_button--;
            if (calc_buttons[selected_calc_button] != "") break;
        }
    }

    if (dpad_down_active())
    {
        input_colour = WHITE;

        //get selected icon number
        int loop_limit = total_icons_per_row;
        int last = no_icons % total_icons_per_row;
        if (selected_calc_button >= (no_icons - total_icons_per_row))
        {
            if (selected_calc_button >= (no_icons - last)) loop_limit = last;
            else loop_limit = total_icons_per_row + last;
        }

        // loop to reach calculated button
        for (int i=0; i < loop_limit; i++)
        {
            selected_calc_button++;
            if (selected_calc_button == calc_buttons.size())
            selected_calc_button = 0;
        }
        //if button has no text, find next button with text
        if (calc_buttons[selected_calc_button] == "") while(1)
        {
            selected_calc_button++;
            if (selected_calc_button == calc_buttons.size())
            selected_calc_button = 0;
            if (calc_buttons[selected_calc_button] != "") break;
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
        else if (calc_buttons[selected_calc_button] == "flr") calculator_expression_thing += "floor";
        else if (calc_buttons[selected_calc_button] == "10^") calculator_expression_thing += "*(10^";
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
        dpad_right_active() || dpad_enter_active() || !watch2::state_init)
    {
        //draw current input
        watch2::oled.drawRoundRect(2, watch2::top_thing_height, SCREEN_WIDTH - 4, watch2::oled.fontHeight() + (2 * icon_spacing), radius, watch2::themecolour);
        watch2::oled.setCursor(2 + icon_spacing, watch2::top_thing_height + icon_spacing);
        watch2::oled.setTextColor(input_colour, BLACK);
        watch2::oled.fillRect(2 + icon_spacing, watch2::top_thing_height + icon_spacing, SCREEN_WIDTH - 4 - (2 * icon_spacing), watch2::oled.fontHeight(), BLACK);
        watch2::oled.print(calculator_expression_thing);

        //add space for top bar thing (10) and input display (8 + (3 * icon_spacing))
        icon_ypos += watch2::top_thing_height + watch2::oled.fontHeight() + (3 * icon_spacing);

        //calculate page of selected item
        int selected_item_column = selected_calc_button % total_icons_per_row;
        int selected_page_number = floor( selected_item_column / columns );

        //clear matrix on page change
        if (selected_page_number != last_page_number)
        {
            int matrix_width = icon_spacing;
            int matrix_height = icon_spacing + watch2::top_thing_height + watch2::oled.fontHeight() + (3 * icon_spacing);
            watch2::oled.fillRect(matrix_width, matrix_height, SCREEN_WIDTH - matrix_width, SCREEN_HEIGHT - matrix_height, BLACK);
            last_page_number = selected_page_number;
        }

        //draw state icons
        for (int i = 0; i < calc_buttons.size(); i++)
        {
            //calculate page of current item
            int item_column = i % total_icons_per_row;
            int page_number = floor( item_column / columns );

            //if (calc_buttons[i] == "4") calculator_expression_thing += String(i) + String(" % ") + String(total_icons_per_row);

            if (page_number == selected_page_number) //if item is on same page as selected item
            {
                // if item has text
                if (calc_buttons[i] != "")
                {
                    //print button text
                    watch2::oled.setTextDatum(MC_DATUM);
                    watch2::oled.setTextColor(WHITE, BLACK);
                    watch2::oled.drawString(calc_buttons[i], icon_xpos + (icon_width / 2), icon_ypos + (icon_height / 2));
                    //watch2::oled.setTextDatum(TL_DATUM);

                    //print outline
                    if (selected_calc_button == i)
                    {
                        watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, watch2::themecolour);
                    }
                    else watch2::oled.drawRect(icon_xpos-1, icon_ypos-1, icon_width+1, icon_height+1, 0x4A49);
                }

                //calculate position of next item
                icon_xpos += icon_width + icon_spacing;
                if ((item_column + 1) >= columns * (page_number + 1))
                {
                    icon_xpos = icon_spacing;
                    icon_ypos += icon_height + icon_spacing;
                }

                //calculator_expression_thing += String(calc_buttons[i]) + String(" ");
            }

            if (!watch2::state_init) no_icons++;
        }
    }

    watch2::drawTopThing();

    if (dpad_enter_active() && selected_calc_button == 4)
    {
        //exit state
        watch2::oled.setTextDatum(TL_DATUM);
        watch2::switchState(2);
    }
}