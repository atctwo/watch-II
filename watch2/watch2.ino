
#define SPI_SPEED SD_SCK_MHZ(4)
#include "src/watch2.h"
#include "icons/app_icons.cpp"
#include "icons/small_icons.cpp"

using namespace watch2;

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
    pinMode(sdcd, INPUT);
    pinMode(BATTERY_DIVIDER_PIN, INPUT);
    pinMode(IR_PIN, OUTPUT);

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);

    //begin serial
    Serial.begin(115200);


    //test putting sdcs High
    //format SD using SDa tool
    //flowchart - ask


    //set up oled
    oled.begin(18000000);
    oled.fillScreen(0);
    //oled.setFont(&SourceSansPro_Regular6pt7b);
    uint8_t contrast = 0x0f;
    oled.sendCommand(0xC7, &contrast, 1);

    //set up SD card
    digitalWrite(cs, HIGH);
    digitalWrite(sdcs, LOW);
    
    initSD();

    digitalWrite(cs, LOW);
    digitalWrite(sdcs, HIGH);
    

    //set up preferences
    preferences.begin("watch2", false);      //open watch II preferences in RW mode
    timeout = preferences.getBool("timeout", false);
    short_timeout = preferences.getInt("short_timeout", 5000);
    long_timeout = preferences.getInt("long_timeout", 30000);
    themecolour = preferences.getInt("themecolour", BLUE);
    trans_mode = preferences.getBool("trans_mode", false);
    screen_brightness = preferences.getUChar("brightness", 15);
    preferences.end();

    //set up buttons
    btn_dpad_up.begin();
    btn_dpad_down.begin();
    btn_dpad_left.begin();
    btn_dpad_right.begin();
    btn_dpad_enter.begin();

    //set up torch
    ledcAttachPin(TORCH_PIN, 0);
    ledcSetup(0, 4000, 8);
    ledcWrite(0, 0);

    //set up time
    timeval tv;
    gettimeofday(&tv, NULL);
    setTime(tv.tv_sec);

    if (boot_count == 0)
    {
        setTime(23, 58, 00, 28, 6, 2019);
        WiFi.mode(WIFI_OFF);
        btStop();

    }

    //set up icons
    registerAppIcons();
    registerSmallIcons();

    if (boot_count == 0)
    {
        //set selected menu icon to first non-hidden state
        selected_menu_icon = 0;
        while(1)
        {
            if (states[selected_menu_icon].hidden)
            {
                //check next state
                selected_menu_icon++;
            }
            else break;
        }
    }
    //else selected_menu_icon = states.find(selected_state);

    //finish up
    boot_count++;

}

////////////////////////////////////////
// loop function
////////////////////////////////////////

