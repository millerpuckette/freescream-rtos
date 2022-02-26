/*
 * RoboPeak RPLIDAR Arduino Example
 * This example shows the easy and common way to fetch data from an RPLIDAR
 * 
 * You may freely add your application code based on this template
 *
 * USAGE:
 * ---------------------------------
 * 1. Download this sketch code to your Arduino board
 * 2. Connect the RPLIDAR's serial port (RX/TX/GND) to your Arduino board (Pin 0 and Pin1)
 * 3. Connect the RPLIDAR's motor ctrl pin to the Arduino board pin 3 
 */
 
/* 
 * Copyright (c) 2014, RoboPeak 
 * All rights reserved.
 * RoboPeak.com
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "../arduino/HardwareSerial.h"
#include "../arduino/analogWrite.h"
 
// This sketch code is based on the RPLIDAR driver library provided by RoboPeak
#include "../arduino/RPLidar.h"
#include "main.h"
#include "pins.h"

// Create driver instance 
RPLidar lidar;

#define LIDAR_VALID 1
#define LIDAR_STARTBIT 2

typedef struct _lidarpoint
{
    short p_distance;   /* distance, millimeters */
    short p_angle;      /* angle, 0-36000, 100ths of a degree */
    short p_quality;    /* 8-bit quantity; don't know what units */
    short p_flags;      /* defs above */
} t_lidarpoint;

#define NHISTORY 100
static t_lidarpoint lidar_history[NHISTORY];
static int lidar_nextpoint;

static void lidar_task(void *unused)
{
    while (1)
    {
        if (IS_OK(lidar.waitPoint()))
        {
            float angle = fmod(lidar.getCurrentPoint().angle * (1./360.), 1.);
            if (angle < 0)
                angle += 1;
            lidar_history[lidar_nextpoint].p_angle =
                36000. * angle;
            lidar_history[lidar_nextpoint].p_distance =
                lidar.getCurrentPoint().distance;
            lidar_history[lidar_nextpoint].p_quality =
                lidar.getCurrentPoint().quality;
            lidar_history[lidar_nextpoint].p_flags = (LIDAR_VALID |
                (lidar.getCurrentPoint().startBit ? LIDAR_STARTBIT : 0));
            if (++lidar_nextpoint >= NHISTORY)
                lidar_nextpoint = 0;
        }
        else
        {
            analogWrite(PIN_LIDARCTRL, 0); //stop the rplidar motor
    
                // try to detect RPLIDAR... 
            rplidar_response_device_info_t info;
            if (IS_OK(lidar.getDeviceInfo(info, 100)))
            {
               // detected...
               lidar.startScan();

               // start motor rotating at max allowed speed
               analogWrite(PIN_LIDARCTRL, 255);

               vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
    }
}

void lidar_init( void)
{
        // bind the RPLIDAR driver to the arduino hardware serial
    Serial2.begin(115200, SERIAL_8N1, PIN_LIDARTX, PIN_LIDARRX);
    lidar.begin(Serial2);

        // set pin mode for motor speed control analog output
    pinMode(PIN_LIDARCTRL, OUTPUT);

    xTaskCreate(lidar_task, "lidar", 3000, NULL, PRIORITY_LIDAR, NULL);
}
