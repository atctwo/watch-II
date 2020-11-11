#include "../watch2.h"
#include <FS.h>
#include <SD.h>

#include "driver/i2s.h"
#include <Audio.h>

void state_func_music_player()
{
    static std::string filename;
    static TaskHandle_t audio_task_handle;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();
        if (filename != "canceled")
        {
            char sfn[25];

            watch2::oled.setCursor(2, 42);
            watch2::oled.print(String(filename.c_str()));
            FatFile f = watch2::sdcard.open(filename.c_str());
            f.getSFN(sfn);
            std::string music_path = "/MUSIC/";
            music_path += (const char*)sfn;
            f.close();
            watch2::drawTopThing();

            SD.begin(sdcs, *watch2::vspi, 4000000U);
            vTaskDelay(20); // wait for tft draw to finish
            
            watch2::play_music(audio_task_handle, music_path.c_str());
        }
    }

    watch2::drawTopThing();
    //if (watch2::audio.isRunning()) watch2::audio.loop();

    if (dpad_left_active()) 
    {
        watch2::audio.stopSong();
        watch2::switchState(watch2::state);
    }
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}