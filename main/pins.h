/* pin assignments - derived from cone_bot_diagram_complex_arts.drawio 
   and with proposed added pin assignments for audio. */

/*      steppers     LIDAR      audio     misc          */
#define PIN_TMC1DIR                                 32
#define PIN_TMC1STEP                                33
#define PIN_TMC1EN                                  27
#define PIN_TMC2DIR                                 0
#define PIN_TMC2STEP                                4
#define PIN_TMC2EN                                  23
#define                                   PIN_LED1  12
#define                                   PIN_LED2  13
#define              PIN_LIDARTX                    39  /* LIDAR->ESP(?) */
#define              PIN_LIDARRX                    15
#define              PIN_LIDARCTRL                  25
#define                         PIN_I2STX           39 /* ADC->ESP */
#define                         PIN_I2SRX           2 /* move LIDAR? */
#define                         PIN_I2SBCLK         14
#define                         PIN_I2SLRCLK        26

