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

    bool play_music(const char *filename, bool repeat, fs::FS *fs)
    {
        Serial.printf("[music player] now playing %s\n", filename);
        audio_filename = filename;
        audio_repeat = repeat;
        audio_fs = fs;

        bool success = false;

        // set I2S parameters
        audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
        audio.setVolume(speaker_volume);

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
            }
            
        }
        Serial.println("[music player] audio task loop ended");
    }

}