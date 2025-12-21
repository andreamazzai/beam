// per la connessione si usa ttyUSB0
// Definizione dei pin immaginando di avere l'USB di Arduino a sinistra
#define SHIFT_DATA 2        // GPIO  D2 =  5 Nano
#define SHIFT_CLK 3         // GPIO  D3 =  6 Nano
#define SHIFT_LATCH 4       // GPIO  D4 =  7 Nano
#define RESET 5             // GPIO  D5 =  8 Nano
#define LDR_CLK 6           // GPIO  D6 =  9 Nano
#define LDR_Active 7        // GPIO  D7 = 10 Nano
#define SHIFT_ENABLE_1 8    // GPIO  D8 = 11 Nano; drive 138s
#define START_STOP_CLOCK 9  // GPIO  D9 = 12 Nano (CLK-Start su KiCad)
#define PISO_READ 10        // GPIO D10 = 13 Nano
#define PISO_CLK 11         // GPIO D11 = 14 Nano
#define PISO_LOAD 12        // GPIO D12 = 15 Nano
#define SHIFT_ENABLE_2 13   // GPIO D13 = 16 Nano; drive data/address bus

const int sequence = 27; // array for KITT supercar LEDs effect
const int LEDs = 8;

#include <math.h>
#include <Arduino.h>

#define DELAY 1 // ritardo nelle scritture di MAR e RAM

void writeProgram();
void setAddress(byte address);
void setRegister(byte registers);
void writeRAM(byte data);
void set_for_programming();
void post_programming();
void waitForKeyPress(String stringa);
void blink();
void reset();
void writeKitt();

