void setupMIDI(){
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    MIDI.setHandleControlChange(handleControlChange);
    
    MIDI.begin(MIDI_CHANNEL_OMNI);
    
    AppleMIDI_W.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
        isConnected++;
        DBG(F("Connected to session"), ssrc, name);
    });
    AppleMIDI_W.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
        isConnected--;
        DBG(F("Disconnected"), ssrc);
    });

    MIDI_W.setHandleNoteOn(handleNoteOn);
    MIDI_W.setHandleNoteOff(handleNoteOff);
    MIDI_W.setHandlePitchBend(handlePitchBend);
    MIDI_W.setHandleControlChange(handleControlChange);

    MIDI_W.begin(MIDI_CHANNEL_OMNI);
}

void runMIDI(){
    MIDI.read();

    MIDI_W.read();
    yield();
}

// do X when a key is pressed
void handleNoteOn(byte channel, byte pitch, byte velocity) {
    // MIDI note values 0 - 127 
    // 36-96 (for 61-key) mapped to LED 0-60
    int temp = map(pitch, 36, 96, 0, NUM_LEDS-1);
    
    if(temp < 0)
        temp = -temp;                   // if note goes above 56 or below 0
    else if(temp > NUM_LEDS)                  //      reverse it
        temp = NUM_LEDS - (temp%NUM_LEDS);
    
    uint8_t _pitch = map(temp, 0, NUM_LEDS, 0, 224); // map note to color 'hue'
    uint8_t _pos = map(temp, 0, NUM_LEDS, 1, NUM_LEDS-1);
    // assign color based on note position and intensity (velocity)
    leds[_pos] = CHSV(_pitch + gHue, 255 - velocity, velocity/127.0 * 255);
    lastPressed = leds[_pos]; // remember last-detected note color
    yield();
}

// do X when a key is released
void handleNoteOff(byte channel, byte pitch, byte velocity) {
    yield();
}


// do X when pitch bend is used
void handlePitchBend(byte channel, int bend) {
    // fill strip with solid color based on pitch bend amount
    fill_solid(leds, NUM_LEDS, CHSV(map(bend, -8192, 8192, 0, 224), 255, 125)); // 0  8192  16383
}

// do X when control channels are used
void handleControlChange(byte channel, byte number, byte value){
    // channel 1 = modulation
    if( number == 1 ){
        fill_solid( leds, NUM_LEDS, 0x222222 );
    }
    // channel 64 = damper / sustain pedal
    if( number == 64 ){
        if( value >= 64 ){
            fill_solid( leds, NUM_LEDS, lastPressed );
            sustain = true;
        } else {
            sustain = false;
        }
    }
}


// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void handleConnected(uint32_t ssrc, char* name) {
    isConnected  = true;
    // Serial.print("Connected to session ");
    // Serial.println(name);
    delay(1000);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void handleDisconnected(uint32_t ssrc) {
    isConnected  = false;
    // Serial.println("Disconnected");
    delay(1000);
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

/*
    inline void setHandleMessage(void (*fptr)(const MidiMessage&)) { mMessageCallback = fptr; };
    inline void setHandleError(ErrorCallback fptr) { mErrorCallback = fptr; }
    inline void setHandleNoteOff(NoteOffCallback fptr) { mNoteOffCallback = fptr; }
    inline void setHandleNoteOn(NoteOnCallback fptr) { mNoteOnCallback = fptr; }
    inline void setHandleAfterTouchPoly(AfterTouchPolyCallback fptr) { mAfterTouchPolyCallback = fptr; }
    inline void setHandleControlChange(ControlChangeCallback fptr) { mControlChangeCallback = fptr; }
    inline void setHandleProgramChange(ProgramChangeCallback fptr) { mProgramChangeCallback = fptr; }
    inline void setHandleAfterTouchChannel(AfterTouchChannelCallback fptr) { mAfterTouchChannelCallback = fptr; }
    inline void setHandlePitchBend(PitchBendCallback fptr) { mPitchBendCallback = fptr; }
    inline void setHandleSystemExclusive(SystemExclusiveCallback fptr) { mSystemExclusiveCallback = fptr; }
    inline void setHandleTimeCodeQuarterFrame(TimeCodeQuarterFrameCallback fptr) { mTimeCodeQuarterFrameCallback = fptr; }
    inline void setHandleSongPosition(SongPositionCallback fptr) { mSongPositionCallback = fptr; }
    inline void setHandleSongSelect(SongSelectCallback fptr) { mSongSelectCallback = fptr; }
    inline void setHandleTuneRequest(TuneRequestCallback fptr) { mTuneRequestCallback = fptr; }
    inline void setHandleClock(ClockCallback fptr) { mClockCallback = fptr; }
    inline void setHandleStart(StartCallback fptr) { mStartCallback = fptr; }
    inline void setHandleTick(TickCallback fptr) { mTickCallback = fptr; }
    inline void setHandleContinue(ContinueCallback fptr) { mContinueCallback = fptr; }
    inline void setHandleStop(StopCallback fptr) { mStopCallback = fptr; }
    inline void setHandleActiveSensing(ActiveSensingCallback fptr) { mActiveSensingCallback = fptr; }
    inline void setHandleSystemReset(SystemResetCallback fptr) { mSystemResetCallback = fptr; }

    inline void disconnectCallbackFromType(MidiType inType);
    
    void handleNoteOff(byte channel, byte note, byte velocity);
    void handleNoteOn(byte channel, byte note, byte velocity);
    void handleAfterTouchPoly(byte channel, byte note, byte pressure);
    void handleControlChange(byte channel, byte number, byte value);
    void handleProgramChange(byte channel, byte number);
    void handleAfterTouchChannel(byte channel, byte pressure);
    void handlePitchBend(byte channel, int bend);
    void handleSystemExclusive(byte* array, unsigned size);
    void handleTimeCodeQuarterFrame(byte data);
    void handleSongPosition(unsigned int beats);
    void handleSongSelect(byte songnumber);
    void handleTuneRequest(void);
    void handleClock(void);
    void handleStart(void);
    void handleContinue(void);
    void handleStop(void);
    void handleActiveSensing(void);
    void handleSystemReset(void);
*/

