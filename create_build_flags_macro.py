import subprocess
import os

revision = (
    subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
    .strip()
    .decode("utf-8")
)
print("-DGIT_REV=\\\"%s\\\"" % revision)
print("-DWIFI_SSID=%s" % os.environ.get('WIFI_SSID'))
print("-DWIFI_PASS=%s" % os.environ.get('WIFI_PASS'))
print("-Iazure_ex/azure-sdk-for-c/sdk/inc")
print("-Iazure_ex/azure-sdk-for-c/sdk/inc/azure")
print("-DCONFIG_I2CDEV_TIMEOUT=100000")
print("-DIOT_CONFIG_IOTHUB_FQDN=%s" % os.environ.get('IOT_CONFIG_IOTHUB_FQDN'))
print("-DIOT_CONFIG_DEVICE_ID=%s" % os.environ.get('IOT_CONFIG_DEVICE_ID'))
print("-DIOT_CONFIG_DEVICE_KEY=%s" % os.environ.get('IOT_CONFIG_DEVICE_KEY'))
print("-DSDA_GPIO=%s" % os.environ.get('SDA_GPIO'))
print("-DSCL_GPIO=%s" % os.environ.get('SCL_GPIO'))
print("-DONE_W_PIN=%s" % os.environ.get('ONE_W_PIN'))
print("-DLED_PIN=%s" % os.environ.get('LED_PIN'))
