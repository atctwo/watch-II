
#define SPI_SPEED SD_SCK_MHZ(4)
#include "src/watch2.h"
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

    //set up spiffs
    if (!SPIFFS.begin())
    {
        Serial.println("[error] spiffs init failed");
    }

    //set up oled
    watch2::oled.begin();
    watch2::oled.fillScreen(0);
    watch2::setFont(MAIN_FONT);

    //set up SD card
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);
    
    watch2::initSD();

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);
    

    //set up preferences
    watch2::preferences.begin("watch2", false);      //open watch II preferences in RW mode
    watch2::timeout = watch2::preferences.getBool("timeout", false);
    watch2::short_timeout = watch2::preferences.getInt("short_timeout", 5000);
    watch2::long_timeout = watch2::preferences.getInt("long_timeout", 30000);
    watch2::themecolour = watch2::preferences.getInt("themecolour", BLUE);
    watch2::trans_mode = watch2::preferences.getBool("trans_mode", false);
    watch2::screen_brightness = watch2::preferences.getUInt("brightness", 2^tftbl_resolution);
    watch2::preferences.end();

    //set up buttons
    watch2::btn_dpad_up.begin();
    watch2::btn_dpad_down.begin();
    watch2::btn_dpad_left.begin();
    watch2::btn_dpad_right.begin();
    watch2::btn_dpad_enter.begin();

    //set up torch
    ledcAttachPin(TORCH_PIN, 0);
    ledcSetup(0, 4000, 8);
    ledcWrite(0, 0);

    //set up tft backlight
    ledcAttachPin(tftbl, 1);
    ledcSetup(1, 4000, tftbl_resolution);
    ledcWrite(1, 2^tftbl_resolution);

    //set up top thing
    watch2::top_thing.createSprite(SCREEN_WIDTH, watch2::oled.fontHeight() + 2);
    watch2::setFont(MAIN_FONT, watch2::top_thing);

    //set up framebuffer
    watch2::framebuffer.createSprite(100, 100);
    watch2::setFont(MAIN_FONT, watch2::framebuffer);

    //set up time
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (watch2::boot_count == 0)
    {
        setTime(23, 58, 00, 28, 6, 2019);
        WiFi.mode(WIFI_OFF);
        btStop();

    }

    //set up icons
    registerAppIcons();
    registerSmallIcons();

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

    //finish up
    watch2::boot_count++;

}

////////////////////////////////////////
// loop function
////////////////////////////////////////

void loop() {

    watch2::startLoop();

    //run current state
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
    else if (watch2::timer_trigger_status != 0) //handle timers
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
            //watch2::oled.setFreeFont(&SourceSansPro_Light8pt7b);
            watch2::oled.setCursor(0, 20);
            watch2::oled.printf("%02d hours,\n%02d minutes, and\n%02d seconds\n", time_left_hrs, time_left_min, time_left_sec);
            //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
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
    else if (watch2::alarm_trigger_status != 0) //handle alarms
    {

        static bool selected_alarm_action = 0;
        static bool last_time = 0;
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
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
        }

        if (dpad_left_active() || dpad_right_active())
        {
            if (selected_alarm_action == 0) selected_alarm_action = 1;
            else if (selected_alarm_action == 1) selected_alarm_action = 0;
        }

        if (now() != last_time)
        {
            //canvas_time->setFreeFont(&SourceSansPro_Light12pt7b);
            //canvas_time->setTextColor(WHITE);
            //canvas_time->fillScreen(BLACK);
            //canvas_time->setCursor(2, 16);
            //canvas_time->printf("%02d:%02d:%02d", hour(), minute(), second());
            //watch2::oled.drawBitmap(2, 16, canvas_time->getBuffer(), SCREEN_WIDTH, 20, watch2::themecolour, BLACK);
            last_time = now();
        }

        //draw buttons
        if (dpad_any_active() || watch2::alarm_trigger_status == 2)
        {
            //draw dismiss button
            watch2::oled.drawRoundRect(24, 42, 28, 28, 4, (!selected_alarm_action) ? watch2::themecolour : BLACK);
            //watch2::oled.drawRGBBitmap(26, 44, watch2::icons["dismiss"].data(), 24, 24);

            //draw snooze button
            watch2::oled.drawRoundRect(SCREEN_WIDTH - 26 - 26, 42, 28, 28, 4, (selected_alarm_action) ? watch2::themecolour : BLACK);
            //watch2::oled.drawRGBBitmap(SCREEN_WIDTH - 26 - 24, 44, watch2::icons["bed"].data(), 24, 24);

            //draw button text
            watch2::oled.fillRect(0, 71, SCREEN_WIDTH, SCREEN_HEIGHT - 71, BLACK);
            //watch2::oled.setFreeFont(&SourceSansPro_Regular6pt7b);
            String button = (selected_alarm_action) ? "Snooze" : "Dismiss";
            //watch2::getTextBounds(button, 24, 80, &x1, &y1, &w, &h);
            watch2::oled.setCursor(
                ( SCREEN_WIDTH / 2 ) - ( w / 2 ),
                80
            );
            watch2::oled.print(button);

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

            watch2::alarm_trigger_status = 0;
            watch2::switchState(watch2::state);
        }

    }

    watch2::endLoop();
    

}


////////////////////////////////////////
// system functions
////////////////////////////////////////


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