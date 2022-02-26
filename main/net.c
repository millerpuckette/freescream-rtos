#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "main.h"

/* lwip (lightweight IP") for sockets: */
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <stdio.h>
#include <string.h>

static const char *TAG = "net";

void udpreceivertask(void *z)
{
    char rx_buffer[128];
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(4499);
    ip_protocol = IPPROTO_IP;

    int rcv_sock = socket(AF_INET, SOCK_DGRAM, ip_protocol);
    if (rcv_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }

    int err = bind(rcv_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Waiting for data");

    while (1) {

        struct sockaddr_storage source_addr; /* enough for IPv4 or IPv6 */
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(rcv_sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
            (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            vTaskDelay(5000 / portTICK_PERIOD_MS);        
            continue;
        }
        else {
            rx_buffer[len] = 0;
            if (rx_buffer[0] == 'a')    /* audio settings and on/off */
            {
                int p1, p2, p3, p4, p5;
                float f1;
                if (sscanf(rx_buffer+1, "%d%d%d%d%d%f",
                    &p1, &p2, &p3, &p4, &p5, &f1) == 6)
                        audioparams(p1, p2, p3, p4, p5, f1);
            }
            else if (rx_buffer[0] == 't')    /* set testing audio params */
            {
                unsigned long p1, p2, p3, p4;
                int z = sscanf(rx_buffer+1, " 0x%lx 0x%lx 0x%lx 0x%lx",
                    &p1, &p2, &p3, &p4);
                if (z == 4)
                    audiotoggle(p1, p2, p3, p4);
                else ESP_LOGI(TAG, "t: %d", z);
            }
            else if (rx_buffer[0] == 'p')    /* ping - just return a msg */
            {
                net_sendudp(rx_buffer, len, 4498);
            }
            else ESP_LOGI(TAG, "rcv: %s", rx_buffer);
        }
    }

}

static int xyz_sock;
static struct sockaddr_in xyz_dest_addr;

void net_init( void)
{
        /* socket to send over -- could use same one? */
    xyz_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    
    xyz_dest_addr.sin_addr.s_addr = inet_addr(CONFIG_ESP_WIFI_SENDADDR);
    xyz_dest_addr.sin_family = AF_INET;
        /* this will get overridden later: */
    xyz_dest_addr.sin_port = htons(CONFIG_ESP_WIFI_SENDPORT);

    xTaskCreate(udpreceivertask, "udprcv", 3000, NULL, PRIORITY_WIFI, NULL);
}

void net_sendudp(void *msg, int len, int port)
{
    int err;
    xyz_dest_addr.sin_port = htons(port);
    err = sendto(xyz_sock, msg, len, 0,
        (struct sockaddr *)&xyz_dest_addr, sizeof(xyz_dest_addr));
    if (err < 0) {
        static int errorcount;
        errorcount++;
        if (errorcount > 100)
        {
            ESP_LOGE(TAG, "100 errors: errno %d", errno);
            errorcount = 0;
        }
    }
}
