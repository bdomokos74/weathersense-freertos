#include "wscommon.h"
#include "ds18x20.h"
#include "esp_log.h"

#define TAG "DALLAS"
#include "dallas_sensor.h"

#include "string.h"
#include <time.h>

DallasSensor::DallasSensor(int tmpPin) {
    ESP_LOGI(TAG, "init");
    //vTaskDelay(500 / portTICK_PERIOD_MS);
    
    this->pin = (gpio_num_t)tmpPin;
    if(onewire_reset(this->pin)) {
        ESP_LOGI(TAG, "One wire devices found");
    } else {
        ESP_LOGI(TAG, "no devices found on %d", (int)this->pin);
    }
    
    ds18x20_addr_t tmpAddr;
    int cnt = 0;
    bool found = false;
    while(!found && cnt < 5) {
        int numFound = ds18x20_scan_devices(this->pin, &tmpAddr, 1);
        found = (numFound>0);
        cnt++;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    if(found) {
        this->addr = tmpAddr;
        ESP_LOGI(TAG, "Devices found at %x", (int)this->addr);
    } else{
        ESP_LOGI(TAG, "no devices found");    
    }
    
    ESP_LOGI(TAG, "init done");
}

int DallasSensor::readMeasurement(float &temperature, float &pressure, float &humidity) {
    float temp;
    vTaskDelay(100 / portTICK_PERIOD_MS);

    //if(ds18x20_measure(this->pin, ds18x20_ANY, true)!= ESP_OK) {
    //    ESP_LOGI(TAG, "measure failed");
    //}
    //if(ds18x20_read_temperature(this->pin, ds18x20_ANY, &temp)== ESP_OK) {
    if(ds18x20_measure_and_read(this->pin, this->addr, &temp)== ESP_OK) {
        temperature = temp;
        return WSOK;
    }
    ESP_LOGI(TAG, "read failed");
    return WSNOK;
}

