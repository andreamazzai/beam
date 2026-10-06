---
title: "Control Logic"
lang: en
locale: en-US
permalink: /docs/en/control/
excerpt: "BEAM computer Control Logic"
---
<small>[Instruction Register (Part 1) and Instructions](#instruction-register-part-1-and-instructions) - [Ring Counter and Microinstructions](#ring-counter-and-microinstructions) - [Phases](#phases) - [Clock, EEPROM "glitching" and Instruction Register (part 2)](#clock-eeprom-glitching-and-instruction-register-part-2) - [Instruction length](#instruction-length) - [The 74LS138s for signal management](#the-74ls138s-for-signal-management) - [Loading a program from the Loader](#loading-a-program-from-the-loader) - [NQSAP and BEAM signal summary](#nqsap-and-beam-signal-summary) - [Control signals](#control-signals) - [Bus and other signals](#bus-and-other-signals) - [Microcode](#microcode) - [Differences from the 6502 Instruction Set](#differences-from-the-6502-instruction-set) - [Schematic](#schematic) - [Differences between NQSAP and BEAM Control Logic](#differences-between-nqsap-and-beam-control-logic) - [Notes](#notes) - [Useful links](#useful-links) - [Thoughts on the microcode](#thoughts-on-the-microcode)</small>

[![BEAM computer Control Logic](../../../assets/control/40-beam-control.png "BEAM computer Control Logic"){:width="100%"}](../../../assets/control/40-beam-control.png)

In general, instruction management is entrusted to the Control Logic, which consists of three cornerstones: the Instruction Register, the Ring Counter and the Microcode. The Instruction Register contains the instruction being executed, the Ring Counter keeps track of the microinstructions that make up the instruction and the Microcode defines the control signals needed to execute the microinstructions.

This page describes the Control Logic of the NQSAP and the BEAM, highlights some differences with the Control Logic of Ben Eater's SAP-1 and delves deeper into the topics I had found most challenging or most interesting.

For ease of reference and to simplify the comparison between the three computers SAP, NQSAP and BEAM, it is useful to summarize in a table some of the recurring aspects in the text.

| ↓ ↓ Feature / System → →               | SAP-1     | NQSAP      | BEAM          |
| -                                      | -         | -          | -             |
| Author                                 | Ben Eater | Tom Nisbet | Andrea Mazzai |
| IR shared between Opcode and Operand   | Yes       | No         | No            |
| Bit IR for Opcode                      | 4         | 8          | 8             |
| Bit IR for Operand                     | 4         | 0          | 0             |
| Bus width from RC to EEPROM (bits)     | 3         | 3          | 4             |
| Maximum number of Steps (RC)           | 5         | 8          | 16            |
| Bus width from IR to EEPROM (bits)     | 4         | 8          | 8             |
| Maximum number of Instructions (IR)    | 16        | 256        | 256           |
| Instructions implemented in Microcode  | ~ 10      | 120        | > 110         |
| Variable Instruction Length            | No        | Yes        | Yes           |
| Buffered IR                            | No        | No\*       | Yes           |
| IR load on Rising or Falling Edge      | Rising    | Rising     | Falling       |
| RC load on Rising or Falling Edge      | Falling   | Falling    | Falling       |
| EEPROM                                 | 2x 28C16  | 4x 28C256  | 4x 28C256     |
| EEPROM (Kb)                            | 2x 16     | 4x 256     | 4x 256        |
| Control Word size (bits)               | 16        | 32         | 32            |
| RAM (bytes)                            | 16        | 256        | 256           |

\* The buffered IR was developed by Tom for the NQSAP-PCB, which inspired me to include this feature.

Legend: IR = Instruction Register; RC = Ring Counter

Some preliminary notes:

1. In Ben Eater's SAP-1 computer, the naming of control signals is "module-centric", reflecting the specific function of each module: for example, the signal RO (RAM Out) exports the contents of the RAM onto the bus, while AI (A Input) loads register A. In Tom Nisbet's NQSAP and in the BEAM computers, on the other hand, the nomenclature is "computer-centric", adopting a bus-level point of view: for example, RO becomes RR (RAM Read) and AI becomes WA (Write A).

2. In the NQSAP and in the BEAM the Instruction Register (IR) is included in the Control Logic schematic, whereas in the SAP-1 schematics it was on a separate sheet.

[![Schematic of the NQSAP Control Logic](../../../assets/control/40-control-logic-schema-nqsap.png "Schematic of the NQSAP Control Logic"){:width="100%"}](../../../assets/control/40-control-logic-schema-nqsap.png)

*Schematic of the NQSAP Control Logic, slightly modified for the sole purpose of improving its readability.*

## Instruction Register (Part 1) and Instructions

The role of the Instruction Register is to store the current instruction by fetching it from memory.

The SAP-1 Instruction Register was one byte in size, within which both the instruction and the operand were contained:

- the 4 most significant bits were dedicated to the instruction;
- the 4 least significant bits were reserved for an optional operand or address.

If the least significant bits contained an operand (for example, an immediate value to be used in an arithmetic operation), this value was loaded into a register for the execution of the instruction; if the least significant bits contained a memory address, this address was loaded into the Memory Address Register (MAR), which thus pointed to the memory location from which to read or write data.

In the following image, taken from Ben Eater's video <a href="https://youtu.be/JUVt_KYAp-I?t=1837" target="_blank">Reprogramming CPU microcode with an Arduino</a>, one can see how each byte of a simple addition and subtraction program includes both the operation and the operand:

![Addition and subtraction in the SAP](../../../assets/control/40-lda-15-add-14.png "Addition and subtraction in the SAP"){:width="50%"}

For example:

- The instruction LDA 15 at memory address 0000 is composed of the 4 most significant bits (MSB) 0001 (which in the microcode define an accumulator load operation) and the 4 least significant bits (LSB) 1111, which indicate memory address 15, in which the value 5 to be loaded into accumulator A is present.
- The instruction ADD 14 at memory address 0001 is composed of the 4 MSB bits 0010 (which in the microcode define an addition operation) and the 4 LSB bits 1110, which indicate memory address 14, in which the value 6 to be added to the value already present in accumulator A is present.
- The OUT instruction does not require an operand: it exposes the contents of A on the Output module, therefore the 4 LSBs are irrelevant.

| Mnemonic  | Address   | Instruction.Operand |
| -         | -         | -                   |
| -         | -         | -                   |
| LDA 15    | 0000      |       0001.1111     |
| ADD 14    | 0001      |       0010.1110     |
| SUB 13    | 0010      |       0011.1101     |
| OUT       | 0011      |       1110.0000     |
| HLT       | 0100      |       1111.0000     |
|           |    .      |                     |
|           |    .      |                     |
|           |    .      |                     |
| 7         | 1101      |       0000.0111     |
| 6         | 1110      |       0000.0110     |
| 5         | 1111      |       0000.0101     |

*Representation of an addition, subtraction and output program loaded into the 16 bytes of the SAP memory.*

As a consequence of the number of bits used for the instruction, the connection between the SAP-1 Instruction Register and the EEPROMs containing the microcode could only be 4 bits wide, as shown in the figure:

[![Schematic of the SAP Control Logic and Instruction Register](../../../assets/control/40-control-logic-schema-SAP.png "Schematic of the SAP Control Logic and Instruction Register"){:width="100%"}](../../../assets/control/40-control-logic-schema-SAP.png)

*Schematic of the SAP Control Logic and Instruction Register.*

A fundamental difference between the IR of the SAP-1 and that of the NQSAP and BEAM is the size. The 6502 has a relatively small instruction set, composed of 56 basic instructions; however, these instructions can be used with different <a href="https://www.masswerk.at/6502/6502_instruction_set.html#modes" target="_blank">addressing</a> modes, which brings the total number of possible combinations to approximately 150.

In order to handle these combinations and thus emulate the 6502 instruction set, the opcode size must be a full byte and the system architecture must handle variable-length instructions:

- one byte long for those with Implied and Accumulator addressing, which therefore have no operand;
- two or three\* bytes long for all others, which instead make use of an operand:
  - two bytes when the operand is a value (Immediate and Relative addressing);
  - two bytes when the operand is a zero page address (within the first 256 bytes of the computer);
  - three\* bytes when the operand is an address within the 64K addressable by the 6502.

\*A computer with 256 bytes of RAM does not need 3-byte instructions, because an operand of a single byte length is able to address all of the computer's memory, as also briefly discussed in the [Addressing Modes](../alu/#addressing-modes) section of the page dedicated to the ALU.

[![Schematics of the NQSAP and BEAM Instruction Register](../../../assets/control/40-cl-ir-beam-nqsap.png "Schematics of the NQSAP and BEAM Instruction Register"){:width="100%"}](../../../assets/control/40-cl-ir-beam-nqsap.png)

*Schematics of the NQSAP and BEAM Instruction Register.*

Pulling it all together, for a computer like the NQSAP or the BEAM:

- the Instruction Register must be dedicated solely to instructions and be one byte in size;
- the connection between the IR and the EEPROMs must be 8 bits wide and no longer just 4 bits as in the SAP;
- EEPROMs with 13 (NQSAP, 2^13 = 64Kb) or 14 (BEAM, 2^14 = 128Kb) addressing pins are required:
  - 8 pins for instructions (2^8 = 256 instructions);
  - 3 or 4 pins for microinstructions (NQSAP, 2^3 = 8 steps; BEAM, 2^4 = 16 steps), which are discussed in the section dedicated to the [Ring Counter](#ring-counter-and-microinstructions);
  - 2 pins for EEPROM selection.

For the NQSAP, Tom decided to use 256Kb EEPROMs anyway instead of 64Kb ones; the BEAM instead mandatorily requires 256Kb EEPROMs (no <a href="https://eu.mouser.com/c/semiconductors/memory-ics/eeprom/?interface%20type=Parallel" target="_blank">128Kb parallel interface EEPROMs</a> are commercially available).

As will be seen later when discussing the Ring Counter, an important aspect of register loading is the [*instant*](#clock-eeprom-glitching-and-instruction-register-part-2) at which they are loaded: at the Falling Edge\* of the clock, or at the Rising Edge\*: the loading of the SAP-1 and NQSAP Instruction Register occurs at the Rising Edge, while that of the BEAM occurs at the Falling Edge

\* This page always uses the terms Rising Edge and Falling Edge in reference to the normal clock (CLK). Some components receive an inverted clock signal (/CLK), which in some diagrams is represented only to visually highlight the phase of the signal actually received.

Before delving deeper into the topic, it is appropriate to also begin discussing the Ring Counter, which plays a primary role in the loading of all registers, including the IR.

## Ring Counter and Microinstructions

To understand the operation of the Ring Counter, it is necessary to grasp the concept of microinstruction: the *instructions* of a microprocessor are composed of a certain number of steps, more precisely called *microinstructions*.

In fact, every instruction of a microprocessor (for example, "load a value into register X", "increment the contents of location $E5" or "perform a right shift of the accumulator") is composed of a sequence of elementary microinstructions, which correspond to the individual steps needed to complete the desired operation.

The Ring Counter (RC) keeps track of the progress of the microinstructions. Each state of the RC corresponds to a particular step in the execution cycle of an instruction, so it can be seen as a mechanism that advances through the different microinstructions needed to execute a complete CPU instruction.

In the BEAM, for example, the instruction LDA #$94 (which in 6502 mnemonic language translates to "load the hexadecimal value $94 into the accumulator") is composed of the following four steps / microinstructions:

~~~text
| ---- | -------------------------- |
| Step | Microinstruction           |
| ---- | -------------------------- |
| 0*   | RPC | WM                   |
| 1*   | RR  | WIR | PCI            |
| 2    | RPC | WM                   |
| 3    | RR  | FNZ | WAH | PCI | NI |
| ---- | -------------------------- |
~~~

*Breakdown of the LDA Immediate instruction into its four elementary microinstructions*.

1. The first step loads the address of the current instruction into the Memory Address Register:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WM, Write Memory Address Register - loads the instruction address into the MAR

2. The second step loads the instruction opcode into the IR and increments the PC to make it point to the next memory location (which in the case of the LDA instruction contains the operand):
    - RR, Read RAM - exposes the instruction opcode on the bus
    - WIR, Write Instruction Register - loads the opcode into the IR\*
    - PCI, Program Counter Increment - increments the PC

3. The third step loads the Program Counter address into the Memory Address Register, which now points to the operand:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WM, Write Memory Address Register - loads the operand address into the MAR

4. The fourth and final step loads the operand into the accumulator, increments the PC to make it point to the next instruction and resets the Ring Counter:
    - RR, Read RAM - exposes the operand on the bus
    - FNZ, Flag N & Z - updates the N and Z Flags
    - WAH, Write A & H - loads the operand into A and H\*\*
    - PCI, Program Counter Increment - increments the PC
    - NI, Next Instruction - resets the Ring Counter\*\*\*

\* One must not overlook the fact that the first two steps of *all* instructions are always identical. At the end of the second step, the Instruction Register contains the instruction opcode, which, together with the microinstructions, defines the operations that the subsequent steps must execute. This applies to any instruction, including the first one that a CPU executes at power-on. Before building Ben Eater's SAP-1, I could not imagine what mechanism would allow a CPU to know what to do once powered on; having understood it was rather satisfying.

\*\* Why H as well? See the dedicated section explaining the [H register](../alu/#the-h-register) on the ALU page.

\*\*\* Further details in the section [Instruction length](#instruction-length) on this same page.

A diagram that clearly shows the steps of some SAP-1 instructions is visible in this image taken from Ben Eater's video <a href="https://www.youtube.com/watch?v=dHWFpkGsxOs" target="_blank">8-bit CPU control logic: Part 3</a>; steps 000 and 001 are common to all instructions and make up what is called the Fetch Phase, highlighted in yellow.

[![SAP Microcode](../../../assets/control/40-cl-ben-step-microcode.png "SAP Microcode"){:width="100%"}](../../../assets/control/40-cl-ben-step-microcode.png)

*SAP Microcode.*

To finalize the analysis of the LDA #$94 instruction, let us summarize the state of the computer at the end of the fourth step:

- Flag Z will not be active (the result of the accumulator load operation is not equal to zero);
- Flag N will be active (according to the 8-bit [Signed number representation method](../math/#unsigned-and-signed-numbers) in Two's Complement, $94 / 1001.0100 is a negative number, - since the most significant bit is at logic state 1);
- Flags V and C will not be modified relative to their previous state;
- Accumulator A and register H will contain the hexadecimal value $94.

### Phases

To ensure the correct operation of the processor, the Control Logic must set the right Control Word for each microinstruction. The Control Word is the string of bits used to govern and coordinate the behavior of the various processor components during the execution of a microinstruction and is defined in the microcode stored in the EEPROMs; each bit / output pin of the EEPROMs corresponds to a control signal (such as RPC, WM, PCI, RR and so on).

The operations of a CPU go through several phases, which we can summarize as:

1. "Fetch", which fetches the instruction from the memory location pointed to by the PC and stores it in the IR.
2. "Decode", which interprets the contents of the IR to determine which instruction must be executed.
3. "Execute", which includes all the microinstructions that actually carry out what the instruction is supposed to do (for example: "increment register X").

The fetch phase was mentioned in the [previous section](#ring-counter-and-microinstructions) and it is fundamental that the microinstructions of this phase are identical for all implemented instructions.

Let us follow step by step what happens in the simplest instruction implemented in the BEAM, the NOP - No Operation:

~~~text
| ---- | ---------------------|
| Step | Microinstruction     |
| ---- | ---------------------|
| 0    | RPC | WM             |
| 1    | RR  | WIR | PCI      |
| 2    | NI                   |
| ---- | ---------------------|
~~~

*Breakdown of the NOP instruction into its three elementary microinstructions*.

1. The first step loads the address of the current instruction into the Memory Address Register:
    - RPC, Read Program Counter - exposes the PC address on the bus
    - WM, Write Memory Address Register - loads the instruction address into the MAR
2. The second step loads the instruction opcode into the IR and increments the PC to make it point to the next memory location (which in the case of the NOP instruction, one byte long, will be the next instruction):
    - RR, Read RAM - exposes the instruction opcode on the bus
    - WIR, Write Instruction Register - loads the opcode into the IR\*
    - PCI, Program Counter Increment - increments the PC
3. The third step resets the Ring Counter to 0:
    - NI, Next Instruction - resets the Ring Counter

In the first step, the MAR is loaded with the value of the PC. The RC is incremented bringing us to the next step.

In the second step the IR is loaded and the PC is incremented, thus pointing to the next memory location. Note that the new value of the PC does not affect the instruction currently being executed, since the PC does not address the EEPROMs containing the microcode. The RC is incremented bringing us to the next step.

In the third step, the NI control signal resets the RC to its initial value of 0.

The execution of the next instruction now begins, but the IR *still contains the opcode of the NOP instruction*: the IR, in fact, has not been modified. Since the first two steps of all instructions are identical, there is no issue: even though we are starting the next instruction, we execute the first two steps of the NOP instruction still present in the IR.

In other words, by resetting the RC we are starting the execution of the next instruction, but the value of the IR has not yet changed. Consequently, the first two steps of the next instruction are executed using the microcode of the previous instruction. This is why it is fundamental that the microcode of the first two steps be identical for all instructions.

Now, in the first step of the "next" instruction, the updated value of the PC is placed in the MAR, and it is at this point that the new value of the PC begins to be relevant. In the second step, the instruction is loaded into the IR and, from this moment on, the computer begins to execute the specific decode and execute steps of the new instruction.

The decode phase occurs thanks to the microcode stored in the EEPROMs: the instruction loaded into the IR has its own specific opcode (for example, 0100.0110), which is presented to the EEPROM inputs together with the Ring Counter outputs. This combination addresses a specific memory location in the EEPROMs, which output the bits of the Control Word and which, in turn, activate the control signals needed to execute the current microinstruction.

The link between decode and execute is very tight, because at every moment the Control Word depends both on the opcode (Decode) and on the microinstruction (Execute)

One can intuit that a CPU must know at every moment which instruction is currently being executed (we receive this information from the Instruction Register) and which step is currently active, for which the Ring Counter comes to our aid. SAP, NQSAP and BEAM develop the Ring Counter around a <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">74LS161</a> counter, capable of counting from 0 to 15, and a <a href="https://www.ti.com/lit/ds/symlink/sn74ls138.pdf" target="_blank">74LS138</a> demultiplexer, which helps us have visual feedback of the microinstruction being executed.

As just mentioned, the combination of the opcode contained in the Instruction Register and the step provided by the Ring Counter addresses a specific memory location in the EEPROMs: this memory location contains the Control Word.

[![IR and RC outputs and BEAM EEPROM inputs](../../../assets/control/40-cl-ir-cr-beam.png "IR and RC outputs and BEAM EEPROM inputs"){:width="100%"}](../../../assets/control/40-cl-ir-cr-beam.png)

*IR and RC outputs and BEAM EEPROM inputs*.

The schematic highlights that the Ring Counter outputs control 4 EEPROM addresses, while another 8 addresses are controlled by the Instruction Register.

Using combinational logic, it is possible to build the microcode to be loaded into the EEPROMs, which will emit the appropriate output signals (Control Word) for each step of each instruction.

In the image it can be observed that the counter outputs also control the demultiplexer, which is used to display the RC state. Rather than using 16 LEDs (and two '138s), a single "extended" LED is driven by the most significant pin of the '161, which has a value of 8: the currently executing step will be indicated by the LED lit by the '138, to which 8 should be added if the "extended" LED is on.

### Clock, EEPROM "glitching" and Instruction Register (part 2)

In general, the essential moments of a clock cycle in a computer are two: the Rising Edge ↗ (transition of the signal from logic state LO to logic state HI) and the Falling Edge ↘ (the opposite).

- Rising Edge: most sequential components\* (such as counters, registers, flip-flops) change their state during the transition of the clock signal from logic state LO to logic state HI; the loading actions of all computer modules (PC, MAR, RAM, A, B, H, Index Registers, Flags, SP, O) occur at this moment, with some exceptions.

- Falling Edge: since at every Rising Edge the sequential components load the input data, it is intuitive that it is necessary to find a *preceding* moment, during which to set the Control Word of all system modules so that the data is present at the component inputs with adequate advance. The Falling Edge is the best moment to set the Control Word; by inverting the phase of the clock sent to the Ring Counter, the configuration of the Control Word — and therefore of the microinstruction — is performed precisely at the Falling Edge.

\* Sequential components produce an output that depends not only on the current inputs, but also on the previous state, unlike combinational circuits which depend exclusively on the present inputs.

Why are exceptions mentioned? The Ring Counter is certainly one of these, for the reason explained in the previous point; the Instruction Register *can* be another exception.

In fact, in the SAP-1 the loading of the IR is synchronous with the Rising Edge of the clock:

[![SAP Instruction Register detail](../../../assets/control/40-cl-sap-ir-detail.png "SAP Instruction Register detail"){:width="66%"}](../../../assets/control/40-cl-sap-ir-detail.png)

This synchrony is also found in the NQSAP:

[![NQSAP Instruction Register detail](../../../assets/control/40-cl-nqsap-ir-detail.png "NQSAP Instruction Register detail"){:width="66%"}](../../../assets/control/40-cl-nqsap-ir-detail.png)

What are the possible consequences of loading the IR at the Rising Edge of the clock?

One must take into consideration a property of EEPROMs: when the input address changes, the outputs can become unstable, oscillating ("glitching", a technical issue) between logic states before finally settling on the correct value. If the phenomenon is not managed, undesired side effects can occur, such as unwanted clock pulses if gates are used to manage the Enable in chips that lack it (such as the [B register](../alu/#the-nqsap-alu) or the [D, X and Y registers](../dxy/) of the NQSAP), or the simultaneous output of multiple modules onto the bus, generating bus contention and high current absorption.

In EEPROMs such as the <a href="https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf" target="_blank">AT28C256</a>, the parameter that indicates the duration of output uncertainty is typically called "Address Access Time" or "t<sub>ACC</sub>" and indicates the period that elapses between the application of a new input address and the moment when the correct data is available at the output, as shown in the figure:

[![AC Read Waveforms EEPROM AT28C256](../../../assets/control/40-28C256-read-waveform.png "AC Read Waveforms EEPROM AT28C256"){:width="50%"}](../../../assets/control/40-28C256-read-waveform.png)

For example, a <a href="https://www.reddit.com/r/beneater/comments/f7gcvx/glitches_on_eeprom_datalines_when_their_adress/" target="_blank">Reddit thread</a> by rolf-electronics highlights the phenomenon in the first 3 quadrants of the following image, with the output signals showing significant oscillations at the moment of EEPROM input changes:

[![Glitching in Rolf Electronics' SAP-1](../../../assets/control/40-glitching-rolf.png "Glitching in Rolf Electronics' SAP-1"){:width="66%"}](../../../assets/control/40-glitching-rolf.png)

Now, what is the relationship between glitching and loading the Instruction Register at the Rising Edge of the clock?

The following graph shows the rising and falling edges of only the control signals activated in the four steps of the SAP LDA instruction. The colors indicate that the glitching is triggered by an intentional change, i.e. by the microcode that deliberately modifies the state of a specific signal. The grey areas, on the other hand, represent the glitching of signals not modified by the current microinstruction.

The glitching due to variations in the input addresses of the SAP-1 EEPROMs (but it is the same in the NQSAP) occurs:

- at every Falling Edge of the Clock as a consequence of the change in the Ring Counter outputs (moments 1, 5, 9, 13, 17)
- at the Rising Edge of the Clock as a consequence of loading the instruction into the Instruction Register (moment 7 in step 1)

The glitching phenomenon manifests itself on all control signals managed by the EEPROMs, both those intentionally changed and those that are not modified in the current step. As a side note, it should be pointed out that all computer control signals are subject to this phenomenon, even if not indicated in the graph.

[![SAP computer - LDA instruction](../../../assets/control/40-wavedrom-sap-lda.png "SAP computer - LDA instruction"){:width="100%"}](../../../assets/control/40-wavedrom-sap-lda.png)

*SAP computer - LDA instruction*.

Before continuing, it is interesting to examine the steps of this instruction and connect back to the explanation of the [LDA #$94](#ring-counter-and-microinstructions) instruction of the NQSAP to see the similarities:

1. PC exposed on the bus (CO, Counter Out) and loading of the MAR (MI, Memory Address Register In)
2. RAM exposed on the bus (RO, RAM Out), loading of the IR (II, Instruction Register In) and incrementing of the PC (CE, Counter Enable)
3. IR exposed on the bus (IO, Instruction Register Out), loading of the MAR (MI, Memory Address Register In)
4. RAM exposed on the bus (RO, RAM Out), loading of A (AI, A In)

After this brief digression, let us return to the main discussion

All these spurious signals are generally not an issue for the SAP, because the microinstructions write to D-type registers <a href="https://www.ti.com/lit/ds/sdls067a/sdls067a.pdf" target="_blank">74LS173</a> activated at the Rising Edge of the clock, i.e. when the control signals are stable. For example, the glitching of MI at moment 7 is not a source of problems, because the '173 of the MAR stores new values only with the Enable signal active and the Rising Edge of the clock: at that moment, the MI signal is in a stable state and there is no risk of loading incorrect data.

There is an exception during the loading of the Flags: since these are mapped directly onto the EEPROM inputs, any change in C or F causes glitching at every Rising Edge that modifies them. For simplicity, the previous graph does not include the representation of this moment of instability.

We can now return to the question asked earlier in this section: "What are the possible consequences of loading the IR at the Rising Edge of the clock?"

If the computer contains registers lacking an Enable signal, their loading can be implemented by implementing combinational logic between the clock and the dedicated control signal. For example, in the NQSAP the [D, X and Y registers](../dxy) and the [B register](../alu/#the-nqsap-alu) are implemented with <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> and NOR gates.

[![NQSAP Y register](../../../assets/control/40-NQSAP-dxy-y.png "NQSAP Y register"){:width="75%"}](../../../assets/control/40-NQSAP-dxy-y.png)

*NQSAP Y register*.

The answer to the question is that the loading of the Instruction Register at moment 7 generates a glitch on the /WY signal, which can cause an unwanted loading of Y. The Enable of the '574 in fact depends on the NOR operation between the inverted clock and /WY. If the latter is unstable, an unwanted write to the register could occur.

[![Glitching at the LDY instruction in the NQSAP](../../../assets/control/40-nqsap-ldy.png "Glitching at the LDY instruction in the NQSAP"){:width="100%"}](../../../assets/control/40-nqsap-ldy.png)

*Glitching at the LDY instruction in the NQSAP*.

It is from the need to address the glitching problem that the design of the Instruction Register of the <a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, the evolution of the NQSAP, takes shape.

To resolve the glitching problems, Tom redesigned the IR by replacing the 74LS173s with two cascaded D-type registers <a href="https://www.ti.com/lit/ds/symlink/sn74ls377.pdf" target="_blank">74LS377</a>. The first updates as usual during the normal loading of the IR, which occurs at the Rising Edge of the clock at moment 7 of step 1 and maintains the computer's operation unchanged. The output of the first register is fed as input to the second '377, which is updated at the Falling Edge simultaneously with the Ring Counter increment. In this way, all EEPROM inputs are updated simultaneously at the Falling Edge of the clock, ensuring that the control signals at the output are already stable when the computer's registers are updated at the subsequent Rising Edge

This improvement has been incorporated into the BEAM, which in its design seeks to include the positive aspects of the NQSAP-PCB as well.

[![Schematics of the NQSAP and BEAM Instruction Register](../../../assets/control/40-cl-ir-beam-nqsap.png "Schematics of the NQSAP and BEAM Instruction Register"){:width="100%"}](../../../assets/control/40-cl-ir-beam-nqsap.png)

*Schematics of the NQSAP and BEAM Instruction Register.*

Moreover, *all* 8-bit registers of the BEAM are implemented with components equipped with separate Enable and Clock inputs. Consequently, unwanted loadings cannot occur since, at the Rising Edge of the clock, the control signals are always stable.

It is nonetheless interesting to visualize the behavior of the control signals at moment 7, during which — as now established — the only register updated is the IR:

[![No glitching on the BEAM at instant 7 in step 1](../../../assets/control/40-beam-ldy.png "No glitching on the BEAM at instant 7 in step 1"){:width="100%"}](../../../assets/control/40-beam-ldy.png)

*No glitching on the BEAM at instant 7 in step 1*.

The first of the two '377s updates at the Rising Edge at moment 7, without causing glitching in the EEPROMs, since their inputs are not modified. The outputs of this first register are then sent as input to the second '377, which updates at the Falling Edge of the clock at moment 9, simultaneously with the RC increment. Only now are all EEPROM inputs updated simultaneously, allowing the control signals to stabilize in anticipation of moment 11, when the registers are loaded according to the microinstructions set in step 2.

How can we be certain that the elimination of glitching in the BEAM actually derives from the double buffering of the Program Counter? All 8-bit registers implemented with components equipped with separate Enable and Clock inputs are immune to the phenomenon, but some other registers lack such an input, such as the 74LS74 Flip-Flop used to store the Flags. An AND gate allows an artificial Enable signal to be created, similarly to the schematic of the *NQSAP Y Register*.

![BEAM Flag C Register](../../../assets/control/40-beam-c-flag.png "BEAM Flag C Register"){:width="66%"}

*BEAM Flag C Register*.

Let us examine the simple SEC instruction, which sets the Carry.

~~~text
| ---- | ---------------------|
| Step | Microinstruction     |
| ---- | ---------------------|
| 0*   | RPC | WM             |
| 1*   | RR  | WIR | PCI      |
| 2    | CC  | FC  | RL  | NI |
| ---- | ---------------------|
~~~

*Breakdown of the SEC instruction into its three elementary microinstructions*.

1. The first step loads the address of the current instruction into the Memory Address Register:
    RPC, Read Program Counter - exposes the PC address on the bus
    WM, Write Memory Address Register - loads the instruction address into the MAR
2. The second step loads the instruction opcode into the IR and increments the PC to make it point to the next memory location (which in the case of the SEC instruction, one byte long, will be the next instruction):
    RR, Read RAM - exposes the instruction opcode on the bus
    WIR, Write Instruction Register - loads the opcode into the IR\*
    PCI, Program Counter Increment - increments the PC
3. The third step writes 1 to the C register of the 74LS74:
    CC, Clear Carry - sets the ALU-Cin input of the ALU (remember that the '181 Carry is [inverted](../alu/#logic-functions-and-arithmetic-operations): HI state = inactive)
    FC, Flag C - prepares the loading of Flag C
    RL, Read ALU - exposes the ALU contents on the bus
    NI, Next Instruction - resets the Ring Counter

\* The first two steps of *all* instructions are *always* identical.

CC active at instant 9 sends a HI signal to the ALU-Cin pin and [opcode 03](../alu/#direct-hardwired-relationship-between-instruction-register-and-alu), without Carry, configures the ALU to emit an output of all 1s on the bus.

[![No glitching on the BEAM at Flag C loading](../../../assets/control/40-beam-sec.png "No glitching on the BEAM at Flag C loading"){:width="100%"}](../../../assets/control/40-beam-sec.png)

*No glitching on the BEAM at Flag C loading*.

 At the Rising Edge of the clock, the value 1 present at the D pin of the Flip-Flop is loaded and retained, thus setting the Carry Flag: at moment 11 the FC signal is stable and the Flip-Flop containing Flag C is updated without side effects.

---

Concluding the section, it is important to remember that all signals of a microinstruction are activated simultaneously, but that the read and write operations set by the Control Word are executed according to different timings. At the Falling Edge of the clock:

- The read signals set by the Control Word immediately activate any module involved in a Read operation, which immediately presents its output on the bus; for example, the activation of a bus transceiver <a href="https://www.mouser.com/datasheet/2/308/74LS245-1190460.pdf" target="_blank">74LS245</a> is immediate.
- Conversely, the loading signals prepare the involved modules, but the Write operations are executed only at the subsequent Rising Edge of the clock, thus ensuring that the registers to be updated receive already stabilized signals. An example is the D-type register 74LS377 mentioned earlier.

## Instruction length

Another important aspect to consider is the number of microinstructions that can make up each instruction.

The SAP-1 provided a fixed number of 5 steps; consequently, all instructions had the same duration, regardless of their complexity. However, in the microcode that follows we can see that in reality the immediate load instruction LDA could be executed in just three steps, while addition and subtraction require five steps:

[![SAP computer Microcode](../../../assets/control/40-cl-sap-microcode.png "SAP computer Microcode"){:width="66%"}](../../../assets/control/40-cl-sap-microcode.png)

*SAP computer Microcode.*

In the schematic of the SAP-1 Ring Counter it can be noted that the '161 counter presents its outputs to the selection inputs of the '138 demultiplexer, which sequentially activates the inverted outputs (active = LO) from 00 to 05: at every activation of the latter, the two NAND gates activate the Reset input /MR of the '161, which resets the step count to the initial zero, thus beginning a new instruction.

It is easy to notice how this architecture results in a waste of processing cycles during the execution of instructions that require few steps, since the RC must in any case wait for the activation of the last output 05 before being reset.

[![SAP Ring Counter](../../../assets/control/40-control-sap-rc.png "SAP Ring Counter"){:width="50%"}](../../../assets/control/40-control-sap-rc.png)

*SAP Ring Counter.*

Tom's NQSAP includes a very clever feature (among others) and improves the computer's performance by introducing variable instruction length; in fact, the last microinstruction of each instruction includes a signal N (NI in the BEAM), which activates the parallel load pin of the '161: since all counter inputs are set to 0, the count returns to the initial zero.

In other words, each instruction is ended early by inserting an RC Load signal in the last microcode step, so as not to have to wait for the execution of all empty steps; the advantage of making this choice increases as one wishes to implement increasingly complex instructions that require an ever greater maximum number of steps. For example, in the BEAM the maximum possible length of an instruction is 16 steps, but a simple instruction like TXA can be executed in just 3 steps, without wasting the other 13 cycles.

The instant of counter loading is visible on page 11 of the <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">datasheet</a>: with /Load at LO state\*, at the subsequent Rising Edge\*\* of the clock the QA-QD outputs assume the LO states present at the A-D inputs (Preset instant on the x-axis).

In practice, the Ring Counter returns to the initial step.

[![Next Instruction signal in the BEAM Ring Counter](../../../assets/control/40-beam-ni.png "Next Instruction signal in the BEAM Ring Counter"){:width="66%"}](../../../assets/control/40-beam-ni.png)

*Next Instruction signal in the BEAM Ring Counter.*

A question might arise: why not connect the N signal of the microcode directly to the Reset pin of the counter?

The reset of the '161 is *asynchronous*, meaning that it is independent of the clock: consequently, the counter would be reset at the Falling Edge of the clock at the very moment the Control Word is set, not allowing the step to complete at the Rising Edge!

In fact, Tom points out, it would still be possible to use the asynchronous Reset of the '161 connected directly to the N signal, but this would mean having to add a dedicated reset step as the last microinstruction of every instruction. By using synchronous loading instead, no dedicated reset step is necessary.

\* and \*\*: recalling what was described at the end of the previous section regarding the Control Word setting and register loading moments, we find here a first concrete example: the /Load signal is set by the Control Word during the Falling Edge of the clock, while the actual loading of the register occurs in conjunction with the Rising Edge.

As also indicated in the [Differences](../alu/#differences-between-nqsap-and-beam-alu-modules) section of the ALU page, it should be noted that the NQSAP computer provides only 8 steps for microinstructions. To emulate some 6502 shift and rotate instructions more steps are needed, therefore 16 have been provided on the BEAM computer.

## The 74LS138s for signal management

The complexity of the NQSAP is such that the mere 16 control signals available in the SAP-1 Control Logic would not have been sufficient to drive complex modules such as the ALU and the Flag register; as a consequence of this, it became necessary to considerably expand the number of usable control lines.

The increase in the number of EEPROMs and the insertion of four <a href="https://www.ti.com/lit/ds/symlink/sn74ls138.pdf" target="_blank">74LS138</a> demultiplexers allows the high number of signals required by the NQSAP and the BEAM to be managed.

As visible in the schematic, each '138 has 8 output pins, 3 selection pins and 3 Enable pins; by appropriately connecting the selection and Enable pins, it is possible to drive four '138s (for a total of 32 output signals) using only 8 signals from a single EEPROM output. In other words, the '138s act as demultiplexers and allow a large number of signals to be addressed from a limited number of input lines.

[![74LS138 demultiplexer in the BEAM](../../../assets/control/40-cl-beam-eeprom-138.png "74LS138 demultiplexer in the BEAM"){:width="100%"}](../../../assets/control/40-cl-beam-eeprom-138.png)

*74LS138 demultiplexer in the BEAM.*

When active, the outputs of the '138s present a LO state; this circumstance is very convenient for managing the computer's signals, since many of the chips present in the various modules use inverted Enable inputs (for example the 74LS245 transceivers and the D-type registers <a href="https://www.ti.com/lit/ds/symlink/sn74ls377.pdf" target="_blank">74LS377</a>).

The '138s present only one active output at a time; the configuration of the selection and Enable pins adopted in the schematic allows two pairs of '138s to be created, each of which presents only one active output at a time:

- one pair dedicated to read signals from registers;
- one pair dedicated to write signals to registers.

A positive side effect of this type of management is that it will be impossible to activate multiple simultaneous reads, thus preventing the risk of inadvertent short circuits between HI state outputs and LO state outputs of different modules.

The reasoning for write operations is different, since it is effectively necessary to be able to write to multiple registers simultaneously. An operation of this type does not cause bus conflicts and is used, for example, by the ADC addition instruction, which includes a step in which data is written simultaneously to both register A and the Flag register.

In the schematic it can be noted that all computer registers that *do not need* to be active simultaneously — both for reading and writing — are addressed with the demultiplexers.

Direct control signals from the EEPROMs are instead indispensable in three cases:

- when a register has multiple input signals that can be active simultaneously (for example the [Flags register](../flags/#components-and-operation), or the [H register](../alu/#the-h-register));
- when it is necessary to be able to write to multiple registers simultaneously (for example A and H, or Flag and A, or Flag and H\*);
- when other completely independent control signals are needed (for example for the Stack, or for managing the [Carry Input](../flags/#the-carry-and-the-h-and-alu-registers) for ALU and H).

\* In this second case, the signals coming directly from the EEPROMs are used to manage other registers that must be able to be active simultaneously with one of the registers individually addressable by the pair of '138s assigned to write signals.

In summary:

- a first EEPROM manages four demultiplexers that drive the *read* signals of all registers and the *load* signals of all registers (except H and Flag);
- three other EEPROMs manage all other signals, including those that control H and Flag.

Note that the '138 output signals actually usable are 30 and not 32, because the microcode must account for situations in which no driven register should be active. For example, an output of 0000.0000 from the first EEPROM will activate the D0 pins of the first and third demultiplexer: since both pins are disconnected, it will be sufficient to set the output on the first EEPROM to 0x00 to avoid the activation of any register managed by the '138s.

## Loading a program from the Loader

NQSAP and BEAM allow the automated loading of a program thanks to the presence of a Loader based on an Arduino Nano.

The Loader controls some signals of the Control Logic and the clock module. The LDR-Active signal can inhibit the first two EEPROMs and allows the Loader to take over control of the '138s using the N0-N7 signals. By inhibiting the computer's main clock, the Loader can inject its own clock and use it to load the MAR and RAM registers.

A more detailed explanation is available on the dedicated [Loader](../loader/) page.

## NQSAP and BEAM signal summary

The first table summarizes the control signals originating from the Control Logic. The second table includes a description of the existing buses in the computer and a list of control signals not originating from the Control Logic.

The "Signal scope or direction" column indicates the context of a bus, or the source and destination of a control signal.

### Control Signals

| NQSAP         | BEAM           | Signal scope or direction  | Description                                                                                                  |
| -----         | ----           | -------------------------- | -----------                                                                                                  |
| N             | NI             | CL                         | Next Instruction; [explanation](#instruction-length).                                                        |
| LF            | LF             | CL → ALU                   | ALU Force; [explanation 1](../alu/#comparison-instructions) eand [explanation 2](../alu/#summary-subtractions-comparisons-and-addressing-modes).    |
| HL-HR         | HL-HR          | CL → ALU                   | Define the operation to be performed on the H register (parallel load, right or left shift / rotate).                                               |
| IR-Q0 / IR-Q4 | IR-S0..3, IR-M | CL → ALU                   | Determine the operation that the ALU must perform; [explanation](../alu/#logic-functions-and-arithmetic-operations).                                |
| HLT           | HLT            | CL → Clock                 | Halts the running program; [explanation](../clock/#the-hlt-instruction).                                                                            |
| DY-DZ         | DX/Y-DZ        | CL → DXY                   | DX/Y HI exposes X to the adders, LO exposes Y; DZ exposes zero; [explanation](../dxy/#use-with-indexed-addressing-modes).                           |
| C0-C1         | C0-C1          | CL → Flag                  | Determine whether the Carry to be saved in Flag C comes from the ALU Carry Output or from H (shift and rotate); [explanation](../flags/#carry).     |
| CC-CS         | CC-CS          | CL → Flag                  | Select which Carry to present to the ALU and H inputs (the real one, or a fixed 0 or 1); [explanation](../flags/#the-carry-and-the-h-and-alu-registers). |
| FC            | FC             | CL → Flag                  | Loading of Flag C into the flag register.                                                                                           |
| FN            | FN             | CL → Flag                  | Loading of Flag N into the flag register.                                                                                           |
| FB            | FS             | CL → Flag                  | Source of the Flags to be loaded into the Flag register (computation or bus); [explanation](../flags/#components-and-operation).    |
| FV            | FV             | CL → Flag                  | Loading of Flag V into the flag register.                                                                   |
| FZ            | FZ             | CL → Flag                  | Loading of Flag Z into the flag register.                                                                   |
| JE            | JE             | CL → Flag                  | Activates conditional jump instructions; [explanation](../flags/#conditional-and-unconditional-jumps).      |
| PI            | PCI            | CL → PC                    | Increments the Program Counter.                                                                             |
| SCE\*         | SE             | CL → SP                    | Enables Stack Pointer increment/decrement.                                                                  |
| SPI\*         | SU/D           | CL → SP                    | Indicates whether the Stack Pointer should count upward (HI) or downward (LO).                              |
| RA            | RA             | CL → ALU                   | Exposes the contents of accumulator A on the bus.                               |
| RB            | RB             | CL → ALU                   | Exposes the contents of register B on the bus.                                  |
| RD            | RD             | CL → DXY                   | Exposes the contents of register D on the bus.                                  |
| RF            | RF             | CL → Flag                  | Exposes the contents of the Flag register on the bus.                           |
| RH            | RH             | CL → ALU                   | Exposes the contents of register H on the bus.                                  |
| RL            | RL             | CL → ALU                   | Exposes the ALU output on the bus.                                              |
| RP            | RPC            | CL → PC                    | Exposes the contents of the Program Counter on the bus.                         |
| RR            | RR             | CL → RAM                   | Exposes the contents of the RAM on the bus.                                     |
| RX            | RX             | CL → DXY                   | Exposes the contents of register X on the bus.                                  |
| RY            | RY             | CL → DXY                   | Exposes the contents of register Y on the bus.                                  |
| RS            | RS             | CL → SP                    | Exposes the Stack Pointer value on the bus.                                     |
| WI            | WIR            | CL                         | Writes the contents of the bus into the Instruction Register.                   |
| WA            | WA             | CL → ALU                   | Writes the contents of the bus into accumulator A.                              |
| WB            | WB             | CL → ALU                   | Writes the contents of the bus into register B.                                 |
| WD            | WD             | CL → DXY                   | Writes the contents of the bus into register D.                                 |
| WX            | WX             | CL → DXY                   | Writes the contents of the bus into register X.                                 |
| WY            | WY             | CL → DXY                   | Writes the contents of the bus into register Y.                                 |
| WM            | WM             | CL → MAR                   | Writes the contents of the bus into the Memory Address Register.                |
| WO            | WO             | CL → Output                | Writes the contents of the bus into the Output register.                        |
| WR            | WR             | CL → RAM                   | Writes the contents of the bus into the RAM.                                    |
| WS            | WS             | CL → SP                    | Writes the contents of the bus into the Stack Pointer.                          |
| WP            | WPC            | CL → PC                    | Writes the contents of the bus into the Program Counter.                        |

### Bus and other signals

| NQSAP           | BEAM                      | Signal scope or direction  | Description                                                |
| -----           | ----                      | -------------------------- | -----------                                                |
| CLK             | CLK                       | Computer                   | Clock signal sent to all computer modules.                 |
| D0..7           | D0..7                     | Computer                   | Computer bus.                                              |
| MA0 - MA10      | A0..11                    | CL                         | Bus between RC and IR outputs and EEPROM inputs; explanation on this same page.                                            |
| IR-Q5 / IR-Q7   | IR-A0 / IR-A2             | CL → Flag                  | Conditional jump selection inputs of the Flag register; [explanation](../flags/#conditional-and-unconditional-jumps).    |
| ALU-to-register interconnect | H0..7, B0..7 | ALU                        | Bus between B and H register outputs and ALU inputs; [explanation](../alu/#the-nqsap-alu).                                |
| ALU output      | Q0..7                     | ALU                        | Bus between ALU output and output transceiver to computer bus; [explanation](../alu/#the-nqsap-alu).                      |
| Selector Inputs | X0..7, Y0..7              | DXY                        | Bus between X and Y outputs and X/Y selector inputs; [explanation](../dxy/#use-with-indexed-addressing-modes).                |
| Adder Inputs    | DQ0..7 XY0..7             | DXY                        | Bus between D output and X/Y selectors and adder inputs; [explanation](../dxy/#use-with-indexed-addressing-modes).            |
| Adder Outputs   | AQ0..7                    | DXY                        | Bus between adder outputs and output transceiver to computer bus; [explanation](../dxy/#use-with-indexed-addressing-modes).   |
| MC-RR0..3       | N0..3                     | Loader → CL                | Used by the Loader to set the read signal '138s; [explanation](#the-74ls138s-for-signal-management).                        |
| MC-RW0..3       | N4..7                     | Loader → CL                | Used by the Loader to set the write signal '138s; [explanation](#the-74ls138s-for-signal-management).                       |
| PC-Load         | PCJ                       | Flag → PC                  | Controls PC loading for conditional and unconditional jumps; [explanation](../flags/#conditional-and-unconditional-jumps).   |
| ?               | MA0-MA7                   | MAR → RAM                  | Bus between MAR output and RAM input; [explanation](../ram/#design-of-the-mar-and-ram-modules).                                      |
| PROG            | PROG                      | MAR → RAM                  | Selection between RAM programming mode and program execution mode; [explanation](../ram/#mux-program-mode-and-run-mode).         |
| RST             | RST                       | Computer                   | Asynchronous computer reset; [explanation](../loader/#program-loading).                                            |
| LDR-ACTIVE      | LDR-Active                | Loader → Clock e → CL      | Clock and Control Logic EEPROM deactivation; [explanation](../loader/#program-loading).                            |
| LDR-CLK         | LDR-CLK                   | Loader → Clock             | Injection of Loader clock into the computer; [explanation](../loader/#program-loading).                            |
| CLK-Start       | CLK-Start                 | Loader → Clock             | (Re-)Start of system clock after program loading into RAM; [explanation](../loader/#program-loading).              |
| ALU-Cin         | ALU-Cin                   | Flag → ALU                 | Selection of Carry to send as input to the '181s; [explanation](../flags/#the-carry-and-the-h-and-alu-registers).                      |
| H-Cin           | H-Cin                     | Flag → ALU                 | Selection of Carry to send as input to H; [explanation 1](../flags/#the-carry-and-the-h-and-alu-registers) and [explanation 2](../alu/#the-h-register).     |
| ALU-Cout        | ALU-Cout                  | ALU → Flag                 | ALU Carry output to send to the Flag register; [Flag C explanation](../flags/#carry).                          |
| ALU-Q7          | ALU-Q7                    | ALU → Flag                 | ALU MSB to send to the Flag register; [Flag V explanation](../flags/#overflow).                                |
| B-Q7            | B-Q7                      | ALU → Flag                 | B MSB to send to the Flag register; [Flag V explanation](../flags/#overflow).                                  |
| H-Q0\*          | H-Q0                      | ALU → Flag                 | H LSB to send to the Flag register; [Flag C explanation](../flags/#carry).                                     |
| H-Q7            | H-Q7                      | ALU → Flag                 | H MSB to send to the Flag register; [Flag V explanation](../flags/#overflow) and [Flag C](../flags/#carry).    |

\* Missing in the ALU module; oversight in Tom's schematic.

## Microcode

The microcode writing phase was not *too* complex. The experience gained with the SAP, the in-depth study of the NQSAP and a great deal of patience had led me to understand fairly well how to develop the instruction steps taking into account the different 6502 addressing modes.

Only a few instructions required more time to be assimilated, in particular those of [comparison](../alu/#comparison-instructions), [jump to subroutine](../stack/#stack-pointer-microcode-implementation) and conditional jump. The comparison instructions required an in-depth understanding of the result in order to correctly set the flags, while for the others it was necessary to learn how to use a temporary register to store information to be restored in a subsequent step.

[![Writing the BEAM microcode with VScode](../../../assets/control/40-microcode-vscode.png "Writing the BEAM microcode with VScode"){:width="100%"}](../../../assets/control/40-microcode-vscode.png)

*Writing the BEAM microcode with VScode.*

Particularly difficult instead was the *organization* of the Instruction Set, on which, in hindsight, I should have invested more time. Unfortunately, I realized that I had not managed to organize the opcode placement in a structured manner only during the microcode writing phase, but at that point I had already started working on the hardware implementation — with the strong hardwired connections between IR, ALU and Flags — and I no longer wanted to go back.

Tom had automated part of the microcode generation through an appropriate logical grouping of instructions. Personally, I was unable to achieve comparable results, since my knowledge of the C language, both at the time and at the time of writing this documentation, is modest. This prevented me from clearly understanding how to structure the Instruction Set to take full advantage of such benefits.

The <a href="../../../assets/BEAM computer.xlsx" target="_blank">Excel workbook</a> I created presents the 6502 Instruction Set, the analysis of the instructions to determine the addressing modes and the development of the BEAM Instruction Set, taking into account the need to use the [LF control signal](../alu/#comparison-instructions) to put the ALU in Subtract Mode and perform comparison operations.

[![Definition of the BEAM Instruction Set](../../../assets/control/40-control-inst-set.png "Definition of the BEAM Instruction Set"){:width="100%"}](../../../assets/control/40-control-inst-set.png)

*Definition of the BEAM Instruction Set.*

I also spent a great deal of time writing the Arduino sketch used to program the EEPROMs. Ben Eater's programmer could take *several* interminable minutes for each EEPROM, while the block programming implemented for the BEAM — the result of studying Tom's code and circuit — made it possible to reduce the write time of an AT28C256 to just 14 seconds. For further notes, please refer to the [dedicated page](../eeprom-programmer).

In conclusion, the physical construction of the BEAM and the microcode writing did not involve a very long *trial and error* process, because the lengthy analyses had the effect of making the modules work from the very first attempts, or in any case with few final adjustments.

The code is partially commented and should be fairly self-explanatory.

Some links:

- An incredibly useful <a href="https://www.atarimania.com/documents/6502%20(65xx)%20Microprocessor%20Instant%20Reference%20Card.pdf" target="_blank">Micro Logic compendium</a> which in just two pages includes opcodes, addressing modes, flags and the instructions that modify them, the operation of shift instructions and much more. Irreplaceable.
- A very valuable reference for analyzing the relationship between the Control Logic (CL) and the IR was Norbert Landsteiner's <a href="https://www.masswerk.at/6502/6502_instruction_set.html" target="_blank">6502 Instruction Set</a> page. It presents the Instruction Set in a convenient tabular view, from which I derived the <a href="../../../assets/BEAM computer.xlsx" target="_blank">Excel view</a> used to define the BEAM instruction opcodes.
- Also by Norbert, I recommend consulting the <a href="https://www.masswerk.at/6502/assembler.html" target="_blank">6502 Assembler</a> and the <a href="https://www.masswerk.at/6502/" target="_blank">Virtual 6502</a> that I used during microcode debugging: very useful for simulating the step-by-step execution of instructions, visualizing flag updates and adjusting the BEAM microcode accordingly.

### Differences from the 6502 Instruction Set

The BEAM computer does not implement the Interrupts and the Decimal mode of the 6502, therefore the instructions SEI, CLI, RTI and SED, CLD are not part of the Instruction Set.

The following instructions have been added: INA, DEA, OUT.

The BRK instruction has also not been implemented, but a similar behavior can be found in the [new HLT](../clock/#the-hlt-instruction).

## Schematic

[![Schematic of the BEAM computer Control Logic](../../../assets/control/40-control-logic-schema-beam.png "Schematic of the BEAM computer Control Logic"){:width="100%"}](../../../assets/control/40-control-logic-schema-beam.png)

*Schematic of the BEAM computer Control Logic.*

## Differences between NQSAP and BEAM Control Logic

The BEAM computer Control Logic incorporates everything that was developed by Tom Nisbet in the NQSAP.

- A substantial difference lies in the Instruction Register, developed in buffered mode as in Tom's NQSAP-PCB to remedy the glitching problems encountered in the NQSAP.
- The BEAM provides 16 steps for microinstructions instead of just 8. The emulation of some shift and rotate instructions requires more than the 8 steps available in the NQSAP, which therefore does not include them.

## Notes

- For space reasons, the BEAM schematic does not include the LED bar showing the output of the '161 counter (in the physical implementation, it is placed alongside the LED bar connected to the Instruction Register output, labeled "EEPROM Address" in the image at the top of the page) and the LED bar inserted between the two 74LS377s of the IR (labeled "Instruction Register").

## Useful links

- Ben Eater's videos describing the <a href="https://eater.net/8bit/control" target="_blank">Control Logic and Microcode</a>.
- Tom Nisbet's <a href="https://tomnisbet.github.io/nqsap/docs/control/" target="_blank">NQSAP Control Logic</a>.

## Thoughts on the microcode

More or less regularly, vulnerabilities are discovered in modern CPUs: for example, a virtual machine (VM) might be able to <a href="https://en.wikipedia.org/wiki/Meltdown_(security_vulnerability)" target="_blank">read the memory of another VM</a>; to address the vulnerabilities, system manufacturers release firmware updates to address the security flaws.

Before undertaking the SAP project, I could not understand the connection between a firmware update and the resolution of a security problem identified in a CPU. Since a CPU is not strictly a programmable component, I could not understand how an update could resolve security problems arising from a partially flawed design of a hardware component.

After building the SAP, I understood the role of <a href="https://en.wikipedia.org/wiki/Microcode" target="_blank">microcode</a>. Industrial CPUs contain their own microcode, similarly to that of the SAP, the NQSAP, the BEAM. This microcode is written in a non-volatile memory of the CPU and therefore cannot be modified, but the CPU also includes a volatile memory area into which microcode updates can be loaded.

When a microcode update is distributed, the operating system loads the updated version of the microcode into the CPU at every boot; this modification is temporary and resides in the CPU's RAM, where it remains loaded until the next reboot.