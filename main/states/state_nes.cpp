
#include "../watch2.h"
#include "state_nes.h"

extern "C" {
    #include <nofrendo.h>
    #include <noftypes.h>
    #include <nofconfig.h>
    #include <log.h>
    #include <osd.h>
    #include <bitmap.h>
    #include <event.h>
    #include <gui.h>
    #include <log.h>
    #include <nes.h>
    #include <nes_pal.h>
    #include <nesinput.h>
    #include <version.h>
}

#define  DEFAULT_SAMPLERATE   22100
#define  DEFAULT_FRAGSIZE     128

#define  DEFAULT_WIDTH        256
#define  DEFAULT_HEIGHT       NES_VISIBLE_HEIGHT

#define CONFIG_SOUND_ENA      1

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "driver/i2s.h"
#include "sdkconfig.h"

TimerHandle_t nes_timer;
std::string rom_path = "";
char configfilename[]="na";

//------------------------------------------
// nofrendo osd function implementations
//------------------------------------------

extern "C" {

    void *mem_alloc(int size, bool fast_mem)
    {
        if (fast_mem)
        {
            return heap_caps_malloc(size, MALLOC_CAP_8BIT);
        }
        else
        {
            return heap_caps_malloc_prefer(size, MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT);
        }
    }

    char *osd_getromdata()
    {
        Serial.printf("[nes] reading file %s...\n", rom_path.c_str());

        fs::File f = SD.open(rom_path.c_str());
        char *file_contents = NULL;// = (unsigned char*) malloc(f.size());
        //*out_len = f.size();

        Serial.printf("[nes] file size: %d\n", f.size());

        int chr;
        uint16_t pos = 0;

        while(chr != -1)
        {
            chr = f.read();
            //Serial.printf("%x ", chr);
            //if (chr != -1) file_contents[pos++] = chr;
        }

        f.seek(0);
        f.close();

        Serial.println("\n[nes] finished");
        return file_contents;
    }


    // int nofrendo_main(int argc, char *argv[])
    // {
    //     Serial.println("this is broken");
    // }

    /* This is os-specific part of main() */
    int osd_main(int argc, char *argv[])
    {
    config.filename = configfilename;

    return main_loop("rom", system_autodetect);
    }

    /* File system interface */
    void osd_fullname(char *fullname, const char *shortname)
    {
    strncpy(fullname, shortname, PATH_MAX);
    }

    /* This gives filenames for storage of saves */
    char *osd_newextension(char *string, char *ext)
    {
    return string;
    }

    /* This gives filenames for storage of PCX snapshots */
    int osd_makesnapname(char *filename, int len)
    {
    return -1;
    }

    static void drawBmpLine(int, int, int, int, const uint8_t **line)
    {
        // not finished
    }

    //Seemingly, this will be called only once. Should call func with a freq of frequency,
    int osd_installtimer(int frequency, void *func, int funcsize, void *counter, int countersize)
    {
        printf("Timer install, freq=%d\n", frequency);
        nes_timer=xTimerCreate("nes",configTICK_RATE_HZ/frequency, pdTRUE, NULL, (TimerCallbackFunction_t) func);
        xTimerStart(nes_timer, 0);
    return 0;
    }

    static int logprint(const char *string)
    {
    return Serial.printf("%s", string);
    }




    //------------------------------------------
    // nofrendo audio stuff
    //------------------------------------------

    static void (*audio_callback)(void *buffer, int length) = NULL;
    #if CONFIG_SOUND_ENA
    QueueHandle_t queue;
    static uint16_t *audio_frame;
    #endif

    static void do_audio_frame() {

        // #if CONFIG_SOUND_ENA
        //     int left=DEFAULT_SAMPLERATE/NES_REFRESH_RATE;
        //     while(left) {
        //         int n=DEFAULT_FRAGSIZE;
        //         if (n>left) n=left;
        //         audio_callback(audio_frame, n); //get more data
        //         //16 bit mono -> 32-bit (16 bit r+l)
        //         for (int i=n-1; i>=0; i--) {
        //             audio_frame[i*2+1]=audio_frame[i];
        //             audio_frame[i*2]=audio_frame[i];
        //         }
        //         //i2s_write_bytes(0, audio_frame, 4*n, portMAX_DELAY);
        //         left-=n;
        //     }
        // #endif
    }

    void osd_setsound(void (*playfunc)(void *buffer, int length))
    {
    //Indicates we should call playfunc() to get more data.
    audio_callback = playfunc;
    }

    static void osd_stopsound(void)
    {
    audio_callback = NULL;
    }


    static int osd_init_sound(void)
    {
    #if CONFIG_SOUND_ENA

        // watch2: i2s audio should already be configured

        // audio_frame=malloc(4*DEFAULT_FRAGSIZE);
        // i2s_config_t cfg={
        // 	.mode=I2S_MODE_DAC_BUILT_IN|I2S_MODE_TX|I2S_MODE_MASTER,
        // 	.sample_rate=DEFAULT_SAMPLERATE,
        // 	.bits_per_sample=I2S_BITS_PER_SAMPLE_16BIT,
        // 	.channel_format=I2S_CHANNEL_FMT_RIGHT_LEFT,
        // 	.communication_format=I2S_COMM_FORMAT_I2S_MSB,
        // 	.intr_alloc_flags=0,
        // 	.dma_buf_count=4,
        // 	.dma_buf_len=512
        // };
        // i2s_driver_install(0, &cfg, 4, &queue);
        // i2s_set_pin(0, NULL);
        // i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN); 

        // //I2S enables *both* DAC channels; we only need DAC1.
        // //ToDo: still needed now I2S supports set_dac_mode?
        // CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_DAC_XPD_FORCE_M);
        // CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC_M);

    #endif

        audio_callback = NULL;

        return 0;
    }

    void osd_getsoundinfo(sndinfo_t *info)
    {
    info->sample_rate = DEFAULT_SAMPLERATE;
    info->bps = 16;
    }





    //------------------------------------------
    // nofrendo video stuff
    //------------------------------------------
    static int nes_video_init(int width, int height);
    static void nes_video_shutdown(void);
    static int nes_video_set_mode(int width, int height);
    static void nes_video_set_palette(rgb_t *pal);
    static void nes_video_clear(uint8 color);
    static bitmap_t *nes_video_lock_write(void);
    static void nes_video_free_write(int num_dirties, rect_t *dirty_rects);
    static void nes_video_custom_blit(bitmap_t *bmp, int num_dirties, rect_t *dirty_rects);
    static char nes_video_fb[1]; //dummy

    QueueHandle_t vidQueue;

    viddriver_t sdlDriver =
    {
    "Simple DirectMedia Layer",         /* name */
    nes_video_init,          /* init */
    nes_video_shutdown,      /* shutdown */
    nes_video_set_mode,      /* set_mode */
    nes_video_set_palette,   /* set_palette */
    nes_video_clear,         /* clear */
    nes_video_lock_write,    /* lock_write */
    nes_video_free_write,    /* free_write */
    nes_video_custom_blit,   /* custom_blit */
    false          /* invalidate flag */
    };


    bitmap_t *myBitmap;

    void osd_getvideoinfo(vidinfo_t *info)
    {
    info->default_width = DEFAULT_WIDTH;
    info->default_height = DEFAULT_HEIGHT;
    info->driver = &sdlDriver;
    }

    /* flip between full screen and windowed */
    void osd_togglefullscreen(int code)
    {
    }

    /* initialise video */
    static int nes_video_init(int width, int height)
    {
        return 0;
    }

    static void nes_video_shutdown(void)
    {
    }

    /* set a video mode */
    static int nes_video_set_mode(int width, int height)
    {
    return 0;
    }

    uint16 myPalette[256];

    /* copy nes palette over to hardware */
    static void nes_video_set_palette(rgb_t *pal)
    {
        uint16 c;

    int i;

    for (i = 0; i < 256; i++)
    {
        c=(pal[i].b>>3)+((pal[i].g>>2)<<5)+((pal[i].r>>3)<<11);
        //myPalette[i]=(c>>8)|((c&0xff)<<8);
        myPalette[i]=c;
    }

    }

    /* clear all frames to a particular color */
    static void nes_video_clear(uint8 color)
    {
    //   SDL_FillRect(mySurface, 0, color);
    }



    /* acquire the directbuffer for writing */
    static bitmap_t *nes_video_lock_write(void)
    {
    //   SDL_LockSurface(mySurface);
    myBitmap = bmp_createhw((uint8*)nes_video_fb, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_WIDTH*2);
    return myBitmap;
    }

    /* release the resource */
    static void nes_video_free_write(int num_dirties, rect_t *dirty_rects)
    {
    bmp_destroy(&myBitmap);
    }


    static void nes_video_custom_blit(bitmap_t *bmp, int num_dirties, rect_t *dirty_rects) {
        xQueueSend(vidQueue, &bmp, 0);
        do_audio_frame();
    }


    //This runs on core 1.
    static void videoTask(void *arg) {
        int x, y;
        bitmap_t *bmp=NULL;
        x = (320-DEFAULT_WIDTH)/2;
        y = ((240-DEFAULT_HEIGHT)/2);
        while(1) {
    //		xQueueReceive(vidQueue, &bmp, portMAX_DELAY);//skip one frame to drop to 30
            xQueueReceive(vidQueue, &bmp, portMAX_DELAY);

            // watch2: updated to use tft espi

            drawBmpLine(x, y, DEFAULT_WIDTH, DEFAULT_HEIGHT, (const uint8_t **)bmp->line);
        }
    }







    /*
    ** Input
    */

    static void osd_initinput()
    {
        //watch2: not using psx input
        //psxcontrollerInit();
    }

    void osd_getinput(void)
    {

        // watch2: not using psx input

    // 	const int ev[16]={
    // 			event_joypad1_select,0,0,event_joypad1_start,event_joypad1_up,event_joypad1_right,event_joypad1_down,event_joypad1_left,
    // 			0,0,0,0,event_soft_reset,event_joypad1_a,event_joypad1_b,event_hard_reset
    // 		};
    // 	static int oldb=0xffff;
    // 	int b=psxReadInput();
    // 	int chg=b^oldb;
    // 	int x;
    // 	oldb=b;
    // 	event_t evh;
    // //	printf("Input: %x\n", b);
    // 	for (x=0; x<16; x++) {
    // 		if (chg&1) {
    // 			evh=event_get(ev[x]);
    // 			if (evh) evh((b&1)?INP_STATE_BREAK:INP_STATE_MAKE);
    // 		}
    // 		chg>>=1;
    // 		b>>=1;
    // 	}
    }

    static void osd_freeinput(void)
    {
    }

    void osd_getmouse(int *x, int *y, int *button)
    {
    }


    //------------------------------------------
    // nofrendo startup
    //------------------------------------------

    int osd_init()
    {
        log_chain_logfunc(logprint);

        if (osd_init_sound())
            return -1;

        // watch2: using tft espi
        //ili9341_init();
        //ili9341_write_frame(0,0,320,240,NULL);
        
        vidQueue=xQueueCreate(1, sizeof(bitmap_t *));
        xTaskCreatePinnedToCore(&videoTask, "nesVideoTask", 2048, NULL, 5, NULL, 1);
        osd_initinput();
        return 0;
    }





    //------------------------------------------
    // nofrendo app code
    //------------------------------------------

    /* this is at the bottom, to eliminate warnings */
    void osd_shutdown()
    {
        osd_stopsound();
        osd_freeinput();
    }

}




