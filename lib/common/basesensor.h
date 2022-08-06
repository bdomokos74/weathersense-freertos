#ifndef _BASESENSOR_H
#define _BASESENSOR_H

class BaseSensor
{
public:
    BaseSensor() {};
    virtual int readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf,  float &humidity, bool &hf ) = 0;
};

#endif
