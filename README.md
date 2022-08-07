
## Prerequisites

* Visual Studio Code, PlatformIO https://docs.platformio.org/en/latest/integration/ide/vscode.html#ide-vscode

## How to build

Open terminal from PlatformIO (PowerShell)

```
<projdir>\.platformio\penv\Scripts\Activate.ps1

cd azure_ex
git clone https://github.com/Azure/azure-sdk-for-c

#az_version.h will be created by this:
git co 1.3.1

$env:WIFI_SSID='\"sid\"'
$env:WIFI_PASS='\"pass\"'
$env:IOT_CONFIG_IOTHUB_FQDN='\"<iothubname>.azure-devices.net\"'
$env:IOT_CONFIG_DEVICE_ID='\"<deviceid>\"'
$env:IOT_CONFIG_DEVICE_KEY='\"<devicekey>\"'

pio run
```

### Run tests

Prerequisites for running tests: PlatformIO native environment installed and C/C++ compiler installed. See: https://piolabs.com/blog/insights/unit-testing-part-1.html

```
pio test -e native
```

### Upload and monitor

```
pio run -t upload | pio device monitor
```

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html


### Cert

```
azure-sdk-for-c\sdk\samples\iot\aziot_esp32\New-TrustedCertHeader.ps1
Get-FileHash -Path "C:\Users\bdomo\ws\weathersense-device2/tmp/BaltimoreCyberTrustRoot.crt" -Algorithm SHA1
Invoke-WebRequest -Uri https://cacerts.digicert.com/BaltimoreCyberTrustRoot.crt.pem -OutFile ca1.pem
Openssl.exe x509 -inform DER -outform PEM -in tmp/BaltimoreCyberTrustRoot.crt -out ca2.pem
```

See also: https://techcommunity.microsoft.com/t5/internet-of-things-blog/mosquitto-client-tools-and-azure-iot-hub-the-beginning-of/ba-p/2824717


### Error: twin requests causes MQTT disconnect

Solution: 

Create the mqtt_username like this: `<iot_hub_name>.azure-devices.net/<device_id>/?api-version=2021-04-12`.

Without the api-version, the mqtt connection gets disconnected.


### Deep sleep

https://github.com/espressif/esp-idf/blob/c2ccc383dae2a47c2c2dc8c7ad78175a3fd11361/examples/system/deep_sleep/main/deep_sleep_example_main.c

To persist variables between wakeups see (the second way): https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/deep-sleep-stub.html#loading-data-into-rtc-memory and [rtc_wake_stub_storage.c](lib/telemetry/rtc_wake_stub_storage.c)


### IDP config

pio run -t menuconfig

configUSE_TRACE_FACILITY = 1


### boards


https://github.com/platformio/platform-espressif32/tree/master/boards


https://github.com/platformio/platform-espressif32/blob/master/boards/featheresp32.json
https://www.adafruit.com/product/3405


https://github.com/platformio/platform-espressif32/blob/master/boards/adafruit_feather_esp32_v2.json
https://www.adafruit.com/product/5400


# check image

https://learn.adafruit.com/adafruit-esp32-s2-tft-feather/factory-reset

~/.platformio/packages/tool-esptoolpy/esptool.py --chip esp32 image_info .pio/build/featheresp32/firmware.bin 

~/.platformio/packages/tool-esptoolpy/esptool.py --chip esp32 image_info .pio/build/featheresp32/firmware.bin 
esptool.py v3.3
Image version: 1
Entry point: 40081c10
7 segments

