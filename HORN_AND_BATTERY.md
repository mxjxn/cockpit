# Horn and Battery Monitor Setup Guide

## Horn System

### Components Needed
- **12V Horn** (motorcycle/automotive horn, 1-2A typical)
- **5V Relay Module** (or individual relay + diode)
  - Recommended: 5V relay with optoisolator
  - Coil voltage: 5V
  - Contact rating: 10A minimum (for safety margin)
- **Flyback Diode** (1N4007 or similar, if not using relay module)
- **Momentary button** (already on handlebar)

### Wiring Diagram

```
                    ┌─────────────────┐
                    │  12V Battery    │
                    └────┬────────────┘
                         │ (+12V)
                         │
                    ┌────▼──────┐
                    │  Horn (+) │
                    │  12V Horn │
                    │  Horn (-) │
                    └────┬──────┘
                         │
                    ┌────▼────────┐
                    │ Relay COM   │  ← Normally Open Relay
                    │ Relay NO    │──→ GND (Battery -)
                    │ Relay NC    │    (not connected)
                    └──┬──────────┘
                       │
                  ┌────▼─────────┐
                  │ Relay Coil + │ ← Connected to RedBoard Pin 8
                  │ Relay Coil - │ ← Connected to GND
                  └──────────────┘

RedBoard Pin 7 ──→ Horn Button ──→ GND (when pressed)
RedBoard Pin 8 ──→ Relay Coil (+)
RedBoard GND   ──→ Relay Coil (-)
```

### How It Works
1. **Horn button pressed** (Pin 7 goes LOW)
2. **Arduino activates** Pin 8 (HIGH)
3. **Relay coil energizes**, closing relay contacts
4. **12V flows** through horn: Battery + → Horn → Relay COM → Relay NO → GND
5. **Horn sounds** and all LEDs strobe white
6. **Button released** → relay opens → horn stops

### Relay Selection Guide

#### Option 1: Relay Module (Recommended)
- **Example**: SRD-05VDC-SL-C relay module (~$2-3)
- **Pros**: Built-in flyback diode, optoisolator, LED indicator
- **Wiring**: VCC → 5V, GND → GND, IN → Pin 8, COM/NO for horn

#### Option 2: Individual Relay
- **Relay**: 5V SPST or SPDT, 10A contacts
- **Add flyback diode**: 1N4007 across relay coil (cathode to +, anode to -)
- **Prevents**: Voltage spikes that can damage Arduino

### Horn Recommendations
- **Automotive horn**: Fiamm, Hella (loud, 1-2A)
- **Motorcycle horn**: Cheaper, smaller, adequate volume
- **Bike horn**: Electric bell-style (lower current, quieter)

**Current Draw**:
- Typical horn: 1-2A @ 12V
- Relay coil: 70-90mA @ 5V (driven by Arduino pin is fine)

### Safety Notes
- ✅ **Always use a relay** - Arduino pins can't handle horn current
- ✅ **Flyback diode required** if not using relay module
- ✅ **Verify relay contacts rated** for horn current
- ⚠️ **Don't connect horn directly to Arduino** - will damage the board!

---

## Battery Voltage Monitor

### Purpose
- Monitor 12V battery charge level
- Warn when battery is low (< 10.5V)
- Alert when critical (< 10.0V)
- Display voltage on Serial Monitor

### Components Needed
- **2 Resistors for voltage divider**:
  - **R1**: 10kΩ resistor (battery → divider)
  - **R2**: 6.8kΩ resistor (divider → GND)
  - Alternative: 20kΩ + 10kΩ also works well
- **Optional**: 0.1µF capacitor across R2 for noise filtering

### Wiring Diagram

```
Battery (+12V) ──┬─→ [To Buck Converter, Horn, etc.]
                 │
                 └──→ R1 (10kΩ) ──┬──→ Pin A0 (Arduino)
                                   │
                                   ├──→ C1 (0.1µF, optional)
                                   │
                                   └──→ R2 (6.8kΩ) ──→ GND

Calculation:
Vout = Vin × (R2 / (R1 + R2))
Vout = 12V × (6.8k / (10k + 6.8k))
Vout = 12V × 0.405 = 4.86V  ✓ SAFE (< 5V)

At 10V (low battery):   Vout = 4.05V  ✓
At 14V (charging):      Vout = 5.67V  ✗ TOO HIGH!

If using charger > 13V, use 20kΩ + 10kΩ instead:
Vout = 14V × (10k / 30k) = 4.67V  ✓ SAFE
```

