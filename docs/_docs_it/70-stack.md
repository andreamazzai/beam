---
title: "Stack Pointer"
permalink: /docs/stack/
excerpt: "Stack Pointer del computer BEAM"
---
<small>[Implementazione del microcode dello Stack Pointer](#implementazione-del-microcode-dello-stack-pointer) - [Lo Stack Pointer dell’NQSAP / NQSAP-PCB](#lo-stack-pointer-dellnqsap--nqsap-pcb) - [Schema](#schema) - [Link utili](#link-utili)</small>

[![Stack Pointer del computer BEAM](../../assets/sp/70-beam-sp.png "Stack Pointer del computer BEAM"){:width="100%"}](../../assets/sp/70-beam-sp.png)

L'implementazione dello stack nel 6502 prevede l'utilizzo di un'area di memoria dedicata alla memorizzazione e al ripristino di informazioni secondo una logica LIFO (Last-In, First-Out, dove l'ultimo elemento inserito è il primo a essere letto), gestita da un puntatore a 8 bit (Stack Pointer, SP) che tiene traccia dell'indirizzo della prossima locazione disponibile.

Due usi comuni dello stack sono il salvataggio dello stato attuale dei Flag prima di eseguire una routine che li modifica, così da poterli ripristinare al termine della routine, e la memorizzazione dell'indirizzo di ritorno di una subroutine invocata dall'istruzione JSR.

Il bus degli indirizzi del 6502 ha un'ampiezza di 16 bit e può indirizzare 2^16 = 64K di memoria. Lo stack occupa la seconda pagina di memoria indirizzabile dalla CPU, che corrisponde agli indirizzi compresi tra 0x0100 e 0x01FF. Prima dell'utilizzo, lo Stack Pointer viene solitamente inizializzato a 0xFF, puntando così alla locazione 0x1FF. Quando si effettua un'operazione di scrittura nello stack ("Push"), il valore viene prima salvato nella posizione indicata dal puntatore, dopodiché lo SP viene decrementato per puntare alla successiva locazione libera. Al contrario, durante un'operazione di lettura dallo stack ("Pull"), il valore viene prima letto dalla posizione indicata dal puntatore, che viene poi incrementato per indirizzare la prossima posizione disponibile.

Lo Stack Pointer del 6502 è un registro, denominato S, la cui implementazione nel BEAM consta di due 4-bit Synchronous Binary Up-Down Counter <a href="https://www.ti.com/lit/ds/symlink/sn74ls157.pdf" target="_blank">74LS169</a> in grado di indirizzare i 256 byte del computer. In definitiva, si tratta di un normale registro che, oltre a poter memorizzare un valore specifico, ha la peculiarità di poter contare sia verso l'alto, sia verso il basso.

Poiché il BEAM dispone di soli 256 byte di memoria e lo SP è in grado di indirizzarli tutti, l'area destinata allo stack dovrà essere necessariamente limitata dal programmatore, ad esempio usando i 16 byte compresi tra gli indirizzi 0xF0 e 0xFF.

Lo Stack Pointer punta sempre alla successiva locazione disponibile nello stack, dunque:

- un'operazione di scrittura sullo stack scrive il valore desiderato nella locazione di memoria indicata dallo Stack Pointer e poi lo decrementa (post-decrement);
- un'operazione di lettura dallo stack dapprima incrementa lo SP (pre-increment) e poi restituisce il contenuto della locazione di memoria da esso indicata.

In altre parole, poiché lo stack "cresce" verso il basso, lo Stack Pointer viene decrementato dopo ogni inserimento di un byte nello stack e incrementato prima di ogni prelievo di un byte.

Poiché i '169 utilizzati nel BEAM non dispongono di un ingresso Reset, all'accensione potrebbero trovarsi in uno stato indefinito: di conseguenza, lo Stack Pointer deve essere inizializzato dal programma caricato in memoria dall'utente.

Le istruzioni del 6502 che interagiscono con lo stack sono:

- PHA, PLA: Push e Pull del registro A verso lo stack / dallo stack
- PHP, PLP: Push e Pull del registro dei Flag verso lo stack / dallo stack
- JSR, RTS: salto a / ritorno da subroutine
- TXS, TSX: caricamento / lettura dello SP tramite il registro X

## Implementazione del microcode dello Stack Pointer

Analizziamo un'istruzione JSR, ipotizzando di aver precedentemente inizializzato lo SP del BEAM a 0xFF. Il BEAM dispone di 256 byte, quindi lo SP punta ora all'ultima locazione di memoria del computer.

Supponiamo di avere il seguente codice, nel quale una istruzione NOP è seguita da un salto a una subroutine che incrementa il registro X. Al ritorno dalla subroutine, il programma continua con un'altra istruzione NOP.

