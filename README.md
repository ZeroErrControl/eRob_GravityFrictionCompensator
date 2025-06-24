# Friction and Gravity Compensation for Strain Wave Gear Rotary Actuator

[中文版本](README_CN.md) | English Version

This project demonstrates how to establish a friction model for an industrial-grade strain wave gear rotary actuator and apply friction and gravity compensation techniques, making the system behave as if there is no resistance or gravity.

## Video Demonstration

<div align="center">
  <a href="https://www.youtube.com/watch?v=I29rnLXZR1I">
    <img src="https://img.youtube.com/vi/I29rnLXZR1I/0.jpg" alt="Strain Wave Gear Rotary Actuator Demo">
  </a>
</div>

## Test Platform Introduction

- **Power Supply**: 48V DC regulated power supply
- **Test Bench**: Including linkage, iron plate, motor output shaft
- **Control Hardware**:
  - Microcontroller: STM32-C8T6
  - Communication Module: TAJ1050
  - USB to Serial Converter
  - USB Debug Box

## Strain Wave Gear Rotary Actuator Specifications

- **Model**: eRob 70H100i
- **Reduction Ratio**: 100:1
- **Rated Power**: 100W
- **Rated Voltage**: 48V
- **Rated Torque**: 10 Nm
- **Peak Torque**: 70 Nm
- **Maximum Speed**: 30 RPM

## Friction Model Identification

Under no-load conditions, we conducted speed loop tests, measuring the average current at different motor speeds to plot the friction curve. Friction is a nonlinear function of the motor's actual speed:

- **X-axis**: Motor speed (RPM)
- **Y-axis**: Friction (represented in milliamps)

### Characteristics:
- When motor speed is zero, friction appears as static friction
- When motor speed is non-zero, friction increases nonlinearly with motor speed
- The friction curves for CW/CCW rotation are symmetric about the origin

The project adopts a lookup table-based mathematical model to precisely describe this friction. Due to symmetry, only the positive half-axis data is stored. The table contains 37 points using piecewise linear interpolation to meet real-time computation requirements.

## Gravity Compensation Model Identification

The current required to counteract gravity depends on the motor's torque constant, linkage length, iron plate weight, and motor angle. The first three parameters can be combined into a constant K. With the motor in position mode held at 90°, we measure the current to calculate the value of K.

## Complete Compensation Model

The complete friction and gravity compensation model is as follows:
1. Compute friction compensation current in real-time based on motor speed
2. Compute gravity compensation current based on motor angle
3. Sum both and send as torque mode command

## Demonstration Results

After adding friction and gravity compensation, the entire system behaves as if there's no resistance or gravity. It can rotate freely. A light touch by hand can easily make it rotate or stop.

- **Friction Compensation Only**: A light touch allows the motor to rotate and stop freely
- **After Adding 2kg Iron Plate**: A gentle push on the linkage causes the motor to swing with constant amplitude as if there were no friction
- **Complete Compensation**: The system behaves as if there is no resistance or gravity

## Project Structure

## Future Work

In the next project, we will establish the speed loop model for the strain wave gear rotary actuator. Stay tuned. 