### Resistor Combinations

| R1 | R2 | Ratio | Max Safe Vin | Notes |
|----|----|-------|--------------|-------|
| 10kΩ | 6.8kΩ | 2.47 | 12.35V | Good for 12V battery only |
| 20kΩ | 10kΩ | 3.0 | 15V | Safe with 14.4V charging |
| 22kΩ | 10kΩ | 3.2 | 16V | Best for lead-acid + charger |
| 10kΩ | 10kΩ | 2.0 | 10V | ⚠️ UNSAFE for 12V! |

**Recommendation**: Use **10kΩ + 6.8kΩ** for 12V battery (no charger connected during use)

### Code Configuration

The firmware is pre-configured for 10kΩ + 6.8kΩ:

```cpp
#define VOLTAGE_DIVIDER_RATIO 2.47   // (R1 + R2) / R2 = 16.8k / 6.8k
#define VOLTAGE_LOW_THRESHOLD 10.5   // Low battery warning (volts)
#define VOLTAGE_CRITICAL_THRESHOLD 10.0  // Critical battery (volts)
```

**To change for different resistors**:
1. Calculate ratio: (R1 + R2) / R2
2. Update `VOLTAGE_DIVIDER_RATIO` in `bike_lights.ino` line 60

### Battery Warning Indicators

| Battery Level | Voltage | LED Indicator |
|---------------|---------|---------------|
| **Good** | > 10.5V | Normal operation |
| **Low** | 10.0V - 10.5V | Amber pulse on bottom corner of tail grids |
| **Critical** | < 10.0V | Fast red flash on 3 bottom LEDs of each tail grid |

**Visual Examples**:
- **Low**: Slow amber breathing on first LED of each tail light (1Hz)
- **Critical**: Fast red strobe on bottom 3 LEDs (4Hz) - very noticeable

### Assembly Instructions

#### Step 1: Build Voltage Divider
```
1. Solder R1 (10kΩ) to a wire from battery (+)
2. Solder R2 (6.8kΩ) between R1 and GND
3. Tap the junction between R1 and R2 → this goes to Pin A0
4. Optional: Solder 0.1µF cap across R2 for stability
5. Use heat shrink to insulate all connections
```

#### Step 2: Connect to RedBoard
```
Battery (+12V) ───→ Wire ───→ R1 (10kΩ) ───┬───→ Pin A0
                                            │
                                            └───→ R2 (6.8kΩ) ───→ GND
```

#### Step 3: Test Voltage Reading
1. Upload firmware
2. Open Serial Monitor (115200 baud)
3. Check voltage reading every 2 seconds
4. Verify it matches a multimeter reading (±0.2V tolerance)

### Calibration

If voltage reading is inaccurate:

1. **Measure actual battery voltage** with multimeter (e.g., 12.3V)
2. **Check Arduino reading** in Serial Monitor (e.g., "Battery: 11.8V")
3. **Calculate correction ratio**:
   ```
   Actual Voltage / Arduino Reading = Correction
   12.3V / 11.8V = 1.042
   ```
4. **Update firmware**:
   ```cpp
   #define VOLTAGE_DIVIDER_RATIO 2.57   // 2.47 × 1.042 = 2.57
   ```
5. **Re-upload** and verify

### Voltage Thresholds for Different Batteries

#### Lead-Acid (SLA)
```cpp
#define VOLTAGE_LOW_THRESHOLD 11.8    // ~50% capacity
#define VOLTAGE_CRITICAL_THRESHOLD 11.0  // ~20% capacity
```

#### Lithium-Ion (3S)
```cpp
#define VOLTAGE_LOW_THRESHOLD 10.5    // ~30% capacity
#define VOLTAGE_CRITICAL_THRESHOLD 9.9   // ~10% capacity (min safe)
```