Segment 1: len 0x25798 load 0x3f400020 file_offs 0x00000018 [DROM]
Segment 2: len 0x04434 load 0x3ffb0000 file_offs 0x000257b8 [BYTE_ACCESSIBLE,DRAM]
Segment 3: len 0x0641c load 0x40080000 file_offs 0x00029bf4 [IRAM]
Segment 4: len 0x99324 load 0x400d0020 file_offs 0x00030018 [IROM]
Segment 5: len 0x108ec load 0x4008641c file_offs 0x000c9344 [IRAM]
Segment 6: len 0x00064 load 0x400c0000 file_offs 0x000d9c38 [RTC_IRAM]
Segment 7: len 0x00048 load 0x50000000 file_offs 0x000d9ca4 [RTC_DATA]
Checksum: ce (valid)
Validation Hash: 61302172867a81c99ff62e00115e642a0a1226969ac0b2b504c32fe9ac64e2c6 (valid)



readelf.py -h -l  .pio/build/featheresp32/firmware.elf 

------
ets Jul 29 2019 12:21:46
rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0018,len:4
load:0x3fff001c,len:1044
load:0x40078000,len:10124
load:0x40080400,len:5856
entry 0x400806a8
I (311) cpu_start: Pro cpu up.
D (311) efuse: In EFUSE_BLK0__DATA3_REG is used 1 bits starting with 15 bit
D (311) efuse: In EFUSE_BLK0__DATA5_REG is used 1 bits starting with 20 bit
D (317) efuse: In EFUSE_BLK0__DATA3_REG is used 3 bits starting with 9 bit
D (324) efuse: In EFUSE_BLK0__DATA3_REG is used 1 bits starting with 2 bit
I (331) cpu_start: Starting app cpu, entry point is 0x40081b58
I (0) cpu_start: App cpu up.
D (346) clk: RTC_SLOW_CLK calibration value: 3386048





cpu_start.c:416 call_start_cpu0() 'pro cpu up'
    -> cpu_start.c:214 start_other_core()  'Starting app cpu, entry point is...'
    -> ets_set_appcpu_boot_addr(call_start_cpu1) // app cpu will call after app cpu boot completed
|
cpu_start.c:153 call_start_cpu1()

startup.c:389 start_cpu0_default() 'Pro cpu start user code'

bootloader_init.c:351 bootloader_init
    bootloader_init.c:394 bootloader_print_banner '2nd stage bootloader'



---- flashing manually
~/.platformio/packages/tool-esptoolpy/esptool.py write_flash 0x8000 .pio/build/featheresp32/partitions.bin 
~/.platformio/packages/tool-esptoolpy/esptool.py write_flash 0x1000 .pio/build/featheresp32/bootloader.bin 


NOTES:

- make the scl/sda twin parameters, as esp32 feather has different pinouts

https://github.com/cpetrich/counterfeit_DS18B20

### OTA

https://docs.platformio.org/en/latest/platforms/espressif32.html#partition-tables

https://github.com/espressif/esp-idf/tree/36f49f361c001b49c538364056bc5d2d04c6f321/examples/system/ota/advanced_https_ota


~/.platformio/packages/tool-esptoolpy/esptool.py read_flash 0x8000 8192 tmp/part_orig.bin


openssl s_client -showcerts -servername weathersenselake.blob.core.windows.net -connect weathersenselake.blob.core.windows.net:443 </dev/null

openssl req -x509 -newkey rsa:2048 -keyout ca_key.pem -out ca_cert.pem -days 365 -nodes

To change firmwareVersion update version.txt.

To trigger OTA update, set device twin's desired properties fwVersion to the new firmware version.

### Notes

- do not request twin update after wdt reset - stay in connected mode instead of possibly sleep mode


- Do not use pin 15 on ESP32 CAM, otherwise the wifi won't connect
https://dr-mntn.net/2021/02/using-the-sd-card-in-1-bit-mode-on-the-esp32-cam-from-ai-thinker


https://electronics.stackexchange.com/questions/94204/ds1822-1-wire-sensor-parasitic-power-and-strong-pull-up-circuit

//ret.append(f'{ts},{temp},{pressure},{humidity},{bat},{offset},{t1},{t2},{mid}\n')
// temp = (t1+t2)/2

- DS12B20 recognized, but always reads 85C when used in parasite mode
Solution: Plug to VIN of ESP32 Devkit 1, so tahat it gets 5V instead of 3.3V.