// ************************************************************
// ********* PROGRAMMA BELLO LUNGO PER TESTARE TUTTO **********
// ************************************************************
  byte TestProgram1[] = {
    0x25, 0x00, // LDY #$00     4 - Z - check 2024-01-28
    0x25, 0x80, // LDY #$80     4 - N - check 2024-01-28
    0xCF,       // DEY          5 -   - check 2024-01-28
    0x28, 0x01, // LDA #$01     4 -   - check 2024-01-28
    0x1C,       // TYA          3 -   - check 2024-01-28
    0xE0,       // INA          3 - N - check 2024-01-28
    0x07, 0x50, // CPY #$50     7 - C - check 2024-01-28  (Y = 7F), A > Y
    0x07, 0x7F, // CPY #$7F     7 - ZC - check 2024-01-28 (Y = 7F), A = Y
    0x07, 0x80, // CPY #$80     7 - N - check 2024-01-28  (Y = 7F), A < Y
    0x10,       // OUT          3
    0x03,       // SEC          3 - NC - check 2024-01-28 (N c'è da prima)
    0x23,       // CLC          3 - N  - check 2024-01-28
    0x28, 0x10, // LDA #$10     4 -   - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x10, // ADC #$10     5 -   - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x60, // ADC #$60     5 - NV   - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x80, // ADC #$80     5 -  VZC - check 2024-01-28
    0x10,       // OUT          3 
  // $20
    0x24, 0x12, // LDX #$12     4 - VC se arrivo da block precedente, altrimenti niente (LD* modifica solo NZ)
    0x1B,       // TXA          3 - VC se arrivo da block precedente, altrimenti niente
    0x10,       // OUT          3 - VC se arrivo da block precedente, altrimenti niente
    0xAF,       // DEX          5 - VC se arrivo da block precedente, altrimenti niente
    0x06, 0x10, // CPX #$10     7 - 1st A > X ==> VC, 2nd A = X ==> VZC (V c'è da prima)
    0x72, 0xF9, // BNE TXA     16 - come precedente; BNE conta da byte + 1 rispetto operando fino a $00
  // $29
    0x24, 0xFF, // LDX #$FF     4 - NV C - VC solo se arrivo da prima
    0x1D,       // TXS            - come precedente
    0x28, 0x70, // LDA #$70     4 -  V C - pulisce NZ se presenti da prima, mantiene VC se presenti da prima
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x43,       // CLV          3 -      - pulisce V se presente 
    0x03,       // SEC          3 -    C - check 2024-02-03 (N c'è da prima)
    0x26, 0x10, // SBC #$10     5 -    C - check 2024-02-04
    0x26, 0x60, // SBC #$60     5 -   ZC - check 2024-02-04
    // 0x00,       // HLT
    0x19,       // PHP          5 -   ZC - scrive bit Z=6 e C=4    01010000
    0x23,       // CLC          3 -   Z  - pulisce C se presente  
    0x28, 0x70, // LDA #$70     4 -      - il valore non è più 0 dunque Z dovrebbe sparire
    // 0x00,       // HLT
    0x18,       // PLP          5 -   ZC - dovrebbe ripristinare flag bit Z=6 e C=4    01010000
  // $3C
    0x28, 0x70, // LDA #$70     4 -    C - resta C da prima
    0x26, 0x78, // SBC #$78     5 - N    - check 2024-02-04
    0x28, 0xA0, // LDA #$A0     4 - N    - N perché signed > 7F, corrisponde a -96 signed
    0x26, 0x40, // SBC #$40     5 -  V C - check 2024-02-04, risultato $60 corrisponde a 96 signed, overflow
  // $44
    0x03,       // SEC          3 -  V C - check 2024-02-03, V se presente da prima
    0x28, 0xA0, // LDA #$A0     4 - NV C - N perché signed > 7F, corrisponde a -96 signed
    0x26, 0x40, // SBC #$40     5 -  V C - check 2024-02-04, risultato $60 corrisponde a 96 signed, overflow
    // 0x00,       // HLT
    0x24, 0x20, // LDX #$20     4 - VC se arrivo da block precedente, altrimenti niente
    0xAF,       // DEX          5 - come precedente
    0x1B,       // TXA          3 - come precedente
    0x10,       // OUT          3 - come precedente
    0x06, 0x10, // CPX #$10     7 - 1st A > X ==> VC, n° A = X ==> VZC
    0x72, 0xF9, // BNE TXA     16 - come precedente; BNE conta da byte + 1 rispetto operando fino a $00
    // 0x00,       // HLT
    0x24, 0x30, // LDX #$30     4 -  V C -
    0xAF,       // DEX          5 - come precedente
    0x1B,       // TXA          3 - come precedente
    0x10,       // OUT          3 - come precedente
    0x06, 0x28, // CPX #$28     7 - 1st A > X ==> VC, n° A = X ==> VZC
    0x72, 0xF9, // BNE TXA     16 - come precedente; BNE conta da byte + 1 rispetto operando fino a $00
    // 0x00,       // HLT
    0x23,       // CLC          3 -  VZ  - pulisce C se presente  
    0x43,       // CLV          3 -   Z  - pulisce V se presente 
    // 0x00,       // HLT
  // $61
    0x03,       // SEC          3 - C Z - Z se presente da prima
    0x28, 0x7F, // LDA #$7F     4 - C  -
    0xEC,       // ASL A        5 - N  - shift verso SX; MSB = 0 perciò C = 0
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0xEC,       // ASL A        5 - N C - shift verso SX; MSB = 1 perciò C = 1
    0x10,       // OUT          3 OUT 252 $FC
    // 0x00,       // HLT
    0x28, 0xFF, // LDA #$FF     4 - N C - N perché bit 7 = HI; C da prima
    0xEC,       // ASL A        5 - N C - shift verso SX; MSB = 1 perciò C = 1
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0xED,       // LSR A        5 -    - shift verso DX; LSB = 0 perciò C = 0
    0x10,       // OUT          3 OUT 127 $7F
    // 0x00,       // HLT
    0x28, 0xFF, // LDA #$FF     4 - N  - N perché bit 7 = HI
    0x4A, 0xC0, // STA $C0      5
    // 0x00,       // HLT
    0x4C, 0xC0, // ASL $C0      9 - N C - shift verso SX e dunque C = 1
    0x48, 0xC0, // LDA $C0      5
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0x28, 0x7E, // LDA #$7E     4 -    C -
    0x4A, 0xC1, // STA $C1      5
    0x4C, 0xC1, // ASL $C1      9 - N    - shift verso SX e dunque C = 0
    0x48, 0xC1, // LDA $C1      5 - N    -
    0x10,       // OUT          3 OUT 252 $FC
    // 0x00,       // HLT
    0x28, 0x7E, // LDA #$7E     4 -      -
    0x4A, 0xC1, // STA $C1      5
    0x24, 0x11, // LDX #$11
    0x6C, 0xB0, // ASL $B0,X   10 - N    - shift verso SX e dunque C = 0
    0x6C, 0xB0, // ASL $B0,X   10 - N  C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X   10 - N  C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X   10 - N  C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X   10 - N  C - shift verso SX e dunque C = 1
    0x48, 0xC1, // LDA $C1      5 - N  C -
    0x10,       // OUT          3 OUT 192 $C0
    // 0x00,       // HLT
  // $9B
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x28, 0xFF, // LDA #$FF     4 - N    -
    0x10,       // OUT          3 OUT 255 $FF
    // 0x00,       // HLT
    0xFC,       // ROL A        4 - N  C - shift verso SX e dunque C = 1; 0 nell'LSB
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x28, 0x7E, // LDA #$7E     4 -      -
    0x10,       // OUT          3 OUT 126 $7E
    // 0x00,       // HLT
    0xFC,       // ROL A        4 - N    - shift verso SX e dunque C = 0; 0 nell'LSB
    0x10,       // OUT          3 OUT 252 $FC
    // 0x00,       // HLT
    0xFD,       // ROR A        4 -      - shift verso SX e dunque C = 0; 0 nell'LSB
    0x10,       // OUT          3 OUT 126 $7E
    // 0x00,       // HLT
    0x51, 0x00  // JMP $00
    ,
};

