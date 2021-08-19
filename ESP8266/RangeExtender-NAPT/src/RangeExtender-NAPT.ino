
// NAPT example released to public domain

#if LWIP_FEATURES && !LWIP_IPV6

#define HAVE_NETDUMP 0

#ifndef STASSID
#define STASSID "Home_1"
#define STAPSK  "12345678"
#endif

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>


#define NAPT 1000
#define NAPT_PORT 10

#if HAVE_NETDUMP

#include <NetDump.h>

void dump(int netif_idx, const char* data, size_t len, int out, int success) {
    (void)success;
    Serial.print(out ? F("out ") : F(" in "));
    Serial.printf("%d ", netif_idx);

    // optional filter example: if (netDump_is_ARP(data))
    {
        netDump(Serial, data, len);
        //netDumpHex(Serial, data, len);
    }
}
#endif

#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

extern "C" void preinit() {
	uint8_t sta_mac[] = { 0xb4, 0xe6, 0x2d, 0x44, 0x86, 0xaf };
	// uint8_t sta_mac[] = { 0xe8, 0x2a, 0x44, 0x94, 0x46, 0x26 };
	wifi_set_opmode(STATIONAP_MODE);
	wifi_set_macaddr(STATION_IF, sta_mac);
}

void setupOTA(){
    ArduinoOTA.setHostname("rangeextender");
    
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
}

void setup() {
    preinit();
    Serial.begin(115200);
    Serial.printf("\n\nNAPT Range extender\n");
    Serial.printf("Heap on start: %d\n", ESP.getFreeHeap());

#if HAVE_NETDUMP
    phy_capture = dump;
#endif

    // first, connect to STA so we can get a proper local DNS server
    WiFi.mode(WIFI_STA);
    WiFi.begin(STASSID, STAPSK);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
    }
    Serial.printf("\nSTA: %s (dns: %s / %s)\n",
    WiFi.localIP().toString().c_str(),
    WiFi.dnsIP(0).toString().c_str(),
    WiFi.dnsIP(1).toString().c_str());

    // give DNS servers to AP side
    // dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
    // dhcpSoftAP.dhcps_set_dns(1, WiFi.dnsIP(1));
    dhcpSoftAP.dhcps_set_dns(0, IPAddress(192, 168, 0, 200));
    // dhcpSoftAP.dhcps_set_dns(1, IPAddress(192, 168, 0,   1));
    // dhcpSoftAP.dhcps_set_dns(0, IPAddress(1, 1, 1, 1));
    // dhcpSoftAP.dhcps_set_dns(1, IPAddress(8, 8, 8, 8));

    WiFi.softAPConfig(  // enable AP, with android-compatible google domain
    IPAddress(172, 217, 28, 254),
    IPAddress(172, 217, 28, 254),
    IPAddress(255, 255, 255, 0));
    // WiFi.softAPConfig(  // enable AP, with android-compatible google domain
    // IPAddress(192, 168,   1,   1),
    // IPAddress(192, 168,   1,   1),
    // IPAddress(255, 255, 255,   0));

    // WiFi.softAP(STASSID "extender", STAPSK);
    WiFi.softAP("Homeextender", STAPSK);
    Serial.printf("AP: %s\n", WiFi.softAPIP().toString().c_str());

    Serial.printf("Heap before: %d\n", ESP.getFreeHeap());
    err_t ret = ip_napt_init(NAPT, NAPT_PORT);
    Serial.printf("ip_napt_init(%d,%d): ret=%d (OK=%d)\n", NAPT, NAPT_PORT, (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) {
        ret = ip_napt_enable_no(SOFTAP_IF, 1);
        Serial.printf("ip_napt_enable_no(SOFTAP_IF): ret=%d (OK=%d)\n", (int)ret, (int)ERR_OK);
        if (ret == ERR_OK) {
            Serial.printf("WiFi Network '%s' with same password is now NATed behind '%s'\n", STASSID "extender", STASSID);
        }
    }
    Serial.printf("Heap after napt init: %d\n", ESP.getFreeHeap());
    if (ret != ERR_OK) {
        Serial.printf("NAPT initialization failed\n");
    }
    
    setupOTA();
}

#else

void setup() {
    Serial.begin(115200);
    Serial.printf("\n\nNAPT not supported in this configuration\n");
}

#endif

void loop() {
    ArduinoOTA.handle();
}
