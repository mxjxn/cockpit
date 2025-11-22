# Synchronization Protocol Specification

Message format and state machine for multi-vehicle LED and music synchronization.

## Version

**Protocol Version**: 0.1.0 (Draft)
**Date**: 2025-11-22

---

## Design Goals

1. **Compact**: Minimize bandwidth (target <100 bytes per message)
2. **Extensible**: Easy to add new message types
3. **Self-describing**: Include version and type in every message
4. **Human-readable**: JSON for development, MessagePack for production
5. **Stateless**: Each message contains full state (idempotent)

---

## Message Format

### Envelope (All Messages)

```json
{
  "version": "0.1.0",
  "type": "message_type",
  "sender_id": "bike_001",
  "timestamp": 1234567890.123,
  "sequence": 42,
  "signature": "optional_hmac_sha256",
  "payload": { ... }
}
```

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `version` | string | Yes | Protocol version (semver) |
| `type` | string | Yes | Message type (see below) |
| `sender_id` | string | Yes | Unique vehicle ID |
| `timestamp` | float | Yes | Unix timestamp with milliseconds |
| `sequence` | int | Yes | Monotonic counter per sender |
| `signature` | string | No | HMAC-SHA256 for security |
| `payload` | object | Yes | Type-specific data |

---

## Message Types

### 1. HEARTBEAT

**Purpose**: Announce presence and basic status

**Frequency**: Every 1-2 seconds

**Payload**:
```json
{
  "status": "active",
  "battery_voltage": 12.3,
  "uptime_sec": 3600,
  "capabilities": ["led", "music", "gps"],
  "role": "follower",
  "ip_address": "192.168.199.5"
}
```

**Fields**:
- `status`: "active", "idle", "error"
- `battery_voltage`: Float (volts)
- `uptime_sec`: Integer (seconds since boot)
- `capabilities`: List of features
- `role`: "leader", "follower", "standalone"
- `ip_address`: For direct TCP connection

---

### 2. PATTERN_SYNC

**Purpose**: Broadcast LED pattern state from leader

**Frequency**: 10 Hz (every 100ms) when active

**Payload**:
```json
{
  "pattern_id": "rainbow_wave",
  "parameters": {
    "hue_offset": 127,
    "speed": 1.5,
    "brightness": 200
  },
  "frame": 42,
  "duration_ms": 5000
}
```

**Fields**:
- `pattern_id`: String identifier from pattern library
- `parameters`: Dict of pattern-specific params
- `frame`: Current animation frame number
- `duration_ms`: How long to run pattern (0 = infinite)

---

### 3. MUSIC_SYNC

**Purpose**: Synchronize music playback

**Frequency**: 2 Hz (every 500ms) when playing

**Payload**:
```json
{
  "track_id": "song_123",
  "track_hash": "sha256_of_file",
  "position_ms": 45230,
  "playing": true,
  "bpm": 120,
  "beat_phase": 0.25,
  "volume": 75
}
```

**Fields**:
- `track_id`: Unique track identifier (filename or hash)
- `track_hash`: SHA256 of audio file (verify same version)
- `position_ms`: Current playback position (milliseconds)
- `playing`: Boolean play/pause state
- `bpm`: Beats per minute (for visualization)
- `beat_phase`: 0.0-1.0, position within current beat
- `volume`: 0-100

---

### 4. LEADER_ELECTION

**Purpose**: Coordinate leader election

**Frequency**: As needed (triggered events)

**Payload**:
```json
{
  "action": "announce_candidacy",
  "priority": 100,
  "reason": "manual_override"
}
```

**Actions**:
- `announce_candidacy`: "I want to be leader"
- `accept_leader`: "I recognize you as leader"
- `challenge`: "My priority is higher"
- `resign`: "I'm stepping down"

**Fields**:
- `action`: One of above
- `priority`: Integer (higher = more likely to lead)
  - Manual override: 1000
  - Plugged into power: 500
  - Highest battery: 100-200
  - Random: 0-99
- `reason`: String (for debugging)

---

### 5. COMMAND