byte CounterUpDown[] = {
// count 0-255 then back and restart
// branch count starts at operand address + 1; target @ $00
    0x28, 0xFF, // LDA #$FF
    0xE0,       // INA
    0x10,       // OUT
    0x72, 0xFC, // BNE $02
    0xEF,       // DEA
    0x10,       // OUT
    0x72, 0xFC, // BNE $06
    0x51, 0x02, // JMP $02
    
    0x24, 0xFF, // LDX #$FF
    0xAF,       // DEX
    0x72, 0xFC, // BNE $DEX
    0x11        // RTS    
    ,
};

byte Fibonacci[] = {

    0x28, 0x00, // LDA #$00
    0x10,       // OUT
    0x4A, 0xC0, // STA $C0  1st
    0x4A, 0xC1, // STA $C1  2nd
    0x4A, 0xC2, // STA $C2  result
    0x28, 0x01, // LDA #$01
    0x4A, 0xC1, // STA $C1  2nd
    0x28, 0x00, // LDA #$00
    0x49, 0xC1, // ADC $C1  2nd
    0x4A, 0xC2, // STA $C2  result
    0x32, 0xEB, // BCS $00
    0x10,       // OUT
    0x48, 0xC1, // LDA $C1  2nd
    0x4A, 0xC0, // STA $C0  1st
    0x48, 0xC2, // LDA $C2  result
    0x4A, 0xC1, // STA $C1  2nd
    0x48, 0xC0, // LDA $C0  1st
    0x51, 0x0F  // JMP $0F // salta a ADC
    ,
};

byte DownCounter[] = {
// byte PROGRAM_DOWN[] = {
    0x28, 0x00, // LDA #$00
    0x24, 0xF0, // LDX #$F0
    0x25, 0x0F, // LDY #$0F
    0xEF,       // DEA
    0x10,       // OUT
    0x01, 0x0A, // JMP ($0A)
    0x06        // locazione che contiene l'indirizzo per l'indirect jump
    ,
};

// ************************************************************
// ***************** PROGRAMMI DA CONTROLLARE *****************
// ************************************************************