~~~text
| Mnemonico | Indirizzo | Valore | Contenuto                         |
| --------- | --------- | ------ | --------------------------------- |
| ...       | ...       | ?      | Istruzione precedente o operando  |
| NOP       | 0x1F      | 0x0F   | Opcode istruzione NOP del BEAM    |
| JSR $30   | 0x20      | 0x41   | Opcode istruzione JSR del BEAM    |
|           | 0x21      | 0x30   | Indirizzo di salto istruzione JSR |
| NOP       | 0x22      | 0x0F   | Opcode istruzione NOP del BEAM    |
| ...       | ...       | ?      | Prossima istruzione               |
| ...       | ...       | ?      | ...                               |
| INX       | 0x30      | 0xA0   | Opcode istruzione INX del BEAM    |
| RTS       | 0x31      | 0x11   | Opcode istruzione RTS del BEAM    |
~~~

La tabella che segue **evidenzia visivamente** l'esecuzione dell'instruzione JSR con le modifiche apportate dalle microistruzioni ai registri Program Counter (PC), Stack Pointer, Memory Address Register (MAR), Instruction Register (IR), B e al valore della locazione di memoria indirizzata dal MAR. La tabella include anche l'istruzione precedente NOP e il primo step della istruzione successiva INX richiamata dalla JSR. I valori in **grassetto** indicano una variazione rispetto allo step precedente.

 | Istruzione | Step   | Microistruzioni   | PC      | SP      | MAR     | RAM     | IR      | B       |
 |------------|--------|-------------------|---------|---------|---------|---------|---------|---------|
 | NOP        | 0\*    | RPC \| WM         | 0x1F    | 0xFF    | <b>0x1F | <b>0x0F | ?       | ?       |
 | NOP        | 1\*    | RR  \| WIR \| PCI | <b>0x20 | 0xFF    | 0x1F    | 0x0F    | <b>0x0F | ?       |
 | NOP        | 2\*    | NI                | 0x20    | 0xFF    | 0x1F    | 0x0F    | 0x0F    | ?       |
 | JSR        | 0      | RPC \| WM         | 0x20    | 0xFF    | <b>0x20 | <b>0x41 | 0x0F    | ?       |
 | JSR        | 1      | RR  \| WIR \| PCI | <b>0x21 | 0xFF    | 0x20    | 0x41    | <b>0x41 | ?       |
 | JSR        | 2      | RPC \| WM         | 0x21    | 0xFF    | <b>0x21 | <b>0x30 | 0x41    | ?       |
 | JSR        | 3      | RR  \| WB  \| PCI | <b>0x22 | 0xFF    | 0x21    | 0x30    | 0x41    | <b>0x30 |
 | JSR        | 4      | RS  \| WM         | 0x22    | 0xFF    | <b>0xFF | ?\*\*   | 0x41    | 0x30    |
 | JSR        | 5      | RPC \| WR         | 0x22    | 0xFF    | 0xFF    | <b>0x22 | 0x41    | 0x30    |
 | JSR        | 6      | SE                | 0x22    | <b>0xFE | 0xFF    | 0x22    | 0x41    | 0x30    |
 | JSR        | 7      | RB  \| WPC \| NI  | <b>0x30 | 0xFE    | 0xFF    | 0x22    | 0x41    | 0x30    |
 | INX        | 0\*\*\*| RPC \| WM         | 0x30    | 0xFE    | <b>0x30 | <b>0xA0 | 0x41    | 0x30    |

*Scomposizione dell'istruzione JSR nelle sue otto microistruzioni elementari e raffigurazione dello stato dei registri e della RAM al termine di ogni step.*

\* Istruzione NOP precedente  
\*\* Il valore contenuto in questo istante nella locazione di memoria 0xFF non è noto; peraltro, è ininfluente, in quanto lo step successivo ne sovrascrive il contenuto con l'indirizzo di ritorno dalla subroutine  
\*\*\* Primo step dell'istruzione INX successiva

1. Il primo step dell'istruzione JSR carica l'indirizzo dell'istruzione corrente nel Memory Address Register:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'istruzione nel MAR
2. Il secondo step carica l'opcode dell'istruzione nell'IR e incrementa il PC per farlo puntare alla locazione di memoria successiva (che contiene l'operando dell'istruzione JSR, cioè l'indirizzo di destinazione del salto):
    - RR, Read RAM - espone sul bus l'opcode dell'istruzione
    - WIR, Write Instruction Register - carica l'opcode nell'IR\*
    - PCI, Program Counter Increment - incrementa il PC
3. Il terzo step carica nel Memory Address Register l'indirizzo del Program Counter, che ora punta all'operando:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'operando nel MAR
4. Il quarto step carica l'indirizzo della subroutine in B e incrementa il PC, che va ora a puntare alla prossima istruzione, coincidente con l'indirizzo di ritorno dalla subroutine:
    - RR, Read RAM - espone l'operando sul bus
    - WB, Write B - memorizza in B l'indirizzo di destinazione del salto
    - PCI, Program Counter Increment - incrementa il PC
