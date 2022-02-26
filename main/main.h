void wifi_init(void);   /* wifi.c - manage 802.11 connection */
void net_init( void);   /* net.c - sending and getting packets */
void net_sendudp(void *msg, int len, int port);

    /* audio.c */
void audiotask(void *x);
void audioparams(int in, int out, int bit16, int bitwise, int amp, float freq);
void audiotoggle(unsigned long xtoggle1a, unsigned long xtoggle1b,
    unsigned long xtoggle2a, unsigned long xtoggle2b);

    /* lidar.cpp */
void lidar_init( void);

    /* utils.c */
void timer_init( void);



/* task priorities */
#define  PRIORITY_GYRO 2
#define  PRIORITY_AUDIO 5
#define  PRIORITY_WIFI 6
#define  PRIORITY_LIDAR 3

#if !defined(CONFIG_LOCALE_NUMBER) || (CONFIG_LOCALE_NUMBER==0)
#define CONFIG_ESP_WIFI_SSID "your-SSID-here"
#define CONFIG_ESP_WIFI_PASSWORD "your-password-here"
#define CONFIG_ESP_WIFI_SENDADDR "base-station-IP-addr"
#define CONFIG_ESP_WIFI_SENDPORT 4498
#endif

#if CONFIG_LOCALE_NUMBER==1
#include "locale1.h"
#endif

#if CONFIG_LOCALE_NUMBER==2
#include "locale2.h"
#endif

#if CONFIG_LOCALE_NUMBER==3
#include "locale3.h"
#endif

#if CONFIG_LOCALE_NUMBER==4
#include "locale4.h"
#endif

