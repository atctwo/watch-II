
#define SPI_SPEED SD_SCK_MHZ(4)
#include "watch2.h"
#include "icons/app_icons.cpp"
#include "icons/small_icons.cpp"

////////////////////////////////////////
// setup function
////////////////////////////////////////

void setup() {

    //set cpu Frequency
    //setCpuFrequencyMhz(80);

    // configure gpio data direction registers
    pinMode(12, OUTPUT);
    pinMode(cs, OUTPUT);
    pinMode(sdcs, OUTPUT);
    pinMode(tftbl, OUTPUT);
    pinMode(BATTERY_DIVIDER_PIN, INPUT);
    pinMode(IR_PIN, OUTPUT);

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);

    //begin serial
    Serial.begin(115200);
    Serial.print("\n\n");

    //set up spiffs
    Serial.print("setting up spiffs: ");
    if (!SPIFFS.begin())
    {
        Serial.println("spiffs init failed");
        watch2::spiffs_state = -1;
    }
    else
    {
        Serial.println("initalised spiffs successfully");
        watch2::spiffs_state = 1;
    }

    //set up oled
    Serial.print("setting up display: ");
    digitalWrite(cs, LOW);
    watch2::oled.begin();
    watch2::oled.fillScreen(0);
    watch2::setFont(MAIN_FONT);
    Serial.println("done");

    //set up SD card
    Serial.print("setting up sd card: ");
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);
    
    watch2::initSD();

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);
    
    //set up preferences
    Serial.print("setting up preferences: ");
    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
    watch2::timeout = watch2::preferences.getBool("timeout", false);
    watch2::short_timeout = watch2::preferences.getInt("short_timeout", 5000);
    watch2::long_timeout = watch2::preferences.getInt("long_timeout", 30000);
    watch2::themecolour = watch2::preferences.getInt("themecolour", BLUE);
    watch2::trans_mode = watch2::preferences.getUInt("trans_mode", 0);
    watch2::animate_watch_face = watch2::preferences.getBool("animate_time", true);
    watch2::screen_brightness = watch2::preferences.getUInt("brightness", 2^tftbl_resolution);
    watch2::timezone = watch2::preferences.getUInt("timezone", 0);
    watch2::ntp_wakeup_connect = watch2::preferences.getBool("ntp_wakeup", true);
    watch2::ntp_boot_connect = watch2::preferences.getBool("ntp_boot", true);
    watch2::wifi_wakeup_reconnect = watch2::preferences.getBool("wifi_wakeup", true);
    watch2::wifi_boot_reconnect = watch2::preferences.getBool("wifi_boot", true);
    watch2::wifi_enabled = watch2::preferences.getBool("wifi_en", false);
    watch2::preferences.end();
    Serial.print("done");

    //set up buttons
    Serial.print("setting up buttons: ");
    watch2::btn_dpad_up.begin();
    watch2::btn_dpad_down.begin();
    watch2::btn_dpad_left.begin();
    watch2::btn_dpad_right.begin();
    watch2::btn_dpad_enter.begin();
    Serial.println("done");

    //set up torch
    Serial.print("setting up torch: ");
    ledcAttachPin(TORCH_PIN, TORCH_PWM_CHANNEL);
    ledcSetup(TORCH_PWM_CHANNEL, 4000, 8);
    ledcWrite(TORCH_PWM_CHANNEL, 0);
    Serial.println("done");

    //set up tft backlight
    Serial.print("setting up backlight: ");
    ledcAttachPin(tftbl, TFTBL_PWM_CHANNEL);
    ledcSetup(TFTBL_PWM_CHANNEL, 4000, tftbl_resolution);
    ledcWrite(TFTBL_PWM_CHANNEL, 2^tftbl_resolution);
    Serial.println("done");

    //set up top thing
    Serial.print("setting up top thing: ");
    Serial.println("a");
    watch2::top_thing.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight() + 2);
    Serial.println("b");
    watch2::setFont(MAIN_FONT, watch2::top_thing);
    Serial.println("done");

    //set up framebuffer
    //watch2::framebuffer.createSprite(100, 100);
    //watch2::setFont(MAIN_FONT, watch2::framebuffer);

    // set up wifi
    if (watch2::wifi_enabled)
    {
        watch2::enable_wifi(false);
        if (watch2::wifi_boot_reconnect) 
        {
            watch2::enable_wifi();
        }
    }

    //set up time
    Serial.print("setting up time: ");
    if (watch2::ntp_boot_connect) watch2::ntp_boot_connected = false;
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (watch2::boot_count == 0)
    {
        setTime(23, 58, 00, 28, 6, 2019);
        WiFi.mode(WIFI_OFF);
        btStop();

    }
    Serial.println("done");

    // set up icons
    Serial.print("setting up icons: ");
    registerAppIcons();
    registerSmallIcons();
    Serial.println("done");

    // set up state menu
    Serial.print("setting up state menu: ");
    if (watch2::boot_count == 0)
    {
        //set selected menu icon to first non-hidden state
        watch2::selected_menu_icon = 0;
        while(1)
        {
            if (watch2::states[watch2::selected_menu_icon].hidden)
            {
                //check next state
                watch2::selected_menu_icon++;
            }
            else break;
        }
    }
    //else selected_menu_icon = states.find(selected_state);
    Serial.println("done");

    //finish up
    watch2::boot_count++;
    Serial.println("setup finished!");

}

