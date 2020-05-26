#include "../watch2.h"

void state_func_image_viewer()
{
    if (!watch2::state_init)
    {
        Serial.println("a");
        std::string filename = watch2::beginFileSelect("/");
        watch2::imageData data = watch2::getImageData(filename.c_str());
        if (data.data == NULL)
        {
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.print("Error loading image: ");
            watch2::oled.print(data.error);
        }
        else watch2::drawImage(data, (SCREEN_WIDTH / 2) - (data.width / 2), watch2::top_thing_height);
    }        

    watch2::drawTopThing();

    if (dpad_any_active())
    {
        watch2::switchState(2);
    }
}