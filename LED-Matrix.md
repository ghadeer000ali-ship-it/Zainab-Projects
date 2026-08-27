# Zainab-Projects
I use Arduino Uno and ESP32 in my projects.
#include <LedControl.h>

// =====================================================
// MAX7219 LED MATRIX
// =====================================================
#define DIN_PIN     6
#define CLK_PIN     5
#define CS_PIN      3
#define NUM_MODULES 4

LedControl lc(DIN_PIN, CLK_PIN, CS_PIN, NUM_MODULES);

// =====================================================
// JOYSTICK + BUZZER
// =====================================================
#define VRx A0
#define VRy A1
#define SW  2
#define BUZZER_PIN 11

// =====================================================
// GAME SCREEN
// =====================================================
const int SCREEN_W = 8;
const int PREVIEW_MODULE = 3;
const int GAME_MODULES = NUM_MODULES - 1;
const int SCREEN_H = SCREEN_W * GAME_MODULES;

// =====================================================
// GAME FIELD
// =====================================================
uint8_t field[SCREEN_H];

// =====================================================
// TIMING
// =====================================================
unsigned long lastDrop = 0;
unsigned long dropInterval = 500;

unsigned long lastMove = 0;
const unsigned long moveInterval = 200;

const unsigned long refreshInterval = 33;
unsigned long lastRefresh = 0;

// =====================================================
// BUTTON DEBOUNCE
// =====================================================
unsigned long lastButtonCheck = 0;
const unsigned long debounceDelay = 30;

bool lastButtonState = HIGH;
bool buttonPressedEvent = false;

// =====================================================
// BUZZER
// =====================================================
bool buzzerOn = false;
unsigned long buzzerOffTime = 0;

// =====================================================
// LANDING DELAY
// =====================================================
bool pendingLock = false;
unsigned long lockTime = 0;
const unsigned long lockDelay = 30;

// =====================================================
// NEXT BLOCK DELAY
// =====================================================
bool waitingForNextBlock = false;
unsigned long nextBlockTime = 0;
const unsigned long nextBlockDelay = 120;

// =====================================================
// DISPLAY BUFFER
// =====================================================
uint8_t prevBuf[NUM_MODULES][SCREEN_W];

// =====================================================
// BLOCK STRUCTURE
// =====================================================
struct Block {
  const int (*shape)[2];
  int len;
  int x, y;
  int rotation;
  char type;
} current;

char nextType;

// =====================================================
// BLOCK SHAPES
// =====================================================

// I
const int I_SHAPE[2][4][2] = {
  {{0,0},{0,1},{0,2},{0,3}},
  {{-1,1},{0,1},{1,1},{2,1}}
};

// O
const int O_SHAPE[1][4][2] = {
  {{0,0},{1,0},{0,1},{1,1}}
};

// T
const int T_SHAPE[4][4][2] = {
  {{1,0},{0,1},{1,1},{2,1}},
  {{1,0},{1,1},{1,2},{0,1}},
  {{0,1},{1,1},{2,1},{1,2}},
  {{1,0},{1,1},{1,2},{2,1}}
};

// L
const int L_SHAPE[4][4][2] = {
  {{0,0},{0,1},{0,2},{1,2}},
  {{0,0},{1,0},{2,0},{0,1}},
  {{0,0},{1,0},{1,1},{1,2}},
  {{2,0},{0,1},{1,1},{2,1}}
};

// J
const int J_SHAPE[4][4][2] = {
  {{1,0},{1,1},{1,2},{0,2}},
  {{0,0},{0,1},{1,1},{2,1}},
  {{0,0},{1,0},{0,1},{0,2}},
  {{0,0},{1,0},{2,0},{2,1}}
};

// S
const int S_SHAPE[2][4][2] = {
  {{1,0},{2,0},{0,1},{1,1}},
  {{1,0},{1,1},{2,1},{2,2}}
};

// Z
const int Z_SHAPE[2][4][2] = {
  {{0,0},{1,0},{1,1},{2,1}},
  {{2,0},{1,1},{2,1},{1,2}}
};

// =====================================================
// GAME OVER LETTERS
// =====================================================
static const uint8_t PAT_G[8] = {
  0x3C,0x42,0x40,0x4E,0x42,0x42,0x3C,0x00
};

