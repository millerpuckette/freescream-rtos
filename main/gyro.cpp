/* use SparkFun BNO080 library (which uses stuf from Arduino) to get orientation of Complex Arts
Sensorboard */

#include "../arduino/SparkFun_BNO080_Arduino_Library.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gyro.h"
#include "pins.h"

/* IMU device */
BNO080 myIMU;

/* SPI device */
SPIClass spiPort;
uint32_t spiPortSpeed = 100*1000;

/* IMU pins */
byte imuCSPin = PIN_IMUCSP;
byte imuWAKPin = PIN_IMUWAK;
byte imuINTPin = PIN_IMUINT;
byte imuRSTPin = PIN_IMURST;

/* SPI pins */
byte spiCLK = PIN_SPICLK;
byte spiMISO = PIN_SPIMISO;
byte spiMOSI = PIN_SPIMOSI;

char TAG[] = "gyro";

static float gyro_roll, gyro_pitch, gyro_yaw;

extern "C" void gyro_geteulerangles(float *rollp, float *pitchp, float *yawp)
{
    *rollp = gyro_roll;
    *pitchp = gyro_pitch;
    *yawp = gyro_yaw;
}

extern "C" void gyro_task(void *arg)
{
    while (1)
    {
        if (myIMU.dataAvailable() == true)
        {
            gyro_roll = myIMU.getRoll();
            gyro_pitch = myIMU.getPitch();
            gyro_yaw = myIMU.getYaw();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);        
    }
}

extern "C" int gyro_init(void)
{
        /* initialize SPI interface for the BNO085 */
    spiPort.begin(spiCLK, spiMISO, spiMOSI, imuCSPin);

        /* initialize IMU */
    if (myIMU.beginSPI(imuCSPin, imuWAKPin, imuINTPin, imuRSTPin, spiPortSpeed, spiPort) == false)
        return (0);

        /* enable "rotation vector", readings will be produced every 50 ms */
    myIMU.enableRotationVector(50);
    
        /* kick off a polling task */
    xTaskCreate(gyro_task, "gyro", 2048, NULL, 2, NULL);
    return (1);
}

