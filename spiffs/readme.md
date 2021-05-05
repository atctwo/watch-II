# SPIFFS

Here you will find all the files needed to setup the SPIFFS partition for watch2.

The files `spiffs_8mb.bin` and `spiffs_16mb.bin` are pre-built images, and are probably okay for anyone who doesn't need to modify the contents of SPIFFS.  The images were created from the contents of the `data` folder, which contains everything that needs to be placed in SPIFFS.  To flash this image, run one of the lines below:

```bash
# if you're using an 8 MB flash
esptool.py --port /dev/ttyUSB0 write_flash 0x340000 spiffs_8mb.bin  

# if you're using a 16 MB flash
esptool.py --port /dev/ttyUSB0 write_flash 0xE20000 spiffs_16mb.bin  
```

If you do need to generate a SPIFFS image, use one of these lines:

```bash
# if you're using an 8 MB flash
python ${IDF_PATH}/components/spiffs/spiffsgen.py 0x4c0000 data spiffs_8mb.bin

# if you're using a 16 MB flash
python ${IDF_PATH}/components/spiffs/spiffsgen.py 0x1e0000 data spiffs_16mb.bin
```