/**
 * @file watch2_images.cpp
 * @author atctwo
 * @brief functions for drawing images.
 * @version 0.1
 * @date 2020-11-21
 * standard drawing functions can be found in watch2_drawing.cpp.  for some reason, icon stuff can also be found there...
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz)           heap_caps_malloc(sz, MALLOC_CAP_SPIRAM)
#define STBI_REALLOC(p,newsz)     heap_caps_realloc(p,newsz, MALLOC_CAP_SPIRAM)
#define STBI_FREE(p)              free(p)
#include "libraries/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_MALLOC(sz, c)        ((void)(c), heap_caps_malloc(sz, MALLOC_CAP_SPIRAM))
#define STBIR_FREE(p, c)           ((void)(c), free(p))
#include "libraries/stb/stb_image_resize.h"

namespace watch2 {

    uint16_t *dma_buffer;

    // These read 16- and 32-bit types from the SD card file.
    // BMP data is stored little-endian, Arduino is little-endian too.
    // May need to reverse subscript order if porting elsewhere.
    uint16_t read16(fs::File &f) {
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
    }

    uint32_t read32(fs::File &f) {
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
    }

    //functions for stb_image
    int img_read(void *user,  char *data, int size)
    {
        fs::File *f = static_cast<fs::File*>(user);
        int bytes_read = f->readBytes(data, size);
        return bytes_read;
    }

    void img_skip(void *user, int n)
    {
        fs::File *f = static_cast<fs::File*>(user);
        f->seek(n);
    }

    int img_eof(void *user)
    {
        fs::File *f = static_cast<fs::File*>(user);
        uint32_t help = f->available();
        if (help == 0) return 1;
        return 0;
    }

    int img_read_spiffs(void *user,  char *data, int size)
    {
        fs::File *f = static_cast<fs::File*>(user);
        int bytes_read = f->readBytes(data, size);
        return bytes_read;
    }

    void img_skip_spiffs(void *user, int n)
    {
        fs::File *f = static_cast<fs::File*>(user);
        f->seek(n);
    }

    int img_eof_spiffs(void *user)
    {
        fs::File *f = static_cast<fs::File*>(user);
        uint32_t help = f->available();
        if (help == 0) return 1;
        return 0;
    }

    // Bodmers BMP image rendering function
    // stolen from https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_SPIFFS_BMP/BMP_functions.ino
    void drawBmp(const char *filename, int16_t x, int16_t y) {

    if ((x >= oled.width()) || (y >= oled.height())) return;

    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS)
    {
        ESP_LOGW(WATCH2_TAG, "[drawBmp] File not found");
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t  r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42)
    {
        read32(bmpFS);
        read32(bmpFS);
        seekOffset = read32(bmpFS);
        read32(bmpFS);
        w = read32(bmpFS);
        h = read32(bmpFS);

        //   planes                  bits per pixel           compression
        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
        {
        y += h - 1;

        bool oldSwapBytes = oled.getSwapBytes();
        oled.setSwapBytes(true);
        bmpFS.seek(seekOffset);

        uint16_t padding = (4 - ((w * 3) & 3)) & 3;
        uint8_t lineBuffer[w * 3 + padding];

        for (row = 0; row < h; row++) {
            
            bmpFS.read(lineBuffer, sizeof(lineBuffer));
            uint8_t*  bptr = lineBuffer;
            uint16_t* tptr = (uint16_t*)lineBuffer;
            // Convert 24 to 16 bit colours
            for (uint16_t col = 0; col < w; col++)
            {
            b = *bptr++;
            g = *bptr++;
            r = *bptr++;
            *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
            }

            // Push the pixel row to screen, pushImage will crop the line if needed
            // y is decremented as the BMP image is drawn bottom up
            oled.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
        }
        oled.setSwapBytes(oldSwapBytes);
        ESP_LOGD(WATCH2_TAG, "Loaded in "); Serial.print(millis() - startTime);
        ESP_LOGD(WATCH2_TAG, " ms");
        }
        else ESP_LOGW(WATCH2_TAG, "BMP format not recognized.");
    }
    bmpFS.close();
    }

    void loadBmp(const char *filename) {

        fs::File bmpFS;

        // Open requested file on SD card
        bmpFS = SPIFFS.open(filename, "r");

        // get icon name from filename
        std::string icon_name = filename;       // copy filename
        icon_name.erase(icon_name.begin());     // remove '/' at start of name
        size_t pos = icon_name.rfind(".");      // find where the extension starts
        icon_name.erase(pos, icon_name.npos);   // remove extension
        ESP_LOGD(WATCH2_TAG, "[loadBmp] using icon name %s", icon_name.c_str());

        if (!bmpFS)
        {
            ESP_LOGW(WATCH2_TAG, "[loadBmp] File not found");
            return;
        }

        uint32_t seekOffset;
        uint16_t w, h, row, col;
        uint8_t  r, g, b;

        uint32_t startTime = millis();

        if (read16(bmpFS) == 0x4D42)
        {
            read32(bmpFS);
            read32(bmpFS);
            seekOffset = read32(bmpFS);
            read32(bmpFS);
            w = read32(bmpFS);
            h = read32(bmpFS);
            uint16_t y = h;

            ESP_LOGD(WATCH2_TAG, "w: %d, h: %d", w, h);

            //   planes                  bits per pixel           compression
            if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
            {
                bmpFS.seek(seekOffset);
                uint16_t padding = (4 - ((w * 3) & 3)) & 3;
                uint8_t *lineBuffer = (uint8_t*) malloc( (w * 3 + padding) * sizeof(uint8_t));
                uint16_t *imageBuffer = (uint16_t*) malloc(w * h * sizeof(uint16_t));

                for (row = 0; row < h; row++) 
                {
                    bmpFS.read(lineBuffer, sizeof(lineBuffer));
                    uint8_t*  bptr = lineBuffer;
                    uint16_t* tptr = (uint16_t*)lineBuffer;
                    // Convert 24 to 16 bit colours
                    for (uint16_t col = 0; col < w; col++)
                    {
                        b = *bptr++;
                        g = *bptr++;
                        r = *bptr++;
                        *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                    }

                    // Push the pixel row to screen, pushImage will crop the line if needed
                    // y is decremented as the BMP image is drawn bottom up
                    ESP_LOGD(WATCH2_TAG, "%d ", row);
                    memcpy(imageBuffer + (row * w), lineBuffer,  w/2);
                }

                //registerIcon(icon_name, imageBuffer); 
                ESP_LOGD(WATCH2_TAG, "[loadBmp] Loaded in "); Serial.print(millis() - startTime);
                ESP_LOGD(WATCH2_TAG, " ms");
            }
            else ESP_LOGW(WATCH2_TAG, "[loadBmp] BMP format not recognized.");
        }
        bmpFS.close();
    }


    imageData getImageData(const char *filename)
    {

        // this method currently uses stb_image for everything

        stbi_io_callbacks callbacks = {
            img_read,
            img_skip,
            img_eof
        };

        // open file
        fs::File f = SD.open(filename);
        ESP_LOGD(WATCH2_TAG, "%s", f.name());

        // read image
        int img_w, img_h, img_n, x, y;
        unsigned char *data = stbi_load_from_callbacks(&callbacks, &f, &img_w, &img_h, &img_n, 3);

        // set up response struct
        const char *error;
        if (data == NULL) error = stbi_failure_reason();
        else error = NULL;
        imageData response = {
            data,
            img_w,
            img_h,
            error
        };

        // return data
        f.close();
        return response;
    }

    imageData getImageDataSPIFFS(const char *filename)
    {

        // this method currently uses stb_image for everything

        stbi_io_callbacks callbacks = {
            img_read_spiffs,
            img_skip_spiffs,
            img_eof_spiffs
        };

        // open file
        fs::File f = SPIFFS.open(filename);
        // ESP_LOGD(WATCH2_TAG, "%*", f.name());

        // read image
        int img_w, img_h, img_n, x, y;
        unsigned char *data = stbi_load_from_callbacks(&callbacks, &f, &img_w, &img_h, &img_n, 3);

        // set up response struct
        const char *error;
        if (data == NULL) error = stbi_failure_reason();
        else error = NULL;
        imageData response = {
            data,
            img_w,
            img_h,
            error
        };

        // return data
        f.close();
        return response;
    }

    imageData getImageDataMemory(const unsigned char *buffer, int len)
    {

        // this method currently uses stb_image for everything

        // read image
        int img_w, img_h, img_n, x, y;
        unsigned char *data = stbi_load_from_memory(buffer, len, &img_w, &img_h, &img_n, 3);
        Serial.println("loaded image!");

        // set up response struct
        const char *error;
        if (data == NULL) error = stbi_failure_reason();
        else error = NULL;
        imageData response = {
            data,
            img_w,
            img_h,
            error
        };

        // return data
        return response;
    }

    void freeImageData(unsigned char *data)
    {
        if (data) stbi_image_free(data);
    }

    void freeImageData(imageData data)
    {
        if (data.data) stbi_image_free(data.data);
    }

    const char* drawImage(imageData data, int16_t img_x, int16_t img_y, float scaling, int array_offset, TFT_eSPI &tft, bool use_dma)
    {
        // numbers
        unsigned long pixels = data.width * data.height * 3;//sizeof(data) / sizeof(unsigned char);
        int x = img_x, y = img_y;

        if (data.data == NULL)
        {
            return stbi_failure_reason();
        }
        else
        {
            // scale image
            unsigned char *actual_data;
            uint16_t img_width = 0, img_height = 0;

            if (scaling != 1)
            {
                ESP_LOGD(WATCH2_TAG, "[drawImage] scaling image, f=%d", scaling);
                img_width = data.width/scaling;
                img_height = data.height/scaling;
                stbir_resize_uint8(
                    data.data, data.width, data.height, 0,
                    actual_data, img_width, img_height, 0, 3
                );
            }
            else
            {
                img_width = data.width;
                img_height = data.height;
                actual_data = data.data;
            }

            // allocate memory for dma buffer
            dma_buffer = (uint16_t*) heap_caps_malloc(img_width * sizeof(uint16_t), use_dma ? MALLOC_CAP_DMA : MALLOC_CAP_SPIRAM);
            if (!dma_buffer) ESP_LOGW(WATCH2_TAG, "[drawImage] failed to allocate memory for dma buffer");
            else
            {

                ESP_LOGD(WATCH2_TAG, "about to draw image; x=%d, y=%d, original w=%d, h=%d, scaled w=%d, h=%d, scale factor = %f, sizeof original = %d, sizeof scaled = %d", 
                    img_x, img_y, data.width, data.height, img_width, img_height, scaling, sizeof(data.data), sizeof(actual_data)
                );

                // set up tft
                tft.startWrite();
                tft.setSwapBytes(true);
                tft.setAddrWindow(img_x, img_y, img_width - 1, img_height - 1);

                // draw pixels
                for (uint16_t y = 0; y < img_height; y+=1)
                {
                    //Serial.printf("\tline %d: ", y);
                    for (uint16_t x = 0; x < img_width; x+=1)
                    {
                        //Serial.printf("%d ", x);
                        uint32_t pixel = (( x + (img_width * y) ) * 3) + array_offset;
                        //printf("%x %x %x\n", actual_data[pixel], actual_data[pixel+1], actual_data[pixel+2]);
                        //tft.drawPixel(img_x + x, img_y + y, tft.color565(actual_data[pixel], actual_data[pixel+1], actual_data[pixel+2]));
                        dma_buffer[x] = tft.color565(actual_data[pixel], actual_data[pixel+1], actual_data[pixel+2]);
                    }
                    watch2::oled.pushPixels(dma_buffer, img_width - 1);
                    //Serial.println("");
                }

                // also set up tft
                tft.endWrite();
                tft.setSwapBytes(false);

                free(dma_buffer);

                // if (actual_data) 
                // {
                //     ESP_LOGD(WATCH2_TAG, "[drawImage] freeing scaled image data");
                //     stbi_image_free(actual_data);
                // }

            }

            return NULL;

        }
    }

}