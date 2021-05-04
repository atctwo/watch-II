# watch2 Source Code

Here is a brief description of how the code for watch2 is organised

- `main.cpp` contains the system initialisation code, main loop, and main entry point
- each of the `watch2_*.cpp` files contain the source code for handling a particular subsystem or related functions.  they share one big header file called `watch2.h`
    - `watch2_audio.cpp` contains functions for using the I2S audio subsystem
    - `watch2_battery.cpp` contains functions to query the battery fuel gauge
    - `watch2_bluetooth.cpp` contains functions to interface with the BLE subsystem
    - `watch2_dialogues.cpp` contains functions to draw a few interactive popup dialogues
    - `watch2_drawing.cpp` contains functions for drawing standard things like menus
    - `watch2_files.cpp` contains utility functions for talking to an SD card (these functions are designed to complement the functions in the Arduino SD Library)
    - `watch2_images.cpp` contains functions for decoding and drawing standard image formats (based on `stb_image.h`)
    - `watch2_system.cpp` contains some core system functions
    - `watch2_utility.cpp` contains some miscellaneous utility functions
    - `watch2_wifi.cpp` contains functions for talking to the WiFi subsystem
- the `states` directory contains source files for each of the system's states / apps
- the `libraries` folder contains most of the libraries the system depends on
- the `custom_fonts` folder contains fonts converted to header files that were used by early versions of the system
- the `icons` folder contains monochrome icons converted to byte arrays, used in some parts of the system (RGB565 icons are loaded as PNGs from SPIFFS)
- `root_store.cpp` contains a few TLS root certificates
- `regret.h` and `coolcrab.h` are unused image byte arrays