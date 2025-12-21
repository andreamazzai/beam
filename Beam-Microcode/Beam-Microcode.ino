#include "Arduino.h"
// per la connessione si usa ttyUSB0

// Questo è lo sketch da utilizzare (04 settembre 2024).
// Modificato N in NI per coerenza con gli schemi

// Definizione dei pin immaginando di avere l'USB di Arduino a sinistra
// Pinout: pagina 10 https://docs.arduino.cc/resources/datasheets/A000005-datasheet.pdf
#define SHIFT_DATA  2 // riferito a GPIO,   dunque pin logico     D2 = fisico  5
#define SHIFT_CLK   3 // riferito a GPIO,   dunque pin logico     D3 = fisico  6
#define SHIFT_LATCH 4 // riferito a GPIO,   dunque pin logico     D4 = fisico  7
#define EEPROM_D0   5 //  D0 per la EEPROM, dunque pin logico     D5 = fisico  8
#define EEPROM_D7  12 //  D7 per la EEPROM, dunque pin logico    D12 = fisico 15
#define WE         14 // /WE per la EEPROM, dunque pin logico A0/D14 = fisico 19
#define OE         15 // /OE per la EEPROM, dunque pin logico A1/D15 = fisico 20
#define CE         16 // /CE per la EEPROM, dunque pin logico A2/D16 = fisico 21

// void initMicroCodeBlock(int block);
void setAddress(uint16_t address, bool outputEnable);
void setDataBusMode(byte mode);
void enableOutput();    // OE
void disableOutput();   // OE
void enableChip();      // CE
void disableChip();     // CE
void strobeWE();        // WE
void writeOpcode(uint8_t opcode);
void writeEEPROM(uint16_t address, byte data);
byte readEEPROM(uint16_t address);
byte readDataBus();
void eeprom_erase(byte value);
void eeprom_program();
void eepromSmallWrite(byte value);
void buildInstruction(uint8_t opcode);
void printStep(uint8_t step, uint16_t address, uint8_t rom);
void printInstruction(uint8_t rom, uint16_t opcode, uint8_t step);
void printContents(uint16_t start, uint16_t lenght);
void printOpcodeContents(uint8_t opcode);
bool waitForWriteCycleEnd(byte lastValue, byte* b1Ptr);
void unlock();
void lock();
uint16_t calcCRC16_pre();
uint16_t calcCRC16_post();
uint16_t calculate_crc(uint8_t data, uint16_t crc, uint16_t polynomial);
uint16_t crc = 0xFFFF, crc_pre, crc_post;

