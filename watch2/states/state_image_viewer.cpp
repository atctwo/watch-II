#include "../src/watch2.h"

void state_func_image_viewer()
{
    if (!watch2::state_init)
    {
        Serial.println("a");
        std::string filename = watch2::beginFileSelect("/");
        Serial.println("b");
        File f = watch2::SD.open(filename.c_str());
        Serial.println("c");

        stbi_io_callbacks callbacks = {
            watch2::img_read,
            watch2::img_skip,
            watch2::img_eof
        };

        Serial.println("d");
        
        
        int w, h, n;
        //unsigned char *data = stbi_load("help.jpg", &w, &h, &n, 3);
        unsigned char *data = stbi_load_from_callbacks(&callbacks, &f, &w, &h, &n, 3);

        Serial.println("e");
        unsigned long pixels = w*h*3;//sizeof(data) / sizeof(unsigned char);


        Serial.println("f");

        if (data == NULL)
        {
            watch2::oled.setCursor(0, watch2::top_thing_height);
            watch2::oled.println("Failed to load image\n");
            watch2::oled.println(stbi_failure_reason());
        }
        else
        {
            int img_x = (SCREEN_WIDTH / 2) - (w / 2), img_y = watch2::top_thing_height;
            int x = img_x;
            int y = img_y;
            Serial.printf(" ram:[%2.0f%%]\n", ((float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize()) * 100);
            //Serial.printf("%d\n", sizeof(data));
            //Serial.println("f1");
            Serial.printf("%d\n", data[0]);
            //Serial.println("f2");
            //Serial.printf("%d\n", data[(pixels*3-1)]);
            for (int i = 0; i < pixels; i+=3)
            {
                watch2::oled.drawPixel(x, y, watch2::oled.color565(data[i], data[i+1], data[i+2]));
                x++;
                if (x >= w + img_x)
                {
                    x = img_x;
                    y++;
                }
            }

            Serial.println("g");
            
            stbi_image_free(data);

            Serial.println("h");

        }

        f.close();

        Serial.println("i");
    }

    watch2::drawTopThing();

    if (dpad_any_active())
    {
        watch2::switchState(2);
    }
}