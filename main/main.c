/* Blink Example

   This example code is in the Public Domain (or CC0 licensed,
   at your option.)

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

/* for wifi: */
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
void wifi_init(void);

static const char *TAG = "example";

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

static int xyz_sock;
static struct sockaddr_in xyz_dest_addr;

static void udp_open( void)
{
    xyz_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    
    xyz_dest_addr.sin_addr.s_addr = inet_addr(CONFIG_ESP_WIFI_SENDADDR);
    xyz_dest_addr.sin_family = AF_INET;
    xyz_dest_addr.sin_port = htons(CONFIG_ESP_WIFI_SENDPORT);
}

static void udp_send(void *msg, int len)
{
    int err = sendto(xyz_sock, msg, len, 0,
        (struct sockaddr *)&xyz_dest_addr, sizeof(xyz_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}



void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();
    wifi_init();
    udp_open();
    if (!gyro_init())
        ESP_LOGI(TAG, "init failed");

    else while (1) {
        static int count = 0;
        float x, y, z;
        gyro_geteulerangles(&x, &y, &z);
        vTaskDelay(20 / portTICK_PERIOD_MS);
        if (++count >= 40)
        {
            char buf[80];
            sprintf(buf, "z %f %f %f;\n", x, y, z);
            udp_send(buf, strlen(buf));
            ESP_LOGI(TAG, "x=%f, y=%f, z=%f",
                (180/3.14159)*x, (180/3.14159)*y, (180/3.14159)*z);
            gpio_set_level(USER_LED1, s_led_state);
            gpio_set_level(USER_LED2, !s_led_state);
            s_led_state = !s_led_state;
            count = 0;
        }
    }
}