// ************************************************************
// **********************   MAIN CODE  ************************
// ************************************************************
void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  pinMode(SHIFT_DATA, OUTPUT);  // Mette in output i pin di Arduino che controllano i '595
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  digitalWrite(WE, HIGH);       // WE = HI e così facendo Arduino mette automaticamente un pull-up...
  digitalWrite(OE, HIGH);
  pinMode(WE, OUTPUT);          // ... così, quando attivo il pin, questo è già attivo HI
  pinMode(OE, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); // D13, per poter lampeggiare alla fine della programmazione
  Serial.println("\n\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("+++++          Microcode EEPROM programmer           +++++");
  Serial.println("+++++            for BEAM 8-bit Computer             +++++");
  Serial.println("+++++++                                            +++++++");
  Serial.println("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
  Serial.println("\n+++++++++++++++++++++++++++++");
  crc_pre = calcCRC16_pre();
  Serial.print("EEPROM CRC-PRE:  $");
  Serial.println(crc_pre, HEX);
  unlock();
  eeprom_erase(0x00);
  eeprom_program();
  lock();
  // eepromSmallWrite(0x77);
  crc_post = calcCRC16_post();
  Serial.println("\n+++++++++++++++++++++++++++++");
  if (crc_pre == crc_post)
  {
    Serial.println("Pre- and post-programming CRC values match. Good job!");
  }
  else
  {
    Serial.println("Careful! Pre- and post-programming CRC values *** do not *** match!");
    Serial.print("EEPROM CRC-POST: $");
    Serial.println(crc_post, HEX);
  }
  // printContents(0x0000, 64);
  // printContents(0x1000, 64);
  // printContents(0x2000, 64);
  // printContents(0x3000, 64);
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.println("Done!");
  Serial.print("Elapsed time: ");
  uint32_t elapsedTime;
  elapsedTime = millis() / 1000;
  Serial.print(elapsedTime);
  Serial.println(" seconds.");
}

// ************************************************************
// ****************** DEFINIZIONE PIN ROM *********************
// ************************************************************
//              ROM0    ROM1    ROM2    ROM3
//            +------++------++------++------+
// ROM 0      WWWWRRRR                                               WA  WB  WS  WR  WM  WD  WX  WY WPC  WO    RA  RB  RS  RH  RR  RL  RD  RX  RY RPC  RF
#define W3  0b10000000000000000000000000000000L // Write bit 3        0   1   0   0   1   0   1   0   1   0
#define W2  0b01000000000000000000000000000000L // Write bit 2        0   0   0   1   1   0   0   1   0   1
#define W1  0b00100000000000000000000000000000L // Write bit 1        0   0   1   1   1   0   0   0   1   1
#define W0  0b00010000000000000000000000000000L // Write bit 0        1   1   1   1   1   0   0   0   0   0    
#define R3  0b00001000000000000000000000000000L // Read bit 3                                                   0   0   0   0   0   0   1   1   1   1   1
#define R2  0b00000100000000000000000000000000L // Read bit 2                                                   0   0   1   1   1   1   0   0   0   1   1
#define R1  0b00000010000000000000000000000000L // Read bit 1                                                   0   1   0   0   1   1   0   0   1   0   1
#define R0  0b00000001000000000000000000000000L // Read bit 0                                                   1   0   0   1   0   1   0   1   0   1   1
// ROM 1              76543210
#define U1  0b00000000100000000000000000000000L // Unused
#define FS  0b00000000010000000000000000000000L // Set VZC Flags from computation in Flags Module (Read from bus if not asserted)
#define FV  0b00000000001000000000000000000000L // Set Overflow Flag
#define FZ  0b00000000000100000000000000000000L // Set Zero Flag
#define FN  0b00000000000010000000000000000000L // Set Negative Flag
#define FC  0b00000000000001000000000000000000L // Set Carry Flag
#define HR  0b00000000000000100000000000000000L // H Register Shift Right
#define HL  0b00000000000000010000000000000000L // H Register Shift Left
// ROM 2                      76543210
#define C1  0b00000000000000001000000000000000L // Carry source select ALU-Cout inverted
#define C0  0b00000000000000000100000000000000L // Carry source select 0
#define U2  0b00000000000000000010000000000000L // Unused
#define SUD 0b00000000000000000001000000000000L // Stack Pointer Up (Down if not asserted)
#define SE  0b00000000000000000000100000000000L // Stack Enable
#define U3  0b00000000000000000000010000000000L // Unused
#define CS  0b00000000000000000000001000000000L // Carry Flag force Set
#define CC  0b00000000000000000000000100000000L // Carry Flag force Clear
// ROM 3                              76543210
#define WIR 0b00000000000000000000000010000000L // Write Instruction Register
#define LF  0b00000000000000000000000001000000L // ALU Force
#define NI  0b00000000000000000000000000100000L // Next Instruction
#define HLT 0b00000000000000000000000000010000L // Halt
#define DXY 0b00000000000000000000000000001000L // DXY = D+X (D+Y if not asserted)
#define DZ  0b00000000000000000000000000000100L // DXY = D
#define JE  0b00000000000000000000000000000010L // Jump Enable per Relative Branch
#define PCI 0b00000000000000000000000000000001L // Program Counter Increment


// ************************************************************
// ************** DEFINIZIONE DEI SEGNALI ROM 0 ***************
// ************************************************************
#define WA  00|00|00|W0 // 0001 Write A
#define WB  00|00|W1|00 // 0002 Write B
#define WS  00|W2|00|00 // 0004 Write Stack Pointer 
#define WR  00|W2|W1|00 // 0006 Write RAM
#define WM  00|W2|W1|W0 // 0007 Write MAR
#define WD  W3|00|00|00 // 0008 Write D
#define WX  W3|00|00|W0 // 0009 Write X
#define WY  W3|00|W1|00 // 000A Write Y
#define WPC W3|W2|00|W0 // 000D Write PC
#define WO  W3|W2|W1|00 // 000E Write Output

#define RA  00|00|00|R0 // 0001 Read A
#define RB  00|00|R1|00 // 0002 Read B
#define RS  00|R2|00|00 // 0004 Read Stack Pointer
#define RH  00|R2|00|R0 // 0005 Read H
#define RR  00|R2|R1|00 // 0006 Read RAM
#define RL  00|R2|R1|R0 // 0007 Read ALU
#define RD  R3|00|00|00 // 0008 Read D
#define RX  R3|00|00|R0 // 0009 Read X
#define RY  R3|00|R1|00 // 000A Read Y
#define RPC R3|R2|00|R0 // 000D Read PC
#define RF  R3|R2|R1|R0 // 000F Read Flag

// ************************************************************
// ****************** DEFINIZIONE SHORTCUT ********************
// ************************************************************
#define F1  RPC|WM            // Fetch / 1st step
#define F2  RR|WIR|PCI        // Fetch / 2nd step
#define WH  HR|HL             // Write H
#define WAH WA|WH             // Write A & H
#define RDX RD|DXY            // Read D + X
#define RDY RD                // Read D + Y
#define RDZ RD|DZ             // Read D + 0
#define FNZ FS|FN|FZ          // Set N, Z flags
#define FNZC FS|FN|FZ|FC      // Set N, Z, C flags
#define FNVZC FS|FN|FV|FZ|FC  // Set N, V, Z, C flags
#define C0C1 C0|C1            // Read Carry from Set N, V, Z, C flags

#define CI    C0            // 1 - Carry source ALU inverted
#define CR    C1            // 2 - Carry source Shift Right (H-Q7)
#define CL    C1|C0         // 3 - Carry source Shift Left (H-Q0)

const uint8_t NUM_STEPS = 16;
const uint8_t NUM_TEMPLATE_STEPS = 8;

typedef uint32_t microcode_t;
typedef microcode_t template_t[256][NUM_TEMPLATE_STEPS];
microcode_t code[NUM_STEPS]; // contiene un opcode composto da 4 byte (uno per ognuna delle 4 ROM) e 16 step = 64 byte

const template_t template0 PROGMEM = {
//<2>          <3>           <4>           <5>           <6>           <7>           <8>           <9>
// Sezione 0 -- Accumulatore, Implicito, Relativo, Immediato -- A, IP, REL, IM
{ HLT,          NI,           0,            0,            0,            0,            0,            0 }, // 00 IP_HLT
{ RPC|WM,       RR|WM,        RR|WPC|NI,    0,            0,            0,            0,            0 }, // 01 IN_JMP
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 02
{ CC|FC|RL|NI,  0,            0,            0,            0,            0,            0,            0 }, // 03 IP_SEC *  -- CC mette HI in ALU-Cin dunque operazione è LLHH = -1 = signed 11111111
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 04
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 05
{ RPC|WM,       RR|WB,        RX|WH,        CS|C0|FNZC|RL,    RA|WH|PCI|NI,   0,      0,            0 }, // 06 IM_CPX * - N ZC
{ RPC|WM,       RR|WB,        RY|WH,        CS|C0|FNZC|LF|RL, RA|WH|PCI|NI,   0,      0,            0 }, // 07 IM_CPY * - N ZC
{ SE|SUD,       RS|WM,        FNZ|RR|WAH,   NI,           0,            0,            0,            0 }, // 08 IM_PLA   - N Z
{ RS|WM,        RA|WR,        SE|NI,        0,            0,            0,            0,            0 }, // 09 IM_PHA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 0a
{ FNZ|RA|WX|NI, 0,            0,            0,            0,            0,            0,            0 }, // 0b IP_TAX   - N Z
{ FNZ|RA|WY|NI, 0,            0,            0,            0,            0,            0,            0 }, // 0c IP_TAY   - N Z
{ FNZ|RS|WX|NI, 0,            0,            0,            0,            0,            0,            0 }, // 0d IP_TSX   - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 0e
{ NI,           0,            0,            0,            0,            0,            0,            0 }, // 0f IP_NOP
{ RA|WO|NI,     0,            0,            0,            0,            0,            0,            0 }, // 10 IP_OUT
{ SE|SUD,       RS|WM,        RR|WPC,       NI,           0,            0,            0,            0 }, // 11 IP_RTS
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // 12 RE_BCC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 13 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 14
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 15
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 16
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 17
{ SE|SUD,       RS|WM,        FNVZC|RR,     NI,           0,            0,            0,            0 }, // 18 IP_PLP - NVZC da Stack verso bus verso Flags
{ RS|WM,        RF|WR,        SE|NI,        0,            0,            0,            0,            0 }, // 19 IP_PHP -      da Flags verso bus verso Stack
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1a
{ FNZ|RX|WAH|NI, 0,           0,            0,            0,            0,            0,            0 }, // 1b IP_TXA   - N Z
{ FNZ|RY|WAH|NI, 0,           0,            0,            0,            0,            0,            0 }, // 1c IP_TYA   - NZ
{ RX|WS|NI,     0,            0,            0,            0,            0,            0,            0 }, // 1d IP_TXS
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 1f

// Sezione 1 -- Immediato IM
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 20
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 21
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 22
{ CS|FC|RL|NI,  0,            0,            0,            0,            0,            0,            0 }, // 23 IP_CLC * - CS mette LO in ALU-Cin dunque operazione è LLHH = 0 = signed 00000000
{ RPC|WM,       RR|FNZ|WX|PCI|NI, 0,        0,            0,            0,            0,            0 }, // 24 IM_LDX   - N Z
{ RPC|WM,       RR|FNZ|WY|PCI|NI, 0,        0,            0,            0,            0,            0 }, // 25 IM_LDY   - N Z
{ RPC|WM,       RR|WB,        CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,        0,            0,            0 }, // 26 IM_SBC * - NVZC
{ RPC|WM,       RR|WB,        CS|C0|FNZC|LF|RL|PCI|NI,    0,  0,        0,            0,            0 }, // 27 IM_CMP * - N ZC
{ RPC|WM,       RR|FNZ|WAH|PCI|NI, 0,       0,            0,            0,            0,            0 }, // 28 IM_LDA   - N Z
{ RPC|WM,       RR|WB,        CC|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,        0,            0,            0 }, // 29 IM_ADC * - NVZC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2a
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2d
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 2f
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 30
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 31
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // 32 RE_BCS
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 33 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 34
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 35
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI, 0,       0,            0,            0,            0 }, // 36 IM_EOR * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 37
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 38
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 39
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3a
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI, 0,       0,            0,            0,            0 }, // 3b IM_AND * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3d
{ RPC|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI, 0,       0,            0,            0,            0 }, // 3e IM_ORA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 3f

