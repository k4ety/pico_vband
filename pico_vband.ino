// SPDX-License-Identifier: MIT
//
// VBAND CW USB dongle based on a Pi Pico.
// Also supports other key outputs for Vail and morsecode.me etc.
//
// Inspired from similar projects at:
// https://github.com/sipsmi/vband_dongle/tree/main
// https://github.com/mgiugliano/MorsePaddle2USB
// https://github.com/nealey/vail-adapter
//
// There is also a PiPico circuit python implementation shown on 
// https://www.qrz.com/db/KG5QNO

#include <Keyboard.h>
#include <KeyboardLayout.h>
#include <singleLEDLibrary.h>	// https://github.com/SethSenpai/singleLEDLibrary

#define VERSION 1.0

#define DEBUG 1

#define KEY_GROUND_PIN 13
#define KEY_RING_PIN 12
#define KEY_TIP_PIN 9

#define HEADER_GROUND_PIN_1 1
#define HEADER_GROUND_PIN_2 3
#define HEADER_PIN_1 0
#define HEADER_PIN_2 2
#define HEADER_PIN_3 4

#define LED_PIN LED_BUILTIN
// If you want an external LED connection then set this value and have an led connected between this pin and ground
//#define LED_PIN 29

// We support these sites:
// vband - https://hamradio.solutions/vband/
// vail - https://vail.woozle.org/#
// morsecode.me - https://morsecode.me/#/help/practice

// VBAND can use a number of different key combinations in the web browser:
// - [ ]
// - CTRL-L CTRL-R
// Supported by VBAND, Vail, and MorseMania iOS app

#define VBAND_DIT '['
#define VBAND_DAH ']'

// Vail can use a number of different key combinations in the web browser:
// - [ ]
// - CTRL-L CTRL-R
// - . /
// - x z
// We are deliberate choosing some different to VBAND
#define VAIL_DIT 'x'
#define VAIL_DAH 'z'

// morsecode.me feels like it only supports a straight key, and has a number of keyboard
// chars it supports
// - .
// - i
// - e
// - [space]
// We are deliberate choosing something different to the others
#define MORSECODE_DIT 'e'
#define MORSECODE_DAH 'i'

// And finally, support the CTRL key mode
// Supported by VBAND, Vail, and Morse-it iOS app
#define CTRL_DIT KEY_LEFT_CTRL
#define CTRL_DAH KEY_RIGHT_CTRL

int ring_index = 0;
int tip_index = 1;

char mode_array[][2] = {
  {CTRL_DIT, CTRL_DAH},           // 0 = L-CTRL/R-CTRL
  {VBAND_DIT, VBAND_DAH},         // 1 = [/]
  {VAIL_DIT, VAIL_DAH},           // 2 = x/z
  {MORSECODE_DIT, MORSECODE_DAH}  // 3 = e/i
};

int default_mode = 0; // Default to using CTRL Keys
int current_mode = default_mode;
const int max_mode = sizeof(mode_array)/sizeof(mode_array[1]);

#define MODE_BLINK_ON_TIME 100
#define MODE_BLINK_OFF_TIME 50
// Make sure this array is at least twice as long as the number of modes we have,
// as it encompasses both on/off times
int ledflash[] = {
  MODE_BLINK_ON_TIME, MODE_BLINK_OFF_TIME,
  MODE_BLINK_ON_TIME, MODE_BLINK_OFF_TIME,
  MODE_BLINK_ON_TIME, MODE_BLINK_OFF_TIME,
  MODE_BLINK_ON_TIME, MODE_BLINK_OFF_TIME,
  MODE_BLINK_ON_TIME, MODE_BLINK_OFF_TIME
};

//Our activity indicator led
sllib led(LED_PIN);

//Track key states so we can flash LED in time.
bool ring_prev = false;
bool tip_prev = false;
bool reverse_ring_tip = false;

// Try to debounce the keys a bit. Let's see if we only need to debounce from the down time - that is,
// ignore any 'ups' for this amount of time after the first down.
unsigned long key_debounce_ms = 30;
unsigned long ring_down_ms = 0;
unsigned long tip_down_ms = 0;

