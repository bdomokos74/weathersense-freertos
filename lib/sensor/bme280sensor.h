#ifndef _BME280SENSOR_H
#define _BME280SENSOR_H

#include "basesensor.h"
#include "bmp280.h"

class BME280Sensor: BaseSensor
{
private:
    bmp280_t temp_sensor;
public:
    BME280Sensor(int sda, int scl);
    virtual int readMeasurement(float &temp, float &pres, float &hum);
};

#endif
