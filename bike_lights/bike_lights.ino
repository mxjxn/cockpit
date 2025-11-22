/*
 * Bike Turn Signals & Lights System
 *
 * Hardware:
 * - SparkFun RedBoard (Arduino Uno compatible)
 * - 2x 60 LED WS2812B strips (front left/right)
 * - 2x 48 LED WS2812B grids (4x12, tail left/right)
 * - 3-way turn signal switch
 * - Brake switch (momentary)
 *
 * Total LEDs: 216 (120 front + 96 rear)
 */

#include <FastLED.h>

// ===== CONFIGURATION =====
#define LED_PIN 6
#define NUM_LEDS 216
#define BRIGHTNESS 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// LED Layout
#define FRONT_LEFT_START 0
#define FRONT_LEFT_COUNT 60
#define FRONT_RIGHT_START 60
#define FRONT_RIGHT_COUNT 60
#define TAIL_LEFT_START 120
#define TAIL_LEFT_COUNT 48  // 4x12 grid
#define TAIL_RIGHT_START 168
#define TAIL_RIGHT_COUNT 48  // 4x12 grid

// Grid dimensions for tail lights
#define GRID_ROWS 4
#define GRID_COLS 12

// Input Pins
#define PIN_TURN_LEFT 2
#define PIN_TURN_RIGHT 3
#define PIN_BRAKE 4
// Future: PIN_HIGH_BEAM 5, PIN_HORN 7

// Animation timing
#define CHEVRON_SPEED 80      // ms between chevron frames
#define TURN_FLASH_SPEED 500  // ms for turn signal flash
#define BRAKE_ANIM_SPEED 50   // ms for brake animation

// ===== GLOBAL STATE =====
CRGB leds[NUM_LEDS];

enum TurnSignalState {
  TURN_OFF,
  TURN_LEFT,
  TURN_RIGHT
};

TurnSignalState turnState = TURN_OFF;
bool brakeActive = false;
unsigned long lastChevronUpdate = 0;
unsigned long lastTurnFlash = 0;
unsigned long lastBrakeUpdate = 0;
int chevronOffset = 0;
bool turnFlashOn = false;
int brakeAnimFrame = 0;

// Tail light brightness levels
#define TAIL_PERIMETER_BRIGHT 255
#define TAIL_CENTER_DIM 76        // 30% of 255
#define TAIL_CENTER_BRAKE 230      // 90% of 255

// ===== GRID HELPER FUNCTIONS =====

// Convert grid coordinates to LED index (snaking pattern)
// Grid snakes up column 0, down column 1, up column 2, etc.
int gridToIndex(int gridStart, int row, int col) {
  if (col % 2 == 0) {
    // Even columns: bottom to top (0,1,2,3)
    return gridStart + (col * GRID_ROWS) + row;
  } else {
    // Odd columns: top to bottom (3,2,1,0)
    return gridStart + (col * GRID_ROWS) + (GRID_ROWS - 1 - row);
  }
}

// Check if a grid position is on the perimeter
bool isPerimeter(int row, int col) {
  return (row == 0 || row == GRID_ROWS - 1 || col == 0 || col == GRID_COLS - 1);
}

// ===== ANIMATION FUNCTIONS =====

void setFrontStrip(int start, int count, CRGB color) {
  for (int i = start; i < start + count; i++) {
    leds[i] = color;
  }
}

void setFrontStripPattern(int start, int count, CRGB color, uint8_t brightness) {
  // Soft breathing pattern
  for (int i = start; i < start + count; i++) {
    CRGB c = color;
    c.nscale8(brightness);
    leds[i] = c;
  }
}

void clearGrid(int gridStart) {
  for (int i = gridStart; i < gridStart + TAIL_LEFT_COUNT; i++) {
    leds[i] = CRGB::Black;
  }
}

// Draw tail lights: bright perimeter, dim center
void drawTailLights(int gridStart, uint8_t centerBrightness) {
  for (int col = 0; col < GRID_COLS; col++) {
    for (int row = 0; row < GRID_ROWS; row++) {
      int idx = gridToIndex(gridStart, row, col);
      if (isPerimeter(row, col)) {
        leds[idx] = CRGB(TAIL_PERIMETER_BRIGHT, 0, 0);  // Bright red outline
      } else {
        leds[idx] = CRGB(centerBrightness, 0, 0);  // Dim red center
      }
    }
  }
}

// Draw chevron arrow (pointing left or right)
void drawChevron(int gridStart, int offset, bool pointLeft) {
  clearGrid(gridStart);

  // Draw 3 chevrons, each 4 pixels wide
  // Chevrons move across the grid
  for (int chevronNum = 0; chevronNum < 3; chevronNum++) {
    int baseCol = (offset + chevronNum * 5) % (GRID_COLS + 5);  // 4px wide + 1px gap

    // Draw a chevron pattern (< or >)
    for (int i = 0; i < 4; i++) {
      int col = baseCol + i;
      if (col >= 0 && col < GRID_COLS) {
        // Create chevron shape
        int row1, row2;
        if (pointLeft) {
          // Left-pointing chevron: \  /
          //                         \/
          if (i < 2) {
            row1 = 1 + i;  // Top half going down
            row2 = 2 - i;  // Bottom half going up
          } else {
            row1 = 3 - (i - 2);  // Top half going up
            row2 = (i - 2);      // Bottom half going down
          }
        } else {
          // Right-pointing chevron: /\
          //                         /  \
          if (i < 2) {
            row1 = 1 - i;      // Top going up
            row2 = 2 + i;      // Bottom going down
          } else {
            row1 = (i - 2);    // Top going down
            row2 = 5 - (i - 2); // Bottom going up
          }
        }

        // Clamp rows to valid range
        row1 = constrain(row1, 0, GRID_ROWS - 1);
        row2 = constrain(row2, 0, GRID_ROWS - 1);

        int idx1 = gridToIndex(gridStart, row1, col);
        int idx2 = gridToIndex(gridStart, row2, col);
        leds[idx1] = CRGB(255, 100, 0);  // Orange chevron
        if (row1 != row2) {
          leds[idx2] = CRGB(255, 100, 0);
        }
      }
    }
  }
}

