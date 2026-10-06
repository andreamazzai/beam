---
title: "Stack Pointer"
lang: en
locale: en-US
permalink: /docs/en/stack/
excerpt: "BEAM computer Stack Pointer"
---
<small>[BEAM computer Stack Pointer](#stack-pointer-microcode-implementation) - [The NQSAP / NQSAP-PCB Stack Pointer](#the-nqsap--nqsap-pcb-stack-pointer) - [Schematic](#schematic) - [Useful links](#useful-links)</small>

[![BEAM computer Stack Pointer](../../../assets/sp/70-beam-sp.png "BEAM computer Stack Pointer"){:width="100%"}](../../../assets/sp/70-beam-sp.png)

The stack implementation in the 6502 involves the use of a dedicated memory area for storing and restoring information according to a LIFO (Last-In, First-Out, where the last element inserted is the first to be read) logic, managed by an 8-bit pointer (Stack Pointer, SP) that keeps track of the address of the next available location.

Two common use cases for the stack are saving the current state of the Flags and/or A, X, Y registers before executing a routine that modifies them, so that they can be restored at the end of the routine, and storing the return address of a subroutine called by a JSR instruction. 

The 6502's address bus is 16 bits wide and can address 2^16 = 64K of memory. The stack occupies the second memory page addressable by the CPU, corresponding to address ranging from 0x100 to 0x01FF. Before use, the Stack Pointer is typically set to 0xFF, thus pointing to location 0x1FF. When a write operation to the stack ("Push") is performed, the value is first saved at the position indicated by the pointer, after which the SP is decremented to point to the next free location. Conversely, during a read operation from the stack ("Pull"), the value is first read from the position indicated by the pointer, which is then incremented to address the next available position.

The 6502 Stack Pointer is a register, named S, whose implementation in the BEAM consists of two 4-bit Synchronous Binary Up-Down Counters <a href="https://www.ti.com/lit/ds/symlink/sn54ls169b.pdf" target="_blank">74LS169</a> capable of addressing the 256 bytes of the computer. In essence, it is a normal register that, in addition to being able to store a specific value, has the peculiarity of being able to count both upward and downward.

Since the BEAM has only 256 bytes of memory and the SP is able to address all of them, the area designated for the stack will necessarily have to be limited by the programmer, for example by using the 16 bytes between addresses 0xF0 and 0xFF.

The Stack Pointer always points to the next available location on the stack, therefore:

- a write operation to the stack writes the desired value to the memory location indicated by the Stack Pointer and then decrements it (post-decrement);
- a read operation from the stack first increments the SP (pre-increment) and then returns the contents of the memory location it points to.

In other words, since the stack "grows" downward, the Stack Pointer is decremented after each byte is pushed onto the stack and incremented before each byte is pulled from it.

Since the '169s used in the BEAM do not have a Reset input, they may be in an undefined state at power-on: consequently, the Stack Pointer must be initialized by the program loaded into memory by the user.

The 6502 instructions that interact with the stack are:

- PHA, PLA: Push and Pull of register A to / from the stack
- PHP, PLP: Push and Pull of the Flag register to / from the stack
- JSR, RTS: jump to / return from subroutine
- TXS, TSX: loading / reading of the SP via the X register

## Stack Pointer microcode implementation

Let us analyze a JSR instruction, assuming we have previously initialized the BEAM's SP to 0xFF. The BEAM has 256 bytes, so the SP now points to the last memory location of the computer.

Suppose we have the following code, in which a NOP instruction is followed by a jump to a subroutine that increments the X register. Upon returning from the subroutine, the program continues with another NOP instruction.

~~~text
| Mnemonic  | Address   | Value  | Content                           |
| --------- | --------- | ------ | --------------------------------- |
| ...       | ...       | ?      | Previous instruction or operand   |
| NOP       | 0x1F      | 0x0F   | BEAM NOP instruction opcode       |
| JSR $30   | 0x20      | 0x41   | BEAM JSR instruction opcode       |
|           | 0x21      | 0x30   | JSR instruction jump address      |
| NOP       | 0x22      | 0x0F   | BEAM NOP instruction opcode       |
| ...       | ...       | ?      | Next instruction                  |
| ...       | ...       | ?      | ...                               |
| INX       | 0x30      | 0xA0   | BEAM INX instruction opcode       |
| RTS       | 0x31      | 0x11   | BEAM RTS instruction opcode       |
~~~

The following table **visually highlights** the execution of the JSR instruction with the changes made by the microinstructions to the Program Counter (PC), Stack Pointer, Memory Address Register (MAR), Instruction Register (IR), B registers and the value of the memory location addressed by the MAR. The table also includes the preceding NOP instruction and the first step of the following INX instruction called by the JSR. Values in **bold** indicate a change from the previous step.

 | Instruction | Step   | Microinstructions | PC      | SP      | MAR     | RAM     | IR      | B       |
 |-------------|--------|-------------------|---------|---------|---------|---------|---------|---------|
 | NOP         | 0\*    | RPC \| WM         | 0x1F    | 0xFF    | <b>0x1F | <b>0x0F | ?       | ?       |
 | NOP         | 1\*    | RR  \| WIR \| PCI | <b>0x20 | 0xFF    | 0x1F    | 0x0F    | <b>0x0F | ?       |
 | NOP         | 2\*    | NI                | 0x20    | 0xFF    | 0x1F    | 0x0F    | 0x0F    | ?       |
 | JSR         | 0      | RPC \| WM         | 0x20    | 0xFF    | <b>0x20 | <b>0x41 | 0x0F    | ?       |
 | JSR         | 1      | RR  \| WIR \| PCI | <b>0x21 | 0xFF    | 0x20    | 0x41    | <b>0x41 | ?       |
 | JSR         | 2      | RPC \| WM         | 0x21    | 0xFF    | <b>0x21 | <b>0x30 | 0x41    | ?       |
 | JSR         | 3      | RR  \| WB  \| PCI | <b>0x22 | 0xFF    | 0x21    | 0x30    | 0x41    | <b>0x30 |
 | JSR         | 4      | RS  \| WM         | 0x22    | 0xFF    | <b>0xFF | ?\*\*   | 0x41    | 0x30    |
 | JSR         | 5      | RPC \| WR         | 0x22    | 0xFF    | 0xFF    | <b>0x22 | 0x41    | 0x30    |
 | JSR         | 6      | SE                | 0x22    | <b>0xFE | 0xFF    | 0x22    | 0x41    | 0x30    |
 | JSR         | 7      | RB  \| WPC \| NI  | <b>0x30 | 0xFE    | 0xFF    | 0x22    | 0x41    | 0x30    |
 | INX         | 0\*\*\*| RPC \| WM         | 0x30    | 0xFE    | <b>0x30 | <b>0xA0 | 0x41    | 0x30    |

*Breakdown of the JSR instruction into its eight elementary microinstructions and representation of the state of the registers and RAM at the end of each step.*

\* Previous NOP instruction  
\*\* The value currently contained in memory location 0xFF is not known; moreover, it is irrelevant, as the next step overwrites its content with the return address from the subroutine  
\*\*\* First step of the following INX instruction

1. The first step of the JSR instruction loads the address of the current instruction into the Memory Address Register:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WM, Write Memory Address Register - loads the instruction address into the MAR
2. The second step loads the instruction opcode into the IR and increments the PC to make it point to the next memory location (which contains the JSR instruction operand, i.e. the jump destination address):
    - RR, Read RAM - exposes the instruction opcode on the bus
    - WIR, Write Instruction Register - loads the opcode into the IR\*
    - PCI, Program Counter Increment - increments the PC
3. The third step loads the Program Counter address into the Memory Address Register, which now points to the operand:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WM, Write Memory Address Register - loads the operand address into the MAR
4. The fourth step loads the subroutine address into B and increments the PC, which now points to the next instruction, coinciding with the return address from the subroutine:
    - RR, Read RAM - exposes the operand on the bus
    - WB, Write B - stores the jump destination address in B
    - PCI, Program Counter Increment - increments the PC
5. The fifth step loads the Stack Pointer address into the Memory Address Register, corresponding to the first free location on the stack:
    - RS, Read Stack - exposes the SP value on the bus
    - WM, Write Memory Address Register - loads the SP address into the MAR
6. The sixth step loads the PC address into the stack, to which the computer will return at the end of the subroutine called by the JSR instruction:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WR, Write RAM - pushes the subroutine return address into the stack
7. The seventh step decrements (post-decrement) the Stack Pointer:
    - SE, Stack Enable - the '169 counters decrement the output by one unit, pointing to the next free location on the stack
8. The eighth step loads the subroutine address called by the JSR instruction into the PC:
    - RB, Read B - exposes register B on the bus, in which the subroutine address had been previously stored
    - WPC, Write Program Counter - loads the subroutine address into the PC
    - NI, Next Instruction - resets the Ring Counter

\*Note how the Instruction Register is updated only at the end of the second step of the instruction, as already seen in the explanation of the CPU [Phases](../control/#phases) of the CPU.

At the end of the subroutine, the RTS instruction performs the following steps:

- increments the SP (pre-increment);
- loads the new SP value into the MAR;
- reads the return address from the stack and loads it into the PC.

## The NQSAP / NQSAP-PCB Stack Pointer

In the NQSAP documentation, Tom notes that he had initially planned to use Synchronous 4-Bit Up/Down Binary Counters <a href="https://www.ti.com/lit/ds/symlink/sn74ls193.pdf" target="_blank">74LS193</a>, running into the [glitching issues](../control/#clock-eeprom-glitching-and-instruction-register-part-2) of the EEPROMs, described in a dedicated section of the BEAM Control Logic documentation.

Since Tom had not published the schematic of the NQSAP Stack Pointer, we replace it with that of the NQSAP-PCB.

[![Schematic of the NQSAP-PCB computer Stack Register](../../../assets/sp/70-stack-nqsap-pcb.png "Schematic of the NQSAP-PCB computer Stack Register"){:width="66%"}](../../../assets/sp/70-stack-nqsap-pcb.png)

*Schematic of the NQSAP-PCB computer Stack Register.*

As can be seen, Tom revisits his earlier decision and uses the '193s after all, which count up or down on the Rising Edge of the dedicated Up and Down signals. However, the NQSAP-PCB does not suffer from the glitching problem, since the Instruction Register has been buffered.

The NQSAP-PCB SP is controlled by the SE (Stack Enable) and C0/C1 signals, which determine the counting direction. C0 and C1 replace part of the NQSAP signals: the consolidation of some Flag register control signals, DXY register control signals, and SP counting direction signals onto C0 and C1 allowed the number of EEPROMs to be reduced from 4 to 3. A side effect is the inability to perform stack, DXY register and/or Flag module operations in the same step.

## Schematic

[![Schematic of the BEAM computer Stack Pointer Register](../../../assets/sp/70-stack-pointer-schema.png "Schematic of the BEAM computer Stack Pointer Register"){:width="100%"}](../../../assets/sp/70-stack-pointer-schema.png)

*Schematic of the BEAM computer Stack Pointer Register.*

## Useful links

- Ben Eater's video <a href="https://www.youtube.com/watch?v=xBjQVxVxOxc&t=945s" target="_blank">What is a stack and how does it work?</a> The video is part of the series dedicated to the 6502 computer (and not to the 8-bit computer), but the topic is closely related and the presentation is absolutely worth watching.
- <a href="https://wilsonminesco.com/stacks/basics.html" target="_blank">Stack definition and basics</a> by Garth Wilson, contributor to <a href="http://www.6502.org" target="_blank">6502.org</a> and curator of <a href="https://wilsonminesco.com/" target="_blank">Wilson Mines Co.</a>, a true goldmine of articles, notions, tutorials and more on the 6502.
- Tom Nisbet's <a href="https://tomnisbet.github.io/nqsap/docs/stack-pointer/" target="_blank">NQSAP Stack Pointer</a>.
- Tom Nisbet's <a href="https://tomnisbet.github.io/nqsap-pcb/docs/program-counter-stack-pointer/" target="_blank">NQSAP-PCB Stack Pointer</a>.
