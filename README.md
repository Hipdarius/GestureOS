# GestureOS

A sensor-equipped glove that recognizes hand gestures and controls household devices.
The ESP32 brain reads an IMU and two flex sensors, classifies gestures in real time,
drives a pan-tilt arm, and streams everything to a browser dashboard served directly
from its own WiFi Access Point. **No internet, no cloud, no phone app.**

> TEINN 2025/2026 project — **Nils Oestrreicher, Darius Ferent, Alyssia Varela Fortes**

## Hardware

- ESP32 DevKit (ESP-WROOM-32)
- MPU-6050 IMU (I2C)
- 2× flex sensors + 2× 10 kΩ pull-down resistors
- 2× SG90 servos on a pan-tilt bracket
- RGB LED (common cathode) + 3× 220 Ω resistors
- Small vibration motor + 2N2222 NPN + 1 kΩ base resistor + 1N4007 flyback diode
- 9.7V LiPo → buck converter → 5V rail
- Breadboard, Dupont jumpers, glove

See [`docs/wiring.md`](docs/wiring.md) for the full pin map and circuit notes.

## Try the dashboard without any hardware

You can preview the full dashboard before any components arrive — no ESP32, no
glove, nothing to wire up. Just open `data/index.html` in any modern browser
(Chrome, Edge, Firefox). The dashboard auto-detects that it has no WebSocket to
talk to and switches into **DEMO mode**, generating simulated sensor data that
cycles through every gesture: tilts in all four directions, GRAB, and POINT.
A yellow `DEMO` badge appears in the header so it's clear the data is fake.

Once a real ESP32 is reachable, the same page connects automatically and the
DEMO badge disappears.

## Software setup (with hardware)

1. Install [PlatformIO](https://platformio.org/) (VS Code extension is easiest).
2. Clone and enter the repo:
   ```bash
   git clone https://github.com/Hipdarius/GestureOS.git
   cd GestureOS
   ```
3. Build and flash the firmware:
   ```bash
   pio run --target upload
   ```
4. Upload the dashboard (HTML/CSS/JS) to the ESP32's SPIFFS partition:
   ```bash
   pio run --target uploadfs
   ```
5. Open serial monitor to see boot logs:
   ```bash
   pio device monitor
   ```

## Using it

1. Power the ESP32.
2. On your phone or laptop, connect to the WiFi network **`GestureOS`** — password
   `gesture123`.
3. Open `http://192.168.4.1` in a browser.
4. You should see the 3D hand visualization tracking the glove in real time, the
   gesture log updating on each gesture change, and the telemetry bars moving.

## Gesture reference

| ID | Name        | Condition                                | LED   |
|----|-------------|------------------------------------------|-------|
| 0  | NONE        | neutral zone                             | off   |
| 1  | TILT_LEFT   | roll < −30°, fingers relaxed             | blue  |
| 2  | TILT_RIGHT  | roll > +30°, fingers relaxed             | blue  |
| 3  | TILT_UP     | pitch < −30°, fingers relaxed            | cyan  |
| 4  | TILT_DOWN   | pitch > +30°, fingers relaxed            | cyan  |
| 5  | GRAB        | both flex > 70% (closed fist)            | red   |
| 6  | POINT       | flex1 < 30% AND flex2 > 60% (index out)  | green |

A gesture must persist for **2 consecutive cycles** (~20 ms) to be confirmed,
and a **10° hysteresis margin** prevents flapping at the boundary.

## Calibration

1. Uncomment `#define DEBUG_SERIAL` in `src/config.h` and flash.
2. Watch the serial monitor. Straighten each finger: note the `F1`/`F2` raw
   values. Bend fully: note the bent values.
3. Update `FLEX_RAW_MIN` / `FLEX_RAW_MAX` in `src/config.h`.
4. At boot the MPU-6050 runs an automatic gyroscope-bias calibration — keep the
   glove **still** for the first second after power-on.

## Project layout

```
GestureOS/
├── platformio.ini
├── src/
│   ├── main.cpp
│   ├── config.h
│   ├── IMUHandler.{h,cpp}
│   ├── FlexReader.{h,cpp}
│   ├── GestureClassifier.{h,cpp}
│   ├── ServoController.{h,cpp}
│   ├── FeedbackManager.{h,cpp}
│   └── WebManager.{h,cpp}
├── data/                # SPIFFS (upload with `pio run -t uploadfs`)
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   └── three.min.js
├── test/
│   └── test_gestures.cpp
├── docs/
│   └── wiring.md
└── README.md
```

## Tests

Gesture classifier has a native Unity test suite:

```bash
pio test -e native
```

## Credits

- [jrowberg/I2Cdevlib-MPU6050](https://github.com/jrowberg/i2cdevlib)
- [links2004/WebSockets](https://github.com/Links2004/arduinoWebSockets)
- [madhephaestus/ESP32Servo](https://github.com/madhephaestus/ESP32Servo)
- [bblanchon/ArduinoJson](https://arduinojson.org)
- [Three.js](https://threejs.org)

## License

MIT
