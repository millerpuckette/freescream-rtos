/* Blink Example

   This example code is in the Public Domain (or CC0 licensed,
   at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "main.h"
#include "gyro.h"
#include "pins.h"

#include <stdio.h>
#include <string.h>

static const char *TAG = "example";

static uint8_t s_led_state = 0;

static void configure_led(void)
{
        /* Set GPIOs as a push/pull outputs */
    gpio_reset_pin(PIN_LED1);
    gpio_reset_pin(PIN_LED2);
    gpio_set_direction(PIN_LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_LED2, GPIO_MODE_OUTPUT);
}

static void sendgyro(float x, float y, float z)
{
    unsigned char packet[7];
    int angle1 = (x >= 0 ? x : x + 2*3.14159) * 10000;
    int angle2 = (y >=  0 ? y : y + 2*3.14159) * 10000;
    int angle3 = (z >=  0 ? z : z + 2*3.14159) * 10000;
    packet[0] = 1;
    packet[1] = (angle1 & 0xff00)>>8;
    packet[2] = (angle1 & 0xff);
    packet[3] = (angle2 & 0xff00)>>8;
    packet[4] = (angle2 & 0xff);
    packet[5] = (angle3 & 0xff00)>>8;
    packet[6] = (angle3 & 0xff);
    net_sendudp(packet, 7, CONFIG_ESP_WIFI_SENDPORT);
}

void app_main(void)
{
    configure_led();
    wifi_init();
    net_init();
    xTaskCreate(audiotask, "audiotask", 4*1024, NULL, PRIORITY_AUDIO, NULL);
    if (!gyro_init())
        ESP_LOGI(TAG, "gyro init failed");

    else while (1) {
        static int count = 0;
        float x, y, z;
        gyro_geteulerangles(&x, &y, &z);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        if (++count >= 40)
        {
            char buf[80];
            int msec = esp_timer_get_time()/1000;
            sprintf(buf, "z %d %f %f %f;\n", msec, x, y, z);
            net_sendudp(buf, strlen(buf), CONFIG_ESP_WIFI_SENDPORT);
            ESP_LOGI(TAG, "x=%f, y=%f, z=%f",
                (180/3.14159)*x, (180/3.14159)*y, (180/3.14159)*z);
            gpio_set_level(PIN_LED1, s_led_state);
            gpio_set_level(PIN_LED2, !s_led_state);
            s_led_state = !s_led_state;
            count = 0;
        }
    }
}
