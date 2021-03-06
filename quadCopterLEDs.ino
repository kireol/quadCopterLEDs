#include <FastLED.h>

#define INPUT_SWITCH 1;
#define LED_PIN     3
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    12
#define NUM_BACK_LEDS 8

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

//added this line as the digispark doesnt have this.  Remove for more standard boards e.g. Arduino Uno
volatile unsigned long timer0_millis = 0;

CRGB leds[NUM_LEDS];

// Quad Copter LEDs
// Borrowed most of the code from Fire2012 with programmable Color Palette
// Fire2012 https://plus.google.com/112916219338292742137/posts/CC6yursCCrN
// For use with an Arduino intended to be used on a quadcopter to control
// LEDs.  Standard mode will be white lights up front and red in back.
// When hyper mode is initiated, the rear LEDs will look like a flame.
// To initial hyper mode, the Arduino simply needs to read a pin, which can
// be sent from a free radio channel.

CRGBPalette16 gPal;

void setup() {
    pinMode(INPUT_SWITCH, INPUT);
    delay(3000); // sanity delay.  Don't want to brown out
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

    setFrontLEDs();
    FastLED.setBrightness(BRIGHTNESS);

    // This first palette is the basic 'black body radiation' colors,
    // which run from black to red to bright yellow to white.
    gPal = HeatColors_p;

    // These are other ways to set up the color palette for the 'fire'.
    // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
    //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

    // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
    //   gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);

    // Third, here's a simpler, three-step gradient, from black to red to white
    //   gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);

}

void loop() {
    setLEDColors();

    FastLED.show(); // display this frame
    FastLED.delay(1000 / FRAMES_PER_SECOND);
}

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

void setLEDColors(){
    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random());

    // Fourth, the most sophisticated: this one sets up a new palette every
    // time through the loop, based on a hue that changes every time.
    // The palette is a gradient from black, to a dark color based on the hue,
    // to a light color based on the hue, to white.
    //
    //   static uint8_t hue = 0;
    //   hue++;
    //   CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
    //   CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
    //   gPal = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);
    int switchValue = 0;
    switchValue = digitalRead(INPUT_SWITCH);
    if (switchValue == HIGH) {
        Fire2012WithPalette(); // run simulation frame, using palette colors
    }else{
        setRearLEDsToSingleColor();
    }
}

void setFrontLEDs() {
    leds[8] = CRGB::White;
    leds[9] = CRGB::White;
    leds[10] = CRGB::White;
    leds[11] = CRGB::White;

}

void setRearLEDsToSingleColor() {
    int colorToSet = CRGB::White;
    for (int i = 0; i < NUM_BACK_LEDS; i++) {
        leds[i] = colorToSet;
    }
}

void Fire2012WithPalette() {
// Array of temperature readings at each simulation cell
    static byte heat[NUM_BACK_LEDS];

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < NUM_BACK_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_BACK_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = NUM_BACK_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKING) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < NUM_BACK_LEDS; j++) {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        byte colorindex = scale8(heat[j], 240);
        leds[j] = ColorFromPalette(gPal, colorindex);
    }
}