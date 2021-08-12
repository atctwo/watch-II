# watch2 libraries

watch 2 depends on lots of different code libraries.  Most of them are Arduino or C/C++ libraries that are included as Git Submodules, although some of them are built into the Arduino ESP32 core (which is included as an ESP-IDF component).  While most libraries will work fine without any modification, some libraries have to be tweaked a bit for watch 2 to be able to work with them.  This page lists each of the libraries that watch2 depends on, and what modifications have to be made.

Most of the required libraries are `#include`d in `main/src/watch2.h`, although libraries that are only required by one app will probably only be `#include`d in the code for that app.

To automatically install and set up each of the libraries, and the Arduino ESP32 Core, you can use the script included in the root of the repository called `install_libraries.sh`.  This will update the submodules, and copy and modify files of libraries that need to be modified.  You can manually update the repository's submodules by running `git submodule update --recursive --init` in the root of the repository.

## Libraries

### STD and STL libraries
These are included as part of the compiler, so you don't have to provide these.  They are just listed here for completion.
- `stdio.h`
- `stdint.h`
- `time.h`
- `sys/time.h`
- `sys/cdefs.h`
- `string`
- `algorithm`
- `functional`
- `stack`
- `map` and `unordered_map`


### built in to ESP-IDF and the Arduino ESP32 Core
These libraries are either part of the ESP-IDF SDK (and thus don't need manually installed), or are part of the Arduino ESP32 Core (which does need installed, but it's included as a submodule, so it will be installed when you run `git submodule update`).
- SPI library (`SPI.h`)
- I2C library (`Wire.h`)
- ESP32 FS library (`FS.h`)
- SD library (`SD.h`)
- WiFi library (`WiFi.h`)
- Wifi secure client library (`WiFiClientSecure.h`)
- HTTP client library (`HTTPClient.h`)
- ESP32 Preferences library (`Preferences.h`)
- [cJSON](https://github.com/DaveGamble/cJSON) (not Arduino or ESP32 specific, but bundled with the ESP32 core)

### external libraries
You will have to install these libraries yourself, although they are included as Git submodules to make installation really easy.
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) by Bodmer
- [JC_Button](https://github.com/JChristensen/JC_Button) by JChristensen
- [Time](https://github.com/PaulStoffregen/Time) by Paul Stoffregen
- [TimeAlarms](https://github.com/PaulStoffregen/TimeAlarms) by Paul Stoffregen
- [ESP32 BLE Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) by T-vK
- [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S) by schreibfaul1
- [Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO) by Adafruit
- [Adafruit MCP23008](https://github.com/adafruit/Adafruit-MCP23008-library) by Adafruit
- [Adafruit LC709203F](https://github.com/adafruit/Adafruit_LC709203F) by Adafruit
- [Adafruit MCP9808](https://github.com/adafruit/Adafruit_MCP9808_Library) by Adafruit
- [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor) by Adafruit
- [Adafruit SHTC3](https://github.com/adafruit/Adafruit_SHTC3) by Adafruit
- [ds1337](https://github.com/richard-clark/ds1337) by Richard Clark
- [csscolorparser](https://github.com/mapbox/css-color-parser-cpp) by Dean McNamee and Konstantin Käfer
- [gumbo-parser](https://github.com/google/gumbo-parser) by Google
- [tinyexpr](https://github.com/codeplea/tinyexpr) by codeplea
- [tinywav](https://github.com/mhroth/tinywav) by mhroth
- [libtris](https://github.com/atctwo/libtris) by atctwo
- [snake-game](https://github.com/atctwo/snake-game) by atctwo

### included with the repository
You won't have to manually install these libraries, because they are directly included with the repository.

The system uses two of nothings' stb libraries (https://github.com/nothings/stb), specifically stb_image.h and stb_image_resize.h.  stb_image was modified slightly to shut the compiler up.  The modified version is included with the source.

HTML character decoding is performed by the [`entities` library](https://stackoverflow.com/a/1082191), by cggaertner.

The NES Emulator is built around [agnes](https://github.com/kgabis/agnes).

The system uses a fork of [IRremote](https://github.com/ExploreEmbedded/Arduino-IRremote) that adds ESP32 send support that was never merged back into the original IRremote.  This fork has been modified to allow the IR receiver to be disabled.  The original IRremote has now added ESP32 send support, and IR receiver disable support, although I have yet to get ESP32 IR send working with the system.

## Modifications

These are the modifications you need to make to some of the libraries to get watch2 to work with them.  The system will still compile if you don't perform these modifications, but you will need to modify them to match with the exact hardware that you are using.  The modifications listed below are specific to the watch 2 hardware that the system is being developed for, so if you are trying to get watch 2 running on a different board, you should see the documentation for the config files, and configure them to work with the hardware you are using.

Most of these modifications are made automatically by `install_libraries.sh`

### Time

The file `Time.h` was removed to avoid conflicts with the STD library `time.h`.

### Arduino-IRremote

The system uses a fork of IRRemote that adds ESP32 support (https://github.com/ExploreEmbedded/Arduino-IRremote).  Because this fork is included directly in the repository, and not as a submodule, you don't have to make these changes yourself (but they are listed here for completion).  

In the file boarddefs.h, the value of TIMER_PWM_PIN (line 565) was changed from 5 to 4, and the value of TIMER_CHANNEL (line 563) was changed from 1 to 0.  The following method was added at line 159 of irRecv.cpp:
```c++
void IRrecv::disableIRIn() {

#ifdef ESP32
    timerEnd(timer);
    timerDetachInterrupt(timer);
#endif 

}
```
A method prototype was also added at line 180 of IRRemote.h:
```c++
171 class IRrecv
172 {
173 	public:
174 		IRrecv (int recvpin) ;
175 		IRrecv (int recvpin, int blinkpin);
176 
177 		void  blink13    (int blinkflag) ;
178 		int   decode     (decode_results *results) ;
179 		void  enableIRIn ( ) ;
180 		void  disableIRIn( ) ;
181 		bool  isIdle     ( ) ;
182 		void  resume     ( ) ;
```

### TFT_eSPI

The TFT screen is controlled using Bodmer's wonderful TFT_eSPI library (https://github.com/Bodmer/TFT_eSPI).  This library is configured using a User setup header file in the library's source directory.  The file is cloned in the extras folder of this repo (it's called "Setup_watch2.h").  To configure TFT_eSPI to use this setup file, 
1. copy `Setup_watch2.h` to `main/libraries/TFT_eSPI/User_Setups`
2. modify `main/libraries/TFT_eSPI/User_Setup_Select.h`:
    - uncomment `#include <User_Setup.h>` on line 22
    - add `#include <User_Setups/Setup_watch2.h>` on the line below
    - please make sure that only one User Setup is included in this file