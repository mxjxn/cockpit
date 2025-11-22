# Bike Turn Signals & Lighting System

A complete Arduino-based lighting system for bicycles featuring animated turn signals, brake lights, and tail lights using WS2812B LED strips.

## Features

- **Turn Signals**: Animated chevron arrows moving across 4x12 LED grids
- **Brake Lights**: Animated alternating pattern that intensifies when braking
- **Tail Lights**: Bright red perimeter outline with soft center illumination
- **Front Rack Lights**: Soft ambient lighting with bright flashing turn indicators
- **Expandable**: Ready for high beam and horn integration

## Hardware Requirements

### Components
- **SparkFun RedBoard** (or Arduino Uno compatible)
- **WS2812B LED Strips**:
  - 2x 60 LED strips (front left/right on rack)
  - 2x 48 LED grids (4x12, arranged as tail lights)
  - Total: 216 LEDs
- **3-way toggle switch** (turn signals - left/center/right)
- **Momentary switch** (brake)
- **12V battery** (recommended: 12V 5Ah+ for good runtime)
- **12V to 5V buck converter** (10A+ rated)
- **Wiring**: 18-20 AWG wire for power, 22-24 AWG for signals

### Power Supply Recommendations

#### Why 12V Battery?
Using a 12V battery provides better capacity/weight ratio and longer runtime compared to a direct 5V battery pack.

#### Buck Converter Selection
You need a **12V to 5V DC-DC buck converter** rated for at least **10 Amps**. Recommended models:
- **LM2596 10A adjustable buck converter** (~$5-10)
- **Pololu D24V50F5** (5A, quieter, more expensive)
- **DROK LM2596** (adjustable, high efficiency)

#### Power Calculations
- Max theoretical draw: 216 LEDs × 60mA = **12.96A** (all LEDs at full white)
- Typical animation draw: **6-8A** (much more realistic)
- 12V 5Ah battery runtime: ~45-60 minutes of continuous use

#### Wiring for Power
```
12V Battery (+) ──→ Buck Converter IN+ ──→ 5V OUT+ ──→ LED Strip VCC (all strips in parallel)
12V Battery (-) ──→ Buck Converter IN- ──→ 5V OUT- ──→ LED Strip GND (all strips in parallel)
                                              │
                                              └──→ RedBoard GND (common ground!)
```

**CRITICAL**: Connect RedBoard GND to LED power GND for common ground. Without this, data signals won't work!

## Wiring Diagram

### LED Data Connections
```
RedBoard Pin 6 ──→ Front Left Strip (60 LEDs)   ──→ Front Right Strip (60 LEDs)
                                                 ──→ Tail Left Grid (48 LEDs)
                                                 ──→ Tail Tail Grid (48 LEDs)
```

All LED strips are **daisy-chained** on the data line in this order:
1. Front Left (LEDs 0-59)
2. Front Right (LEDs 60-119)
3. Tail Left (LEDs 120-167)
4. Tail Right (LEDs 168-215)

### Control Inputs
```
RedBoard Pin 2 ──→ Turn Signal Left Switch ──→ GND (when active)
RedBoard Pin 3 ──→ Turn Signal Right Switch ──→ GND (when active)
RedBoard Pin 4 ──→ Brake Switch ──→ GND (when pressed)
```

All switches use **INPUT_PULLUP** mode (active LOW). Wire one side of each switch to the pin, the other to GND.

### 3-Way Toggle Switch Wiring
Your handlebar switch has a 3-position toggle:
```
Left Position:  Pin 2 → GND (Pin 3 open)
Center:         Both pins open
Right Position: Pin 3 → GND (Pin 2 open)
```

### Full Pin Assignment
| Pin | Function | Type | Notes |
|-----|----------|------|-------|
| 2 | Turn Left | Digital Input | Active LOW, pullup enabled |
| 3 | Turn Right | Digital Input | Active LOW, pullup enabled |
| 4 | Brake | Digital Input | Active LOW, pullup enabled |
| 5 | High Beam (future) | Digital Input | Reserved |
| 6 | LED Data | Digital Output | WS2812B data line |
| 7 | Horn (future) | Digital Input | Reserved |

## LED Grid Layout

