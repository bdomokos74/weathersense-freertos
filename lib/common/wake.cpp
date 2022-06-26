#include "wake.h"
#include "az_core.h"
#include "esp_log.h"

#include "esp_sleep.h"
#include "esp_timer.h"

#define TAG "WAKE"

void logWakeReason() {
    char reasonStr[100];
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
    az_span result = AZ_SPAN_FROM_BUFFER(reasonStr);
    result = az_span_copy(result, az_span_create_from_str("Wakeup caused by "));
    switch(reason) {
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_UNDEFINED"));
            break;
        case ESP_SLEEP_WAKEUP_ALL:          //!< Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_ALL"));
            break;
        case ESP_SLEEP_WAKEUP_EXT0:         //!< Wakeup caused by external signal using RTC_IO
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_EXT0 - external signal using RTC_IO"));
            break;
        case ESP_SLEEP_WAKEUP_EXT1:         //!< Wakeup caused by external signal using RTC_CNTL
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_EXT1 - external signal using RTC_CNTL"));
            break;
        case ESP_SLEEP_WAKEUP_TIMER:        //!< Wakeup caused by timer
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_TIMER"));
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:     //!< Wakeup caused by touchpad
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_TOUCHPAD"));
            break;
        case ESP_SLEEP_WAKEUP_ULP:       //!< Wakeup caused by ULP program
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_ULP"));
            break;
        case ESP_SLEEP_WAKEUP_GPIO:         //!< Wakeup caused by GPIO (light sleep only)
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_GPIO"));
            break;
        case ESP_SLEEP_WAKEUP_UART:          //!< Wakeup caused by UART (light sleep only)
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_UART"));
            break;
        case ESP_SLEEP_WAKEUP_WIFI:              //!< Wakeup caused by WIFI (light sleep only)
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_WIFI"));
            break;
        case ESP_SLEEP_WAKEUP_COCPU:             //!< Wakeup caused by COCPU int
                result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_COCPU - COCPU int"));
            break;
        case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:   //!< Wakeup caused by COCPU crash
                result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG - COCPU crash"));
            break;
        case ESP_SLEEP_WAKEUP_BT:           //!< Wakeup caused by BT (light sleep only)
            result = az_span_copy(result, az_span_create_from_str("ESP_SLEEP_WAKEUP_BT"));
            break;
        default:
            result = az_span_copy(result, az_span_create_from_str("UNKNONW"));
        break;
    }
    result = az_span_copy_u8(result, 0);
    ESP_LOGI(TAG, "%s", reasonStr);
}