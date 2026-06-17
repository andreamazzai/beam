[**🇮🇹 Italiano**](README.md) | [**🇬🇧 English**](README.en.md)

# BEAM
## 8-bit TTL Breadboard Computer based on Ben Eater's design and Tom Nisbet's improvements

BEAM is a TTL breadboard computer inspired by [Ben Eater's SAP-1 8-bit computer](https://eater.net/8bit); it also includes the improvements and expansions found in [Tom Nisbet's NQSAP](https://github.com/tomnisbet/nqsap).

Some interesting aspects of the project:

* Instruction set and addressing modes inspired by the 6502 processor
* A, X, Y registers as in the 6502
* Stack Pointer
* ALU based on the 74LS181 for all arithmetic and logic operations
* Rotate and shift instructions implemented with shift registers
* NVZC flags
* Development of a bootloader and EEPROM programmer

This repository contains:

* [Complete BEAM computer documentation](https://andreamazzai.github.io/beam/docs/en/home/)
* [KiCad](https://github.com/andreamazzai/beam/releases/tag/v1.1.0) schematics
* Arduino sketch for the [EEPROM programmer](Beam-Microcode/Beam-Microcode.ino)
* Arduino sketch for the [bootloader](Beam-Bootloader/Beam-Bootloader.ino)
* [Instruction Set](https://github.com/andreamazzai/beam/raw/master/docs/assets/BEAM%20computer.xlsx) Definition

[![BEAM Breadboard Computer](/docs/assets/home/beam.png "BEAM breadboard computer")](docs/assets/home/beam.png)

| Left side       |  8-bit bus |      Right side |
|:---             |:----------:|             ---:|
| Clock           |  IIIIIIII  | Program Counter |
| RAM             |  IIIIIIII  | X Register      |
| RAM             |  IIIIIIII  | Y Register      |
| MAR             |  IIIIIIII  | D Register      |
| Control Logic   |  IIIIIIII  | A Register      |
| Control Logic   |  IIIIIIII  | H Register      |
| Ring Ctr / IR   |  IIIIIIII  | ALU             |
| Loader          |  IIIIIIII  | B Register      |
| Loader          |  IIIIIIII  | Flags Register  |
| Output Register |  IIIIIIII  | Flags Register  |
| Output / Loader |  IIIIIIII  | Stack Pointer   |
