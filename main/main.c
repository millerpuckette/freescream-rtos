/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "gyro.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 12
#define USER_LED1 12
#define USER_LED2 13

static uint8_t s_led_state = 0;


static void configure_led(void)
{
    gpio_reset_pin(USER_LED1);
    gpio_reset_pin(USER_LED2);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(USER_LED1, GPIO_MODE_OUTPUT);
    gpio_set_direction(USER_LED2, GPIO_MODE_OUTPUT);
}

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();
    if (!gyro_init())
        ESP_LOGI(TAG, "init failed");

    else while (1) {
        static int count = 0;
        float x, y, z;
        gyro_geteulerangles(&x, &y, &z);
        ESP_LOGI(TAG, "x=%f, y=%f, z=%f",
            (180/3.14159)*x, (180/3.14159)*y, (180/3.14159)*z);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        if (++count >= 40)
        {
            gpio_set_level(USER_LED1, s_led_state);
            gpio_set_level(USER_LED2, !s_led_state);
            s_led_state = !s_led_state;
            count = 0;
        }
    }
}