// Sezione 2 - Assoluto -- AB
{ RPC|WM,       RR|WH,        CS|FNZ|RL|WR, RA|WH|PCI|NI, 0,            0,            0,            0 }, // 40 AB_INC * - N Z
{ RPC|WM,       RR|WB|PCI,    RS|WM,        RPC|WR,       SE,           RB|WPC|NI,    0,            0 }, // 41 AB_JSR (SU/D = LOW => counts downward)
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 42
{ CS|FV|RL|NI,  0,            0,            0,            0,            0,            0,            0 }, // 43 IP_CLV *
{ RPC|WM,       RR|WM,        RR|FNZ|WX|PCI|NI, 0,        0,            0,            0,            0 }, // 44 AB_LDX   - N Z
{ RPC|WM,       RR|WM,        RR|FNZ|WY|PCI|NI, 0,        0,            0,            0,            0 }, // 45 AB_LDY   - N Z
{ RPC|WM,       RR|WM,        RR|WB,        CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,        0,            0 }, // 46 AB_SBC * - NVZC // devo settare Cin = HI cioè 0 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WM,        RR|WB,        CS|C0|FNZC|LF|RL|PCI|NI,    0,  0,        0,            0 }, // 47 AB_CMP * - N ZC
{ RPC|WM,       RR|WM,        RR|FNZ|WAH|PCI|NI, 0,       0,            0,            0,            0 }, // 48 AB_LDA
{ RPC|WM,       RR|WM,        RR|WB,        CC|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,        0,            0 }, // 49 AB_ADC *  // devo settare Cin = LO cioè 1 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WM,        RA|WR|PCI|NI, 0,            0,            0,            0,            0 }, // 4a AB_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4b
{ RPC|WM,       RR|WM,        RR|WH,        C1|FS|FC,     HL|CC,        FNZ|RH|WR,    RA|WH|PCI|NI, 0 }, // 4c AB_ASL   - C <= xxxxxxxx <= 0 - non uso ALU
{ RPC|WM,       RR|WM,        RR|WH,        C0|C1|FS|FC,  HR|CC,        FNZ|RH|WR,    RA|WH|PCI|NI, 0 }, // 4d AB_LSR   - 0 => xxxxxxxx => C - non uso ALU
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 4e
{ RPC|WM,       RR|WH,        CC|FNZ|RL|WR, RA|WH|PCI|NI, 0,            0,            0,            0 }, // 4f AB_DEC *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 50
{ RPC|WM,       RR|WPC|NI,    0,            0,            0,            0,            0,            0 }, // 51 AB_JMP
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // 52 RE_BEQ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 53 
{ RPC|WM,       RR|WM,        RX|WR|PCI|NI, 0,            0,            0,            0,            0 }, // 54 AB_STX
{ RPC|WM,       RR|WM,        RY|WR|PCI|NI, 0,            0,            0,            0,            0 }, // 55 AB_STY
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0,            0 }, // 56 AB_EOR * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 57
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 58
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 59
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5a
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0,            0 }, // 5b AB_AND * - N Z
{ RPC|WM,       RR|WM,        RR|WH,        C1|FS|FC|HL,  FNZ|RH|WR,    RA|WH|PCI|NI, 0,            0 }, // 5c AB_ROL   - N ZC - C <= xxxxxxxx <= C - non uso ALU
{ RPC|WM,       RR|WM,        RR|WH,        C0|C1|FS|FC|HR,  FNZ|RH|WR, RA|WH|PCI|NI, 0,            0 }, // 5d AB_ROR   - N ZC - C >= xxxxxxxx >= C - non uso ALU
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0,            0 }, // 5e AB_ORA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 5f

// Sezione 3 - Indexed Addressing: Absolute,X
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,        CS|FNZ|RL|WR, RA|WH|PCI|NI, 0,            0 }, // 60 AX_INC * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 61
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 62
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 63
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 64
{ RPC|WM,       RR|WD,        RDX|WM,       RR|FNZ|WY|PCI|NI,   0,      0,            0,            0 }, // 65 AX_LDY   - N Z
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,            0 }, // 66 AX_SBC * - NVZC // devo settare Cin = HI cioè 0 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|NI,    0,  0,            0 }, // 67 AX_CMP * - N ZC
{ RPC|WM,       RR|WD,        RDX|WM,       FNZ|RR|WAH|PCI|NI,  0,      0,            0,            0 }, // 68 AX_LDA
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|NI, 0,   0,            0 }, // 69 AX_ADC * - NZVC // devo settare Cin = LO cioè 1 sul pin dell'ALU-Cin
{ RPC|WM,       RR|WD,        RDX|WM,       RA|WR|PCI|NI, 0,            0,            0,            0 }, // 6a AX_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6b
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,  C1|FS|FC,     HL|CC,  FNZ|RH|WR,     RA|WH|PCI|NI }, // 6c AX_ASL   - C <= xxxxxxxx <= 0 - non uso ALU
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,  C0|C1|FS|FC,  HR|CC,  FNZ|RH|WR,     RA|WH|PCI|NI }, // 6d AX_LSR   - 0 => xxxxxxxx => C - non uso ALU
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 6e
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,        CC|FNZ|RL|WR, RA|WH|PCI|NI, 0,            0 }, // 6f AX_DEC *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 70
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 71
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // 72 RE_BNE    // con i branch devo calcolare dove saltare
// salvo X in B, leggo PC e metto in MAR, leggo RAM e metto in D,
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 73
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 74
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 75
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 76 AX_EOR * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 77 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 78
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 79
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7a
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 7b AX_AND * - NZ
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,      C1|FS|FC|HL,    FNZ|RH|WR,    RA|WH|PCI|NI, 0 }, // 7c AX_ROL   - N ZC - C <= xxxxxxxx <= C - non uso ALU
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WH,      C0|C1|FS|FC|HR, FNZ|RH|WR,    RA|WH|PCI|NI, 0 }, // 7d AX_ROR   - N ZC - C >= xxxxxxxx >= C - non uso ALU
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 7e AX_ORA * - NZ
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 7f

