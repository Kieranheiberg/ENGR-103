/* GAME NOTES    
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
float x, y, nx, ny, angle; //for accelerometer
int numLED = 9;
const int brightness = 5;
float midi[127];
int A_four = 440;

//ease of coding variables
bool sound = true; //enables sound features for game

// logic/game variables
bool inRound = true; 
bool onTarget = false; //variable for if player bullet lines up with enemy pixel
int enemyBrightness = 0;
int enemyPixel = 6;
int playerPixel = 0;
int red = 0xff0000;
int blue = 0x0000ff;

//gameplay variables
int roundsCompleted = 0;
int redOrBlue = red; //determines color of enemy
int deaths = 0;
int lives = 3; 
int enemysAlive = 10; //enemys per round
bool invincible = 0; //invincible feature added due to bug that would immediatly cause a death after starting new round 

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
  playerCursorDelay.start(50, AsyncDelay::MILLIS); //player cursor pixel delay timer
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
  generateMIDI();
}

void loop() {
  if (switchFlag == 1) { //if switch flipped
    delay(5); //debounce switch
    delay_500ms.repeat(); //start enemy approach delay timer
    switchState = digitalRead(7); //save switch state to variable
    switchFlag = 0; //reset switchFlag
  }

  //if switch on small red light turns on
  if (switchState == 1) {
    digitalWrite(LED_BUILTIN, HIGH);

    
    if (inRound == true && lives != deaths && enemysAlive > 0) { //if in a round and not dead and havnt killed all 10 enemys in a round
      
      //red and blue bullet cooldowns using interrrupts
      if (LButtonFlag && delay_1sBlue.isExpired()) { 
        delay(200); //debounce for button press
        if (redOrBlue == blue && onTarget) { // if enemy is blue and cursor is on taget with enemy location
          enemyBrightness = 0;
          enemysAlive--; 
          generateEnemy(); //creates blue or red enemy at random pixel between 0 and 9
          if (sound) CircuitPlayground.playTone(midi[65], 200); //hit sound effect
        }
        else {
          delay_1sBlue.repeat(); //restart blue bullet cooldown
          if (sound) CircuitPlayground.playTone(midi[40], 200); //miss sound effect
        }
        LButtonFlag = 0; //reset blue button flag
      }
      if (RButtonFlag && delay_1sRed.isExpired()) {
        delay(200); //debounce button
        if (redOrBlue == red && onTarget) { //if cursor matches red enemy
          enemyBrightness = 0;
          enemysAlive--; //kill an enemy
          generateEnemy();
          if (sound) CircuitPlayground.playTone(midi[65], 200); //hit sound effect
        }
        else {
          delay_1sRed.repeat();
          if (sound) CircuitPlayground.playTone(midi[40], 200); //miss sound effect
        }
        RButtonFlag = 0; //reset red button flag
      }



      if (delay_500ms.isExpired()) { //500ms asynch timer for enemy approach speed 

        if (enemyBrightness >= 100) { //enemy starts at 0 brightness and increases to 100 brightness. If enemy at 100 brightness then player dies
          generateEnemy(); 

          if (!invincible) deaths++; //invincible loop to prevent instant death after round starts
          CircuitPlayground.clearPixels(); 
          if (sound && !invincible) {
            for (int i = 0; i < 5; i++) {
              CircuitPlayground.playTone(midi[40 - i*2], 50);
              delay(20);
            }
          }

          enemyBrightness = 0;
          invincible = false; //resets invincibility so player can be damaged throughout round
        }
        delay_500ms.repeat(); //restarts enemy approch timer
        if (!invincible) { //prevents instant loss of life once round starts
          CircuitPlayground.setBrightness(enemyBrightness); //sets brightness depending on enemy proximity to player
          CircuitPlayground.setPixelColor(enemyPixel, redOrBlue);
        }
        
        enemyBrightness += (10 + roundsCompleted); //increment enemy brightness by defualt of 10 brightness plus # of rounds completed to increase difficulty as rounds progress

        //prints out variables for debug purposes
        Serial.print("Enemy proximity: ");
        Serial.print(enemyBrightness);
        Serial.print(" deaths: ");
        Serial.print(deaths);
        Serial.print(" Enemys alive: ");
        Serial.print(enemysAlive);
        Serial.print(" Enemy color: ");
          if (redOrBlue == red) Serial.print("red"); //prints enemy color depending in global variable
          else Serial.print("blue");
        Serial.print(" Enemy Pixel: ");
        Serial.print(enemyPixel);
        Serial.print(" Player Pixel: ");
        Serial.print(playerPixel);
        Serial.print(" Rounds completed: ");
        Serial.println(roundsCompleted);
        
      }
      
      
    }

    else if (enemysAlive <= 0) { //round over and display levels completed
      if (sound) {
        CircuitPlayground.playTone(midi[60], 200);
        delay(50);
        CircuitPlayground.playTone(midi[65], 200);
        delay(50);
        CircuitPlayground.playTone(midi[70], 500);
      }
      inRound = false;
      enemysAlive = 10; //resets enemy count
      roundsCompleted++; //increments rounds
      Serial.print("Rounds Completed: ");
      Serial.println(roundsCompleted);
      roundLEDColor(roundsCompleted); //method that sets LED green if corresponding level been beaten and white otherwise and then flashes white to show next level starting
      delay(2000); //2 sec delay to allow player time to see lveels completed
      CircuitPlayground.clearPixels(); 
      if (roundsCompleted == 10) beatGame(); //if roundsCompleted = 10 then beaten game and sets LEDs to rainbow sequence
      else inRound = true; 
      enemyBrightness = 0; 
      invincible = true; 
    }

    if (deaths == lives) { //player is dead because deaths == lives (3 lives) 
      inRound = false; //prevents new round from strating
      CircuitPlayground.clearPixels();
      delay(500);
      CircuitPlayground.setBrightness(20);
      for(int i = 0; i <= numLED; i++) {
        CircuitPlayground.setPixelColor(i, red); //sets all pixels red to reflect death
      }
      Serial.println("You're Dead");
      if (sound) {
        CircuitPlayground.playTone(midi[45], 500);
        delay(100);
        CircuitPlayground.playTone(midi[40], 500);
        delay(100);
        CircuitPlayground.playTone(midi[35], 1000);
        delay(100);
      }
      deaths++; //prevents from reentering death method
    }
    
    //sets playerPixel white or green if playerPixel == enemyPixel
    if(playerCursorDelay.isExpired() && inRound && !invincible) { //playerCursorDelay increases accuracy and precision of players cursor
      playerPixel = calculateAngle() / 36;
      if (playerPixel == enemyPixel) {
        CircuitPlayground.setPixelColor(playerPixel, 0x00ff00); //green cursor color
        onTarget = true;
      }
      else { 
        CircuitPlayground.setPixelColor(playerPixel, 0xffffff); //white cursor color
        onTarget = false;
      }
      for (int i = 0; i <= numLED; i++) { //ensures previous playerPixel turns off 
        if (i != playerPixel && i != enemyPixel) {
          CircuitPlayground.setPixelColor(i, 0x000000); //no color
        }
      }
      playerCursorDelay.repeat(); //restarts cursor timer
    }

  }

  else { //if switch off turns off pixels and resets game
    digitalWrite(LED_BUILTIN, LOW);
    resetGame();
    CircuitPlayground.clearPixels();
  }

}

void resetGame() { //resets all game variables to value at game start
  LButtonFlag = 0;
  RButtonFlag = 0;

  // logic/game variables
  inRound = true;
  onTarget = false;
  enemyBrightness = 0;
  CircuitPlayground.setBrightness(enemyBrightness);
  enemyPixel = 0;
  playerPixel = 0;

  //gameplay variables
  roundsCompleted = 0;
  redOrBlue = red; //red
  deaths = 0;
  enemysAlive = 10;

  //timers
  delay_500ms.start(500, AsyncDelay::MILLIS); // initializes 0.5s second polling timer
}

int calculateAngle() {
  x = CircuitPlayground.motionX(); // Get the CP accelerometer x and y positions  
  y = CircuitPlayground.motionY(); 
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

void roundLEDColor(int roundsCompleted) { //shows level progression visually through green or white LEDs

  int color = 0xFFFFFF; //white;
  CircuitPlayground.setBrightness(20);
  Serial.print("roundNum: ");
  Serial.println(roundsCompleted);
  for (int i = 0; i <= numLED; i++) { //if round i completed appears green else it is white
    if (i < roundsCompleted) {
        CircuitPlayground.setPixelColor(i, 0x00ff00); //green
    }
    else {
      CircuitPlayground.setPixelColor(i, color);
    }
  }
  delay(1000);
  CircuitPlayground.clearPixels();
  delay(1000);
  for(int i = 0; i <= numLED; i++) { //flashes all pixels white for 1sec to indicate next level starting
    CircuitPlayground.setPixelColor(i, color); 
  }
  delay(1000);
  CircuitPlayground.clearPixels();
}

void beatGame() { //method that makes rainbow LED pattern when player beats game
  for (int i = 0; i <= numLED; i++) {
      CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / 10) + 10) & 255));
    }
    delay(10000);
}

void generateEnemy() { //creates blue or red enemy at random pixel 
  enemyPixel = random(0, 10);
  if(random(2) == 1) redOrBlue = 0xff0000; //red
  else redOrBlue = 0x0000ff; //blue
}

void generateMIDI() { //converts MIDI to frequency
  for (int x = 0; x < 127; ++x)  {
    midi[x] = (A_four / 32.0) * pow(2.0, ((x - 9.0) / 12.0));
    Serial.println(midi[x]);
  }
}

//changes button or switch flags based on interrupts
void onOffSwitch() {
  switchFlag = 1;
}
void leftOnOffButton() {
  LButtonFlag = 1;
}
void rightOnOffButton() {
  RButtonFlag = 1;
}
