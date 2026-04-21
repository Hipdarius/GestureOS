# GestureOS Wiring Reference

## Pin Map

| ESP32 Pin | Component           | Protocol  | Notes                                |
|-----------|---------------------|-----------|--------------------------------------|
| GPIO 21   | MPU-6050 SDA        | I2C Data  | `Wire.h` default SDA                 |
| GPIO 22   | MPU-6050 SCL        | I2C Clock | `Wire.h` default SCL                 |
| GPIO 34   | Flex Sensor 1       | ADC1      | Input-only pin, no internal pull-up  |
| GPIO 35   | Flex Sensor 2       | ADC1      | Input-only pin, no internal pull-up  |
| GPIO 18   | Servo 1 (Pan)       | PWM       | Signal only — 5V power from buck     |
| GPIO 19   | Servo 2 (Tilt)      | PWM       | Signal only — 5V power from buck     |
| GPIO 16   | Vibration Motor     | Digital   | Through 2N2222 NPN + flyback diode   |
| GPIO 25   | RGB LED — Red       | PWM       | 220 Ω series resistor                |
| GPIO 26   | RGB LED — Green     | PWM       | 220 Ω series resistor                |
| GPIO 27   | RGB LED — Blue      | PWM       | 220 Ω series resistor                |
| 3V3       | MPU-6050, Flex Vcc  | Power     | ESP32 regulator                      |
| 5V (buck) | Servos, Motor       | Power     | From 9.7V LiPo → buck converter      |
| GND       | All components      | Ground    | Single common rail                   |

## Circuit Notes

### Flex sensors
Each flex sensor forms a voltage divider with a 10 kΩ pull-down:

```
3V3 ──[ Flex ]──┬── GPIO 34 (or 35)
                │
             [10 kΩ]
                │
               GND
```

Bending the flex sensor raises its resistance → the divider drops the voltage
on the GPIO pin.

### Vibration motor
GPIO cannot drive a motor directly (≈12 mA max on ESP32).

```
          5V rail
             │
         [ Motor ]──┐
             │      │
   1N4007 ◀──┘      │    (flyback, cathode → 5V)
             │      │
           Collector
             │
  GPIO 16 ──[1 kΩ]── Base   (2N2222 NPN)
             │
          Emitter
             │
            GND
```

### RGB LED (common cathode)

```
GPIO 25 ──[220 Ω]── R anode ┐
GPIO 26 ──[220 Ω]── G anode ├─ LED
GPIO 27 ──[220 Ω]── B anode ┘
                   Common cathode ── GND
```

### Servos (SG90)
- **Power from the 5V buck output**, NOT from the ESP32's 3V3 or 5V pin.
- Servo signal line is 3.3V-compatible.
- Add a 100 µF electrolytic across the 5V rail near the servos to absorb inrush.

### Power
```
9.7V LiPo ──▶ Buck Converter ──▶ 5V rail
                                   ├── ESP32 VIN
                                   ├── Servos (Vcc)
                                   └── Motor (via transistor)

ESP32 3V3 regulator ──▶ MPU-6050 Vcc, Flex sensor Vcc
```

## ADC Caveat

GPIO 34 and 35 are on **ADC1** — they keep working while WiFi is active.
ADC2 pins (GPIO 0, 2, 4, 12–15, 25–27) cannot be used for `analogRead()` when
WiFi is running. GPIO 25/26/27 are fine here because we only use them as PWM
**outputs**, not ADC inputs.
