#define wifiTimeout 20
#define dns     IPAddress(192, 168, 13, 1)
#define gateway IPAddress(192, 168, 13, 1)
#define subnet  IPAddress(255, 255, 255, 0)
#define AP_prefix "HelioMesh_"

// String ssid = "Energy-XS", password = "000617E720", meshSSID = "", meshPass = "";
String ssid = "linksys1", password = "9182736450", meshSSID = "", meshPass = "";
String APpassword = String(ESP.getChipId());
String APssid = AP_prefix + APpassword;

IPAddress blynkServer(192, 168, 13, 2);
//~ IPAddress ip(192, 168, 13, 10); char blynkAuth[] = "6b14b321c9e84b639784e6c019d37269";
//~ IPAddress ip(192, 168, 13, 12); char blynkAuth[] = "0976ccba4c454ea1833863427a11c380";
//~ IPAddress ip(192, 168, 13, 14); char blynkAuth[] = "fb23e64fc82d472c8f6179a635c55b42";
IPAddress ip(192, 168, 13, 16); char blynkAuth[] = "fe59ba2760c54fb6bdf3e9b1751f49cc";
//~ IPAddress ip(192, 168, 13, 18); char blynkAuth[] = "fe59ba2760c54fb6bdf3e9b1751f49cc";

ESP8266WebServer server(80);
String webSite, javaScript, XML, webUpdate;
unsigned long temp = 0;

unsigned int localPort = 2390;      // local port to listen for UDP packets
//~ IPAddress timeServerIP(192, 168, 13, 5); // IPAddress timeServerIP;
IPAddress timeServerIP(129, 6, 15, 28); // IPAddress timeServerIP;
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
WiFiUDP udp, UDP;

void connectToWifi(){
	Serial.print("\r\nScanning networks ... \r\n");
	int n = WiFi.scanNetworks(); // scan networks including hidden, returns # of networks found
	boolean wifi_available = 0, mesh_available = 0;
	int best_RSSI = -999;
	uint32_t target_chip_id;

	for (int i = 0; i < n; ++i) { // iterate thru network list to find main network
		String current_ssid = WiFi.SSID(i);
		int rssi = WiFi.RSSI(i); 
		//~ if(current_ssid == ssid) { wifi_available = 0; }
		if(current_ssid == ssid) { wifi_available = 1; }
		if(current_ssid.substring(0,10) == String(AP_prefix)) { mesh_available = 1; }
		
		if(mesh_available && rssi > best_RSSI){
			best_RSSI = rssi;
			uint32_t id = current_ssid.substring(10).toInt();
			target_chip_id = (id != ESP.getChipId() ? id : target_chip_id);
		}
		Serial.print("\r\n");
		Serial.print(current_ssid);
		Serial.print("   ");
		Serial.print(rssi);
	}

	int Now = millis();
	if (!wifi_available && !mesh_available) {
		Serial.print("\r\nNo WiFi or mesh available, only AP: ");
		Serial.print(APssid);
		Serial.print("\r\n");
		WiFi.mode(WIFI_AP);
	}else{
		WiFi.mode(WIFI_AP_STA);
		if (wifi_available) {
			Serial.print("\r\n\nPreconfigured WiFi found: ");
			Serial.print(ssid);
			Serial.print("\r\n");
			// WiFi.config(ip, dns, gateway, subnet);
			WiFi.begin(ssid.c_str(), password.c_str()); 
			//~ Blynk.config(blynkAuth, blynkServer);
		}else if(mesh_available){
			Serial.print("\r\n\nMesh found! Strongest signal from: ");
			Serial.print(AP_prefix);
			Serial.print(target_chip_id);
			Serial.print("\r\n");
			meshPass = String(target_chip_id);
			meshSSID = AP_prefix + meshPass; // set mesh network details
			WiFi.begin(meshSSID.c_str(), meshPass.c_str());
		}

		while ((WiFi.status() != WL_CONNECTED) && (millis() <= (Now + wifiTimeout*1000))){
			yield();
			if(millis()%5000 == 0){ Serial.print("."); }
		}
		if(WiFi.status() != WL_CONNECTED){
			Serial.print("\r\nConnecting to WiFi took more than ");
			Serial.print(wifiTimeout);
			Serial.print(" seconds, aborted\r\n");
		}
	}
	

	if(WiFi.status() == WL_CONNECTED){
		// if(WiFi.localIP()[2] == 4){
			// WiFi.softAPConfig(IPAddress(192,168,3,1), IPAddress(192,168,3,1), IPAddress(255,255,255,0));
		// }
	}
	// WiFi.softAP(APssid.c_str(), APpassword.c_str());
	
	if(WiFi.status() == WL_CONNECTED){
		String wifiInfo = "";
		wifiInfo += "\r\nWiFi connected in " + String(millis() - Now) + " ms\r\n";
		wifiInfo += "\r\nNetwork SSID: " + WiFi.SSID();
		wifiInfo += "\r\nIP address:" + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "\r\n";
		wifiInfo += "\r\nAP SSID: " + APssid;
		wifiInfo += "\r\nAP IP address:" + (String)WiFi.softAPIP()[0] + "." + (String)WiFi.softAPIP()[1] + "." + (String)WiFi.softAPIP()[2] + "." + (String)WiFi.softAPIP()[3] + "\r\n";
		Serial.print(wifiInfo);
	}
}

