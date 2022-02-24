/* pin assignments - derived from cone_bot_diagram_complex_arts.drawio 
   and with proposed added pin assignments for audio. */

/*      steppers   */
#define PIN_TMC1DIR     32
#define PIN_TMC1STEP    33
#define PIN_TMC1EN      27
#define PIN_TMC2DIR     0
#define PIN_TMC2STEP    4
#define PIN_TMC2EN      23
    /* LEDs */
#define  PIN_LED1       12
#define  PIN_LED2       13
    /* LIDAR (UART) */
#define  PIN_LIDARTX    39  /* LIDAR->ESP (MOVED) */
#define  PIN_LIDARRX    15
#define  PIN_LIDARCTRL  25
    /* audio (I2S) */

#define  PIN_I2STX      39 /* ADC->ESP */
#define  PIN_I2SRX      2  /* need to move LIDAR RX to free this */
#define  PIN_I2SBCLK    14
#define  PIN_I2SLRCLK   26
    /* IMU (gyro / BNO085) */
#define PIN_IMUCSP      17
#define PIN_IMUWAK      16
#define PIN_IMUINT      35
#define PIN_IMURST      5
    /* SPI (gyro / BNO085) */
#define PIN_SPICLK      18
#define PIN_SPIMISO     19
#define PIN_SPIMOSI     23
