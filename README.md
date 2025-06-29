# Alto_UltraP_Board V1.0

## Project Summary

The purpose of this project is to propose an optimized 1~4 Virtex Ultrascale+ XCVU9P/XCVU13P FPGAs platform for cryptocurrency mining, in a PCIe form factor, and connected to a host computer with a single USB cable only. A PCIe interface is also added, so it can be used as an acceleration card for other projects.

Unlike other products on the market, it should be BETTER and CHEAPER. Choosen ICs, design specification, and PCB layout had been heavily optimized to fit that purpose:
- Better Power/Cooling design for the FPGA VCCint.
- Better protection for the FPGA, interface ICs, and power supplies.
- Cheaper PCB design.
- Easy debugging and repair. 
- Only 1 USB cable to access ALL the buses and interfaces of the system.

FPGA is installed on a Mezzzanine/SOM type architecture, that allows flexibility, and potential cheaper reparation/maintenance.

The Backplane adopts a "cover-them-all" approach where any Xilinx Ultrascale/Ultrascale+ FPGA can be installed, as long as there is an existing mezzanine/SOM for it. Many aspects of the design has been over-engineered, that can be simplified on a potential V2.0 of the Backplane.

Shematic design and PCB layout had been done on Kicad, a free tool.

First set of prototypes produced already and running continuously since 2022 with XCVU9P. An exemple of FPGA code and bitstream can be found [here](https://github.com/OlivierHK/FPGA_Monolithic_SHA3-Solidity_Miner_VU9p_600MHz).

All firmawres (including Arduino one, with specification for the board management) can be find in this project repository. 

As for 2025, a [V2.0](https://github.com/OlivierHK/Alto_UltraP_Mezzanine_V2.0) of the mezzanine had been produced and validated already.

This project is the sole work of the author, developed during his free time, using his personal computer and with his personnal money. ^^

The Mezzanine fits a XCVU9P-2FLGA2104I. There is direct compatibility with XCVU5P, XCVU7P and XCVU13P in FLGA2104 package. if new parts, or new package to be designed, please contact the author olivier.faurie.hk@gmail.com.

## Design specification

### Specification listing

- x1 PCIe card format. Height a bit extended. Can fit in tube-like miner, with 2 boards. ~480W per board max.
- x2 FPGAs per board. x2 boards per system set:
  - x1 Master Board, x1 slave board. Connected by a flat cable & Sata(Via Aurora x1 8b/10b). Master/Slave set by jumpers on the board.
  - On-board FPGAs interconnected by Aurora x1 8b/10b.
- Board powered by x2 +12Vdc PCIe power connector standard.
- All power supplies on board are shared, exept FPGA's Vccint and Vccbram that are independants, implemented on the mezzazine.
- Connect to PC by x1 USB by using a 1:2 USB hub (or optionnally by 2 USBs if issues with the USB 1:2 HUB):
 - x1 USB for miner data tranfert (required). 1USB to x4 UART. x1 UART per FPGA.
 - x1 USB for JTAG and Board management. 1USB to 1JTAG + 1UART to Arduino pro-mini.
- Board management done by Arduino pro mini board:
  - FPGA config/Reset management.
  - FPGAs Power cycle and loading managment.
  - On-board power monitoring and protection (direct ADC reading and I2C to FPGA's Sysmon).
  - Vccint/VccIOBRAM control and readback. (PMBus).
  - Vccint/VccIOBRAM to be adjustable for optimizing power.
- FPGA I/O is 1,8Vdc and 3.3Vdc. All electronic in 3.3Vdc and 1.8Vdc. 5V for USB side.
- FPGA I2C, UART and JTAG isoltated by 3-state buffer on both direction For protection.
- FPGA Bitstream stored on x4 SPI-Flash chip, on FPGA mezzanine.
- For PCB:
  - Economical 6 layers PCB.
  - Can handle 340A @0.8V. Need design many vias for thermal management, naked pads and extra aconnectors to add extra Cables and busbar if needed.
  - ENIG Finishing for BGA soldering.
- x2 Plug for 12V PWM fan. Water Cooling can be used as well.
- Lots of extra/redondant connectors for PMBus, JTAG I2C,....more easy to debug or use external dongle.
- LOTs of testing Points for measurement, debug and shunt.

### Functionnal Diagram

This diagram does not include all the signals of the system, but shows all the interfaces and their interactions. both Master/slave configuration are shown:

![Functionnal_Diagram](https://github.com/user-attachments/assets/1d15ac87-314d-4bb4-981a-0fd3e9492113)


# Product delivery
A Quad FPGA Crypto Mining Platform designed for Xilinx Ultrascale+ VU9P/VU13P over a single USB cable.
![Untitled](https://github.com/user-attachments/assets/f3da800b-ec6e-4877-8b1d-7bc5dfcb32a9)


System running with custom watercooling:
![Untitled2](https://github.com/user-attachments/assets/dd328e59-1fe9-47dc-863a-1a310da1e685)
