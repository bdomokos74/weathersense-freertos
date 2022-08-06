#include "sensor_stub.h"

SensorStub::SensorStub(float f1, bool b1, float f2, bool b2, float f3, bool b3 ) : BaseSensor() {
    this->f1 = f1;
    this->f2 = f2;
    this->f3 = f3;
    this->b1 = b1;
    this->b2 = b2;
    this->b3 = b3;

}
int SensorStub::readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf,  float &humidity, bool &hf ) {
    temperature = f1; tf = b1;
    pressure = f2; pf = b2;
    humidity = f3; hf = b3;
}
