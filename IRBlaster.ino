#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#ifndef APSSID
#define APSSID "IPSTransform"
#define APPSK  "PagaTuInternetMotherFucker123"
#endif

const char* ssid     = APSSID;
const char* password = APPSK;
const char* mqtt_server = "10.255.254.200";

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationGotIPHandler;
WiFiEventHandler stationDisconnectedHandler;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int ledState = false;

void mqtt_callback(char* topic, byte* payload, unsigned int length);
void reconnect();

const uint16_t kIrLed = 4;
IRsend irsend(kIrLed);

// Códigos de DISH
uint16_t dishPower[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0017, 0x0101, 
  0x0017, 0x00ff, 0x0017, 0x00fe, 0x0017, 0x009d, 0x0017, 0x0101, 
  0x0016, 0x0100, 0x0017, 0x0100, 0x0016, 0x0100, 0x0016, 0x0100, 
  0x0016, 0x00fe
};

uint16_t dish1[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0017, 0x0101, 
  0x0017, 0x00fe, 0x0017, 0x009d, 0x0017, 0x00ff, 0x0017, 0x0101, 
  0x0016, 0x0100, 0x0016, 0x0100, 0x0016, 0x0100, 0x0016, 0x0100, 
  0x0016, 0x00fe
};

uint16_t dish2[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0017, 0x0101, 
  0x0017, 0x00fe, 0x0017, 0x009d, 0x0017, 0x00fe, 0x0017, 0x009f, 
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 
  0x0017, 0x00fd
};