// Sezione 4 - Indexed Addressing: Absolute,Y
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 80
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 81
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 82
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 83
{ RPC|WM,       RR|WD,        RDY|WM,       RR|FNZ|WX|PCI|NI,   0,      0,            0,            0 }, // 84 AY_LDX
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 85
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,            0 }, // 86 AY_SBC * - NVZC
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|NI,    0,  0,            0 }, // 87 AY_CMP * - N ZC
{ RPC|WM,       RR|WD,        RDY|WM,       FNZ|RR|WAH|PCI|NI,  0,      0,            0,            0 }, // 88 AY_LDA   - N Z
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|NI,  0,  0,            0 }, // 89 AY_ADC * - NVZC
{ RPC|WM,       RR|WD,        RDY|WM,       RA|WR|PCI|NI, 0,            0,            0,            0 }, // 8a AY_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8b
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8d
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8e
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 8f
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 90
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 91
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // 92 RE_BMI
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 93 
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 94
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 95
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 96 AY_EOR * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 97
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 98
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 99
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9a
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 9b AY_AND * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9c
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9d
{ RPC|WM,       RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0,            0 }, // 9e AY_ORA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // 9f

// Sezione 5 - Pre-Indexed Indirect, "(Zero-Page,X)"
{ RX|WH,        CS|FNZ|RL|WX, RA|WH|NI,     0,            0,            0,            0,            0 }, // a0 IP_INX * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // a5
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0 }, // a6 IX_SBC * - NVZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CS|C0|FNZC|LF|RL|PCI|NI,    0,  0 }, // a7 IX_CMP * - NZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        FNZ|RR|WAH|PCI|NI,  0,      0,            0 }, // a8 IX_LDA   - NZ
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|NI,  0,  0 }, // a9 IX_ADC * - NVZC
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RA|WR|PCI|NI, 0,            0,            0 }, // aa IX_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ab
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ac
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ad
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ae
{ RX|WH,        CC|FNZ|RL|WX, RA|WH|NI,     0,            0,            0,            0,            0 }, // af IP_DEX * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b1
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // b2 RE_BPL
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b3 
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RY|WR|PCI|NI, 0,            0,            0 }, // b4 IX_STY
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b5
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // b6 IX_EOR * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b7
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // b9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ba
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // bb IX_AND * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bd
{ RPC|WM,       RR|WD,        RDX|WM,       RR|WM,        RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // be IX_ORA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // bf

// Sezione 6 - Post-Indexed Indirect, "(Zero-Page),Y"
{ RY|WH,        CS|FNZ|RL|WY, RA|WH|NI,     0,            0,            0,            0,            0 }, // c0 IP_INY * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // c5
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNVZC|RL|WAH|PCI|NI,  0,  0 }, // c6 IY_SBC * - NVZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CS|C0|FNZC|LF|RL|PCI|NI,    0,  0 }, // c7 IY_CMP * - N ZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       FNZ|RR|WAH|PCI|NI,  0,      0,            0 }, // c8 IY_LDA   - N Z
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,    CC|C0|FNVZC|RL|WAH|PCI|NI,  0,  0 }, // c9 IY_ADC * - NVZC
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RA|WR|PCI|NI, 0,            0,            0 }, // ca IY_STA
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cb
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // cd
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ce
{ RY|WH,        CC|FNZ|RL|WY, RA|WH|NI,     0,            0,            0,            0,            0 }, // cf IP_DEY * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d1
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // d2 RE_BVC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d3 
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RX|WR|PCI|NI, 0,            0,            0 }, // d4 IY_STX
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d5
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // d6 IY_EOR * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d7
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // d9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // da
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // db IY_AND * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // dc
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // dd
{ RPC|WM,       RR|WM,        RR|WD,        RDY|WM,       RR|WB,        FNZ|RL|WAH|PCI|NI,  0,      0 }, // de IY_ORA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // df

// Sezione 7
{ CS|FNZ|RL|WAH|NI, 0,        0,            0,            0,            0,            0,            0 }, // e0 AA_INA *
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e1
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e2
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e5
{ RPC|WM,       RR|WM,        RR|WB,        RX|WH,        CS|C0|FNZC|RL,    RA|WH|PCI|NI,   0,      0 }, // e6 AB_CPX * - N ZC
{ RPC|WM,       RR|WM,        RR|WB,        RY|WH,        CS|C0|FNZC|LF|RL, RA|WH|PCI|NI,   0,      0 }, // e7 AB_CPY * - N ZC
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // e9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ea
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // eb
{ C1|FS|FC,     HL|CC,        FNZ|RH|WA|NI, 0,            0,            0,            0,            0 }, // ec AA_ASL - C <= xxxxxxxx <= 0 - non uso ALU
{ C0|C1|FS|FC,  HR|CC,        FNZ|RH|WA|NI, 0,            0,            0,            0,            0 }, // ed AA_LSR - 0 => xxxxxxxx => C - non uso ALU
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ee
{ CC|FNZ|RL|WAH|NI, 0,        0,            0,            0,            0,            0,            0 }, // ef AA_DEA * - N Z
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f0
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f1
{ RX|WB,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|JE,       RB|WX|NI,     0,            0 }, // f2 RE_BVS
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f3
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f4
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f5
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f6
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f7
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f8
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // f9
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // fa
{ RPC|WM,       RR|WM,        RR|WB,        FNZ|RL,       RB|WH,        FV|HR,        RA|WH|PCI|NI, 0 }, // fb AB_BIT * - NVZ
{ C1|FS|FC|HL,  FNZ|RH|WA|NI, 0,            0,            0,            0,            0,            0 }, // fc AA_ROL
{ C0|C1|FS|FC|HR,  FNZ|RH|WA|NI, 0,         0,            0,            0,            0,            0 }, // fd AA_ROR
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // fe
{ 0,            0,            0,            0,            0,            0,            0,            0 }, // ff
};

// Copia di un branch a 10 step
// { RX|WH,        RPC|WM,       RR|WX|PCI,    RPC|WD,       RDX|WB,       RH|WX,        RA|WH,    RB|JE }, // 32 RE_BCS

// ************************************************************
// *********************                  *********************
// ********************   EEPROM PROGRAM   ********************
// *********************                  *********************
// ************************************************************
void eeprom_program()
{
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.println("Programming EEPROM");
  for (uint16_t opcode = 0; opcode < 256; opcode++)
  {
    buildInstruction((uint8_t) opcode);
    // printOpcodeContents(opcode);
    char buf[80];
    if ((opcode) % 16 == 0)
    {
       Serial.print("Opcode: 0x");
    }
    sprintf(buf, "%02X ", opcode);
    Serial.print(buf);
    if ((opcode + 1) % 16 == 0)
    {
      Serial.println("");
    }
    writeOpcode((uint8_t) opcode);
  }
  Serial.println("Done!");
}

