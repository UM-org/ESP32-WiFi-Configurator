#include "BluetoothSerial.h"
#include <EEPROM.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Default WIFI
const char *ssid = "your ssid";
const char *password = "your password";
String storedSSID;
String storedPWD;

String storedServer;
const char *mqttServer = "192.168.137.1";
const char *topic = "example";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";

WiFiClient espClient;
BluetoothSerial ESP_BT;
WiFiServer server(80);
PubSubClient client(espClient);

String device_name = "ESP32-BT-devkit-v4";

const byte wifi_pin = 21;
const byte feed_back_pin = 19;
const byte error_pin = 18;

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

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
    digitalWrite(error_pin, HIGH);
  }
  else
  {
    Serial.println("Connected to WiFi..");
    ESP_BT.println("Connected to WiFi..");
    digitalWrite(wifi_pin, HIGH);
    digitalWrite(error_pin, LOW);
    connectMQTT();
  }
  ESP_BT.print("LOCAL IP : " + WiFi.localIP().toString());
  server.begin();
  ESP_BT.println("Server Begin ...");
}

void connectMQTT()
{
  int maxTries = 20;
  int tries = 0;
  Serial.println(storedServer);
  client.setServer(storedServer.c_str(), 1883);
  client.setCallback(callback);
  while (!client.connected() && tries < maxTries)
  {
    tries++;
    String client_id = "esp32-ttt-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
      digitalWrite(wifi_pin, HIGH);
      digitalWrite(error_pin, LOW);
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      digitalWrite(error_pin, HIGH);
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
      digitalWrite(wifi_pin, HIGH);
      delay(100);
      digitalWrite(wifi_pin, LOW);
      delay(100);
      digitalWrite(wifi_pin, HIGH);
      delay(100);
      digitalWrite(wifi_pin, LOW);
      delay(100);
    }
  }
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
  for (int i = 0; i < 64; ++i)
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

void saveServer(String ip)
{
  for (int i = 0; i < ip.length(); ++i)
  {
    EEPROM.write(i + 96, ip[i]);
  }
  EEPROM.commit();
}

String getStoredServer()
{
  String storedServer;
  for (int i = 0; i < 128; ++i)
  {
    if (EEPROM.read(i + 96) != 255 && EEPROM.read(i + 96) != 0)
      storedServer += char(EEPROM.read(i + 96));
  }
  return storedServer;
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
  for (int i = 0; i < 128; ++i)
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
  pinMode(error_pin, OUTPUT);
  // clearEEPROM(); // uncommit whene dev

  storedSSID = getStoredSSID();
  if (storedSSID == NULL)
  {
    storedSSID = ssid;
  }

  storedServer = getStoredServer();
  if (storedServer == NULL)
  {
    storedServer = mqttServer;
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
    digitalWrite(feed_back_pin, HIGH);
    delay(500);
    digitalWrite(feed_back_pin, LOW);

    String bte_serial = ESP_BT.readStringUntil('\n');
    Serial.println(bte_serial);
    if (bte_serial.startsWith("SERVER"))
    {
      if (bte_serial.indexOf("IP") != -1)
      {
        int start = bte_serial.indexOf('"') + 1;
        int end = bte_serial.indexOf('"', start + 1);
        String serverIP = bte_serial.substring(start, end);
        saveServer(serverIP);
        storedServer = getStoredServer();
        ESP_BT.print("SERVER IP : {" + storedServer + "}");
      }
      if (bte_serial.indexOf("CONNECT") != -1)
      {
        if (WiFi.status() == WL_CONNECTED)
        {
          connectMQTT();
        }
        else
        {
          digitalWrite(error_pin, HIGH);
          delay(200);
          digitalWrite(error_pin, LOW);
          delay(200);
          digitalWrite(error_pin, HIGH);
          delay(200);
          digitalWrite(error_pin, LOW);
          ESP_BT.print("CANT CONNECT TO MQTT SERVER WHILE WIFI IS NOT CONNECTED");
        }
      }

      if (bte_serial.startsWith("CACHE"))
      {
        if (bte_serial.indexOf("CLEAR") != -1)
        {
          clearEEPROM();
          ESP_BT.print("Finish");
        }

        if (bte_serial.indexOf("GET") != -1)
        {
          ESP_BT.println("SSID : {" + storedSSID + "} , PWD : {" + storedPWD + "} , MQTT SERVER : {" + storedServer + "}");
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
      delay(500);
      digitalWrite(feed_back_pin, HIGH);
      delay(500);
      digitalWrite(feed_back_pin, LOW);
    }
    delay(20);
  }

  void loop()
  {
    checkBLTEConfig();
    client.publish(topic, "hello world");
    delay(2000);
  }