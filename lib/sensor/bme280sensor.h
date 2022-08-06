#ifndef _BME280SENSOR_H
#define _BME280SENSOR_H

#include "basesensor.h"
#include "bmp280.h"

class BME280Sensor: public BaseSensor
{
private:
    bmp280_t temp_sensor;
public:
    BME280Sensor(int sda, int scl);
    virtual int readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf,  float &humidity, bool &hf );
};

#endif
