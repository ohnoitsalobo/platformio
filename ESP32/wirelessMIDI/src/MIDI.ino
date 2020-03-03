#include <WiFiUdp.h>
#include <AppleMidi.h>
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h
bool isConnected = false;

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
    void handleConnected(uint32_t ssrc, char* name) {
    isConnected  = true;
    // Serial.print(F("Connected to session "));
    Serial.print("Connected to session ");
    Serial.println(name);
    delay(1000);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void handleDisconnected(uint32_t ssrc) {
    isConnected  = false;
    // Serial.println(F("Disconnected"));
    Serial.println("Disconnected");
    delay(1000);
}

void setupMIDI(){
#ifdef DEBUGGER
    Serial.print("Setting up MIDI ... ");
#endif
    AppleMIDI.begin("wirelessMIDI");

    AppleMIDI.OnConnected(handleConnected);
    AppleMIDI.OnDisconnected(handleDisconnected);

    AppleMIDI.OnReceiveNoteOn(handleNoteOn);
    AppleMIDI.OnReceiveNoteOff(handleNoteOff);
    AppleMIDI.OnReceiveControlChange(handleControlChange);
#ifdef DEBUGGER
    Serial.println("done!\n");  
#endif
}

void runMIDI(){
#ifdef DEBUGGER
    Serial.print("MIDI loop ... ");
#endif
    // AppleMIDI.run();
    AppleMIDI.read();
#ifdef DEBUGGER
    Serial.print("done!\n");
#endif
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

/*	inline void OnConnected(void(*fptr)(uint32_t, char*));
	inline void OnDisconnected(void(*fptr)(uint32_t));

    inline void OnReceiveNoteOn(void (*fptr)(byte channel, byte note, byte velocity));
    inline void OnReceiveNoteOff(void (*fptr)(byte channel, byte note, byte velocity));
    inline void OnReceiveAfterTouchPoly(void (*fptr)(byte channel, byte note, byte pressure));
    inline void OnReceiveControlChange(void (*fptr)(byte channel, byte number, byte value));
    inline void OnReceiveProgramChange(void (*fptr)(byte channel, byte number));
    inline void OnReceiveAfterTouchChannel(void (*fptr)(byte channel, byte pressure));
    inline void OnReceivePitchBend(void (*fptr)(byte channel, int bend));
    inline void OnReceiveSysEx(void (*fptr)(const byte * data, uint16_t size));
    inline void OnReceiveTimeCodeQuarterFrame(void (*fptr)(byte data));
    inline void OnReceiveSongPosition(void (*fptr)(unsigned short beats));
    inline void OnReceiveSongSelect(void (*fptr)(byte songnumber));
    inline void OnReceiveTuneRequest(void (*fptr)(void));
    inline void OnReceiveClock(void (*fptr)(void));
    inline void OnReceiveStart(void (*fptr)(void));
    inline void OnReceiveContinue(void (*fptr)(void));
    inline void OnReceiveStop(void (*fptr)(void));
    inline void OnReceiveActiveSensing(void (*fptr)(void));
    inline void OnReceiveReset(void (*fptr)(void));
*/
