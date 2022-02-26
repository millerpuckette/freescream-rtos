Code for Free Ice Cream robots.  Unless otherwise noted in source files,
this is copyright H0TClub, with BSD license.


This project uses the Espressif "IDF" (something-development-framework) which
includes freeRTOS.

SETTING UP COMPILE CHAIN IN LINUX:
follow instructions in 
    https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/
then from linux command line (edit for your own private path):

export IDF_TOOLS_PATH=/home/msp/bis/work/esp/toolchain
export IDF_PATH=/home/msp/bis/work/esp/esp-idf
. /home/msp/bis/work/esp/esp-idf/export.sh

idf.py menuconfig
    --> go to "free ice cream" and customize WIFI settings
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor 

files:
main/ - our code
   main.c - main program
   wifi.c - IN PROGRESS, not in project yet
   gyro.cpp, gyro.h - task to read roll, pitch, and yaw from BNO085
arduino/ - shit copied over from arduino to get BNO085 working
sdkconfig.defaults - default configuration for building sdkconfig



dolist:
time tags for gyro and lidar to align them together (or use gyro latest data
to rotate lidar data?)

time measurement - measure time differences between incoming pings
