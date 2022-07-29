#ifndef _DALLASSENSOR_H
#define _DALLASSENSOR_H

#include "basesensor.h"
#include "ds18x20.h"

class DallasSensor: BaseSensor
{
private:
    gpio_num_t pin;
    ds18x20_addr_t addr;
public:
    DallasSensor(int tmpPin);
    virtual int readMeasurement(float &temp, float &pres, float &hum);
};

#endif
