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
#include "main.h"
#include "pins.h"

static const char *TAG = "audio";

#define INCHANS 2
#define OUTCHANS 2
#define MAXCHANS 2
#define BLKSIZE 64

float soundin[INCHANS * BLKSIZE], soundout[OUTCHANS * BLKSIZE];
static int audio_amp;
static float audio_freq;
static int audio_printme;

static unsigned char audio_in = 0;
static unsigned char audio_out = 0;
static unsigned char audio_16bit = 0;
static unsigned char audio_bitwise = 0;

static unsigned char audio_newin = 0;
static unsigned char audio_newout = 0;
static unsigned char audio_new16bit = 0;
static unsigned char audio_restart = 0;

static unsigned long toggle1a, toggle1b, toggle2a, toggle2b;

#define BIGFATINT 0x7fffff  /* max. absolute value of a 24-bit int */

static float dumbcos(float phase)
{
    if (phase > 0.5)
        phase = 1-phase;
    phase -= 0.25;
    phase *= 4;
    if (phase < -1 )
        return (1);
    else if (phase > 1)
        return (-1);
    else return (-1.5 *  (phase - 0.3333*(phase*phase*phase)));
}

void senddacs( void)
{
    int i;
    size_t written;
    long intbuf[MAXCHANS * BLKSIZE];
    short sbuf[MAXCHANS * BLKSIZE];

    if (audio_in)
    {
        if (audio_16bit)
        {
            if (i2s_read(I2S_NUM_0, sbuf, INCHANS*BLKSIZE*sizeof(sbuf[0]),
                &written, portMAX_DELAY) != ESP_OK)
                    ESP_LOGE(TAG, "error reading");
            for (i = 0; i < BLKSIZE; i++)
            {
                intbuf[INCHANS*i] = (sbuf[INCHANS*i] << 16);
                intbuf[INCHANS*i + 1] = (sbuf[INCHANS*i + 1] << 16);
            }
        }
        else  if (i2s_read(I2S_NUM_0, intbuf, INCHANS*BLKSIZE*sizeof(intbuf[0]),
                &written, portMAX_DELAY) != ESP_OK)
                    ESP_LOGE(TAG, "error reading");
        for (i = 0; i < BLKSIZE; i++)
        {
            soundin[i] = (1./(256.*BIGFATINT)) * intbuf[INCHANS * i];
            if (INCHANS > 1)
                soundin[i+BLKSIZE] = (1./(256.*BIGFATINT)) *
                    intbuf[INCHANS * i + 1];
        }
    }
    if (audio_out)
    {
        for (i = 0; i < BLKSIZE; i++)
        {
            if (audio_bitwise)    /* for testing - toggle between 2 values */
            {   
                if (soundout[i] <= 0)
                {
                    intbuf[OUTCHANS*i] = toggle1a;
                    intbuf[OUTCHANS*i+1] = toggle2a;
                }
                else
                {
                    intbuf[OUTCHANS*i] = toggle1b;
                    intbuf[OUTCHANS*i+1] = toggle2b;
                }
            }
            else    /* play correct audio output */
            {
                long isamp = BIGFATINT * soundout[i];
                isamp = (isamp > BIGFATINT ? BIGFATINT :
                    (isamp < -BIGFATINT ? -BIGFATINT : isamp));
                intbuf[OUTCHANS*i] = (isamp << 8);
                soundout[i] = 0;
                if (OUTCHANS > 1)
                {
                    isamp = BIGFATINT * soundout[i + BLKSIZE];
                    isamp = (isamp > BIGFATINT ? BIGFATINT :
                        (isamp < -BIGFATINT ? -BIGFATINT : isamp));
                    intbuf[OUTCHANS*i+1] = (isamp << 8);
                    soundout[i + BLKSIZE] = 0;
                }
            }
            sbuf[OUTCHANS*i] = (intbuf[OUTCHANS*i])>>16;
            sbuf[OUTCHANS*i+ 1] = (intbuf[OUTCHANS*i + 1])>>16;
        }
        if (audio_printme)
        {
            for (i = 0; i < 3; i++)
                if (audio_16bit)
                    ESP_LOGI(TAG, "sample %d %d, %x %x", i, 
                        sizeof(sbuf[0]), sbuf[OUTCHANS*i],
                        sbuf[OUTCHANS*i+1]);
                else ESP_LOGI(TAG, "sample %d %d, %lx %lx", i, 
                        sizeof(intbuf[0]), intbuf[OUTCHANS*i],
                            intbuf[OUTCHANS*i+1]);         
            if (audio_bitwise)
                ESP_LOGI(TAG, "ch1 %lx %lx, ch2 %lx %lx",
                    (long)toggle1a, (long)toggle1b, (long)toggle2a, (long)toggle2b);
            audio_printme = 0;
        }
        if (audio_16bit)
        {
            if (i2s_write(I2S_NUM_0, sbuf, OUTCHANS*BLKSIZE*sizeof(sbuf[0]),
                &written, portMAX_DELAY) != ESP_OK)
                    ESP_LOGE(TAG, "error writing");
        }
        else
        {
            if (i2s_write(I2S_NUM_0, intbuf, OUTCHANS*BLKSIZE*sizeof(intbuf[0]),
                &written, portMAX_DELAY) != ESP_OK)
                    ESP_LOGE(TAG, "error writing");
        }
    }

}

