#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();
bool MIDIconnected = false;
byte MIDIdata[] = {0, 0, 0};
uint8_t _hue = 0;             // modifier for key color cycling
bool sustain = false;         // is sustain pedal on?
bool MidiEventReceived = false;
CRGB lastPressed;  // holder for last-detected key color