time_t getNtpTime(){
	udp.begin(localPort);
	while (udp.parsePacket() > 0) ; // discard any previously received packets
	Serial.print("\r\nTransmitting NTP request ... ");
	// WiFi.hostByName(ntpServerName, timeServerIP); 
	sendNTPpacket(timeServerIP);
	uint32_t beginWait = millis();
	while (millis() - beginWait < 1500) {
		int size = udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Serial.print("response received");
			udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			udp.stop();
			lastSync = secsSince1900 - 2208988800UL + timezone * SECS_PER_HOUR;
			Serial.print("\r\nClock successfully synchronized with time server.\r\n");
			return lastSync;
		}
	}
	Serial.print("no response\r\n");
	return 0; // return 0 if unable to get the time
}

void sendNTPpacket(IPAddress& address)
{
	//~ Serial.println("sending NTP packet...");
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;

	// all NTP fields have been given values, now
	// you can send a packet requesting a timestamp:
	udp.beginPacket(address, 123); // NTP requests are to port 123
	udp.write(packetBuffer, NTP_PACKET_SIZE);
	udp.endPacket();
}
//~ /*

void buildWebsite(){
	webSite =  "<!DOCTYPE html>\n";
	webSite += "\t <body>\n";
	webSite += upTime();
	webSite += info();
	webSite += "\t\t <form method='POST' action='/update' enctype='multipart/form-data'>\n";
	webSite += "\t\t\t <input type='file' name='update'>\n";
	webSite += "\t\t\t <input type='submit' value='Update'>\n";
	webSite += "\t\t </form>\n";
	webSite += "\t </body>\n";
	webSite += "</html>\n";
}

String upTime(){
	//~ String Time = "\t\tProgram compile time: " + __TIME__ + "<br />";
	String Time = "\t\tESP8266 Uptime: ";
	unsigned long ss;
	byte mm, hh;
	ss = millis()/1000;
	hh = ss / 3600;
	mm = (ss - hh * 3600) / 60;
	ss = (ss - hh * 3600) - mm * 60;
	Time += (hh < 10 ? "0" : "") + (String)hh + ":";
	Time += (mm < 10 ? "0" : "") + (String)mm + ":";
	Time += (ss < 10 ? "0" : "") + (String)ss + "<br />\n";
	if(timeStatus() == timeNotSet){
		Time += "\n\t\tCurrent Time: Unknown <br />\n\t\tLast clock update: Never<br />\n";
	}else{
		Time += "\n\t\tCurrent Time: ";
		Time += (  hour(now()) < 10 ? "0" : "") +   (String)hour(now()) + ":";
		Time += (minute(now()) < 10 ? "0" : "") + (String)minute(now()) + ":";
		Time += (second(now()) < 10 ? "0" : "") + (String)second(now()) + "<br />\n";
		Time += "\n\t\tLast clock update: ";
		time_t t = now() - lastSync;
		Time +=   (String)hour(t) + " hr ";
		Time += (String)minute(t) + " min ";
		Time += (String)second(t) + " sec ago<br />\n";
	}
	return Time;
}

String info(){
	String Info = "\t <br />Machine information:<br />\n";
	Info += "\t <br />IP Address: " + (String)WiFi.localIP()[0] + "." + (String)WiFi.localIP()[1] + "." + (String)WiFi.localIP()[2] + "." + (String)WiFi.localIP()[3] + "<br /><br />";
	Info += "\t <table border =\"1\" style=\"width=50%\">\n";
	Info += "\t\t <tr>\n<th></th>\n<th>Sun</th>\n<th>Target</th>\n<th>Machine</th>\n</tr>\n";
	Info += "\t\t <tr><th>Altitude:</th>\n<td>" + (String)SunsAltitude + "</td>\n<td>" + (String)MachineTargetAlt[0] + "</td>\n<td>" + (String)MachinesPrevAlt[0] + "</tr>\n";
	Info += "\t\t <tr><th>Azimuth:</th>\n<td>" +  (String)SunsAzimuth + "</td>\n<td>" +  (String)MachineTargetAz[0] + "</td>\n<td>" +  (String)MachinesPrevAz[0] + "</tr>\n";
	Info += "\t </table><br />\n<br />\n";
	return Info;
}

void handleWebsite(){
	buildWebsite();
	Serial.print("\r\n Serving webpage \r\n");
	server.sendHeader("Connection", "close");
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "text/html", webSite);
}