static const uint8_t PAT_A[8] = {
  0x18,0x24,0x42,0x7E,0x42,0x42,0x42,0x00
};

static const uint8_t PAT_M[8] = {
  0x42,0x66,0x5A,0x5A,0x42,0x42,0x42,0x00
};

static const uint8_t PAT_E[8] = {
  0x7E,0x40,0x5C,0x40,0x40,0x40,0x7E,0x00
};

static const uint8_t PAT_O[8] = {
  0x3C,0x42,0x42,0x42,0x42,0x42,0x3C,0x00
};

static const uint8_t PAT_V[8] = {
  0x42,0x42,0x42,0x42,0x42,0x24,0x18,0x00
};

static const uint8_t PAT_R[8] = {
  0x7C,0x42,0x42,0x7C,0x48,0x44,0x42,0x00
};

// =====================================================
// BUZZER FUNCTIONS
// =====================================================

// تشغيل نغمة بدون تعطيل اللعبة
void startBeep(unsigned int frequency, unsigned long duration) {
  tone(BUZZER_PIN, frequency);

  buzzerOn = true;
  buzzerOffTime = millis() + duration;
}

// إيقاف النغمة بعد انتهاء مدتها
void updateBuzzer() {
  if (buzzerOn && millis() >= buzzerOffTime) {
    noTone(BUZZER_PIN);
    buzzerOn = false;
  }
}

// -----------------------------------------------------
// START SOUND
// -----------------------------------------------------
void soundStart() {

  tone(BUZZER_PIN, 700);
  delay(80);
  noTone(BUZZER_PIN);

  delay(50);

  tone(BUZZER_PIN, 1000);
  delay(80);
  noTone(BUZZER_PIN);

  delay(50);

  tone(BUZZER_PIN, 1400);
  delay(120);
  noTone(BUZZER_PIN);

  delay(40);

  tone(BUZZER_PIN, 1800);
  delay(180);
  noTone(BUZZER_PIN);
}

// -----------------------------------------------------
// ROTATE SOUND
// -----------------------------------------------------
void soundRotate() {
  startBeep(1500, 60);
}

// -----------------------------------------------------
// LAND SOUND
// -----------------------------------------------------
void soundLand() {
  startBeep(450, 100);
}

// -----------------------------------------------------
// GAME OVER SOUND
// -----------------------------------------------------
void soundGameOver() {

  tone(BUZZER_PIN, 1000);
  delay(120);
  noTone(BUZZER_PIN);

  delay(60);

  tone(BUZZER_PIN, 750);
  delay(120);
  noTone(BUZZER_PIN);

  delay(60);

  tone(BUZZER_PIN, 500);
  delay(150);
  noTone(BUZZER_PIN);

  delay(60);

  tone(BUZZER_PIN, 300);
  delay(350);
  noTone(BUZZER_PIN);
}

// =====================================================
// CLEAR DISPLAY
// =====================================================
void clearAll() {
  for (int m = 0; m < NUM_MODULES; m++) {
    lc.clearDisplay(m);
  }
}

// =====================================================
// BUTTON
// =====================================================
bool readButton() {

  bool currentState = digitalRead(SW);
  unsigned long now = millis();

  if (currentState != lastButtonState) {
    lastButtonCheck = now;
    lastButtonState = currentState;
  }

  if ((now - lastButtonCheck) > debounceDelay) {

    if (currentState == LOW && !buttonPressedEvent) {
      buttonPressedEvent = true;
      return true;
    }

    if (currentState == HIGH) {
      buttonPressedEvent = false;
    }
  }

  return false;
}

// =====================================================
// LETTER PATTERN
// =====================================================
const uint8_t* letterPattern(char c) {

  switch (c) {

    case 'G': return PAT_G;
    case 'A': return PAT_A;
    case 'M': return PAT_M;
    case 'E': return PAT_E;
    case 'O': return PAT_O;
    case 'V': return PAT_V;
    case 'R': return PAT_R;

    default:
      return PAT_E;
  }
}

// =====================================================
// RANDOM BLOCK
// =====================================================
char randomBlockType() {

  const char types[] = {
    'I', 'O', 'T', 'L', 'J', 'S', 'Z'
  };

  return types[random(7)];
}

