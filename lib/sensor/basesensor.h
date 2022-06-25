#ifndef _BASESENSOR_H
#define _BASESENSOR_H

#define WSOK 1
#define WSNOK 0

class BaseSensor
{
public:
    BaseSensor() {};
    virtual int readMeasurement(float &temp, float &pres, float &hum);
};

#endif
