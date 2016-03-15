#include <stdarg.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define BLPIN 8
#define POTLPIN 2
#define POTPIN 0
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

// missing sprinf
char* p(char *fmt, ... ) {
  char buf[128];
  va_list args;
  va_start (args, fmt );
  vsnprintf(buf, 128, fmt, args);
  va_end (args);
  return buf;
}

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

void setup() {
  // set backlight
  pinMode(BLPIN, OUTPUT);
  digitalWrite(BLPIN, HIGH);

  // set pot led
  pinMode(POTLPIN, OUTPUT);

  // turn on display
  display.begin();

  // set contrast
  display.setContrast(56);

  resetGame();
}

int level = 0;
int hits = 0;
int gameSpeed = 100;
int headerH = 8;
unsigned long previousMillis = 0;
int pot;

// ball
int ballD = 2;
int ballX = 0;
int ballY = 0;
bool ballXD = false;
bool ballYD = false;

// paddle
int paddleX = 0;
int paddleW;
int paddleH = 3;
int paddleWMin = 10;

void resetGame() {
  display.clearDisplay();
  drawText(0, 0, "Get ready ...");
  delay(1500);

  level = 1;
  hits = 0;
  ballX = display.width() / 2;
  ballY = display.height() / 2;
  ballXD = false;
  ballYD = false;
  paddleW = 20;

  digitalWrite(POTLPIN, HIGH);
}

void hit() {
  hits++;

  if (hits % 5 == 0) {
    if (paddleW > paddleWMin) {
      paddleW -= 2;
    }

    level++;
  }
}

void gameOver() {
  display.clearDisplay();
  drawPaddle();
  digitalWrite(POTLPIN, LOW);
  drawText(0, 0, "Game Over :(");
  delay(3000);
}

void drawHeader() {
  drawText(58, 0, "lvl");
  drawText(display.width() - 7, 0, p("%d", level));
  drawText(0, 0, p("%d", hits));
  //display.fillRect(0, headerH, display.width(), 1, BLACK);
}

void drawBall() {
  display.fillCircle(ballX, ballY, ballD, BLACK);
}

void drawPaddle() {
  display.fillRoundRect(paddleX, display.height() - paddleH, paddleW, paddleH, 1, BLACK);
}

void drawText(int x, int y, const char* text) {
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}

void updatePaddle() {
  pot = analogRead(POTPIN);
  paddleX = map(pot, 0, 1023, 0, display.width() - paddleW);
  paddleX = (paddleX - (display.width() - paddleW)) * (-1);
}

void updateBall() {
  int delta = 1;
  ballX += ballXD ? (-1) * delta : delta;
  ballY += ballYD ? (-1) * delta : delta;
}

void nextFrame() {
  // sides (horizontal)
  if (ballX < ballD || ballX >= display.width() - ballD) {
    ballXD = !ballXD;
  }

  // top
  if (ballY < ballD + headerH) {
    ballYD = !ballYD;
  } else {
    // bottom - collision with paddle
    if (ballY == display.height() - paddleH - ballD) {
      //if (ballX == constrain(ballX, paddleX, paddleX + paddleW)) {
      if (ballX >= paddleX && ballX <= paddleX + paddleW) {
        ballYD = !ballYD;
        hit();
      }
    } else {
      if (ballY > display.height() + ballD) {
        gameOver();
        resetGame();
      }
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= gameSpeed) {
    nextFrame();
    previousMillis = currentMillis;
  }

  display.clearDisplay();
  updatePaddle();
  updateBall();
  drawBall();
  drawPaddle();
  drawHeader();
  display.display();

  delay(25);
}

