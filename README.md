![logo](https://static.creatordev.io/logo-md-s.svg)

# Embedded World 2017 - bulb controller app

## Overview

This app controlls the bluetooth bulb. App observes IPSO color object on device server and changes bulb color accordingly.

Other components of the EW17 Creator demo are

* [Ci40 application](https://github.com/CreatorDev/ci40-ew17-uart-to-ds) to receive the detected colour over UART and update an IPSO object with the new colour
* [Clicker application](https://github.com/CreatorDev/clicker-ew-demo) that detects the current block colour and echo the value over UART
* [A javascript application](https://github.com/CreatorDev/webapp-ew17) that receives notifications when new colours are detected, stores them to a database, and pushes bulb-colour value changes
* A python application on a Mediatek 7688 Duo that drives the motors for the conveyor belt when a user presses the stop/start button, or when an inactivity timeout triggers

## Running the app

awa_clientd must be running on the board.

`bulb_controller -t <TARGET DEVICE MAC>`