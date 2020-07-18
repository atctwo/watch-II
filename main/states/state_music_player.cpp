#include "../watch2.h"
#include <FS.h>
#include <SD.h>

#include "driver/i2s.h"
#include <AudioFileSourceHTTPStream.h>
#include <AudioFileSourceFS.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioGeneratorMIDI.h>
#include <AudioGeneratorMOD.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorRTTTL.h>
#include <AudioGeneratorWAV.cpp>
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
        ESP.restart();
        // SD.end();
        // vTaskDelay(100);

        // Serial.println("[music player] uninstalling i2s driver");
        // i2s_driver_uninstall(I2S_NUM_1);
        // delay(10);

        // Serial.println("[music player] detatching vspi");
        // watch2::vspi->end();
        // delay(10);

        // Serial.println("[music player] screen reset");
        // digitalWrite(spi_rst, LOW);
        // delay(5);
        // digitalWrite(spi_rst, HIGH);
        // delay(10);

        // Serial.println("[music player] setting up display");
        // digitalWrite(cs, LOW);
        // watch2::oled.setAttribute(PSRAM_ENABLE, true);
        // watch2::oled.begin();
        // //watch2::oled.fillScreen(0);
        // watch2::setFont(MAIN_FONT);
        // delay(10);

        // watch2::switchState(2);
    }
}

void audio_task(void *pvParameters)
{
    const char *music_path = (const char*)pvParameters;
    std::string extension = watch2::file_ext(std::string(music_path));

    AudioFileSource             *file;
    AudioFileSourceBuffer       *buffer;
    AudioGenerator              *audio_generator;
    AudioOutputI2S              *out;
    float gain = 0.2;

    Serial.printf("[music player] Now Playing: %s\n", music_path);

    // set up audio file source
    file = new AudioFileSourceFS(SD, music_path);
    //file = new AudioFileSourceHTTPStream("http://media-ice.musicradio.com:80/RadioXLondonMP3");
    //file = new AudioFileSourceHTTPStream("http://bbcmedia.ic.llnwd.net/stream/bbcmedia_radio1_mf_p");
    //file = new AudioFileSourceHTTPStream("http://bbcmedia.ic.llnwd.net/stream/bbcmedia_radio2_mf_p");
    //file = new AudioFileSourceHTTPStream("http://media-ice.musicradio.com:80/RadioXLondonMP3");
    buffer = new AudioFileSourceBuffer(file, 2048);
    
    // set up audio output
    out = new AudioOutputI2S(I2S_NUM_1);
    out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    out->SetGain(gain);

    // set up audio generator
    if      (extension == "aac") audio_generator = new AudioGeneratorAAC();
    else if (extension == "fla") audio_generator = new AudioGeneratorFLAC();
    else if (extension == "mid") audio_generator = new AudioGeneratorMIDI();
    else if (extension == "rtt") audio_generator = new AudioGeneratorRTTTL();
    else if (extension == "wav") audio_generator = new AudioGeneratorWAV();
    else if (extension == "mp3") audio_generator = new AudioGeneratorMP3();
    else if (extension == "mod" || extension == "it" || extension == "xm" || extension == "s3m") audio_generator = new AudioGeneratorMOD();
    else {
        Serial.println("[music player] invalid file format");
        vTaskDelete(NULL);
    }
    audio_generator->begin(buffer, out);

    while(1)
    {
        if (audio_generator->isRunning()) 
        {
            digitalWrite(cs, HIGH);
            if (!audio_generator->loop()) 
            {
                audio_generator->stop(); 
                Serial.println("audio stopped");
            }
        }
        if (digitalRead(dpad_left)) 
        {
            Serial.println("[music player] ending playback");
            audio_generator->stop();
            vTaskDelete(NULL);
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