#include "../watch2.h"
#include <FS.h>
#include <SD.h>

#include <AudioFileSourceHTTPStream.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourceBuffer.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

void state_func_music_player()
{
    static std::string filename;
    static AudioGeneratorMP3 *mp3;
    static AudioFileSourceHTTPStream *file;
    static AudioFileSourceBuffer *buffer;
    static AudioOutputI2S *out;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();
        char sfn[25];

        watch2::oled.setCursor(2, 42);
        watch2::oled.print(String(filename.c_str()));
        FatFile f = watch2::SD.open(filename.c_str());
        // f.getSFN(sfn);
        // std::string music_path = "/MUSIC/";
        // music_path += (const char*)sfn;
        // Serial.printf("[music player] Now Playing: %s", music_path.c_str());
        // f.close();

        // SD.begin(sdcs, *watch2::vspi, 4000000U);
        
        //file = new AudioFileSourceFS(SD, music_path.c_str());
        file = new AudioFileSourceHTTPStream("http://media-ice.musicradio.com:80/RadioXLondonMP3");
        buffer = new AudioFileSourceBuffer(file, 2048);
        out = new AudioOutputI2S();
        out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
        out->SetGain(0.2);
        mp3 = new AudioGeneratorMP3();
        mp3->begin(buffer, out);
    }

    watch2::drawTopThing();

    if (mp3->isRunning()) 
    {
        if (!mp3->loop()) mp3->stop(); 
    }

    if (dpad_left_active()) 
    {
        //SD.end();
        mp3->stop();
        watch2::switchState(2);
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