#include <ResponsiveAnalogRead.h>
#include <TeensyThreads.h>

// FADER TRIM SETTINGS
#define TOP 960
#define BOT 70
int faderTrimTop[8] = {TOP, TOP, TOP, TOP, TOP, TOP, TOP, TOP}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 255 AT THE TOP OF ITS TRAVEL
int faderTrimBottom[8] = {BOT, BOT, BOT, BOT, BOT, BOT, BOT, BOT}; // ADJUST THIS IF A SINGLE FADER ISN'T READING 0 AT THE BOTTOM OF ITS TRAVEL

// MOTOR SETTINGS
#define TOUCH_THRESHOLD 30
#define MIDI_LISTEN_CHANNEL 16

// MIDI SETTINGS
#define MIDI_SEND_CHANNEL 1
byte MIDI_CONTROLS[8] = {0, 1, 2, 3, 4, 5, 6, 7};

#define DEBUG true


elapsedMillis sinceMoved[8];
elapsedMillis sinceSent[8];
byte lastSentValue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte mode[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int previous[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static byte MOTOR_PINS_A[8] = {0, 2, 4, 6, 8, 10, 24, 28};
static byte MOTOR_PINS_B[8] = {1, 3, 5, 7, 9, 12, 25, 29};
ResponsiveAnalogRead faders[8] = {
  ResponsiveAnalogRead(A9, true),
  ResponsiveAnalogRead(A8, true),
  ResponsiveAnalogRead(A7, true),
  ResponsiveAnalogRead(A6, true),
  ResponsiveAnalogRead(A5, true),
  ResponsiveAnalogRead(A4, true),
  ResponsiveAnalogRead(A3, true),
  ResponsiveAnalogRead(A2, true)
};



#define REST 0
#define MOTOR 1
#define TOUCH 2
void setup() {
  Serial.begin(9600);
  delay(1500);
  Serial.println("###############################");
  Serial.println("Setting up Faders...");

  for (byte i = 0; i < 8; i++) {
    pinMode(MOTOR_PINS_A[i], OUTPUT);
    pinMode(MOTOR_PINS_B[i], OUTPUT);
    digitalWrite(MOTOR_PINS_A[i], LOW);
    digitalWrite(MOTOR_PINS_B[i], LOW);
    analogWriteFrequency(MOTOR_PINS_A[i], 18000);
    analogWriteFrequency(MOTOR_PINS_B[i], 18000);
    faders[i].setActivityThreshold(TOUCH_THRESHOLD);
  }
}



void loop() {

  usbMIDI.read(MIDI_LISTEN_CHANNEL);

  for (byte i = 0; i < 8; i++) {
    faders[i].update();
    
    if (faders[i].hasChanged()) {
      sinceMoved[i] = 0;
      if (mode[i] != MOTOR && previous[i] - getFaderValue(i) != 0) {
        mode[i] = TOUCH;
      }
    }

    if (mode[i] == TOUCH) {
      if (lastSentValue[i] == getFaderValue(i) && sinceMoved[i] > 900) {
        mode[i] = REST;
      } else if (sinceSent[i] > 30 && lastSentValue[i] != getFaderValue(i)) {
        sinceMoved[i] = 0;
        sinceSent[i] = 0;
        lastSentValue[i] = getFaderValue(i);
        usbMIDI.sendControlChange (MIDI_CONTROLS[i], getFaderValue(i)/2, 1);
      }
    }
    
    if(DEBUG){
      Serial.print(getFaderValue(i));
      Serial.print("\t");
    }
    previous[i] = getFaderValue(i);

  }
  if(DEBUG){
    Serial.println("");
  }
}



int getFaderValue(byte fader) {
  return max(0, min(255, map(faders[fader].getValue(), faderTrimBottom[fader], faderTrimTop[fader], 0, 255)));
}
