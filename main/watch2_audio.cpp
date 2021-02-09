/**
 * @file watch2_audio.cpp
 * @author atctwo
 * @brief functions to handle the audio subsystem
 * @version 0.1
 * @date 2020-11-21
 * 
 * @copyright Copyright (c) 2020 atctwo
 * 
 */

#include "watch2.h"

namespace watch2 {

    // audio
    uint8_t speaker_volume = 5;
    TaskHandle_t audio_task_handle;
    bool audio_repeat = false;
    std::string audio_filename = "";
    fs::FS *audio_fs = NULL;
    Audio audio;
    bool is_driver_installed = false;
    bool is_playing = false;

    void setup_audio_for_playback()
    {
        Serial.println("[audio] setting up I2S for playback");

        // set up I2S driver
        i2s_driver_uninstall(I2S_NUM_0);
        const i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX), // transmit audio
            .sample_rate = 16000,                         // 16KHz
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // could only get it to work with 32bits
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // use right channel
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
            .dma_buf_count = 8,                           // number of buffers
            .dma_buf_len = 1024,                          // 1024 samples per buffer (minimum)
            .use_apll = true,
            .tx_desc_auto_clear = true,
            .fixed_mclk = I2S_PIN_NO_CHANGE
        };
        esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        if (err) Serial.printf("[recorder] error installing i2s driver: %s (%d)", esp_err_to_name(err), err);
        is_driver_installed = (err == 0);


        // set I2S parameters
        audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_DIN);
        audio.setVolume(speaker_volume);
    }

    void setup_audio_for_input()
    {
        Serial.println("[audio] setting up I2S for input");

        // set up i2s for audio input
        i2s_driver_uninstall(I2S_NUM_0);

        const i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
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
        is_driver_installed = (err == 0);

        i2s_pin_config_t pins = {
            .bck_io_num = I2S_BCLK,
            .ws_io_num =  I2S_LRC,
            .data_out_num = I2S_DOUT,
            .data_in_num = I2S_DIN
        };
        i2s_set_pin(I2S_NUM_0, &pins);

        REG_SET_BIT(  I2S_TIMING_REG(I2S_NUM_0),BIT(9));   /*  #include "soc/i2s_reg.h"   I2S_NUM -> 0 or 1*/
        REG_SET_BIT( I2S_CONF_REG(I2S_NUM_0), I2S_RX_MSB_SHIFT);

        is_playing = true;
    }

    void uninstall_i2s_driver()
    {
        Serial.println("[audio] uninstalling i2s");
        i2s_driver_uninstall(I2S_NUM_0);
        is_driver_installed = false;
        is_playing = false;
    }

    bool play_music(const char *filename, bool repeat, fs::FS *fs)
    {
        Serial.printf("[music player] now playing %s\n", filename);
        audio_filename = filename;
        audio_repeat = repeat;
        audio_fs = fs;

        bool success = false;
        is_playing = true;

        setup_audio_for_playback();

        // if a filesystem has been passed, load the file from the filesystem
        if (fs) success = audio.connecttoFS(*fs, filename);

        // otherwise, internet
        else success = audio.connecttohost(filename);

        // create the audio task
        // int x = 10;
        // xTaskCreatePinnedToCore(audio_task, "audio", 8192, (void*)x, ESP_TASK_PRIO_MAX - 2, &task_handle, 1);

        // if repeat is enabled, set it to the success value
        // if playback failed, repeat will be set to false, so it won't repeat forever
        if (repeat) audio_repeat = success;

        return success;
    }

    void stop_music()
    {
        audio.stopSong();
        audio_filename = "";
        audio_repeat = false;
        audio_fs = NULL;
        is_playing = false;
    }

    void audio_task(void *pvParameters)
    {
        Serial.println("[music player] starting audio");
        while(true)
        {
            if (audio.isRunning()) audio.loop();
            else 
            {
                if (audio_repeat) play_music(audio_filename.c_str(), true, audio_fs);
                else vTaskDelay(500);
                if (!is_playing && is_driver_installed) uninstall_i2s_driver();
            }
            
        }
        Serial.println("[music player] audio task loop ended");
    }

}