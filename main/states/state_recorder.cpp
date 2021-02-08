#include "states.h"
#include <tinywav.h>

void state_func_recorder()
{
    static TFT_eSprite graph = TFT_eSprite(&watch2::oled);
    static std::vector<int32_t> points;
    static uint8_t no_y_points = 240;
    static uint8_t points_pitch = SCREEN_WIDTH / no_y_points;

    static int32_t max_raw_sample = -120000000;
    static int32_t min_raw_sample = -104644608;

    static int32_t min_scaled_sample = -216629248;
    static int32_t max_scaled_sample = 277381120;

    static int32_t graph_y_scale = max_raw_sample / 100;

    static bool is_recording = false;
    static TinyWav tw;

    if (!watch2::state_init)
    {
        // set up screen
        watch2::oled.setCursor(0, watch2::top_thing_height);
        watch2::oled.setTextColor(WHITE, BLACK);
        watch2::oled.println("I2S Recorder Test");

        // set up graph
        graph.setColorDepth(1);
        graph.createSprite(SCREEN_WIDTH, 200);

        points.clear();
        for (int i = 0; i < no_y_points; i++) points.push_back(0);

        watch2::setup_audio_for_input();
    }

    // sample the i2s bus
    int32_t data;
    size_t bytes_read;
    i2s_read(I2S_NUM_0, &data, 4, &bytes_read, portMAX_DELAY); // skip every other sample
    i2s_read(I2S_NUM_0, &data, 4, &bytes_read, portMAX_DELAY);
    if (bytes_read > 0) 
    {
        // save data to array for graph
        //Serial.println(data - min_raw_sample);
        if (points.size() == no_y_points) points.erase(points.begin());
        points.push_back(data);

        // save data to file
        if (is_recording)
        {
            // normalise sample from 0 to 1
            float sample = (float)-(data - min_raw_sample) / max_scaled_sample;
            //Serial.println(sample);

            // write sample
            int bytes_written = tinywav_write_f(&tw, &sample, 1);
            //Serial.printf("bytes written: %d\n", bytes_written);
        }
    }

    // redraw the graph
    if (!is_recording)
    {
        graph.fillScreen(BLACK);
        graph.setTextColor(WHITE);
        //graph.drawString("hello", 0, 0);
        for (int i = 0; i < points.size() ; i++)
        {
            // draw data point
            graph.drawPixel(i * points_pitch, abs(points[i] / graph_y_scale), WHITE);

            // if this isn't the last point, draw a line to the next point
            if (i < points.size() - 1) graph.drawLine(
                i * points_pitch,           abs(points[i] / graph_y_scale),
                (i + 1) * points_pitch,     abs(points[i+1] / graph_y_scale),
                WHITE
            );
        }
        graph.pushSprite(0, 40);
    }

    if (dpad_enter_active())
    {
        if (is_recording)
        {
            // stop recording
            Serial.println("[recorder] stopping recording");

            tinywav_close_write(&tw);

            is_recording = false;
        }
        else
        {
            // start recording
            Serial.println("[recorder] starting recording");

            int err = tinywav_open_write(
                &tw,                    // tinywav header
                1,                      // one channel
                16000,                  // sample rate
                TW_FLOAT32,             // store samples as 32 bit float
                TW_INLINE,              // store samples as one inline buffer
                "/sd/test.wav"      // wav file path
            );

            if (err)
            {
                Serial.printf("[recorder] error starting recording: %d\n", err);
            }
            else is_recording = true;
        }
    }

    if (dpad_left_active() && !is_recording)
    {
        graph.deleteSprite();
        i2s_driver_uninstall(I2S_NUM_0);
        watch2::switchState(2);
    }
}