#include "../watch2.h"

void state_func_image_viewer()
{
    static std::string filename;

    if (!watch2::state_init)
    {
        Serial.println("a");
        filename = watch2::beginFileSelect("/");
        if (filename == "canceled")
        {
            watch2::switchState(2);
        }
        else
        {
            watch2::imageData data = watch2::getImageData(filename.c_str());
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
                uint16_t scaling = 1;
                if (data.width >= data.height) // image is wider than it is tall
                {
                    scaling = (data.width + SCREEN_WIDTH - 1) / SCREEN_WIDTH;
                    Serial.printf("[image viewer] image is wider, scaling factor is %d\n", scaling);
                }
                else // image is taller than it is wide
                {
                    scaling = (data.height + SCREEN_HEIGHT - 1) / SCREEN_HEIGHT;
                    Serial.printf("[image viewer] image is taller, scaling factor is %d\n", scaling);
                }

                // draw image
                watch2::drawImage(data, (SCREEN_WIDTH / 2) - (data.width / 2), watch2::top_thing_height, scaling);
            }
        }
    }        

    //watch2::drawTopThing();

    if (dpad_any_active())
    {
        watch2::switchState(watch2::state);
    }
}