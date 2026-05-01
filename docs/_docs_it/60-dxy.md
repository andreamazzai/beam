---
title: "D, X e Y"
permalink: /docs/dxy/
excerpt: "Registri indice del computer BEAM"
---
<small>[Il registro D](#il-registro-d) - [Utilizzo con le modalità di indirizzamento indicizzate](#utilizzo-con-le-modalità-di-indirizzamento-indicizzate) - [Utilizzo per i salti condizionali](#utilizzo-per-i-salti-condizionali) - [Schema](#schema) - [Differenze tra Registri indice dell’NQSAP e del BEAM](#differenze-tra-registri-indice-dellnqsap-e-del-beam) - [Link utili](#link-utili)</small>

[![Registri indice del computer BEAM](../../assets/dxy/60-beam-dxy.png "Registri indice del computer BEAM"){:width="100%"}](../../assets/dxy/60-beam-dxy.png)

Nel microprocessore 6502 sono presenti due registri indice, X e Y, che possono facilmente essere riprodotti in un computer TTL; sono registri indipendenti che possono essere scritti e letti alla bisogna.

## Il registro D

Il registro D dell'NQSAP e del BEAM viene utilizzato a supporto delle istruzioni di salto condizionale e di quelle che eseguono operazioni in una locazione di memoria che è il risultato di un computo tra l'indirizzo base specificato nell'operando e quattro diversi modi di interpretazione dei valori assunti dai registri indice X e Y:

| Modalità indirizzamento | Agisce sull'indirizzo definito                                            | Mnemonico    | Risultato                                                          | Esempio*                                                |
| -                  | -                                                                              | -            | -                                                                  | -                                                       |  
| Absolute, X        | dalla somma tra operando e valore contenuto in X                               | LDA ($20, X) | A = valore contenuto nella locazione ($20 + X)                     | X = #$03; A = valore presente nella locazione $23       |
| Absolute, Y        | dalla somma tra operando e valore contenuto in Y                               | LDA ($20, Y) | A = valore contenuto nella locazione ($20 + Y)                     | Y = #$03; A = valore presente nella locazione $23       |
| Indexed Indirect X | nella locazione puntata dalla somma tra operando e valore contenuto in X | LDA ($20, X) | A = valore contenuto nella locazione puntata dal pointer ($20 + X) | X = #$03; $23 = #$50; A = valore presente nella locazione $50 |
| Indirect Indexed Y | dalla somma tra locazione puntata dall’operando e valore contenuto in Y | LDA ($20), Y  | A = valore contenuto nella locazione puntata dal pointer ($20) + Y | Y = #$03; $20 = #$60; A = valore presente nella locazione $63 |

\* $23 = #$50 significa che l'indirizzo esadecimale $23 contiene il valore esadecimale #$50:

- la notazione $ fa riferimento a un indirizzo
- la notazione #$ fa riferimento a un valore

Va ricordato che alcune modalità di indirizzamento del 6502 risultano ridondanti in un computer con soli 256 byte di RAM: per un chiarimento si veda la sezione [Indirizzamenti](../alu/#indirizzamenti) nella pagina dell'ALU.

### Utilizzo con le modalità di indirizzamento indicizzate

Il contenuto del registro D può essere sommato al contenuto dei registri X, Y o nessuno (in questo ultimo caso, D potrebbe risultare utile come ulteriore registro di appoggio per definire altre istruzioni personalizzate; l'utilizzo di questa modalità non è invece necessario per emulare le istruzioni originali del 6502).

Per selezionare se D debba essere sommato a X, Y o nulla, si usano dei multiplexer (MUX) <a href="https://www.ti.com/lit/ds/symlink/sn74ls157.pdf" target="_blank">74LS157</a> pilotati dai segnali DY e DZ nell'NQSAP e DX/Y e DZ nel BEAM.

Per eseguire il computo si usano dei 4-Bit Binary Full Adders With Fast Carry <a href="https://www.ti.com/lit/ds/symlink/sn54s283.pdf" target="_blank">74LS283</a> che eseguono la somma tra quanto viene caricato nel registro D e il valore contenuto nei registri indice X o Y.

Il risultato del computo può essere esportato attivando il segnale RD: gli output degli Adder saranno così esposti sul bus. Notare che il registro D è "Write only", dunque non è possibile metterne direttamente il contenuto in ouput sul bus; tuttavia, è possibile leggere il valore originariamente presente in D portando il segnale DZ allo stato HI, disabilitando così le uscite dei '157 e portando gli Adder a eseguire l'operazione D + 0 = D.

Il flusso logico tratto dalla <a href="https://tomnisbet.github.io/nqsap/docs/dxy-registers/" targe ="_blank">spiegazione di Tom Nisbet</a> chiarisce il funzionamento:

![Registri D, X e Y in azione](../../assets/dxy/60-dxy-nqsap-tom-flow.png "Registri D, X e Y in azione"){:width="66%"}

### Utilizzo per i salti condizionali

Un salto condizionale viene eseguito se una determinata situazione è verificata. Nell'esempio seguente, l'istruzione BCS è sicuramente eseguita. Il valore dell'operando $03 viene sommato all'indirizzo dell'istruzione *successiva* a quella di salto, cioé $03 + $83 = $86, che sarà caricato nel Program Counter (PC).

~~~text
SEC         ; $80 - Set Carry Flag
BCS $03     ; $81 - Salta a $03 + $83 = $86 se il Carry è settato
INX         ; $83 - Questa istruzione sarà saltata
INX         ; $84 - Questa istruzione sarà saltata
INX         ; $85 - Questa istruzione sarà saltata
LDA #$01    ; $86 - Questa istruzione verrà eseguita
~~~

L'operando è un valore a 8 bit con segno (Signed) che può variare da -128 a +127, il che comporta che i salti condizionali possono saltare in avanti di 128 indirizzi e all'indietro di 127.

Per saltare a un indirizzo precedente a quello del salto, il valore dell'operando dovrà essere un valore compreso tra $80 (-128) e $FF (-1), secondo le regole visibile nella sezione [Numeri Unsigned e numeri Signed](../math/#numeri-unsigned-e-numeri-signed) della pagina dedicata all'Aritmetica binaria (se aggiungo 0x01 all'indirizzo più grande 0xFF, ritorno all'indirizzo 0x00).

~~~text
LDX #$05    ; $80 - Carica il registro X con 5
DEX         ; $82 - Decrementa X
BNE $FD     ; $83 - Salta a $85 + $FD = $85 - $03 = $82 se X non è zero
RTS         ; $85 - Ritorno da subroutine
~~~

Poiché la configurazione utilizzata per gli Adder '283 permette la sola esecuzione di addizioni, ci si potrebbe chiedere come sia possibile eseguire un salto all'indietro. Nell'NQSAP, così come nel BEAM, i registri sono a 8 bit: una semplice scorciatoia per giungere al risultato desiderato è quella di aggiungere l'operando al PC considerando indirizzo e operando come numeri Unsigned: nel caso specifico, $85 + $FD = $182. Il 9° bit (corrispondente al Carry) non viene preso in considerazione e il rimanente $82 verrà caricato nel PC.

Questa tecnica funziona correttamente grazie alla natura ciclica dell'indirizzo di memoria in un sistema a 8 bit.

[![Schema dei registri indice dell'NQSAP](../../assets/dxy/60-nqsap-dxy-schema.png "Schema dei registri indice dell'NQSAP"){:width="100%"}](../../assets/dxy/60-nqsap-dxy-schema.png)

*Schema dei registri indice dell'NQSAP.*

## Schema

[![Schema dei Registri indice del computer BEAM](../../assets/dxy/60-beam-dxy-schema.png "Schema dei Registri indice del computer BEAM"){:width="100%"}](../../assets/dxy/60-beam-dxy-schema.png)

*Schema dei registri indice del computer BEAM.*

## Differenze tra Registri indice dell'NQSAP e del BEAM

Dal punto di vista funzionale, gli schemi dei Registri indice dell'NQSAP e del BEAM sono identici.

Negli appunti annotavo che "... come per gli altri registri del BEAM, anche qui uso dei registri tipo D <a href="https://www.ti.com/lit/ds/symlink/sn54ls377.pdf" target="_blank">74LS377</a> anziché gli Octal D-Type Flip-Flop with 3-State Outputs <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> usati da Tom nell'NQSAP"; si veda la sezione [L'ALU dell'NQSAP](../alu/#lalu-dellnqsap) per un chiarimento in tal senso.

Per completezza, devo segnalare di aver conosciuto il 74LS377 studiando l'<a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, evoluzione dell’NQSAP che Tom aveva successivamente ingegnerizzato su PCB anziché su breadboard.

## Link utili

- I <a href="https://tomnisbet.github.io/nqsap/docs/dxy-registers/" target="_blank">registri DXY dell'NQSAP</a> di Tom Nisbet
