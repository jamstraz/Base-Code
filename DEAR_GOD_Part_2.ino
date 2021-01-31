//LegionDD Code Alpha
//v 0.6a
//Rev 2 June 10, 2020


#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN 12
#define NUM_LEDS 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

enum {
  STATE_ATTACK,
  STATE_SUSTAIN,
  STATE_DECAY,
  STATE_RELEASE,
  STATE_FINISHED
};

typedef struct {
  uint8_t r, g, b;
} pixelColor_t;

typedef struct {
  pixelColor_t attackColor; // the color the animation starts at
  pixelColor_t sustainColor; // the color the animation peaks at
  pixelColor_t releaseColor; // the color the animation ends at to release
  pixelColor_t decayColor; // the color the animation ends at

  uint16_t attack; // milliseconds to take to fade from attackColor to sustainColor (0 - 65536)
  uint16_t sustain; // milliseconds to sustain the sustainColor (0 - 65536)
  uint16_t release; // milliseconds to sustain the releasenColor (0 - 65536)
  uint16_t decay; // milliseconds to take to fade from sustainColor to decayColor (0 - 65536)
  uint8_t loops; // how many times should the animation be repeated? 1 = only execute once; 0 = loop forever; n>1 = loop n number of times
} animation_t;

typedef struct {
  animation_t animation; // definition of animation
  pixelColor_t currentColor; // current color to display
  uint8_t loopCount; // keep track of how many times we've looped the animation
  uint32_t lastFrameTime; // record each time a new frame has been computed
  uint8_t state;  // remember which state (attack, sustain, decay, finished) the animation is in
} animationState_t;

animationState_t animationGroups[6]; // our animation groups

const int buttonPins[5] = {A0, A1, A2, A3, A4}; // Port Pins the buttons are attached to