void loop() {

    startLoop();

    //run current state
    if (timer_trigger_status == 0 && alarm_trigger_status == 0)
    {
        if ( state >= states.size() )
        {
            //dim screen
            uint8_t contrast = 0x0F;
            oled.sendCommand(0xC7, &contrast, 1);

            digitalWrite(12, HIGH);

            oled.setCursor(5, 10);
            oled.setTextColor(WHITE, BLACK);
            oled.print("no state has been\nloaded");
            oled.drawRGBBitmap(0, 29, coolcrab, 128, 55);
            if (dpad_any_active()) switchState(0);
        }
        else states[state].stateFunc();

    }
    else if (timer_trigger_status != 0) //handle timers
    {
        if (timer_trigger_status == 1)
        {
            //dim screen
            dimScreen(0, 10);
            oled.fillScreen(BLACK);

            //get time to display
            time_t duration = timers[timer_trigger_id].initial_duration;
            time_t time_left_hrs = floor(duration / 3600);
            time_t time_left_min = floor(duration % 3600 / 60);
            time_t time_left_sec = floor(duration % 3600 % 60);

            //draw message
            oled.setFont(&SourceSansPro_Light8pt7b);
            oled.setCursor(0, 20);
            oled.printf("%02d hours,\n%02d minutes, and\n%02d seconds\n", time_left_hrs, time_left_min, time_left_sec);
            oled.setFont(&SourceSansPro_Regular6pt7b);
            oled.print("have elapsed.  press any\nkey to continue");

            //brighten screen
            dimScreen(1, 10);

            //set timer trigger status
            timer_trigger_status = 2;
        }

        drawTopThing();
        if (dpad_any_active())
        {
            timer_trigger_status = 0;
            switchState(state);
        }
    }
    else if (alarm_trigger_status != 0) //handle alarms
    {

        static bool selected_alarm_action = 0;
        static bool last_time = 0;
        static char text_aaaa[150];
        static int16_t x1, y1;
        static uint16_t w=0, h=0;
        static GFXcanvas1 *canvas_time = new GFXcanvas1(SCREEN_WIDTH, 20);
        //0 - dismiss
        //anything else - sleep

        if (alarm_trigger_status == 1)
        {
            //dim screen
            dimScreen(0, 10);
            oled.fillScreen(BLACK);

            //brighten screen
            dimScreen(1, 10);

            //set timer trigger status
            alarm_trigger_status = 2;
        }

        if (dpad_left_active() || dpad_right_active())
        {
            if (selected_alarm_action == 0) selected_alarm_action = 1;
            else if (selected_alarm_action == 1) selected_alarm_action = 0;
        }

        if (now() != last_time)
        {
            canvas_time->setFont(&SourceSansPro_Light12pt7b);
            //canvas_time->setTextColor(WHITE);
            canvas_time->fillScreen(BLACK);
            canvas_time->setCursor(2, 16);
            canvas_time->printf("%02d:%02d:%02d", hour(), minute(), second());
            oled.drawBitmap(2, 16, canvas_time->getBuffer(), SCREEN_WIDTH, 20, themecolour, BLACK);
            last_time = now();
        }

        //draw buttons
        if (dpad_any_active() || alarm_trigger_status == 2)
        {
            //draw dismiss button
            oled.drawRoundRect(24, 42, 28, 28, 4, (!selected_alarm_action) ? themecolour : BLACK);
            oled.drawRGBBitmap(26, 44, icons["dismiss"].data(), 24, 24);

            //draw snooze button
            oled.drawRoundRect(SCREEN_WIDTH - 26 - 26, 42, 28, 28, 4, (selected_alarm_action) ? themecolour : BLACK);
            oled.drawRGBBitmap(SCREEN_WIDTH - 26 - 24, 44, icons["bed"].data(), 24, 24);

            //draw button text
            oled.fillRect(0, 71, SCREEN_WIDTH, SCREEN_HEIGHT - 71, BLACK);
            oled.setFont(&SourceSansPro_Regular6pt7b);
            String button = (selected_alarm_action) ? "Snooze" : "Dismiss";
            oled.getTextBounds(button, 24, 80, &x1, &y1, &w, &h);
            oled.setCursor(
                ( SCREEN_WIDTH / 2 ) - ( w / 2 ),
                80
            );
            oled.print(button);

            alarm_trigger_status = 3;  //hack hack hack hack
        }

        drawTopThing();

        if (dpad_enter_active())
        {
            if (selected_alarm_action == 1)  //snooze alarm
            {
                time_t time_alarm = Alarm.read(alarms[alarm_trigger_id].alarm_id);
                time_alarm += alarm_snooze_time;
                Alarm.write(alarms[alarm_trigger_id].alarm_id, time_alarm);
            }
            else
            {
                Serial.print("i wanna be a girl and i can't type");
                Alarm.write(alarms[alarm_trigger_id].alarm_id, alarms[alarm_trigger_id].initial_time);
            }

            alarm_trigger_status = 0;
            switchState(state);
        }

    }

    
    endLoop();
    

}


////////////////////////////////////////
// system functions
////////////////////////////////////////



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
            if (!trans_mode)   HSVtoRGB(&R, &G, &B, fmodf(((((float) i) * colour_parts) + phase_difference), (float)360.0), 1.0, 1.0);
            else                                            getHeatMapColor(fmod(((float)i/(float)w) + (phase_difference/360.0), (float)1.0), &R, &G, &B);
            if(i & 7) byte <<= 1;
            else      byte   = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            writePixel(x+i, y, (byte & 0x80) ? oled.color565((int)(R*255.0), (int)(G*255.0), (int)(B*255.0)) : bg);
        }
    }
    endWrite();
}
