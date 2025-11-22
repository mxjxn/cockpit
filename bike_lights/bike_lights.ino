/*
 * Bike Turn Signals & Lights System
 *
 * Hardware:
 * - SparkFun RedBoard (Arduino Uno compatible)
 * - 2x 60 LED WS2812B strips (front left/right)
 * - 2x 48 LED WS2812B grids (4x12, tail left/right)
 * - 3-way turn signal toggle switch
 * - Momentary button (standin for brake until real brake switch wired)
 * - Toggle button (party mode on/off)
 * - Horn button (momentary) + 12V horn with relay
 * - Battery voltage monitor (12V with voltage divider)
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
#define PIN_BRAKE 4        // Momentary button (standin until real brake switch)
#define PIN_PARTY_MODE 5   // Toggle button for party mode
#define PIN_HORN_BUTTON 7  // Horn momentary button
#define PIN_HORN_RELAY 8   // Output to relay for 12V horn
#define PIN_VOLTAGE_SENSE A0  // Analog input for battery voltage (voltage divider)

// Animation timing
#define CHEVRON_SPEED 80      // ms between chevron frames
#define TURN_FLASH_SPEED 500  // ms for turn signal flash
#define BRAKE_ANIM_SPEED 50   // ms for brake animation
#define PARTY_SPEED 30        // ms for party mode animations
#define HORN_FLASH_SPEED 100  // ms for horn strobe effect

// Battery voltage monitoring (voltage divider: 12V -> 5V max)
// Using 10k and 10k resistor divider: Vout = Vin * (R2 / (R1 + R2))
// For 12V input with 10k/10k: Vout = 12V * 0.5 = 6V (DANGER!)
// Use 10k and 6.8k: Vout = 12V * (6.8 / 16.8) = 4.86V (SAFE)
#define VOLTAGE_DIVIDER_RATIO 2.47   // (R1 + R2) / R2 = 16.8k / 6.8k
#define VOLTAGE_SAMPLES 10           // Number of samples to average
#define VOLTAGE_LOW_THRESHOLD 10.5   // Low battery warning (volts)
#define VOLTAGE_CRITICAL_THRESHOLD 10.0  // Critical battery (volts)
#define VOLTAGE_CHECK_INTERVAL 2000  // Check battery every 2 seconds

// ===== GLOBAL STATE =====
CRGB leds[NUM_LEDS];

enum TurnSignalState {
  TURN_OFF,
  TURN_LEFT,
  TURN_RIGHT
};

TurnSignalState turnState = TURN_OFF;
bool brakeActive = false;
bool partyModeActive = false;
bool hornActive = false;
unsigned long lastChevronUpdate = 0;
unsigned long lastTurnFlash = 0;
unsigned long lastBrakeUpdate = 0;
unsigned long lastPartyUpdate = 0;
unsigned long lastHornFlash = 0;
unsigned long lastVoltageCheck = 0;
int chevronOffset = 0;
bool turnFlashOn = false;
bool hornFlashOn = false;
int brakeAnimFrame = 0;
uint8_t partyHue = 0;  // Rotating hue for party mode

// Battery monitoring
float batteryVoltage = 12.0;
bool batteryLow = false;
bool batteryCritical = false;

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

// Party mode: rainbow wave on front strips
void drawPartyFrontStrip(int start, int count, uint8_t hueOffset) {
  for (int i = 0; i < count; i++) {
    // Create a moving rainbow wave
    uint8_t hue = hueOffset + (i * 256 / count);
    leds[start + i] = CHSV(hue, 255, 200);
  }
}

// Party mode: color-changing tail lights (perimeter stays red, center changes)
void drawPartyTailLights(int gridStart, uint8_t hue) {
  for (int col = 0; col < GRID_COLS; col++) {
    for (int row = 0; row < GRID_ROWS; row++) {
      int idx = gridToIndex(gridStart, row, col);
      if (isPerimeter(row, col)) {
        // Perimeter stays bright red for safety
        leds[idx] = CRGB(TAIL_PERIMETER_BRIGHT, 0, 0);
      } else {
        // Center: color-changing rings radiating from center
        int distFromCenter = abs(col - GRID_COLS / 2) + abs(row - GRID_ROWS / 2);
        uint8_t pixelHue = hue + (distFromCenter * 30);
        leds[idx] = CHSV(pixelHue, 255, 150);
      }
    }
  }
}

// ===== BATTERY VOLTAGE MONITORING =====

float readBatteryVoltage() {
  long sum = 0;
  // Take multiple samples and average for stability
  for (int i = 0; i < VOLTAGE_SAMPLES; i++) {
    sum += analogRead(PIN_VOLTAGE_SENSE);
    delay(1);  // Small delay between samples
  }

  float avgReading = sum / (float)VOLTAGE_SAMPLES;

  // Convert ADC reading to voltage
  // ADC: 0-1023 = 0-5V
  float voltage = (avgReading / 1023.0) * 5.0;

  // Apply voltage divider ratio to get actual battery voltage
  voltage = voltage * VOLTAGE_DIVIDER_RATIO;

  return voltage;
}

void checkBatteryVoltage() {
  unsigned long now = millis();

  if (now - lastVoltageCheck > VOLTAGE_CHECK_INTERVAL) {
    lastVoltageCheck = now;
    batteryVoltage = readBatteryVoltage();

    // Update battery status flags
    if (batteryVoltage < VOLTAGE_CRITICAL_THRESHOLD) {
      batteryCritical = true;
      batteryLow = true;
    } else if (batteryVoltage < VOLTAGE_LOW_THRESHOLD) {
      batteryCritical = false;
      batteryLow = true;
    } else {
      batteryCritical = false;
      batteryLow = false;
    }

    // Debug output
    Serial.print("Battery: ");
    Serial.print(batteryVoltage);
    Serial.print("V ");
    if (batteryCritical) {
      Serial.println("(CRITICAL!)");
    } else if (batteryLow) {
      Serial.println("(LOW)");
    } else {
      Serial.println("(OK)");
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

  // Read brake switch (momentary button standin)
  brakeActive = digitalRead(PIN_BRAKE) == LOW;  // Active low with pullup

  // Read party mode toggle
  partyModeActive = digitalRead(PIN_PARTY_MODE) == LOW;  // Active low with pullup

  // Read horn button and control relay
  hornActive = digitalRead(PIN_HORN_BUTTON) == LOW;  // Active low with pullup
  digitalWrite(PIN_HORN_RELAY, hornActive ? HIGH : LOW);  // Activate relay when horn pressed
}

// ===== MAIN RENDERING =====

void updateLights() {
  unsigned long now = millis();

  // Update party mode animation
  if (partyModeActive && (now - lastPartyUpdate > PARTY_SPEED)) {
    lastPartyUpdate = now;
    partyHue += 2;  // Rotate hue for party mode
  }

  // ===== HORN FLASH (HIGHEST PRIORITY) =====
  // When horn is pressed, strobe all LEDs white for maximum visibility
  if (hornActive) {
    if (now - lastHornFlash > HORN_FLASH_SPEED) {
      lastHornFlash = now;
      hornFlashOn = !hornFlashOn;
    }

    if (hornFlashOn) {
      // Flash all LEDs bright white
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::White;
      }
    } else {
      // Off phase - show normal tail lights for safety
      drawTailLights(TAIL_LEFT_START, TAIL_CENTER_DIM);
      drawTailLights(TAIL_RIGHT_START, TAIL_CENTER_DIM);
      // Front strips off during horn
      setFrontStrip(FRONT_LEFT_START, FRONT_LEFT_COUNT, CRGB::Black);
      setFrontStrip(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, CRGB::Black);
    }
    return;  // Horn overrides everything else
  }

  // ===== BATTERY WARNING OVERLAY =====
  // Subtle battery warning that doesn't interfere with safety features
  // Will be applied at the end of this function

  // ===== FRONT LIGHTS =====
  // Turn signals OVERRIDE party mode
  if (turnState == TURN_OFF) {
    // No turn signal active
    if (partyModeActive) {
      // Party mode: rainbow wave on front strips
      drawPartyFrontStrip(FRONT_LEFT_START, FRONT_LEFT_COUNT, partyHue);
      drawPartyFrontStrip(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, partyHue + 128);  // Offset for variety
    } else {
      // Normal mode: soft ambient lighting
      setFrontStripPattern(FRONT_LEFT_START, FRONT_LEFT_COUNT, CRGB(50, 50, 50), 100);
      setFrontStripPattern(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, CRGB(50, 50, 50), 100);
    }
    turnFlashOn = false;
  } else {
    // Turn signal ACTIVE - overrides party mode
    if (now - lastTurnFlash > TURN_FLASH_SPEED) {
      lastTurnFlash = now;
      turnFlashOn = !turnFlashOn;
    }

    if (turnState == TURN_LEFT) {
      setFrontStrip(FRONT_LEFT_START, FRONT_LEFT_COUNT,
                    turnFlashOn ? CRGB(255, 100, 0) : CRGB::Black);
      // Non-signaling side: show party mode if active, otherwise ambient
      if (partyModeActive) {
        drawPartyFrontStrip(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, partyHue + 128);
      } else {
        setFrontStripPattern(FRONT_RIGHT_START, FRONT_RIGHT_COUNT, CRGB(50, 50, 50), 100);
      }
    } else {  // TURN_RIGHT
      // Non-signaling side: show party mode if active, otherwise ambient
      if (partyModeActive) {
        drawPartyFrontStrip(FRONT_LEFT_START, FRONT_LEFT_COUNT, partyHue);
      } else {
        setFrontStripPattern(FRONT_LEFT_START, FRONT_LEFT_COUNT, CRGB(50, 50, 50), 100);
      }
      setFrontStrip(FRONT_RIGHT_START, FRONT_RIGHT_COUNT,
                    turnFlashOn ? CRGB(255, 100, 0) : CRGB::Black);
    }
  }

  // ===== TAIL LIGHTS =====
  // Priority: Brake > Turn Signal > Party Mode > Normal
  if (brakeActive) {
    // Brake animation (highest priority for safety)
    if (now - lastBrakeUpdate > BRAKE_ANIM_SPEED) {
      lastBrakeUpdate = now;
      brakeAnimFrame = (brakeAnimFrame + 1) % 2;
    }
    drawBrakeAnimation(TAIL_LEFT_START, brakeAnimFrame);
    drawBrakeAnimation(TAIL_RIGHT_START, brakeAnimFrame);

  } else if (turnState == TURN_LEFT || turnState == TURN_RIGHT) {
    // Turn signal chevron animation - overrides party mode
    if (now - lastChevronUpdate > CHEVRON_SPEED) {
      lastChevronUpdate = now;
      chevronOffset = (chevronOffset + 1) % (GRID_COLS + 5);
    }

    if (turnState == TURN_LEFT) {
      drawChevron(TAIL_LEFT_START, chevronOffset, true);
      // Non-signaling side: party mode if active, otherwise normal
      if (partyModeActive) {
        drawPartyTailLights(TAIL_RIGHT_START, partyHue + 128);
      } else {
        drawTailLights(TAIL_RIGHT_START, TAIL_CENTER_DIM);
      }
    } else {  // TURN_RIGHT
      // Non-signaling side: party mode if active, otherwise normal
      if (partyModeActive) {
        drawPartyTailLights(TAIL_LEFT_START, partyHue);
      } else {
        drawTailLights(TAIL_LEFT_START, TAIL_CENTER_DIM);
      }
      drawChevron(TAIL_RIGHT_START, chevronOffset, false);
    }

  } else {
    // No brake, no turn signal
    if (partyModeActive) {
      // Party mode: color-changing centers, red perimeter
      drawPartyTailLights(TAIL_LEFT_START, partyHue);
      drawPartyTailLights(TAIL_RIGHT_START, partyHue + 128);
    } else {
      // Normal tail lights
      drawTailLights(TAIL_LEFT_START, TAIL_CENTER_DIM);
      drawTailLights(TAIL_RIGHT_START, TAIL_CENTER_DIM);
    }
    brakeAnimFrame = 0;  // Reset brake animation
  }

  // ===== BATTERY WARNING OVERLAY =====
  // Subtle visual warning for low battery - doesn't override safety features
  if (batteryCritical) {
    // Critical: Flash first 3 LEDs of each tail grid red rapidly
    bool criticalFlash = (now / 250) % 2;  // Fast 4Hz flash
    if (criticalFlash) {
      for (int i = 0; i < 3; i++) {
        leds[TAIL_LEFT_START + i] = CRGB::Red;
        leds[TAIL_RIGHT_START + i] = CRGB::Red;
      }
    }
  } else if (batteryLow) {
    // Low: Dim first LED of each tail grid amber (slow pulse)
    uint8_t pulseLevel = (sin(now / 1000.0) + 1.0) * 64;  // Pulse 0-128
    leds[TAIL_LEFT_START] = CRGB(pulseLevel, pulseLevel / 2, 0);
    leds[TAIL_RIGHT_START] = CRGB(pulseLevel, pulseLevel / 2, 0);
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
  pinMode(PIN_PARTY_MODE, INPUT_PULLUP);
  pinMode(PIN_HORN_BUTTON, INPUT_PULLUP);

  // Initialize output pins
  pinMode(PIN_HORN_RELAY, OUTPUT);
  digitalWrite(PIN_HORN_RELAY, LOW);  // Relay off initially

  // Initialize analog input for voltage sensing
  pinMode(PIN_VOLTAGE_SENSE, INPUT);

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

  // Initial battery voltage check
  batteryVoltage = readBatteryVoltage();
  Serial.print("Initial Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.println("V");

  Serial.println("Bike Lights Ready!");
  Serial.println("Horn: Pin 7 (button) -> Pin 8 (relay)");
  Serial.println("Battery Monitor: Pin A0");
}

void loop() {
  checkBatteryVoltage();  // Checks periodically, not every loop
  readInputs();
  updateLights();
  FastLED.show();

  // Small delay to prevent excessive updates
  delay(10);
}
