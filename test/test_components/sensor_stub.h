#ifndef _SENSOR_STUB_H
#define _SENSOR_STUB_H

#include "basesensor.h"

class SensorStub : public BaseSensor {
private:
    float f1; bool b1; float f2; bool b2; float f3; bool b3;
public:
    SensorStub(float f1, bool b1, float f2, bool b2, float f3, bool b3 );
    virtual int readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf,  float &humidity, bool &hf );
};

#endif