# Parts List / Bill of Materials

## Electronics

### Core Components

| Item | Quantity | Specs | Est. Price | Notes |
|------|----------|-------|------------|-------|
| **SparkFun RedBoard** | 1 | Arduino Uno compatible | $20-25 | Or genuine Arduino Uno |
| **WS2812B LED Strip** | 2 | 60 LEDs/meter, waterproof (IP65+) | $15-20 each | Front rack strips - get 1 meter each |
| **WS2812B LED Grid/Strip** | 2 | 48 LEDs total (create 4x12 grid) | $12-15 each | Tail lights - can cut from strip |
| **12V Battery** | 1 | 5Ah+ SLA or Li-ion | $20-60 | LiPo/Li-ion preferred for weight |
| **Buck Converter** | 1 | 12V→5V, 10A rated, adjustable | $8-15 | LM2596 or similar |
| **3-way Toggle Switch** | 1 | SPDT or handlebar-mount | $5-15 | Your existing handlebar switch |
| **Momentary Switch** | 1 | Normally open, for brake | $3-8 | Hydraulic banjo or lever-mount |
| **Fuse Holder + 10A Fuse** | 1 | Inline blade fuse | $3-5 | Safety for battery circuit |

**Total Electronics**: ~$100-150

### Wiring & Connectors

| Item | Quantity | Specs | Est. Price | Notes |
|------|----------|-------|------------|-------|
| **Wire - 18 AWG** | 10 feet | Stranded, red & black | $5-10 | Power distribution |
| **Wire - 22-24 AWG** | 20 feet | Stranded, various colors | $5-10 | Signal wires |
| **JST-SM Connectors** | 10 sets | 3-pin, waterproof | $8-12 | LED strip connections |
| **Heat Shrink Tubing** | 1 kit | Assorted sizes | $8-12 | Insulation |
| **XT60 Connector** | 1 pair | For battery connection | $2-3 | High-current rated |
| **Terminal Blocks** | 2-3 | 3-position screw terminal | $3-5 | Power distribution |

**Total Wiring**: ~$30-50

## Mounting & Enclosures

| Item | Quantity | Specs | Est. Price | Notes |
|------|----------|-------|------------|-------|
| **Waterproof Box** | 1 | For RedBoard, ~4"x3"x2" | $8-15 | IP65 rated |
| **Small Enclosure** | 1 | For buck converter | $5-10 | With ventilation holes |
| **Zip Ties** | 1 pack | UV resistant, various sizes | $5-8 | Cable management |
| **Velcro Straps** | 1 pack | For battery mounting | $5-10 | Removable mounting |
| **Mounting Brackets** | As needed | L-brackets, aluminum | $5-15 | For LED grids |

**Total Mounting**: ~$30-60

## Tools (if you don't have them)

| Item | Est. Price | Notes |
|------|------------|-------|
| **Soldering Iron Kit** | $20-40 | Temperature controlled recommended |
| **Multimeter** | $15-30 | Essential for testing |
| **Wire Strippers** | $10-20 | Automatic strippers save time |
| **Heat Gun** | $15-30 | For heat shrink (or use lighter carefully) |
| **Helping Hands** | $10-15 | Makes soldering easier |

**Total Tools**: ~$70-135 (one-time investment)

## Optional Upgrades

| Item | Quantity | Specs | Est. Price | Purpose |
|------|----------|-------|------------|---------|
| **Voltage Display** | 1 | LED voltmeter, 0-30V | $5-8 | Monitor battery level |
| **Power Switch** | 1 | 12V 10A toggle | $5-10 | Master on/off |
| **USB Charging Port** | 1 | 5V 2A output | $5-10 | Charge phone from battery |
| **Capacitor** | 1 | 1000µF, 10V | $1-2 | Smooth LED power (optional) |
| **Level Shifter** | 1 | 3.3V-5V, 4-channel | $3-5 | Better signal (usually not needed) |

## Consumables

