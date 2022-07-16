#include <Arduino.h>
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
// #include <RTClib.h>
#include <LittleFS.h>
///////////////////////////////////////////////////
#include <ESP8266WiFi.h>
// Network ID
const char *ssid = "Kos putra Fajar 2";
const char *password = "berkahjaya";
const char *host = "termosys.online";
const int port = 80;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
///////////////////////////////////////////////////
void split(float a);
void write_flash(String flash_data);
void read_flash();
// void get_time();
RF24 rv(2, 15);
// RTC_DS3231 rtc;
uint8_t address[][6] = {"Node1", "Node2"};

float data;
int addr;
unsigned long prevMillis;

// DHT get temp dan humid
float sph;
float stemps;
float stempd;
float stempt;
float sfd;
float swd = random(60, 75);
float swf = random(60, 90);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(50);
  }
  ///////////////////////////////////////////////////
  // Networking
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ///////////////////////////////////////////////////

  // DateTime test = rtc.now();
  // char time_now[30];
  // sprintf(time_now, "%i-%i-%i %i:%i:%i", test.year(), test.month(), test.day(), test.hour(), test.minute(), test.second());
  while (!rv.begin())
  {
    Serial.println("RF24 Init failed!");
    delay(200);
  }
  while (!LittleFS.begin())
  {
    Serial.println("FS Init Failed!");
    delay(500);
  }
  // while (!rtc.begin()){
  //   Serial.println("RTC Init Failed!");
  //   delay(500);
  // }
  Serial.println("Test Run!");
  // Serial.println(time_now);
  rv.setPALevel(RF24_PA_LOW);
  rv.openReadingPipe(1, address[1]);
  rv.startListening();
}

void loop()
{
  uint8_t pipe;
  if (rv.available(&pipe))
  {
    String write = "";
    rv.read(&data, sizeof(data));
    split(data);
    Serial.print(F("Received "));
    Serial.print(sizeof(data)); // print the size of the payload
    Serial.print(F(" bytes on pipe "));
    Serial.print(pipe);
    Serial.print("\nData: ");
    Serial.println(data);
    Serial.print("Address: ");
    Serial.println(addr);
    Serial.print("\n");
    write = String(data, 2);
    write_flash(write);
  }
  if (addr != 0)
  {
    if (addr == 1)
    {
      sfd = data;
    }
    else if (addr == 2)
    {
      sph = data;
    }
    else if (addr == 3)
    {
      stemps = data;
    }
    else if (addr == 4)
    {
      stempd = data;
    }
    else if (addr == 5)
    {
      stempt = data;
    }
  }
  if (millis() - prevMillis >= 10000)
  {
    prevMillis = millis();
    ////////////////////////////////////////////////////////////////////
    WiFiClient client;

    if (!client.connect(host, port))
    {
      Serial.println("Connection failed");
      return;
    }

    // /nodemcuphp/index.php?mode=save&vph=20&vtemps=20&vtempd=20&vtempt=20&vfd=20&vwd=20&vwf=20
    String apiUrl = "/api/index.php?";
    apiUrl += "mode=save";
    apiUrl += "&vph=" + String(sph);
    apiUrl += "&vtemps=" + String(stemps);
    apiUrl += "&vtempd=" + String(stempd);
    apiUrl += "&vtempt=" + String(stempt);
    apiUrl += "&vfd=" + String(sfd);
    apiUrl += "&vwd=" + String(swd);
    apiUrl += "&vwf=" + String(swf);

    // Set header Request
    client.print(String("GET ") + apiUrl + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    // Pastikan tidak berlarut-larut
    unsigned long timeout = millis();
    while (client.available() == 0)
    {
      if (millis() - timeout > 3000)
      {
        Serial.println(">>> Client Timeout !");
        Serial.println(">>> Operation failed !");
        client.stop();
        return;
      }
    }

    // Baca hasil balasan dari PHP
    // while (client.available())
    // {
    //   String line = client.readStringUntil('\r');
    //   Serial.println(line);
    //   Serial.println("ph kolam 1");
    //   Serial.println(sph);
    //   Serial.println("Suhu kolam 1 : ");
    //   Serial.println(stemps);

    //   Serial.println("Suhu kolam 2 : ");
    //   Serial.println(stempd);
    //   Serial.println("Suhu kolam 3 : ");
    //   Serial.println(stempt);
    //   Serial.println("Jarak Pakan : ");
    //   Serial.println(sfd);
    //   Serial.println("Ketinggian Air Kolam 1 : ");
    //   Serial.println(swd);
    //   Serial.println("Water Flow Kolam 1 : ");
    //   Serial.println(swf);
    //   Serial.println("==========================");
    // }
    ////////////////////////////////////////////////////////////////////
    // read_flash();
    // get_time();
  }
}

void split(float a)
{
  String s = String(a, 2);
  addr = s.substring(0, 1).toInt();
  data = s.substring(1).toFloat();
}

void write_flash(String flash_data)
{
  File file = LittleFS.open("/sensor_log.txt", "a");
  if (file)
  {
    file.print(flash_data);
    file.print("\n");
  }
  file.close();
}

void read_flash()
{
  File file = LittleFS.open("/sensor_log.txt", "r");
  while (file.available())
  {
    Serial.println(file.readString());
  }
  file.close();
}

// void get_time(){
//   DateTime current = rtc.now();
//   Serial.println(current.hour());
//   Serial.println(current.minute());
// }