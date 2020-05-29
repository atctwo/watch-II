# watch2 Arduino libraries

watch 2 depends on lots of different code libraries.  Most of them are Arduino libraries that have to be installed either using the built in library manager, or manually to one of the locations that Arduino searches for libraries.  While most libraries will work fine without any modification, some libraries have to be tweaked a bit for watch 2 to be able to work with them.  This page lists each of the libraries that watch2 depends on, and what modifications have to be made.

Most of the required libraries are `#include`d in `watch2/src/watch2.h`, although libraries that are only required by one app will probably only be `#include`d in the code for that app.

## Libraries

### built in to Arduino / the esp32 core
You won't have to manually install these, because they are included with the ESP32 Arduino core.
- SPI library (`SPI.h`)
- ESP32 FS library (`FS.h`)
- WiFi library (`WiFi.h`)
- Wifi secure client library (`WiFiClientSecure.h`)
- HTTP client library (`HTTPClient.h`)
- ESP32 Preferences library (`Preferences.h`)
- [cJSON](https://github.com/DaveGamble/cJSON) (not Arduino or ESP32 specific, but bundled with the ESP32 core)

### external libraries
You will have to install these libraries yourself.
- [SdFat](https://github.com/greiman/SdFat) by greiman
- [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) by Bodmer
- [JC_Button](https://github.com/JChristensen/JC_Button) by JChristensen
- [Time](https://github.com/PaulStoffregen/Time) by Paul Stoffregen
- [TimeAlarms](https://github.com/PaulStoffregen/TimeAlarms) by Paul Stoffregen
- a fork of [IRremote](https://github.com/ExploreEmbedded/Arduino-IRremote) that adds ESP32 send support that was never merged back into the original IRremote
- [ESP32 BLE Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard) by T-vK

### included with the repository
You won't have to manually install these libraries, because they are included with the repository.

The library csscolorparser included in the source directory isn't an Arduino library, so was included with the source.  It's GitHub repository can be found at https://github.com/mapbox/css-color-parser-cpp.

The Wikipedia app uses Google's [gimbo-parser](https://github.com/google/gumbo-parser) to parse HTML.

The system uses two of nothings' stb libraries (https://github.com/nothings/stb), specifically stb_image.h and stb_image_resize.h.  stb_image was modified slightly to shut the compiler up.  The modified version is included with the source.

## Modifications

These are the modifications you need to make to some of the libraries to get watch2 to work with them.

### hardware dependant

The system will still compile if you don't perform these modifications, but you will need to modify them to match with the exact hardware that you are using.  The modifications listed below are specific to the watch 2 hardware that the system is being developed for, so if you are trying to get watch 2 running on a different board, you should see the documentation for the config files, and configure them to work with the hardware you are using.

The system uses a fork of IRRemote that adds ESP32 support (https://github.com/ExploreEmbedded/Arduino-IRremote).  In the file boarddefs.h, the value of TIMER_PWM_PIN (line 565) was changed from 5 to 12, and the value of TIMER_CHANNEL (line 563) was changed from 1 to 2.

The TFT screen is controlled using Bodmer's wonderful TFT_eSPI library (https://github.com/Bodmer/TFT_eSPI).  This library is configured using a User setup header file in the library's source directory.  The file is cloned in the extras folder of this repo (it's called "Setup_watch2.h").  Make sure you only have one setup file included.

The SD card is handled by a library called SdFat (https://github.com/greiman/SdFat).  Because the SD card is attatched to the same SPI bus as the screen, and this bus isn't the default SPI bus, the config file for SdFat needs to be modified to allow custom SPI Class objects to be used by SdFat.  In the SdFat libary, open the file called `<SdFat library>/src/SdFatConfig.h`, and go to line 80.  Change `#define USE_STANDARD_SPI_LIBRARY 0` to `#define USE_STANDARD_SPI_LIBRARY 2`.

### esp32 core modifications

The ESP32 Arduino core was modified to enable C++17 support.  The modification concerns the file called `platform.txt` that is included with the ESP32 core.  Every Arduino core has a file with the same name that tells Arduino how to build sketches (among other things).  The file is usually located with the rest of the files that make up an Arduino core.  Arduino cores are usually located in a folder called "Arduino15".  On Windows, this can be found at `C:\Users\<username>\AppData\Local\Arduino15\`, and on Linux, it can be found at `~/.arduino15/`.  Go to `arduino15/packages/esp32/hardware/esp32/<version number>/`, and you will find `platform.txt`.  In this file, modify the section of code that starts `compiler.cpp.flags=` (in my install, it is on line 31).  Modify the flag `-std=gnu++11` to say `-std=gnu++17`, and save the file.

The core was also modified to double the stack size of the loopTask.  This is the task that the ESP32 core sets up to run whatever is placed in the `loop()` function.  To make this modification, start in the ESP32 core directory (the folder where `platform.txt` lives).  Navigate to `cores/esp32/`, and find a file called `main.cpp`.  Open it, and look for a call to the function called `xTaskCreateUniversal`.  This modification concerns the third parameter, which sets the stack size of the main loop task.  By default it is set to 8192 bytes, but you can change it to anything else.  I changed it to 16384, which is twice the size of the original stack size.

Some apps rely on HTTPS support, which is implemented using ARM's mbedTLS library.  This is built into the ESP32 core, but the watch system should be built using a recompiled version of the library that lets mbedTLS allocate memory from external PSRAM.  It was built using [esp32-arduino-lib-builder](https://github.com/espressif/esp32-arduino-lib-builder), and is included with this repo as `extras/libmbedtls.a`.
