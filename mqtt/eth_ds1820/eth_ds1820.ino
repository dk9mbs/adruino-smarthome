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
#include <PubSubClient.h>
#include <OneWire.h>

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
OneWire ds(ONE_WIRE_BUS);

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
  Ethernet.begin(mac, ip);
  #ifdef DEBUG
  printIPAddress();
  #endif
  delay(250);
}

void loop() {
  //delay(10000);

  if (!client.connected()) {
    reconnect();
  }

  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  while(ds.search(addr)) {  
    Serial.print("R=");
    for( i = 0; i < 8; i++) {
      Serial.print(addr[i], HEX);
      Serial.print(" ");
    }
  
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print("CRC is not valid!\n");
        return;
    }
    
    if ( addr[0] != 0x10) {
        Serial.print("Device is not a DS18S20 family device.\n");
        return;
    }
  
    // The DallasTemperature library can do all this work for you!
  
    ds.reset();
    ds.select(addr);
    ds.write(0x44,1);         // start conversion, with parasite power on at the end
    
    delay(1000);     // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.
    
    present = ds.reset();
    ds.select(addr);    
    ds.write(0xBE);         // Read Scratchpad
  
    Serial.print("P=");
    Serial.print(present,HEX);
    Serial.print(" ");

    char tmp[10];
    char tmpchar;
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print( OneWire::crc8( data, 8), HEX);
    Serial.println();

    int16_t temp = ((data[1] & 0x80) << 8) | (data[0] >> 1);
    Serial.println(temp, DEC);
    
    //float tempC = 0;
    char buf[15];
    itoa(temp,buf,10);
    //dtostrf(tempC,7,3,buf);
    phlClient.publish("test", buf );
    phlClient.loop();
  }
  delay(1000);
  phlClient.loop();

}


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


