#!/usr/bin/env bash
# script to install and modify the libraries that watch2 depends on

echo "--- watch2 Library Installer ---"

echo "--- Updating Git Submodules ---"
# update git submodules
git submodule update --recursive --init

echo "--- Modifying Time.h ---"

# remove Time.h
rm main/libraries/Time/Time.h

echo "--- Modifying TFT_eSPI.h ---"

# copy watch2 setup file to TFT_eSPI folder
cp extras/Setup_watch2.h main/libraries/TFT_eSPI/User_Setups/.

# uncomment default setup
sed -i '/<User_Setup.h>/s/^/\/\//g' main/libraries/TFT_eSPI/User_Setup_Select.h

# include watch2 setup
sed -i '/Default setup is root library folder/a #include <User_Setups/Setup_watch2.h>' main/libraries/TFT_eSPI/User_Setup_Select.h

echo "--- Finished Installing Libraries :) ---"