**Purpose**: Send imperative commands (mobile app, override)

**Frequency**: Event-driven (user input)

**Payload**:
```json
{
  "target": "all",
  "command": "set_brightness",
  "args": {
    "brightness": 150
  }
}
```

**Common Commands**:
- `set_brightness`: Adjust LED brightness
- `emergency_stop`: Turn off all LEDs
- `reboot`: Restart controller
- `sync_time`: Force NTP sync
- `upload_pattern`: New pattern definition

**Fields**:
- `target`: "all", "leader", specific vehicle ID
- `command`: Command name
- `args`: Command-specific arguments

---

### 6. FILE_TRANSFER

**Purpose**: Share music tracks or patterns

**Frequency**: As needed (rare)

**Payload**:
```json
{
  "file_id": "song_123",
  "filename": "coolsong.mp3",
  "size_bytes": 5242880,
  "chunk_index": 0,
  "total_chunks": 100,
  "data": "base64_encoded_chunk",
  "hash": "sha256_of_chunk"
}
```

**Note**: Large files use chunking. Consider switching to TCP for file transfer.

---

### 7. GPS_UPDATE (Optional)

**Purpose**: Share location for formations

**Frequency**: 1 Hz when moving

**Payload**:
```json
{
  "latitude": 37.7749,
  "longitude": -122.4194,
  "altitude_m": 10.5,
  "speed_mps": 5.2,
  "heading_deg": 270
}
```

---

### 8. ERROR

**Purpose**: Report errors and warnings

**Frequency**: Event-driven

**Payload**:
```json
{
  "severity": "warning",
  "code": "battery_low",
  "message": "Battery voltage below threshold",
  "details": {
    "current_voltage": 10.2,
    "threshold": 10.5
  }
}
```

**Severity Levels**: "info", "warning", "error", "critical"

---

## State Machine

### Vehicle States

```
     ┌──────────┐
     │  BOOT    │
     └────┬─────┘
          │
          ▼
     ┌──────────┐
     │STANDALONE│◄───────┐
     └────┬─────┘        │
          │              │
    Discovery            │
          │         Lost Connection
          ▼              │
     ┌──────────┐        │
     │SEARCHING │────────┘
     └────┬─────┘
          │
    Found Peer(s)
          │
          ▼
     ┌──────────┐
     │CONNECTED │
     └────┬─────┘
          │
   Leader Election
          │
     ┌────┴────┐
     │         │
     ▼         ▼
┌────────┐ ┌─────────┐
│ LEADER │ │FOLLOWER │
└────────┘ └─────────┘
```

### State Transitions

| From | To | Trigger | Action |
|------|-----|---------|--------|
| BOOT | STANDALONE | Startup complete | Begin heartbeat |
| STANDALONE | SEARCHING | Enable discovery | Start scanning |
| SEARCHING | STANDALONE | Timeout (30s) | Return to solo mode |
| SEARCHING | CONNECTED | Peer found | Exchange info |
| CONNECTED | LEADER | Won election | Start broadcasting |
| CONNECTED | FOLLOWER | Lost election | Start listening |
| LEADER | FOLLOWER | Higher priority peer | Step down |
| FOLLOWER | LEADER | Leader timeout | Trigger election |
| ANY | STANDALONE | Manual override | Disconnect |

---

## Leader Election Algorithm

### Bully Algorithm (Simple)

**When to trigger**:
- No leader detected (timeout)
- Current leader announces resign
- Manual trigger

**Process**:
1. Candidate broadcasts `LEADER_ELECTION` with its priority
2. If no higher priority responds in 2 seconds → become leader
3. If higher priority responds → become follower
4. Leader broadcasts `HEARTBEAT` with `role: "leader"`

**Priority Calculation**:
```python
def calculate_priority(vehicle):
    priority = 0
    if vehicle.manual_leader:
        priority += 1000
    if vehicle.plugged_in:
        priority += 500
    priority += int(vehicle.battery_voltage * 10)  # 100-140
    priority += random.randint(0, 99)  # tiebreaker
    return priority
```

**Timeout**: 5 seconds without leader heartbeat → trigger election

