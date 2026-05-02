---
title: "BEAM"
lang: en
locale: en-US
permalink: /docs/en/home/
excerpt: "My 8-bit TTL breadboard computer based on Ben Eater's design and Tom Nisbet's improvements."
header:
#   image: /assets/home/beam.png
  overlay_image: /assets/home/beam.png
  overlay_filter: 0.5
---
## BEAM 8-bit computer

<small>[The First Project: 6502](#the-first-project-6502) - [8-Bit TTL Logic Computer: SAP](#8-bit-ttl-logic-computer-sap) - [Evolution of the 8-Bit Computer: BEAM](#evolution-of-the-8-bit-computer-beam) - [BEAM?](#beam) - [Study and Design](#study-and-design) - [Implementation](#implementation) - [Notes](#notes) - [Credits and Acknowledgments](#credits-and-acknowledgments) - [Literature](#literature) - [Intellectual Property Notice](#intellectual-property-notice)</small>

<!-- TLDR: vai subito alla [documentazione del BEAM 8-bit computer](https://andreamazzai.github.io/beam/) su GitHub Pages -->

>NOTE: In some parts of the documentation, it is assumed that the reader has built or understands the SAP-1 Computer project by Ben Eater. Where possible, references are provided to facilitate consultation.

After several decades since completing my studies (a high school diploma in electronics obtained in 1989), in 2021, I decided to return to electronics, particularly digital electronics. I began exploring Arduino and Raspberry Pi but immediately encountered some difficulties: my programming and system administration skills were either underdeveloped or very rusty. I had no knowledge of C or other compiled languages, and my understanding of Linux was quite limited.

### The First Project: 6502

Toward the end of the year, I accidentally came across <a href="https://eater.net/" target="_blank">Ben Eater's blogs</a> and <a href="https://www.youtube.com/@BenEater/playlists" target="_blank">videos</a>. Remembering my first computer (a Commodore 64 from 1984) and the fact that I had done some assembly programming back then, I built Ben’s breadboard computer based on the 6502. I also started reading numerous blogs and forums on the subject, reviving some of my old knowledge.

### 8-Bit TTL Logic Computer: SAP

In the early months of 2022, after completing the basic modules of the 6502 computer, I followed the **SAP-1** (Simple As Possible) 8-bit TTL logic computer <a href="https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU" target="_blank">playlist</a> which I became even more passionate about. During the build, I learned a great deal — both thanks to Ben’s excellent presentation style and the significant effort I put into deepening my understanding through other websites and books.

[![My Implementation of the SAP-1 Computer.](../../../assets/home/sap.png "My Implementation of the SAP-1 Computer."){:width="66%"}](../../../assets/home/sap.png)

*My Implementation of the SAP-1 Computer.*

### Evolution of the 8-Bit Computer: BEAM

I completed the SAP-1 computer in September, but before that, I had started studying the improvements and expansions made by other members of Ben's community, particularly regarding the possibility of automating the program loading process with a bootloader, as well as the need to increase the RAM beyond 16 bytes. During the summer of that year, I discovered the fantastic <a href="https://tomnisbet.github.io/nqsap/" target="_blank">NQSAP</a> project by **Tom Nisbet**: an 8-bit computer based on Ben Eater’s design but engineered to emulate the 6502 instructions. I immediately knew this would be my next project because it brought together many aspects I found very interesting:

1. Evolution of Ben Eater’s Computer, with which I had started to regain confidence in digital electronics.
2. Emulation of the processor from my first computer, complete with index registers, addressing modes, and Stack Pointer.
3. In Ben Eater’s original design, the computer had only 16 bytes of RAM. This amount was enough to program a minimal Fibonacci sequence, but it was absolutely inadequate for creating more complex programs. Expanding the RAM, therefore, became essential.
4. The need to learn at least the basics of C, which was used to create the [EEPROM programmer](docs/eeprom-programmer/) and the Arduino-based [Loader](docs/loader/).

### BEAM?

When searching for a name for my project, I identified "BEAM" as the ideal choice: I wanted to honor Ben Eater for bringing me back to this wonderful world and add something of my own, so **B**en **E**ater **A**ndrea **M**azzai ==> **BEAM** was born.

## Study and Design

In the first eight months of work on the BEAM project, I dedicated myself entirely to studying. The goal was to understand every detail of the NQSAP, which introduced numerous additions to the SAP-1 and implemented some truly ingenious ideas. This period of analysis was crucial for mastering the new concepts and innovations of the project.

The documentation for the NQSAP was incomplete, and I had also found a few errors, which I shared with Tom, who promptly corrected them. To deepen my understanding of the subject, after studying the NQSAP, I also analyzed the NQSAP-PCB evolution, to which Tom had made several improvements. In creating the BEAM, I aimed to take the best from both projects.

Among the aspects to mention, on which I spent a lot, **a **lot**, ***really a lot of time***:

- understanding the operation of the 74181 ALU, for which I created a test bench based on what I learned from [David Courtney](docs/en/alu/#useful-links);
- understanding binary arithmetic in two's complement;
- understanding overflow, closely related to the previous point;
- integration of the 74181 ALU and Instruction Register (IR) in "[hardwired](docs/en/alu/#relazione-diretta-hardwired-tra-instruction-register-e-alu)" mode to automatically generate the ALU input signals based on the instruction present in the IR;
- new memory module based on SRAM with common IO pins, unlike the 74189 used in the SAP-1 with dedicated input and output ports;
- the flag register—certainly the most complex part of the computer:
  - using the 74151 to decode the overflow state and manage the flag itself—an issue closely tied to the perfect understanding of binary arithmetic;
  - Using the 74151 to handle [conditional jump instructions in "hardwired" mode](docs/en/flags/#i-salti-condizionali-e-incondizionati), similar to how it was done for the ALU;

- understanding the Stack Pointer and writing microcode for the instructions that use it;
- development of the EEPROM programmer / microcode software based on Arduino: I studied Tom's code in depth, which I mostly understood:
  - schematization of the 6502 instructions and categorization;
  - understanding the very clever method developed by Tom to address more than 16 arithmetic instructions with only 5 signals dedicated to the ALU operation selection signals;
- understanding the "glitch" problem;
- development of the bootloader software.

[![Logical Diagram of the BEAM Computer, July 2023.](../../../assets/home/hand-drawn-logic.jpg "Logical Diagram of the BEAM Computer, July 2023."){:width="66%"}](../../../assets/home/hand-drawn-logic.jpg)

*Logical Diagram of the BEAM Computer, July 2023..*

In July 2023, I tried to put together everything I had learned and attempted to transcribe it on paper: I had a good understanding of how some modules worked, while I still had many doubts about others. Additionally, several interactions between modules, especially in the control signals for the flags, were still unclear. By re-reading the material I had studied and the notes I had transcribed on OneNote in the previous months, I connected the dots and created the logical diagram of the computer, beginning to see not just a series of standalone modules, but a coherent logical construct.

[![Paper representation of a possible layout of the BEAM computer modules](../../../assets/home/beam-paper-breadboards.png "Paper representation of a possible layout of the BEAM computer modules"){:width="66%"}](../../../assets/home/beam-paper-breadboards.png)

*Paper representation of a possible layout of the BEAM computer modules.*

## Implementation

The construction phase started in October 2023 and was completed in the first days of May 2024. The result is what can be seen in the photo.

These pages contain notes, observations, and lessons learned and applied in the creation of BEAM, analysis of the NQSAP and NQSAP-PCB modules, differences between the systems, some comparisons with the SAP-1, and much more.

[![BEAM Breadboard Computer](../../../assets/home/beam.png "BEAM breadboard computer"){:width="66%"}](../../../assets/home/beam.png)

Among the things I learned:

1. The flags (of the 6502), their role, and their interactions within a computer  
2. The role of an ALU and the representation of Signed numbers  
3. Creation of a very fast EEPROM programmer  
4. Basics of the C language  
5. Basics of Arduino  
6. Use of Visual Studio Code  
7. The Markdown language  
8. Use of GitHub and GitHub Pages  
9. Schematic design with KiCad

### Notes

- The frequency range of the BEAM computer varies from about one Hertz to about 2 KHz. Higher frequencies were not tested, as the main goal was not to achieve high performance but rather to acquire knowledge.
- The KiCad schematics do not include most of the individual LEDs: only the LEDs in the Clock module and those indicating the Program Mode / Run Mode in the RAM / MAR module are shown. Almost all of the LED Bars, however, are represented.

## Credits and Acknowledgments

- Ben Eater for his incredible ability to create <a href="https://www.youtube.com/@BenEater/playlists/" target="_blank">such interesting content</a>, especially for the <a href="https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU" target="_blank">Building an 8-bit breadboard computer</a> playlist;
- Tom Nisbet for the <a href="https://tomnisbet.github.io/nqsap/" target="_blank">NQSAP</a> and <a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, significantly expanding and improving Ben Eater's project;
- the <a href="https://www.reddit.com/r/beneater/" target="_blank">r/beneater</a> subreddit - especially The8BitEnthusiast — thank you!
- Ken Shirrif for some <a href="https://www.righto.com/" target="_blank">very interesting articles</a>  on Overflow, the 6502, 74181, and much more!
- The enthusiastic and inspiring Dr. Brock LaMeres from the University of Montana for the excellent <a href="https://www.youtube.com/playlist?list=PL643xA3Ie_Et2uM4xu1yFk-A5ZQQ8gQ5e" target="_blank">Intro to Logic Circuits</a> playlist on YouTube; a <a href="https://www.montana.edu/blameres/book_content_vhdl.html" target="_blank">more readable index</a> of the same material is available on the official University of Montana website;
- The <a href="http://6502.org" target="_blank">6502.org</a> website and its forum: detailed, no-frills documentation and truly experienced members;
- Dieter Müller, whose pages on the <a href="http://6502.org/users/dieter/v_flag/v_0.htm" target="_blank">74LS151</a> inspired me to delve into the topic of Overflow in detail, with whom I developed a very friendly online acquaintance;
- Norbert Landsteiner for the <a href="https://www.masswerk.at/6502/6502_instruction_set.html" target="_blank">6502 Instruction Set</a>, the <a href="https://www.masswerk.at/6502/assembler.html" target="_blank">6502 Assembler</a>, and the <a href="https://www.masswerk.at/6502" target="_blank">Virtual 6502</a> - exceptional tools I used extensively for the creation and debugging of the microcode. Norbert was also extremely responsive in fixing a bug I reported to him.

Special thanks to Tom Nisbet and Dieter Müller, who, even during the documentation writing phase, did not hesitate to provide me with some clarifications.

## Literature

- <a href="https://archive.org/details/Programming_the_6502_OCR" target="_blank">Programming the 6502</a> by  Rodnay Zaks.
- <a href="https://archive.org/details/367026792DigitalComputerElectronicsAlbertPaulMalvinoAndJeraldABrownPdf1/" target="_blank">Digital Computer Electronics</a> by Albert Paul Malvino and Jerald A. Brown.

## Intellectual Property Notice

This project may include links, documents, or images owned by third parties. I am committed to fully respecting the intellectual property rights of all owners and do not intend to infringe any copyrights.

If it is believed that any materials in this documentation violate your copyright, please notify me. I will promptly verify the claim and, if a violation is confirmed, will immediately remove the material in question.

On the other hand, all original material produced in this project is released under the MIT License. This means that it is freely usable, modifiable, and distributable by others, provided that the original copyright notice is maintained, and this same license is included in any copies or modified versions of the project.
