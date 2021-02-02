#include <Adafruit_NeoPixel.h>
 
#define PIXEL_PIN 12
#define NUM_LEDS 10
 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
 
enum {
  STATE_ATTACK,
  STATE_SUSTAIN,
  STATE_DECAY,
  STATE_RELEASE,
  STATE_LOOP_WAIT,
  STATE_FINISHED
};
 
typedef struct {
  uint8_t r, g, b;
} pixelColor_t;
 
typedef struct {
  pixelColor_t attackColor; // the color the animation starts at
  pixelColor_t sustainColor; // the color the animation peaks at
  pixelColor_t decayColor; // the color the animation decays to
  pixelColor_t releaseColor; // the color the animation ends at
  uint16_t attack; // milliseconds to take to fade from attackColor to sustainColor (0 - 65536)
  uint16_t sustain; // milliseconds to sustain the sustainColor (0 - 65536)
  uint16_t decay; // milliseconds to take to fade from sustainColor to decayColor (0 - 65536)
  uint16_t release; // milliseconds to take to fade from decayColor to releaseColor (0 - 65536)
  uint16_t loopInterval; // milliseconds before starting the next loop
  uint8_t loops; // how many times should the animation be repeated? 1 = only execute once; 0 = loop forever; n>1 = loop n number of times
  uint16_t randomLoopMin; // randomLoopMin and randomLoppMax define the range of random intervals
  uint16_t randomLoopMax;
  bool randomLoopInterval; // if true loopInterval will be set to a random value
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
  pixelColor_t color = {0,0,0};
  for (uint8_t i=0; i < 6; i++) {
    animationGroups[i] = {anim, color, 0, millis(), STATE_FINISHED};

   //Starship Windows
    animationGroups[3].animation.attackColor = {5,5,5};
    animationGroups[3].animation.sustainColor = {5,5,5};
    animationGroups[3].animation.decayColor = {5,5,5};
    animationGroups[3].animation.releaseColor = {5,5,5};
    animationGroups[3].animation.loops = 1;
    animationGroups[3].state = STATE_ATTACK;
    animationGroups[3].currentColor = {5,5,5};
  }
}
 
void loop() {
  
  checkButtons(); // evaluate button presses
  checkAnimations(); // advance playing animations
  updateLeds(); // output current state to LEDs

 
  // debugging
  if(Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    uint8_t id = cmd.toInt();
    Serial.println(id);
    buttonPressed(id);
  }
}
 
/**
 * See if any buttons are pressed
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
 * Trigger animations on button press
 */
void buttonPressed(uint8_t buttonIndex) {
  pixelColor_t attackColor, sustainColor, decayColor, releaseColor;
  animation_t anim;
  uint16_t attack, sustain, decay, release, loopInterval, randomLoopMin, randomLoopMax;
  attack = sustain = decay = release = loopInterval = randomLoopMin = randomLoopMax = 0;
  uint8_t numLoops = 1;
  bool randomLoopInterval = false; // by default don't do random loop intervals
 
  switch (buttonIndex) {
    case 0: // fade group 0 to dim red, then briefly flash to bright red; repeat three times
      attack = 500;
      attackColor = {0,0,0};
     
      sustain = 300;
      sustainColor = {150,0,0};
     
      decay = 30;
      decayColor = {255,255,255};
     
      release = 0;
      releaseColor = {0,0,0};
     
      numLoops = 2; // how many times to repeat the animation
      loopInterval = 100; // 100ms between "shots"
     
      anim = {attackColor, sustainColor, decayColor, releaseColor, attack, sustain, decay, release, loopInterval, numLoops, randomLoopMin, randomLoopMax, randomLoopInterval};
      animationGroups[0].animation = anim;
      animationGroups[0].loopCount = 0;
      animationGroups[0].state = STATE_ATTACK;
      animationGroups[0].lastFrameTime = millis();
      break;
 case 1: // toggle fade group 1 + 2 (example for fading two groups between two colors until the button is pressed again)
      attack = 1000;
      sustain = 0;
      decay = 0;
      release = 0;
      attackColor = {0,5,0};
      sustainColor = {0,5,0};
      decayColor = {0,5,0};
      releaseColor = {0,5,0};
      
      numLoops = 1; // infinite loops
      loopInterval = 0; // no wait between loops

      if (animationGroups[1].state == STATE_FINISHED) { // start animation if not playing

        anim = {attackColor, sustainColor, decayColor, releaseColor, attack, sustain, decay, release, loopInterval, numLoops, randomLoopMin, randomLoopMax, randomLoopInterval};
        
        animationGroups[1].animation = anim;
        animationGroups[1].loopCount = 0;
        animationGroups[1].state = STATE_ATTACK;
        animationGroups[1].lastFrameTime = millis();
  
        animationGroups[2].animation = anim;
        animationGroups[2].loopCount = 0;
        animationGroups[2].state = STATE_ATTACK;
        animationGroups[2].lastFrameTime = millis();
        
      if (animationGroups[1].animation.releaseColor.r == 0 && animationGroups[1].animation.releaseColor.g == 5 && animationGroups[1].animation.releaseColor.b == 0) {
        animationGroups[1].animation.sustainColor = {5,0,0};
        animationGroups[2].animation.sustainColor = {5,0,0};
        animationGroups[1].state == STATE_SUSTAIN;
        animationGroups[2].state == STATE_RELEASE;
      } else if (animationGroups[1].animation.releaseColor.r == 5 && animationGroups[1].animation.releaseColor.g == 0 && animationGroups[1].animation.releaseColor.b == 0)  {
        animationGroups[1].animation.sustainColor ={0,0,5};
        animationGroups[2].animation.sustainColor = {0,0,5};
        }

      } else { // stop animation if playing by setting max loops to 1 (ensuring proper decay ending of animation)
        animationGroups[1].animation.loops = 1;
        animationGroups[2].animation.loops = 1;
      }
    

        
      
   break;
    case 2: // example for random loop intervals
      attack = 10;
      attackColor = {0,0,0};
     
      sustain = 30;
      sustainColor = {128,32,0};
     
      decay = 30;
      decayColor = {255,64,0};
     
      release = 10;
      releaseColor = {0,0,0};
     
      numLoops = 10;
      loopInterval = 100;
      randomLoopInterval = true; // use random intervals between loops
      randomLoopMin = 250; // at least a quarter second between effects
      randomLoopMax = 500; // at most two seconds between effects
     
      anim = {attackColor, sustainColor, decayColor, releaseColor, attack, sustain, decay, release, loopInterval, numLoops, randomLoopMin, randomLoopMax, randomLoopInterval};
      animationGroups[3].animation = anim;
      animationGroups[3].loopCount = 0;
      animationGroups[3].state = STATE_ATTACK;
      animationGroups[3].lastFrameTime = millis();
      
      
      
      break;
  }
}
 
