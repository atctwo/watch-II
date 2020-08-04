#include "../watch2.h"
#include "../libraries/libtris/src/libtris.h"

uint16_t popup_menu(const char *title, std::vector<std::string> items, uint16_t colour=watch2::themecolour)
{
    uint16_t selected_item = 0;
    uint16_t padding = 4;
    uint16_t item_height = (3 * padding) + watch2::oled.fontHeight();

    uint16_t dialogue_w = 150;
    uint16_t dialogue_h = (2 * padding) + watch2::oled.fontHeight() + (items.size() * item_height);
    uint16_t dialogue_x = (SCREEN_WIDTH / 2) - (dialogue_w / 2);
    uint16_t dialogue_y = (SCREEN_HEIGHT / 2) - (dialogue_h / 2);
    uint16_t dialogue_r = 10;

    watch2::oled.fillRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, BLACK);
    watch2::oled.drawRoundRect(dialogue_x, dialogue_y, dialogue_w, dialogue_h, dialogue_r, colour);

    watch2::oled.setTextDatum(TC_DATUM);
    watch2::oled.drawString(title, dialogue_x + (dialogue_w / 2), dialogue_y + padding);

    watch2::drawMenu(
        dialogue_x + padding,
        dialogue_y + padding + watch2::oled.fontHeight(),
        dialogue_w - (padding * 2), item_height * items.size(),
        items, selected_item, false, true, colour
    );

    while(1)
    {
        watch2::startLoop();

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
            watch2::drawMenu(
                dialogue_x + padding,
                dialogue_y + padding + watch2::oled.fontHeight(),
                dialogue_w - (padding * 2), item_height * items.size(),
                items, selected_item, false, true, colour
            );
        }

        if (dpad_enter_active()) break;

        watch2::endLoop();
    }

    watch2::oled.fillScreen(BLACK);
    watch2::forceRedraw = true;
    return selected_item;
}

