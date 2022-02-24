set(COMPONENT_SRCS "main.c \
    wifi.c \
    net.c \
    audio.c \
    gyro.cpp \
    ../arduino/SparkFun_BNO080_Arduino_Library.cpp \
    ../arduino/SPI.cpp \
    ../arduino/Print.cpp \
    ../arduino/Wire.cpp \
    ../arduino/Stream.cpp \
    ../arduino/WString.cpp \
    ../arduino/esp32-hal-misc.c \
    ../arduino/esp32-hal-gpio.c \
    ../arduino/esp32-hal-spi.c \
    ../arduino/esp32-hal-i2c.c \
    ../arduino/esp32-hal-i2c-slave.c \
    ../arduino/esp32-hal-matrix.c \
    ../arduino/esp32-hal-cpu.c \
")

set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