//
// ************************************************************
// ************** COPIA BLOCCO MICROCODE IN RAM ***************
// ************************************************************
void buildInstruction(uint8_t opcode)
{
  // Ogni istruzione = 16 step: i primi 2 uguali per tutti; 8 presi dal template; 6 che - per ora - non utilizzo, dunque a 0.
  code[0] = F1; // Fetch instruction from memory (RPC, WM - Read Program Counter, Write Memory Address Register)
  code[1] = F2; // Opcode into IR (sets ALU mode and S bits) (RR, WIR, PCI - Read RAM, Write Instruction Register, Program Counter Increment)
  // Copia parte del template dalla memoria Flash alla memoria RAM
  // Gli step presenti nel template sono 8 e ognuno di essi sono 4 byte, uno per ogni ROM
  // Ogni step è composto da 4 byte che vanno allo stesso indirizzo su ognuna delle EEPROM; è una vista delle 4 ROM "affiancate"
  // In ogni ROM 0x000-0x00F è 1° opcode, 0x010-0x01f è 2° opcode... 0xFF0-FFF è 256° opcode
  memcpy_P(code + 2, template0[opcode], NUM_TEMPLATE_STEPS * 4);
  code[10] = code[11] = code[12] = code[13] = code[14] = code[15] = 0;
}

// ************************************************************
// **************** PREPARA E SCRIVE OPCODE *******************
// ************************************************************
void writeOpcode(uint8_t opcode)
{
  // Sono 4 ROM, mappate $0-$0FFF, $1000-$1FFF, $2000-$2FFF, $3000-$3FFF
  // Ogni opcode è lungo 16 step = 16 byte per ogni ROM, dunque 256 * 16 = 4096 byte = $1000
  for (uint8_t rom = 0; rom < 4; rom++)
  {
    // Serial.print(" - ROM: ");
    // Serial.print(rom);
    // Serial.print(" - ");
    for (uint8_t step = 0; step < NUM_STEPS; step++) // ciclo fra i 16 step di ogni opcode e dunque li scrivo consecutivamente su ogni EEPROM
    {
      uint16_t address;
      address = 0x1000 * rom;
      address += opcode * NUM_STEPS;
      address += step;
      // printStep(step, address, rom);
      writeEEPROM(address, ((code[step]) >> (24 - 8 * rom)) & 0xFF); // code[step] prende tutti i 4 byte delle 4 ROM "affiancate" e poi con shift seleziono il byte relativo ad ogni ROM specifica, ad esempio 1a ROM bit 25-32, poi 17-24 etc
    }
    byte b1Value;
    bool status = waitForWriteCycleEnd(((code[15]) >> (24 - 8 * rom)) & 0xFF, &b1Value);
    if (status == false)
    {
      Serial.print("\n******** Error in Opcode 0x");
      Serial.print(opcode, HEX);
      Serial.print(" - Value sent: 0x");
      Serial.print(((code[15]) >> (24 - 8 * rom)) & 0xFF, HEX);
      Serial.print(" - Value read: 0x");
      Serial.println(b1Value, HEX);
      Serial.print("Opcode: 0x");
    }
  }
}

// ************************************************************
// ******************** WRITE TO EEPROM ***********************
// ************************************************************
void writeEEPROM(uint16_t address, byte data)
{
  // Setto indirizzo e disabilito output EEPROM; solo in seguito abilito output di Arduino, così non causo cortocircuiti.
  // La EEPROM non ha resistenze in uscita; se EEPROM ha output attivo e Arduino è in input, si genera un cortocircuito.
  setAddress(address, /*outputEnable */ false /* cioè porto a uno /OE*, cosi posso scrivere sulla EEPROM*/);
  setDataBusMode(OUTPUT);
  enableChip();
  // 5 LSB = D0-D4 data bus; shiftare dato a DX di 3 posizioni
  // 6 MSB PORTD = D0-D5 bus; shiftare dato a SX di 5 posizioni
  PORTB = (PORTB & 0xE0) | (data >> 3); // data = BBBB.Bxxx ==> shift DX 3 pos. degli MSB BBBBB, che carico in PORTB0-PORTB4 del Nano, cioè D3-D7 del data bus
  PORTD = (PORTD & 0x1F) | (data << 5); // data = xxxx.xBBB ==> shift SX 5 pos. degli LSB BBB, che carico in PORTD5-PORTD7 del Nano, cioè D0-D2 del data bus
  strobeWE();
  disableChip();
}

void writeEEPROM2(uint16_t address, byte data)
{
  // Setto indirizzo e disabilito output EEPROM; solo in seguito abilito output di Arduino, così non causo cortocircuiti.
  // La EEPROM non ha resistenze in uscita; se EEPROM ha output attivo e Arduino è in input, si genera un cortocircuito.
  setAddress(address, /*outputEnable */ false /* cioè porto a uno /OE*, cosi posso scrivere sulla EEPROM*/);
  delayMicroseconds(2);
  setDataBusMode(OUTPUT);
  delayMicroseconds(2);
  enableChip();
  delayMicroseconds(2);
  // 5 LSB = D0-D4 data bus; shiftare dato a DX di 3 posizioni
  // 6 MSB PORTD = D0-D5 bus; shiftare dato a SX di 5 posizioni
  PORTB = (PORTB & 0xE0) | (data >> 3); // data = BBBB.Bxxx ==> shift DX 3 pos. degli MSB BBBBB, che carico in PORTB0-PORTB4 del Nano, cioè D3-D7 del data bus
  PORTD = (PORTD & 0x1F) | (data << 5); // data = xxxx.xBBB ==> shift SX 5 pos. degli LSB BBB, che carico in PORTD5-PORTD7 del Nano, cioè D0-D2 del data bus
  strobeWE();
  disableChip();
}

const uint32_t mMaxWriteTime = 20;  // Max time (in ms) to wait for write cycle to complete

bool waitForWriteCycleEnd(byte lastValue, byte *b1Ptr)
{
  // Codice tratto da Tom Nisbet e fatta qualche mia modifica
  // Vedi DATA Polling e Toggle Bit pagina 3 https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf
  //
  // Verify programming complete by reading the last value back until it matches the
  // value written twice in a row.  The D7 bit will read the inverse of last written
  // data and the D6 bit will toggle on each read while in programming mode.
  //
  // Note that the max readcount is set to the device's maxReadTime (in uSecs)
  // divided by two because there are two 1 uSec delays in the loop (rimossi).  In reality,
  // the loop could run for longer because this does not account for the time needed
  // to run all of the loop code.  In actual practice, the loop will terminate much
  // earlier because it will detect the end of the write well before the max time.

  // * Nota che in https://github.com/TomNisbet/TommyPROM/issues/17 Tom dice che potrebbero verificarsi
  // errori in rilettura gestendo solo OE, dunque ha aggiunto CE, che "rispecchia meglio le waveforms
  // del datasheet", e in effetti "AC Read Waveforms" a pagina 6 del DS mostra che la Read completa è 
  // 1) attivare /CE e poi /OE
  // 2) disattivare /OE e /CE
  // mentre io faccio un po' diverso:
  // 1) attivare /OE e poi /CE
  // 2) disattivare /OE e /CE
  // e nel punto * non disabilito e riabilito CE come indicato da Tom... però mi funziona lo stesso.

  setDataBusMode(INPUT);
  uint32_t readCount;
  for (readCount = mMaxWriteTime * 1000 / 2; readCount > 0; readCount--)
  {
    enableOutput();
    enableChip();
    byte b1 = readDataBus();
    *b1Ptr = b1;
    disableOutput(); // *
    enableOutput();
    byte b2 = readDataBus();
    disableOutput();
    disableChip();
    if ((b1 == b2) && (b1 == lastValue))
    {
      return true;
    }
  }
  return false;

      /*
      // Segue il CODICE ORIGINALE di Tom; si vede che lui prima attiva il chip e poi l'output,
      // mentre io prima lo metto in output e poi lo attivo
      bool waitForWriteCycleEnd(byte lastValue) {
          setDataBusMode(INPUT);
          delayMicroseconds(1);
          for (int readCount = mMaxWriteTime * 1000 / 2; (readCount > 0); readCount--) {
              enableChip();
              enableOutput();
              delayMicroseconds(1);
              byte b1 = readDataBus();
              disableOutput();
              disableChip();
              enableChip();
              enableOutput();
              delayMicroseconds(1);
              byte b2 = readDataBus();
              disableOutput();
              disableChip();
              if ((b1 == b2) && (b1 == lastValue)) {
                  return true;
              }
          }

          return false;
      }
      */
}