byte PROGRAM_SHIFT[] = {
// byte PROGRAM[] = {
    0x03,       // SEC          3 -    C
    0x28, 0x41, // LDA #$41     4 -    C - $41 = b0100.0001
    0xEC,       // ASL A        5 - N    - shift SX; MSB = 0 => C = 0; precedente bit 6 = 1 => N = 1
    0x10,       // OUT          3 OUT 130 = $82 = b1000.0010
    // 0x00,       // HLT
    0xEC,       // ASL A        5 -    C - shift SX; precedente MSB = 1 => C = 1
    0x10,       // OUT          3 OUT   4 = $04 = b0000.0100
    // 0x00,       // HLT
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x28, 0xFF, // LDA #$FF     4 - N    -
    0xED,       // LSR A        5 -    C - shift verso DX; LSB = 1 => C = 1
    0x10,       // OUT          3 OUT 127 = $7F = b0111.1111
    // 0x00,       // HLT
    0xFD,       // ROR A        4 - N  C - shift verso DX; al primo passaggio C = 1 >> MSB = 1; LSB = 1 => C = 1
    0x10,       // OUT          3 OUT 191 = $BF = b1011.1111
    0x32, 0xFC, // BCS ROR     16 Loop until C = 0 e tutti i bit a 1
    0x10,       // OUT          3 OUT 255 = $FF = b1111.1111
    // 0x00,       // HLT
    0x03,       // SEC          3 -    C
    0xFC,       // ROL A        4 - N  C - shift SX; precedente MSB = 1 => C = 1
    0x10,       // OUT          3 OUT 255 = $FF = b1111.1111
    // 0x00,       // HLT
 // Inserire qui quotazioni con registri indicizzati
    0xEC,       // ASL A        5 - N  C - shift verso SX; MSB = 1 perciò C = 1
    0x10,       // OUT          3 OUT 254 $FE
    0x10,       // OUT          3 OUT 127 $7F
    // 0x00,       // HLT
    0x28, 0xFF, // LDA #$FF     4 - N  - N perché bit 7 = HI
    0x4A, 0xC0, // STA $C0      5
    // 0x00,       // HLT
    0x4C, 0xC0, // ASL $C0      9 - N C - shift verso SX e dunque C = 1
    0x48, 0xC0, // LDA $C0      5
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0x28, 0x7E, // LDA #$7E     4 -    C -
    0x4A, 0xC1, // STA $C1      5
    0x4C, 0xC1, // ASL $C1      9 - N    - shift verso SX e dunque C = 0
    0x48, 0xC1, // LDA $C1      5 - N    -
    0x10,       // OUT          3 OUT 252 $FC
};
byte PROGRAM_4[] = {
    0x28, 0x10, // LDA #$10
    // 0x28, 0xFF, // LDA #$FF
    0x10,       // OUT
    0x0B,       // TAX
    0xE0,       // INA
    0x10,       // OUT
    0x0C,       // TAY
    0x28, 0x50, // LDA #$50
    0x10,       // OUT
    0x0B,       // TAX
    0x54, 0xAA, // STX $AA
    0x45, 0xAA, // LDY $AA
    0x55, 0x40, // STY $40
    0x24, 0x66, // LDX #$66
    0x1B,       // TXA
    0x10,       // OUT
    0x0C,       // TAY
    0x48, 0xAA, // LDA $AA
    0x10,       // OUT
    0x29, 0x0F, // ADC #$0F
    0x10,       // OUT
    0x1C,       // TYA
    0x10,       // OUT
    0x24, 0xAA, // LDX #$AA
    0x54, 0x40, // STX $40
    0x24, 0x10, // LDX #$10
    0x68, 0x30, // LDA $30, X
    0x10,       // OUT
    0x00        // HLT
    ,
};
byte PROGRAM_STACK[] = {
// byte PROGRAM_STACK[] = {
    // 0x25, 0x40, // LDY #$40
    // 0x1C,       // TYA
    // 0x0B,       // TAX
    // 0x10,       // OUT
    // 0x00,       // HLT
    0x24, 0xFF, // LDX #$FF
    0x1D,       // TXS
    0x41, 0x07, // JSR $07   ==> attenti a dove si punta
    0x51, 0x03, // JMP $03
    0x28, 0x64, // LDA #$64
    0x10,       // OUT
    0xEF,       // DEA
    0x72, 0xFC, // BNE $----
    0x10,       // OUT
    0x28, 0x9A, // LDA #$9A
    0x10,       // OUT
    0xE0,       // INA
    0x72, 0xFC, // BNE $----
    0x10,       // OUT
    0x11,       // RTS
    0x09,       // PHA
    0x00,       // HLT
    0x28, 0xC3, // LDA #$C3
    0x09,       // PHA
    0x00,       // HLT
    0x28, 0x66, // LDA #$66
    0x09,       // PHA
    0x00,       // HLT
    0x09,       // PHA
    0x28, 0x01, // LDA #$01
    0x08,       // PLA
    0x10,       // OUT
    0x00,       // HLT
    0x08,       // PLA
    0x10,       // OUT
    0x00,       // HLT
    0x08,       // PLA
    0x10,       // OUT
    0x00        // HLT
    // 0x05        // locazione che contiene l'indirizzo per l'indirect jump
    // 0x24, 0xCC, // LDX #$CC
    // 0x25, 0x33, // LDY #$33
    ,
};

// ************************************************************
// *********                                         **********
// ************************************************************
// Sembra ok, non ho controllato i Flag
byte PROGRAM_20[] = {
    0x28, 0xCC, // LDA #$CC = 16 * 12 + 12 = 204
    0x10,       // OUT
    0x4A, 0x40, // STA $40
    0xE0,       // INA
    0x10,       // OUT
    0x24, 0x10, // LDX #$10
    0x68, 0x30, // LDA $30, X
    0x10,       // OUT
    0x00,       // HLT
    0x28, 0xCE, // LDA #$CC = 16 * 12 + 14 = 206
    0x10,       // OUT
    0x4A, 0x40, // STA $40
    0xE0,       // INA
    0x10,       // OUT
    0x24, 0x10, // LDX #$10
    0x68, 0x30, // LDA $30, X
    0x10,       // OUT
    0x00        // HLT
    ,
};

