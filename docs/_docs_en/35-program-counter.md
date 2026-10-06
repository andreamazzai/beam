---
title: "Program Counter"
lang: en
locale: en-US
permalink: /docs/en/programcounter/
excerpt: "BEAM computer Program Counter"
---
<small>[Notes on signals](#notes-on-signals) - [Useful links](#useful-links)</small>

[![BEAM computer Program Counter](../../../assets/pc/35-beam-pc.png "BEAM computer Program Counter"){:width="100%"}](../../../assets/pc/35-beam-pc.png)

The Program Counter (PC) of the BEAM computer presents few differences compared to the PC of Ben Eater's SAP-1.

It is now an 8-bit register instead of a 4-bit one, therefore allowing 256 bytes to be addressed instead of just 16:

[![Schematic of the BEAM computer Program Counter](../../../assets/pc/35-program-counter-schema.png "Schematic of the BEAM computer Program Counter"){:width="100%"}](../../../assets/pc/35-program-counter-schema.png)

*Schematic of the BEAM computer Program Counter.*

The two Synchronous 4-Bit Binary Counters <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">74LS161</a> are connected in cascade according to the method illustrated on page 21 of the datasheet:

[![Cascaded counters](../../../assets/pc/35-program-counter-161-rco.png "Cascaded counters"){:width="66%"}](../../../assets/pc/35-program-counter-161-rco.png)

The Carry Out of a 4-bit binary counter, active upon reaching count 2^4, allows the next counter to increment its count by one unit. Two cascaded counters allow counting up to 2^4 * 2^4 = 16 * 16 = 256.

## Notes on signals

- The PC increment is performed by activating the PCI signal in the microcode (Program Counter Increment).
- Loading the PC to a specific value following a jump instruction or return from subroutine is performed by activating the PCJ signal (Program Counter Jump). The Flags page includes a dedicated section covering [jump operations](../flags/#conditional-and-unconditional-jumps) in depth.

## Useful links

- Ben Eater's <a href="https://eater.net/8bit/pc" target="_blank">videos</a> describing the operation of Flip-Flops and the construction of the PC.
