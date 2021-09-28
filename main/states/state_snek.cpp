
#include "states.h"
#include <snake.h>

SnakeGame snake(24, 24);
uint16_t grid_column_w = SCREEN_WIDTH / snake.getGridWidth();
uint16_t grid_row_h = SCREEN_HEIGHT / snake.getGridHeight();
uint32_t last_update_time = 0;

uint16_t setting_menu_y = 0;
uint16_t button_y = 0;
uint8_t selected_setting = 0;
std::vector<watch2::settingsMenuData> settings = {
    (watch2::settingsMenuData){"Update Tile", 100, {}, 24},
    (watch2::settingsMenuData){"Loop at Edge", false, {"No", "Yes"}, 24},
    (watch2::settingsMenuData){"Collide with Snake", true, {"No", "Yes"}, 24}
};
std::vector<std::string> buttons = {"Play", "Exit"};

void state_func_snek()
{
    if (watch2::states[watch2::state].variant == 0) // main menu
    {

        if (!watch2::state_init)
        {
            // print title
            watch2::setFont(LARGE_FONT);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.setCursor(0, 0);
            watch2::oled.println("Snek 2");

            // save setting menu y
            setting_menu_y = watch2::oled.fontHeight();

            // reset font size
            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextColor(WHITE, BLACK);

            // calculate button y
            button_y = setting_menu_y + (5 * watch2::oled.fontHeight());
        }

        if (dpad_up_active())
        {
            if (selected_setting == 0) selected_setting = (settings.size() + 2) - 1;
            else selected_setting--;
        }

        if (dpad_down_active())
        {
            if (selected_setting == (settings.size() + 2) - 1) selected_setting = 0;
            else selected_setting++;
        }

        if (dpad_left_active())
        {
            switch(selected_setting)
            {
                case 0: // update time
                    if (settings[0].setting_value > 0) settings[0].setting_value -= 50;
                    break;

                case 1: // loop at edge
                    settings[1].setting_value = !settings[1].setting_value;
                    snake.setLoopAtEdge(settings[1].setting_value);
                    break;

                case 2: // collide with snake
                    settings[2].setting_value = !settings[2].setting_value;
                    snake.setCollideWithSnake(settings[2].setting_value);
                    break;

            }
        }

        if (dpad_right_active())
        {
            switch(selected_setting)
            {
                case 0: // update time
                    settings[0].setting_value += 50;
                    break;

                case 1: // loop at edge
                    settings[1].setting_value = !settings[1].setting_value;
                    snake.setLoopAtEdge(settings[1].setting_value);
                    break;

                case 2: // collide with snake
                    settings[2].setting_value = !settings[2].setting_value;
                    snake.setCollideWithSnake(settings[2].setting_value);
                    break;

            }
        }

        if (dpad_enter_active())
        {
            if (selected_setting == 3) // play game
            {
                watch2::switchState(watch2::state, 1);
            }

            if (selected_setting == 4) // exit
            {
                watch2::switchState(-1);
            }
        }

        draw(dpad_any_active(), {

            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::drawSettingsMenu(2, setting_menu_y, SCREEN_WIDTH - 4, SCREEN_HEIGHT - setting_menu_y, settings, selected_setting);
            watch2::drawMenu(2, button_y, SCREEN_WIDTH - 4, SCREEN_HEIGHT - button_y - setting_menu_y, buttons, selected_setting - settings.size(), {}, false, true);

        });

    }

    else if (watch2::states[watch2::state].variant == 1) // snek
    {
        if (!watch2::state_init)
        {
            // reset snake game
            snake.reset();
        }

        // set snake direction
        if (dpad_up_active())           snake.setSnakeDirection(SNAKE_DIRECTION_UP);
        if (dpad_down_active())         snake.setSnakeDirection(SNAKE_DIRECTION_DOWN);
        if (dpad_left_active())         snake.setSnakeDirection(SNAKE_DIRECTION_LEFT);
        if (dpad_right_active())        snake.setSnakeDirection(SNAKE_DIRECTION_RIGHT);

        // draw everything
        if (millis() - last_update_time > settings[0].setting_value)
        {
            // print score
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(0, 0);
            watch2::oled.fillRect(0, 0, SCREEN_WIDTH / 2, watch2::oled.fontHeight(), BLACK);
            watch2::oled.printf("Score: %d", snake.getScore());

            // clear last snake segment
            std::vector<std::pair<int16_t, int16_t>> segments = snake.getSnakeSegments();
            std::pair<int16_t, int16_t> segment = segments[segments.size() - 1];
            watch2::oled.fillRect(
                segment.first * grid_column_w, 
                segment.second * grid_row_h, 
                grid_column_w, grid_row_h, BLACK
            );

            // update the snek
            snake.update();

            // // draw grid
            // for (int y = 0; y < snake.getGridHeight(); y++)
            // {
            //     for (int x = 0; x < snake.getGridWidth(); x++)
            //     {
            //         watch2::oled.drawRect(x * grid_column_w, y * grid_row_h, grid_column_w, grid_row_h, TFT_DARKGREY);
            //     }
            // }

            // draw snake segments
            segments = snake.getSnakeSegments();
            for (std::pair<int16_t, int16_t> segment : segments)
            {
                watch2::oled.fillRect(
                    segment.first * grid_column_w, 
                    segment.second * grid_row_h, 
                    grid_column_w, grid_row_h, BLUE
                );
            }

            // draw food
            std::vector<std::pair<int16_t, int16_t>> food = snake.getFood();
            for (std::pair<int16_t, int16_t> f : food)
            {
                watch2::oled.fillRect(
                    f.first * grid_column_w, 
                    f.second * grid_row_h, 
                    grid_column_w, grid_row_h, RED
                );
            }

            // reset update time
            last_update_time = millis();
        }

        // detect game over
        if (snake.isGameOver())
        {
            uint8_t selected_menu_item = watch2::popup_menu("Game Over", {"Play Again", "Exit Game"});
            if (selected_menu_item == 0) snake.reset();
            else if (selected_menu_item == 1) watch2::switchState(watch2::state, 0);
        }

        // detect io0 button
        if (watch2::btn_zero.wasPressed())
        {
            uint8_t selected_menu_item = watch2::popup_menu("Paused", {"Resume", "Restart", "Exit Game"});
            if (selected_menu_item == 1) snake.reset();
            if (selected_menu_item == 2) watch2::switchState(watch2::state, 0);
        }
    }

}