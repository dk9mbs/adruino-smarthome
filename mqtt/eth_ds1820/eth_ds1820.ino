/*
  do not use the dhcp extension - because the binary file becomes to big for a nano!
  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12.01.2017
  by Markus Buehler, DK9MBS (http://dk9mbs.de)

*/

//#define DEBUG
//#define SUBSCRIBE
#define ENC28J60
#define DS1820
#define ONE_WIRE_BUS 3

#ifdef ENC28J60
  #include <UIPEthernet.h>
  #warning use ENC28J60 eth hardware!!!
#else
  #include <Ethernet.h>
  #warning do not use ENC28J60 eth hardware!!!
#endif

#include <SPI.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Enter a MAC address for your controller below.
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0xB9};
byte ip[] = {192, 168, 2, 184};
byte server[] = {82, 165, 61, 184};
char subTopic[] = "test1";


void callback(char* topic, byte* payload, unsigned int length) {
  #ifdef DEBUG
  Serial.print(topic);
  Serial.print(" -> ");
  String s=String((char*)payload);
  Serial.println(s);
  #endif
}

EthernetClient client;
PubSubClient phlClient(server, 1883, callback, client);

#ifdef DS1820
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#warning DALLAS!!!!
#endif

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    #ifdef DEBUG
    Serial.print("Attempting MQTT connection...");
    #endif
    
    if (phlClient.connect("arduinoClient")) {
      #ifdef DEBUG
      Serial.println("connected");
      #endif
      phlClient.publish("outTopic", "hello world");
      phlClient.subscribe("inTopic");
    } else {
      #ifdef DEBUG
      Serial.print("failed, rc=");
      Serial.print(phlClient.state());
      Serial.println(" try again in 5 seconds");
      #endif
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  #ifdef DS1820
  OneWire oneWire(ONE_WIRE_BUS);
  #endif
  delay (500);
  Ethernet.begin(mac, ip);
  #ifdef DEBUG
  printIPAddress();
  #endif
  delay(250);
  sensors.begin();
  delay(250);
}

void loop() {
  #ifdef DEBUG
  Serial.print("sending...");
  #endif

  if (!client.connected()) {
    reconnect();
  }

  sensors.requestTemperatures();
  delay(1000);
  char buf[7];
  #ifdef DEBUG
  Serial.println(sensors.getTempCByIndex(0));
  #endif
  dtostrf(sensors.getTempCByIndex(0),3,3,buf);
  phlClient.publish("test", buf);
  //phlClient.publish("test", "19.6");

  #ifdef DEBUG
  Serial.println("sended!!!");
  #endif

  delay(4000);

  phlClient.loop();

}

#ifdef DEBUG
void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
#endif

