
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
    static uint16_t *dma_buffer;
    static uint32_t dma_buffer_length = SCREEN_WIDTH;

    static int game_x = (SCREEN_WIDTH - AGNES_SCREEN_WIDTH) / 2;
    static int game_y  = 0;

    uint16_t x = 0;
    uint16_t y = 0;
    uint32_t dma_buffer_pos = 0;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect("/");
        if (filename.compare("canceled") == 0) watch2::switchState(2);
        else
        {
            Serial.println("1");
            game_data_size = 0;
            game_data = read_nes_rom(filename.c_str(), &game_data_size);

            Serial.println("2");
            agnes = agnes_make();
            agnes_load_ines_data(agnes, game_data, game_data_size);
            agnes_loaded = true;

            Serial.println("3");

            dma_buffer = (uint16_t*) heap_caps_malloc(dma_buffer_length * sizeof(uint16_t), MALLOC_CAP_DMA);
            if (!dma_buffer) Serial.println("\0[31mfailed to allocate memory for dma buffer\0[0m");

            Serial.println("4");
        }
    }

    if (agnes_loaded)
    {
        while(true)
        {
            //Serial.println("a");
            //wait until DMA has finished
            watch2::oled.dmaWait();

            //Serial.println("b");
            agnes_next_frame(agnes);
            //Serial.println("c");

            // if (draw_frame == 0)
            // {
                
                //Serial.println("d");
                
                watch2::oled.startWrite();
                watch2::oled.setSwapBytes(true);
                watch2::oled.setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

                //Serial.println("d2");

                for (uint16_t p = 0; p < (AGNES_SCREEN_WIDTH * AGNES_SCREEN_HEIGHT); p++)
                {
                    //Serial.println("A");

                    x = p % AGNES_SCREEN_WIDTH;
                    y = p / AGNES_SCREEN_WIDTH;
                    
                    //Serial.println("B");

                    uint8_t color_ix = agnes->ppu.screen_buffer[p];
                    agnes_color_t c = g_colors[color_ix & 0x3f];

                    //Serial.println("C");

                    // save pixels to buffer
                    uint16_t screen_x = game_x + x;
                    if (screen_x >= 0 && screen_x < SCREEN_WIDTH - 1) 
                    {
                        //uint16_t index = ((y * SCREEN_WIDTH) + screen_x);
                        //printf("%d is the index we're writing the colour to\n", index);
                        dma_buffer[screen_x] = watch2::oled.color565(c.r, c.g, c.b);
                    }

                    //Serial.println("D");

                    // draw pixels to screen
                    //watch2::oled.drawPixel(game_x + x, game_y + y, watch2::oled.color565(c.r, c.g, c.b));
                    if (screen_x == dma_buffer_length - 1) watch2::oled.pushPixelsDMA(dma_buffer, dma_buffer_length - 1);

                    //Serial.println("E");
                    
                }

                //Serial.println("e");
                
                //wait until DMA has finished
                //watch2::oled.dmaWait();
                //watch2::oled.pushPixelsDMA(dma_buffer_top, dma_buffer_length - 1);
                //watch2::oled.pushPixelsDMA(dma_buffer_bottom, dma_buffer_length - 1);

                //Serial.println("f");

                watch2::oled.endWrite();
                watch2::oled.setSwapBytes(false);
                draw_frame += 1;

                //Serial.println("g");
            // }
            // else
            // {
            //     if (draw_frame == 10) draw_frame = 0;
            //     else draw_frame += 1;
            // }
            


            if (digitalRead(dpad_enter)) break;
        }
    }

    if (dpad_any_active())
    {
        agnes_destroy(agnes);
        agnes_loaded = false;
        free(dma_buffer);
        watch2::switchState(2);
    }


}