// Brake animation: alternating bright/dim squares expanding inward
void drawBrakeAnimation(int gridStart, int frame) {
  // Start with tail lights at brake brightness
  drawTailLights(gridStart, TAIL_CENTER_BRAKE);

  // Add animated squares in center (columns 1-10, rows 1-2)
  for (int col = 1; col < GRID_COLS - 1; col++) {
    for (int row = 1; row < GRID_ROWS - 1; row++) {
      // Checkerboard pattern that alternates each frame
      bool isBright = ((col + row + frame) % 2) == 0;
      int idx = gridToIndex(gridStart, row, col);
      if (isBright) {
        leds[idx] = CRGB(TAIL_CENTER_BRAKE, 0, 0);
      } else {
        leds[idx] = CRGB(TAIL_CENTER_DIM, 0, 0);
      }
    }
  }
}

// ===== INPUT HANDLING =====

void readInputs() {
  // Read turn signal switch (3-way toggle)
  bool leftActive = digitalRead(PIN_TURN_LEFT) == LOW;   // Active low with pullup
  bool rightActive = digitalRead(PIN_TURN_RIGHT) == LOW;

  if (leftActive && !rightActive) {
    turnState = TURN_LEFT;
  } else if (rightActive && !leftActive) {
    turnState = TURN_RIGHT;
  } else {
    turnState = TURN_OFF;
  }

  // Read brake switch
  brakeActive = digitalRead(PIN_BRAKE) == LOW;  // Active low with pullup
}

// ===== MAIN RENDERING =====

void updateLights() {
  unsigned long now = millis();

  // ===== FRONT LIGHTS =====
  if (turnState == TURN_OFF) {
    // Soft ambient lighting on front strips
    setFrontStripPattern(FRONT_LEFT_START, FRONT_LEFT_COUNT, CRGB(50, 50, 50), 100);
    setFrontStripPattern(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, CRGB(50, 50, 50), 100);
    turnFlashOn = false;
  } else {
    // Flash turn signals
    if (now - lastTurnFlash > TURN_FLASH_SPEED) {
      lastTurnFlash = now;
      turnFlashOn = !turnFlashOn;
    }

    if (turnState == TURN_LEFT) {
      setFrontStrip(FRONT_LEFT_START, FRONT_LEFT_COUNT,
                    turnFlashOn ? CRGB(255, 100, 0) : CRGB::Black);
      setFrontStripPattern(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, CRGB(50, 50, 50), 100);
    } else {  // TURN_RIGHT
      setFrontStripPattern(FRONT_LEFT_START, FRONT_LEFT_COUNT, CRGB(50, 50, 50), 100);
      setFrontStrip(FRONT_RIGHT_START, FRONT_RIGHT_COUNT,
                    turnFlashOn ? CRGB(255, 100, 0) : CRGB::Black);
    }
  }

  // ===== TAIL LIGHTS =====
  if (brakeActive) {
    // Brake animation
    if (now - lastBrakeUpdate > BRAKE_ANIM_SPEED) {
      lastBrakeUpdate = now;
      brakeAnimFrame = (brakeAnimFrame + 1) % 2;
    }
    drawBrakeAnimation(TAIL_LEFT_START, brakeAnimFrame);
    drawBrakeAnimation(TAIL_RIGHT_START, brakeAnimFrame);

  } else if (turnState == TURN_LEFT || turnState == TURN_RIGHT) {
    // Chevron animation on appropriate side
    if (now - lastChevronUpdate > CHEVRON_SPEED) {
      lastChevronUpdate = now;
      chevronOffset = (chevronOffset + 1) % (GRID_COLS + 5);
    }

    if (turnState == TURN_LEFT) {
      drawChevron(TAIL_LEFT_START, chevronOffset, true);
      drawTailLights(TAIL_RIGHT_START, TAIL_CENTER_DIM);
    } else {
      drawTailLights(TAIL_LEFT_START, TAIL_CENTER_DIM);
      drawChevron(TAIL_RIGHT_START, chevronOffset, false);
    }

  } else {
    // Normal tail lights
    drawTailLights(TAIL_LEFT_START, TAIL_CENTER_DIM);
    drawTailLights(TAIL_RIGHT_START, TAIL_CENTER_DIM);
    brakeAnimFrame = 0;  // Reset brake animation
  }
}

// ===== SETUP & LOOP =====

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("Bike Lights System Starting...");

  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // Initialize input pins with pullups
  pinMode(PIN_TURN_LEFT, INPUT_PULLUP);
  pinMode(PIN_TURN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_BRAKE, INPUT_PULLUP);

  // Startup animation: quick test of all LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(300);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Green;
  }
  FastLED.show();
  delay(300);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Blue;
  }
  FastLED.show();
  delay(300);

  FastLED.clear();
  FastLED.show();

  Serial.println("Bike Lights Ready!");
}

void loop() {
  readInputs();
  updateLights();
  FastLED.show();

  // Small delay to prevent excessive updates
  delay(10);
}
