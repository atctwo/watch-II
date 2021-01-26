#include "../watch2.h"

void state_func_recorder()
{
    if (!watch2::state_init)
    {
        watch2::oled.setCursor(0, watch2::top_thing_height);
        watch2::oled.setTextColor(WHITE, BLACK);
        watch2::oled.println("I2S Recorder Test");
    }

    char data[200];
    size_t bytes_read;
    i2s_read(I2S_NUM_0, data, 200, &bytes_read, portMAX_DELAY);

    for (int i = 0; i < bytes_read; i++)
    {
        Serial.printf("%d ", data[i]);
    }
}