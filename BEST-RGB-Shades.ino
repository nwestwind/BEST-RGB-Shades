// RGB Shades data output to LEDs is on pin 5
#define LED_PIN  5

// RGB Shades color order (Green/Red/Blue)
#define COLOR_ORDER GRB
#define CHIPSET     WS2811

// Global maximum brightness value, maximum 255
#define MAXBRIGHTNESS 50
#define STARTBRIGHTNESS 96
byte currentBrightness = STARTBRIGHTNESS; // 0-255 will be scaled to 0-MAXBRIGHTNESS

// Cycle time (milliseconds between pattern changes)
#define cycleTime 15000

// Hue time (milliseconds between hue increments)
#define hueTime 30

// Time after changing settings before settings are saved to EEPROM
#define EEPROMDELAY 2000

// Include FastLED library and other useful files
#include <FastLED.h>
#include <EEPROM.h>
#include "messages.h"
#include "graphicsframe.h"
#include "font.h"
#include "palettes.h"
#include "XYmap.h"
#include "utils.h"
#include "audio.h"
#include "FireworksXY.h"
#include "effects.h"
#include "buttons.h"

// list of functions that will be displayed
functionList effectListAudio[] = {
  audioShadesOutline, 
  audioStripes, 
  audioCirc, 
  drawVU, 
  RGBpulse, 
  audioPlasma, 
  drawAnalyzer
};

functionList effectListNoAudio[] = {
  amazingNoise,
  Fire2012WithPalette, 
  snow,
  slantBars,
  eyesAnim,
  coloredSnow,
  colorRotation,
  barfight,
  radiateCenter,
  shadesOutline,
  eyes, 
  shadesOutline2,
  fireworks,  
  shadesOutline3,
  vertThreeSine,
  amazing,
  barfight2,
  hearts,
  threeSine, 
  radiateCenterMultiPalette,
  plasma,
  confetti,
  rider,
  glitter,
  threeDee,
  colorFill,
  sideRain
  // scrollTextZero
  // scrollTextOne, 
  // scrollTextTwo,
  // scrollTextThree
  };


byte numEffects;
const byte numEffectsAudio = (sizeof(effectListAudio) / sizeof(effectListAudio[0]));
const byte numEffectsNoAudio = (sizeof(effectListNoAudio) / sizeof(effectListNoAudio[0]));


// Runs one time at the start of the program (power up or reset)
void setup() {

  byte eepromWasWritten = EEPROM.read(0);
  if (eepromWasWritten == 99) {
    currentEffect = EEPROM.read(1);
    autoCycle = EEPROM.read(2);
    currentBrightness = EEPROM.read(3);
    audioEnabled = EEPROM.read(4);
  }
  
  if(audioEnabled) {
    numEffects = numEffectsAudio;
  } else {
    numEffects = numEffectsNoAudio;
  }

  if (currentEffect > (numEffects - 1)) currentEffect = 0;

  // write FastLED configuration data
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LAST_VISIBLE_LED + 1);

  // set global brightness value
  FastLED.setBrightness( scale8(currentBrightness, MAXBRIGHTNESS) );
  //FastLED.setDither(0);
  // configure input buttons
  pinMode(MODEBUTTON, INPUT_PULLUP);
  pinMode(BRIGHTNESSBUTTON, INPUT_PULLUP);
  pinMode(STROBEPIN, OUTPUT);
  pinMode(RESETPIN, OUTPUT);

  analogReference(DEFAULT);

  digitalWrite(RESETPIN, LOW);
  digitalWrite(STROBEPIN, HIGH);

  random16_add_entropy(analogRead(ANALOGPIN));
  //Serial.begin(115200);
}

// Runs over and over until power off or reset
void loop()
{
  currentMillis = millis(); // save the current timer value
  updateButtons();          // read, debounce, and process the buttons
  doButtons();              // perform actions based on button state
  checkEEPROM();            // update the EEPROM if necessary
    
  // analyze the audio input
  if (audioActive) {
    if (currentMillis - audioMillis > AUDIODELAY) {
      audioMillis = currentMillis;
      doAnalogs();
    }
  }

  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > cycleTime && autoCycle == true) {
    cycleMillis = currentMillis;
    if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
    effectInit = false; // trigger effect initialization when new effect is selected
    audioActive = false;
  }

  // increment the global hue value every hueTime milliseconds
  if (currentMillis - hueMillis > hueTime) {
    hueMillis = currentMillis;
    hueCycle(1); // increment the global hue value
  }

  // run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;
    if (audioEnabled) {
      effectListAudio[currentEffect]();
    } else {
      effectListNoAudio[currentEffect]();
    }
    //random16_add_entropy(1); // make the random values a bit more random-ish
  }

  // run a fade effect
  if (fadeActive > 0) {
    fadeAll(fadeActive);
  }

  FastLED.show(); // send the contents of the led memory to the LEDs

}
