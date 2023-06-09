/* GAME NOTES

Maybe add:
- sound component for killing enemy and finishing round

To fix:
- lives (especially first one depleting instantly due to 500ms.isExpired() being true from setup());
- round finish light and restart is buggy
- change async delay time to correspond to round (fucks a lotta shit up);

*/

#include <Adafruit_CircuitPlayground.h>
#include <AsyncDelay.h>

//system global varibles
AsyncDelay delay_500ms;
AsyncDelay delay_1sBlue;
AsyncDelay delay_1sRed;
AsyncDelay playerCursorDelay;
bool switchFlag = 0;
bool switchState = 0;
bool LButtonFlag = 0;
bool RButtonFlag = 0;
float x, y, nx, ny, angle; 
int numLED = 9;
const int brightness = 5;
float midi[127];
int A_four = 440;

//ease of coding variables
bool sound = false;
bool invincible = true;

// logic/game variables
bool inRound = true;
bool onTarget = false;
int enemyBrightness = 0;
int enemyPixel = 6;
int playerPixel = 0;
int red = 0xff0000;
int blue = 0x0000ff;

//gameplay variables
int enemyDelay = 200; //200ms enemy spawn delay. -10 for each round
int roundsCompleted = 0;
int redOrBlue = red; //red
int lives = 3;
int enemysAlive = 10;

void setup() {
  Serial.begin(9600);
  CircuitPlayground.begin();
  pinMode(LED_BUILTIN, OUTPUT); //initializes game on off light
  CircuitPlayground.setBrightness(brightness);
  CircuitPlayground.clearPixels();

  pinMode(7, INPUT_PULLUP); //initalizes switch
  pinMode(5, INPUT_PULLDOWN); //initalizes R button
  pinMode(4, INPUT_PULLDOWN); //initalizes L button
  attachInterrupt(digitalPinToInterrupt(7), onOffSwitch, CHANGE); //initalizes switch interrupt

  attachInterrupt(digitalPinToInterrupt(5), leftOnOffButton, CHANGE); //initalizes L interrupt
  attachInterrupt(digitalPinToInterrupt(4), rightOnOffButton, CHANGE); //initalizes R interrupt

  delay_500ms.start(500, AsyncDelay::MILLIS); // initializes 0.5s second polling timer
  delay_1sBlue.start(1000, AsyncDelay::MILLIS); //blue gun cooldown timer
  delay_1sRed.start(1000, AsyncDelay::MILLIS); //red gun cooldown timer
  playerCursorDelay.start(50, AsyncDelay::MILLIS);
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G); //what mean??
  generateMIDI();

}

void loop() {
  if (switchFlag == 1) {
    delay(5);
    switchState = digitalRead(7);
    switchFlag = 0;
  }

  //if switch on small red light turns on
  if (switchState == 1) {
    digitalWrite(LED_BUILTIN, HIGH);

    
    if (inRound == true && lives != 0 && enemysAlive > 0) {
      
      //red and blue bullet cooldowns using interrrupts
      if (LButtonFlag && delay_1sBlue.isExpired()) { 
        delay(200);
        if (redOrBlue == blue && onTarget) {
          enemyBrightness = 0;
          enemysAlive--;
          generateEnemy();
          if (sound) CircuitPlayground.playTone(midi[95], 200); //hit sound effect
        }
        else {
          delay_1sBlue.repeat();
          if (sound) CircuitPlayground.playTone(midi[40], 200); //miss sound effect
        }
        LButtonFlag = 0;
      }
      if (RButtonFlag && delay_1sRed.isExpired()) {
        delay(200);
        if (redOrBlue == red && onTarget) {
          enemyBrightness = 0;
          enemysAlive--;
          generateEnemy();
          if (sound) CircuitPlayground.playTone(midi[95], 200); //hit sound effect
        }
        else {
          delay_1sRed.repeat();
          if (sound) CircuitPlayground.playTone(midi[40], 200); //miss sound effect
        }
        RButtonFlag = 0;
      }

      if (delay_500ms.isExpired()) {
        if (enemyBrightness >= 100) {
          //testing
          generateEnemy();

          if (!invincible) lives--;
          CircuitPlayground.clearPixels();
          enemyBrightness = 0;
        }
        CircuitPlayground.setBrightness(enemyBrightness);
        enemyBrightness += 10;
        CircuitPlayground.setPixelColor(enemyPixel, redOrBlue);
        delay_500ms.repeat();

        //delay(50);
        
        Serial.print("Enemy proximity: ");
        Serial.print(enemyBrightness);
        Serial.print(" lives: ");
        Serial.print(lives);
        Serial.print(" Enemys alive: ");
        Serial.print(enemysAlive);
        Serial.print(" Enemy color: ");
          if (redOrBlue == red) Serial.print("red");
          else Serial.print("blue");
        Serial.print(" Enemy Pixel: ");
        Serial.print(enemyPixel);
        Serial.print(" Player Pixel: ");
        Serial.print(playerPixel);
        Serial.print(" Rounds completed: ");
        Serial.println(roundsCompleted);
        
      }
      
      
    }
    else if (lives <= 0) { //player is dead because lives = 0
      inRound = false;
      for(int i = 0; i <= numLED; i++) {
        CircuitPlayground.setPixelColor(i, red); //red
      }
      delay(5000);
      roundsCompleted = 0;
      lives = 4;
    }

    else if (enemysAlive <= 0) { //round over and display levels completed
      inRound = false;
      enemysAlive = 10;
      roundsCompleted++;
      roundLEDColor(roundsCompleted);
      for (int i = 0; i <= numLED; i++) {
        CircuitPlayground.setPixelColor(i, 0xffffff);
      }
      delay(2000);
      CircuitPlayground.clearPixels();
      if (roundsCompleted == 25) beatGame();
      else inRound = true;
      delay(5000);
    }

    if(playerCursorDelay.isExpired() && inRound) {
      playerPixel = calculateAngle() / 36;
      if (playerPixel == enemyPixel) {
        CircuitPlayground.setPixelColor(playerPixel, 0x00ff00);
        onTarget = true;
      }
      else {
        CircuitPlayground.setPixelColor(playerPixel, 0xffffff); //white
        onTarget = false;
      }
      for (int i = 0; i <= numLED; i++) {
        if (i != playerPixel && i != enemyPixel) {
          CircuitPlayground.setPixelColor(i, 0x000000);
        }
      }
      playerCursorDelay.repeat();
    }

  }

  else if (switchState == 0) { 
    digitalWrite(LED_BUILTIN, LOW);
    CircuitPlayground.clearPixels();
    resetGame();
  }

}
//system variables
void resetGame() {
  LButtonFlag = 0;
  RButtonFlag = 0;

  // logic/game variables
  inRound = true;
  onTarget = false;
  enemyBrightness = 0;
  enemyPixel = 0;
  playerPixel = 0;

  //gameplay variables
  roundsCompleted = 0;
  redOrBlue = red; //red
  lives = 3;
  enemysAlive = 10;
}

