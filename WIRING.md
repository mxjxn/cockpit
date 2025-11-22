# Detailed Wiring Guide

## Complete System Wiring

### Power Distribution
```
                    ┌─────────────────┐
                    │  12V Battery    │
                    │   (5Ah+ rec.)   │
                    └────┬────────┬───┘
                         │        │
                      (+)│        │(-)
                         │        │
                    ┌────▼────────▼────┐
                    │  Buck Converter  │
                    │   12V → 5V       │
                    │   (10A rated)    │
                    └────┬────────┬────┘
                         │        │
                      5V │        │ GND
         ┌───────────────┴────────┴──────────────┐
         │                                       │
         │  ┌────────────────────────────────┐   │
         ├──┤ Front Left Strip (60 LEDs)    │   │
         │  │ VCC: 5V  GND: GND  DIN: Pin 6 │   │
         │  └──────────────┬─────────────────┘   │
         │                 │ DOUT                │
         │  ┌──────────────▼─────────────────┐   │
         ├──┤ Front Right Strip (60 LEDs)   │   │
         │  │ VCC: 5V  GND: GND  DIN: DOUT  │   │
         │  └──────────────┬─────────────────┘   │
         │                 │ DOUT                │
         │  ┌──────────────▼─────────────────┐   │
         ├──┤ Tail Left Grid (48 LEDs)      │   │
         │  │ VCC: 5V  GND: GND  DIN: DOUT  │   │
         │  └──────────────┬─────────────────┘   │
         │                 │ DOUT                │
         │  ┌──────────────▼─────────────────┐   │
         ├──┤ Tail Right Grid (48 LEDs)     │   │
         │  │ VCC: 5V  GND: GND  DIN: DOUT  │   │
         │  └─────────────────────────────────┘   │
         │                                        │
         │         ┌──────────────────┐           │
         └─────────┤ RedBoard GND     │───────────┘
                   │                  │
                   │ Pin 6 ───────────┼──→ First LED Strip DIN
                   │                  │
                   │ Pin 2 ←──────────┼──── Turn Left Switch ──→ GND
                   │ Pin 3 ←──────────┼──── Turn Right Switch ──→ GND
                   │ Pin 4 ←──────────┼──── Brake Switch ──→ GND
                   │                  │
                   │ 5V or VIN ───────┼──→ USB or 5V from Buck
                   └──────────────────┘
```

## Step-by-Step Wiring Instructions

### Step 1: Buck Converter Setup
1. Adjust buck converter to output exactly **5.0V** before connecting LEDs
   - Use a multimeter to verify voltage
   - Turn adjustment screw slowly
2. Connect 12V battery to input terminals (observe polarity!)
3. Verify 5V output with multimeter

### Step 2: Power Distribution to LEDs
Create a power distribution point:
1. Use a terminal block or solder junction
2. Connect buck converter 5V OUT+ to all LED strip VCC (red wires)
3. Connect buck converter GND to all LED strip GND (white/black wires)
4. Use 18-20 AWG wire for these connections (high current!)

### Step 3: Common Ground
**CRITICAL**: Connect RedBoard GND to the LED power GND
- Use a wire from RedBoard GND pin to the same GND as buck converter
- Without this, data signals won't work properly

### Step 4: Data Line Chain
Connect in this exact order:
1. RedBoard Pin 6 → Front Left Strip DIN
2. Front Left DOUT → Front Right DIN
3. Front Right DOUT → Tail Left DIN
4. Tail Left DOUT → Tail Right DIN

### Step 5: Control Switches
Wire switches to these pins (all active LOW):

**Turn Signal 3-Way Toggle:**
```
         Left Position    Center    Right Position
Pin 2:   Connected to GND   Open     Open
Pin 3:   Open               Open     Connected to GND
```

**Brake Switch:**
- One terminal to Pin 4
- Other terminal to GND
- When pressed, connects Pin 4 to GND

### Step 6: Power for RedBoard
Choose one option:
- **Option A**: USB power (separate from LED power) - good for testing
- **Option B**: Connect 5V from buck converter to RedBoard 5V pin
- **Option C**: Use VIN pin with 7-12V (use a second output from battery before buck converter)

