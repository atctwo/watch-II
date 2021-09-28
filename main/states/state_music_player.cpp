#include "states.h"
#include <FS.h>
#include <SD.h>

#include "driver/i2s.h"
#include <Audio.h>

void state_func_music_player()
{
    EXT_RAM_ATTR static std::string filename;
    EXT_RAM_ATTR static unsigned long last_draw_time = 0;
    EXT_RAM_ATTR static bool cancelled = false;

    if (!watch2::state_init) 
    {
        filename = watch2::beginFileSelect(watch2::dir_name(filename));
        if (filename == "canceled") {
            watch2::switchState(-1);
            cancelled = true;
            filename = "";
        }
        else {
            cancelled = false;
            char sfn[25];

            //watch2::oled.setCursor(2, 42);
            //watch2::oled.setTextColor(WHITE, BLACK);
            ESP_LOGD(WATCH2_TAG, "[music player] file name: %s (%s)", filename.c_str(), watch2::file_name(filename.c_str()).c_str());
            //watch2::oled.println(filename.c_str());
            
            ESP_LOGD(WATCH2_TAG, "[music player] music path: %s", filename.c_str());

            SD.begin(sdcs, *watch2::vspi, 4000000U);
            vTaskDelay(20); // wait for tft draw to finish
            
            watch2::play_music(filename.c_str());
            watch2::updated_track_info = true;
        }
    }

    // if track info updates, redraw track info
    if (!watch2::state_init || watch2::updated_track_info)
    {
        Serial.println("updated track info");

        // clear screen
        watch2::oled.fillScreen(0);

        watch2::oled.setCursor(0, watch2::top_thing_height);
        watch2::oled.setTextColor(WHITE, BLACK);

        // print track title (if no title is specified in the id3 tags, use the filename)
        watch2::setFont(SLIGHTLY_BIGGER_FONT);
        watch2::oled.setTextColor(watch2::themecolour, BLACK);
        if (watch2::track_name.length() > 0) watch2::oled.println(watch2::track_name.c_str());
        else watch2::oled.println(watch2::file_name(filename.c_str()).c_str());
        watch2::setFont(MAIN_FONT);
        watch2::oled.setTextColor(WHITE, BLACK);

        // print track artist
        if (!watch2::track_artist.empty()) watch2::oled.println(watch2::track_artist.c_str());

        // print track album
        if (!watch2::track_album.empty()) watch2::oled.println(watch2::track_album.c_str());

        // reset updated track info flag
        watch2::updated_track_info = false;
    }

    // if the updated track info code above runs at all, drawing the top thing causes audio to stutter
    // i really don't know why

    //watch2::drawTopThing();
    //if (watch2::audio.isRunning()) watch2::audio.loop();

    // update progress bar
    if (!cancelled && (millis() - last_draw_time > 1000))
    {
        //ESP_LOGD(WATCH2_TAG, "update progress");
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
//     ESP_LOGD(WATCH2_TAG, "info        "); ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_id3data(const char *info){  //id3 metadata
//     ESP_LOGD(WATCH2_TAG, "id3data     ");ESP_LOGD(WATCH2_TAG, "%*", info);
//     std::string data = std::string(info);
//     if      (data.find("Title") == 0) watch2::oled.println(info);
//     else if (data.find("Artist") == 0) watch2::oled.println(info);
//     else if (data.find("Album") == 0) watch2::oled.println(info);
// }
// void audio_id3image(fs::File& f, const int s){  //id3 metadata
//     ESP_LOGD(WATCH2_TAG, "id3image     ");ESP_LOGD(WATCH2_TAG, "%*", s);
// }
// void audio_eof_mp3(const char *info){  //end of file
//     ESP_LOGD(WATCH2_TAG, "eof_mp3     ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_showstation(const char *info){
//     ESP_LOGD(WATCH2_TAG, "station     ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_showstreaminfo(const char *info){
//     ESP_LOGD(WATCH2_TAG, "streaminfo  ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_showstreamtitle(const char *info){
//     ESP_LOGD(WATCH2_TAG, "streamtitle ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_bitrate(const char *info){
//     ESP_LOGD(WATCH2_TAG, "bitrate     ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_commercial(const char *info){  //duration in sec
//     ESP_LOGD(WATCH2_TAG, "commercial  ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_icyurl(const char *info){  //homepage
//     ESP_LOGD(WATCH2_TAG, "icyurl      ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_lasthost(const char *info){  //stream URL played
//     ESP_LOGD(WATCH2_TAG, "lasthost    ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }
// void audio_eof_speech(const char *info){
//     ESP_LOGD(WATCH2_TAG, "eof_speech  ");ESP_LOGD(WATCH2_TAG, "%*", info);
// }