// ************************************************************
// ******************** READ DATA FROM BUS ********************
// ************************************************************
// Read a byte from the data bus. The caller must set the bus to input_mode
// before calling this or no useful data will be returned.
byte readDataBus()
{
  return (PINB << 3) | (PIND >> 5);
  // legge PINB; serve PB0-PB4, 5x LSB di PORTB, che sono però MSB del data bus, dunque li traslo 3 posizioni a SX
  // legge PIND; serve PD5-PD7, 3x MSB di PORTD, che sono però LSB del data bus, dunque li traslo 5 posizioni a DX
}

// ************************************************************
// *********************** SET ADDRESS ************************
// ************************************************************
// prima metto il dato in D2 (SHIFT_DATA) e poi pulso D3 (SHIFT_CLK) per mandarlo
void setAddress(uint16_t address, bool outputEnable) // 2° parametro = outputEnable, se True setto /OE basso, se False /OE alto
{
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (uint8_t) (address >> 8));
  // Qui vedrò sullo scope il SER e il SRCLK
  // L'indirizzo arriva in due byte. Quanto sopra gestisce la parte alta dell'indirizzo (che
  // sono i bit da A8 a A10, e poi aggiunge il bit 15, che controlla /OE
  // se il pin /OE = 0, attivo l'output, dunque leggo dalla EEPROM
  // se il pin /OE = 1, disattivo l'output, dunque scrivo sulla EEPROM
  // poiché uso "(outputEnable ? 0x00L: 0x80)", sto dicendo che se nella routine passo un "outputEnable" true,
  // setto a zero /OE e dunque attivo la lettura; se passo un "outputEnable" false, il MSb è HI e dunque attivo la scrittura
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (uint8_t) address);
  // Quanto sopra gestisce la parte bassa dell'indirizzo (da A0 ad A7)
  // Quanto sotto sblocca il 595
  // Qui vedrò sullo scope il RCLK
  digitalWrite(OE, outputEnable ? LOW : HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

// ************************************************************
// ********************* EEPROM ERASE *************************
// ************************************************************
void eeprom_erase(byte value)
{
  // value = 0xff; // overwrite value that was set on call - used for debug purposes
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.print("Erasing EEPROM with value 0x");
  Serial.print(value, HEX);
  Serial.println("...");
  uint16_t salto = 64;

  for (uint16_t address = 0; address < 0x8000; address += salto)
  {
    for (uint16_t step = 0; step < salto; step++)
    {
      // printStep(step, address, rom);
      writeEEPROM(address + step, value);
    }
    byte b1Value;
    bool status = waitForWriteCycleEnd(value, &b1Value);
    if (status == false)
    {
      Serial.print("\n******** Error in address 0x");
      Serial.print(address, HEX);
      Serial.print(" - Value sent: 0x");
      Serial.print(value, HEX);
      Serial.print(" - Value read: 0x");
      Serial.println(b1Value, HEX);
      Serial.print("Opcode: 0x");
    }

    // ************************************************************
    // ******************** FUNZIONA NON TOCCARE ******************
    // ************************************************************

    /*   for (uint16_t opcode = 0; opcode < 1024; opcode++)
      {
        for (uint8_t step = 0; step < 16; step++)
        {
          uint16_t address;
          address = 0x0000;
          address += opcode * 16;
          address += step;
          // printStep(step, address, rom);
          writeEEPROM(address, value);
        }
        byte b1Value;
        bool status = waitForWriteCycleEnd(value, &b1Value);
        if (status == false)
        {
          Serial.print("\n******** Error in Opcode 0x");
          Serial.print(opcode, HEX);
          Serial.print(" - Value sent: 0x");
          Serial.print(value, HEX);
          Serial.print(" - Value read: 0x");
          Serial.println(b1Value, HEX);
          Serial.print("Opcode: 0x");
        } */
  }
  // printContents(0x0000, 512);
  Serial.println("Done!");
}

  // ************************************************************
  // ******************** READ FROM EEPROM **********************
  // ************************************************************
  byte readEEPROM(uint16_t address)
  {
    setAddress(address, true);
    setDataBusMode(INPUT);
    enableChip();
    delayMicroseconds(2);
    byte value = readDataBus();
    disableChip();
    return value;
  }

  // STAMPA RIGA CON ROM, OPCODE, VALORI (PER DEBUG)
  void printInstruction(uint8_t rom, uint16_t opcode, uint8_t step)
  {
    if (step == 0)
    {
      char buf[80];
      sprintf(buf, "%04x:  ", rom * 1000 + opcode * 16);
      Serial.print(buf);
    }
    if (step == 8)
    {
      char buf[2];
      sprintf(buf, " ");
      Serial.print(buf);
    }
    char buf[80];
    sprintf(buf, "%02x ", ((code[step]) >> (24 - 8 * rom)) & 0xFF);
    if (step == 15)
    {
      Serial.println(buf);
    }
    else
    {
      Serial.print(buf);
    }
  }

  // ************************************************************
  // *********************** PRINT STEP *************************
  // ************************************************************
  void printStep(uint8_t step, uint16_t address, uint8_t rom)
  {
    Serial.print("Step: ");
    Serial.print(step, HEX);
    Serial.print(" - Address: 0x");
    Serial.print(address, HEX);
    Serial.print(" - Value: ");
    Serial.println(((code[step]) >> (24 - 8 * rom)) & 0xFF, HEX);
  }

  // ************************************************************
  // ************* PRINT EEPROM CONTENTS (DEBUG) ****************
  // ************************************************************
  void printContents(uint16_t start, uint16_t lenght)
  {
    Serial.println("\n+++++++++++++++++++++++++++++");
    Serial.print("EEPROM Content $");
    Serial.print(start, HEX);
    Serial.print(" - $");
    Serial.println(start + lenght - 1, HEX);
    for (uint16_t baseAddress = start, end = start + lenght; baseAddress < end; baseAddress += 16)
    {
      byte data[16];
      for (uint16_t offset = 0; offset <= 15; offset += 1)
      {
        data[offset] = readEEPROM(baseAddress + offset);
      }
      char buf[80];
      sprintf(buf, "%04X:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
              baseAddress, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
              data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
      Serial.println(buf);
    }
  }

  // ************************************************************
  // ****************** PRINT OPCODES (DEBUG) *******************
  // ************************************************************
  void printOpcodeContents(uint8_t opcode)
  {
    // stampa il contenuto dell'array temporaneo creato in RAM per l'opcode
    // sono 4 ROM e 16 step
    Serial.println("_________");
    for (int step = 0; step < NUM_STEPS; step += 4)
    {
      // Serial.print(code[step], HEX);
      // Serial.print(":");
      // for (int shift = 24; shift >= 0; shift -= 8)
      // {
      Serial.print("Opcode 0x");
      Serial.print(opcode, HEX);
      Serial.print(" - Step: ");
      Serial.print(step, HEX);
      Serial.print("/");
      Serial.print(step + 3, HEX);
      Serial.print(":  ");
      Serial.print(((code[step]) >> 24) & 0xFF, HEX); // ROM 0
      Serial.print(":");
      Serial.print(((code[step]) >> 16) & 0xFF, HEX); // ROM 1
      Serial.print(":");
      Serial.print(((code[step]) >> 8) & 0xFF, HEX); // ROM 2
      Serial.print(":");
      Serial.print(((code[step]) >> 0) & 0xFF, HEX); // ROM 3
      Serial.print(" : ");
      //
      Serial.print(((code[step + 1]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 1]) >> 0) & 0xFF, HEX);
      Serial.print(" : ");
      //
      Serial.print(((code[step + 2]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 2]) >> 0) & 0xFF, HEX);
      //
      Serial.print(" : ");
      Serial.print(((code[step + 3]) >> 24) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 16) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 8) & 0xFF, HEX);
      Serial.print(":");
      Serial.print(((code[step + 3]) >> 0) & 0xFF, HEX);
      Serial.println("");
      // uint32_t data[4];
      // {
      //   data[col + 0] = (code[col + 0]);
      //   data[col + 1] = (code[col + 1]);
      //   data[col + 2] = (code[col + 2]);
      //   data[col + 3] = (code[col + 3]);
      // }
      // char buf[80];
      // sprintf(buf, "Opcode %02x:  %04lx %04lx %04lx %04lx",
      //         opcode, data[0], data[1], data[2], data[3]);
      // Serial.println(buf);
      // }
    }
  }



  // ********************************************
  // ********** TEST LETTURA DA ARRAY ***********
  // ********************************************
  // Questo non funziona, restituisce spazzatura
  /*     uint32_t val[16];
      for (int row = 0; row < 4; row += 1) {
        for (int col = 0; col < 16; col += 4) {
          val[col] = my_ram_microcode_template[row][col];
        }
        char buf[80];
        sprintf(buf, "%03x:  %02lx %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                row, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
                val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
        Serial.println(buf);
      } */
  /* void read_RAM()
  {
    // Qui voglio provare a leggere il contenuto della RAM copiata e stamparlo in sequenza, per vedere se quanto
    // ho copiato dalla Flash con initMicroCodeBlock sia o meno corretto
    uint32_t data;
    // per 256 elementi ognuno lungo 4 byte
    //
    for (int row = 0 + 16 * block; row < 4 + 16 * block; row += 1)
    {
      Serial.print(row, HEX);
      Serial.print(":  ");
      for (int col = 0; col < 16; col += 4)
      {
        data = my_ram_microcode_template[row][col];
        Serial.print(data, HEX);
        Serial.print(" / ");
        byte value1, value2, value3, value4;
        char buf[60];
        sprintf(buf, "%02lx:%02lx:%02lx:%02lx", (data & 0xFF000000) >> 24, (data & 0x00FF0000) >> 16, (data & 0x0000FF00) >> 8, (data & 0x000000FF));
        Serial.print(buf);
        value1 = data >> 24;
        value2 = (data & 0x00FF0000) >> 16;
        value3 = (data & 0x0000FF00) >> 8;
        value4 = (data & 0x000000FF);
        sprintf(buf, " - %02x:%02x:%02x:%02x", value1, value2, value3, value4);
        Serial.print(buf);
        Serial.print(" - Totale = ");
        Serial.println(value1 + value2 + value3 + value4);
        // sprintf(buf, "%03x:  %02x %02x %02x %02x", row, data >> 24, data & 0x00FF000 > 16, data & 0x0000FF000 > 8, data & 0x00000FF);
        // Serial.print("    ");
        //           for (int cnt = 0; cnt < 16; cnt += 1) {
        //           data = current_microcode_block[i][j];
        //           Serial.print(data >> 24, HEX);
        //           Serial.print(F(":"));
        //         }
      }
      Serial.println("");
    }
  } */

  /*
    // ********************************************
    // ************ EEPROM PROGRAM ****************
    // ********************************************
    void eeprom_program() {
    Serial.print("\nProgramming EEPROM ");
    // ogni block = 16 righe * 16 colonne * elemento uint32_t = 1024 Byte
    // ogni sezione sono 32 righe dunque due blocchi = 2K
    // 8 sezioni = 16 KB divisi in 4 parti, 0-FFF / 1000-1FFF / 2000-2FFF / 3000-3FFF
      for (int block = 0; block < 4; block += 1) {
        Serial.print("\nBlock = ");
        Serial.print(block);
        Serial.print(" - Starting element = ");
        Serial.print(block * 256);
        Serial.print(" - Ending element = ");
        Serial.print((block + 1) * 256);
        initMicroCodeBlock(block);
        for (int address = 256 * block; address < 256 * (block + 1); address += 1) {
          int byte_sel    = (address & 0b11000000000000) >> 12; // selezione della ROM
          int instruction = (address & 0b00111111110000) >> 4;
          int step        = (address & 0b00000000001111);
          if (address % 16 == 0) {
            Serial.print("\naddress / instruction = ");
            Serial.print(address, HEX);
            Serial.print(" / ");
            Serial.print(current_microcode_block[instruction][step] >> 24, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x00FF0000) >> 16, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x0000FF00) >>  8, HEX);
            Serial.print("|");
            Serial.print((current_microcode_block[instruction][step] & 0x000000FF),  HEX);
          }
          // switch (byte_sel) {
            // case 0:
              writeEEPROM(address, current_microcode_block[instruction][step] >> 24);
              // if (address % 64 == 0) {
              //   Serial.print(".");
              //   break;
              // }
            // case 1:
              writeEEPROM(address + 4096, current_microcode_block[instruction][step] >> 16);
              // if (address % 64 == 0) {
              //   Serial.print(":");
              //   break;
              // }
            // case 2:
              writeEEPROM(address + 4096 * 2, current_microcode_block[instruction][step] >> 8);
              //   if (address % 64 == 0) {
              //     Serial.print(",");
              //     break;
              // }
            // case 3:
              writeEEPROM(address + 4096 * 3, current_microcode_block[instruction][step]);
              // if (address % 64 == 0) {
              //   Serial.print(";");
              //   break;
              // }
          // if (address % 64 == 0) {
          //   Serial.print(".");
          // }
          // }
        }
      Serial.println("\nDone!");
      }
    }
   */