byte PROGRAM_plp[] = {
// byte PROGRAM_CHECK_CMP[] = {
    // 0x51, 0x20, // JMP CICLO
    // 0x0F, 0x0F, // NOP NOP
    0x51, 0x29, // JMP SEC      4
    // 0x51, 0x57, // JMP ASL      4
    // 0x51, 0x5E, // JMP ROL      4
    0x25, 0x00, // LDY #$00     4 - Z - check 2024-01-28
    0x25, 0x80, // LDY #$80     4 - N - check 2024-01-28
    0xCF,       // DEY          5 -   - check 2024-01-28
    0x28, 0x01, // LDA #$01     4 -   - check 2024-01-28
    0x1C,       // TYA          3 -   - check 2024-01-28
    0xE0,       // INA          3 - N - check 2024-01-28
    0x07, 0x50, // CPY #$50     7 - C - check 2024-01-28  (Y = 7F), A > Y
    0x07, 0x7F, // CPY #$7F     7 - ZC - check 2024-01-28 (Y = 7F), A = Y
    0x07, 0x80, // CPY #$80     7 - N - check 2024-01-28  (Y = 7F), A < Y
    0x10,       // OUT          3
    0x03,       // SEC          3 - NC - check 2024-01-28 (N c'è da prima)
    0x23,       // CLC          3 - N  - check 2024-01-28
    0x28, 0x10, // LDA #$10     4 -   - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x10, // ADC #$10     5 -   - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x60, // ADC #$60     5 - NV  - check 2024-01-28
    0x10,       // OUT          3
    0x29, 0x80, // ADC #$80     5 - VZC - check 2024-01-28
    0x10,       // OUT          3
// CICLO $20
    0x24, 0x12, // LDX #$12     4 - VC se arrivo da block precedente, altrimenti niente (LD* modifica solo NZ)
    0x1B,       // TXA          3 - VC se arrivo da block precedente, altrimenti niente
    0x10,       // OUT          3 - VC se arrivo da block precedente, altrimenti niente
    0xAF,       // DEX          5 - VC se arrivo da block precedente, altrimenti niente
    0x06, 0x10, // CPX #$10     7 - 1st A > X ==> VC, 2nd A = X ==> VZC (V c'è da prima)
    0x72, 0xF9, // BNE TXA     16 - VC o VZC - devo contare dal byte + 1 rispetto all'operando e arrivare a 00
// SEC $29
      0x24, 0xFF, // LDX #$FF
      0x1D,       // TXS
    0x28, 0x70, // LDA #$70     4 -    - pulisce NZ se presenti da prima, mantiene VC se presenti da prima
    0x23,       // CLC          3 -    - pulisce C se presente  
    0x43,       // CLV          3 -    - pulisce V se presente 
    0x03,       // SEC          3 - C  - check 2024-02-03 (N c'è da prima)
    0x26, 0x10, // SBC #$10     5 - C  - check 2024-02-04
    0x26, 0x60, // SBC #$60     5 - ZC - check 2024-02-04
      0x00,       // HLT
      0x19,       // PHP          5 - ZC - scrive bit Z=6 e C=4    01010000
      0x23,       // CLC          3 - Z  - pulisce C se presente  
      0x28, 0x70, // LDA #$70     4 -    - il valore non è più 0 dunque Z dovrebbe sparire
      0x00,       // HLT
      0x18,       // PLP          5 - ZC - dovrebbe ripristinare flag bit Z=6 e C=4    01010000
// SEC $32
    0x03,       // SEC          3 - ZC - check 2024-02-03 - Z se vengo dal blocco precedente
    0x28, 0x70, // LDA #$70     4 - C  - resta C da prima
    0x26, 0x78, // SBC #$78     5 - N  - check 2024-02-04
    0x28, 0xA0, // LDA #$A0     4 - N  - N perché signed > 7F, corrisponde a -96 signed
    0x26, 0x40, // SBC #$40     5 - VC  - check 2024-02-04, risultato $60 corrisponde a 96 signed, overflow
// SEC $3B
    0x03,       // SEC          3 - C  - check 2024-02-03 - Z se vengo dal blocco precedente
    0x28, 0xA0, // LDA #$A0     4 - NC  - N perché signed > 7F, corrisponde a -96 signed
    0x26, 0x40, // SBC #$40     5 - VC  - check 2024-02-04, risultato $60 corrisponde a 96 signed, overflow
    0x00,       // HLT
    0x24, 0x20, // LDX #$20     4 - VC se arrivo da block precedente, altrimenti niente
    0xAF,       // DEX          5
    0x1B,       // TXA          3
    0x10,       // OUT          3
    0x06, 0x10, // CPX #$10     7 - 1st A > X ==> C, 2nd A = X ==> ZC
    0x72, 0xF9, // BNE TXA     16 - devo contare dal byte + 1 rispetto all'operando e arrivare a 00
    0x00,       // HLT
    0x24, 0x30, // LDX #$30     4 - VC se arrivo da block precedente, altrimenti niente
    0xAF,       // DEX          5
    0x1B,       // TXA          3
    0x10,       // OUT          3
    0x06, 0x28, // CPX #$28     7 - 1st A > X ==> C, 2nd A = X ==> ZC
    0x72, 0xF9, // BNE TXA     16 - devo contare dal byte + 1 rispetto all'operando e arrivare a 00
    0x00,       // HLT
    0x23,       // CLC          3 -    - pulisce C se presente  
    0x43,       // CLV          3 -    - pulisce V se presente 
// ASL $57    
    // 0x00,       // HLT
    0x03,       // SEC          3 - C  -
    0x28, 0x7F, // LDA #$7F     4 - C  -
    0xEC,       // ASL A        5 - N  - shift verso SX; MSB = 0 perciò C = 0
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0xEC,       // ASL A        5 - N C - shift verso SX; MSB = 1 perciò C = 1
    0x10,       // OUT          3 OUT 252 $FC
    // 0x00,       // HLT
    0x28, 0xFF, // LDA #$FF     4 - N C - N perché bit 7 = HI; C da prima
    0xEC,       // ASL A        5 - N C - shift verso SX; MSB = 1 perciò C = 1
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0xED,       // LSR A        5 -    - shift verso DX; LSB = 0 perciò C = 0
    0x10,       // OUT          3 OUT 127 $7F
    // 0x00,       // HLT
    0x28, 0xFF, // LDA #$FF     4 - N  - N perché bit 7 = HI
    0x4A, 0xC0, // STA $C0      5
    // 0x00,       // HLT
    0x4C, 0xC0, // ASL $C0      9 - N C - shift verso SX e dunque C = 1
    0x48, 0xC0, // LDA $C0      5
    0x10,       // OUT          3 OUT 254 $FE
    // 0x00,       // HLT
    0x28, 0x7E, // LDA #$7E     4 -    -
    0x4A, 0xC1, // STA $C1      5
    0x4C, 0xC1, // ASL $C1      9 - N  - shift verso SX e dunque C = 0
    0x48, 0xC1, // LDA $C1      5
    0x10,       // OUT          3 OUT 252 $FC
    0x00,       // HLT
    0x28, 0x7E, // LDA #$7E     4 -    -
    0x4A, 0xC1, // STA $C1      5
    0x24, 0x11, // LDX #$11
    0x6C, 0xB0, // ASL $B0,X    10 - N   - shift verso SX e dunque C = 0
    0x6C, 0xB0, // ASL $B0,X    10 - N C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X    10 - N C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X    10 - N C - shift verso SX e dunque C = 1
    0x6C, 0xB0, // ASL $B0,X    10 - N C - shift verso SX e dunque C = 1
    0x48, 0xC1, // LDA $C1      5
    0x10,       // OUT          3 OUT 192 $C0
    0x00,       // HLT
// ROR $5E
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x28, 0xFF, // LDA #$FF     4 - N    -
    0x10,       // OUT          3 OUT 255 $FF
    0x00,       // HLT
    0xFC,       // ROL A        4 - N  C - shift verso SX e dunque C = 1; 0 nell'LSB
    0x10,       // OUT          3 OUT 254 $FE
    0x00,       // HLT
    0x23,       // CLC          3 -      - pulisce C se presente  
    0x28, 0x7E, // LDA #$7E     4 -      -
    0x10,       // OUT          3 OUT 126 $7E
    0x00,       // HLT
    0xFC,       // ROL A        4 - N    - shift verso SX e dunque C = 0; 0 nell'LSB
    0x10,       // OUT          3 OUT 252 $FC
    0x00,       // HLT
    0xFD,       // ROR A        4 -      - shift verso SX e dunque C = 0; 0 nell'LSB
    0x10,       // OUT          3 OUT 126 $7E
    0x00,       // HLT
};