5. Il quinto step carica nel Memory Address Register l'indirizzo dello Stack Pointer, che corrisponde alla prima locazione libera dello stack:
    - RS, Read Stack - espone sul bus l'indirizzo il valore dello SP
    - WM, Write Memory Address Register - carica l'indirizzo dello SP nel MAR
6. Il sesto step carica nello stack l'indirizzo del PC, al quale il computer ritornerà alla fine della subroutine invocata dall'istruzione JSR:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WR, Write RAM - carica nello stack l'indirizzo di ritorno dalla subroutine
7. Il settimo step decrementa (post-decrement) lo Stack Pointer:
    - SE, Stack Enable - i contatori '169 decrementano l'output di un'unità, andando a puntare alla prossima locazione libera dello stack
8. L'ottavo step carica nel PC l'indirizzo della subroutine invocata dall'istruzione JSR:
    - RB, Read B - espone sul bus il registro B, nel quale era stato precedentemente memorizzato l'indirizzo della subroutine
    - WPC, Write Program Counter - carica nel PC l'indirizzo della subroutine
    - NI, Next Instruction - resetta il Ring Counter

\*Notare come l'Instruction Register venga aggiornato solo alla fine del secondo step dell'istruzione, come già visto nella spiegazione delle [Fasi](../control/#fasi) della CPU.

Alla fine della subroutine, l'istruzione RTS esegue i seguenti passaggi:

- incrementa l'SP (pre-increment);
- carica il nuovo valore dell'SP nel MAR;
- legge dallo stack l'indirizzo di ritorno e lo carica nel PC.

## Lo Stack Pointer dell'NQSAP / NQSAP-PCB

Nella documentazione dell'NQSAP, Tom segnala di aver inizialmente previsto l'utilizzo di Synchronous 4-Bit Up/Down Binary Counters <a href="https://www.ti.com/lit/ds/symlink/sn74ls193.pdf" target="_blank">74LS193</a>, incorrendo nelle [problematiche di glitching](../control/#clock-glitching-delle-eeprom-e-instruction-register-parte-2) delle EEPROM, descritte in una apposita sezione della documentazione della Control Logic del BEAM.

Poiché Tom non aveva pubblicato lo schema dello Stack Pointer dell'NQSAP, lo sostituiamo con quello dell'NQSAP-PCB.

[![Schema dello Stack Register del computer NQSAP-PCB](../../assets/sp/70-stack-nqsap-pcb.png "Schema dello Stack Register del computer NQSAP-PCB"){:width="66%"}](../../assets/sp/70-stack-nqsap-pcb.png)

*Schema dello Stack Register del computer NQSAP-PCB.*

Come si può vedere, Tom ritorna sui suoi passi ed utilizza proprio i '193, che contano verso l'alto o verso il basso in corrispondenza del Rising Edge dei segnali dedicati Up e Down. Tuttavia, l'NQSAP-PCB non soffre del problema del glitching, in quanto l'Instruction Register è stato bufferizzato.

L'SP dell'NQSAP-PCB è governato dai segnali SE (Stack Enable) e C0/C1, che determinano la direzione del conteggio. C0 e C1 sostituiscono una parte dei segnali dell'NQSAP: Il consolidamento su C0 e C1 di alcuni segnali di controllo del registro dei Flag, dei segnali di controllo del registro DXY e dei segnali di direzione del conteggio dello SP ha permesso di ridurre il numero di EEPROM da 4 a 3. Un effetto collaterale è l'impossibilità di eseguire nello stesso step operazioni su stack, registri DXY e/o modulo dei Flag.

## Schema

[![Schema dello Stack Register del computer BEAM](../../assets/sp/70-stack-pointer-schema.png "Schema dello Stack Register del computer BEAM"){:width="100%"}](../../assets/sp/70-stack-pointer-schema.png)

*Schema dello Stack Register del computer BEAM.*

## Link utili

- Il video <a href="https://www.youtube.com/watch?v=xBjQVxVxOxc&t=945s" target="_blank">What is a stack and how does it work?</a> di Ben Eater. Il video fa parte della serie dedicata al computer 6502 (e non all'8-bit computer), ma l'argomento è affine e la trattazione merita assolutamente di essere vista.
- <a href="https://wilsonminesco.com/stacks/basics.html" target="_blank">Stack definition and basics</a> di Garth Wilson, contributore di <a href="http://www.6502.org" target="_blank">6502.org</a> e curatore di <a href="https://wilsonminesco.com/" target="_blank">Wilson Mines Co.</a>, vera miniera di articoli, nozioni, tutorial ed altro sul 6502.
- Lo <a href="https://tomnisbet.github.io/nqsap/docs/stack-pointer/" target="_blank">Stack Pointer dell'NQSAP</a> di Tom Nisbet
- Lo <a href="https://tomnisbet.github.io/nqsap-pcb/docs/program-counter-stack-pointer/" target="_blank">Stack Pointer dell'NQSAP-PCB</a> di Tom Nisbet
