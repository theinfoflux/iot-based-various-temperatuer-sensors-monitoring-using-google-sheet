#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
///////////////////////////////////These libraries are for interfacing DHT sensor
#include "DHT.h"
const int DHTPIN = D3;
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
//////////////////////////////////These libraries are for DS18B20 sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
/////////////////////////////////These libraries are for MAX6675 temperature sensor
#include <Wire.h>
#include <max6675.h>
#define CLK D5
#define CS D6
#define DO D7
MAX6675 thermocouple(CLK, CS, DO);
/////////////////////////////////////
#define ON_Board_LED 2  //--> Defining an On Board LED, used for indicators when the process of connecting to a wifi router

////////////////// SSD1306 oled Display instructions

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

///////////////////////Variables for various temperature sensor
float temp; //to store the temperature value
float hum; // to store the humidity value
float dstemp;
float maxtemp;

const char* ssid = "theinfoflux"; //--> Your wifi name or SSID.
const char* password = "12345678"; //--> Your wifi password.


//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

String Gsheetid = "AKfycbyKwCFWpEOB644N2tgFBo43ALoUGXB4qWM-ztUi8w-YmJsFFTNsZm8tEXJH14HtxJMO"; //--> spreadsheet script ID

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();          //Begins to receive Temperature and humidity values.  
  sensors.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  delay(500);
  
  WiFi.begin(ssid, password); //--> Connect to your WiFi router
  Serial.println("");
    
  pinMode(ON_Board_LED,OUTPUT); //--> On Board LED port Direction output
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off Led On Board

  //----------------------------------------Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //----------------------------------------Make the On Board Flashing LED on the process of connecting to the wifi router.
    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> Turn off the On Board LED when it is connected to the wifi router.
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
  
}

void loop() {
 /////DHT sensor reading
 temp = dht.readTemperature();
 hum = dht.readHumidity();

 /////DS18B20 sensor reading
 sensors.requestTemperatures(); 
  dstemp=sensors.getTempCByIndex(0);

 /////MAX6675 sensor reading
  maxtemp = thermocouple.readCelsius();
 Serial.print("temperature = ");
 Serial.println(temp);
 Serial.print("humidity = ");
 Serial.println(hum);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 0);
  display.print("DH:"+String(temp));
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("DS:"+String(dstemp)); 
  display.setTextSize(2);
  display.setCursor(10,45);
  display.print("MX:"+String(maxtemp)); 
  display.display();

  String Temp = "Temperature : " + String(temp) + " °C";
  String Hum = "Humidity : " + String(hum) + " °C";
  sendData(temp, hum,dstemp,maxtemp);
  delay(3000);
}
// Subroutine for sending data to Google Sheets
void sendData(float tem, int hum, float dstemp,float maxtemp) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Processing data and sending data
  String string_temperature =  String(tem); 
  String string_humidity =  String(hum, DEC); 
  String string_dstemperature =  String(dstemp);
  String string_maxtemperature =  String(maxtemp);
  String url = "/macros/s/" + Gsheetid + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity+ "&ds18b20=" + string_dstemperature+ "&max6675=" + string_maxtemperature;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //----------------------------------------Checking whether the data was sent successfully or not
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();

} 
