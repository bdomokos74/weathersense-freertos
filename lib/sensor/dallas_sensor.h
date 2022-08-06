#ifndef _DALLASSENSOR_H
#define _DALLASSENSOR_H

#include "basesensor.h"
#include "ds18x20.h"

class DallasSensor: public BaseSensor
{
private:
    gpio_num_t pin;
    ds18x20_addr_t addr;
public:
    DallasSensor(int tmpPin);
    virtual int readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf,  float &humidity, bool &hf ) ;
};

#endif
