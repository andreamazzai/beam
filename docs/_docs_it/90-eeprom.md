---
title: "EEPROM Programmer"
permalink: /docs/eeprom-programmer/
excerpt: "EEPROM Programmer"
---
<small>[Il programmatore di EEPROM](#il-programmatore-di-eeprom) - [Schema](#schema) - [Spiegazione del codice](#spiegazione-del-codice) - [Le EEPROM e il loro contenuto](#le-eeprom-e-il-loro-contenuto) - [Calcolo del CRC pre-programmazione](#calcolo-del-crc-pre-programmazione) - [Sblocco e blocco della EEPROM](#sblocco-e-blocco-della-eeprom) - [Cancellazione della EEPROM](#cancellazione-della-eeprom) - [Programmazione della EEPROM](#programmazione-della-eeprom) - [Verifica del CRC post-programmazione](#verifica-del-crc-post-programmazione) - [Note](#note) - [Link utili](#link-utili)</small>

[![EEPROM programmer](../../assets/eeprom/eeprom-programmer.png "EEPROM programmer"){:width="100%"}](../../assets/eeprom/eeprom-programmer.png)

Il programmatore di EEPROM serve a programmare le EEPROM della [Control Logic](../control) con il microcode necessario ad eseguire le istruzioni definite per il computer BEAM.

## Il programmatore di EEPROM

La mia prima esperienza con la programmazione di EEPROM risale alla costruzione del computer SAP-1 di Ben Eater e alla realizzazione del programmatore basato sui suoi schema e sketch. Questo progetto, molto semplice, permetteva di programmare le EEPROM del microcode, anche se la scrittura risultava particolarmente lenta. Ciononostante, la programmazione di una EEPROM (dapprima manualmente, poi con il programmatore) è stato un momento euforico - mai avrei pensato di riuscire a comprendere i (tutto sommato abbastanza semplici) meccanismi che lo rendevano possibile.

[![Schema del programmatore di EEPROM del computer SAP](../../assets/eeprom/eeprom-ben.png "Schema del programmatore di EEPROM del computer SAP"){:width="100%"}](../../assets/eeprom/eeprom-ben.png)

*Schema del programmatore di EEPROM del computer SAP-1.*

Durante lo studio del computer NQSAP di Tom Nisbet e la rivisitazione del suo progetto per creare il computer BEAM, mi ero reso conto che il numero di EEPROM da programmare (quattro anziché due) e la loro [dimensione](../control/#instruction-register-e-istruzioni) avrebbero ulteriormente aumentato i tempi di programmazione di ogni revisione del microcode. Il programmatore di Ben era funzionale, ma non sfruttava la funzionalità di scrittura a *pagine* delle EEPROM moderne, che permette la programmazione di una EEPROM da 32KB in pochi secondi. Ciononostante, continuavo a usarlo, con tempi di scrittura di parecchie decine di secondi per ogni EEPROM.

Con il progredire della costruzione dei moduli del BEAM, le riprogrammazioni necessarie ad implementare l'Instruction Set e risolvere i bug del microcode diventavano sempre più frequenti; il tempo trascorso ad attendere il completamento dei cicli di programmazione iniziava a diventare frustrante.

Uno dei progetti di Tom, il programmatore <a href="https://github.com/TomNisbet/TommyPROM" target="_blank">TommyPROM</a>, supportava una varietà di EPROM, EEPROM e memorie Flash. Questo programmatore includeva un software molto ricco e molto veloce grazie al supporto della modalità di scrittura *Page Write*. Una versione ridotta del software del TommyPROM - col solo scopo di creare il microcode e programmare le EEPROM 28C256 - venne pubblicata da Tom sul suo repository dell'NQSAP.

Non volevo iniziare il progetto di un nuovo programmatore di EEPROM mentre stavo ancora lavorando sul BEAM. Tuttavia, a un certo punto avevo realizzato che avrei speso meno tempo nel costruire un programmatore basato sul TommyPROM rispetto al tempo che avrei sprecato continuando ad utilizzare il programmatore elementare di Ben.

[![Schema del programmatore di EEPROM TommyPROM di Tom Nisbet](../../assets/eeprom/TommyPROM-nano-sch.png "Schema del programmatore di EEPROM TommyPROM di Tom Nisbet"){:width="100%"}](../../assets/eeprom/TommyPROM-nano-sch.png)

*Schema del programmatore di EEPROM TommyPROM di Tom Nisbet.*

Il mio programmatore iniziale era uguale a quello sviluppato da Ben per il SAP-1 e utilizzava i <a href="https://www.ti.com/lit/ds/symlink/sn74hc595.pdf" target="_blank">74HC595</a> anziché i <a href="https://www.ti.com/lit/ds/symlink/sn74ls164.pdf" target="_blank">74HCT164</a> utilizzati da Tom. In una sezione della documentazione, Tom evidenziava che il suo codice non funzionava sul progetto di Ben e che necessitava di significative modifiche. Se si desideravano i benefici del TommyPROM sull'hardware di Ben, Tom indicava che la strada più facile non era quella di modificare il software, bensì di adattare l'hardware rendendolo uguale a quello del TommyPROM.

Nella mia cocciutaggine, avevo optato per un compromesso: modificare solo parzialmente l'hardware sviluppato nel progetto iniziale di Ben e contemporaneamente approfittarne per studiare e modificare il software di Tom rendendolo compatibile col mio hardware, al quale desideravo applicare il minor numero di modifiche possibili. In quel momento non sapevo ancora che il programmatore rivisitato avrebbe visto la luce in un tempo sostanzialmente limitato, ma che la corretta implementazione della modalità *Page Write* e soprattutto gli errori riscontrati nel computo del CRC avrebbero richiesto uno sforzo piuttosto oneroso.

Le modifiche minime necessarie rispetto al programmatore originale del SAP-1 erano le seguenti:

1. il pin Write Enable della EEPROM (/WE) non più controllato dal segnale D13 di Arduino, bensì da A2.
2. il pin Chip Enable della EEPROM (/CE) non più connesso a ground (chip sempre attivo), bensì controllato dal pin A0 di Arduino.
3. il pin Output Enable della EEPROM (/OE) non più connesso allo Shift Register, ma controllato dal pin A1 di Arduino.

Nella sua documentazione Tom spiega che nel programmatore di Ben Eater D13 controlla il segnale Write Enable. Poiché D13 è internamente connesso al LED integrato su Arduino, che durante il boot lampeggia, a ogni accensione del programmatore potrebbero verificarsi scritture indesiderate nella EEPROM. Questo non rappresentava un problema per lo sketch di Ben, poiché a ogni esecuzione la EEPROM veniva completamente riprogrammata, sovrascrivendo eventuali scritture spurie. Tuttavia, dato che il TommyPROM poteva essere utilizzato anche per la sola lettura di una EEPROM, le scritture iniziali indesiderate avrebbero potuto causare problemi: di qui la necessità di governare /WE con un segnale diverso da D13.

Altro aspetto da tenere in considerazione era legato al pin Output Enable delle EEPROM: gli Shift Register '164 non sono dotati di una memoria intermedia che funga da *latch* durante il caricamento. Di conseguenza, i loro output vengono immediatamente modificati ad ogni Rising Edge del clock, sottoponendo /OE a possibili attivazioni indesiderate. Viceversa, i 74HC595 permettono il caricamento seriale dei dati in un latch interno e la erogazione contemporanea di tutti i nuovi stati di output al Rising Edge di un segnale dedicato (RCLK), annullando il rischio di attivazioni indesiderate di /OE. Tuttavia, durante la realizzazione delle modifiche al programmatore non avevo ben compreso questo aspetto e avevo deciso di seguire l'indicazione di Tom, cioè di dedicare un output di Arduino specificamente ad /OE della EEPROM. In altre parole, la modifica al punto 3 si sarebbe potuta evitare, ma in quel momento non l'avevo capito.

Un'altra differenza stava nella modalità di scrittura degli Shift Register: il programmatore di Ben presentava i due SR connessi in cascata, mentre quello di Tom utilizzava due segnali di clock distinti per selezionare quale SR indirizzare. L'idea molto furba alla base della modifica di Tom permetteva di modificare le sole uscite del secondo SR senza effettuare lo scorrimento completo dell'indirizzo, risparmiando preziosi cicli di clock dell'Arduino. Avevo scelto di non riprendere questa modifica nel programmatore del BEAM e continuare con la sequenza di shift completa, sperando di rispettare i parametri di timing molto stringenti richiesti dalla modalità di programmazione *Page Write* delle EEPROM.

## Schema

[![Schema del programmatore di EEPROM del computer BEAM](../../assets/eeprom/90-eeprom-schema.png "Schema del programmatore di EEPROM del computer BEAM"){:width="100%"}](../../assets/eeprom/90-eeprom-schema.png)

*Schema del programmatore di EEPROM del computer BEAM.*

## Spiegazione del codice

Il programmatore di EEPROM del BEAM, basato su quello dell'NQSAP di Tom, non è interattivo, a differenza del ben più completo TommyProm. Una volta fatto partire, esegue in sequenza i seguenti passaggi, alla fine dei quali è pronto per un reset e la programmazione di una nuova EEPROM:

1. Calcolo del checksum dei dati da scrivere sulla EEPROM
2. Sblocco della EEPROM
3. Cancellazione della EEPROM
4. Programmazione della EEPROM
5. Blocco della EEPROM
6. Calcolo del checksum rileggendo i dati scritti e confronto col valore calcolato al punto 1
7. Stampa del tempo trascorso

### Le EEPROM e il loro contenuto

Per governare i 42 [segnali di controllo](../control/#segnali-di-controllo) di ALU, RAM, SP, registri ecc. (21 direttamente in uscita dalle EEPROM + 21 demultiplexati dai [74LS138](../control/#i-74ls138-per-la-gestione-dei-segnali)) sono necessarie quattro EEPROM, ognuna delle quali esporta una word da 8 bit per un totale di 32 bit (i segnali fisici realmente necessari sono 29, cioè i 21 diretti ed 8 per governare i '138, lasciando 3 pin inutilizzati):

- una EEPROM mette a disposizione 8 bit in output, perciò ne servono 4 per pilotare simultaneamente 29 segnali;
- poiché ognuna delle 256 istruzioni del BEAM può essere composta da un massimo di 16 step, sono necessarie EEPROM di dimensione 256 * 16 = 4096 byte;
- per indirizzare 4096 byte sono necessari 12 pin di indirizzamento ((2^8 = 256 istruzioni) * (2^4 = 16 step) = 2^12 = 4096), cioè da A0 a A11;
- quattro EEPROM da 4KB, ognuna delle quali programmata con il proprio microcode, possono svolgere il compito richiesto.

Vediamo di seguito un dettaglio del microcode di alcune istruzioni di esempio, in particolar modo HLT (blocca l'esecuzione del codice), JMP (salta a un nuovo indirizzo definito nella locazione di memoria indicata dall'operando) e CPX (confronta il registro X con l'operando). L'istruzione più lunga tra quelle rappresentate è CPX, la cui durata è di 7 step (da 0 a 6); altre istruzioni del BEAM raggiungono una lunghezza di ben 10 step.

~~~c++
// Step:
// <0>     <1>         <2>     <3>    <4>        <5>            <6>           
{  RPC|WM, RR|WIR|PCI, HLT,    NI,    0,         0,             0,           }, // 00 HLT
{  RPC|WM, RR|WIR|PCI, RPC|WM, RR|WM, RR|WPC|NI, 0,             0,           }, // 01 JMP
{  RPC|WM, RR|WIR|PCI, RPC|WM, RR|WB, RX|WH,     CS|C0|FNZC|RL, RA|WH|PCI|NI }, // 06 CPX
~~~

*Dettaglio microcode di alcune istruzioni di esempio.*

Ogni step abilita uno o più segnali: ad esempio il settimo step dell'istruzione CPX attiva contemporaneamente RA, WH (che a sua volta è composto da HL ed HR), PCI ed NI.

Come si può vedere nello [sketch Arduino](https://github.com/andreamazzai/beam), ad ogni segnale di controllo corrispondono uno o più pin specifici nelle tre EEPROM numerate 1, 2 e 3:

[![Definizione dei segnali di controllo gestiti dalle EEPROM 1, 2 e 3](../../assets/eeprom/eeprom-pins.png "Definizione dei segnali di controllo gestiti dalle EEPROM 1, 2 e 3"){:width="100%"}](../../assets/eeprom/eeprom-pins.png)

*Definizione dei segnali di controllo gestiti dalle EEPROM 1, 2 e 3.*

Perché *uno o più pin specifici*? Perché, ad esempio, il mnemonico WH è in realtà una macro che attiva sia HL, sia HR.

~~~c++
#define WH  HR|HL             // Write H
~~~

La EEPROM 0 governa invece i quattro demultiplexer '138, dunque le combinazioni dei suoi 8 bit di output sono in grado di pilotare ben 32 segnali (ma quelli utilizzati sono solo 21).

Si può dedurre che ogni EEPROM contiene solamente *una parte* del microcode di ogni istruzione, cioè (ovviamente) la porzione relativa ai segnali cablati sui suoi output. Ma come è suddiviso il microcode nelle quattro EEPROM? La seguente tabella mostra, per le istruzioni di esempio indicate in precedenza, quali segnali siano attivi su ogni EEPROM nei diversi step dell'istruzione correntemente in esecuzione:

[![Suddivisione sulle quattro EEPROM del microcode di alcune istruzioni](../../assets/eeprom/4-eeprom-rappresentazione.png "Suddivisione sulle quattro EEPROM del microcode di alcune istruzioni"){:width="100%"}](../../assets/eeprom/4-eeprom-rappresentazione.png)

*Suddivisione sulle quattro EEPROM del microcode di alcune istruzioni.*

Ogni step di ogni istruzione va dunque letto come la concatenazione logica di ogni ennesimo byte di ogni EEPROM:

 | Istruzione | Step | Concatenazione logica                                                                                                                                    |
 |------------|------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
 | 0          | 0    | EEPROM<sub>0</sub>byte<sub>0</sub> OR EEPROM<sub>1</sub>byte<sub>0</sub> OR EEPROM<sub>2</sub>byte<sub>0</sub> OR EEPROM<sub>3</sub>byte<sub>0</sub>     |
 | 0          | 1    | EEPROM<sub>0</sub>byte<sub>1</sub> OR EEPROM<sub>1</sub>byte<sub>1</sub> OR EEPROM<sub>2</sub>byte<sub>1</sub> OR EEPROM<sub>3</sub>byte<sub>1</sub>     |
 | 0          | ...  | ...                                                                                                                                                      |
 | 0          | 14   | EEPROM<sub>0</sub>byte<sub>14</sub> OR EEPROM<sub>1</sub>byte<sub>14</sub> OR EEPROM<sub>2</sub>byte<sub>14</sub> OR EEPROM<sub>3</sub>byte<sub>14</sub> |
 | 0          | 15   | EEPROM<sub>0</sub>byte<sub>15</sub> OR EEPROM<sub>1</sub>byte<sub>15</sub> OR EEPROM<sub>2</sub>byte<sub>15</sub> OR EEPROM<sub>3</sub>byte<sub>15</sub> |
 | 1          | 0    | EEPROM<sub>0</sub>byte<sub>16</sub> OR EEPROM<sub>1</sub>byte<sub>16</sub> OR EEPROM<sub>2</sub>byte<sub>16</sub> OR EEPROM<sub>3</sub>byte<sub>16</sub> |
 | 1          | 1    | EEPROM<sub>0</sub>byte<sub>17</sub> OR EEPROM<sub>1</sub>byte<sub>17</sub> OR EEPROM<sub>2</sub>byte<sub>17</sub> OR EEPROM<sub>3</sub>byte<sub>17</sub> |
 | 1          | ...  | ...                                                                                                                                                      |
 | 1          | 14   | EEPROM<sub>0</sub>byte<sub>30</sub> OR EEPROM<sub>1</sub>byte<sub>30</sub> OR EEPROM<sub>2</sub>byte<sub>30</sub> OR EEPROM<sub>3</sub>byte<sub>30</sub> |
 | 1          | 15   | EEPROM<sub>0</sub>byte<sub>31</sub> OR EEPROM<sub>1</sub>byte<sub>31</sub> OR EEPROM<sub>2</sub>byte<sub>31</sub> OR EEPROM<sub>3</sub>byte<sub>31</sub> |

Ad esempio, il sesto step dell'istruzione CPX risulta in effetti composto dalla concatenazione del byte 101 di ogni EEPROM (le istruzioni e gli step si contano a partire da zero, dunque (settima istruzione \* 16 step + sesto step) = ((7-1) \* 16 + (6-1)) = 96 + 5 = 101):

EEPROM<sub>0</sub>byte<sub>101</sub> + EEPROM<sub>1</sub>byte<sub>101</sub> + EEPROM<sub>2</sub>byte<sub>101</sub> + EEPROM <sub>3</sub>byte<sub>101</sub>, cioè

(RL) + (FNZC) + (CS\|C0) + (), cioè

RL + FNZC + CS\|C0, così come indicato nel *Dettaglio microcode di alcune istruzioni di esempio*.

In pratica, si devono tenere in considerazione i segnali di output cablati su ogni EEPROM e indicare quali di questi debbano essere attivi ad ogni combinazione di istruzione / step. Questo spiega la necessità di programmare le quattro EEPROM ognuna con una propria porzione specifica di microcode.

Ora, anziché effettuare quattro programmazioni distinte, risulta molto più comodo (anche se più dispendioso) utilizzare quattro EEPROM da 16KB e scrivere su ognuna di esse, in sequenza, tutti e quattro i microcode da 4KB inizialmente definiti. In questo modo, si possono programmare quattro EEPROM identiche da 16KB, ognuna delle quali conterrà tutte le porzioni di microcode da 4KB.

La tabella riassume la collocazione dei microcode da 4KB consolidati in un'unica EEPROM di dimensioni maggiori:

 | Microcode<br>originario | Indirizzo<br>iniziale<sub>base10</sub> | Indirizzo<br>finale<sub>base10</sub> | Indirizzo<br>iniziale<sub>hex</sub> | Indirizzo<br>finale<sub>hex</sub> | A12 | A13 |
 |------------|-------|-------------------|--------|--------|---|---|
 | 1°         | 0     | 4095              | 0x0000 | 0x0FFF | 0 | 0 |
 | 2°         | 4096  | 8191              | 0x1000 | 0x1FFF | 1 | 0 |
 | 3°         | 8192  | 12287             | 0x2000 | 0x2FFF | 0 | 1 |
 | 4°         | 12288 | 16383             | 0x2000 | 0x3FFF | 1 | 1 |

 *Consolidamento dei microcode in un'unica EEPROM.*

Le colonne degli indirizzi indicano in quali locazioni della EEPROM da 16KB trovino posto le singole istanze da 4KB del microcode originariamente definito utilizzando quattro EEPROM da 4KB.

[![Rappresentazione dei quattro microcode consolidati in un'unica EEPROM](../../assets/eeprom/eeprom-consolidata.png "Rappresentazione dei quattro microcode consolidati in un'unica EEPROM"){:width="100%"}](../../assets/eeprom/eeprom-consolidata.png)

*Rappresentazione dei quattro microcode consolidati in un'unica EEPROM.*

Impostando opportunamente le linee di indirizzamento A12 e A13 delle EEPROM di dimensioni maggiori, è possibile mettere in output una porzione specifica di microcode; si vedano le connessioni fisse a Vcc o GND nello [schema](../control/#schema) della Control Logic. Ad esempio, l'impostazione di A12 a 1 e A13 a 0 farà in modo che una EEPROM da 16KB esponga sui suoi output il microcode da 4KB originariamente definito per la seconda EEPROM da 4KB.

In altre parole, abbiamo 256 istruzioni che si sviluppano in 16 step, ognuno dei quali è composto da una Control Word da 32 bit (4 byte) = 16.384 byte totali. Si dovrebbero programmare 4 EEPROM da 4KB: una per i primi 8 bit della Control Word, una per gli 8 bit successivi e così via. Anziché programmare quattro diverse EEPROM da 4KB ognuna con una porzione specifica di microcode, è possibile programmare quattro EEPROM identiche da 16KB. Nei primi 4KB si posiziona il microcode per i primi 8 bit della Control Word, nei secondi 4KB il microcode per i secondi 8 bit, e così via. Infine, si impostano opportunamente gli indirizzi A12 e A13 delle quattro EEPROM da 16KB, in modo che ognuna esponga solo la porzione specifica di microcode relativa ai segnali di controllo cablati sui suoi output.

Una ulteriore spiegazione visiva di quanto appena descritto si può ottenere a partire dal minuto 17m:52s del video <a href="https://youtu.be/JUVt_KYAp-I?t=1072" target="_blank">Reprogramming CPU microcode with an Arduino</a> di Ben Eater.

In relazione al conteggio della dimensione, si veda anche la sezione [Instruction Register e Istruzioni](../control/#instruction-register-e-istruzioni). Un aspetto da ricordare è che nel mercato non sono presenti EEPROM parallele da 4KB e da 16KB, dunque al loro posto si utilizzano EEPROM da 8KB e 32KB impostando a 0 l'ultimo pin di indirizzamento, avvalendosi dunque solo della prima metà dello spazio disponibile.

Fatta questa premessa, utile per capire la distribuzione del microcode nelle varie EEPROM, possiamo analizzare alcuni aspetti importanti dello sketch Arduino.

### Calcolo del CRC pre-programmazione

La programmazione delle EEPROM col metodo originale "un byte alla volta in sequenza" risultava, tutto sommato, abbastanza facile. Molto più difficile mi stava risultando l'implementazione della scrittura in modalità Page Write e, pur avendo a disposizione il codice di Tom dal quale attingere e trarre ispirazione, continuavo a riscontrare errori nel microcode memorizzato sulle EEPROM.

Dunque, per capire se il programmatore stesse funzionando correttamente, avevo introdotto una verifica della corrispondenza tra il Cyclic Redundancy Check (CRC) calcolato *prima* della scrittura del microcode e quello calcolato dalla rilettura della EEPROM alla *fine* della sua programmazione.

Per semplicità e per facilitare la scrittura del codice, immaginavo una lettura sequenziale simulata dei valori da utilizzare per calcolare il CRC, partendo dall'indirizzo 0x0000 fino all'indirizzo 0x3FFF.\
Perché *simulata*? Perché in questa fase del programma la EEPROM non è ancora stata programmata e ciò che si desidera ottenere ora è proprio un checksum del dato *da scrivere*, checksum che sarà poi confrontato con quello calcolato rileggendo la EEPROM alla *fine* del ciclo di programmazione.

Ci troviamo nella situazione in cui la routine del calcolo del CRC deve ricevere i dati sequenzialmente (dati che dobbiamo produrre utilizzando la routine **buildInstruction** preposta alla creazione di istruzioni e step); tuttavia, la buildInstruction esegue il frazionamento di ogni istruzione nei modi esposti nella [sezione precedente](#le-eeprom-e-il-loro-contenuto), generando cioè un opcode completo e suddividendo le scritture dei 32 bit della Control Word di ogni step sulle corrispondenti porzioni di microcode, come evidenziato nella tabella *Consolidamento dei microcode in un'unica EEPROM* e nella grafica *Rappresentazione dei quattro microcode consolidati in un'unica EEPROM.*

![Microcode](../../assets/eeprom/tabella-grafica.png){:width="100%"}

La **buildInstruction** prepara infatti i 32 bit / 4 byte di microcode di ogni step dell'istruzione corrente e li memorizza in un array tipo uint32_t di lunghezza 16, cioè 4 byte * 16 step = 64 byte; successivamente, le scritture avvengono in questa sequenza (routine **writeOpcode**):

- il microcode relativo ai primi 8 segnali\* viene scritto sui primi 16 byte della prima porzione della EEPROM (indirizzo 0x0000 a 0x000F)
- il microcode relativo ai successivi 8 segnali\* viene scritto sui primi 16 byte della seconda porzione della EEPROM (indirizzo 0x1000 a 0x100F)
- il microcode relativo ai successivi 8 segnali\ viene scritto sui primi 16 byte della terza porzione della EEPROM (indirizzo 0x2000 a 0x200F)
- il microcode relativo agli ultimi 8 segnali\* viene scritto sui primi 16 byte della quarta porzione della EEPROM (indirizzo 0x3000 a 0x300F)

Il contatore dell'istruzione viene poi incrementato e vengono preparati i 16 step dell'istruzione successiva, scritti considerando l'offset di 16 byte di lunghezza di ogni istruzione:

- il microcode relativo ai primi 8 segnali\* viene scritto sui 16 byte successivi della prima porzione della EEPROM (indirizzo 0x0010 a 0x001F)
- il microcode relativo ai successivi 8 segnali\* viene scritto sui 16 byte successivi della seconda porzione della EEPROM (indirizzo 0x1010 a 0x101F)
- il microcode relativo ai successivi 8 segnali\* viene scritto sui 16 byte successivi della terza porzione della EEPROM (indirizzo 0x2010 a 0x201F)
- il microcode relativo agli ultimi 8 segnali\* viene scritto sui 16 byte successivi della quarta porzione della EEPROM (indirizzo 0x3010 a 0x301F)

e così via fino alla fine delle istruzioni.

\* Si faccia riferimento all'immagine *Definizione dei segnali di controllo gestiti da ogni EEPROM* nella [sezione precedente](#le-eeprom-e-il-loro-contenuto).

Riprendendo lo schema visto in precedenza, la **buildInstruction** prepara istruzioni e relativi step, dei quali vengono dapprima eseguite le scritture evidenziate dalle frecce rosse (istruzione 0), successivamente quelle evidenziate dalle frecce blu (istruzione 1) e così via, fino all'ultima istruzione.

[![Sequenza di scrittura delle istruzioni](../../assets/eeprom/eeprom-consolidata-sequenza.png "Sequenza di scrittura delle istruzioni"){:width="100%"}](../../assets/eeprom/eeprom-consolidata-sequenza.png)

*Sequenza di scrittura delle istruzioni.*

Volendo riutilizzare la routine **buildInstruction**, dovevo trovare il modo di interpretarne l'output ed estrapolarne in tempo reale solo ciò che mi interessava al fine di realizzare quella lettura simulata sequenziale discussa all'inizio di questa sezione.

Per ottenere il risultato desiderato, viene eseguita una serie di cicli annidati: per ogni istruzione si genera la Control Word a 32 bit di tutti gli step, dalla quale si estrapola solo la word a 8 bit relativa alla porzione di EEPROM correntemente indirizzata dal ciclo più esterno

~~~c++
for (uint8_t rom = 0; rom < 4; rom++)
~~~

Così facendo, la routine di calcolo del CRC pre-programmazione riceve sequenzialmente in input i 4096 byte di ognuna delle quattro porzioni di microcode:

~~~c++
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
~~~

Con **rom** = 0, **crc** sarà calcolato tenendo in considerazione gli 8 bit più significativi (shift a destra di 24 - 8 \* 0 = 24 posizioni) della Control Word a 32 bit generata da **buildInstruction** e il ciclo sarà eseguito per tutti i 16 step di tutte le 256 istruzioni; quando **rom** = 1, **crc** sarà calcolato tenendo in considerazione i bit da 16 a 23 (shift a destra di 24 - 8 \* 1 = 16 posizioni) della Control Word a 32 bit generata da **buildInstruction** e il ciclo sarà nuovamente eseguito per tutti i 16 step di tutte le 256 istruzioni. Il processo si ripete per **rom** = 2 e 3, prendendo in considerazione i bit da 8 a 15 (shift a destra di 8 posizioni) e infine i bit da 0 a 7 (nessuno shift a destra).

### Sblocco e blocco della EEPROM

Le EEPROM <a href="https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf" target="_blank">AT28C256</a> dispongono della funzione Software Data Protection, che permette di evitare scritture indesiderate: blocco e sblocco si eseguono inviando alla EEPROM una breve sequenza specifica di indirizzi / valori secondo le specifiche di pagina 10 del datasheet.

### Cancellazione della EEPROM

Prima della programmazione, la EEPROM viene azzerata. Questa operazione comporta una maggior usura della EEPROM e una minor velocità rispetto alla funzionalità Software Chip Erase indicata nella <a href="https://ww1.microchip.com/downloads/en/Appnotes/doc0544.pdf" target="_blank">Application Note</a>, al momento non ancora implementata.

### Programmazione della EEPROM

La sequenza di preparazione del microcode è già stata sostanzialmente esposta nella sezione [Calcolo del CRC pre-programmazione](#calcolo-del-crc-pre-programmazione), in quanto la importante routine di generazione del microcode **buildInstruction** è comune.

La programmazione materiale della EEPROM, come indicato, avviene secondo la logica frazionata dettata dalla mia necessità di comprensione del codice esposta in precedenza (immagine *Sequenza di scrittura delle istruzioni* nella sezione [Calcolo del CRC pre-programmazione](#calcolo-del-crc-pre-programmazione)), cioè quella di scrivere un opcode per intero. La ruotine principale **eeprom_program** prepara l'opcode **buildInstruction**

~~~c++
void eeprom_program()
{
  for (uint16_t opcode = 0; opcode < 256; opcode++)
  {
    buildInstruction((uint8_t) opcode);
    // ...
    writeOpcode((uint8_t) opcode);
  }
  Serial.println("Done!");
}
~~~

e richiama la routine **writeOpcode**, che, dopo ogni 16 byte scritti, richiama a sua volta **waitForWriteCycleEnd** per verificare che la EEPROM sia pronta per ricevere nuove scritture, secondo la modalità descritta nella sezione 4.4 DATA Polling del <a href="https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf" target="_blank">datasheet</a>.

~~~c++
void writeOpcode(uint8_t opcode)
{
  for (uint8_t rom = 0; rom < 4; rom++)
  {
    for (uint8_t step = 0; step < NUM_STEPS; step++) // ciclo fra i 16 step di ogni opcode
    // e li scrivo consecutivamente su ogni porzione di EEPROM (modalità Page Write)
    {
      uint16_t address;
      address = 0x1000 * rom;
      address += opcode * NUM_STEPS;
      address += step;
      writeEEPROM(address, ((code[step]) >> (24 - 8 * rom)) & 0xFF); // code[step] prende tutti
      // i 4 byte delle 4 ROM "consolidate" e poi con lo shift si seleziona il byte relativo ad
      // ogni ROM specifica, ad esempio per la prima ROM bit 24-31, poi 16-23 etc
    }
    byte b1Value; // attende che la EEPROM confermi di aver completato le scritture prima di
                  // passare ai prossimi 16 byte
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
~~~

### Verifica del CRC post-programmazione

La verifica del CRC post-programmazione è molto più semplice rispetto a quella pre-programmazione, perché in questo caso non si deve sottostare alla routine **buildInstruction** e lavorare con cicli annidati per ottenere una vista sequenziale simulata dei dati: giunti a questo punto, la EEPROM è stata realmente programmata ed è sufficiente passare alla routine di calcolo del CRC i valori letti consecutivamente da 0x0000 a 0x3FFF.

~~~c++
// CALCOLO CRC16 POST-PROGRAMMAZIONE
// Nella lettura di una EEPROM precedentemente programmata leggo tutti i byte in sequenza,
// dunque è sufficiente leggere il contenuto da 0x0000 a 0x3FFF per calcolare il checksum.
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
~~~

Alla fine, i valori dei CRC calcolati pre-programmazione e post-programmazione vengono confrontati e viene stampato un messaggio positivo in caso di match, nonché il tempo totale trascorso.

## Note

Nella prima sezione menzionavo che la corretta implementazione della modalità *Page Write* avrebbe richiesto uno sforzo maggiore di quanto non pensassi, ma la difficoltà maggiore è stata in realtà riscontrata nella verifica del CRC post-programmazione: anche il timing delle letture è critico e avevo speso diverso tempo cercando la sequenza corretta di comandi. Quella corretta è la seguente:

~~~c++
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
~~~

Negli appunti ritrovo che la sequenza

~~~c++
    setDataBusMode(INPUT);
    enableChip();
    byte value = readDataBus();
~~~

causava errori in lettura, mentre spostando **enableChip** prima di **setDataBusMode**, oppure aggiungendo 2uS di ritardo, riuscivo a leggere correttamente il contenuto della EEPROM. Alla fine avevo realizzato che fosse preferibile aggiungere i 2uS dopo aver abilitato il chip come ultimo step, anziché abilitare il chip e poi settare il data bus in input, perché nella seguente condizione si sarebbe potuto generare un cortocircuito:

- **enableChip()**: la EEPROM (che non ha resistenze in uscita) presenta uno o più output LO
- **setDataBusMode(INPUT)**: Arduino presenta uno o più output HI, che vengono dunque cortocircuitati verso ground dagli output LO della EEPROM.

## Link utili

- Il video <a href="https://www.youtube.com/watch?v=BA12Z7gQ4P0" target="_blank">Using an EEPROM to replace combinational logic</a> di Ben Eater, che descrive la programmazione manuale di una EEPROM.
- Il video <a href="https://www.youtube.com/watch?v=K88pgWhEb1M" target="_blank">Build an Arduino EEPROM programmer</a> e il repository GitHub <a href="https://github.com/beneater/eeprom-programmer" target="_blank">Arduino EEPROM programmer</a> di Ben Eater.
- Il programmatore di EEPROM <a href="https://github.com/TomNisbet/TommyPROM" target="_blank">TommyProm</a> e lo <a href="https://tomnisbet.github.io/nqsap/docs/microcode/" target="_blank">sketch semplificato</a> necessario alla sola programmazione delle EEPROM dell'NQSAP di Tom Nisbet, che ho studiato per sviluppare il codice del programmatore di EEPROM del BEAM.

## TO DO

- Aggiungere informazioni sulla creazione delle istruzioni con buildInstruction
