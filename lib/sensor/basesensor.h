#ifndef _BASESENSOR_H
#define _BASESENSOR_H

class BaseSensor
{
public:
    BaseSensor() {};
    virtual int readMeasurement(float &temp, float &pres, float &hum);
};

#endif
