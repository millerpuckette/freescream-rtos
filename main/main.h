void wifi_init(void);

void net_init( void);
void net_sendudp(void *msg, int len, int port);

void audiotask(void *x);
void audioparams(int in, int out, int bit16, int bitwise, int amp, float freq);

void audiotoggle(unsigned long xtoggle1a, unsigned long xtoggle1b,
    unsigned long xtoggle2a, unsigned long xtoggle2b);

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

