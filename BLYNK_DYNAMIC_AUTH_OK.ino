/*
 Proof of implementation for two concepts:
 1. Dynamic connection to Blynk by entering BLYNK_AUTH_TOKEN in the browser (no additional code inside the Blynk library is now required!!!)
 2. A universal bridge for all Blynk connection types. I created
 Dynamic connection based on a flag from Blynk (or from a sensor, button, etc.) for Wi-Fi and modem connections.

It works flawlessly. For example, I implemented it: if Wi-Fi stops working, the modem reconnects and disables the controller's Wi-Fi. Data is sent to MQTT, Node Red, or the Blynk server without rebooting. Now even engineers can use Blynk, as true connection redundancy has been introduced, overcoming the ridiculous limitation of Blynk's creators.
 
  ========================================================================================================================================================
  I also developed asynchronous connection code for the modem and Wi-Fi, which allows the system to execute code (sensors, buttons) even while the modem and Wi-Fi are connecting. 
  
  I don't intend to offer it for free yet...
     
*/

#define BLYNK_PRINT Serial
// this 2 lines  need if you can not modified Blynk library. :). 
#define BLYNK_TEMPLATE_ID "NO_WRITE_HERE!!!" // :)
#define BLYNK_TEMPLATE_NAME "NO_WRITE_HERE!!!"
bool blynkRUN_b = false; // flag for Blynk Up/Down

#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
GyverDBFile db(&LittleFS, "/data.db");

#include <SettingsESP.h>
SettingsESP sett("WiFi config", &db);

DB_KEYS(
    DB,
    wifi_ssid,
    wifi_pass,
    BLYNK_AUTH_TOKEN,
    apply );

void build(sets::Builder& b) {
    {
        sets::Group g(b, "WiFi");
        b.Input(DB::wifi_ssid, "SSID");
        b.Pass(DB::wifi_pass, "Password");
        b.Input(DB::BLYNK_AUTH_TOKEN, "BLYNK_AUTH_TOKEN");
        if (b.Button(DB::apply, "Save & Restart")) {
            db.update();  // save in DB no wait for save timer
            delay(400);
            ESP.restart();
        }
    }
}
//******************************************************************
//Blynk_Bridge for all type of connections/////////////////////////////////////////////////////////
WiFiClient wifiClient; //  WiFi client (for BlynkBridge.config)

#ifndef ESP32
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#include <Blynk.h>
#include <Blynk/BlynkProtocol.h>
#include <Adapters/BlynkArduinoClient.h>

#ifndef BLYNK_INFO_CONNECTION
#define BLYNK_INFO_CONNECTION "Bridge"
#endif

#ifndef BLYNK_HEARTBEAT
#define BLYNK_HEARTBEAT 60
#endif

#ifndef BLYNK_TIMEOUT_MS
#define BLYNK_TIMEOUT_MS 6000
#endif

#define BLYNK_SEND_ATOMIC

class BlynkBridge
  : public BlynkProtocol<BlynkArduinoClient> {
  typedef BlynkProtocol<BlynkArduinoClient> Base;
public:
  BlynkBridge(BlynkArduinoClient& transp)
    : Base(transp) {
  }

  void config(const char* auth,
              const char* domain = BLYNK_DEFAULT_DOMAIN,
              uint16_t port = BLYNK_DEFAULT_PORT) {
    Base::begin(auth);
    conn.begin(domain, port);
  }


  void connectClient(Client& client) {
    conn.setClient(&client);
  }

  void begin(Client& client, const char* auth,
             const char* domain = BLYNK_DEFAULT_DOMAIN,
             uint16_t port = BLYNK_DEFAULT_PORT) {
    connectClient(client);
    config(auth, domain, port);
    while (this->connect() != true) {}
  }
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_BLYNK)
static BlynkArduinoClient _blynkTransport;
BlynkBridge Blynk(_blynkTransport);
#else
extern BlynkBridge Blynk;
#endif

#include <BlynkWidgets.h>
//////////////////////////////////////////////////////////////////
//******************************************************************

void setup() {
    Serial.begin(115200);
    Serial.println();
     // Mode AP_STA. CALL BEFORE sett.begin(), let
    // settings to know wifi mode
    WiFi.mode(WIFI_AP_STA);

    sett.begin();
    sett.onBuild(build);

    //Start Data Base Before connections!
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif
    db.begin();
    db.init(DB::wifi_ssid, "");
    db.init(DB::wifi_pass, "");
    db.init(DB::BLYNK_AUTH_TOKEN, "");

    // ======= AP =======
    WiFi.softAP("BLYNK_CONFIG");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());

//BLYNK CONNECTION /////////////////////
 // 1. settings Blynk from memory (input BLYNK_AUTH_TOKEN )
 String token = db[DB::BLYNK_AUTH_TOKEN];
  Blynk.config(token.c_str());
 
  // 2. Start WiFi:
  // ======= STA =======
    // if  BLYNK_AUTH_TOKEN inputed (no check validation for this example!!!)
    if (db[DB::BLYNK_AUTH_TOKEN].length()) {
        WiFi.begin(db[DB::wifi_ssid], db[DB::wifi_pass]);
        Serial.print("Connect STA");
        int tries = 20;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print('.');
            if (!--tries) break;
        }
        Serial.println();
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        blynkRUN_b = true; // flag Blynk connect is ON
    }

  //WiFi.begin(ssid, pass); // or simple wifi start
  Serial.println("Setup done. Waiting for connection...");
 // 3. Connecting bridge 
  Blynk.connectClient(wifiClient);

  if (WiFi.isConnected() and blynkRUN_b) Blynk.connect();

////////////////////////////////////////

}

void loop() {
    sett.tick();
    if (WiFi.isConnected() and blynkRUN_b) {
Blynk.run();  // Use global Blynk from  BlynkBridge.h
    
 }
}