void unlock()
{
  delay(10);
  setDataBusMode(OUTPUT);
  enableChip();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0xAA >> 3); // data = BBBB.Bxxx ==> shift DX 3 pos. degli MSB BBBBB, che carico in PORTB0-PORTB4 del Nano, cioè D3-D7 del data bus
  PORTD = (PORTD & 0x1F) | (0xAA << 5); // data = xxxx.xDDD ==> shift SX 5 pos. degli LSB   DDD, che carico in PORTD5-PORTD7 del Nano, cioè D0-D2 del data bus
  strobeWE();
  setAddress(0x2AAA, false);
  PORTB = (PORTB & 0xE0) | (0x55 >> 3);
  PORTD = (PORTD & 0x1F) | (0x55 << 5);
  strobeWE();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0x80 >> 3);
  PORTD = (PORTD & 0x1F) | (0x80 << 5);
  strobeWE();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0xAA >> 3);
  PORTD = (PORTD & 0x1F) | (0xAA << 5);
  strobeWE();
  setAddress(0x2AAAA, false);
  PORTB = (PORTB & 0xE0) | (0x55 >> 3);
  PORTD = (PORTD & 0x1F) | (0x55 << 5);
  strobeWE();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0x20 >> 3);
  PORTD = (PORTD & 0x1F) | (0x20 << 5);
  strobeWE();
  disableChip();
  delay(10);
}

