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
WiFiServer server(80);

String device_name = "ESP32-BT-Slave";

const byte wifi_pin = 21;
const byte feed_back_pin = 19;

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
  int maxTries = 20;
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
    digitalWrite(wifi_pin, LOW);
    delay(100);
    digitalWrite(wifi_pin, HIGH);
    delay(100);
    digitalWrite(wifi_pin, LOW);
    delay(100);
    digitalWrite(wifi_pin, HIGH);
    delay(100);
    digitalWrite(wifi_pin, LOW);
    delay(100);
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
    digitalWrite(wifi_pin, HIGH);
  }
  ESP_BT.print("LOCAL IP : " + WiFi.localIP().toString());
  server.begin();
  ESP_BT.println("Server Begin ...");
}

String getStoredSSID()
{
  String storedSSID;
  for (int i = 0; i < 32; ++i)
  {
    if (EEPROM.read(i) != 255 && EEPROM.read(i) != 0)
      storedSSID += char(EEPROM.read(i));
  }
  return storedSSID;
}

String getStoredPWD()
{
  String storedPWD;
  for (int i = 0; i < 96; ++i)
  {
    if (EEPROM.read(i + 32) != 255 && EEPROM.read(i + 32) != 0)
      storedPWD += char(EEPROM.read(i + 32));
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
  pinMode(feed_back_pin, OUTPUT);
  pinMode(wifi_pin, OUTPUT);
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
  Serial.println(WiFi.localIP());
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
    String bte_serial = ESP_BT.readStringUntil('\n');
    Serial.println(bte_serial);
    if (bte_serial.startsWith("CACHE"))
    {
      if (bte_serial.indexOf("CLEAR") != -1)
      {
        clearEEPROM();
        ESP_BT.print("Finish");
      }
    }
    if (bte_serial.startsWith("WIFI"))
    {
      if (bte_serial.indexOf("CONNECT") != -1)
      {
        connectToWifi(storedSSID, storedPWD);
      }

      if (bte_serial.indexOf("SSID") != -1)
      {
        int start = bte_serial.indexOf('"') + 1;
        int end = bte_serial.indexOf('"', start + 1);
        String ssid = bte_serial.substring(start, end);
        saveSSID(ssid);
        storedSSID = getStoredSSID();
        ESP_BT.print("SSID stored : {" + storedSSID + "}");
      }

      if (bte_serial.indexOf("PWD") != -1)
      {
        int start = bte_serial.indexOf('"') + 1;
        int end = bte_serial.indexOf('"', start + 1);
        String pwd = bte_serial.substring(start, end);
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
        ESP_BT.print("LOCAL IP : " + WiFi.localIP().toString());
      }
    }
  }
  delay(20);
}
bool motor = false;

void loop()
{
  checkBLTEConfig();
  WiFiClient client = server.available();
  if (client)
  { // if you get a client,
    String req = client.readStringUntil('\r');
    Serial.println(req);
    // client.flush();
  }
}