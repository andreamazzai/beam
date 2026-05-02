---
title: "D, X e Y"
lang: en
locale: en-US
permalink: /docs/en/dxy/
excerpt: "Registri indice del computer BEAM"
---
<small>[The D register](#the-d-register) - [Use with indexed addressing modes](#use-with-indexed-addressing-modes) - [Use for conditional jumps](#use-for-conditional-jumps) - [Schematic](#schematic) - [Differences between NQSAP and BEAM index registers](#differences-between-nqsap-and-beam-index-registers) - [Useful links](#useful-links)</small>

[![BEAM computer index registers](../../../assets/dxy/60-beam-dxy.png "BEAM computer index registers"){:width="100%"}](../../../assets/dxy/60-beam-dxy.png)

The 6502 microprocessor features two index registers, X and Y, which can easily be reproduced in a TTL computer; they are independent registers that can be written to and read from as needed.

## The D register

The D register of the NQSAP and BEAM is used to support conditional jump instructions and those that perform operations on a memory location that is the result of a computation between the base address specified in the operand and four different ways of interpreting the values assumed by the X and Y index registers:

| Addressing Mode    | Acts on the address defined                                             | Mnemonic     | Result                                                              | Example*                                        |
| -                  | -                                                                       | -            | -                                                                   | -                                               |  
| Absolute, X        | by the sum of the operand and the value in X                            | LDA ($20, X) | A = value contained in location ($20 + X)                           | X = #$03; A = value at location $23             |
| Absolute, Y        | by the sum of the operand and the value in Y                            | LDA ($20, Y) | A = value contained in location ($20 + Y)                           | Y = #$03; A = value at location $23             |
| Indexed Indirect X | at the location pointed to by the sum of the operand and the value in X | LDA ($20, X) | A = value contained in the location pointed to by pointer ($20 + X) | X = #$03; $23 = #$50; A = value at location $50 |
| Indirect Indexed Y | by the sum of the location pointed to by the operand and the value in Y | LDA ($20), Y | A = value contained in the location pointed to by pointer ($20) + Y | Y = #$03; $20 = #$60; A = value at location $63 |

\* $23 = #$50 means that hexadecimal address $23 contains hexadecimal value #$50:

- the $ notation refers to an address
- the #$ notation refers to a value

It should be noted that some 6502 addressing modes are redundant in a computer with only 256 bytes of RAM: for clarification see the [Addressing Modes](../alu/#addressing-modes) section on the ALU page.

### Use with indexed addressing modes

The contents of register D can be added to the contents of registers X, Y, or neither (in this last case, D could be useful as an additional auxiliary register for defining other custom instructions; the use of this mode is not necessary to emulate the original 6502 instructions).

To select whether D should be added to X, Y, or nothing, <a href="https://www.ti.com/lit/ds/symlink/sn74ls157.pdf" target="_blank">74LS157</a> multiplexers (MUX) are used, driven by signals DY and DZ in the NQSAP and DX/Y and DZ in the BEAM.

To perform the computation, 4-Bit Binary Full Adders With Fast Carry <a href="https://www.ti.com/lit/ds/symlink/sn54s283.pdf" target="_blank">74LS283</a> are used, which add whatever is loaded into register D to the value contained in index registers X or Y.

The result of the computation can be exported by activating the RD signal: the Adder outputs will then be exposed on the bus. Note that register D is "Write only", so it is not possible to directly output its contents onto the bus; however, it is possible to read the value originally present in D by bringing the DZ signal to the HI state, thereby disabling the '157 outputs and causing the Adders to perform the operation D + 0 = D.

The logical flow taken from <a href="https://tomnisbet.github.io/nqsap/docs/dxy-registers/" targe ="_blank">Tom Nisbet's explanation</a> clarifies the operation:

![D, X and Y registers in action](../../../assets/dxy/60-dxy-nqsap-tom-flow.png "D, X and Y registers in action"){:width="66%"}

### Use for conditional jumps

A conditional jump is executed if a certain condition is met. In the following example, the BCS instruction is unconditionally executed. The operand value $03 is added to the address of the instruction *following* the jump instruction, i.e. $03 + $83 = $86, which will be loaded into the Program Counter (PC).

~~~text
SEC         ; $80 - Set Carry Flag
BCS $03     ; $81 - Salta a $03 + $83 = $86 if Carry is set
INX         ; $83 - This instruction will be skipped
INX         ; $84 - This instruction will be skipped
INX         ; $85 - This instruction will be skipped
LDA #$01    ; $86 - This instruction will be executed
~~~

The operand is a signed 8-bit value that can range from -128 to +127, which means that conditional jumps can jump forward by 128 addresses and backward by 127.

To jump to an address preceding the jump instruction, the operand value must be between $80 (-128) and $FF (-1), according to the rules described in the [Unsigned and Signed Numbers](../math/#unsigned-and-signed-numbers) section of the Binary Arithmetic page (if I add 0x01 to the largest address 0xFF, I return to address 0x00).

~~~text
LDX #$05    ; $80 - Load register X with 5
DEX         ; $82 - Decrement X
BNE $FD     ; $83 - Jump to $85 + $FD = $85 - $03 = $82 if X is not zero
RTS         ; $85 - Return from subroutine
~~~

Since the configuration used for the '283 Adders only allows additions to be performed, one might wonder how it is possible to execute a backward jump. In the NQSAP, as in the BEAM, registers are 8 bits wide: a simple shortcut to achieve the desired result is to add the operand to the PC treating both address and operand as unsigned numbers: in this specific case, $85 + $FD = $182. The 9th bit (corresponding to the Carry) is not taken into account and the remaining $82 will be loaded into the PC.

This technique works correctly thanks to the cyclic nature of memory addressing in an 8-bit system.

[![Schematic of the NQSAP index registers](../../../assets/dxy/60-nqsap-dxy-schema.png "Schematic of the NQSAP index registers"){:width="100%"}](../../../assets/dxy/60-nqsap-dxy-schema.png)

*Schematic of the NQSAP index registers.*

## Schematic

[![Schematic of the BEAM computer index registers](../../../assets/dxy/60-beam-dxy-schema.png "Schematic of the BEAM computer index registers"){:width="100%"}](../../../assets/dxy/60-beam-dxy-schema.png)

*Schematic of the BEAM computer index registers.*

## Differences between NQSAP and BEAM index registers

From a functional standpoint, the schematics of the NQSAP and BEAM index registers are identical.

In my notes I had written that "… as with the other BEAM registers, here too I use <a href="https://www.ti.com/lit/ds/symlink/sn54ls377.pdf" target="_blank">74LS377</a> D-type registers instead of the Octal D-Type Flip-Flop with 3-State Outputs <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> used by Tom in the NQSAP"; see the section [The NQSAP ALU](../alu/#the-nqsap-alu) for clarification on this point.

For completeness, I should mention that I became acquainted with the 74LS377 while studying the <a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, an evolution of the NQSAP that Tom had subsequently engineered on PCB rather than on breadboard.

## Useful links

- Tom Nisbet's <a href="https://tomnisbet.github.io/nqsap/docs/dxy-registers/" target="_blank">NQSAP DXY registers</a>.
