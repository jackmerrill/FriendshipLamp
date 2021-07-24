#include "defines.h"

#include <Adafruit_NeoPixel.h>
#include "EasyWiFi.h"
#include <SPI.h>
#include <WebSockets2_Generic.h>

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

const int buttonPin = 11;

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

  pinMode(buttonPin, INPUT);

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
    Serial.println("* Not Connected, starting EasyWiFi");
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.clear();
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));

        pixels.show();   // Send the updated pixel colors to the hardware.

        delay(DELAYVAL); // Pause before next pass through loop
      }
    }
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(155, 155, 0));
    }
    pixels.show();
    if (WiFi.status() == WL_CONNECTED) {
      break;
    } else {
      MyEasyWiFi.start();     // Start Wifi login
    }
  }

  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
  }
  pixels.show();

  delay(1500);

  client.onMessage([&](WebsocketsMessage message)
  {
    pixels.clear();
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, message.data().toInt());
    }
    pixels.show();
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

    client.send("getColor");
  }
  else
  {
    Serial.println("Not Connected!");
  }
}

int buttonPushCounter = 0;
int buttonState = 0;
int lastButtonState = 0;

int colors[] = { 16711680, 16746240, 16765696, 14614282, 10616586, 720793, 716799, 1342965, 5769983, 12454655 };

void updateColor(int color) {
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, colors[color]);
  }
  pixels.show();

  Serial.println("color");

  client.send("setColor " + colors[color]);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED)
  {
    if (client.available()) {
      Serial.println("available");
      buttonState = digitalRead(buttonPin);

      // compare the buttonState to its previous state
      if (buttonState != lastButtonState) {
        // if the state has changed, increment the counter
        if (buttonState == HIGH) {
          // if the current state is HIGH then the button went from off to on:
          buttonPushCounter++;
          Serial.println("on");
          Serial.print("number of button pushes: ");
          Serial.println(buttonPushCounter);
          updateColor(buttonPushCounter - 1);
        } else {
          // if the current state is LOW then the button went from on to off:
          Serial.println("off");
        }
        // Delay a little bit to avoid bouncing
        delay(50);
      }
      // save the current state as the last state, for next time through the loop
      lastButtonState = buttonState;
      if (buttonPushCounter == 10) {
        buttonPushCounter = 0;
      }
//      client.poll();
    }
  }
  else
  {
    Serial.println("disconnected");
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