void state_func_tetris()
{

    static uint16_t matrix_width = 10;
    static uint16_t matrix_height = 20;
    static uint16_t vanish_zone_height = 20;
    static uint16_t block_size = 0;
    static uint16_t matrix_width_px = 0, matrix_height_px = 0;
    static uint16_t matrix_x = 0, matrix_y = 0;
    static uint32_t frame_start = 0, frame_end = 0;
    static uint16_t dt = 0;
    static bool paused = false;
    static uint32_t last_score = 0;
    static uint16_t last_level = 0, last_lines = 0, last_combo = 0;
    static uint16_t tetrimino_colours[14] = {YELLOW, 0x87FF, ORANGE, BLUE, GREEN, RED, TFT_PURPLE, 0xfff0, 0xBFFF, 0xFF53, 0xB5BF, 0xBFF7, 0xFDF7, 0xDDFF};
    static libtris<uint16_t> tetris(matrix_width, matrix_height + vanish_zone_height, vanish_zone_height, 19, tetrimino_colours);
    block_info<uint16_t> **matrix = tetris.getMatrix();
    

    if (watch2::states[watch2::state].variant == 0) // menu
    {
        if (!watch2::state_init)
        {
            // watch2::oled.setCursor(0, watch2::top_thing_height);
            // watch2::oled.setTextColor(RED, BLACK);          watch2::oled.print("T");
            // watch2::oled.setTextColor(ORANGE, BLACK);       watch2::oled.print("E");
            // watch2::oled.setTextColor(YELLOW, BLACK);       watch2::oled.print("T");
            // watch2::oled.setTextColor(GREEN, BLACK);        watch2::oled.print("R");
            // watch2::oled.setTextColor(CYAN, BLACK);         watch2::oled.print("I");
            // watch2::oled.setTextColor(PURPLE, BLACK);       watch2::oled.print("S\n");

            watch2::oled.pushImage(35, 10, 170, 118, watch2::icons["tetris_logo"].data());

            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(0, SCREEN_HEIGHT - (2 * watch2::oled.fontHeight()));
            watch2::oled.print("press enter to start\npress left to exit");
        }

        if (dpad_left_active())
        {
            watch2::switchState(2);
        }

        if (dpad_enter_active())
        {
            watch2::switchState(watch2::state, 1);
        }

    }

    else if (watch2::states[watch2::state].variant == 1) // tetris
    {
        if (!watch2::state_init)
        {
            block_size = 0;
            if (matrix_height >= matrix_width) block_size = SCREEN_HEIGHT / matrix_height;
            else                               block_size = SCREEN_WIDTH  / matrix_width;

            matrix_width_px = matrix_width * block_size;
            matrix_height_px = matrix_height * block_size;

            matrix_x = (SCREEN_WIDTH / 2) - (matrix_width_px / 2);
            matrix_y = (SCREEN_HEIGHT / 2) - (matrix_height_px / 2);

            tetris.startGame();
        }

        frame_start = millis();

        tetris.setMovingLeft(digitalRead(dpad_left));
        tetris.setMovingRight(digitalRead(dpad_right));
        tetris.setMovingDown(digitalRead(dpad_down));
        if (dpad_enter_active()) tetris.hardDrop();
        if (dpad_up_active()) tetris.rotateClockwise();

        tetris.update(dt);

        for (uint16_t y = 0; y < tetris.getVisibleMatrixHeight(); y++)
        {
            for (uint16_t x = 0; x < tetris.getMatrixWidth(); x++)
            {
                // filled in bit
                if (!paused) watch2::oled.fillRect(
                    matrix_x + (x * block_size),
                    matrix_y + (y * block_size),
                    block_size, block_size, matrix[x][y+vanish_zone_height].colour
                );

                // outline
                watch2::oled.drawFastVLine(matrix_x + (x * block_size), matrix_y + (y * block_size), block_size, WHITE);
                watch2::oled.drawFastHLine(matrix_x + (x * block_size), matrix_y + (y * block_size), block_size, WHITE);
                if (y == tetris.getVisibleMatrixHeight() - 1) watch2::oled.drawFastHLine(matrix_x + (x * block_size), matrix_y + (y * block_size) + block_size, block_size, WHITE);
                if (x == tetris.getMatrixWidth() - 1)         watch2::oled.drawFastVLine(matrix_x + (x * block_size) + block_size, matrix_y + (y * block_size), block_size, WHITE);
            }
        }

        uint8_t next_block_count = 6;
        uint16_t next_blocks_x = matrix_x + matrix_width_px + (block_size / 2);
        uint16_t next_blocks_h = (2.5 * block_size);
        uint16_t total_next_blocks_h = next_blocks_h * next_block_count;

        block_info<uint16_t> **next_blocks = tetris.getNextBlocks(next_block_count);
        for (uint8_t i = 0; i < next_block_count; i++)
        {
            for (uint16_t pos = 0; pos < 8; pos++)
            {
                uint8_t x = pos % 4;
                uint8_t y = pos / 4;

                // filled in bit
                if (!paused) watch2::oled.fillRect(
                    next_blocks_x + (x * block_size),
                    (i *next_blocks_h) + (y * block_size),
                    block_size, block_size, next_blocks[i][pos].colour
                );

                // outline
                watch2::oled.drawFastHLine(next_blocks_x + (x * block_size), (i * next_blocks_h) + (y * block_size), block_size, WHITE);
                watch2::oled.drawFastVLine(next_blocks_x + (x * block_size), (i * next_blocks_h) + (y * block_size), block_size, WHITE);
                if(x == 3) watch2::oled.drawFastVLine(next_blocks_x + (x * block_size) + block_size, (i * next_blocks_h) + (y * block_size), block_size, WHITE);
                if(y == 1) watch2::oled.drawFastHLine(next_blocks_x + (x * block_size), (i * next_blocks_h) + (y * block_size) + block_size, block_size, WHITE);
            }
        }
        free(next_blocks);

        draw(tetris.getScore() != last_score, {
            watch2::oled.setCursor(0, 0);
            watch2::oled.fillRect(0, watch2::oled.fontHeight(), matrix_x, watch2::oled.fontHeight(), BLACK);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println("Score");
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.println(tetris.getScore());
            last_score = tetris.getScore();
        });

        draw(tetris.getLevel() != last_level, {
            watch2::oled.setCursor(0, 2 * watch2::oled.fontHeight());
            watch2::oled.fillRect(0, 3 * watch2::oled.fontHeight(), matrix_x, watch2::oled.fontHeight(), BLACK);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println("Level");
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.println(tetris.getLevel());
            last_level = tetris.getLevel();
        });

        draw(tetris.getLinesLeft() != last_lines, {
            watch2::oled.setCursor(0, 4 * watch2::oled.fontHeight());
            watch2::oled.fillRect(0, 5 * watch2::oled.fontHeight(), matrix_x, watch2::oled.fontHeight(), BLACK);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.println("Goal");
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.println(tetris.getLinesLeft());
            last_lines = tetris.getLinesLeft();
        });

        if (tetris.getCombo() > 1)
        {
            draw(tetris.getCombo() != last_combo, {
                watch2::oled.setCursor(0, 6 * watch2::oled.fontHeight());
                watch2::oled.fillRect(0, 7 * watch2::oled.fontHeight(), matrix_x, watch2::oled.fontHeight(), BLACK);
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.println("Combo");
                watch2::oled.setTextColor(watch2::themecolour, BLACK);
                watch2::oled.println(tetris.getCombo());
                last_combo = tetris.getCombo();
            });
        }
        else
        {
            watch2::oled.fillRect(0, 6 * watch2::oled.fontHeight(), matrix_x, 2 * watch2::oled.fontHeight(), BLACK);
        }


        
        if (watch2::btn_zero.wasPressed())
        {
            uint8_t selected_menu_item = popup_menu("Paused", {"Resume", "Exit Game"});
            if (selected_menu_item == 1)
            {
                watch2::switchState(watch2::state, 0);
            }
        }

        frame_end = millis();
        dt = frame_end - frame_start;
    }

}