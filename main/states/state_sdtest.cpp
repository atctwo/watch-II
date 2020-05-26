#include "../watch2.h"

void state_func_SDtest()
{
    static std::string filename;
    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();

        watch2::oled.setCursor(2, 42);
        watch2::oled.print(String(filename.c_str()));
    }

    watch2::drawTopThing();

    if (dpad_left_active()) 
    {
        watch2::switchState(2);
    }
    
}