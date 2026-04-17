/*
===========================================================
 Project Title:
 Arduino Snake Game (32x8 MAX7219 LED Matrix)

 YouTube: https://www.youtube.com/channel/UCpfAZXTqOsfymN9km-hLfFQ

 Description:
 A beginner-friendly Snake Game using a 32x8 LED Matrix.
 This project uses dual rendering:
 - MD_Parola for text display
 - MD_MAX72XX for game rendering (NO ghosting)

 Features:
 - Start screen (loop until button pressed)
 - Smooth snake movement
 - Score system
 - Game over screen with score loop
 - Restart using joystick button
 - Sound effects (move, eat, game over, start)
 - Clean rendering (no ghosting)

===========================================================

================= COMPONENT LIST =================
1. Arduino UNO / Nano
2. 32x8 LED Matrix (MAX7219, 4 modules)
3. Joystick Module (VRX, VRY, SW)
4. Buzzer (Active/Passive)
5. Jumper Wires
6. Breadboard (optional)

================= WIRING =================

--- MAX7219 LED MATRIX ---
VCC  -> 5V
GND  -> GND
DIN  -> D11 (MOSI)
CLK  -> D13 (SCK)
CS   -> D10

--- JOYSTICK MODULE ---
VCC  -> 5V
GND  -> GND
VRX  -> A0
VRY  -> A1
SW   -> D2 (INPUT_PULLUP)

--- BUZZER ---
(+)  -> D3
(-)  -> GND

Notes:
- If joystick direction feels reversed:
  adjust invertX / invertY values
===========================================================
*/

// ================= LIBRARIES =================
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// ================= MATRIX =================
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 10

MD_Parola mx = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// ================= PIN CONFIG =================
#define JOY_X A0
#define JOY_Y A1
#define JOY_SW 2
#define BUZZER 3

// ================= GRID =================
const int WIDTH  = 32;
const int HEIGHT = 8;
const int MAX_LEN = 100;

// ================= SNAKE =================
int snakeX[MAX_LEN];
int snakeY[MAX_LEN];
int length = 3;

int dirX = 1, dirY = 0;
int nextDirX = 1, nextDirY = 0;

int foodX, foodY;
bool gameOver = false;

// ================= SCORE =================
int score = 0;

// ================= CONTROL =================
int invertX = 1;
int invertY = -1;

// ================= TIMING =================
unsigned long lastMove = 0;
int speedDelay = 150;

// ===================================================
void setup() {
  mx.begin();
  mx.setIntensity(3);

  pinMode(BUZZER, OUTPUT);
  pinMode(JOY_SW, INPUT_PULLUP);

  randomSeed(analogRead(0));

  // TEXT MODE
  mx.displaySuspend(false);
  showStartScreen();
  playStartSound();

  // GAME MODE
  mx.displaySuspend(true);
  resetGame();
}

// ===================================================
void loop() {
  if (gameOver) {
    mx.displaySuspend(false);
    showGameOver();

    mx.displaySuspend(true);
    resetGame();
    return;
  }

  readJoystick();

  if (millis() - lastMove > speedDelay) {
    updateSnake();
    lastMove = millis();
  }

  draw();
}

// ===================================================
void showStartScreen() {
  mx.displayClear();

  mx.displayText("SNAKE GAME", PA_CENTER, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!mx.displayAnimate());

  mx.displayText("PRESS START", PA_CENTER, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  while (true) {
    if (mx.displayAnimate()) mx.displayReset();

    if (digitalRead(JOY_SW) == LOW) {
      delay(200);
      break;
    }
  }
}

// ===================================================
void resetGame() {
  length = 3;
  score = 0;

  int mid = WIDTH / 2;

  snakeX[0] = mid;
  snakeY[0] = 4;

  snakeX[1] = mid - 1;
  snakeY[1] = 4;

  snakeX[2] = mid - 2;
  snakeY[2] = 4;

  dirX = 1; dirY = 0;
  nextDirX = 1; nextDirY = 0;

  spawnFood();
  gameOver = false;
}

// ===================================================
void spawnFood() {
  foodX = random(0, WIDTH);
  foodY = random(0, HEIGHT);
}

// ===================================================
void readJoystick() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  if (x < 350 && dirX == 0) {
    nextDirX = -1 * invertX;
    nextDirY = 0;
  } else if (x > 650 && dirX == 0) {
    nextDirX = 1 * invertX;
    nextDirY = 0;
  }

  if (y < 350 && dirY == 0) {
    nextDirX = 0;
    nextDirY = -1 * invertY;
  } else if (y > 650 && dirY == 0) {
    nextDirX = 0;
    nextDirY = 1 * invertY;
  }
}

// ===================================================
void playMoveSound() {
  tone(BUZZER, 1200, 5);
}

void playEatSound() {
  tone(BUZZER, 1000, 80);
  delay(80);
  tone(BUZZER, 1400, 100);
}

void playGameOverSound() {
  int notes[] = {1000, 800, 600, 400, 300};

  for (int i = 0; i < 5; i++) {
    tone(BUZZER, notes[i], 120);
    delay(130);
  }
}

void playStartSound() {
  int notes[] = {800, 1200, 1600};

  for (int i = 0; i < 3; i++) {
    tone(BUZZER, notes[i], 80);
    delay(100);
  }
}

// ===================================================
void updateSnake() {
  playMoveSound();

  dirX = nextDirX;
  dirY = nextDirY;

  int newX = snakeX[0] + dirX;
  int newY = snakeY[0] + dirY;

  if (newX < 0) newX = WIDTH - 1;
  if (newX >= WIDTH) newX = 0;
  if (newY < 0) newY = HEIGHT - 1;
  if (newY >= HEIGHT) newY = 0;

  for (int i = 0; i < length - 1; i++) {
    if (snakeX[i] == newX && snakeY[i] == newY) {
      gameOver = true;
      return;
    }
  }

  for (int i = length; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  snakeX[0] = newX;
  snakeY[0] = newY;

  if (newX == foodX && newY == foodY) {
    length++;
    score++;
    playEatSound();
    spawnFood();
  }
}

// ===================================================
void draw() {
  MD_MAX72XX *mxg = mx.getGraphicObject();

  mxg->clear();

  for (int i = 0; i < length; i++) {
    mxg->setPoint(snakeY[i], snakeX[i], true);
  }

  mxg->setPoint(foodY, foodX, true);

  mxg->update(); // clean render
}

// ===================================================
void showGameOver() {
  playGameOverSound();

  mx.displayClear();

  mx.displayText("GAME OVER", PA_CENTER, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!mx.displayAnimate());

  char scoreText[20];
  sprintf(scoreText, "SCORE %d", score);

  mx.displayText(scoreText, PA_CENTER, 50, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  while (true) {
    if (mx.displayAnimate()) mx.displayReset();

    if (digitalRead(JOY_SW) == LOW) {
      delay(200);
      break;
    }
  }
}
