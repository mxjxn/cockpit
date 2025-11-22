# Networking Architecture

Comprehensive design for multi-vehicle synchronization using Raspberry Pi Zero controllers.

## Table of Contents
- [Overview](#overview)
- [Hardware Options](#hardware-options)
- [Communication Protocols](#communication-protocols)
- [Network Topologies](#network-topologies)
- [Synchronization Strategies](#synchronization-strategies)
- [EchoFlux Integration](#echoflux-integration)
- [Implementation Recommendations](#implementation-recommendations)

---

## Overview

### System Goals
1. **Low Latency**: Light sync within 50ms visual tolerance
2. **Reliability**: Graceful degradation when connection lost
3. **Scalability**: Support 2-20 vehicles initially
4. **Auto-Discovery**: Zero-config joining
5. **Independence**: Each vehicle works standalone
6. **Energy Efficient**: Battery-powered operation

### Architecture Principles
- **Decentralized first**: No single point of failure
- **Leader election**: For coordinated patterns only
- **Local autonomy**: Arduino works without Pi
- **Layered approach**: Multiple fallback mechanisms

---

## Hardware Options

### Raspberry Pi Models

| Model | CPU | RAM | WiFi | BT | Power | Price | Best For |
|-------|-----|-----|------|----|----|-------|----------|
| **Pi Zero W** | 1GHz 1-core | 512MB | 2.4GHz | 4.1 | ~100mA | $10 | Budget, basic sync |
| **Pi Zero 2 W** ⭐ | 1GHz 4-core | 512MB | 2.4GHz | 4.2 BLE | ~150mA | $15 | **Recommended** |
| Pi 3A+ | 1.4GHz 4-core | 512MB | 2.4/5GHz | 4.2 BLE | ~500mA | $25 | 5GHz needed |
| Pi 4 Model B (2GB) | 1.5GHz 4-core | 2GB | 2.4/5GHz | 5.0 BLE | ~600mA | $35 | Overkill, heavy |

**Recommendation**: **Raspberry Pi Zero 2 W**
- Quad-core for parallel processing (sync + music + serial)
- Low power consumption (~150mA)
- Built-in WiFi and BLE
- Small form factor
- Best price/performance ratio

### Power Considerations

**Pi Zero 2 W Power Draw**:
- Idle: ~100mA (0.5W @ 5V)
- WiFi active: ~150mA (0.75W)
- WiFi + BLE + CPU load: ~200mA (1W)

**Runtime on 12V 5Ah battery** (using buck converter):
- Pi only: ~30 hours
- Pi + LEDs (avg): 6-8 hours (LEDs dominate)
- With music (USB speaker): 4-6 hours

### Communication Hardware

#### 1. Built-in WiFi (802.11n, 2.4GHz)
**Range**: 10-30m outdoors (depends on antenna)
**Pros**:
- No extra hardware
- Mesh networking possible
- Good throughput (72Mbps theoretical)
- Can run web server for mobile app

**Cons**:
- Power hungry (~150mA)
- Range limited
- 2.4GHz congestion in cities

#### 2. Bluetooth Low Energy (BLE 4.2)
**Range**: 10-50m outdoors
**Pros**:
- Very low power (~15mA)
- Good for close-range sync
- Mobile app friendly

**Cons**:
- Limited throughput (~1Mbps)
- Complex mesh networking
- Connection limit (~7 devices)

#### 3. External LoRa Module (Optional)
**Range**: 1-5km (!)
**Hardware**: RFM95W + antenna (~$15)
**Pros**:
- Extreme range for parades/events
- Very low power
- Penetrates obstacles well

**Cons**:
- Extra hardware/cost
- Low data rate (50kbps)
- Requires SPI connection

**Recommendation**: Start with built-in WiFi + BLE, add LoRa later if needed.

---

## Communication Protocols

### Option 1: WiFi Ad-Hoc Network (Recommended for Start)

**How it works**:
1. Each Pi creates or joins ad-hoc network (`cockpit-mesh`)
2. Devices discover each other via mDNS/Avahi
3. UDP broadcast for sync messages
4. TCP for command/control

**Pros**:
- Simple to implement
- No infrastructure needed
- Good throughput for music sync
- Python libraries well-supported

**Cons**:
- All devices must be in range of each other
- No true mesh routing

**Implementation**:
```bash
# On each Pi, create ad-hoc network
sudo nmcli dev wifi connect cockpit-mesh password "your-password"

# Or create if it doesn't exist
sudo nmcli connection add type wifi ifname wlan0 con-name cockpit-adhoc \
  autoconnect yes wifi.mode adhoc wifi.ssid cockpit-mesh \
  wifi-sec.key-mgmt wpa-psk wifi-sec.psk "your-password" \
  ipv4.method shared
```

**Python code** (discovery):
```python
import socket
import json

# Broadcast sync message
sock = socket.socket(socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
message = {"type": "heartbeat", "vehicle_id": "bike_001", "battery": 12.3}
sock.sendto(json.dumps(message).encode(), ('<broadcast>', 9999))
```

### Option 2: WiFi Mesh (batman-adv)

**How it works**:
1. Install `batman-adv` kernel module
2. Each Pi becomes mesh node
3. Auto-routing through intermediate nodes
4. Extended range via multi-hop

**Pros**:
- True mesh with routing
- Extended range (hops)
- Self-healing network
- Well-tested protocol

**Cons**:
- More complex setup
- Higher latency (multi-hop)
- Requires kernel module

**Implementation**:
```bash
# Install batman-adv
sudo apt install batctl

# Configure mesh
sudo batctl if add wlan0
sudo ip link set up dev bat0
sudo ip addr add 192.168.199.1/24 dev bat0
```

**Use case**: Large groups (>10 vehicles), parades, extended range needed

### Option 3: Bluetooth Low Energy (BLE) Mesh

**How it works**:
1. BLE advertising for discovery
2. GATT characteristics for data exchange
3. Custom mesh protocol (BLE Mesh spec is complex)

**Pros**:
- Very low power
- Good for close formation
- Mobile app integration easy

**Cons**:
- Limited range
- Connection limits
- Complex mesh implementation

**Python library**: `bleak` (BLE abstraction)

```python
from bleak import BleakClient, BleakScanner

# Scan for nearby vehicles
async def scan():
    devices = await BleakScanner.discover()
    for d in devices:
        if d.name and d.name.startswith("COCKPIT_"):
            print(f"Found vehicle: {d.name}")
```

**Use case**: Close formations, battery-critical scenarios

### Option 4: Hybrid WiFi + BLE

**Recommended Approach**:
- **WiFi**: Music sync, large data, web server
- **BLE**: Quick discovery, beacon heartbeats, fallback

**How it works**:
1. BLE advertising for discovery (<1 sec)
2. Once discovered, exchange WiFi credentials via BLE
3. Switch to WiFi for data-heavy tasks
4. BLE heartbeat continues for proximity detection

**Best of both worlds**: Fast discovery + high throughput

---

## Network Topologies

### Topology 1: Star (Leader-Follower)

```
        ┌──────────┐
        │  Leader  │  (elected or manual)
        │  (Bike1) │
        └────┬─────┘
      ┌──────┼──────┐
      │      │      │
   ┌──▼──┐ ┌─▼───┐ ┌▼────┐
   │Bike2│ │Bike3│ │Bike4│
   └─────┘ └─────┘ └─────┘
```

**Pros**:
- Simple synchronization (one source of truth)
- Low latency (single hop)
- Clear authority

**Cons**:
- Leader must be in range of all
- Single point of failure

**Best for**: Small groups (2-5 bikes), coordinated shows

### Topology 2: Mesh (Peer-to-Peer)

```
   ┌─────┐     ┌─────┐
   │Bike1│◄───►│Bike2│
   └──┬──┘     └──┬──┘
      │   ╲   ╱   │
      │    ╳     │
      │   ╱  ╲   │
   ┌──▼──┐     ┌─▼───┐
   │Bike3│◄───►│Bike4│
   └─────┘     └─────┘
```

**Pros**:
- No single point of failure
- Extended range (multi-hop)
- Self-healing

**Cons**:
- More complex sync logic
- Higher latency
- Potential loops/conflicts

**Best for**: Large groups, parades, dynamic formations

### Topology 3: Hierarchical

```
      ┌──────────┐
      │  Leader  │
      │ (Bike1)  │
      └────┬─────┘
     ┌─────┴─────┐
     │           │
  ┌──▼──┐     ┌──▼──┐
  │Sub1 │     │Sub2 │  (sub-leaders)
  │Bike2│     │Bike4│
  └──┬──┘     └──┬──┘
     │           │
  ┌──▼──┐     ┌──▼──┐
  │Bike3│     │Bike5│
  └─────┘     └─────┘
```

**Pros**:
- Scalable to many bikes
- Load distribution
- Fault tolerance (sub-leaders)

**Cons**:
- Complex leader election
- Increased latency for leaves

**Best for**: Very large groups (>20 bikes)

---

## Synchronization Strategies

### Strategy 1: Leader Broadcast (Simplest)

**How it works**:
1. Leader elected (first to advertise, or manual)
2. Leader broadcasts state every 100ms
3. Followers receive and apply immediately
4. Heartbeat timeout → re-election

**Message format**:
```json
{
  "type": "sync",
  "timestamp": 1234567890.123,
  "leader_id": "bike_001",
  "pattern": "rainbow_wave",
  "hue_offset": 127,
  "music_position_ms": 45230
}
```

**Pros**: Dead simple, works great for <10 bikes
**Cons**: Leader must reach everyone, no consensus

### Strategy 2: Lamport Clock (Eventual Consistency)

**How it works**:
1. Each node has logical clock
2. Increment clock on each event
3. Include clock in all messages
4. Resolve conflicts by clock value

**Use case**: Distributed decision-making, no clear leader

**Python pseudo-code**:
```python
class LamportClock:
    def __init__(self):
        self.time = 0

    def tick(self):
        self.time += 1
        return self.time

    def receive(self, msg_time):
        self.time = max(self.time, msg_time) + 1
```

### Strategy 3: NTP Time Sync (Precise Coordination)

**How it works**:
1. All Pis sync to internet NTP server (before ride)
2. Or one Pi acts as local NTP server
3. Absolute timestamps for music position
4. Drift compensation algorithm

**Setup**:
```bash
# On leader Pi (acts as NTP server)
sudo apt install ntp
# Configure as server

# On follower Pis
sudo ntpdate 192.168.199.1  # leader IP
```

**Accuracy**: ~10ms sync possible
**Best for**: Music synchronization where tight timing matters

### Strategy 4: Phase-Locked Loop (Audio Sync)

**How it works**:
1. Leader broadcasts music position + timestamp
2. Followers adjust playback speed slightly to catch up
3. Continuous adjustment maintains lock

**Python pseudo-code**:
```python
class PhaseLockSync:
    def __init__(self, target_latency_ms=50):
        self.target = target_latency_ms

    def adjust_speed(self, leader_pos, local_pos, latency):
        error = (leader_pos - latency) - local_pos
        speed = 1.0 + (error / 1000.0)  # +/- 10% max
        return clamp(speed, 0.9, 1.1)
```

**Pros**: Smooth sync, handles network jitter
**Cons**: Requires audio framework that supports speed adjust

---

## EchoFlux Integration

### Architecture

```
┌─────────────────────────────────────┐
│  Raspberry Pi                       │
│                                     │
│  ┌──────────────┐  ┌─────────────┐ │
│  │  Sync Daemon │  │  EchoFlux   │ │
│  │  (Python)    │◄─┤  (Music)    │ │
│  └──────┬───────┘  └─────────────┘ │
│         │                           │
│    ┌────▼────┐      ┌───────────┐  │
│    │ Arduino │      │  Speaker  │  │
│    │ Control │      │  (USB)    │  │
│    └─────────┘      └───────────┘  │
└─────────────────────────────────────┘
         ║ WiFi Mesh ║
┌────────▼───────────▼────────┐
│   Other Bikes (same stack)  │
└─────────────────────────────┘
```

### Communication Flow

**Scenario**: Leader plays song, followers join

1. **Leader starts EchoFlux**:
   - Song starts playing
   - Sync daemon queries EchoFlux API: `GET /api/playback/status`
   - Receives: `{"position_ms": 12345, "track": "song.mp3", "playing": true}`

2. **Leader broadcasts**:
   ```json
   {
     "type": "music_sync",
     "timestamp": 1234567890.123,
     "track": "song.mp3",
     "position_ms": 12345,
     "bpm": 120,
     "beat_phase": 0.25
   }
   ```

3. **Followers receive**:
   - Check if they have `song.mp3`
   - If yes: Seek to position_ms + latency compensation
   - If no: Request file from leader, or skip

4. **Continuous sync**:
   - Leader broadcasts every 500ms
   - Followers adjust playback speed to stay locked
   - LED patterns sync to beat_phase

### EchoFlux API Integration

Assuming EchoFlux provides HTTP REST API:

```python
import aiohttp
import asyncio

class EchoFluxClient:
    def __init__(self, base_url="http://localhost:5000"):
        self.base = base_url

    async def get_playback_status(self):
        async with aiohttp.ClientSession() as session:
            async with session.get(f"{self.base}/api/playback/status") as resp:
                return await resp.json()

    async def seek(self, position_ms):
        async with aiohttp.ClientSession() as session:
            data = {"position_ms": position_ms}
            async with session.post(f"{self.base}/api/playback/seek", json=data) as resp:
                return await resp.json()

    async def play_track(self, track_path):
        async with aiohttp.ClientSession() as session:
            data = {"track": track_path}
            async with session.post(f"{self.base}/api/playback/play", json=data) as resp:
                return await resp.json()
```

### Music-Reactive LED Patterns

**Option A**: Beat detection
- Extract BPM from audio (librosa)
- Send beat events to Arduino
- Arduino flashes on beat

**Option B**: Frequency bands
- FFT analysis (low, mid, high)
- Map to LED brightness
- Real-time reactive

**Option C**: Pre-analyzed
- Analyze song offline
- Store beat map
- Playback synced to beat map

---

## Implementation Recommendations

### Phase 1: Proof of Concept (1-2 weeks)

**Goal**: Get 2 bikes syncing a simple pattern

**Steps**:
1. Set up 2x Pi Zero 2 W
2. Install Raspberry Pi OS Lite
3. Create WiFi ad-hoc network
4. Write Python script:
   - UDP broadcast heartbeat
   - Receive heartbeats
   - Send pattern state
5. Serial communication to Arduino:
   - Simple protocol: `PATTERN:rainbow\n`
   - Arduino parses and changes mode
6. **Demo**: Both bikes flash same color in sync

**Hardware needed**:
- 2x Pi Zero 2 W
- 2x microSD cards (16GB)
- 2x USB power cables (for testing)
- Existing Arduino setups

**Code estimate**: ~300 lines Python

---

### Phase 2: Leader Election + Music (2-3 weeks)

**Goal**: Auto-discovery, leader election, basic music sync

**Steps**:
1. Implement mDNS discovery (Avahi)
2. Leader election (Bully algorithm or Raft-lite)
3. Install EchoFlux on Pis
4. Sync daemon queries EchoFlux API
5. Broadcast music position
6. Followers seek to position

**Demo**: 3+ bikes play same song in sync

---

### Phase 3: Mesh + Mobile App (4-6 weeks)

**Goal**: Production-ready multi-vehicle system

**Steps**:
1. Implement batman-adv mesh (if needed)
2. Web server on Pi for mobile app
3. React Native or Flutter app
4. Pattern library and upload
5. GPS integration (optional)
6. Analytics/logging

---

### Recommended Tech Stack (Python)

```python
# Core libraries
aiohttp          # Async HTTP for EchoFlux API
pyserial-asyncio # Async serial to Arduino
python-osc       # OSC messages for music sync (alternative)
zeroconf         # mDNS service discovery
msgpack          # Efficient binary serialization

# Optional
bleak            # BLE support
librosa          # Audio analysis
numpy            # FFT for music reactive

# Utilities
python-dotenv    # Configuration
loguru           # Better logging
```

### Project Structure (Python)

```
pi-controller/
├── main.py              # Entry point
├── requirements.txt     # Dependencies
├── config.yaml          # Configuration
├── sync/
│   ├── __init__.py
│   ├── discovery.py     # mDNS, BLE discovery
│   ├── leader.py        # Leader election
│   ├── protocol.py      # Message format
│   └── mesh.py          # Mesh routing (if needed)
├── echoflux/
│   ├── __init__.py
│   ├── client.py        # API client
│   └── sync.py          # Music position sync
├── arduino/
│   ├── __init__.py
│   ├── serial_comm.py   # UART protocol
│   └── commands.py      # Command formatting
├── patterns/
│   ├── __init__.py
│   ├── library.py       # Pattern definitions
│   └── generator.py     # Pattern to LED data
└── web/
    ├── api.py           # REST API for mobile app
    └── static/          # Mobile web app
```

---

## Security Considerations

### Threat Model
- **Spoofing**: Fake "leader" vehicle sends malicious patterns
- **DoS**: Flood network with sync messages
- **Eavesdropping**: Sniff WiFi to see patterns

### Mitigations
1. **Pre-shared key**: All vehicles share secret key
2. **Message signing**: HMAC-SHA256 for each message
3. **Rate limiting**: Ignore vehicles sending >10 msg/sec
4. **Whitelist**: Only accept known vehicle IDs

**Simple implementation**:
```python
import hmac
import hashlib

SECRET_KEY = b"your-secret-key-here"

def sign_message(data):
    signature = hmac.new(SECRET_KEY, data.encode(), hashlib.sha256).hexdigest()
    return {"data": data, "signature": signature}

def verify_message(msg):
    expected = hmac.new(SECRET_KEY, msg["data"].encode(), hashlib.sha256).hexdigest()
    return hmac.compare_digest(expected, msg["signature"])
```

---

## Performance Targets

| Metric | Target | Critical? |
|--------|--------|-----------|
| Discovery time | <2 sec | No |
| Sync latency | <50ms | Yes (LEDs) |
| Music drift | <100ms | Yes (audio) |
| Battery life (Pi) | >6 hours | Moderate |
| Range (WiFi) | >20m | Moderate |
| Max vehicles | 20+ | No (start small) |

---

## Open Questions / Decisions Needed

1. **Pi Model**: Pi Zero 2 W confirmed? Or test Pi 3A+ for 5GHz?
2. **EchoFlux API**: Does it exist? Need to check repo or build one
3. **Mesh vs Ad-Hoc**: Start simple (ad-hoc) or go straight to batman-adv?
4. **BLE or WiFi first**: Which to prototype first?
5. **Leader election**: Manual or automatic? Algorithm preference?
6. **Music file sync**: How to share tracks between bikes? USB? WiFi transfer?
7. **Power budget**: Need to measure actual Pi + Arduino + LED draw
8. **Enclosure**: Waterproof case for Pi? Mounting strategy?

---

## Next Steps

1. **Research EchoFlux**: Check current API capabilities at github.com/mxjxn/echoflux
2. **Prototype hardware**: Get 2x Pi Zero 2 W + accessories
3. **Test range**: Measure WiFi range in outdoor conditions
4. **Arduino protocol**: Define serial command set (next doc)
5. **Write sync protocol spec**: Message formats, state machine
6. **Prototype discovery**: Simple UDP broadcast test

---

**Document Status**: 🚧 Draft for Discussion
**Last Updated**: 2025-11-22
**Author**: AI Assistant (with user collaboration)

**Feedback needed on**:
- Preferred network topology
- WiFi vs BLE priority
- EchoFlux integration feasibility
- Power/battery constraints