void handleUpload(){
	if(server.uri() != "/update") return;
	HTTPUpload& upload = server.upload();
	Serial.setDebugOutput(true);
	if(upload.status == UPLOAD_FILE_START){
		WiFiUDP::stopAll();
		Serial.printf("\r\nReceiving update file: %s\r\n", upload.filename.c_str()); temp = millis();
		uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if(!Update.begin(maxSketchSpace)){ //start with max available size
			Update.printError(Serial);
		}
	} else if(upload.status == UPLOAD_FILE_WRITE){
		Serial.print(".");
		if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
			Update.printError(Serial);
		}
	} else if(upload.status == UPLOAD_FILE_END){
		if(Update.end(true)){ //true to set the size to the current progress
			temp = millis() - temp;
			Serial.printf("\r\nUpdate Success: %u bytes in %u ms (", upload.totalSize, temp);
			Serial.print((float)upload.totalSize/temp);
			Serial.printf(" kB/s)\r\nEffective program size: %u bytes\r\n", upload.totalSize - 202656);
			Serial.print("\r\nRebooting...\r\n");
		} else {
			Update.printError(Serial);
		}
	}
	yield();
	Serial.setDebugOutput(false);
}

void beginServer(){
	server.onFileUpload(handleUpload);
	server.on("/update", HTTP_POST, [](){
		server.sendHeader("Connection", "close");
		server.sendHeader("Access-Control-Allow-Origin", "*");
		server.send(200, "text/plain", (Update.hasError())?"FAIL\n":"OK\n");
		ESP.restart();
	});
	server.on("/", handleWebsite);
	server.begin(); serverOn = 1;
	Serial.print("\r\nESP8266 server started\r\n");
}

void runServer(){
	server.handleClient();
}

//~ void runBlynk(){
	//~ Blynk.run();
//~ }
// */

//~ void client_status(){
//~ 
	//~ unsigned char number_client;
	//~ struct station_info *stat_info;
//~ 
	//~ struct ip_addr *IPaddress;
	//~ IPAddress address;
	//~ int i=1;
//~ 
	//~ number_client= wifi_softap_get_station_num(); // Count of stations which are connected to ESP8266 soft-AP
	//~ stat_info = wifi_softap_get_station_info();
//~ 
	//~ Serial.print(" Total connected_client are = ");
	//~ Serial.println(number_client);
//~ 
	//~ while (stat_info != NULL) {
//~ 
		//~ IPaddress = &stat_info->ip;
		//~ address = IPaddress->addr;
//~ 
		//~ Serial.print("client= ");
//~ 
		//~ Serial.print(i);
		//~ Serial.print(" ip adress is = ");
		//~ Serial.print((address));
		//~ Serial.print(" with mac adress is = ");
//~ 
		//~ Serial.print(stat_info->bssid[0],HEX);
		//~ Serial.print(stat_info->bssid[1],HEX);
		//~ Serial.print(stat_info->bssid[2],HEX);
		//~ Serial.print(stat_info->bssid[3],HEX);
		//~ Serial.print(stat_info->bssid[4],HEX);
		//~ Serial.print(stat_info->bssid[5],HEX);
//~ 
		//~ stat_info = STAILQ_NEXT(stat_info, next);
		//~ i++;
		//~ Serial.println();
//~ 
	//~ }
//~ }

void sendUDP(time_t t){
	union u_tag{ byte b[4]; unsigned long l; } u;
	u.l = t;
	Serial.println(u.l);
	//~ for(int i = 2; i < 6; i++){
		//~ udp.beginPacketMulticast(IPAddress(192, 168, 4, 255), 12345, WiFi.softAPIP());
		UDP.beginPacket(IPAddress(192,168,13,18), 12345);
		UDP.write(u.b[3]);
		UDP.write(u.b[2]);
		UDP.write(u.b[1]);
		UDP.write(u.b[0]);
		UDP.endPacket();
	//~ }
}

void receiveUDP(){
	//~ UDP.beginMulticast(WiFi.localIP(), IPAddress(192, 168, 4, 255), 12345);
	UDP.begin(12345);
	//~ int size = udp.parsePacket();
	//~ union u_tag{ byte b[4]; unsigned long l; } u; u.l = 0;
	//~ if(size >= 1){
		//~ UDP.read(u.b, 4);
		//~ UDP.stop();
		//~ Serial.print(u.l);
	//~ }
	int noBytes = UDP.parsePacket();
	Serial.println(noBytes);
	if ( noBytes ) {
		Serial.print(millis() / 1000);
		Serial.print(":Packet of ");
		Serial.print(noBytes);
		Serial.print(" received from ");
		Serial.print(udp.remoteIP());
		Serial.print(":");
		Serial.println(udp.remotePort());
		// We've received a packet, read the data from it
		UDP.read(packetBuffer,noBytes); // read the packet into the buffer

		// display the packet contents in HEX
		for (int i=1;i<=noBytes;i++){
			Serial.print(packetBuffer[i-1],HEX);
			if (i % 32 == 0){
				Serial.println();
			}
			else Serial.print(' ');
		} // end for
		Serial.println();
    }
    UDP.stop();
}
