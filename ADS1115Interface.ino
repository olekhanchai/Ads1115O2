#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ADS1115.h>

#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);

#define STASSID "true_home2G_a64"
#define STAPSK  "178fda64"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
ADS1115 adc0(ADS1115_DEFAULT_ADDRESS);
static double o2_factor = 0.0;
double o2_opset = 0.0;
double o2_val = 0.0;
double o2_mv = 0.0;

void handleRoot() {
  server.send(200, "text/plain", "O:" + String(o2_val) + " " + "V:" + String(o2_mv));
}

void handleSetup() {
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "gain") {
      adc0.setGain(server.arg(i).toDouble());
    }
    if (server.argName(i) == "factor") {
      o2_factor = server.arg(i).toDouble();
    }
  }
  server.send(200, "text/plain", "Setup completed.");  
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.println("Not connected.");
  }
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  adc0.initialize(); // initialize ADS1115 16 bit A/D chip
  
  if(!adc0.testConnection()) display.println("ADS1115 connection failed");

  
  // We're going to do continuous sampling
  adc0.setMode(ADS1115_MODE_CONTINUOUS); 
  
  // Set the gain (PGA) +/- 0.512 v
  adc0.setGain(ADS1115_PGA_0P256);   

  server.on("/", handleRoot);
  server.on("/setup", handleSetup);

  server.onNotFound(handleNotFound);

  server.begin();
}
void loop() {
   
  server.handleClient();
  
  int sensorOneCounts=adc0.getConversionP0N1();  // counts up to 16-bits  
  
  display.clearDisplay(); 
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0,0); 
  display.print("IP : ");
  display.println(WiFi.localIP());

  display.setCursor(0,10); 
  display.print("Oxygen : ");
  o2_val = (sensorOneCounts*adc0.getMvPerCount()*o2_factor)+o2_opset;
  display.print(o2_val);
  display.println(" %"); 
  display.setCursor(0,20); 
  display.print("Vout : ");
  o2_mv = sensorOneCounts*adc0.getMvPerCount();
  display.print(o2_mv);
  display.println(" mV"); 
  display.display();    

  delay(1000);
}
