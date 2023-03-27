# Micromouse Jerry

The main goal of presented robot is to determinate the shortest way between two locations in unknown environment.

## Description

The construction base is a PCB with all elements such as: distance sensors, motors with encoders, motor controller, display, buttons, voltage stabilizer and battery. The device is controlled by Arduino Uno platform. The route is marked with wave propagation algorithm in two-dimensional space. Controllers PI and PD were used to control the robot.

## Presentation

The unknown environment is a labyrinth. Start position is a left-bottom cell and target position is a right-bottom. At the beginning robot moves in almost every cell to find out arrangement of walls. After first ride robot has determined the shortest way and move on only that path.

### [Checkout video here!](https://www.youtube.com/watch?v=5ZXAKI9qJMU)
