
azure-sdk-for-c\sdk\samples\iot\aziot_esp32\New-TrustedCertHeader.ps1

Get-FileHash -Path "C:\Users\bdomo\ws\weathersense-device2/tmp/BaltimoreCyberTrustRoot.crt" -Algorithm SHA1
Invoke-WebRequest -Uri https://cacerts.digicert.com/BaltimoreCyberTrustRoot.crt.pem -OutFile ca1.pem

Openssl.exe x509 -inform DER -outform PEM -in tmp/BaltimoreCyberTrustRoot.crt -out ca2.pem

https://techcommunity.microsoft.com/t5/internet-of-things-blog/mosquitto-client-tools-and-azure-iot-hub-the-beginning-of/ba-p/2824717


TODO:
integrate azure-iot lib, similar to:
C:\Users\bdomo\ws\Internet-of-Things-with-ESP32\common\components\esp-aws-iot

NOTE: 