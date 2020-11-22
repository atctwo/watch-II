#include "../watch2.h"
#include <FS.h>
#include <SD.h>

#include "driver/i2s.h"
#include <Audio.h>

void state_func_music_player()
{
    static std::string filename;
    static unsigned long last_draw_time = 0;
    static bool cancelled = false;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect();
        if (filename == "canceled") {
            watch2::switchState(2);
            cancelled = true;
        }
        else {
            cancelled = false;
            char sfn[25];

            watch2::oled.setCursor(2, 42);
            watch2::oled.setTextColor(WHITE, BLACK);
            Serial.printf("[music player] file name: %s (%s)\n", filename.c_str(), watch2::file_name(filename.c_str()).c_str());
            watch2::oled.println(filename.c_str());
            
            Serial.printf("[music player] music path: %s\n", filename.c_str());

            SD.begin(sdcs, *watch2::vspi, 4000000U);
            vTaskDelay(20); // wait for tft draw to finish
            
            watch2::play_music(filename.c_str());
        }
    }

    watch2::drawTopThing();
    //if (watch2::audio.isRunning()) watch2::audio.loop();

    // update progress bar
    if (!cancelled && (millis() - last_draw_time > 1000))
    {
        //Serial.println("update progress");
        uint32_t length         = watch2::audio.getAudioFileDuration();
        uint32_t current_time   = watch2::audio.getAudioCurrentTime();

        if (length)
        {
            uint16_t bar_width = (current_time * SCREEN_WIDTH) / length;
            uint16_t bar_height = 5;

            watch2::oled.fillRect(0, SCREEN_HEIGHT - bar_height, bar_width, bar_height, watch2::themecolour);
        }

        last_draw_time = millis();
    }

    if (dpad_enter_active())
    {
        watch2::audio.pauseResume();
    }
    if (dpad_up_active())
    {
        watch2::speaker_volume = std::min(watch2::speaker_volume + 1, 21);
        watch2::audio.setVolume(watch2::speaker_volume);
    }
    if (dpad_down_active())
    {
        watch2::speaker_volume = std::max(watch2::speaker_volume - 1, 0);
        watch2::audio.setVolume(watch2::speaker_volume);
    }
    if (dpad_left_active() || watch2::btn_dpad_enter.pressedFor(1000)) 
    {
        watch2::stop_music();
        watch2::switchState(watch2::state);
    }
}

// void audio_info(const char *info){
//     Serial.print("info        "); Serial.println(info);
// }
// void audio_id3data(const char *info){  //id3 metadata
//     Serial.print("id3data     ");Serial.println(info);
//     std::string data = std::string(info);
//     if      (data.find("Title") == 0) watch2::oled.println(info);
//     else if (data.find("Artist") == 0) watch2::oled.println(info);
//     else if (data.find("Album") == 0) watch2::oled.println(info);
// }
// void audio_id3image(fs::File& f, const int s){  //id3 metadata
//     Serial.print("id3image     ");Serial.println(s);
// }
// void audio_eof_mp3(const char *info){  //end of file
//     Serial.print("eof_mp3     ");Serial.println(info);
// }
// void audio_showstation(const char *info){
//     Serial.print("station     ");Serial.println(info);
// }
// void audio_showstreaminfo(const char *info){
//     Serial.print("streaminfo  ");Serial.println(info);
// }
// void audio_showstreamtitle(const char *info){
//     Serial.print("streamtitle ");Serial.println(info);
// }
// void audio_bitrate(const char *info){
//     Serial.print("bitrate     ");Serial.println(info);
// }
// void audio_commercial(const char *info){  //duration in sec
//     Serial.print("commercial  ");Serial.println(info);
// }
// void audio_icyurl(const char *info){  //homepage
//     Serial.print("icyurl      ");Serial.println(info);
// }
// void audio_lasthost(const char *info){  //stream URL played
//     Serial.print("lasthost    ");Serial.println(info);
// }
// void audio_eof_speech(const char *info){
//     Serial.print("eof_speech  ");Serial.println(info);
// }