/*
    0x01, 0x0B, // JMP ($0B)
    0x07        // locazione che contiene l'indirizzo per l'indirect jump
    ,
*/

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("+++++                BEAM Bootloader                 +++++");
  Serial.println("+++++             Caricamento programma              +++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  // waitForKeyPress("Inizio programma, funzione setup.");
}

void loop()
{
  
  // set_for_programming();
  // writeProgram(CounterUpDown, sizeof(CounterUpDown));
  // writeKitt();
  // post_programming();
  // delay(120000);

  set_for_programming();
  writeProgram(TestProgram1, sizeof(TestProgram1));
  writeKitt();
  post_programming();
  delay(120000);

  set_for_programming();
  writeProgram(Fibonacci, sizeof(Fibonacci));
  writeKitt();
  post_programming();
  delay(120000);

  set_for_programming();
  writeProgram(DownCounter, sizeof(DownCounter));
  writeKitt();
  post_programming();
  delay(120000);

  // set_for_programming();
  // writeProgram(PROGRAM_SHIFT, sizeof(PROGRAM_SHIFT));
  // writeKitt();
  // post_programming();
  // delay(60000);

  // set_for_programming();
  // writeProgram(PROGRAM_STACK, sizeof(PROGRAM_STACK));
  // writeKitt();
  // post_programming();
  // delay(1000000);

  // set_for_programming();
  // writeProgram(PROGRAM_4, sizeof(PROGRAM_4));
  // writeKitt();
  // post_programming();
  // delay(1000000);

  // set_for_programming();
  // writeProgram(PROGRAM_plp, sizeof(PROGRAM_plp));
  // writeKitt();
  // post_programming();
  // delay(1000000);
  
  // set_for_programming();
  // writeProgram(PROGRAM_20, sizeof(PROGRAM_20));
  // writeKitt();
  // post_programming();
  // delay(1000000);
  
  set_for_programming();
  writeProgram(PROGRAM_STACK, sizeof(PROGRAM_STACK));
  writeKitt();
  post_programming();
  delay(40000);

  // for (int i = 1; i <= 2; i++)
  // {
  //   // blink();
  // delay(60000);
  // }

}