// =====================================================
// GET SHAPE
// =====================================================
const int (*getShapeByType(char type))[2] {

  if (type == 'I') return I_SHAPE[0];
  else if (type == 'O') return O_SHAPE[0];
  else if (type == 'T') return T_SHAPE[0];
  else if (type == 'L') return L_SHAPE[0];
  else if (type == 'J') return J_SHAPE[0];
  else if (type == 'S') return S_SHAPE[0];
  else if (type == 'Z') return Z_SHAPE[0];

  return O_SHAPE[0];
}

// =====================================================
// SET BLOCK
// =====================================================
void setBlockByType(char type, int sx, int sy) {

  current.rotation = 0;
  current.type = type;
  current.len = 4;

  current.x = sx;
  current.y = sy;

  if (type == 'I')
    current.shape = I_SHAPE[0];

  else if (type == 'O')
    current.shape = O_SHAPE[0];

  else if (type == 'T')
    current.shape = T_SHAPE[0];

  else if (type == 'L')
    current.shape = L_SHAPE[0];

  else if (type == 'J')
    current.shape = J_SHAPE[0];

  else if (type == 'S')
    current.shape = S_SHAPE[0];

  else if (type == 'Z')
    current.shape = Z_SHAPE[0];
}

// =====================================================
// DRAW NEXT BLOCK
// =====================================================
void drawNextBlock() {

  uint8_t preview[8] = {};

  const int (*shape)[2] =
    getShapeByType(nextType);

  int offsetX = 3;
  int offsetY = 3;

  if (nextType == 'I') {
    offsetX = 4;
    offsetY = 3;
  }

  else if (nextType == 'O') {
    offsetX = 4;
    offsetY = 4;
  }

  for (int i = 0; i < 4; i++) {

    int x =
      offsetX + shape[i][0];

    int y =
      offsetY + shape[i][1];

    if (x >= 0 && x < 8 &&
        y >= 0 && y < 8) {

      preview[y] |=
        (1 << x);
    }
  }

  // حدود منطقة المعاينة
  for (int row = 0; row < 8; row++) {
    preview[row] |= (1 << 0);
  }

  for (int row = 0; row < 8; row++) {

    lc.setRow(
      PREVIEW_MODULE,
      row,
      preview[row]
    );

    prevBuf[PREVIEW_MODULE][row] =
      preview[row];
  }
}

// =====================================================
// SPAWN BLOCK
// =====================================================
void spawnBlock() {

  int sx =
    SCREEN_W / 2 - 2;

  setBlockByType(
    nextType,
    sx,
    0
  );

  nextType =
    randomBlockType();
}

// =====================================================
// GAME OVER
// =====================================================
void gameOverSequence() {

  soundGameOver();

  // وميض الشاشة
  for (int i = 0; i < 3; i++) {

    clearAll();

    delay(300);

    for (int m = 0;
         m < NUM_MODULES;
         m++) {

      for (int r = 0;
           r < SCREEN_W;
           r++) {

        lc.setRow(
          m,
          r,
          0xFF
        );
      }
    }

    delay(300);
  }

  // GAME
  const char* w1 = "GAME";

  for (int seg = 0;
       seg < 4;
       seg++) {

    const uint8_t* pat =
      letterPattern(w1[seg]);

    uint8_t rot[8] = {};

    for (int y = 0; y < 8; y++) {

      for (int x = 0; x < 8; x++) {

        if (pat[y] & (1 << x)) {

          int nx = 7 - y;
          int ny = x;

          rot[ny] |=
            (1 << nx);
        }
      }
    }

    int module =
      NUM_MODULES - 1 - seg;

    for (int row = 0;
         row < 8;
         row++) {

      lc.setRow(
        module,
        row,
        rot[row]
      );
    }
  }

  delay(1000);

  // OVER
  const char* w2 = "OVER";

  for (int seg = 0;
       seg < 4;
       seg++) {

    const uint8_t* pat =
      letterPattern(w2[seg]);

    uint8_t rot[8] = {};

    for (int y = 0; y < 8; y++) {

      for (int x = 0; x < 8; x++) {

        if (pat[y] & (1 << x)) {

          int nx = 7 - y;
          int ny = x;

          rot[ny] |=
            (1 << nx);
        }
      }
    }

    int module =
      NUM_MODULES - 1 - seg;

    for (int row = 0;
         row < 8;
         row++) {

      lc.setRow(
        module,
        row,
        rot[row]
      );
    }
  }

  delay(1200);

  // انتظار الضغط
  while (digitalRead(SW) != LOW) {
    delay(10);
  }

  while (digitalRead(SW) == LOW) {
    delay(10);
  }
}

