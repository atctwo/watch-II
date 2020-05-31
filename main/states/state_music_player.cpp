#include "../watch2.h"
#include <FS.h>
#include <SD.h>

#include <AudioFileSourceHTTPStream.h>
#include <AudioFileSourceFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourceBuffer.h>

void audio_task(void *pvParameters);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

void state_func_music_player()
{
    static std::string filename;
    static TaskHandle_t audio_task_handle;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();
        char sfn[25];

        watch2::oled.setCursor(2, 42);
        watch2::oled.print(String(filename.c_str()));
        FatFile f = watch2::SD.open(filename.c_str());
        f.getSFN(sfn);
        std::string music_path = "/MUSIC/";
        music_path += (const char*)sfn;
        f.close();
        watch2::drawTopThing();

        SD.begin(sdcs, *watch2::vspi, 4000000U);
        vTaskDelay(20); // wait for tft draw to finish
        
        xTaskCreatePinnedToCore(audio_task, "audio", 8192, (void*)music_path.c_str(), ESP_TASK_PRIO_MAX - 2, &audio_task_handle, 1);
    }

    
    //vTaskDelay(10); // yield so that audio can update
    

    if (dpad_left_active()) 
    {
        SD.end();
        vTaskDelay(100);
        watch2::switchState(2);
    }
}

void audio_task(void *pvParameters)
{
    AudioGeneratorMP3 *mp3;
    AudioFileSourceFS *file;
    AudioFileSourceBuffer *buffer;
    AudioOutputI2S *out;
    float gain = 0.2;

    const char *music_path = (const char*)pvParameters;
    Serial.printf("[music player] Now Playing: %s\n", music_path);

    file = new AudioFileSourceFS(SD, music_path);
    //file = new AudioFileSourceHTTPStream("http://media-ice.musicradio.com:80/RadioXLondonMP3");
    buffer = new AudioFileSourceBuffer(file, 2048);
    out = new AudioOutputI2S();
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(gain);
    mp3 = new AudioGeneratorMP3();
    mp3->begin(buffer, out);

    while(1)
    {
        if (mp3->isRunning()) 
        {
            digitalWrite(cs, HIGH);
            if (!mp3->loop()) 
            {
                mp3->stop(); 
                Serial.println("mp3 stopped");
            }
        }
        if (digitalRead(dpad_left)) 
        {
            Serial.println("[music player] ending playback");
            mp3->stop();
            vTaskDelete(NULL);
        }
        if (digitalRead(dpad_up))
        {
            if (gain < 1.0) 
            {
                gain += 0.05;
                out->SetGain(gain);
                Serial.printf("gain: %f", gain);
            }
        }
        if (digitalRead(dpad_down))
        {
            if (gain > 0.0) 
            {
                gain -= 0.05;
                out->SetGain(gain);
                Serial.printf("gain: %f", gain);
            }
        }
        //vTaskDelay(1);

        // update bare minimum of system peripherals and internals

        //initial button setup
        watch2::btn_dpad_up.read();
        watch2::btn_dpad_down.read();
        watch2::btn_dpad_left.read();
        watch2::btn_dpad_right.read();
        watch2::btn_dpad_enter.read();
        watch2::btn_zero.read();

        //check timers and alarms
        Alarm.delay(0);
    }
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    fs::File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    fs::File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}