void lock()
{
  delay(10);
  setDataBusMode(OUTPUT);
  enableChip();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0xAA >> 3); // data = BBBB.Bxxx ==> shift DX 3 pos. degli MSB BBBBB, che carico in PORTB0-PORTB4 del Nano, cioè D3-D7 del data bus
  PORTD = (PORTD & 0x1F) | (0xAA << 5); // data = xxxx.xDDD ==> shift SX 5 pos. degli LSB   DDD, che carico in PORTD5-PORTD7 del Nano, cioè D0-D2 del data bus
  strobeWE();
  setAddress(0x2AAA, false);
  PORTB = (PORTB & 0xE0) | (0x55 >> 3);
  PORTD = (PORTD & 0x1F) | (0x55 << 5);
  strobeWE();
  setAddress(0x5555, false);
  PORTB = (PORTB & 0xE0) | (0xA0 >> 3);
  PORTD = (PORTD & 0x1F) | (0xA0 << 5);
  strobeWE();
  disableChip();
  delay(10);
}

// ************************************************************
// **************** EEPROM SMALL WRITE (DEBUG) ****************
// ************************************************************
void eepromSmallWrite(byte value)
{
  Serial.println("\n+++++++++++++++++++++++++++++");
  Serial.println("EEPROM Small Write");
  for (int address = 0; address <= 256; address += 1)
  {
    writeEEPROM(address, value);
    if (address % 16 == 0)
    {
      // Serial.print(".");
      Serial.print("Address ");
      Serial.println(address, HEX);
    }
  }
  Serial.println("Done!");
}

void strobeWE() // Setto low e poi high il bit che corrisponde a A0/D14
{
  PORTC = (PORTC & 0x3E);
  PORTC = (PORTC & 0x3E) | 0x01;
}

// Set the status of the device control pins
void enableOutput()   { PORTC = (PORTC & 0x3D);         }  // set OE LO (pin 1 PORTC / A1/D15) 
void disableOutput()  { PORTC = (PORTC & 0x3D) | 0x02;  }  // set OE HI (pin 1 PORTC / A1/D15)
void enableChip()     { PORTC = (PORTC & 0x3B);         }  // set CE LO (pin 2 PORTC / A2/D16)
void disableChip()    { PORTC = (PORTC & 0x3B) | 0x04;  }  // set CE HI (pin 2 PORTC / A2/D16)

void setDataBusMode(byte mode)
{
  if (mode == OUTPUT)
  { // Set data bus pins to OUTPUT (write to EEPROM)
  DDRB |= 0x1F; // set Nano D8-D12 to HI (output); these are PORTB 5x LSB bits (0001.1111), aka D3-D7 of Nano <==> EEPROM data bus
  DDRD |= 0xE0; // set Nano D5-D7  to HI (output); these are PORTD 3x MSB bits (1110.0000), aka D0-D2 of Nano <==> EEPROM data bus
  }
  else
  { // Set data bus pins to INPUT (read from EEPROM)
  DDRB &= 0xE0; // viceversa of above
  DDRD &= 0x1F; // viceversa of above
  }
}

// ************************************************************
// *********************                 **********************
// ******************   CRC16 CALCULATION   *******************
// *********************                 **********************
// ************************************************************

// CALCOLO CRC16 PRE-PROGRAMMAZIONE
/* Il calcolo del CRC prevede la ricezione dei dati in sequenza (da 0x0000 a 0x3FFF). Non posso calcolare il CRC
durante la programmazione della EEPROM, perché in quel momento genero un opcode completo e ne scrivo i 4 segmenti
da 16 byte "frazionandoli" sulla EEPROM corrispondente (1a, 2a, 3a, 4a).
Faccio dunque un ciclo: per ogni ROM genero le 256 istruzioni in sequenza; ricavo solo i 16 byte che mi interessano
in quel momento (1a EEPROM, 2a EEPROM etc) e li utilizzo per calcolare il checksum. */
uint16_t calcCRC16_pre(void)
{
  crc = 0xFFFF;
  uint16_t polynomial = 0x1021;
  for (uint8_t rom = 0; rom < 4; rom++)
  {
    for (uint16_t opcode = 0; opcode < 256; opcode++)
    {
      buildInstruction(opcode);
      for (uint8_t step = 0; step < NUM_STEPS; step++)
      {
        // printInstruction(rom, opcode, step); // solo per Debug
        crc = calculate_crc(((code[step]) >> (24 - 8 * rom)) & 0xFF, crc, polynomial);
      }
    }
  }
  return crc;
}

// CALCOLO CRC16 POST-PROGRAMMAZIONE
/* Nella lettura di una EEPROM precedentemente programmata leggo tutti i byte in sequenza, dunque è sufficiente
leggere il contenuto da 0x0000 a 0x3FFF per calcolare il checksum. */
uint16_t calcCRC16_post(void)
{
  setDataBusMode(INPUT);
  crc = 0xFFFF;
  uint16_t polynomial = 0x1021;
  byte data;
  for (uint16_t address = 0x0000; address <= 0x3FFF; address++)
  {
    data = readEEPROM(address);
    crc = calculate_crc(data, crc, polynomial);
  }
  return crc;
}

// CRC16 MATH
uint16_t calculate_crc(uint8_t data, uint16_t crc, uint16_t polynomial)
{
  crc ^= data;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (crc & 0x0001)
    {
      crc >>= 1;
      crc ^= polynomial;
    }
    else
    {
      crc >>= 1;
    }
  }
  return crc;
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100);
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(800);
}