int calculateAngle() {
  x = CircuitPlayground.motionX(); // Get the CP accelerometer x and y positions  
  y = CircuitPlayground.motionY(); // (we ignore the z axis for this one)
  nx = x / 10.0;
  ny = y / 10.0;
  angle = atan((ny/nx)) * 180 / 3.14; // Figure out the angle of the accelerometer
  if(angle > 0.0) { // Adjust based on arctangent function (in degrees)
    if(nx < 0.0)
      angle += 180;
  }
  else
  { 
    if(ny > 0.0)
      angle += 180;
    else
      angle += 360;
  }
  if(angle == 360.0) { // a 360 degree angle is the same as a zero degree angle  angle = 0;
    angle = 0;
  }
  return angle;
}

void roundLEDColor(int roundsCompleted) {

  int color = 0xFFFFFF;
  if (roundsCompleted < 10) { //rounds 0 -9 and white
    color = 0xffffff;
  }
  else if (roundsCompleted < 20) { //rounds 10 - 19 and yellow
    color = 0xfff200;
  }
  else if (roundsCompleted < 25) { //rounds 20 -24 and red LEDS for round end
    color = red;
  }

  for (int i = 0; i <= numLED; i++) { //sets leds to round # color
    if (i >= roundsCompleted % 10) {
      CircuitPlayground.setPixelColor(i, color);
    }
  }
  for (int i = 0; i <= numLED; i++) { //sets leds green depending on how many levels beat
      if (i < roundsCompleted % 10) {
        CircuitPlayground.setPixelColor(i, 0x09ff00); //green
        delay(100);
      }
  }

}

void beatGame() {
  for (int i = 0; i <= numLED; i++) {
      CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / 10) + 10) & 255));
    }
    delay(10000);
    //rainbowCycle(50, 10);
    //roundsCompleted = 0;
}

void generateEnemy() {
  enemyPixel = random(0, 10);
  if(random(2) == 1) redOrBlue = 0xff0000; //red
  else redOrBlue = 0x0000ff; //blue
}

void rainbowCycle(int currentSpeed, int stripLen) {

  // Make an offset based on the current millisecond count scaled by the current speed.
  CircuitPlayground.setBrightness(30);
  uint32_t offset = millis() / currentSpeed;

  // Loop through each pixel and set it to an incremental color wheel value.

  for(int i = 0; i <= numLED; i++) {

    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / stripLen) + offset) & 255));

  }

}

void generateMIDI() {
  for (int x = 0; x < 127; ++x)  {
    midi[x] = (A_four / 32.0) * pow(2.0, ((x - 9.0) / 12.0));
    Serial.println(midi[x]);
  }
}


void onOffSwitch() {
  switchFlag = 1;
}
void leftOnOffButton() {
  LButtonFlag = 1;
}
void rightOnOffButton() {
  RButtonFlag = 1;
}
