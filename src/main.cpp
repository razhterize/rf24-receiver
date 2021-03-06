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
void write_flash(String flash_data, String name);
void read_flash(String name);
RF24 rv(2, 15);
uint8_t address[][6] = {"Node1", "Node2"};

bool status = false;
int addr;
unsigned long prevMillis;
float data;
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
  }
  if (addr != 0)
  {
    if (addr == 1)
    {
      sfd = data;
    }
    else if (addr == 2)
    {
      stemps = data;
    }
    else if (addr == 3)
    {
      sph = data;
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
  }
}

void split(float a)
{
  String s = String(a, 2);
  addr = s.substring(0, 1).toInt();
  data = s.substring(1).toFloat();
}

// Write Flash (String flash_data, String name)
// Nulis data ke EEPROM, data String (text), nama file Name
// Ex: write_flash(data, "sensor.txt"), data = pembacaan sensor (harus String), nama file "sensor.txt"

void write_flash(String flash_data, String name)
{
  File file = LittleFS.open(name, "a");
  if (file)
  {
    file.print(flash_data);
    file.print("\n");
  }
  file.close();
}

/*
  Read Flash (String name), baca flash pake while loop, selama masih ada baris selanjutnya. String name buat nama file
  hasilnya ntar String a, mau buat apa terserah

*/
void read_flash(String name)
{
  File file = LittleFS.open(name, "r");
  while (file.available())
  {
    String a = file.readStringUntil('\n');
    
  }
  file.close();
}

void server_check(){
  WiFiClient client;
  String write = "";

  if (!client.connect(host, port))
  {
    status = true;
    Serial.println("Connection failed");
    write = String(sph) + "," + String(stemps) + "," + String(stempd) + "," + String(stempt) + "," + String(sfd) + "," + String(swd) + "," + String(swf) + ",";
    write_flash(write, "log.txt");
    return;
  }

  if (status == true){
    read_flash("log.txt");
  }

  send_to_server();
}

void send_to_server(){
  WiFiClient client;

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
}
