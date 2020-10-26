
#include "../watch2.h"

#define AGNES_IMPLEMENTATION
#include "../libraries/agnes.h"

static void *read_nes_rom(const char *filename, size_t *out_len)
{
    Serial.printf("[nes] reading file %s...\n", filename);

    File f = watch2::SD.open(filename, 0);
    unsigned char *file_contents = (unsigned char*) malloc(f.size());
    *out_len = f.size();

    int chr;
    uint16_t pos = 0;

    while(chr != -1)
    {
        chr = f.read();
        //Serial.printf("%x ", chr);
        if (chr != -1) file_contents[pos++] = chr;
    }

    f.rewind();
    f.close();

    Serial.println("\n[nes] finished");
    return file_contents;
}

void state_func_nes()
{
    static std::string filename = "";
    static size_t game_data_size = 0;
    static void *game_data;
    static agnes_t *agnes;
    static bool agnes_loaded = false;
    static uint8_t draw_frame = 0;

    static int game_x = (SCREEN_WIDTH - AGNES_SCREEN_WIDTH) / 2;
    static int game_y  = 0;

    uint16_t x = 0;
    uint16_t y = 0;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect("/");
        if (filename.compare("canceled") == 0) watch2::switchState(2);
        else
        {
            game_data_size = 0;
            game_data = read_nes_rom(filename.c_str(), &game_data_size);

            agnes = agnes_make();
            agnes_load_ines_data(agnes, game_data, game_data_size);

            agnes_loaded = true;
        }
    }

    if (agnes_loaded)
    {
        while(true)
        {
            agnes_next_frame(agnes);

            if (draw_frame == 0)
            {
                watch2::oled.startWrite();
                watch2::oled.setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

                for (uint16_t p = 0; p < (AGNES_SCREEN_WIDTH * AGNES_SCREEN_HEIGHT); p++)
                {
                    x = p % AGNES_SCREEN_WIDTH;
                    y = p / AGNES_SCREEN_WIDTH;
                    
                    uint8_t color_ix = agnes->ppu.screen_buffer[p];
                    agnes_color_t c = g_colors[color_ix & 0x3f];

                    //watch2::oled.drawPixel(game_x + x, game_y + y, watch2::oled.color565(c.r, c.g, c.b));
                    uint16_t screen_x = game_x + x;
                    if (x >= 0 && x < SCREEN_WIDTH - 1) watch2::oled.pushBlock(watch2::oled.color565(c.r, c.g, c.b), 1);
                }

                watch2::oled.endWrite();
                draw_frame += 1;
            }
            else
            {
                if (draw_frame == 10) draw_frame = 0;
                else draw_frame += 1;
            }
            


            if (digitalRead(dpad_enter)) break;
        }
    }

    if (dpad_any_active())
    {
        agnes_destroy(agnes);
        agnes_loaded = false;
        watch2::switchState(2);
    }


}