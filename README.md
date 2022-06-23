
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

pio run
```

### Upload and monitor

```
pio run -t upload | pio device monitor
```


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