// =====================================================
// RESET GAME
// =====================================================
void resetGame() {

  memset(
    field,
    0,
    sizeof(field)
  );

  clearAll();

  for (int m = 0;
       m < NUM_MODULES;
       m++) {

    for (int r = 0;
         r < SCREEN_W;
         r++) {

      prevBuf[m][r] = 0;
    }
  }

  pendingLock = false;
  waitingForNextBlock = false;

  buzzerOn = false;
  noTone(BUZZER_PIN);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  nextType =
    randomBlockType();

  spawnBlock();

  drawNextBlock();

  writeBuffer();

  soundStart();

  lastDrop = millis();
  lastRefresh = millis();
}

// =====================================================
// DISPLAY BUFFER
// =====================================================
void writeBuffer() {

  uint8_t buf[NUM_MODULES][SCREEN_W] = {};

  // رسم القطع المثبتة
  for (int y = 0;
       y < SCREEN_H;
       y++) {

    uint8_t row =
      field[y];

    if (!row)
      continue;

    int mod =
      2 - (y / SCREEN_W);

    int bit =
      1 << (7 - (y % SCREEN_W));

    for (int x = 0;
         x < SCREEN_W;
         x++) {

      if (row & (1 << x)) {

        buf[mod][x] |=
          bit;
      }
    }
  }

  // رسم القطعة الحالية
  if (!waitingForNextBlock) {

    for (int i = 0;
         i < current.len;
         i++) {

      int xx =
        current.x +
        current.shape[i][0];

      int yy =
        current.y +
        current.shape[i][1];

      if (xx < 0 ||
          xx >= SCREEN_W ||
          yy < 0 ||
          yy >= SCREEN_H) {

        continue;
      }

      int mod =
        2 - (yy / SCREEN_W);

      int bit =
        1 << (7 - (yy % SCREEN_W));

      buf[mod][xx] |=
        bit;
    }
  }

  // تحديث الصفوف المتغيرة فقط
  for (int m = 0; m < 3; m++) {

    for (int r = 0;
         r < SCREEN_W;
         r++) {

      if (buf[m][r] !=
          prevBuf[m][r]) {

        lc.setRow(
          m,
          r,
          buf[m][r]
        );

        prevBuf[m][r] =
          buf[m][r];
      }
    }
  }
}

// =====================================================
// COLLISION
// =====================================================
bool checkCollision(
  int nx,
  int ny
) {

  for (int i = 0;
       i < current.len;
       i++) {

    int xx =
      nx +
      current.shape[i][0];

    int yy =
      ny +
      current.shape[i][1];

    if (xx < 0 ||
        xx >= SCREEN_W ||
        yy >= SCREEN_H) {

      return true;
    }

    if (yy >= 0 &&
        (field[yy] &
         (1 << xx))) {

      return true;
    }
  }

  return false;
}

// =====================================================
// TOP CHECK
// =====================================================
bool isAtTop() {

  for (int i = 0;
       i < current.len;
       i++) {

    if (
      current.y +
      current.shape[i][1]
      == 0
    ) {

      return true;
    }
  }

  return false;
}

// =====================================================
// PLACE BLOCK
// =====================================================
void placeBlock() {

  for (int i = 0;
       i < current.len;
       i++) {

    int xx =
      current.x +
      current.shape[i][0];

    int yy =
      current.y +
      current.shape[i][1];

    if (yy >= 0 &&
        yy < SCREEN_H) {

      field[yy] |=
        (1 << xx);
    }
  }

  // حذف الصفوف الممتلئة
  for (int y = 0;
       y < SCREEN_H;
       y++) {

    if (field[y] == 0xFF) {

      for (int j = y;
           j > 0;
           j--) {

        field[j] =
          field[j - 1];
      }

      field[0] = 0;
    }
  }
}

// =====================================================
// LOCK BLOCK
// =====================================================
void lockCurrentBlock(
  unsigned long now
) {

  if (isAtTop()) {

    gameOverSequence();

    resetGame();

    return;
  }

  placeBlock();

  waitingForNextBlock = true;
  pendingLock = false;

  soundLand();

  writeBuffer();

  nextBlockTime =
    now +
    nextBlockDelay;
}