#if 1

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
    net_sendudp(&p, sizeof(p), CONFIG_ESP_WIFI_SENDPORT);
}
#endif

#define LYRAT_PINOUT 0   /* true for LyraT, false for generic ESP32 boards */
#define OLDBREADBOARD 1  /* generic board w/o BNO085 */

void audiotask(void *x)
{
    int nblock = 0, opened = 0;
    i2s_config_t i2s_cfg = {
        .mode = I2S_MODE_MASTER  | I2S_MODE_TX  | I2S_MODE_RX ,
        .sample_rate = 48000,
        .bits_per_sample = 32,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        /* .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, */
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .dma_buf_count = 16,
        .dma_buf_len = 64,
        .use_apll = 1,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    };
    i2s_pin_config_t pin_config = {
        .mck_io_num = 0,
#if LYRAT_PINOUT
        .bck_io_num = GPIO_NUM_5,
        .ws_io_num = GPIO_NUM_25,
        .data_out_num = GPIO_NUM_26,
        .data_in_num = GPIO_NUM_35
#elif OLDBREADBOARD
        .bck_io_num = 13,
        .ws_io_num = 33,
        .data_out_num = 32,
        .data_in_num = 35
#else
        .bck_io_num = PIN_I2SBCLK,  /* defs for complex arts board in pins.h */
        .ws_io_num = PIN_I2SLRCLK,
        .data_out_num = PIN_I2SRX,
        .data_in_num = PIN_I2STX
#endif
    };

    while (1)
    {
        while (!audio_restart)
            vTaskDelay(10 / portTICK_PERIOD_MS);
        audio_16bit = audio_new16bit;
        audio_in = audio_newin;
        audio_out = audio_newout;
        audio_restart = 0;
        ESP_LOGI(TAG, "Start audio driver");
        if (opened)
            i2s_driver_uninstall(I2S_NUM_0);
        if (!audio_in && !audio_out)
            continue;
        i2s_cfg.bits_per_sample = (audio_16bit ? I2S_BITS_PER_SAMPLE_16BIT :
            I2S_BITS_PER_SAMPLE_32BIT);
        i2s_cfg.mode = I2S_MODE_MASTER  | (audio_out ? I2S_MODE_TX : 0) |
            (audio_in ? I2S_MODE_RX : 0);
        i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, NULL);
        i2s_set_pin(I2S_NUM_0, &pin_config);
        opened = 1;

        ESP_LOGI(TAG, "start audio loop");
        while (!audio_restart)
        {
            static float phase;
            float phaseinc = audio_freq * (1./48000.);
            float amp = audio_amp * audio_amp/10000.;
            int i;
            if (amp > 1)
                amp = 1;
            for (i = 0; i < BLKSIZE; i++)
            {
                phase += phaseinc;
                while (phase > 1)
                    phase -= 1;
                while (phase < 0)
                    phase += 1;
                soundout[i] = soundout[i+BLKSIZE] = amp * dumbcos(phase);
            }
            senddacs();
            nblock = (nblock + 1)&0xfffff;
            if ((audio_in && (nblock & 0x1ff) == 0))
            {
                ESP_LOGI(TAG, "ADC %f %f", soundin[0], soundin[BLKSIZE]);
            }
            if (audio_in)
                udp_send_quacktrip(soundin, soundin+BLKSIZE);
        }
    }
}

void audioparams(int in, int out, int bit16, int bitwise, int amp, float freq)
{
    if (audio_in != in || audio_out != out || audio_16bit != bit16)
    {
        audio_new16bit = bit16;
        audio_newin = in;
        audio_newout = out;
        audio_restart = 1;
    }
    audio_bitwise = bitwise;
    audio_amp = amp;
    audio_freq = freq;
    ESP_LOGI(TAG, "params in=%d out=%d 16bit=%d bitwise=%d amp=%d f=%f",
        in, out, bit16, bitwise, amp, freq);
    audio_printme = 1;
}

void audiotoggle(unsigned long xtoggle1a, unsigned long xtoggle1b,
    unsigned long xtoggle2a, unsigned long xtoggle2b)
{
    toggle1a = xtoggle1a;
    toggle1b = xtoggle1b;
    toggle2a = xtoggle2a;
    toggle2b = xtoggle2b;
    audio_printme = 1;
}
