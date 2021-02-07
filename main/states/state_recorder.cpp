#include "../watch2.h"

void state_func_recorder()
{
    static TFT_eSprite graph = TFT_eSprite(&watch2::oled);
    static std::vector<int32_t> points;
    static uint8_t no_y_points = 240;
    static uint8_t points_pitch = SCREEN_WIDTH / no_y_points;
    static double graph_y_scale = -107806720 / 100;

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

        // set up i2s for audio input
        i2s_driver_uninstall(I2S_NUM_0);

        const i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // Receive, not transfer
            .sample_rate = 16000,                         // 16KHz
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
            .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // use right channel
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
            .dma_buf_count = 4,                           // number of buffers
            .dma_buf_len = 8                              // 8 samples per buffer (minimum)
        };
        esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        if (err) Serial.printf("[recorder] error installing i2s driver: %s (%d)", esp_err_to_name(err), err);

        i2s_pin_config_t pins = {
            .bck_io_num = I2S_BCLK,
            .ws_io_num =  I2S_LRC,
            .data_out_num = I2S_DOUT,
            .data_in_num = I2S_DIN
        };
        i2s_set_pin(I2S_NUM_0, &pins);

        REG_SET_BIT(  I2S_TIMING_REG(I2S_NUM_0),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
        REG_SET_BIT( I2S_CONF_REG(I2S_NUM_0), I2S_RX_MSB_SHIFT);
    }

    // sample the i2s bus
    int32_t data;
    size_t bytes_read;
    i2s_read(I2S_NUM_0, &data, 4, &bytes_read, portMAX_DELAY); // skip every other sample
    i2s_read(I2S_NUM_0, &data, 4, &bytes_read, portMAX_DELAY);
    if (bytes_read > 0) 
    {
        Serial.println(data);
        if (points.size() == no_y_points) points.erase(points.begin());
        points.push_back(data);
    }

    // redraw the graph
    graph.fillScreen(BLACK);
    graph.setTextColor(WHITE);
    graph.drawString("hello", 0, 0);
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

    if (dpad_any_active())
    {
        i2s_driver_uninstall(I2S_NUM_0);
        watch2::switchState(2);
    }
}