void waitForKeyPress(String stringa)
{
  Serial.print(" ==> ");
  Serial.println(stringa);
  while (!Serial.available())
  {
  }
  char input = Serial.read(); // must read IO buffer to empty it
}

void reset()
{
  digitalWrite(RESET, LOW);
  delay(50); // millisecondi
  digitalWrite(RESET, HIGH);
}

// ************************************************************
// *********************** WRITE PROGRAM **********************
// ************************************************************
//    Write a program to memory.
void writeProgram(byte program[], size_t size)
{
  // digitalWrite(SHIFT_ENABLE, LOW); ////////// mi serve??????????
  for (byte address = 0; address < size; address += 1)
  {
    if ((address) % 8 == 0)
    {
      Serial.print("\nAddress: 0x");
      Serial.print(address, HEX);
      Serial.print(".");
      // Serial.println("");
    }
    else
    {
      Serial.print(".");
    }
    // Serial.println("**********");
    // waitForKeyPress("Scrittura indirizzo sul bus; premi un tasto per continuare: ");
    setAddress(address);
    delay(10);
    // waitForKeyPress("Scrittura dato sul bus; premi un tasto per continuare: ");
    // Serial.print(" - Value: 0x");
    // Serial.println(PROGRAM[address], HEX);
    writeRAM(program[address]);
    delay(10);
  }
  Serial.println("\nDone!");
}

// ************************************************************
// ********************** SET ADDRESS, WM *********************
// ************************************************************
// shiftOut sends data to SR via (SHIFT_DATA) + (SHIFT_CLK)
void setAddress(byte address)
{
  // attiva i '595
  digitalWrite(SHIFT_ENABLE_1, LOW);
  digitalWrite(SHIFT_ENABLE_2, LOW);
  // carica sui '595 indirizzo e segnale WM
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address); // indirizzo
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, 0x70); // configura i 138 per scrittura MAR (WM)
  // aggiorna l'output dei '595 con quanto caricato sopra
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  // impulso di clock
  digitalWrite(LDR_CLK, LOW);
  digitalWrite(LDR_CLK, HIGH);
  delay(DELAY); // 1 ms
  digitalWrite(LDR_CLK, LOW);
  digitalWrite(SHIFT_ENABLE_1, HIGH);
  digitalWrite(SHIFT_ENABLE_2, HIGH);
}


// ************************************************************
// ********************** SET REGISTER(s) *********************
// ************************************************************
void setRegister(byte registers)
{
  digitalWrite(SHIFT_ENABLE_1, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, registers); // set register(s)
  // outputs 595 data
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  // pulse clock
  digitalWrite(LDR_CLK, LOW);
  digitalWrite(LDR_CLK, HIGH);
  delay(DELAY); // 1 ms
  digitalWrite(LDR_CLK, LOW);
  // digitalWrite(SHIFT_ENABLE_1, HIGH); // must keep driving 138s, there's no latching register 
}