---

## Synchronization Timing

### Clock Synchronization

**Method 1: NTP (Internet Available)**
```bash
# One-time sync before ride
sudo ntpdate pool.ntp.org
```

**Method 2: Local NTP Server**
```
Leader Pi runs chrony as NTP server
Followers sync to leader's IP
```

**Method 3: Broadcast Sync (No NTP)**
```
Leader includes timestamp in PATTERN_SYNC
Followers estimate clock offset
Adjust local time gradually
```

### Latency Compensation

**Measure network latency**:
```python
# Ping-pong test
send_time = time.time()
send_message("PING")
# ... receive PONG ...
latency_ms = (time.time() - send_time) * 1000 / 2
```

**Apply compensation**:
```python
# When receiving music position
leader_position_ms = msg["position_ms"]
network_latency_ms = 25  # measured average
current_position_ms = leader_position_ms + network_latency_ms
```

---

## Frame Synchronization

### For LED Patterns

**Goal**: All bikes show same frame within 50ms

**Algorithm**:
```python
class FrameSync:
    def __init__(self, fps=10):
        self.fps = fps
        self.frame_duration_ms = 1000 / fps

    def get_current_frame(self, start_time):
        elapsed_ms = (time.time() - start_time) * 1000
        return int(elapsed_ms / self.frame_duration_ms)

# Leader
frame = get_current_frame(pattern_start_time)
broadcast_pattern_sync(frame=frame)

# Follower
pattern_start_time = calculate_start_time(msg)
local_frame = get_current_frame(pattern_start_time)
render_frame(local_frame)
```

### For Music Sync

**Phase-locked loop**:
```python
class MusicSync:
    def __init__(self, target_latency_ms=50):
        self.target = target_latency_ms
        self.speed = 1.0

    def update(self, leader_pos_ms, local_pos_ms, latency_ms):
        expected_local = leader_pos_ms - latency_ms
        error_ms = expected_local - local_pos_ms

        # Adjust playback speed to converge
        if abs(error_ms) > 200:
            # Big error: seek
            seek_to(expected_local)
            self.speed = 1.0
        else:
            # Small error: adjust speed
            self.speed = 1.0 + (error_ms / 10000.0)
            self.speed = clamp(self.speed, 0.95, 1.05)

        set_playback_speed(self.speed)
```

---

## Network Transport

### UDP Broadcast (Default)

**Port**: 9999
**Broadcast Address**: 192.168.199.255 (ad-hoc network)

**Pros**: Simple, no connection management
**Cons**: No delivery guarantee, no ordering

**Use for**: HEARTBEAT, PATTERN_SYNC, MUSIC_SYNC

### TCP (Optional)

**Port**: 9998
**Use for**: FILE_TRANSFER, COMMAND (critical)

**Pros**: Reliable delivery, ordered
**Cons**: Connection overhead, slower

### Message Size Limits

| Transport | Max Size | Recommended |
|-----------|----------|-------------|
| UDP | 508 bytes | <200 bytes |
| TCP | 64 KB | <4 KB |

**For large payloads**: Chunk or switch to TCP

---

## Example: Pattern Sync Flow

### Leader Side

```python
import asyncio
import json
import socket
import time

async def pattern_sync_loop():
    sock = socket.socket(socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

    pattern_start = time.time()
    sequence = 0

    while True:
        frame = int((time.time() - pattern_start) * 10)  # 10 FPS

        msg = {
            "version": "0.1.0",
            "type": "PATTERN_SYNC",
            "sender_id": "bike_001",
            "timestamp": time.time(),
            "sequence": sequence,
            "payload": {
                "pattern_id": "rainbow_wave",
                "parameters": {"hue_offset": (frame * 10) % 256},
                "frame": frame,
                "duration_ms": 0
            }
        }

        data = json.dumps(msg).encode()
        sock.sendto(data, ('<broadcast>', 9999))

        sequence += 1
        await asyncio.sleep(0.1)  # 10 Hz
```

### Follower Side