void setup() {
  if (DEBUG) { Serial.begin(115200); delay(3000); }
  
  // Setup Paddles/Key Pins
  pinMode(KEY_GROUND_PIN, OUTPUT);
  digitalWrite(KEY_GROUND_PIN, LOW);
  pinMode(KEY_RING_PIN, INPUT_PULLUP);
  pinMode(KEY_TIP_PIN, INPUT_PULLUP);

  // Setup Mode Pins and determine current mode
  pinMode(HEADER_PIN_1, INPUT_PULLUP);
  pinMode(HEADER_PIN_2, INPUT_PULLUP);
  pinMode(HEADER_PIN_3, INPUT_PULLUP);

  pinMode(HEADER_GROUND_PIN_1, OUTPUT);
  digitalWrite(HEADER_GROUND_PIN_1, LOW);
  if (digitalRead(HEADER_PIN_1) == LOW) current_mode = 1;
  else if (digitalRead(HEADER_PIN_2) == LOW) current_mode = 2;
  pinMode(HEADER_GROUND_PIN_1, INPUT);

  pinMode(HEADER_GROUND_PIN_2, OUTPUT);
  digitalWrite(HEADER_GROUND_PIN_2, LOW);
  if (digitalRead(HEADER_PIN_2) == LOW) current_mode = 3;
  
  if (digitalRead(HEADER_PIN_3) == LOW) reverse_ring_tip = true;
  pinMode(HEADER_GROUND_PIN_2, INPUT);

  if (reverse_ring_tip) {
    if (DEBUG) Serial.println("Reversing ring and tip inputs");
    ring_index = 1;
    tip_index = 0;
  } else {
    Serial.println("Ring and tip inputs normal");
  }
  if (DEBUG) Serial.println("Mode " + String(current_mode));

  // initialise HID library
  Keyboard.begin();

  // Start with a 'heartbeat' to show we are alive. Once a key is pressed this will stop and we move into
  // flashing the LED for each further activity.
  led.setBreathSingle(1000);  //We start with the breathing LED.
}

void loop(){
  int clicks;
  bool ring_current = false;
  bool tip_current = false;

  // timer.tick();
  led.update();
  
  // Normally debounce routines wait for a set period of time (say, 10ms)
  // and look for 'stability' of the button value before registering the
  // button (key) has been pressed. What we want is to know as soon as the
  // button is pressed (or released), so acknowledge the press/release as
  // soon as we detect a change in state - but then to avoid the following
  // potential bounces, we ignore all other changes for a set time (say, 10ms).
  //
  // One potential downside of this method is that it is likely not noise immune.
  // That is, if some noise turns up and briefly twiddles the pin, we will take that
  // as a press or release. But, that's the price we pay.

  ring_current = digitalRead(KEY_RING_PIN) == LOW;  // active low!
  tip_current = digitalRead(KEY_TIP_PIN) == LOW;    // active low!

  if (ring_current != ring_prev ) {                 // Did the key change state?
    unsigned long ms = millis();
    if (ring_down_ms + key_debounce_ms < ms ) {     // Done debouncing, so lets process it
      if (ring_current) {                           // Key down
        ring_down_ms = ms;
        digitalWrite(LED_PIN, HIGH);
        if (DEBUG) Serial.println("Ring down");
        Keyboard.press(mode_array[current_mode][ring_index]);        
      } else {                                      // Key up
        ring_down_ms = ms;
        led.setOffSingle();
        if (DEBUG) Serial.println("Ring up");
        Keyboard.release(mode_array[current_mode][ring_index]);      
      }
      ring_prev = ring_current;                     // Remember which state we are in...
    }
  }

  if (tip_current != tip_prev ) {                   // Did the key change state?
    unsigned long ms = millis();
    if (tip_down_ms + key_debounce_ms < ms ) {      // Done debouncing, so lets process it      
      if (tip_current) {                            // Key down
        tip_down_ms = ms;
        digitalWrite(LED_PIN, HIGH);
        if (DEBUG) Serial.println("Tip down");
        Keyboard.press(mode_array[current_mode][tip_index]);        
      } else {                                      // Key up
        tip_down_ms = ms;
        led.setOffSingle();
        if (DEBUG) Serial.println("Tip up");
        Keyboard.release(mode_array[current_mode][tip_index]);      
      }
      tip_prev = tip_current;                       // Remember which state we are in...
    }
  }
}