**Recommendation**: Use Option A (USB) for initial testing, then Option B for final installation.

## Wire Gauge Recommendations

| Connection | Wire Gauge | Length | Notes |
|------------|-----------|--------|-------|
| Battery to Buck IN | 18 AWG | Short as possible | High current |
| Buck OUT to LED VCC | 18 AWG | < 6 feet | High current, minimize voltage drop |
| Buck GND to LED GND | 18 AWG | < 6 feet | Return path for high current |
| LED Data (DIN/DOUT) | 22-24 AWG | < 6 feet per segment | Signal wire |
| Switch wires | 24-26 AWG | As needed | Low current signal |
| RedBoard to switches | 24-26 AWG | As needed | Low current |

## Connector Recommendations

### For LED Strips (Waterproof)
- **JST-SM connectors** (3-pin: VCC, GND, Data)
- Seal with heat shrink tubing
- Add dielectric grease for extra water resistance

### For Power
- **XT60 or XT30 connectors** for battery connection
- **Screw terminals** for buck converter
- **Wago lever nuts** for parallel VCC/GND distribution (easy, reliable)

## Hydraulic Brake Switch Installation

### Banjo Bolt Switch Wiring
Your hydraulic brake uses a banjo bolt - here's how to integrate a switch:

1. **Purchase a brake light switch** designed for motorcycles/bikes
   - Example: "M10x1.0 hydraulic brake light switch"
   - Typical pressure activation: 10-20 PSI

2. **Installation on banjo bolt:**
   - Replace the banjo bolt with a "T" adapter or switch-integrated bolt
   - Wire runs from switch to RedBoard Pin 4 and GND

3. **Wiring:**
   ```
   Brake Switch Terminal 1 ──→ RedBoard Pin 4
   Brake Switch Terminal 2 ──→ GND
   ```
   Switch closes circuit when brake pressure applied.

**Alternative**: Use a **mechanical lever switch** attached to the brake lever itself
- Mount microswitch near brake lever pivot
- Adjust so it triggers just before brake pads engage
- Easier installation, no hydraulic work needed

## Testing Checklist

### Before Powering On
- [ ] Buck converter adjusted to 5.0V (tested with multimeter)
- [ ] All LED VCC connected to 5V (parallel)
- [ ] All LED GND connected to GND (parallel)
- [ ] RedBoard GND connected to LED power GND
- [ ] Data line properly chained through all strips
- [ ] Switches wired to correct pins (2, 3, 4)
- [ ] No short circuits (check with multimeter)
- [ ] All connections insulated (heat shrink, tape)

### Power-On Test Sequence
1. Power buck converter only (no RedBoard yet)
2. Verify 5V at LED strips with multimeter
3. Power RedBoard via USB
4. Should see startup animation (R-G-B flash)
5. Test turn switches (see orange chevrons/flashes)
6. Test brake switch (see brake animation)

### Troubleshooting Tests
If LEDs don't work:
1. Check voltage at first LED strip (should be 5V)
2. Check GND continuity between RedBoard and LEDs
3. Disconnect data line, check if Pin 6 shows ~2.5V idle
4. Test with just first 60 LEDs (disconnect rest)
5. Reduce BRIGHTNESS to 50 in code, try again

## Safety Notes

### Electrical Safety
- **Fuse your battery**: Use a 10A fuse between battery and buck converter
- **Reverse polarity protection**: Many buck converters have this, but verify
- **Heat management**: Buck converters can get hot at high current (add heatsink)

### Mechanical Safety
- **Secure all wiring**: Use zip ties, don't let wires tangle in wheels/chain
- **Waterproofing**: Use IP65+ rated enclosures for electronics
- **Vibration**: Use threadlocker on screws, strain relief on connectors

### Visibility & Legal
- **Brightness**: Ensure lights don't blind other road users
- **Regulations**: Check local laws about bike lighting colors
- **Redundancy**: Keep traditional lights as backup

## Tools Needed

- Soldering iron & solder
- Wire strippers
- Multimeter
- Heat shrink tubing & heat gun
- Zip ties
- Screwdrivers
- Electrical tape
- (Optional) Crimping tool for connectors

Happy building! 🚴✨
