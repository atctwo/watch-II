# watch2

Software and Hardware for my custom smartwatch

The system is built on an ESP32, and uses software written using the ESP-IDF (with the ESP32 Arduino Core as a component).  The system contains a few pre-built "apps".

| ![](extras/icons/watch.png) Watch Face            | ![](extras/icons/settings.png) Settings           | ![](extras/icons/calculator.png) Calculator       |![](extras/icons/stopwatch.png) Stopwatch          |
|---------------------------------------------------|---------------------------------------------------|---------------------------------------------------|---------------------------------------------------|
| ![](extras/icons/timer.png) Timer                 | ![](extras/icons/alarms.png) Alarms               | ![](extras/icons/file_browser.png) File Browser   | ![](extras/icons/notepad.png) Notepad (read only) |
| ![](extras/icons/ir_remote.png) IR Remote         | ![](extras/icons/image_viewer.png) Image Viewer   | ![](extras/icons/weather.png) Weather             | ![](extras/icons/wikipedia.png) Wikipedia         |
| ![](extras/icons/quiz.png) Quiz                   | ![](extras/icons/music_player.png) Music Player   | ![](extras/icons/radio.png) Internet Radio        | ![](extras/icons/ble_remote.png) BLE Remote       |
| ![](extras/icons/ltris.png) Tetris                |                                                   |                                                   |                                                   |

# Project Structure

- `components` ESP-IDF components
- `design_files` KiCad files for the schematic and PCB
- `docs` documentation for the system (see `docs/readme.md` for more detail)
- `extras` other stuff like icons, data files, and pinout for the original breadboard prototype
- `main` where all the code lives
- `spiffs` data and images for the SPIFFS partition

# Build Instructions

These are some rough instructions for installing and flashing the software on to an ESP32 board.  These steps are mainly aimed for Linux systems.

1. make sure ESP-IDF v4.3-beta1 is installed and setup
2. `git clone` this repository somewhere - `git clone https://github.com/atctwo/watch-II.git`
3. `cd` into the project directory
    - then run these commands:
    ```
    chmod +x install_libraries.sh
    ./install_libraries.sh
    ```
    - this script automatically clones all of the submodules that the system depends on, then makes any neccessary modifications
4. to build the software, run `idf.py build`, and to write the built firmware to an ESP32 board, run `idf.py flash` (see the ESP-IDF documentation for more detail on these commands)

# Credits
- *lots* of external libraries are used.  the details of these libraries can be found at [`docs/notes/libraries.md`](docs/notes/libraries.md)
- Most app icons are part of the [Papirus Icon Theme](https://github.com/PapirusDevelopmentTeam/papirus-icon-theme)
- Some icons made by [Icongeek26](https://www.flaticon.com/authors/icongeek26) from [Flaticon](https://www.flaticon.com/).
- The weather icons were designed by [Ian Amaral](https://dribbble.com/shots/5446697-Material-Design-inspired-weather-icons).
- The settings icon used in the IR Remote app is the one used by the Windows 10 Settings app.
- The icon used for the internet time button is [time zone](https://thenounproject.com/term/time-zone/2406165/) by Delwar Hossain from the Noun Project.
- The default font used is [HelvetiHand](https://www.dafont.com/helvetihand.font) by [Billy Snyder](https://www.dafont.com/billy-snyder.d4452).  It is modified slightly to add a ™ symbol.
