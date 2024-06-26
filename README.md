# Configure esp32 WIFI connectivity over BLTE

Store wifi credentials in EEPROM using BLTE for future connection

## Change default credentials
```cpp
// Default WIFI
const char *ssid = "your ssid";
const char *password = "your password";
```
Change BLTE device name
```cpp
String device_name = "ESP32-BT-Slave";
```

## Upload script
To upload the code press boot button on the esp.

## CMDS

WIFI Status
```

WIFI STATUS
```

Connect to wifi
```
WIFI CONNECT
```

Set new SSID
```
WIFI SSID : "new_ssid"
```

Set new PASSWORD
```
WIFI PWD : "new_pwd"
```

Consult STORED DATA
```
CACHE GET
```

Clear STORED PARAMS
```
CACHE CLEAR
```

Set new MQTT SERVER IP
```
SERVER IP : "new_server"
```

Connect to Server
```
SERVER IP : "new_server"
```

# Using PlatformIO

## Change Patitions table for esp32 nodemcu-32s
```
# Name   ,Type ,SubType  ,Offset   ,Size     ,Flags
nvs      ,data ,nvs      ,0x9000   ,0x5000   ,
otadata  ,data ,ota      ,0xe000   ,0x2000   ,
app0     ,app  ,ota_0    ,0x10000  ,0x300000 ,
spiffs   ,data ,spiffs   ,0x310000 ,0xE0000  ,
coredump ,data ,coredump ,0x3F0000 ,0x10000  ,
```
update platformio.ini file
```
[env:nodemcu-32s]
platform = espressif32
board = nodemcu-32s
framework = arduino
board_build.partitions = partitions.csv
upload_port = COM3
monitor_speed = 115200
```
