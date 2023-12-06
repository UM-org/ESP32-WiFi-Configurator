#include "BluetoothSerial.h"
#include <EEPROM.h>
#include <WiFi.h>

// Default WIFI
const char *ssid = "your ssid";
const char *password = "your password";
String storedSSID;
String storedPWD;

WiFiClient espClient;
BluetoothSerial ESP_BT;

String device_name = "ESP32-BT-Slave";

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

void connectToWifi(String ssid, String password)
{
  int maxTries = 10;
  int tries = 0;
  Serial.println("CONNECTING WIFI WITH : ");
  Serial.println("SSID : " + ssid);
  Serial.println("PASSWORD : " + password);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && tries < maxTries)
  {
    Serial.println("Connecting to WiFi..");
    ESP_BT.print("Connecting to WiFi..");
    tries++;
    delay(500);
    // digitalWrite(22, LOW);
    // delay(100);
    // digitalWrite(22, HIGH);
    // delay(100);
    // digitalWrite(22, LOW);
    // delay(100);
    // digitalWrite(22, HIGH);
    // delay(100);
    // digitalWrite(22, LOW);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Fail to connect to WiFi..");
    ESP_BT.println("Fail to connect to WiFi..");
  }
  else
  {
    Serial.println("Connected to WiFi..");
    ESP_BT.println("Connected to WiFi..");
  }
}

String getStoredSSID()
{
  String storedSSID;
  for (int i = 0; i < 32; ++i)
  {
    if (EEPROM.read(i) != 255)
      storedSSID += char(EEPROM.read(i));
  }
  return storedSSID;
}

String getStoredPWD()
{
  String storedPWD;
  for (int i = 0; i < 96; ++i)
  {
    if (EEPROM.read(i+32) != 255)
      storedPWD += char(EEPROM.read(i+32));
  }
  return storedPWD;
}

void saveSSID(String ssid)
{
  for (int i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.commit();
}

void savePWD(String pwd)
{
  for (int i = 0; i < pwd.length(); ++i)
  {
    EEPROM.write(i + 32, pwd[i]);
  }
  EEPROM.commit();
}

void clearEEPROM()
{
  for (int i = 0; i < 96; ++i)
  {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}

void setup()
{
  delay(1000);
  Serial.begin(115200);
  ESP_BT.begin(device_name);
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  EEPROM.begin(128);
  Serial.println("start connecting ...");
  // clearEEPROM(); // uncommit whene dev

  storedSSID = getStoredSSID();
  if (storedSSID == NULL)
  {
    storedSSID = ssid;
  }

  storedPWD = getStoredPWD();
  Serial.println(storedPWD == NULL);
  if (storedPWD == NULL)
  {
    storedPWD = password;
  }
  connectToWifi(storedSSID, storedPWD);
}

String cleanString(String txt)
{
  String out = "";
  for (int i = 0; i < txt.length() - 1; i++)
  {
    if (!isSpace(txt[i]))
      out += txt[i];
  }
  return out;
}

void checkBLTEConfig()
{
  if (ESP_BT.available())
  {
    String bte_serial = cleanString(ESP_BT.readStringUntil('\n'));
    if (bte_serial.startsWith("WIFI"))
    {
      if (bte_serial.indexOf("CONNECT") != -1)
      {
        connectToWifi(storedSSID, storedPWD);
      }

      if (bte_serial.indexOf("SSID") != -1)
      {
        int start = bte_serial.indexOf(":") + 1;
        String ssid = bte_serial.substring(start, bte_serial.length());
        saveSSID(ssid);
        storedSSID = getStoredSSID();
        ESP_BT.println("SSID stored : {" + storedSSID+"}");
      }

      if (bte_serial.indexOf("PWD") != -1)
      {
        int start = bte_serial.indexOf(":") + 1;
        String pwd = bte_serial.substring(start, bte_serial.length());
        savePWD(pwd);
        storedPWD = getStoredPWD(),
        ESP_BT.println("PWD stored : {" + storedPWD + "}");
      }

      if (bte_serial.indexOf("CRD") != -1)
      {
        ESP_BT.println("SSID : {" + storedSSID + "} , PWD : {" + storedPWD + "}");
      }

      if (bte_serial.indexOf("STATUS") != -1)
      {
        String status = "Connected";
        if (WiFi.status() != WL_CONNECTED)
        {
          status = "Not connected";
        }
        ESP_BT.println("SSID : " + storedSSID + " ,Status : " + status);
      }
    }
  }
  delay(20);
}

void loop()
{
  checkBLTEConfig();
}