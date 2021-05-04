# Extras

- `icons` where most of the system icons are kept.  the actual code loads the icons from SPIFFS.  the icons used to generate the SPIFFS image are stored under `/spiffs/data`, meaning that this folder might not be completely up-to-date (or well organised)
- `images` some test images that were used at one point
- `i2s mic samples` and `i2s mic samples scaled` i can't remember what these were used for
- `imageconverter.py` a python script to convert a image (eg: png, jpg) to an RGB565 array for drawing to a TFT (most of the system icons are now loaded from SPIFFS as pngs)
- `ir.json` sample IR Remote profile for the IR Remote app
- `pinout.txt` a really old pinout for the original breadboard prototype
- `pinout.xlsx` a much more up to date pinout for the original breadboard prototype
- `Setup_watch2.h` setup file for TFT_eSPI (this is installed automatically by `install_libraries.sh`)