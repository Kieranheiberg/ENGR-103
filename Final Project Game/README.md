# A Twist on Space Invaders
Created by Kieran Heiberg for his ENGR 103 final project at Oregon State University. Go beavs!!

## Code Instruction
Game is designed for the Adafruit circuit playground express board and should be run in the Arduino IDE for optimal game experience. The circuit playground express board requires a USB B type cable for power. The AsyncDelay and Adafruit_CircuitPlayground libraries wre utilized for this game and are required for its operation.

## Device Instructions
Game is turned on by switch. The small built in LED will light up red when game is on. Turning switch off and on will cause game to reset (helpful if a player dies and wants to play again). 
The player controls their position relative to the enemy with the devices incorporated accelerometer to track rotation on the xy plane. The device should be held with the USB B port pointing down and the LEDs facing the player. Ensure the LEDs are always facing the player and then spin the device to the left or right to aim the players weapon. The players current position will be indicated by a white colored cursor on one of the LEDS or a green colored cursor if it is lined up with the approaching enemy. 
Left button fires blue bullet.
Red button fires red bullet.
Each color bullet has a 1 sec cooldown before they can be fired again.

## Game Rules:
A single enemy will approach the player from any direction (LED). The enemy starts 100 steps away and gets 10 steps closer every 0.5 seconds. This is indicated by the enemys LED which will become brighter as the enemy advances. To eliminate the enemy use either the left button to fire a blue bullet or the right button to fire the red bullet. You have to match the bullet color to the enemy color in order to eliminate them. If an enemy is hit with the correct color bullet the device will play a high pitched tone. If it is the wrong color bullet or the bullet misses the enemy a low pitched note will play. A player has 3 lives and will lose one if an enemy advances all 100 steps. A loss of life will be signaled by the device playing a 5 note sequence that decreases in pitch. There are a total of 10 enemies per round and a total of 10 rounds. If all ten enemies in a round are eliminated a high pitched 3 note sequence will play to signal the round is over. To increase difficulty the enemies will approach one extra step every round. For example in round 3 an enemy will start 100 steps away and approach 13 steps every 0.5 seconds. If a player loses all 3 lives the game is over and the device will play a very low pitched 3 note sequence and light up all pixels red.

## Notes about code
All important game variables are printed to the Arduino IDE serial monitor during the game for reference and debugging purposes
Sound effect volume is non adjustable and can be quite loud. All game sound effects can be turned of by changing boolean variable sound to false (located on line 23 of "A Twist on Space Invaders.ino" file).
There is a known bug where an enemy will approach at a much faster speed than anticipated following the start of a new round. This will often cause a player to lose a life as the window to eliminate the enemy is much smaller. This bug is due to the program entering the increase enemy brightness portion of the code more than once within the 0.5 second asynchronous delay window. The code was tested and debugged for a while but could not be fixed. A temporary solution was enacted with an invincible boolean variable that makes the player invincible to the first enemy that hits them in a round.

## Code Credit
Acceleration calculations and method were taken and modified from code written by ENGR 103 instructor Chet Udell to create calculateAngle() method. LED loigc for beatGame() method was taken from a sample code posted by Chet Udell as part of the classes kaleidoscope project.
