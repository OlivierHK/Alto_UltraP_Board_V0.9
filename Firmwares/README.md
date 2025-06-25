# Firmwares

## Summary

The Board needs several Firmwares in order to work properly. Those have to be installed in a certain order.

All needed Firmware can be found in the sub folders.

1. FTDI Firmware to set the board as a 4x UART/COM port (1 per FPGA), and 1x UART/COM (for Arduino) + 1x JTAG (for FPGAs).
2. TPS53681 Firmware to pre-set the VCC and VCCIO_BRAM power supply parameter of the FPGA.
3. Arduino Firmware for controlling and monitoring the board.
   


## FTDIs Programming
There is x2 FTDI on the board:

- x1 FT4232H to convert USB to x4 UARTs/COM. Each UART/COM port go fo an FPGA.
- x1 FT2232H to convert USB to x1 UART/COM for Arduino, and x1 JTAG that daisy-Chain on every FPGA.

Programming them should be done only once, when new board received:

1. Set the card's jumper as "Lone card". Arduino not installed. Mezzanine board not connected.
2. Power ON the board.
3. Connect the card to the computer via USB.
4. Open FTDI prog tool (can be found [here](https://ftdichip.com/utilities/))on Windows and scan:
    1. Program FT2232H as "Self powered" and write EEPROM.
    2. Program FT4232H as "Self powered" and write EEPROM.
5. Open Ubuntu in VM, and connect the USB of the FT2232 (product ID0x6010) to the VM.
6. Follow steps as [ft2232_to_digilent_jtag](https://gist.github.com/rikka0w0/24b58b54473227502fa0334bbe75c3c1)
7. Close Ubuntu, open Vivado in Windows and scan inside Hardware Manager page. Should find "Digilent cable" and see as connected (but no target found as no FPGA connected).


=>Alternatively, a more "legal" way for programming the FTDI FT2232 as Vivado JTAG programmer is to follow this [AMD](https://docs.amd.com/r/en-US/ug908-vivado-programming-debugging/Programming-FTDI-Devices-for-Vivado-Hardware-Manager-Support) solution.


## TPS53681 Programming

- The First TPS53681 programming should be done with the TI USB-to-PMBus programming dongle. Alternatively, the donggle can also be use for monitoring the TPS power supply, but then, the Arduino's PMbus cannot be used (can only have 1 Master).

- Read/Write/Update of the TSP registers ( VccINT/VccIO_BRAM value, OV/UV/OC/UC/OT...) can then be done live by the Arduino via the [AltoP_SysCtrlMon](https://github.com/OlivierHK/AltoP_SysCtrlMon) Application.

### To program the TPS via TI's Dongle:
1. Set the card's jumper as "Lone card". Arduino not installed. Mezzanine board connected. TPS adress set to 96d via resistors by default.
2. Power ON the board.
3. Connect the TI Dongle on J8 (see [schematic](https://github.com/OlivierHK/Alto_UltraP_Board_V1.0/tree/main/Schematic), and to the computer. Open TI [Fusion Digital Power Designer](https://www.ti.com/tool/FUSION_DIGITAL_POWER_DESIGNER) Application.
4. Scan for TPS on the PMbus, and connect.
5. Load the `TPS53681 @ PMBus Address 96d Project - ALTO Ultra+ V0.9 0.84V,300kMhz.xml` file.
6. Press `write to hardware` and `write EEPROM` to set the TPS and save the configuration into the TPS's EEPROM.


## Arduino Programming

Arduino programming should be done only once, when new board received, or when new version is available:

1. Set the card's jumper as "Lone card". Arduino installed.
2. Power ON the Board.
3. open the [Arduino IDE](https://www.arduino.cc/en/software/) and set the correct target (Ardiono Pro Mini, ATMega328P @ 3.3V).
4. Load the c code `Arduino_V0.x.C` and Launch the programming of the Arduino.
5. If programming is hanging, you may need to press the reset button on the Arduino to trigger the downloading.