/**
 * Check if any animations are playing and need updating
 */
void checkAnimations() {
  for (uint8_t i = 0; i < 6; i++) {
    if (animationGroups[i].state != STATE_FINISHED) {
      updateAnimation(i);
    }
  }
}
 
/**
 * Animation state machine
 */
void updateAnimation(uint8_t animIndex) {
  uint8_t progress;
  if (animationGroups[animIndex].state == STATE_LOOP_WAIT) { // wait loopInterval ms before starting the next animation loop
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.loopInterval) { // wait time before starting the next loop
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_ATTACK;
    }
   
  } else if (animationGroups[animIndex].state == STATE_ATTACK) { // process attack -> fade between attackColor and sustainColor
    float attackTime = (float) animationGroups[animIndex].animation.attack; // time the animation should take in ms
    float attackStep = attackTime / 255.0f; // progress is from 0 (only startColor) to 255 (only endColor); calculate how many steps per ms are needed
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / attackStep, 0, 255); // calculate progress based on ms elapsed
   
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.attackColor, animationGroups[animIndex].animation.sustainColor, progress); // apply color fade from startColor to endColor based on progress
   
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.attack) { // when attack animation has reached its alottet time switch to sustain
      animationGroups[animIndex].lastFrameTime = millis(); // time keeping
      animationGroups[animIndex].state = STATE_SUSTAIN; // state changing
      animationGroups[animIndex].currentColor = animationGroups[animIndex].animation.sustainColor; // make sure the final color at this state is really the target color
    }
   
  } else if (animationGroups[animIndex].state == STATE_SUSTAIN) { // all we do here is maintain the sustainColor for sustain ms
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.sustain) { // when the time is up change to decay
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_DECAY;
    }
   
  } else if (animationGroups[animIndex].state == STATE_DECAY) { // fade from sustainColor to decayColor in sustain ms
    float decayTime = (float) animationGroups[animIndex].animation.decay;
    float decayStep = decayTime / 255.0f;
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / decayStep, 0, 255);
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.sustainColor, animationGroups[animIndex].animation.decayColor, progress);
   
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.decay) { // when decay animation has finished, switch to release
      animationGroups[animIndex].lastFrameTime = millis();
      animationGroups[animIndex].state = STATE_RELEASE;
      animationGroups[animIndex].currentColor = animationGroups[animIndex].animation.decayColor; // make sure the final color at this state is really the target color
    }
   
  } else if (animationGroups[animIndex].state == STATE_RELEASE) { // fade from decayColor to releaseColor in release ms
    float releaseTime = (float) animationGroups[animIndex].animation.release;
    float releaseStep = releaseTime / 255.0f;
    progress = constrain((millis() - animationGroups[animIndex].lastFrameTime) / releaseStep, 0, 255);
   
    animationGroups[animIndex].currentColor = linearInterpolation(animationGroups[animIndex].animation.decayColor, animationGroups[animIndex].animation.releaseColor, progress);
   
    if (millis() - animationGroups[animIndex].lastFrameTime >= animationGroups[animIndex].animation.release) {
      animationGroups[animIndex].lastFrameTime = millis(); // time keeping
      animationGroups[animIndex].currentColor = animationGroups[animIndex].animation.releaseColor;
     
      if ( // process the loop variables
          (
            animationGroups[animIndex].animation.loops > 1
            && animationGroups[animIndex].loopCount < animationGroups[animIndex].animation.loops
          )
          || animationGroups[animIndex].animation.loops == 0
        )
      {
        animationGroups[animIndex].state = STATE_LOOP_WAIT;
        animationGroups[animIndex].loopCount++;
 
        if (animationGroups[animIndex].animation.randomLoopInterval == true) { // process the random loop stuff
          animationGroups[animIndex].animation.loopInterval = (uint16_t) random(animationGroups[animIndex].animation.randomLoopMin, animationGroups[animIndex].animation.randomLoopMax);
        }
      } else {
        Serial.println("finished");
        animationGroups[animIndex].state = STATE_FINISHED;
      }
    }
  }
}
 
/*
 * Interpolate between <startColor> and <endColor> in 255 steps; current step is defined by <progress>
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
 * Apply animations (ie colors) to the LEDs
 */
void updateLeds() {
  // manual mapping of anim groups to LEDs required
  for (uint8_t i = 0; i < 6; i++) {
    pixels.setPixelColor(i, pixels.Color(animationGroups[i].currentColor.r, animationGroups[i].currentColor.g, animationGroups[i].currentColor.b));
  }
  pixels.show();
}
