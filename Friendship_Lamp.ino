#include <Base64.h>
#include <global.h>
#include <sha1.h>
#include <Adafruit_NeoPixel.h>
//#include <WiFiNINA.h>
//#include <WiFiUdp.h>
#include "EasyWiFi.h"
#include <SPI.h>
#include <WebSockets2_Generic.h>
#include "defines.h"

using namespace websockets2_generic;

/*********** Global Settings  **********/
EasyWiFi MyEasyWiFi;
WebsocketsClient client;

char MyAPName[] = {"Friendship Lamp"};

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 12 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 100 // Time (in milliseconds) to pause between pixels

#define USE_SERIAL Serial
bool alreadyConnected = false;

void onEventsCallback(WebsocketsEvent event, String data) 
{
  (void) data;
  
  if (event == WebsocketsEvent::ConnectionOpened) 
  {
    Serial.println("Connnection Opened");
  } 
  else if (event == WebsocketsEvent::ConnectionClosed) 
  {
    Serial.println("Connnection Closed");
  } 
  else if (event == WebsocketsEvent::GotPing) 
  {
    Serial.println("Got a Ping!");
  } 
  else if (event == WebsocketsEvent::GotPong) 
  {
    Serial.println("Got a Pong!");
  }
}

void setup() {

  pixels.begin();

  pixels.clear();

  int t = 10;
  Serial.begin(115200);
  while (!Serial) {
    delay(1000);
    if ( (t--) == 0 ) break; // no serial, but continue program
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  MyEasyWiFi.apname(MyAPName);
  MyEasyWiFi.seed(0);

  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    } else {
      MyEasyWiFi.start();     // Start Wifi login
    }
  }

  client.onMessage([&](WebsocketsMessage message) 
  {
    Serial.print("Got Message: ");
    Serial.println(message.data());
  });

  // run callback when events are occuring
  client.onEvent(onEventsCallback);

  sendMessage();
}

void sendMessage(void)
{
// try to connect to Websockets server
  bool connected = client.connect(websockets_server_host, websockets_server_port, "/");
  
  if (connected) 
  {
    Serial.println("Connected!");

    String WS_msg = String("Hello to Server from ") + BOARD_NAME;
    client.send(WS_msg);
  } 
  else 
  {
    Serial.println("Not Connected!");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    printWiFiStatus();
    if (client.available()) {
      client.poll();
    }
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    }
    pixels.show();
  }
  else
  {
    Serial.println("* Not Connected, starting EasyWiFi");
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
        pixels.clear(); // Set all pixel colors to 'off'

        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));

        pixels.show();   // Send the updated pixel colors to the hardware.

        delay(DELAYVAL); // Pause before next pass through loop
      }
    }
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++) { // For each pixel...
      // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
      // Here we're using a moderately bright green color:
      pixels.setPixelColor(i, pixels.Color(155, 155, 0));
    }
    pixels.show();
  }
}

void printWiFiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("\nStatus: SSID: "); Serial.print(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP(); Serial.print(" - IPAddress: "); Serial.print(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI(); Serial.print("- Rssi: "); Serial.print(rssi); Serial.println("dBm");

}