| Item | Notes |
|------|-------|
| **Solder** | Lead-free or 60/40 rosin core |
| **Electrical Tape** | Black vinyl, good quality |
| **Zip Ties** | Get extras, you'll use them |
| **Dielectric Grease** | For waterproofing connectors |
| **Threadlocker** | Blue Loctite for vibration resistance |
| **Isopropyl Alcohol** | 90%+ for cleaning flux |

## Where to Buy

### Online Retailers
- **Electronics**: SparkFun, Adafruit, Amazon, AliExpress
- **LEDs**: BTF-Lighting (Amazon), AliExpress (cheaper, slower shipping)
- **Battery**: Amazon, local battery stores
- **Wire/Connectors**: Amazon, Digi-Key, Mouser

### Money-Saving Tips
1. **Buy LED strips in bulk** from AliExpress (wait 2-4 weeks for shipping)
2. **Bundle shipping** - order everything at once from one vendor when possible
3. **Check local electronics stores** - sometimes cheaper for wire/connectors
4. **Reuse existing components** - old USB cables for wire, etc.

## Project Cost Summary

| Category | Budget Option | Premium Option |
|----------|--------------|----------------|
| Electronics | $100 | $150 |
| Wiring/Connectors | $30 | $50 |
| Mounting/Enclosures | $30 | $60 |
| Tools (if needed) | $70 | $135 |
| **Total** | **$230** | **$395** |

**Without tools (if you have them)**: $160 - $260

## Recommended Starter Kit

If buying from scratch, here's a good combo:

**Amazon/SparkFun Order (~$150)**
- RedBoard
- Buck converter
- Waterproof boxes
- Quality wire kit
- JST connectors
- Heat shrink kit
- 12V battery

**AliExpress Order (~$50, wait 3-4 weeks)**
- 4x 1-meter WS2812B strips (IP65)
- Extra connectors
- Spare buck converters (cheap backup)

## LED Strip Specifications to Look For

When buying WS2812B strips, ensure they have:
- ✅ **60 LEDs/meter** density (or 30/m is okay but less dense)
- ✅ **IP65 or IP67** waterproof rating (silicone sleeve)
- ✅ **5V power** (not 12V WS2811 variants)
- ✅ **3-wire connection**: 5V, GND, Data
- ✅ **Individually addressable** (true WS2812B)

Avoid:
- ❌ WS2811 (12V, different control)
- ❌ "RGB strip" without "addressable" (analog, won't work)
- ❌ Non-waterproof for bike use

## Battery Selection Guide

### Lead-Acid (SLA)
- **Pros**: Cheap ($20-30), widely available
- **Cons**: Heavy (3-5 lbs), bulky
- **Recommendation**: 12V 5Ah SLA

### Lithium Ion / LiPo
- **Pros**: Lightweight (1-2 lbs), compact, longer life
- **Cons**: More expensive ($40-60), needs proper charging
- **Recommendation**: 12V 4000-5000mAh Li-ion pack with BMS

### Lithium Polymer (LiPo)
- **Pros**: Lightest option, high discharge
- **Cons**: More fragile, requires LiPo charger, fire risk if damaged
- **Recommendation**: 3S (11.1V) 5000mAh 30C with XT60

**Best Value**: 12V 5Ah SLA for testing, upgrade to Li-ion later

## Print This Checklist Before Shopping

Shopping list for electronics store:
- [ ] RedBoard or Arduino Uno
- [ ] 12V to 5V buck converter (10A)
- [ ] 12V battery (5Ah+)
- [ ] 18 AWG wire (red & black)
- [ ] 22-24 AWG wire (colors)
- [ ] Heat shrink tubing kit
- [ ] JST-SM connectors (3-pin)
- [ ] XT60 connector
- [ ] Waterproof enclosure
- [ ] Zip ties
- [ ] Fuse holder + 10A fuse

Online order for LEDs:
- [ ] 2× 60-LED WS2812B strips (1 meter, IP65)
- [ ] 2× 48-LED count from strips (can cut 4×12 layout)

Good luck with your build! 🚴💡
