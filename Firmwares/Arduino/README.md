# Summary

- The Arduino Firmware is used to control and monitor the Whole Alto System. For the moment, it can handle 1 FPGA only.
- It handles automatically the startup of the Board, and boot-up of the FPGA.
- Once the board fully booted, at every loop, the arduino read all the available (and enabled) data and refresh its memory.
- The arduino work as a slave and in a non-blocking way. at every loop, and after refreshing its memory, it checks if any UART packet arrived. If yes, it get decoded, and send back over UART aknowledgement (in case of a command or write request), or the needed data (in case of a reas command).
- Software protection to put FPGA under reset and Power-Off the board to be implemented still.


# Pinout

```
////////////SYSMON I2C DEFINE///////////////
#define SDA_PORT PORTB
#define SDA_PIN 3 // = B3
#define SCL_PORT PORTB
#define SCL_PIN 4 // = B4
///////////////////////////////////////////

#define I2C_PCA_nRST_CTRL_PIN       8                     //is 5

////////////SIPO Pin enumeration////////////
#define SIPO_latchPin   4				  // SIPO Latch Pin. Udate SIPO chip output when asserted Low. 
#define SIPO_clockPin   2				  // SIPO Serial clock out.
#define SIPO_dataOutPin 3				  // SIPO Serial data out.
#define SIPO_nEO        13                                // Hi-Z the SIPO output and 1V8 Level shifter output if High.
////////////////////////////////////////////

////////////PISO Pin enumeration//////////// 
#define PISO_loadPin    8				  // PISO Laod Pin. Update PISO chip Input registers when asserted LOW.
#define PISO_dataInPin  7				  // PISO serial data input.
#define PISO_clockPin   6				  // PISO Serial clock out.
////////////////////////////////////////////
```
![Arduino](https://github.com/user-attachments/assets/31c4b5a2-be7f-4782-b586-03a25b34e6a7)



# Library

Because the design is using I2C and PMBus, external Libraries are used:

- SPI bus to read/write the FPGA's Sysmon is using soft I2C Library.
- PMBus to read/write the TPS power units is using the Arduino's Hard SPI module.

```
#include <SoftI2CMaster.h>
#include <Wire.h>
```

# Address Mapping

```
|---------------|---------|-----------|---------|---------|------------------|-------------------|-------------------|-----------------||---------------------------------------------------------------------------------------------------|
| Byte Index    | 0       | 1         | 2       | 3       | 4                | 5                 | 6                 | 7               ||                                               Additionnal info                                    |
|---------------|---------|-----------|---------|---------|------------------|-------------------|-------------------|-----------------||---------------------------------------------------------------------------------------------------|
| (Mask)   0x   | 1       | F         | F       | F       | F                | F                 | F                 | F               ||                                                                                                   |
|---------------|---------|-----------|---------|---------|------------------|-------------------|-------------------|-----------------||---------------------------------------------------------------------------------------------------|
|               | Rd/nWr  | (section) |      Address      |           16-Bit High Data           |           16-Bit Low Data           ||                                                                                                   |
|---------------|---------|-----------|-------------------|--------------------------------------|-------------------------------------||---------------------------------------------------------------------------------------------------|
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| info          | Rd      | 0x0       | 0x00              | Product Name                                                               ||                                                                                                   |
|               |         |           | 0x01              | Version                                                                    ||                                                                                                   |
|               |         |           | 0x02              | Date                                                                       ||                                                                                                   |
|               |         |           | 0x03              | Author                                                                     ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| ReadBack      |Rd/Wr    |           | 0x10              | 0x0001 : Monitor PM Bus FPGA1                                              ||                                                                                                   |
|               |Rd/Wr    |           | 0xA0              | 0x0001 : Monitor SYSMON I2C Bus FPGA1                                      ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| ADC           | Rd      | 0x1       | 0x00              | 1.8V           [As String]                                                 ||                                                                                                   |
|               |         |           | 0x01              | 3.3V           [As String]                                                 ||                                                                                                   |
|               |         |           | 0x02              | 5V             [As String]                                                 ||                                                                                                   |
|               |         |           | 0x03              | 12V            [As String]                                                 ||                                                                                                   |
|               |         |           | 0x04              |                                                                            ||                                                                                                   |
|               |         |           | 0x05              | 1.8V Min       [As String]                                                 ||                                                                                                   |
|               |         |           | 0x06              | 3.3V Min       [As String]                                                 ||                                                                                                   |
|               |         |           | 0x07              | 5V Min         [As String]                                                 ||                                                                                                   |
|               |         |           | 0x08              | 12V Min        [As String]                                                 ||                                                                                                   |
|               |         |           | 0x09              | 1.8V Max       [As String]                                                 ||                                                                                                   |
|               |         |           | 0x0A              | 3.3V Max       [As String]                                                 ||                                                                                                   |
|               |         |           | 0x0B              | 5V Max         [As String]                                                 ||                                                                                                   |
|               |         |           | 0x0C              | 12V Max        [As String]                                                 ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0x10              |                                                                            ||                                                                                                   |
|               |         |           | 0x11              | 3.3V  SLAVE    [As String]                                                 ||                                                                                                   |
|               |         |           | 0x12              | 5V SLAVE       [As String]                                                 ||                                                                                                   |
|               |         |           | 0x13              |                                                                            ||                                                                                                   |
|               |         |           | 0x14              |                                                                            ||                                                                                                   |
|               |         |           | 0x15              |                                                                            ||                                                                                                   |
|               |         |           | 0x16              | 3.3V SALVE Min [As String]                                                 ||                                                                                                   |
|               |         |           | 0x17              | 5V SLAVE Min   [As String]                                                 ||                                                                                                   |
|               |         |           | 0x18              |                                                                            ||                                                                                                   |
|               |         |           | 0x19              |                                                                            ||                                                                                                   |
|               |         |           | 0x1A              | 3.3V SLAVE Max [As String]                                                 ||                                                                                                   |
|               |         |           | 0x1B              | 5V SALVE Max   [As String]                                                 ||                                                                                                   |
|               |         |           | 0x1C              |                                                                            ||                                                                                                   |
|               | Wr      |           | 0xFF              | Reset ADC Min/Max                                                          ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| SIPO (output) | Rd/Wr   | 0x3       | 0x00              | SIPO_REG 16Bits (0x0000).                                                  ||                                                                                                   |
|               |         |           |                   | Mask       Bit  Name                                                       ||                                                                                                   |
|               |         |           |                   | 0x8000     15   VCCINT_FPGA_1_EN                                           ||                                                                                                   |
|               |         |           |                   | 0x4000     14   VCCINT_FPGA_2_EN                                           ||                                                                                                   |
|               |         |           |                   | 0x2000     13   VCCINTIO_BRAM_FPGA_1_EN                                    ||                                                                                                   |
|               |         |           |                   | 0x1000     12   VCCINTIO_BRAM_FPGA_2_EN                                    ||                                                                                                   |
|               |         |           |                   | 0x800      11   MGTVCCAUX_FPGA_EN                                          ||                                                                                                   |
|               |         |           |                   | 0x400      10   1V8_SYS_EN                                                 ||                                                                                                   |
|               |         |           |                   | 0x200       9   MGTAVTT_FPGA_EN                                            ||                                                                                                   |
|               |         |           |                   | 0x100       8   MGTAVCC_FPGA_EN                                            ||                                                                                                   |
|               |         |           |                   | 0x80        7   (nINIT_B_FPGA_1)                                           ||                                                                                                   |
|               |         |           |                   | 0x40        6   (nINIT_B_FPGA_2)                                           ||                                                                                                   |
|               |         |           |                   | 0x20        5   nPROGRAM_B_FPGA_1                                          ||                                                                                                   |
|               |         |           |                   | 0x10        4   nPROGRAM_B_FPGA_2                                          ||                                                                                                   |
|               |         |           |                   | 0x8         3   nRST_FPGA_1                                                ||                                                                                                   |
|               |         |           |                   | 0x4         2   nRST_FPGA_2                                                ||                                                                                                   |
|               |         |           |                   | 0x2         1   Reserved                                                   ||                                                                                                   |
|               |         |           |                   | 0x1         0   Reserved                                                   ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| PISO (input)  |  Rd     | 0x4       | 0x00              | PISO_REG 24Bits (0x0000).                                                  ||                                                                                                   |
|               |         |           |                   | Mask 0x    Bit    Name                                                     ||																									                                                  |
|               |         |           |                   | 0x800000    24    VCCINT_FPGA_1_OK                                         ||																									                                                  |
|               |         |           |                   | 0x400000    23    VCCINT_FPGA_2_OK                                         ||                                                                                                   |
|               |         |           |                   | 0x200000    22    VCCINTIO_BRAM_FPGA_1_OK                                  ||                                                                                                   |
|               |         |           |                   | 0x100000    21    VCCINTIO_BRAM_FPGA_2_OK                                  ||                                                                                                   |
|               |         |           |                   | 0x80000     20    reserved                                                 ||                                                                                                   |
|               |         |           |                   | 0x40000     19    MGTAVTT_FPGA_OK                                          ||                                                                                                   |
|               |         |           |                   | 0x20000     18    MGTVCCAUX_FPGA_OK                                        ||                                                                                                   |
|               |         |           |                   | 0x10000     17    MGTAVCC_FPGA_OK                                          ||                                                                                                   |
|               |         |           |                   | 0x8000      15    (1V2_FPGA_OK)                                            ||                                                                                                   |
|               |         |           |                   | 0x4000      14    5V_UTIL_OK                                               ||                                                                                                   |
|               |         |           |                   | 0x2000      13    3V3_UTIL_OK                                              ||                                                                                                   |
|               |         |           |                   | 0x1000      12    1V8_UTIL_OK                                              ||                                                                                                   |
|               |         |           |                   | 0x800       11    nINIT_B_FPGA_1                                           ||                                                                                                   |
|               |         |           |                   | 0x400       10    nINIT_B_FPGA_2                                           ||                                                                                                   |
|               |         |           |                   | 0x200       9     DONE_FPGA_1                                              ||                                                                                                   |
|               |         |           |                   | 0x100       8     DONE_FPGA_2                                              ||                                                                                                   |
|               |         |           |                   | 0x80        7     VCCINT_nHOT_FPGA_1                                       ||                                                                                                   |
|               |         |           |                   | 0x40        6     VCCINT_nHOT_FPGA_2                                       ||                                                                                                   |
|               |         |           |                   | 0x20        5     VCCINT_nFAULT_FPGA_1                                     ||                                                                                                   |
|               |         |           |                   | 0x10        4     VCCINT_nFAULT_FPGA_2                                     ||                                                                                                   |
|               |         |           |                   | 0x8         3     nOT_FAN_1                                                ||                                                                                                   |
|               |         |           |                   | 0x4         2     nOT_FAN_2                                                ||                                                                                                   |
|               |         |           |                   | 0x2         1     nFANFAIL_FAN_1                                           ||                                                                                                   |
|               |         |           |                   | 0x1         0     nFANFAIL_FAN_2                                           ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| PM_VCCINT     |  Wr     | 0x5       | 0x03              | PM_CLEAR_TSP_FAULT                                                         ||                                                                                                   |
|               |  Rd     |           | 0x88              | PM_READ_VIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x89              | PM_READ_IIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x97              | PM_READ_PIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x79              | PM_READ_STATUS_REG                                                         ||                                                                                                   |
|               |         |           | 0xD4              | PM_READ_VOUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x8C              | PM_READ_IOUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x96              | PM_READ_POUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x8D              | PM_READ_TOUT_REG                                                           ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| PM_VCCIO_BRAM |  Wr     | 0x6       | 0x03              | PM_CLEAR_TSP_FAULT                                                         ||                                                                                                   |
|               |  Rd     |           | 0x88              | PM_READ_VIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x89              | PM_READ_IIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x97              | PM_READ_PIN_REG                                                            ||                                                                                                   |
|               |         |           | 0x79              | PM_READ_STATUS_REG                                                         ||                                                                                                   |
|               |         |           | 0xD4              | PM_READ_VOUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x8C              | PM_READ_IOUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x96              | PM_READ_POUT_REG                                                           ||                                                                                                   |
|               |         |           | 0x8D              | PM_READ_TOUT_REG                                                           ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| I2C Sysmon    | Rd/Wr   | 0xA       | 0x00              | TEMPERATURE_SLR0                                                           || a write reset the register for MIN/MAX Only. Min reset value is 0xFFFF, Max reset value is 0x0000 |
|               |         |           | 0x01              | TEMPERATURE_SLR0_MIN (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x02              | TEMPERATURE_SLR0_MAX (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x03              | VCCINT_SLR0                                                                ||                                                                                                   |
|               |         |           | 0x04              | VCCINT_SLR0_MIN (Rd/Rw)                                                    ||                                                                                                   |
|               |         |           | 0x05              | VCCINT_SLR0_MAX (Rd/Rw)                                                    ||                                                                                                   |
|               |         |           | 0x06              | VCCAUX_SLR0                                                                ||                                                                                                   |
|               |         |           | 0x07              | VCCAUX_SLR0_MIN (Rd/Rw)                                                    ||                                                                                                   |
|               |         |           | 0x08              | VCCAUX_SLR0_MAX (Rd/Rw)                                                    ||                                                                                                   |
|               |         |           | 0x09              | VCCBRAM_SLR0                                                               ||                                                                                                   |
|               |         |           | 0x0A              | VCCBRAM_SLR0_MIN (Rd/Rw)                                                   ||                                                                                                   |
|               |         |           | 0x0B              | VCCBRAM_SLR0_MAX (Rd/Rw)                                                   ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0x10              | TEMPERATURE_SLR1                                                           ||                                                                                                   |
|               |         |           | 0x11              | TEMPERATURE_SLR1_MIN (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x12              | TEMPERATURE_SLR1_MAX (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x13              | VCCINT_SLR1                                                                ||                                                                                                   |
|               |         |           | 0x14              | VCCINT_SLR1_MIN (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x15              | VCCINT_SLR1_MAX (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x16              | VCCAUX_SLR1                                                                ||                                                                                                   |
|               |         |           | 0x17              | VCCAUX_SLR1_MIN (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x18              | VCCAUX_SLR1_MAX (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x19              | VCCBRAM_SLR1                                                               ||                                                                                                   |
|               |         |           | 0x1A              | VCCBRAM_SLR1_MIN (Rd/Rw)                                                   || (non-Functionnal)                                                                                 |
|               |         |           | 0x1B              | VCCBRAM_SLR1_MAX (Rd/Rw)                                                   || (non-Functionnal)                                                                                 |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0x20              | TEMPERATURE_SLR2_M                                                         ||                                                                                                   |
|               |         |           | 0x21              | TEMPERATURE_SLR2_MIN (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x22              | TEMPERATURE_SLR2_MAX (Rd/Rw)                                               ||                                                                                                   |
|               |         |           | 0x23              | VCCINT_SLR2                                                                ||                                                                                                   |
|               |         |           | 0x24              | VCCINT_SLR2_MIN (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x25              | VCCINT_SLR2_MAX (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x26              | VCCAUX_SLR2                                                                ||                                                                                                   |
|               |         |           | 0x27              | VCCAUX_SLR2_MIN (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x28              | VCCAUX_SLR2_MAX (Rd/Rw)                                                    || (non-Functionnal)                                                                                 |
|               |         |           | 0x29              | VCCBRAM_SLR2                                                               ||                                                                                                   |
|               |         |           | 0x2A              | VCCBRAM_SLR2_MIN (Rd/Rw)                                                   || (non-Functionnal)                                                                                 |
|               |         |           | 0x2B              | VCCBRAM_SLR2_MAX (Rd/Rw)                                                   || (non-Functionnal)                                                                                 |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0xF0              | I2C_SSYSMON_Flag_SLR0                                                      ||                                                                                                   |
|               |         |           | 0xF1              | I2C_SSYSMON_Flag_SLR1                                                      ||                                                                                                   |
|               |         |           | 0xF2              | I2C_SSYSMON_Flag_SLR2                                                      ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0xFE              | MIN/MAX Reset (Wr)                                                         ||                                                                                                   |
|               |         |           | 0xFF              | I2C_SYSMON_STATUS (Rd/Wr)                                                  || a write reset the register                                                                        |
|               |         |           |                   | Mask:    bit:     Name:                                                    ||                                                                                                   |
|               |         |           |                   | 0x8000   15     I2C Init Fail                                              ||                                                                                                   |
|               |         |           |                   | 0x4000   14     Update PCA Channel failed                                  ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           |                   | 0x0002   01     I2C read Fail                                              ||                                                                                                   |
|               |         |           |                   | 0x0001   00     I2C Write Fail                                             ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
| Protection    | Rd/Wr   | 0xE       | 0x00              | ptt_ADC_array[0]_en (Bit0), ptt_ADC_array[0]_nReset_Shutdown (bit1)        || 12V System (Board).                                                                               |
|               |         |           | 0x01              | ptt_ADC_array[0]_MIN (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x02              | ptt_ADC_array[0]_MAX (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x03              | ptt_ADC_array[1]_en (Bit0), ptt_ADC_array[1]_nReset_Shutdown (bit1)        || 5V System (Board).                                                                                |
|               |         |           | 0x04              | ptt_ADC_array[1]_MIN (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x05              | ptt_ADC_array[1]_MAX (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x06              | ptt_ADC_array[2]_en (Bit0), ptt_ADC_array[2]_nReset_Shutdown (bit1)        || 3.3V System (Board).                                                                              |
|               |         |           | 0x07              | ptt_ADC_array[2]_MIN (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x08              | ptt_ADC_array[2]_MAX (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x09              | ptt_ADC_array[3]_en (Bit0), ptt_ADC_array[3]_nReset_Shutdown (bit1)        || 1.8V System (Board).                                                                              |
|               |         |           | 0x0A              | ptt_ADC_array[3]_MIN (Rd/Wr)                                               ||                                                                                                   |
|               |         |           | 0x0B              | ptt_ADC_array[3]_MAX (Rd/Wr)                                               ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0x40              | ptt_TEMPERATURE_SLR_en (Bit0), ptt_TEMP_SLR_nReset_Shutdown (bit1)         || a write reset the register for MIN/MAX Only. Min reset value is 0xFFFF, Max reset value is 0x0000 |
|               |         |           | 0x41              | ptt_TEMPERATURE_SLR_MIN (Rd/Rw)                                            ||                                                                                                   |
|               |         |           | 0x42              | ptt_TEMPERATURE_SLR_MAX (Rd/Rw)                                            ||                                                                                                   |
|               |         |           | 0x43              | ptt_VCCINT_en (Bit0), ptt_VCCINT_SLR_nReset_Shutdown (Bit1)                ||                                                                                                   |
|               |         |           | 0x44              | ptt_VCCINT_SLR_MIN (Rd/Rw)                                                 ||                                                                                                   |
|               |         |           | 0x45              | ptt_VCCINT_SLR_MAX (Rd/Rw)                                                 ||                                                                                                   |
|               |         |           | 0x46              | ptt_VCCAUX_SLR_en (Bit0), ptt_VCCAUX_SLR_nReset_Shutdown (Bit1)            ||                                                                                                   |
|               |         |           | 0x47              | ptt_VCCAUX_SLR_MIN (Rd/Rw)                                                 ||                                                                                                   |
|               |         |           | 0x48              | ptt_VCCAUX_SLR_MAX (Rd/Rw)                                                 ||                                                                                                   |
|               |         |           | 0x49              | ptt_VCCBRAM_SLR_en (Bit0), ptt_VCCBRAM_SLR_nReset_Shutdown (Bit1)          ||                                                                                                   |
|               |         |           | 0x4A              | ptt_VCCBRAM_SLR_MIN (Rd/Rw)                                                ||                                                                                                   |
|               |         |           | 0x4B              | ptt_VCCBRAM_SLR_MAX (Rd/Rw)                                                ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0x20              |                                                                            ||                                                                                                   |
|               |         |           | 0x21              |                                                                            ||                                                                                                   |
|               |         |           | 0x22              |                                                                            ||                                                                                                   |
|               |         |           | 0x23              |                                                                            ||                                                                                                   |
|               |         |           | 0x24              |                                                                            ||                                                                                                   |
|               |         |           | 0x25              |                                                                            ||                                                                                                   |
|               |         |           | 0x26              |                                                                            ||                                                                                                   |
|               |         |           | 0x27              |                                                                            ||                                                                                                   |
|               |         |           | 0x28              |                                                                            ||                                                                                                   |
|               |         |           | 0x29              |                                                                            ||                                                                                                   |
|               |         |           | 0x2A              |                                                                            ||                                                                                                   |
|               |         |           | 0x2B              |                                                                            ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0xF0              |                                                                            ||                                                                                                   |
|               |         |           | 0xF1              |                                                                            ||                                                                                                   |
|               |         |           | 0xF2              |                                                                            ||                                                                                                   |
|               |         |           |                   |                                                                            ||                                                                                                   |
|               |         |           | 0xFE              |                                                                            ||                                                                                                   |
|               |         |           | 0xFF              |                                                                            ||                                                                                                   |
|---------------|---------|-----------|-------------------|----------------------------------------------------------------------------||---------------------------------------------------------------------------------------------------|
```