```python
async def pattern_receive_loop():
    sock = socket.socket(socket.SOCK_DGRAM)
    sock.bind(('', 9999))

    while True:
        data, addr = sock.recvfrom(1024)
        msg = json.loads(data.decode())

        if msg["type"] == "PATTERN_SYNC":
            pattern_id = msg["payload"]["pattern_id"]
            params = msg["payload"]["parameters"]
            frame = msg["payload"]["frame"]

            # Send command to Arduino
            await send_arduino_command(f"PATTERN:{pattern_id},{params}\n")
```

---

## Security

### Message Signing (Optional)

```python
import hmac
import hashlib

SECRET_KEY = b"shared-secret-key"

def sign_message(msg_dict):
    msg_json = json.dumps(msg_dict, sort_keys=True)
    signature = hmac.new(SECRET_KEY, msg_json.encode(), hashlib.sha256).hexdigest()
    msg_dict["signature"] = signature
    return msg_dict

def verify_message(msg_dict):
    claimed_sig = msg_dict.pop("signature", None)
    if not claimed_sig:
        return False

    msg_json = json.dumps(msg_dict, sort_keys=True)
    expected_sig = hmac.new(SECRET_KEY, msg_json.encode(), hashlib.sha256).hexdigest()
    return hmac.compare_digest(expected_sig, claimed_sig)
```

---

## Error Handling

### Packet Loss

**Strategy**: Send critical state redundantly

**Example**: Leader sends PATTERN_SYNC every 100ms
- Follower can miss 1-2 packets without visible glitch
- After 500ms (5 missed), re-sync or freeze

### Clock Drift

**Strategy**: Periodic re-sync

**Example**: Every 60 seconds, follower measures offset
```python
# Leader includes precise timestamp
leader_time = msg["timestamp"]
receive_time = time.time()
offset = leader_time - receive_time

# Gradually adjust clock
adjust_clock(offset)
```

### Leader Failure

**Strategy**: Automatic re-election

**Timeout**: 5 seconds without leader HEARTBEAT
**Action**: Trigger LEADER_ELECTION

---

## Performance Metrics

### Bandwidth Usage (10 vehicles)

| Message Type | Size | Frequency | Bandwidth |
|--------------|------|-----------|-----------|
| HEARTBEAT | 150 B | 1 Hz | 1.5 KB/s |
| PATTERN_SYNC | 200 B | 10 Hz | 2 KB/s |
| MUSIC_SYNC | 180 B | 2 Hz | 360 B/s |
| **Total** | | | **~4 KB/s** |

**With 10 vehicles**: 40 KB/s (0.3 Mbps) → Well within WiFi capacity

---

## Future Extensions

### Version 0.2.0 Ideas
- [ ] Compressed binary format (MessagePack)
- [ ] Multi-cast groups (subsets of vehicles)
- [ ] Formation patterns (GPS-based)
- [ ] Mesh routing headers
- [ ] Encryption (AES-GCM)

### Version 1.0.0 Ideas
- [ ] WebRTC for low-latency peer connections
- [ ] Voice commands (wake word → pattern change)
- [ ] Sensor fusion (IMU for synchronized tricks)

---

## Implementation Checklist

### Minimum Viable Protocol
- [x] Message envelope defined
- [x] HEARTBEAT message
- [x] PATTERN_SYNC message
- [ ] Leader election algorithm
- [ ] UDP broadcast implementation
- [ ] Clock sync (NTP or local)
- [ ] Frame sync logic
- [ ] Error handling (timeouts)

### Production Ready
- [ ] Message signing
- [ ] Encryption
- [ ] File transfer
- [ ] TCP fallback
- [ ] Metrics/monitoring
- [ ] Protocol version negotiation

---

**Document Status**: 🚧 Draft Specification
**Next Steps**:
1. Prototype UDP broadcast with HEARTBEAT
2. Test latency and packet loss in real conditions
3. Implement frame sync algorithm
4. Refine based on testing

**Feedback Needed**:
- Message format: JSON vs MessagePack?
- Security: Required for MVP or later?
- Leader election: Bully vs Raft?
- Clock sync: NTP mandatory or optional?