// ************************************************************
// *********************** WRITE RAM, WR **********************
// ************************************************************
// prima metto il dato in D2 (SHIFT_DATA) e poi pulso D3 (SHIFT_CLK) per mandarlo
void writeRAM(byte data)
{
  digitalWrite(SHIFT_ENABLE_1, LOW);
  digitalWrite(SHIFT_ENABLE_2, LOW);
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, data); // set data on bus
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, 0x60); // set RAM
  // sblocco i 595 per mettere in uscita quanto caricato fino a ora
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  // pulso il Clock
  digitalWrite(LDR_CLK, LOW);
  digitalWrite(LDR_CLK, HIGH);
  delay(DELAY); // millisecondi
  digitalWrite(LDR_CLK, LOW);
  // digitalWrite(SHIFT_ENABLE_1, HIGH);
  // digitalWrite(SHIFT_ENABLE_2, HIGH);
}

/*
   Set Arduino pins for computer Program Mode
*/
void set_for_programming()
{
  // waitForKeyPress("Set for programming.");
  // Disable 595 Shift Registers
  digitalWrite(SHIFT_ENABLE_1, HIGH);
  digitalWrite(SHIFT_ENABLE_2, HIGH);
  digitalWrite(START_STOP_CLOCK, HIGH);
  // Arduino-generated clock
  digitalWrite(LDR_CLK, LOW);
  digitalWrite(RESET, HIGH);
  // Loader Active: disable Clock Module output, disable ROM0 & ROM1
  digitalWrite(LDR_Active, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SHIFT_ENABLE_1, OUTPUT);
  pinMode(SHIFT_ENABLE_2, OUTPUT);
  pinMode(START_STOP_CLOCK, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(LDR_Active, OUTPUT);
  pinMode(LDR_CLK, OUTPUT);
  // Enable 595 Shift Registers
  // digitalWrite(SHIFT_ENABLE, LOW); ////////// mi serve??????????
  digitalWrite(RESET, LOW); // attivo reset per la programmazione così RC non si incrementa
  // Enable 165 PISO Register
  digitalWrite(PISO_CLK, LOW);
  digitalWrite(PISO_LOAD, HIGH);
  pinMode(PISO_CLK, OUTPUT);
  pinMode(PISO_LOAD, OUTPUT);
  pinMode(PISO_READ, INPUT);
}

/*
   Set Arduino pins for computer Run Mode
*/
void post_programming()
{
  // porto a Input tutti i PIN così passano in HI-Z
  pinMode(SHIFT_DATA, INPUT);
  pinMode(SHIFT_CLK, INPUT);
  pinMode(SHIFT_LATCH, INPUT);
  pinMode(LDR_CLK, INPUT);
  // Enable ROMs
  digitalWrite(LDR_Active, LOW);
  // Disabilito i 595
  digitalWrite(SHIFT_ENABLE_1, HIGH);
  digitalWrite(SHIFT_ENABLE_2, HIGH);
  // Disattivo reset
  delay(100);
  digitalWrite(RESET, HIGH);
  // Attivo Clock
  digitalWrite(START_STOP_CLOCK, LOW);
  pinMode(START_STOP_CLOCK, INPUT);
}

void blink()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(400);
  digitalWrite(LED_BUILTIN, LOW);
  delay(800);
}

// ************************************************************
// ***************** KITT supercar LEDs effect ****************
// ************************************************************

int KITT[sequence][LEDs] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 1},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
    {1, 1, 1, 1, 1, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

void writeKitt()
{
  uint8_t tempFF = 0; // $FF used to play the LEDs sequence
  setAddress(0xFF);
  setRegister(0x06);
  digitalWrite(PISO_LOAD, LOW);   // load bus content into PISO register
  delay(5);
  digitalWrite(PISO_LOAD, HIGH);
  for (int i = 7; i >= 0; i -= 1) // read $FF value and save into tempFF
  {
    tempFF += digitalRead(PISO_READ) * ((int)(pow(2, i) + 0.5));
    digitalWrite(PISO_CLK, HIGH);
    delay(5);
    digitalWrite(PISO_CLK, LOW);
  }
    digitalWrite(SHIFT_ENABLE_1, HIGH);
    digitalWrite(SHIFT_ENABLE_2, HIGH);
  for (int i = 0; i < 3; i++)     // three times
  {
    for (int j = 0; j < sequence; j += 1) // step thru the sequence
    {
      uint8_t pinsValue = 0;
      for (int k = 0; k < LEDs; k += 1) // calculate value for the current step
        pinsValue += KITT[j][k] * ((int)(pow(2, 7 - k) + 0.5));
      writeRAM(pinsValue);  // push value in $FF
      delay(40);
    }
  }
  writeRAM(tempFF); // Restore content of $FF
}
