#include "../watch2.h"

void state_func_image_viewer()
{
    static std::string filename;
    static watch2::imageData data;

    if (!watch2::state_init)
    {
        Serial.println("a");
        filename = watch2::beginFileSelect(watch2::dir_name(filename).c_str());
        if (filename == "canceled")
        {
            watch2::switchState(2);
        }
        else
        {
            data = watch2::getImageData(filename.c_str());
            if (data.data == NULL)
            {
                watch2::oled.setTextColor(WHITE, BLACK);
                watch2::oled.setCursor(2, watch2::top_thing_height);
                watch2::oled.println("Error loading image: ");
                watch2::oled.print(data.error);
            }
            else 
            {
                // calculate scaling
                float scaling = 1;
                if (data.width >= data.height) // image is wider than it is tall
                {
                    if (data.width > SCREEN_WIDTH) scaling = (float)data.width / SCREEN_WIDTH;
                    else scaling = 1.0;
                    Serial.printf("[image viewer] image is wider, scaling factor is %f\n", scaling);
                }
                else // image is taller than it is wide
                {
                    if (data.height > SCREEN_HEIGHT) scaling = (float)data.height / SCREEN_HEIGHT;
                    else scaling = 1.0;
                    Serial.printf("[image viewer] image is taller, scaling factor is %f\n", scaling);
                }

                // draw image
                watch2::drawImage(data, (SCREEN_WIDTH / 2) - (data.width / 2), watch2::top_thing_height, scaling);
            }
        }
    }        

    //watch2::drawTopThing();

    if (dpad_any_active())
    {
        // free image data
        if (data.data) 
        {
            Serial.println("[image viewer] freeing original image data");
            watch2::freeImageData(data.data);
        }

        // return to file select
        watch2::switchState(watch2::state);
    }
}