////////////////////////////////////////
// loop function
////////////////////////////////////////

void loop() {

    watch2::startLoop();

    //---------------------------------------
    // run current state
    //---------------------------------------

    if (watch2::timer_trigger_status == 0 && watch2::alarm_trigger_status == 0)
    {
        if ( watch2::state >= watch2::states.size() )
        {
            //dim screen
            uint8_t contrast = 0x0F;
            //watch2::oled.sendCommand(0xC7, &contrast, 1);

            digitalWrite(12, HIGH);

            watch2::oled.setCursor(5, 10);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("no state has been\nloaded");
            //watch2::oled.drawRGBBitmap(0, 29, coolcrab, 128, 55);
            if (dpad_any_active()) watch2::switchState(0);
        }
        else watch2::states[watch2::state].stateFunc();

    }

    //---------------------------------------
    // handle timers
    //---------------------------------------

    else if (watch2::timer_trigger_status != 0)
    {
        if (watch2::timer_trigger_status == 1)
        {
            //dim screen
            watch2::dimScreen(0, 10);
            watch2::oled.fillScreen(BLACK);

            //get time to display
            time_t duration = watch2::timers[watch2::timer_trigger_id].initial_duration;
            time_t time_left_hrs = floor(duration / 3600);
            time_t time_left_min = floor(duration % 3600 / 60);
            time_t time_left_sec = floor(duration % 3600 % 60);

            //draw message
            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.setCursor(2, watch2::top_thing_height);
            watch2::oled.println("Time's up");

            watch2::setFont(REALLY_BIG_FONT);
            watch2::oled.setTextColor(watch2::themecolour, BLACK);
            watch2::oled.printf("%02d:%02d:%02d\n", time_left_hrs, time_left_min, time_left_sec);
            //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);

            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextColor(WHITE, BLACK);
            watch2::oled.print("have elapsed.  press any\nkey to continue");

            //brighten screen
            watch2::dimScreen(1, 10);

            //set timer trigger status
            watch2::timer_trigger_status = 2;
        }

        watch2::drawTopThing();
        if (dpad_any_active())
        {
            watch2::timer_trigger_status = 0;
            watch2::switchState(watch2::state);
        }
    }

    //---------------------------------------
    // handle alarms
    //---------------------------------------

    else if (watch2::alarm_trigger_status != 0)
    {

        static bool selected_alarm_action = 0;
        static bool last_time = 0;
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static uint16_t button_y;
        static uint16_t button_w = 60;
        static uint16_t button_h = 60;
        static uint16_t button_r = 7;
        static TFT_eSprite time_sprite = TFT_eSprite(&watch2::oled);
        //static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH, 20);
        //0 - dismiss
        //anything else - sleep

        if (watch2::alarm_trigger_status == 1)
        {
            //dim screen
            watch2::dimScreen(0, 10);
            watch2::oled.fillScreen(BLACK);

            //brighten screen
            watch2::dimScreen(1, 10);

            //set timer trigger status
            watch2::alarm_trigger_status = 2;

            //calculate button y pos
            watch2::setFont(REALLY_BIG_FONT);
            button_y = watch2::top_thing_height + watch2::oled.fontHeight() + (watch2::oled.fontHeight() / 4);

            //set up time buffer
            watch2::setFont(REALLY_BIG_FONT, time_sprite);
            time_sprite.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight());
            time_sprite.fillScreen(BLACK);
        }

        if (dpad_left_active() || dpad_right_active())
        {
            if (selected_alarm_action == 0) selected_alarm_action = 1;
            else if (selected_alarm_action == 1) selected_alarm_action = 0;
        }

        if (now() != last_time)
        {
            watch2::setFont(REALLY_BIG_FONT, time_sprite);
            time_sprite.fillRect(0, 0, SCREEN_WIDTH, time_sprite.fontHeight(), BLACK);
            time_sprite.setTextColor(watch2::themecolour, BLACK);
            time_sprite.setCursor(2, 0);
            time_sprite.printf("%02d:%02d:%02d", hour(), minute(), second());
            time_sprite.pushSprite(0, watch2::top_thing_height);
            last_time = now();
        }

        //draw buttons
        if (dpad_any_active() || watch2::alarm_trigger_status == 2)
        {
            uint16_t split_width = SCREEN_WIDTH / 4;

            //draw dismiss button
            watch2::oled.pushImage(split_width - (button_w / 2), button_y, button_w, button_h, watch2::icons["dismiss"].data());
            watch2::oled.drawRoundRect(split_width - (button_w / 2), button_y, button_w, button_h, button_r, (!selected_alarm_action) ? watch2::themecolour : BLACK);

            //draw snooze button
            watch2::oled.pushImage((split_width * 3) - (button_w / 2), button_y, button_w, button_h, watch2::icons["bed"].data());
            watch2::oled.drawRoundRect((split_width * 3) - (button_w / 2), button_y, button_w, button_h, button_r, (selected_alarm_action) ? watch2::themecolour : BLACK);

            //draw button text
            watch2::oled.fillRect(0, button_y + button_h + (button_h / 4), SCREEN_WIDTH, watch2::oled.fontHeight(), BLACK);
            watch2::setFont(MAIN_FONT);
            watch2::oled.setTextDatum(TC_DATUM);
            String button = (selected_alarm_action) ? "Snooze" : "Dismiss";
            watch2::oled.drawString(button, SCREEN_WIDTH / 2, button_y + button_h + (button_h / 4));
            watch2::oled.setTextDatum(TL_DATUM);

            watch2::alarm_trigger_status = 3;  //hack hack hack hack
        }

        watch2::drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_alarm_action == 1)  //snooze alarm
            {
                time_t time_alarm = Alarm.read(watch2::alarms[watch2::alarm_trigger_id].alarm_id);
                time_alarm += watch2::alarm_snooze_time;
                Alarm.write(watch2::alarms[watch2::alarm_trigger_id].alarm_id, time_alarm);
            }
            else
            {
                Serial.print("i wanna be a girl and i can't type");
                Alarm.write(watch2::alarms[watch2::alarm_trigger_id].alarm_id, watch2::alarms[watch2::alarm_trigger_id].initial_time);
            }

            time_sprite.deleteSprite();
            watch2::alarm_trigger_status = 0;
            watch2::switchState(watch2::state);
        }

    }

    watch2::endLoop();
    

}