void state_func_nes()
{
    static std::string filename = "";
    static size_t game_data_size = 0;
    static void *game_data;
    static uint8_t draw_frame = 0;
    static uint16_t *dma_buffer;
    static uint32_t dma_buffer_length = SCREEN_WIDTH;
    static bool game_loaded = false;

    static int game_x = 0; //(SCREEN_WIDTH - AGNES_SCREEN_WIDTH) / 2;
    static int game_y  = 0;
    static int game_w = 250;
    static int game_h = 240;

    uint16_t x = 0;
    uint16_t y = 0;
    uint32_t dma_buffer_pos = 0;
    clock_t frame_time_start = 0;
    clock_t frame_time_end = 0;

    if (!watch2::state_init) 
    {
        rom_path = watch2::beginFileSelect("/");
        if (filename.compare("canceled") == 0) watch2::switchState(2);
        else
        {
            Serial.println("1");
            game_data_size = 0;
            //game_data = read_nes_rom(filename.c_str(), &game_data_size);

            Serial.println("2");

            printf("NoFrendo start!\n");
            nofrendo_main(0, NULL);
            printf("NoFrendo died? WtF?\n");

            Serial.println("3");

            dma_buffer = (uint16_t*) heap_caps_malloc(dma_buffer_length * sizeof(uint16_t), MALLOC_CAP_DMA);
            if (!dma_buffer) Serial.println("\0[31mfailed to allocate memory for dma buffer\0[0m");

            Serial.println("4");
        }
    }

    if (game_loaded)
    {
        while(true)
        {
            //Serial.println("a");
            //wait until DMA has finished
            //watch2::oled.dmaWait();

            //Serial.println("b");
            //frame_time_start = clock();
            // get frame
            //frame_time_end = clock();
            //Serial.printf("frame time: %d\n", frame_time_end - frame_time_start);
            //Serial.println("c");

            // if (draw_frame == 0)
            // {
                
                //Serial.println("d");
                
                //frame_time_start = clock();

                // watch2::oled.startWrite();
                // watch2::oled.setSwapBytes(true);
                // watch2::oled.setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

                //Serial.println("d2");

                //for (uint16_t p = 0; p < (game_w * game_h); p++)
                //{
                    //Serial.println("A");

                    //x = p % game_h;
                    //y = p / game_w;
                    
                    //Serial.println("B");

                    // uint8_t color_ix = agnes->ppu.screen_buffer[p];
                    // agnes_color_t c = g_colors[color_ix & 0x3f];

                    //Serial.println("C");

                    // save pixels to buffer
                    //uint16_t screen_x = game_x + x;
                    //if (screen_x >= 0 && screen_x < SCREEN_WIDTH - 1) 
                    //{
                        //uint16_t index = ((y * SCREEN_WIDTH) + screen_x);
                        //printf("%d is the index we're writing the colour to\n", index);
                        //dma_buffer[screen_x] = watch2::oled.color565(c.r, c.g, c.b);
                    //}

                    //Serial.println("D");

                    // draw pixels to screen
                    //watch2::oled.drawPixel(game_x + x, game_y + y, watch2::oled.color565(c.r, c.g, c.b));
                    //if (screen_x == dma_buffer_length - 1) watch2::oled.pushPixelsDMA(dma_buffer, dma_buffer_length - 1);

                    //Serial.println("E");
                    
                //}

                //Serial.println("e");
                
                //wait until DMA has finished
                //watch2::oled.dmaWait();
                //watch2::oled.pushPixelsDMA(dma_buffer_top, dma_buffer_length - 1);
                //watch2::oled.pushPixelsDMA(dma_buffer_bottom, dma_buffer_length - 1);

                //Serial.println("f");

                // watch2::oled.endWrite();
                // watch2::oled.setSwapBytes(false);
                //draw_frame += 1;

                //frame_time_end = clock();

                //Serial.printf("draw time: %d\n", frame_time_end - frame_time_start);

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
        game_loaded = false;
        //free(dma_buffer);
        watch2::switchState(2);
    }


}