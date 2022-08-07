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
        ESP_LOGI(TAG, "found: %llx", tmpAddr);
        found = (numFound>0);
        cnt++;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

    if(found) {
        this->addr = tmpAddr;
        ESP_LOGI(TAG, "Devices found at %llx", (uint64_t)this->addr);
        this->found = true;
    } else{
        ESP_LOGI(TAG, "no devices found");    
        this->found = false;
    }
    
    ESP_LOGI(TAG, "init done");
}

int DallasSensor::readMeasurement(float &temperature, bool &tf, float &pressure, bool &pf, float &humidity, bool &hf ) {
    if(!this->found) {
        tf = false; pf = false; hf = false;
        return WSNOK;
    }
    float temp;

    if(ds18x20_measure_and_read(this->pin, this->addr, &temp)== ESP_OK) {
        temperature = temp;
        tf = true;
        pf = false;
        hf = false;
        ESP_LOGI(TAG, "read successful, %llx, %.02f", this->addr, temp);
        return WSOK;
    } else {
        ESP_LOGI(TAG, "read failed");
        return WSNOK;
    }
}