// =====================================================
// ROTATE BLOCK
// =====================================================
void rotateBlock() {

  int limit =
    (
      current.type == 'I' ||
      current.type == 'S' ||
      current.type == 'Z'
    )
    ? 2
    :
    (
      current.type == 'O'
      ? 1
      : 4
    );

  int nr =
    (current.rotation + 1)
    % limit;

  const int (*ns)[2] =
    nullptr;

  if (current.type == 'I')
    ns = I_SHAPE[nr];

  else if (current.type == 'O')
    ns = O_SHAPE[0];

  else if (current.type == 'T')
    ns = T_SHAPE[nr];

  else if (current.type == 'L')
    ns = L_SHAPE[nr];

  else if (current.type == 'J')
    ns = J_SHAPE[nr];

  else if (current.type == 'S')
    ns = S_SHAPE[nr];

  else if (current.type == 'Z')
    ns = Z_SHAPE[nr];

  Block bak =
    current;

  current.shape =
    ns;

  current.rotation =
    nr;

  // إذا ما ناسبت القطعة
  if (checkCollision(
        current.x,
        current.y
      )) {

    current =
      bak;
  }

  else {

    soundRotate();
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  pinMode(
    SW,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  randomSeed(
    analogRead(A2)
  );

  // تشغيل المصفوفات
  for (int m = 0;
       m < NUM_MODULES;
       m++) {

    lc.shutdown(
      m,
      false
    );

    lc.setIntensity(
      m,
      8
    );

    lc.clearDisplay(m);

    for (int r = 0;
         r < SCREEN_W;
         r++) {

      prevBuf[m][r] = 0;
    }
  }

  resetGame();
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {

  unsigned long now =
    millis();

  // تحديث البازر
  updateBuzzer();

  // ===================================================
  // LOCK DELAY
  // ===================================================
  if (pendingLock) {

    if (now >= lockTime) {

      lockCurrentBlock(now);
    }

    if (
      now - lastRefresh
      >= refreshInterval
    ) {

      writeBuffer();

      lastRefresh =
        now;
    }

    return;
  }

  // ===================================================
  // WAIT FOR NEXT BLOCK
  // ===================================================
  if (waitingForNextBlock) {

    if (now >= nextBlockTime) {

      spawnBlock();

      drawNextBlock();

      waitingForNextBlock =
        false;

      lastDrop =
        now;

      lastRefresh =
        now;
    }

    if (
      now - lastRefresh
      >= refreshInterval
    ) {

      writeBuffer();

      lastRefresh =
        now;
    }

    return;
  }

  // ===================================================
  // JOYSTICK LEFT / RIGHT
  // ===================================================
  int ax =
    analogRead(VRx);

  if (
    now - lastMove
    > moveInterval
  ) {

    if (
      ax < 400 &&
      !checkCollision(
        current.x + 1,
        current.y
      )
    ) {

      current.x++;

      lastMove =
        now;
    }

    else if (
      ax > 600 &&
      !checkCollision(
        current.x - 1,
        current.y
      )
    ) {

      current.x--;

      lastMove =
        now;
    }
  }

  // ===================================================
  // ROTATE
  // ===================================================
  if (readButton()) {

    rotateBlock();
  }

  // ===================================================
  // JOYSTICK DOWN
  // ===================================================
  int ay =
    analogRead(VRy);

  dropInterval =
    700 -
    constrain(
      map(
        ay,
        512,
        1023,
        0,
        690
      ),
      0,
      690
    );

  // ===================================================
  // AUTOMATIC DROP
  // ===================================================
  if (
    now - lastDrop
    > dropInterval
  ) {

    lastDrop =
      now;

    if (
      !checkCollision(
        current.x,
        current.y + 1
      )
    ) {

      current.y++;

      writeBuffer();

      if (
        checkCollision(
          current.x,
          current.y + 1
        )
      ) {

        pendingLock =
          true;

        lockTime =
          now +
          lockDelay;
      }
    }

    else {

      pendingLock =
        true;

      lockTime =
        now;
    }
  }

  // ===================================================
  // DISPLAY REFRESH
  // ===================================================
  if (
    now - lastRefresh
    >= refreshInterval
  ) {

    writeBuffer();

    lastRefresh =
      now;
  }
}
