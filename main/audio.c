/*

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/i2s.h"
/* for IP stuff: */
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "pins.h"

static const char *TAG = "audio";

#define INCHANS 2
#define OUTCHANS 2
#define MAXCHANS 2
#define BLKSIZE 64

float soundin[INCHANS * BLKSIZE], soundout[OUTCHANS * BLKSIZE];
int outbit;

#define INTMAX 2147483648.

void senddacs( void)
{
    int i, ret;
    size_t written;
    long poodle[MAXCHANS * BLKSIZE];

#if 1
    ret = i2s_read(I2S_NUM_0, poodle, INCHANS*BLKSIZE*sizeof(short),
        &written, portMAX_DELAY);
    if (ret != ESP_OK)
        ESP_LOGE(TAG, "error reading");
    for (i = 0; i < BLKSIZE; i++)
    {
        soundin[i] = (1./INTMAX) * poodle[INCHANS * i];
        if (INCHANS > 1)
            soundin[i+BLKSIZE] = (1./INTMAX) * poodle[INCHANS * i + 1];
    }
#endif
#if 1
    for (i = 0; i < BLKSIZE; i+= OUTCHANS)
    {
#if 0   
        int ch1 = 32767*soundout[i];
        if (ch1 > 32767)
            ch1 = 32767;
        else if (ch1 < -32768)
            ch1 = -32768;
        poodle[OUTCHANS*i] = ch1;
        soundout[i] = 0;
        if (OUTCHANS > 1)
        {
            int ch2 = 32767*soundout[i+BLKSIZE];
             if (ch2 > (int)(INTMAX)-1)
                ch2 = (int)(INTMAX)-1;
            else if (ch2 < -(int)(INTMAX)+1)
                ch2 = -(int)(INTMAX)+1;
            poodle[OUTCHANS*i+1] = ch2;
            soundout[i + BLKSIZE] = 0;
        }
#else
        if (soundout[i] <= 0)
            poodle[OUTCHANS*i] = poodle[OUTCHANS*i+1] = 0;
        else if (outbit < 32)
        {
            poodle[OUTCHANS*i] = (1 << outbit);
            poodle[OUTCHANS*i+1] = 0;
        }
        else
        {
            poodle[OUTCHANS*i+ 1] = (1 << (outbit-32));
            poodle[OUTCHANS*i] = 0;
        }
            
#endif
    }
    ret = i2s_write(I2S_NUM_0, poodle, OUTCHANS*BLKSIZE*sizeof(short),
        &written, portMAX_DELAY);

    if (ret != ESP_OK)
        ESP_LOGE(TAG, "error writing");
#endif
}

#if 0  /* put this back later with a wifi-out queue */
static int adc_sock;
static struct sockaddr_in adc_dest_addr;

static void udp_open( void)
{
    adc_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    
    adc_dest_addr.sin_addr.s_addr = inet_addr(QUACKADDR);
    adc_dest_addr.sin_family = AF_INET;
    adc_dest_addr.sin_port = htons(QUACKPORT);
}

static void udp_send(void *msg, int len)
{
    int err = sendto(adc_sock, msg, len, 0,
        (struct sockaddr *)&adc_dest_addr, sizeof(adc_dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
}

typedef struct packet
{
    char p_header[16];
    short p_data[2*BLKSIZE];
} t_packet;

static void udp_send_quacktrip(float *ch1, float *ch2)
{
    int i;
    static int count;
    t_packet p;
    memset(&p.p_header, 0, 8);
    p.p_header[8] = count & 255;
    p.p_header[9] = (count >> 8) & 255;
    p.p_header[10] = (BLKSIZE & 255);
    p.p_header[11] = ((BLKSIZE >> 8) & 255);
    p.p_header[12] = 3; /* 48K */
    p.p_header[13] = 16;
    p.p_header[14] = 2; /* channel count ? */
    p.p_header[15] = 2;
    count++;
    for (i = 0; i < BLKSIZE; i++)
    {
        int ch1, ch2;
        ch1 = 32767*soundin[i];
         if (ch1 > 32767)
            ch1 = 32767;
        else if (ch1 < -32768)
            ch1 = -32768;
        ch2 = 32767*soundin[BLKSIZE+i];
        if (ch2 > 32767)
            ch2 = 32767;
        else if (ch2 < -32768)
            ch2 = -32768;
        p.p_data[i] = ch1;
        p.p_data[BLKSIZE+i] = ch2;
    }
    udp_send(&p, sizeof(p));
}
#endif

void audiotask(void *x)
{
    int nblock = 0;
    i2s_config_t i2s_cfg = {
        .mode = I2S_MODE_MASTER  | I2S_MODE_TX  | I2S_MODE_RX ,
        .sample_rate = 48000,
        .bits_per_sample = 32,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        /* .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, */
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = 1,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = 0,
#if 0  /* old pinouts ofr LyraT boards: */
        .bck_io_num = GPIO_NUM_5,
        .ws_io_num = GPIO_NUM_25,
        .data_out_num = GPIO_NUM_26,
        .data_in_num = GPIO_NUM_35
#else
        .bck_io_num = PIN_I2SBCLK,
        .ws_io_num = PIN_I2SLRCLK,
        .data_out_num = PIN_I2SRX,
        .data_in_num = PIN_I2STX
#endif
    };

    /*
    ESP_LOGI(TAG, "[1] open udp connection");
    udp_open();
    */

    ESP_LOGI(TAG, "[2] Start audio codec chip");
    i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);

    ESP_LOGI(TAG, "[2] start main loop");
    while (1)
    {
        static int phase;
        int i;
        for (i = 0; i < BLKSIZE; i++)
        {
            phase = ((phase + 1)%400);
            soundout[i] = soundout[i+BLKSIZE] = (0.1/200.) * (phase-200);
        }
        senddacs();
        nblock = (nblock + 1)&0xfffff;
        if ((nblock & 0x1ff) == 0)
        {
            outbit = (outbit+1)%64;
            ESP_LOGI(TAG, "bit %d / ADC %f %f",
                outbit, soundin[0], soundin[BLKSIZE]);
        }
        /* udp_send_quacktrip(soundin, soundin+BLKSIZE); */
        if ( !(nblock%1000))
        {
            if (nblock < 1e5)
                ;
        }
    }
}