#### LiPo (3S)
```cpp
#define VOLTAGE_LOW_THRESHOLD 10.8    // ~20% capacity
#define VOLTAGE_CRITICAL_THRESHOLD 10.2  // ~10% capacity (don't go lower!)
```

### Troubleshooting

| Issue | Solution |
|-------|----------|
| Voltage reads 0V | Check R1/R2 connections, verify Pin A0 wired correctly |
| Voltage too high | Resistor ratio wrong - use higher R1 or lower R2 |
| Voltage too low | Check resistor values with multimeter |
| Voltage fluctuates | Add 0.1µF capacitor across R2 |
| Reading unstable | Check for loose connections, EMI from motor/horn |

### Safety Warnings

⚠️ **CRITICAL**: Never exceed 5V on Arduino analog input!
- If using a charger (14.4V+), **use 20kΩ + 10kΩ resistors**
- Double-check resistor values before connecting
- Test with multimeter first before connecting to Arduino

⚠️ **Voltage spikes**: Add a 5.1V Zener diode from A0 to GND for overvoltage protection (optional but recommended)

---

## Complete Pin Summary

| Pin | Function | Connection |
|-----|----------|------------|
| A0 | Battery Voltage | Voltage divider (between R1 and R2) |
| 7 | Horn Button | Momentary button → GND |
| 8 | Horn Relay | Relay module IN pin (or relay coil +) |

---

## Serial Monitor Output

When everything is working, you'll see:

```
Bike Lights System Starting...
Initial Battery Voltage: 12.34V
Bike Lights Ready!
Horn: Pin 7 (button) -> Pin 8 (relay)
Battery Monitor: Pin A0
Battery: 12.34V (OK)
Battery: 12.31V (OK)
Battery: 10.42V (LOW)
Battery: 10.01V (CRITICAL!)
```

---

## Shopping List

| Item | Quantity | Price | Notes |
|------|----------|-------|-------|
| 10kΩ resistor (1/4W) | 1 | $0.10 | Voltage divider R1 |
| 6.8kΩ resistor (1/4W) | 1 | $0.10 | Voltage divider R2 |
| 5V relay module | 1 | $2-3 | SRD-05VDC-SL-C or similar |
| 12V horn | 1 | $5-15 | Motorcycle or automotive |
| 0.1µF capacitor | 1 | $0.10 | Optional, for filtering |
| Wire & connectors | As needed | $2-5 | 22-24 AWG for signals |

**Total**: ~$10-25

---

## Testing Procedure

### Test 1: Voltage Monitor
1. Connect voltage divider without Arduino
2. Measure voltage at divider output with multimeter
3. Verify < 5V at 12V input
4. Connect to Arduino Pin A0
5. Check Serial Monitor for accurate reading

### Test 2: Horn Relay
1. Connect relay module to Pin 8 and GND
2. Don't connect horn yet
3. Press horn button (Pin 7)
4. Listen for relay click
5. Verify relay LED illuminates
6. Measure 12V across relay contacts when active

### Test 3: Full Horn System
1. Connect 12V horn through relay
2. Press horn button
3. Horn should sound AND LEDs should strobe white
4. Release button → horn stops, LEDs resume normal

### Test 4: Battery Warnings
1. Use adjustable power supply to simulate low battery
2. Lower voltage to 10.4V
3. Verify amber pulse on tail lights
4. Lower to 9.8V
5. Verify fast red flash (critical warning)

---

## Performance Notes

- **Battery check frequency**: Every 2 seconds (configurable)
- **Horn strobe speed**: 100ms (10Hz flash)
- **Current draw**:
  - Horn: 1-2A from 12V battery
  - Relay coil: ~80mA from 5V (Arduino can handle this)
  - Voltage monitor: ~0.7mA (negligible)

---

## Future Enhancements

1. **Low battery auto-dim**: Reduce LED brightness when battery low to extend runtime
2. **Voltage display**: Use OLED display to show voltage numerically
3. **Logging**: Record voltage over time to EEPROM/SD card
4. **Horn patterns**: Different horn button press patterns (double-tap, long press)
5. **Custom thresholds**: Adjust warning levels via serial commands

Happy building! 🚴📯🔋