////////////////////////////////////////
// system functions
////////////////////////////////////////

extern "C"
{
    void vPortCleanUpTCB ( void *pxTCB ) {
        // place clean up code here
        // this isn't actually used.  this is just here to shut the compiler up if freertos is compiled with
        // static allocation enabled (which it isn;t by default, so if you haven't gone out of your way to
        // modify your esp32 core install, then this doesn't really do anything).
        // see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-freertos-enable-static-task-clean-up
        // for the deets.
    }
}

/*
void Adafruit_GFX::drawRainbowBitmap(int16_t x, int16_t y,
  const uint8_t bitmap[], int16_t w, int16_t h,
  uint16_t bg, int phase_difference) {

    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

	float colour_parts = 360 / w;

    startWrite();
    for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
            float R, G, B;
            if (!watch2::trans_mode)   watch2::HSVtoRGB(&R, &G, &B, fmodf(((((float) i) * colour_parts) + phase_difference), (float)360.0), 1.0, 1.0);
            else                                            watch2::getHeatMapColor(fmod(((float)i/(float)w) + (phase_difference/360.0), (float)1.0), &R, &G, &B);
            if(i & 7) byte <<= 1;
            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            writePixel(x+i, y, (byte & 0x80) ? watch2::oled.color565((int)(R*255.0), (int)(G*255.0), (int)(B*255.0)) : bg);
        }
    }
    endWrite();
}
*/