uint16_t dish3[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fd,
  0x0017, 0x00fa, 0x0017, 0x009a, 0x0017, 0x009b, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dish4[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fd,
  0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc,
  0x0016, 0x00fa
};

uint16_t dish5[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0017, 0x00fa, 0x0017, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dish6[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009b, 0x0017, 0x00fa, 0x0017, 0x009b, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dish7[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009a, 0x0017, 0x009b, 0x0017, 0x00fb, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dish8[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009a, 0x0017, 0x009b, 0x0017, 0x00fb, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dish9[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009a, 0x0017, 0x009b, 0x0016, 0x009c, 0x0016, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dish0[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0016, 0x009f,
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00fe, 0x0017, 0x009f,
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff,
  0x0017, 0x00fd
};

uint16_t dishSelect[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fd,
  0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc,
  0x0016, 0x00fa
};

uint16_t dishVolPlus[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishVolLess[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0016, 0x009c, 0x0016, 0x00fb, 0x0017, 0x009c,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishMute[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fa, 0x0017, 0x009b, 0x0017, 0x00fb, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishBrowse[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fa, 0x0017, 0x009b, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishGuide[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fa, 0x0017, 0x009b, 0x0016, 0x009b, 0x0017, 0x009c,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishJump[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0017, 0x009f,
  0x0017, 0x00fe, 0x0017, 0x009d, 0x0017, 0x00ff, 0x0017, 0x0101,
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff,
  0x0017, 0x00fd
};

uint16_t dishMenu[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009b, 0x0017, 0x00fa, 0x0017, 0x009b, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishUp[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0016, 0x009c, 0x0016, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishDown[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0016, 0x009b, 0x0017, 0x009b, 0x0017, 0x00fc,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishLeft[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0016, 0x009c, 0x0016, 0x00fc, 0x0016, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishRight[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dishTVVCR[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0016, 0x009b, 0x0017, 0x009a, 0x0017, 0x009d,
  0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc, 0x0016, 0x00fc,
  0x0016, 0x00fa
};

uint16_t dishView[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fa, 0x0017, 0x009b, 0x0016, 0x009c, 0x0016, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishInfo[26] = {
  0x0000, 0x0047, 0x0000, 0x000b, 0x0016, 0x0222, 0x0017, 0x0101,
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x0100,
  0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff, 0x0017, 0x00ff,
  0x0017, 0x00fe
};

uint16_t dishCancel[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fa, 0x0017, 0x009b, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dishGuidePageUp[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fd,
  0x0017, 0x00fb, 0x0017, 0x00fa, 0x0017, 0x009a, 0x0017, 0x009d,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00f9
};

uint16_t dishGuidePageDown[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0017, 0x00fc,
  0x0017, 0x009a, 0x0017, 0x009b, 0x0016, 0x009b, 0x0017, 0x009c,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

uint16_t dishSysInfo[26] = {
  0x0000, 0x0048, 0x0000, 0x000b, 0x0016, 0x021a, 0x0016, 0x009c,
  0x0017, 0x009b, 0x0017, 0x00fb, 0x0016, 0x009b, 0x0017, 0x009c,
  0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb, 0x0017, 0x00fb,
  0x0017, 0x00fa
};

// Códigos de Samsung
uint64_t samsungPower = 0xE0E040BF;
uint64_t samsungSource = 0xE0E0807F;
uint64_t samsungFutbol = 0xE0E01DE2;
uint64_t samsung1 = 0xE0E020DF;
uint64_t samsung2 = 0xE0E0A05F;
uint64_t samsung3 = 0xE0E0609F;
uint64_t samsung4 = 0xE0E010EF;
uint64_t samsung5 = 0xE0E0906F;
uint64_t samsung6 = 0xE0E050AF;
uint64_t samsung7 = 0xE0E030CF;
uint64_t samsung8 = 0xE0E0B04F;
uint64_t samsung9 = 0xE0E0708F;
uint64_t samsung0 = 0xE0E08877;
uint64_t samsungHyphen = 0xE0E0C43B;
uint64_t samsungPreCh = 0xE0E0C837;
uint64_t samsungVolPlus = 0xE0E0E01F;
uint64_t samsungVolLess = 0xE0E0D02F;
uint64_t samsungMute = 0xE0E0F00F;
uint64_t samsungChList = 0xE0E0D629;
uint64_t samsungChPlus = 0xE0E048B7;
uint64_t samsungChLess = 0xE0E008F7;
uint64_t samsungMenu = 0xE0E058A7;
uint64_t samsungSmart = 0xE0E09E61;
uint64_t samsungGuide = 0xE0E0F20D;
uint64_t samsungTools = 0xE0E0D22D;
uint64_t samsungInfo = 0xE0E0F807;
uint64_t samsungReturn = 0xE0E01AE5;
uint64_t samsungExit = 0xE0E0B44B;
uint64_t samsungSelect = 0xE0E016E9;
uint64_t samsungUp = 0xE0E006F9;
uint64_t samsungDown = 0xE0E08679;
uint64_t samsungLeft = 0xE0E0A659;
uint64_t samsungRight = 0xE0E046B9;
uint64_t samsungA = 0xE0E036C9;
uint64_t samsungB = 0xE0E028D7;
uint64_t samsungC = 0xE0E0A857;
uint64_t samsungD = 0xE0E06897;
uint64_t samsungSleep = 0xE0E0C03F;
uint64_t samsungSearch = 0xE0E0CE31;
uint64_t samsungPSize = 0xE0E07C83;
uint64_t samsungEManual = 0xE0E0FC03;
uint64_t samsungPIP = 0xE0E004FB;
uint64_t samsungCC = 0xE0E0A45B;
uint64_t samsungRewind = 0xE0E0A25D;
uint64_t samsungForward = 0xE0E012ED;
uint64_t samsungPause = 0xE0E052AD;
uint64_t samsungPlay = 0xE0E0E21D;
uint64_t samsungRecord = 0xE0E0926D;
uint64_t samsungStop = 0xE0E0629D;

uint64_t atvioPower = 0xFDC03F;
uint64_t atvioTV = 0xFDE01F;
uint64_t atvioSource = 0xFD7887;
uint64_t atvio1 = 0xFD00FF;
uint64_t atvio2 = 0xFD807F;
uint64_t atvio3 = 0xFD40BF;
uint64_t atvio4 = 0xFD20DF;
uint64_t atvio5 = 0xFDA05F;
uint64_t atvio6 = 0xFD609F;
uint64_t atvio7 = 0xFD10EF;
uint64_t atvio8 = 0xFD906F;
uint64_t atvio9 = 0xFD50AF;
uint64_t atvio0 = 0xFDB04F;
uint64_t atvioDot = 0xFD30CF;
uint64_t atvioYoutube = 0xFDBA45;
uint64_t atvioVolPlus = 0xFD6897;
uint64_t atvioVolLess = 0xFD58A7;
uint64_t atvioChPlus = 0xFD28D7;
uint64_t atvioChLess = 0xFD18E7;
uint64_t atvioMute = 0xFDA857;
uint64_t atvioHome = 0xFDE21D;
uint64_t atvioNetflix = 0xFDC23D;
uint64_t atvioBrowser = 0xFD3AC5;
uint64_t atvioSettings = 0xFD38C7;
uint64_t atvioUp = 0xFDB847;
uint64_t atvioDown = 0xFDA25D;
uint64_t atvioLeft = 0xFD02FD;
uint64_t atvioRight = 0xFD42BD;
uint64_t atvioOk = 0xFD827D;
uint64_t atvioBack = 0xFD708F;
uint64_t atvioExit = 0xFD22DD;
uint64_t atvioInfo = 0xFD629D;
uint64_t atvioUSB = 0xFD6A95;
uint64_t atvioMenu = 0xFDE817;
uint64_t atvioSleep = 0xFD9867;
uint64_t atvioGuide = 0xFD9A65;
uint64_t atvioFav = 0xFD1AE5;
uint64_t atvioChList = 0xFDEA15;
uint64_t atvioRed = 0xFD12ED;
uint64_t atvioGreen = 0xFD926D;
uint64_t atvioYellow = 0xFD52AD;
uint64_t atvioBlue = 0xFD32CD;
uint64_t atvioPicture = 0xFD08F7;
uint64_t atvioSound = 0xFD8877;
uint64_t atvioCC = 0xFDAA55;
uint64_t atvioPrevious = 0xFD0AF5;
uint64_t atvioStop = 0xFDCA35;
uint64_t atvioPlayPause = 0xFD4AB5;
uint64_t atvioNext = 0xFD8A75;
uint64_t atvioRewind = 0xFDB24D;
uint64_t atvioForward = 0xFD728D;
uint64_t atvioMTS = 0xFDC837;


void setup() {
  // Configuración de puerto serial
  Serial.begin(115200);

  // Configuración de led para visualización
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println();
  Serial.print("Led builtin ");
  Serial.println(LED_BUILTIN);
  
  // Configuración de tarjeta como estación y conexión a la red
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Registro de los handlers para  el manejo de estados de WiFi
  stationConnectedHandler = WiFi.onStationModeConnected(&onStationConnected);
  stationGotIPHandler = WiFi.onStationModeGotIP(&onStationGotIP);
  stationDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationDisconnected);

  // Handlers para OTA
  ArduinoOTA.setHostname("IRBlasterSala");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  // Configuración del servidor MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqtt_callback);

  // Configuración de la libreria para enviar señales IR
  irsend.begin();
}

void loop() {
  ArduinoOTA.handle();

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "Connected to Main IRBlaster #%ld", value);
    // Serial.print("Publish message: ");
    // Serial.println(msg);
    client.publish("rc/devices", msg);
    
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }

  
}

void onStationConnected(const WiFiEventStationModeConnected& evt) {
  Serial.print("Station connected: ");
  Serial.println(ssid);
}

void onStationGotIP(const WiFiEventStationModeGotIP& evt) {
  Serial.print("Station got ip: ");
  Serial.println(WiFi.localIP());
}

void onStationDisconnected(const WiFiEventStationModeDisconnected& evt) {
  Serial.print("Station disconnected: ");
  Serial.println(ssid);

  WiFi.reconnect();
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String payload_string = String((char *) payload).substring(0, length);
  Serial.print(String(topic));
  Serial.print(" ");
  Serial.println(payload_string);

  if (String(topic).equals("rc/samsung")) {
    if (payload_string.equals("power")) {
      irsend.sendSAMSUNG(samsungPower);
    } else if (payload_string.equals("source")) {
      irsend.sendSAMSUNG(samsungSource);
    } else if (payload_string.equals("futbol")) {
      irsend.sendSAMSUNG(samsungFutbol);
    } else if (payload_string.equals("1")) {
      irsend.sendSAMSUNG(samsung1);
    } else if (payload_string.equals("2")) {
      irsend.sendSAMSUNG(samsung2);
    } else if (payload_string.equals("3")) {
      irsend.sendSAMSUNG(samsung3);
    } else if (payload_string.equals("4")) {
      irsend.sendSAMSUNG(samsung4);
    } else if (payload_string.equals("5")) {
      irsend.sendSAMSUNG(samsung5);
    } else if (payload_string.equals("6")) {
      irsend.sendSAMSUNG(samsung6);
    } else if (payload_string.equals("7")) {
      irsend.sendSAMSUNG(samsung7);
    } else if (payload_string.equals("8")) {
      irsend.sendSAMSUNG(samsung8);
    } else if (payload_string.equals("9")) {
      irsend.sendSAMSUNG(samsung9);
    } else if (payload_string.equals("0")) {
      irsend.sendSAMSUNG(samsung0);
    } else if (payload_string.equals("-")) {
      irsend.sendSAMSUNG(samsungHyphen);
    } else if (payload_string.equals("prech")) {
      irsend.sendSAMSUNG(samsungPreCh);
    } else if (payload_string.equals("vol+")) {
      irsend.sendSAMSUNG(samsungVolPlus);
    } else if (payload_string.equals("vol-")) {
      irsend.sendSAMSUNG(samsungVolLess);
    } else if (payload_string.equals("mute")) {
      irsend.sendSAMSUNG(samsungMute);
    } else if (payload_string.equals("chlist")) {
      irsend.sendSAMSUNG(samsungChList);
    } else if (payload_string.equals("ch+")) {
      irsend.sendSAMSUNG(samsungChPlus);
    } else if (payload_string.equals("ch-")) {
      irsend.sendSAMSUNG(samsungChLess);
    } else if (payload_string.equals("menu")) {
      irsend.sendSAMSUNG(samsungMenu);
    } else if (payload_string.equals("smart")) {
      irsend.sendSAMSUNG(samsungSmart);
    } else if (payload_string.equals("guide")) {
      irsend.sendSAMSUNG(samsungGuide);
    } else if (payload_string.equals("tools")) {
      irsend.sendSAMSUNG(samsungTools);
    } else if (payload_string.equals("info")) {
      irsend.sendSAMSUNG(samsungInfo);
    } else if (payload_string.equals("return")) {
      irsend.sendSAMSUNG(samsungReturn);
    } else if (payload_string.equals("exit")) {
      irsend.sendSAMSUNG(samsungExit);
    } else if (payload_string.equals("select")) {
      irsend.sendSAMSUNG(samsungSelect);
    } else if (payload_string.equals("up")) {
      irsend.sendSAMSUNG(samsungUp);
    } else if (payload_string.equals("down")) {
      irsend.sendSAMSUNG(samsungDown);
    } else if (payload_string.equals("left")) {
      irsend.sendSAMSUNG(samsungLeft);
    } else if (payload_string.equals("right")) {
      irsend.sendSAMSUNG(samsungRight);
    } else if (payload_string.equals("a")) {
      irsend.sendSAMSUNG(samsungA);
    } else if (payload_string.equals("b")) {
      irsend.sendSAMSUNG(samsungB);
    } else if (payload_string.equals("c")) {
      irsend.sendSAMSUNG(samsungC);
    } else if (payload_string.equals("d")) {
      irsend.sendSAMSUNG(samsungD);
    } else if (payload_string.equals("sleep")) {
      irsend.sendSAMSUNG(samsungSleep);
    } else if (payload_string.equals("search")) {
      irsend.sendSAMSUNG(samsungSearch);
    } else if (payload_string.equals("psize")) {
      irsend.sendSAMSUNG(samsungPSize);
    } else if (payload_string.equals("emanual")) {
      irsend.sendSAMSUNG(samsungEManual);
    } else if (payload_string.equals("pip")) {
      irsend.sendSAMSUNG(samsungPIP);
    } else if (payload_string.equals("cc")) {
      irsend.sendSAMSUNG(samsungCC);
    } else if (payload_string.equals("rewind")) {
      irsend.sendSAMSUNG(samsungRewind);
    } else if (payload_string.equals("forward")) {
      irsend.sendSAMSUNG(samsungForward);
    } else if (payload_string.equals("pause")) {
      irsend.sendSAMSUNG(samsungPause);
    } else if (payload_string.equals("play")) {
      irsend.sendSAMSUNG(samsungPlay);
    } else if (payload_string.equals("record")) {
      irsend.sendSAMSUNG(samsungRecord);
    } else if (payload_string.equals("stop")) {
      irsend.sendSAMSUNG(samsungStop);
    }
  } else if (String(topic).equals("rc/dish")) {
    if (payload_string.equals("power")) {
      irsend.sendPronto(dishPower, 26, 4);
    } else if (payload_string.equals("1")) {
      irsend.sendPronto(dish1, 26, 4);
    } else if (payload_string.equals("2")) {
      irsend.sendPronto(dish2, 26, 4);
    } else if (payload_string.equals("3")) {
      irsend.sendPronto(dish3, 26, 4);
    } else if (payload_string.equals("4")) {
      irsend.sendPronto(dish4, 26, 4);
    } else if (payload_string.equals("5")) {
      irsend.sendPronto(dish5, 26, 4);
    } else if (payload_string.equals("6")) {
      irsend.sendPronto(dish6, 26, 4);
    } else if (payload_string.equals("7")) {
      irsend.sendPronto(dish7, 26, 4);
    } else if (payload_string.equals("8")) {
      irsend.sendPronto(dish8, 26, 4);
    } else if (payload_string.equals("9")) {
      irsend.sendPronto(dish9, 26, 4);
    } else if (payload_string.equals("0")) {
      irsend.sendPronto(dish0, 26, 4);
    } else if (payload_string.equals("select")) {
      irsend.sendPronto(dishSelect, 26, 4);
    } else if (payload_string.equals("vol+")) {
      irsend.sendPronto(dishVolPlus, 26, 4);
    } else if (payload_string.equals("vol-")) {
      irsend.sendPronto(dishVolLess, 26, 4);
    } else if (payload_string.equals("mute")) {
      irsend.sendPronto(dishMute, 26, 4);
    } else if (payload_string.equals("browse")) {
      irsend.sendPronto(dishBrowse, 26, 4);
    } else if (payload_string.equals("guide")) {
      irsend.sendPronto(dishGuide, 26, 4);
    } else if (payload_string.equals("jump")) {
      irsend.sendPronto(dishJump, 26, 4);
    } else if (payload_string.equals("menu")) {
      irsend.sendPronto(dishMenu, 26, 4);
    } else if (payload_string.equals("up")) {
      irsend.sendPronto(dishUp, 26, 4);
    } else if (payload_string.equals("down")) {
      irsend.sendPronto(dishDown, 26, 4);
    } else if (payload_string.equals("left")) {
      irsend.sendPronto(dishLeft, 26, 4);
    } else if (payload_string.equals("right")) {
      irsend.sendPronto(dishRight, 26, 4);
    } else if (payload_string.equals("tvvcr")) {
      irsend.sendPronto(dishTVVCR, 26, 4);
    } else if (payload_string.equals("view")) {
      irsend.sendPronto(dishView, 26, 4);
    } else if (payload_string.equals("info")) {
      irsend.sendPronto(dishInfo, 26, 4);
    } else if (payload_string.equals("cancel")) {
      irsend.sendPronto(dishCancel, 26, 4);
    } else if (payload_string.equals("guidepageup")) {
      irsend.sendPronto(dishGuidePageUp, 26, 4);
    } else if (payload_string.equals("guidepagedown")) {
      irsend.sendPronto(dishGuidePageDown, 26, 4);
    } else if (payload_string.equals("sysinfo")) {
      irsend.sendPronto(dishSysInfo, 26, 4);
    }
  } else if (String(topic).equals("rc/atvio")) {
    if (payload_string.equals("power")) {
      irsend.sendNEC(atvioPower);
    } else if (payload_string.equals("tv")) {
      irsend.sendNEC(atvioTV);
    } else if (payload_string.equals("source")) {
      irsend.sendNEC(atvioSource);
    } else if (payload_string.equals("1")) {
      irsend.sendNEC(atvio1);
    } else if (payload_string.equals("2")) {
      irsend.sendNEC(atvio2);
    } else if (payload_string.equals("3")) {
      irsend.sendNEC(atvio3);
    } else if (payload_string.equals("4")) {
      irsend.sendNEC(atvio4);
    } else if (payload_string.equals("5")) {
      irsend.sendNEC(atvio5);
    } else if (payload_string.equals("6")) {
      irsend.sendNEC(atvio6);
    } else if (payload_string.equals("7")) {
      irsend.sendNEC(atvio7);
    } else if (payload_string.equals("8")) {
      irsend.sendNEC(atvio8);
    } else if (payload_string.equals("9")) {
      irsend.sendNEC(atvio9);
    } else if (payload_string.equals("0")) {
      irsend.sendNEC(atvio0);
    } else if (payload_string.equals(".")) {
      irsend.sendNEC(atvioDot);
    } else if (payload_string.equals("youtube")) {
      irsend.sendNEC(atvioYoutube);
    } else if (payload_string.equals("vol+")) {
      irsend.sendNEC(atvioVolPlus);
    } else if (payload_string.equals("vol-")) {
      irsend.sendNEC(atvioVolLess);
    } else if (payload_string.equals("ch+")) {
      irsend.sendNEC(atvioChPlus);
    } else if (payload_string.equals("ch-")) {
      irsend.sendNEC(atvioChLess);
    } else if (payload_string.equals("mute")) {
      irsend.sendNEC(atvioMute);
    } else if (payload_string.equals("home")) {
      irsend.sendNEC(atvioHome);
    } else if (payload_string.equals("netflix")) {
      irsend.sendNEC(atvioNetflix);
    } else if (payload_string.equals("browser")) {
      irsend.sendNEC(atvioBrowser);
    } else if (payload_string.equals("settings")) {
      irsend.sendNEC(atvioSettings);
    } else if (payload_string.equals("up")) {
      irsend.sendNEC(atvioUp);
    } else if (payload_string.equals("down")) {
      irsend.sendNEC(atvioDown);
    } else if (payload_string.equals("left")) {
      irsend.sendNEC(atvioLeft);
    } else if (payload_string.equals("right")) {
      irsend.sendNEC(atvioRight);
    } else if (payload_string.equals("ok")) {
      irsend.sendNEC(atvioOk);
    } else if (payload_string.equals("back")) {
      irsend.sendNEC(atvioBack);
    } else if (payload_string.equals("exit")) {
      irsend.sendNEC(atvioExit);
    } else if (payload_string.equals("info")) {
      irsend.sendNEC(atvioInfo);
    } else if (payload_string.equals("usb")) {
      irsend.sendNEC(atvioUSB);
    } else if (payload_string.equals("menu")) {
      irsend.sendNEC(atvioMenu);
    } else if (payload_string.equals("sleep")) {
      irsend.sendNEC(atvioSleep);
    } else if (payload_string.equals("guide")) {
      irsend.sendNEC(atvioGuide);
    } else if (payload_string.equals("fav")) {
      irsend.sendNEC(atvioFav);
    } else if (payload_string.equals("chlist")) {
      irsend.sendNEC(atvioChList);
    } else if (payload_string.equals("red")) {
      irsend.sendNEC(atvioRed);
    } else if (payload_string.equals("green")) {
      irsend.sendNEC(atvioGreen);
    } else if (payload_string.equals("yellow")) {
      irsend.sendNEC(atvioYellow);
    } else if (payload_string.equals("blue")) {
      irsend.sendNEC(atvioBlue);
    } else if (payload_string.equals("picture")) {
      irsend.sendNEC(atvioPicture);
    } else if (payload_string.equals("sound")) {
      irsend.sendNEC(atvioSound);
    } else if (payload_string.equals("cc")) {
      irsend.sendNEC(atvioCC);
    } else if (payload_string.equals("previous")) {
      irsend.sendNEC(atvioPrevious);
    } else if (payload_string.equals("stop")) {
      irsend.sendNEC(atvioStop);
    } else if (payload_string.equals("play/pause")) {
      irsend.sendNEC(atvioPlayPause);
    } else if (payload_string.equals("next")) {
      irsend.sendNEC(atvioNext);
    } else if (payload_string.equals("rewind")) {
      irsend.sendNEC(atvioRewind);
    } else if (payload_string.equals("forward")) {
      irsend.sendNEC(atvioForward);
    } else if (payload_string.equals("mts")) {
      irsend.sendNEC(atvioMTS);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attemping MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.subscribe("rc/samsung");
      client.subscribe("rc/dish");
      client.subscribe("rc/atvio");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}