### Tail Light Grids (4 rows × 12 columns)
The 48-LED grids use a **snaking pattern**:
```
Column:  0  1  2  3  4  5  6  7  8  9  10 11
Row 0:   ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓
Row 1:   ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓
Row 2:   ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓
Row 3:   ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓  ↑  ↓
```
- Even columns (0,2,4...): Data flows **bottom to top** (Row 0→3)
- Odd columns (1,3,5...): Data flows **top to bottom** (Row 3→0)

## Animations

### Tail Lights (Default)
- **Perimeter**: Bright red outline (255 brightness)
- **Center**: Soft red fill (76 brightness, ~30%)

### Turn Signals
- **Front strips**: Bright orange flash (500ms on/off)
- **Tail grids**: 3 moving chevron arrows (4px wide each)
  - Left signal: Chevrons point left (<)
  - Right signal: Chevrons point right (>)
  - Animation speed: 80ms per frame

### Brake Lights
- **Perimeter**: Remains bright red
- **Center**: Animates from 30% to 90% brightness
- Alternating checkerboard pattern for visual impact
- Animation speed: 50ms per frame

## Installation

### Software Setup

1. **Install Arduino IDE** (1.8.19+ or 2.x)
   - Download from https://arduino.cc/en/software

2. **Install FastLED Library**
   - Open Arduino IDE
   - Go to **Sketch → Include Library → Manage Libraries**
   - Search for "FastLED"
   - Install **FastLED by Daniel Garcia**

3. **Upload the Sketch**
   - Open `bike_lights/bike_lights.ino`
   - Select **Tools → Board → Arduino Uno** (or SparkFun RedBoard)
   - Select your COM port under **Tools → Port**
   - Click **Upload** button

### Hardware Assembly

1. **Test LEDs First**
   - Connect just the first LED strip to test
   - Use a 5V power supply (not USB - not enough current)
   - Verify data flow through all strips

2. **Mount LEDs on Bike**
   - Front strips: Along front rack sides
   - Tail grids: Rear of bike (arrange in 4x12 pattern)
   - Use waterproof housing/coating

3. **Wire Power System**
   - Install buck converter near battery
   - Run heavy gauge wire (18-20 AWG) for 5V power
   - Connect all LED VCC/GND in parallel
   - **Don't forget common ground to RedBoard!**

4. **Wire Control Inputs**
   - Mount handlebar switch
   - Run wires to RedBoard pins 2,3,4
   - Install brake switch on hydraulic banjo bolt

5. **Weatherproof Everything**
   - Use heat shrink on all connections
   - Silicone sealant on RedBoard enclosure
   - Waterproof housing for buck converter

## Testing

### Startup Test
On power-up, all LEDs will flash:
1. Red (300ms)
2. Green (300ms)
3. Blue (300ms)
4. Off

This confirms all LEDs are working.

### Serial Monitor
Open Serial Monitor (115200 baud) to see debug output:
```
Bike Lights System Starting...
Bike Lights Ready!
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| LEDs don't light up | Check power connections, verify 5V at LED strips |
| LEDs show wrong colors | Verify GND is common between RedBoard and LED power |
| First few LEDs work, rest don't | Check data line connections between strips |
| Dim or flickering LEDs | Power supply insufficient - use thicker wire or higher amp converter |
| Turn signals don't respond | Check switch wiring, verify pins 2,3 connections |

## Future Enhancements

- **High Beam**: Pin 5 (toggle switch)
- **Horn**: Pin 7 (momentary button) - could flash all LEDs white
- **Battery monitor**: Analog pin to check battery level
- **Wireless control**: Bluetooth module for smartphone app

## Customization

### Adjust Brightness
Change line 16 in `bike_lights.ino`:
```cpp
#define BRIGHTNESS 200  // 0-255, default 200
```

### Adjust Animation Speeds
Lines 48-50:
```cpp
#define CHEVRON_SPEED 80      // Lower = faster chevrons
#define TURN_FLASH_SPEED 500  // Lower = faster flashing
#define BRAKE_ANIM_SPEED 50   // Lower = faster brake animation
```

### Change Colors
- Turn signals: Line 137, 219, 250 - `CRGB(255, 100, 0)` (orange)
- Tail lights: Lines 110, 112, 186 - `CRGB(R, G, B)` (red)

## License

MIT License - Feel free to modify and share!

## Contributing

Found a bug or have an improvement? Open an issue or submit a pull request!
