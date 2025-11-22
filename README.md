# Cockpit - Networked Vehicle Lighting & Synchronization Platform

A modular platform for synchronized lighting, music, and sensor systems across multiple vehicles (bikes, skateboards, etc.) using Arduino, Raspberry Pi, and wireless mesh networking.

## Vision

Create a network of vehicles that can:
- **Sync light shows** in real-time when within range
- **Synchronize music playback** using EchoFlux
- **Share sensor data** (speed, location, battery, etc.)
- **Coordinate animations** for group rides
- **Auto-discover** nearby vehicles
- **Mesh network** for extended range

## Project Structure

```
cockpit/
├── arduino-bike-lights/          # Arduino-based LED control system
│   ├── bike_lights/              # Main Arduino sketch
│   ├── README.md                 # Arduino setup guide
│   ├── WIRING.md                 # Hardware wiring diagrams
│   ├── PARTS_LIST.md             # Bill of materials
│   └── HORN_AND_BATTERY.md       # Horn and voltage monitor setup
│
├── pi-controller/                # Raspberry Pi Zero controller (future)
│   ├── main.py                   # Main Pi control application
│   ├── sync/                     # Synchronization protocol
│   ├── echoflux_integration/     # Music sync integration
│   └── README.md                 # Pi setup guide
│
├── sync-protocol/                # Cross-platform sync specification (future)
│   ├── PROTOCOL.md               # Protocol specification
│   ├── message_types.md          # Message format definitions
│   └── examples/                 # Implementation examples
│
└── docs/                         # Architecture and design docs
    ├── NETWORKING_ARCHITECTURE.md   # Overall system architecture
    ├── SYNC_PROTOCOL.md          # Synchronization design
    ├── ECHOFLUX_INTEGRATION.md   # Music sync design
    └── DEPLOYMENT.md             # Multi-vehicle setup guide
```

## Current Status

### ✅ Implemented
- **Arduino Bike Lights** - Complete standalone LED control system
  - Turn signals with animated chevrons
  - Brake lights with pulsing animation
  - Party mode with rainbow effects
  - 12V horn with white LED strobe
  - Battery voltage monitoring
  - 216 WS2812B LEDs (120 front + 96 rear grids)

### 🚧 In Design
- **Raspberry Pi Controller** - Network coordinator and music player
- **Sync Protocol** - Wireless synchronization specification
- **EchoFlux Integration** - Shared music playback

### 📋 Planned
- **Multi-vehicle mesh network**
- **Mobile app control**
- **GPS-based formations**
- **Sensor fusion** (accelerometer, gyro, GPS)

## Architecture Overview

### Hardware Layers

```
┌─────────────────────────────────────────────────────┐
│  Vehicle 1              Vehicle 2              Vehicle 3   │
│  ┌──────────┐          ┌──────────┐          ┌──────────┐ │
│  │ Arduino  │          │ Arduino  │          │ Arduino  │ │
│  │ + LEDs   │          │ + LEDs   │          │ + LEDs   │ │
│  └────┬─────┘          └────┬─────┘          └────┬─────┘ │
│       │                     │                     │        │
│  ┌────▼─────┐          ┌────▼─────┐          ┌────▼─────┐ │
│  │ Pi Zero  │◄────────►│ Pi Zero  │◄────────►│ Pi Zero  │ │
│  │ WiFi/BT  │   Mesh   │ WiFi/BT  │   Mesh   │ WiFi/BT  │ │
│  └──────────┘          └──────────┘          └──────────┘ │
│  EchoFlux              EchoFlux              EchoFlux      │
└─────────────────────────────────────────────────────────────┘
```

### Communication Paths

1. **Arduino ↔ Pi**: Serial UART (commands, sensor data)
2. **Pi ↔ Pi**: WiFi mesh or Bluetooth (sync messages)
3. **Pi → EchoFlux**: Local API (music control)
4. **User ↔ System**: Mobile app via WiFi/BLE

## Component Projects

### 1. Arduino Bike Lights
**Status**: ✅ Complete
**Purpose**: Low-level LED control, sensors, and local features

**Features**:
- Real-time LED animations (turn signals, brake, party mode)
- Battery voltage monitoring
- Horn control with relay
- Standalone operation (works without Pi)
- Serial interface for external control

**Hardware**: SparkFun RedBoard, WS2812B LEDs, 12V battery

[📖 Full Documentation](./arduino-bike-lights/README.md)

---

### 2. Raspberry Pi Controller
**Status**: 🚧 Design Phase
**Purpose**: Network coordination, music sync, and intelligence

**Planned Features**:
- WiFi mesh networking (802.11s or custom)
- Bluetooth LE for close-range sync
- EchoFlux integration for music playback
- Pattern/animation library
- Auto-discovery of nearby vehicles
- Command relay to Arduino via serial
- GPS for location-based formations
- Web API for mobile app control

**Hardware**: Raspberry Pi Zero W (or Pi Zero 2 W)

[📋 See Architecture Docs](./docs/NETWORKING_ARCHITECTURE.md)

---

### 3. Sync Protocol
**Status**: 🚧 Design Phase
**Purpose**: Define message format and timing for synchronization