void setup() {
  Serial.begin(115200);

  // initialize neopixel (WS281x) strip
  pixels.begin();
  pixels.show();

  // declare button pins as inputs
  for (uint8_t i = 0; i < sizeof(buttonPins); i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // make sure all animations are in the off state
  animation_t anim;
  pixelColor_t color = {0, 0, 0};
  for (uint8_t i = 0; i < 6; i++) {
    animationGroups[i] = {anim, color, 0, millis(), STATE_FINISHED};
  }
}

void loop() {
  checkButtons(); // evaluate button presses
  checkAnimations(); // advance playing animations
  updateLeds(); // output current state to LEDs
  //windowsConstant();

  // debugging
  if (Serial.available()) {
    String cmd = Serial.readStringUntil("\n");
    uint8_t id = cmd.toInt();
    Serial.println(id);
    buttonPressed(id);
  }
}

/**
   See if any buttons are pressed
*/
void checkButtons() {
  for (uint8_t i = 0; i < sizeof(buttonPins); i++) {
    if (digitalRead(buttonPins[i]) == 0) {
      uint32_t startTime = millis();
      while (digitalRead(buttonPins[i]) == 0 && millis() - startTime < 25) {} // wait for button to be depressed; but never longer than 25ms
      if (millis() - startTime < 10) { // debounce; key has to be pressed at least 10ms or it doesn't count
        continue;
      }
      buttonPressed(i); // execute button press action
    }
  }
}

/**
   Trigger animations on button press
*/
/*void windowsConstant() {
 pixelColor_t attackColor, sustainColor, releaseColor, decayColor;
  animation_t anim;
  uint16_t attack, sustain, release, decay;
   attack = 100;
      attackColor = {155, 155, 155};
      sustain = 300;
      sustainColor = {0 ,0, 255};
      release = 100;
      releaseColor = {255, 255, 255};
      decay = 100;
      decayColor = {0, 0, 0};
      anim = {attackColor, sustainColor, releaseColor, decayColor, attack, sustain, release, decay, 0};
      animationGroups[4].animation = anim;
      animationGroups[4].loopCount = 1;
      animationGroups[4].state = STATE_ATTACK;
      animationGroups[4].lastFrameTime = millis();
      if (animationGroups[4].state == STATE_FINISHED) { // start animation if not playing
        anim = {attackColor, sustainColor, releaseColor, decayColor, attack, sustain, release, decay, 0};

        animationGroups[4].animation = anim;
        animationGroups[4].loopCount = 0;
        animationGroups[4].state = STATE_ATTACK;
        animationGroups[4].lastFrameTime = millis();

        animationGroups[4].animation = anim;
        animationGroups[4].loopCount = 0;
        animationGroups[4].state = STATE_ATTACK;
        animationGroups[4].lastFrameTime = millis();
      } else { // stop animation if playing by setting max loops to 1 (ensuring proper decay ending of animation)
        animationGroups[4].animation.loops = 0;
   
      }
}*/

void buttonPressed(uint8_t buttonIndex) {
  pixelColor_t attackColor, sustainColor, releaseColor, decayColor;
  animation_t anim;
  uint16_t attack, sustain, release, decay;
  switch (buttonIndex) {
    case 0: // release group 0 in red 3 times
      attack = 500;
      attackColor = {0, 0, 0};
      sustain = 30;
      sustainColor = {100, 0, 0};
      release = 0;
      releaseColor = {255, 255, 255};
      decay = 30;
      decayColor = {0, 0, 0};
      anim = {attackColor, sustainColor, releaseColor, decayColor, attack, sustain, release, decay, 3};
      animationGroups[0].animation = anim;
      animationGroups[0].loopCount = 0;
      animationGroups[0].state = STATE_ATTACK;
      animationGroups[0].lastFrameTime = millis();
      break;
    case 1: // toggle fade group 1 + 2
      attack = 3000;
      sustain = 0;
      release = 0;
      decay = 1500;
      attackColor = {255, 0, 0};
      sustainColor = {0, 255, 255};
      releaseColor = {0, 255, 255};
      decayColor = {255, 0, 0};

      if (animationGroups[1].state == STATE_FINISHED) { // start animation if not playing
        anim = {attackColor, sustainColor, releaseColor, decayColor, attack, sustain, release, decay, 0};

        animationGroups[1].animation = anim;
        animationGroups[1].loopCount = 0;
        animationGroups[1].state = STATE_ATTACK;
        animationGroups[1].lastFrameTime = millis();

        animationGroups[2].animation = anim;
        animationGroups[2].loopCount = 0;
        animationGroups[2].state = STATE_ATTACK;
        animationGroups[2].lastFrameTime = millis();
      } else { // stop animation if playing by setting max loops to 1 (ensuring proper decay ending of animation)
        animationGroups[1].animation.loops = 1;
        animationGroups[2].animation.loops = 1;
      }
      break;

      case 2: // Windows
      attack = 3000;
      sustain = 0;
      release = 0;
      decay = 1500;
      attackColor = {255, 0, 0};
      sustainColor = {0, 255, 255};
      releaseColor = {0, 255, 255};
      decayColor = {255, 0, 0};

      if (animationGroups[4].state == STATE_FINISHED) { // start animation if not playing
        anim = {attackColor, sustainColor, releaseColor, decayColor, attack, sustain, release, decay, 0};

        animationGroups[4].animation = anim;
        animationGroups[4].loopCount = 0;
        animationGroups[4].state = STATE_ATTACK;
        animationGroups[4].lastFrameTime = millis();

      } else { // stop animation if playing by setting max loops to 1 (ensuring proper decay ending of animation)
        animationGroups[4].animation.loops = 1;
      }
      break;

  }
}

/**
   Check if any animations are playing and need updating
*/
void checkAnimations() {
  for (uint8_t i = 0; i < 6; i++) {
    if (animationGroups[i].state != STATE_FINISHED) {
      updateAnimation(i);
    }
  }
}

/**
   Animation state machine
*/
void updateAnimation(uint8_t animIndex) {
  uint8_t progress;
  if (animationGroups[animIndex].state == STATE_ATTACK) {
    float attackTime = (float) animationGroups[animIndex].animation.attack;
    float attackStep = attackTime / 255.0f;
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / attackStep, 0, 255);
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.attackColor, animationGroups[animIndex].animation.sustainColor, progress);
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.attack) {
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_SUSTAIN;
      animationGroups[animIndex].currentColor = animationGroups[animIndex].animation.sustainColor;
    }
  } else if (animationGroups[animIndex].state == STATE_SUSTAIN) {
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.sustain) {
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_RELEASE;
    }
  } else if (animationGroups[animIndex].state == STATE_RELEASE) {
    float releaseTime = (float) animationGroups[animIndex].animation.release;
    float releaseStep = releaseTime / 255.0f;
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / releaseStep, 0, 255);
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.releaseColor, animationGroups[animIndex].animation.sustainColor, progress);
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.attack) {
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_DECAY;
      animationGroups[animIndex].currentColor = animationGroups[animIndex].animation.releaseColor;
    }
  } else if (animationGroups[animIndex].state == STATE_DECAY) {
    float decayTime = (float) animationGroups[animIndex].animation.decay;
    float decayStep = decayTime / 255.0f;
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / decayStep, 0, 255);
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.releaseColor, animationGroups[animIndex].animation.decayColor, progress);
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.decay) {
      animationGroups[animIndex].lastFrameTime = millis();
      if (
        (
          animationGroups[animIndex].animation.loops > 1
          && animationGroups[animIndex].loopCount < animationGroups[animIndex].animation.loops
        )
        || animationGroups[animIndex].animation.loops == 0
      )
      {
        animationGroups[animIndex].state = STATE_ATTACK;
        animationGroups[animIndex].loopCount++;
      } else {
        Serial.println("finished");
        animationGroups[animIndex].state = STATE_FINISHED;
      }
    }
  }
}

/*
   Interpolate between <startColor> and <endColor> in 255 steps; current step is defined by <progress>
*/
pixelColor_t linearInterpolation(pixelColor_t startColor, pixelColor_t endColor, uint8_t progress) {
  float stepSizeR = constrain(endColor.r - startColor.r, -255.0f, 255.0f) / 255.0f;
  float stepSizeG = constrain(endColor.g - startColor.g, -255.0f, 255.0f) / 255.0f;
  float stepSizeB = constrain(endColor.b - startColor.b, -255.0f, 255.0f) / 255.0f;

  int16_t dr = stepSizeR * progress;
  int16_t dg = stepSizeG *  progress;
  int16_t db = stepSizeB * progress;

  pixelColor_t newColor = {
    constrain(startColor.r + dr, 0, 255),
    constrain(startColor.g + dg, 0, 255),
    constrain(startColor.b + db, 0, 255)
  };

  return newColor;
}

/**
   Apply animations (ie colors) to the LEDs
*/
void updateLeds() {
  // manual mapping of anim groups to LEDs required
  for (uint8_t i = 0; i < 6; i++) {
    pixels.setPixelColor(i, pixels.Color(animationGroups[i].currentColor.r, animationGroups[i].currentColor.g, animationGroups[i].currentColor.b));
  }
  pixels.show();
}
