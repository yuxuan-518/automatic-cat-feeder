# Motor and Automation Development

**[Project Overview](../README.md) · [Mechanical Design](mechanical-design.md) · [Motor & Automation](motor-and-automation.md) · [Food Preservation](food-preservation.md)**

This portion was developed by [Shaoxuan Xiong](https://github.com/yuxuan-518). The automation system progressed through two motor configurations because correct control signals alone did not guarantee sufficient mechanical torque under the feeder's physical load.

## Iteration 1 — Dual NEMA17 Test System

The first controller was a standalone PlatformIO project for the Arduino Mega 2560, written in register-level AVR C rather than as an Arduino `.ino` sketch. Two independent stepper channels used hardware timers to generate STEP signals without requiring the CPU to toggle every pulse in the main loop.

| Channel | STEP | DIR | Timer |
| --- | --- | --- | --- |
| Motor 1 | D11 / PB5 / OC1A | D12 / PB6 | Timer1 |
| Motor 2 | D6 / PH3 / OC4A | D7 / PH4 | Timer4 |

The serial interface supported independent or combined `start`, `stop`, `speed`, `reverse`, and `status` commands. The configured command range was 50–8,000 Hz. The software behavior was validated through a complete build with zero warnings and 29 host-side integration checks, but simulated registers could not verify real electrical waveforms, wiring, or mechanical torque.

Physical feeder testing showed that the NEMA17 motor did not provide enough torque to move the cat food reliably. This separated two different questions: the controller could generate the intended commands, but the selected actuator could not consistently overcome the real load.

[View the NEMA17 source and documentation](../code/nema17_motor/)

## Iteration 2 — Higher-Torque Motor and CL86Y-V20 Driver

The second controller used an Arduino Mega 2560 with a CL86Y-V20 closed-loop stepper driver. The project remained register-level AVR C and added an acceleration/deceleration state machine, controlled reversal, software stop behavior, serial logging, and a repeatable one-shot demonstration.

| Mega 2560 connection | Driver connection |
| --- | --- |
| D23 / PA1 | PU+ pulse input |
| D22 / PA0 | DR+ / DIR+ direction input |
| GND | PU− and DR− / DIR− |

The driver was configured for **1,600 pulses per revolution**. The first-test software limited commands to 50–2,000 Hz, corresponding to approximately 1.875–75 RPM at that pulse setting. The default target was 400 Hz, the default acceleration was 200 Hz/s, and the demonstration ramped from 50 Hz to 200 Hz, held for approximately three seconds, and decelerated to a stop.

The higher-torque motor allowed the dispensing mechanism to operate normally. However, the motor weighs approximately **5 kg**. Permanently mounting that mass to the feeder could shift the center of gravity and create a tipping hazard, so the motor remains separate from the main structure and is connected to the mechanism during operation.

[View the higher-torque source, tests, and documentation](../code/high_torque_motor/)

[View the motor used for this iteration](https://www.amazon.com/dp/B0FVX5SVB4?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)

[Watch the higher-torque motor rotation test](../assets/videos/high-torque-motor-test.mp4)

## Control and Feedback Limitations

- The reported frequency and state describe the controller's commands; they are not direct measurements of shaft speed or position.
- The encoder connects to the driver, while the Mega does not receive the driver's ALM or PEND outputs.
- The serial `estop` command stops pulse generation but does not disconnect driver power.
- Host tests validate state-machine and command behavior, not real torque, driver alarms, wiring, or waveform quality.
- Motor wiring and driver settings must be changed only with power removed.

## Source Layout

| Component | Purpose |
| --- | --- |
| `src/main.c` | Main loop, startup state, and status output |
| `src/motor.c` | Acceleration, deceleration, stop, and reversal state machine |
| `src/motor_io.c` | Timer-driven pulse output and direction timing |
| `src/timer.c` | One-millisecond time base |
| `src/usart.c`, `src/command/` | Serial input, validation, and command handling |
| `src/demo.c` | Repeatable acceleration–hold–deceleration demonstration |
| `tests/test_controller.c` | Host-side controller and command tests |