**Key Requirements**:
- Low latency (<50ms for light sync)
- Resilient to packet loss
- Time synchronization (NTP or custom)
- State broadcasting (animation, music position)
- Leader election for coordinated shows
- Backward compatible

[📋 Protocol Specification](./docs/SYNC_PROTOCOL.md)

---

## Use Cases

### 🚴 Group Ride Sync
**Scenario**: 5 bikes riding together
**Behavior**:
- Auto-discover when within 50m range
- Elect leader (first to arrive or manual)
- Leader broadcasts pattern every 100ms
- All bikes sync animations to leader
- Turn signals remain independent (safety)

### 🎵 Music-Synced Light Show
**Scenario**: Parked bikes at night with music
**Behavior**:
- All Pis running EchoFlux
- Leader broadcasts music position every 500ms
- Lights animate to beat/frequency
- Phase-locked to prevent drift
- Graceful degradation if sync lost

### 🌐 Mesh Network Relay
**Scenario**: Long line of bikes (parade, etc.)
**Behavior**:
- Each Pi acts as mesh node
- Messages hop through network
- Extended range beyond direct WiFi
- Dynamic routing if bikes move

### 📱 Mobile App Control
**Scenario**: User wants to change patterns
**Behavior**:
- Phone connects to nearest Pi via BLE or WiFi
- Send commands to one or all vehicles
- Upload custom patterns
- Monitor battery levels
- Emergency stop

## Getting Started

### Quick Start (Arduino Only)
If you just want basic bike lights without networking:

1. **Build the hardware**: See [arduino-bike-lights/PARTS_LIST.md](./arduino-bike-lights/PARTS_LIST.md)
2. **Wire it up**: Follow [arduino-bike-lights/WIRING.md](./arduino-bike-lights/WIRING.md)
3. **Upload firmware**: [arduino-bike-lights/README.md](./arduino-bike-lights/README.md)
4. **Ride!** 🚴💡

### Full Platform (Arduino + Pi + Networking)
Coming soon! This will enable:
- Multi-vehicle synchronization
- EchoFlux music integration
- Mobile app control
- Advanced sensor features

*See [docs/DEPLOYMENT.md](./docs/DEPLOYMENT.md) for multi-vehicle setup guide (in progress)*

## Technology Stack

### Current (Arduino)
- **Platform**: Arduino Uno compatible (ATmega328P)
- **Language**: C++ (Arduino framework)
- **Library**: FastLED for LED control
- **LEDs**: WS2812B addressable RGB (216 total)

### Future (Raspberry Pi)
- **Platform**: Raspberry Pi Zero W / Zero 2 W
- **OS**: Raspberry Pi OS Lite (Debian-based)
- **Language**: Python 3.9+ (asyncio for concurrency)
- **Networking**:
  - WiFi: Built-in 802.11n (2.4GHz)
  - Bluetooth: Built-in BLE 4.2
  - Mesh: batman-adv or custom protocol
- **Libraries**:
  - `pyserial` - Arduino communication
  - `asyncio` - Event loop for sync
  - `aiohttp` - EchoFlux API client
  - `bluepy` or `bleak` - BLE communication
  - `python-osc` - Music sync messages

### Future (Mobile App)
- **Platform**: React Native or Flutter (cross-platform)
- **Protocols**: BLE, HTTP REST API
- **Features**: Pattern control, battery monitor, group management

## Development Roadmap

### Phase 1: Arduino Foundation ✅ COMPLETE
- [x] LED control with animations
- [x] Turn signals with chevrons
- [x] Brake lights
- [x] Party mode
- [x] Horn integration
- [x] Battery monitoring

### Phase 2: Pi Controller (Next)
- [ ] Pi Zero setup and serial communication
- [ ] Arduino command protocol over UART
- [ ] WiFi discovery and connection
- [ ] Basic pattern broadcasting
- [ ] EchoFlux integration
- [ ] Time synchronization

### Phase 3: Multi-Vehicle Sync
- [ ] Sync protocol implementation
- [ ] Leader election algorithm
- [ ] State broadcasting
- [ ] Latency compensation
- [ ] Mesh networking (optional)

### Phase 4: Advanced Features
- [ ] Mobile app (basic control)
- [ ] GPS integration
- [ ] Formation patterns
- [ ] Music visualization
- [ ] Custom pattern uploads
- [ ] Analytics dashboard

## Contributing

This is a personal project, but ideas and suggestions are welcome!

**Current Focus**: Designing the Pi controller and sync protocol

## Resources

### Related Projects
- **EchoFlux**: https://github.com/mxjxn/echoflux - Synchronized music player
- **FastLED**: https://fastled.io/ - Arduino LED library
- **WLED**: https://github.com/Aircoookie/WLED - Inspiration for WiFi LED control

### Useful Links
- [Raspberry Pi Zero W Specs](https://www.raspberrypi.com/products/raspberry-pi-zero-w/)
- [WS2812B Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf)
- [Batman-adv Mesh Protocol](https://www.open-mesh.org/projects/batman-adv/wiki)

## License

MIT License - Feel free to fork, modify, and share!

---

**Status**: 🚧 Active Development
**Last Updated**: 2025-11-22
**Next Milestone**: Pi Controller Design Document

🚴 Happy riding! 💡
