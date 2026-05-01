---
title: "Flag"
permalink: /docs/flags/
excerpt: "Modulo dei Flag del computer BEAM"
---
<small>[Componenti e funzionamento](#componenti-e-funzionamento) - [I salti condizionali e incondizionati](#i-salti-condizionali-e-incondizionati) - [Calcolo dei Flag N, V, Z e C](#calcolo-dei-flag-n-v-z-e-c) - [Il Carry e i registri H e ALU](#il-carry-e-i-registri-h-e-alu) - [Schema](#schema) - [Differenze tra Moduli Flag dell’NQSAP e del BEAM](#differenze-tra-moduli-flag-dellnqsap-e-del-beam) - [Link utili](#link-utili)</small>

[![Modulo dei Flag del computer BEAM](../../assets/flags/30-flag-beam.png "Modulo dei Flag del computer BEAM"){:width="100%"}](../../assets/flags/30-flag-beam.png)

Più volte, analizzando l'NQSAP e leggendo la documentazione di Tom Nisbet, avevo trovato delle idee molto ingegnose; alcune di queste si trovano nel modulo dei Flag.

[![Schema del modulo dei Flag di Tom Nisbet, leggermente modificato al solo scopo di migliorarne la leggibilità](../../assets/flags/30-flag-nqsap-schematics.png "Schema del modulo dei Flag di Tom Nisbet, leggermente modificato al solo scopo di migliorarne la leggibilità"){:width="100%"}](../../assets/flags/30-flag-nqsap-schematics.png)

*Schema del modulo dei Flag di Tom Nisbet, leggermente modificato al solo scopo di migliorarne la leggibilità.*

Il registro dei Flag dell'NQSAP emula i 4 flag **NVZC** del 6502:

- **N**egative (N)
- O**V**erflow (V)
- **Z**ero (Z)
- **C**arry (C)

E' completamente differente dal semplice registro dei Flag del computer SAP-1 di Ben Eater, nel quale un unico registro tipo D <a href="https://www.ti.com/lit/ds/sdls067a/sdls067a.pdf" target="_blank">74LS173</a> memorizzava i soli 2 flag C e Z nello stesso momento: la gestione delle istruzioni necessitava di 4 set di microcode, cioè uno per ogni combinazione dei segnali di flag portati agli ingressi delle EEPROM; ogni set di microcode era infatti sviluppato "su misura" per attivare in output i corretti segnali per la gestione di C e/o Z. Questo è ben spiegato nel video di Ben Eater <a href="https://www.youtube.com/watch?v=Zg1NdPKoosU" target="_blank">Conditional jump instructions</a>.

L'approccio di Tom era invece basato su una verifica logica eseguita in hardware: il microcode non variava a seconda dello stato dei flag, che non erano più direttamente connessi agli indirizzi delle ROM che attivano poi a loro volta diversi segnali di output in base all'indirizzo/flag presentato in ingresso!

Analizzando ad esempio un'istruzione di salto condizionale legata al flag Z riscontravo che:

- il microcode dell'istruzione di salto attivava un generico segnale "Jump Enable" connesso al pin 7 del Data Selector/Multiplexer <a href="https://www.ti.com/lit/ds/symlink/sn54s151.pdf" target="_blank">74LS151</a> visibile in basso a destra nello schema generale;

- la selezione del flag da mettere in uscita sul '151 dipendeva dalla codifica dell'istruzione in esecuzione, poiché i 3 bit Select S2, S1 ed S0 erano direttamente collegati all'Instruction Register, cioè *hardwired*, in maniera similare a quanto realizzato anche nel modulo ALU;

 [![Output dell'Instruction Register verso il modulo Flag con evidenza dei 3 bit di selezione dell'istruzione di salto condizionale](../../assets/flags/30-flag-cl-ir-out.png "Output dell'Instruction Register verso il modulo Flag con evidenza dei 3 bit di selezione dell'istruzione di salto condizionale"){:width="100%"}](../../assets/flags/30-flag-cl-ir-out.png)

*Output dell'Instruction Register verso il modulo Flag con evidenza dei 3 bit di selezione dell'istruzione di salto condizionale.*

[![Ingressi di selezione dell'istruzione di salto condizionale del registro dei Flag e connessione "hardwired" con l'IR](../../assets/flags/30-flag-select-in.png "Ingressi di selezione dell'istruzione di salto condizionale del registro dei Flag e connessione "hardwired" con l'IR"){:width="80%"}](../../assets/flags/30-flag-select-in.png)

*Ingressi di selezione dell'istruzione di salto condizionale del registro dei Flag e connessione "hardwired" con l'IR.*

Se ad esempio una generica istruzione *Jump on Zero* fosse codificata come 010 sui 3 segnali S2, S1 ed S0 comuni tra Instruction Register (IR) e registro dei Flag, si verificherebbe la seguente sequenza:

- il pin I2 di ingresso del '151 sarebbe attivato;
- in presenza del flag Z, l'uscita Q del Flip-Flop connessa al pin I2 avrebbe valore logico HI;
- l'uscita del '151 andrebbe ad abilitare il segnale /PC-LOAD sul Program Counter (PC) per saltare al nuovo indirizzo.

![Tabella funzioni Selector/Multiplexer 74LS151 con evidenza della ipotetica istruzione Jump on Zero](../../assets/flags/30-flag-151-table.png){:width="33%"}

*Tabella funzioni Selector/Multiplexer 74LS151 con evidenza della ipotetica istruzione Jump on Zero.*

Detto in altre parole: la logica del salto condizionale del SAP-1 era implementata nel microcode, utilizzando linee di indirizzamento delle ROM. Poiché i flag dell'NQSAP erano invece implementati in hardware, non c'era bisogno di consumare preziose linee di indirizzamento delle EEPROM (e di raddoppiare la dimensione del microcode ad ogni aggiunta di un flag).

I miglioramenti derivanti da questa architettura sono:

- possibilità di settare i flag anche singolarmente;
- risparmio di linee di indirizzamento delle EEPROM;
- l'aggiornamento dei flag non modifica gli indirizzi delle EEPROM nel bel mezzo dell'esecuzione di un'istruzione ed elimina questa possibile causa di [glitching](../control/#il-clock-il-glitching-delle-eeprom-e-linstruction-register-parte-2).

## Componenti e funzionamento

Un multiplexer (MUX) <a href="https://www.ti.com/lit/ds/symlink/sn74ls157.pdf" target="_blank">74LS157</a> prende in input i valori dei flag V, Z e C selezionandone la provenienza:

1. **dal bus**; quando il '157 legge dal bus, è possibile caricare i registri dei flag leggendo valori arbitrari dalla memoria del computer (o, più precisamente, dalla zona di memoria adibita allo Stack) similarmente a quanto svolto dall'istruzione Pull Processor Status **PLP** del 6502;

2. **da un computo**:
    - **V** attraverso un Data Selector/Multiplexer '151 che ricrea la funzione logica dell'Overflow verificando un eventuale cambio di segno nel risultato delle operazioni di somma o sottrazione dei numeri con segno (Signed);
    - **Z** come risultato del comparatore <a href="https://www.ti.com/lit/ds/symlink/sn74ls688.pdf" target="_blank">74LS688</a>;
    - **C** attraverso un altro '151 che seleziona la sorgente del Carry;

Il segnale di controllo **FS** a livello LO permette il caricamento dei Flag dal bus; viceversa, il caricamento dei Flag computati richiede che FS sia attivo.

V, Z e C in output dal MUX '157 sono presentati a 3 dei 4 Flip-Flop disponibili in una coppia di <a href="https://www.ti.com/lit/ds/symlink/sn54ls74a.pdf" target="_blank">74LS74</a>.

Il flag **N**egative viene invece sempre letto direttamente dalla linea D7 del bus e caricato sul 4° Flip-Flop.

Quattro porte AND permettono il caricamento dei FF in presenza del segnale di clock e della contemporanea attivazione degli opportuni segnali **FN**, **FV**, **FZ** ed **FC** provenienti dalla Control Logic (CL); è opportuno ricordare che il caricamento dei registri viene sempre effettuato in corrispondenza del Rising Edge del Clock.

Ogni istruzione del computer, grazie alla personalizzazione del microcode, può settare anche più di un flag alla volta (come accade ad esempio per le operazioni ADC e SBC, che sul 6502 influiscono contemporaneamente su tutti i 4 flag **NVZC**).

![Caricamento dei Flip-Flop C e Z con segnali Clock AND FC / FZ](../../assets/flags/30-flag-ff-cz.png){:width="50%"}

*Caricamento dei Flip-Flop C e Z con segnali Clock AND FC / FZ.*

- Si noti che i FF non vengono mai pre-settati, pertanto /Preset resta fisso a Vcc (e dunque mai attivo); è presente invece una connessione al segnale di reset generale del computer (/RST).

- Le uscite dei FF sono connesse a un '151 per gestire i salti condizionali.

- Il flag C viene esportato anche verso i '181 e il registro H del modulo ALU; si veda la sezione [Il Carry e i registri H e ALU](#il-carry-e-i-registri-h-e-alu) in questa stessa pagina.

Un bus transceiver <a href="https://www.mouser.com/datasheet/2/308/74LS245-1190460.pdf" target="_blank">74LS245</a> permette infine di esportare i 4 flag NVZC sul bus per salvarli in memoria, o più precisamente nello Stack, similarmente a quanto accade nel 6502 con l'istruzione Push Processor Status (PHP).

Interessante notare che le istruzioni CLC, CLV e SEC non hanno bisogno di segnali dedicati della CL per azzerare o settare i flag C e V: si utilizza la ALU per mettere 0x00 o 0xFF sul bus e si modifica il solo flag di interesse attivando opportunamente il segnale di controllo FC o FV.

## I salti condizionali e incondizionati

Ogni variazione di un flag nel computer SAP-1 di Ben Eater generava una variazione degli indirizzi delle EEPROM, così da poter attivare segnali in uscita opportunamente diversi in conseguenza delle diverse combinazioni degli stati dei flag.

L'approccio dell'NQSAP è molto diverso, in quanto tutti i segnali dei flag (presenti sulle uscite Q e /Q dei FF '74) vengono presentati a un '151; la sua funzione è quella di selezionare uno dei flag da mettere sulla sua uscita per eventualmente permettere l'attivazione del segnale /PC-LOAD, che abilita il caricamento del contenuto del bus sul Program Counter (e dunque i salti). E' sufficiente infatti caricare un nuovo valore nel Program Counter (PC) perché questo diventi il nuovo indirizzo a partire dal quale saranno caricate ed eseguite (fetch/execute) le successive istruzioni del programma caricato nella memoria del computer.

Come avviene la selezione del flag da portare all'uscita Z del '151? I segnali IR-Q5 (S0), IR-Q6 (S1) ed IR-Q7 (S2) provenienti dall'IR selezionano quale tra gli input del '151 (I0-I7) si debba portare sull'output Z, come visto in precedenza nella tabella delle funzioni del Selector/Multiplexer 74LS151. I segnali IR-Q5, Q6 e Q7 sono infatti *hardwired* con l'IR: le istruzioni di branch, ognuna con la sua codifica specifica, determinano quale ingresso del '151 sarà attivato ed esposto in output; a seconda dello stato del flag esposto, /PC-LOAD sarà eventualmente attivo e l'istruzione di salto relativo potrà essere eseguita.

Prendiamo come esempio l'istruzione BCS (Branch on Carry Set) ipotizzando che l'istruzione precedente abbia generato un Carry e che dunque il corrispondente FF presenti lo stato logico HI sull'uscita Q:

[![Esempio istruzione Branch on Carry Set](../../assets/flags/30-flag-bcs.png "Esempio istruzione Branch on Carry Set"){:width="66%"}](../../assets/flags/30-flag-bcs.png)

*Esempio istruzione Branch on Carry Set.*

- l'istruzione sarà codificata nel microcode in modo che S0, S1 ed S2 selezionino l'ingresso I0, che è collegato al FF del Carry:

  - S0 = LO
  - S1 = LO
  - S2 = LO

- poiché il Carry è attivo, l'output della NOR connessa all'uscita Z è certamente LO (/PC-LOAD = NOT (1+x) = 0), pertanto il valore presente nel bus viene caricato nel PC (il segnale /PC-LOAD è attivo LO).

Un aspetto che inizialmente sfuggiva alla mia comprensione era come poter includere le istruzioni di branch (8 combinazioni = 3 bit), le istruzioni dell'ALU (5 bit) e tutte le altre istruzioni (ad esempio caricamento, trasferimento) in soli 8 bit: come era possibile gestire tutte le combinazioni e costruire una matrice di istruzioni? Nel momento in cui dovevo eseguire una qualsiasi altra istruzione, cosa sarebbe successo nella gestione dei salti relativi, visto che erano direttamente funzione della codifica dell'istruzione correntemente in esecuzione? Non rischiavo di eseguire un salto condizionale non voluto se mi fossi trovato ad esempio in una situazione in cui (come descritto poco sopra) IR-Q5, Q6 e Q7 fossero LO e il Carry fosse attivo?

In seguito avevo notato che nel '151 addetto alla selezione dei flag era presente anche un segnale di Enable JE, presente nel microcode delle sole istruzioni di salto condizionale, pertanto:

- se JE è HI **e** se l'output del FF selezionato è HI, l'uscita del '151 sarà essa stessa HI, permettendo il caricamento del Program Counter;

- se JE è LO, l'uscita del '151 sarà disattivata, pertanto nessun segnale di caricamento verrà inviato al Program Counter.

Dunque le istruzioni - e solo quelle - il cui microcode comprendeva il segnale JE erano considerate istruzioni di salto condizionale; tutte le altre ignoravano il salto, in quanto il segnale /PC-LOAD non sarebbe mai stato attivato.

Tom evidenziava che "*questo metodo semplifica il microcode, perché tutte le operazioni di salto utilizzeranno lo stesso microcode*".

Perché tutte le istruzioni di salto dovrebbero essere "uguali"? La spiegazione, semplice solo dopo averla ben compresa, sta nel fatto che la scelta del flag da utilizzare per il salto condizionale non dipende più dal microcode dell'istruzione, bensì dalla codifica dell'istruzione stessa: essendo *hardwired* con l'IR, i segnali IR-Q5, Q6 e Q7 vengono automaticamente applicati agli ingressi Select del '151; è nella realizzazione del set di istruzioni che si deve tenere conto di quale codifica associare alle varie istruzioni BCS, BCC, BNE, BPL eccetera. Nessuna variazione è richiesta nel microcode.

Prendiamo come ulteriore esempio l'istruzione BVC (Branch on OVerflow Clear) ipotizzando che non ci sia Overflow e che dunque il segnale /V sia attivo:

[![Esempio istruzione Branch on OVerflow Clear](../../assets/flags/30-flag-bvc.png "Esempio istruzione Branch on OVerflow Clear"){:width="66%"}](../../assets/flags/30-flag-bvc.png)

*Esempio istruzione Branch on OVerflow Clear.*

- l'istruzione dovrà essere codificata nel microcode in modo che S0, S1 ed S2 selezionino l'ingresso I5, che è collegato all'uscita /Q del FF dell'Overflow:

  - S0 = HI
  - S1 = LO
  - S2 = HI

Tenendo ora in considerazione l'esistenza del segnale Jump Enable (JE), evidenziato in giallo, si evince che l'attivazione di /PC-LOAD e una conseguente esecuzione condizionale di un salto determinata dalla eventuale assenza di Overflow richiedono un'istruzione:

- la cui codifica porti ad avere S2/S1/S0 = 101 agli ingressi di selezione del '151, **e**
- il cui microcode attivi il segnale JE.

A questo punto della spiegazione si sarà notato che i FF '74 espongono i flag sia in logica normale (Q), sia in logica invertita (/Q): questo risulta molto comodo per determinare la presenza o assenza di un determinato flag. Ricorrendo a un altro esempio, se il Carry non fosse presente e si desiderasse eseguire un salto verificando la condizione "Carry non presente" (Branch on Carry Clear, BCC), si può verificare se /Q sia HI, così da attivare opportunamente il segnale di salto /PC-LOAD.

L'utilizzo di una NOR all'uscita Z del '151 permette di gestire sia i salti condizionali (dunque da validare con una apposita verifica logica, cioè quella della presenza/assenza di un determinato flag), sia i salti incondizionati:

- In caso di salto condizionale (BCS, BVC, BEQ eccetera), una verifica positiva di presenza/assenza del flag selezionato (normale o invertito) genera un'uscita HI sul '151 --> la NOR presenta dunque output LO ed attiva il caricamento dell'indirizzo del salto sul Program Counter. La pagina dei registri [D, X e Y](../dxy/#utilizzo-per-i-salti-condizionali) descrive il calcolo del valore da inserire nel PC.

- In caso di salto incondizionato (JMP, JSR), il microcode dell'istruzione di salto attiva /WP (in logica invertita) che a sua volta attiva il caricamento sul PC del valore presente sul bus: (/PC-LOAD = NOT (x+1) = 0), pertanto il PC carica dal bus il suo nuovo valore.

![NOR per l'attivazione di /PC-LOAD con salti condizionali ed incondizionati](../../assets/flags/30-flag-je-wp.png){:width="50%"}

*NOR per l'attivazione di /PC-LOAD con salti condizionali ed incondizionati.*

In definitiva, il microcode delle istruzioni di salto prevede:

- l'attivazione di JE per eseguire i salti condizionali;
- l'attivazione di WP per eseguire i salti incondizionati.

## Calcolo dei Flag N, V, Z e C

### Negative

Come detto precedentemente, il flag **N**egative è ricavato dal 7° bit del bus, cioè l'MSB: il flag viene gestito sulla linea D7 del bus in quanto i numeri Signed utilizzano proprio il Most Significant Bit (MSB) per indicare il proprio segno. Interessante notare che, essendo N mappato sul bus e non direttamente sull'ALU, è possibile rilevare un numero Negative anche in contesti esterni ai '181, ad esempio nel risultato di una rotazione fatta con lo Shift-Register o in un trasferimento di dato da un registro a un altro: tutto ciò che transita sul bus può essere oggetto di verifica del suo stato di numero Signed positivo o negativo.

### Zero

Il flag **Z**ero è attivo quando il valore presente nel bus è zero; anziché usare una serie di porte logiche NOR e AND per verificare se tutte le linee sono LO (come accadeva nel SAP), un singolo comparatore '688 può svolgere la stessa funzione. Notare che anche questo flag opera sul bus e non sui risultati della sola ALU.

![Comparatore 74LS688 per verifica dello stato zero sul bus](../../assets/flags/30-flag-688.png){:width="50%"}

*Comparatore 74LS688 per verifica dello stato zero sul bus.*

### Overflow

Il flag O**V**erflow è calcolato utilizzando un '151 nella modalità descritta in <a href="http://6502.org/users/dieter/v_flag/v_4.htm" target="_blank">74181 with V_Flag</a> sul sito 6502.org.

Avevo trovato la spiegazione molto criptica, o forse non propriamente adatta ai profani, tanto da impiegare alcune *decine* di ore per comprendere a fondo quanto enunciato rileggendo, cercando altre fonti, seguendo video di aritmetica binaria, facendo esercizi su carta e su uno spreadsheet.

Tom evidenziava che gli MSB degli operandi dell'ALU H e B, insieme all'MSB risultante dall'operazione dell'ALU, erano utilizzati come input per verificare la condizione di Overflow: iniziavo a realizzare che quello dell'Overflow era in realtà un calcolo molto semplice e preciso di bit.

In seguito avevo capito che il calcolo dell'Overflow è strettamente legato all'utilizzo di numeri [Signed](../math/#numeri-unsigned-e-numeri-signed): questi numeri vengono rappresentati con il **Complemento di 2** (Two's Complement, o anche 2C), nel quale un MSB = LO indica un numero positivo, mentre un MSB = HI indica un numero negativo.

In una delle innumerevoli sessioni di approfondimento e studio, ero finalmente arrivato a comprendere che se nella somma di due numeri con segno si nota un imprevisto cambiamento di segno del risultato, si ha una situazione di Overflow: il cambiamento di segno è rappresentato da una variazione dell'MSB del risultato, cosa che un '151 opportunamente connesso permette di identificare.

![Utilizzo di un 74LS151 per il calcolo dell'Overflow con evidenza degli MSB di H, B e dell'ALU e degli ingressi di selezione dell'operazione IR-Q1 e IR-Q3.](../../assets/flags/30-flag-v-151.png){:width="50%"}

*Utilizzo di un 74LS151 per il calcolo dell'Overflow con evidenza degli MSB di H, B e dell'ALU e degli ingressi di selezione dell'operazione IR-Q1 e IR-Q3.*

Per identificare l'esecuzione di un'operazione di addizione o di sottrazione e dunque selezionare quale debba essere l'ingresso corretto del '151 da attivare, si utilizzano due delle linee di selezione dell'operazione dell'ALU, in particolar modo:

| IR-Q1 | IR-Q3 | Operazione  |
| -     | -     | -           |
| HI    | LO    | Sottrazione |
| LO    | HI    | Addizione   |

Anticipo brevemente un altro aspetto esposto in maggior dettaglio nella sezione dedicata ai [numeri senza segno e con segno](../math/#numeri-unsigned-e-numeri-signed): in un byte sono possibili 256 combinazioni; in caso di numeri senza segno (Unsigned) è possibile contare da 0 a 255. Nel caso di numeri Signed, i numeri positivi da 0 a 127 hanno un riferimento paritetico con i numeri senza segno da 0 a 127, mentre i numeri con segno da -128 a -1 fanno il paio con i numeri senza segno da 128 a 255.

Nella pagina dedicata all'Aritmetica Binaria è presente anche una approfondita analisi dell'[Overflow](../math/#approfondimento-overflow) e dell'utilizzo di un 74LS151 per la gestione dei salti condizionali.

### Carry

Il registro dei Flag include un registro dedicato al **C**arry. L'NQSAP include diverse  operazioni che possono generare un Carry:

- per i calcoli aritmetici il Carry corrisponde al Carry Output dell'ALU '181
- per le operazioni di scorrimento e rotazione, il Carry è tratto dal Least Significant Bit (LSB) (pin H-Q0) o dal MSB (pin H-Q7) del registro H.

L'utilizzo di un altro '151 rappresenta il sistema più efficiente per selezionare la sorgente del Carry. A seconda dell'istruzione in esecuzione, il microcode di quella istruzione provvederà infatti ad attivare opportunamente i segnali C0 e C1:

![Utilizzo di un 74LS151 per la selezione del Carry da memorizzare nel Carry flag](../../assets/flags/30-flag-c-151.png){:width="50%"}

*Utilizzo di un 74LS151 per la selezione del Carry da memorizzare nel Carry flag.*

| C1 | C0 | Selezione del Carry                                          |
| -  | -  | -                                                            |
| LO | LO | Provenienza dal Carry Output dell'ALU (non invertito\*)      |
| LO | HI | Provenienza dal Carry Output dell'ALU (invertito\*\*)        |
| HI | LO | Provenienza dall'MSB (H-Q7) del registro H                   |
| HI | HI | Provenienza dall'LSB (H-Q0) del registro H                   |

- \* questa configurazione non viene utilizzata

- \*\* Come già discusso nella pagina dell'ALU, il Carry del '181 lavora in logica negativa, pertanto un segnale C = LO indica che il Carry è presente; va da sé che per registrare lo stato del Carry in logica positiva sul registro del flag C è necessario invertire il segnale in ingresso.

## Il Carry e i registri H e ALU

Oltre ad essere utilizzato per eseguire salti condizionali, il Carry trova chiaramente uso nel [modulo ALU](../alu/#lalu-dellnqsap) per eseguire operazioni aritmetiche ('181) e di scorrimento e rotazione ('194).

![Selezione del Carry da passare al Carry Input di H e dei '181 del modulo ALU](../../assets/flags/30-flag-c-h-alu.png){:width="50%"}

*Selezione del Carry da passare al Carry Input di H e dei '181 del modulo ALU.*

L'opportuna programmazione dei segnali **CC** (**C**arry **C**lear) e **CS** (**C**arry **S**et) nel microcode può di volta in volta passare al Carry Input di H (**H-Cin**) e dei '181 (**ALU-Cin**):

- un valore *hard-coded* 0
- un valore *hard-coded* 1
- il valore realmente presente nel registro del flag C

La necessità di inviare al modulo ALU non solo il valore reale del flag C, ma anche valori predefiniti 0 o 1, dipende da due fattori:

- Alcune operazioni aritmetiche del '181 richiedono uno specifico stato del Carry: ad esempio le operazioni A Minus 1 e A Plus B richiedono assenza del Carry in ingresso, mentre le operazioni A Plus 1 e A Minus B richiedono la sua presenza; il segnale inviato ai '181 è ALU-Cin.
- Le istruzioni ASL ed LSR (Arithmetic Shift Left e Logical Shift Right) eseguite dal '194 richiedono l'inserimento di uno 0 rispettivamente nell'LSB e nell'MSB di H; il segnale inviato ai '194 è H-Cin.

| CS | CC | Carry presentato al modulo ALU      |
| -  | -  | -                                   |
| LO | LO | Valore presente nel registro Flag C |
| LO | HI | LO                                  |
| HI | LO | HI                                  |
| HI | HI | Non usato                           |

La negazione del segnale inviato in ingresso al Carry Input del '181 deriva dal fatto che la configurazione utilizzata dall'ALU (logica attiva alta, “Active-High data”) richiede un segnale Carry In [invertito](../alu/#funzioni-logiche-e-operazioni-aritmetiche).

Si noti che la Truth Table della tabella richiederebbe i componenti evidenziati nello schema seguente, ma l'applicazione delle <a href="https://www.allaboutcircuits.com/textbook/digital/chpt-7/demorgans-theorems/" target="_blank">leggi di De Morgan</a> permette la semplificazione utilizzata poi nello schema adottato nell'NQSAP e nel BEAM.

![Schema originale per realizzazione della Truth Table di selezione del Carry](../../assets/flags/30-flag-c-h-alu-de-morgan.png){:width="50%"}

*Schema originale per realizzazione della Truth Table di selezione del Carry.*

![Schema rivisto con l'applicazione del teorema di De Morgan](../../assets/flags/30-flag-c-h-alu-de-morgan-nor.png){:width="50%"}

*Schema rivisto con l'applicazione del teorema di De Morgan.*

## Schema

[![Schema del modulo dei Flag del computer BEAM](../../assets/flags/30-flag-beam-schematics.png "Schema del modulo dei Flag del computer BEAM"){:width="100%"}](../../assets/flags/30-flag-beam-schematics.png)

*Schema del modulo dei Flag del computer BEAM.*

## Differenze tra Moduli Flag dell'NQSAP e del BEAM

Il modulo Flag del computer BEAM è sostanzialmente una copia del modulo Flag del computer NQSAP; con le mie conoscenze non avrei saputo svilupparne una versione migliore, ma il fatto di averne compreso perfettamente tanto il funzionamento quanto l'integrazione con gli altri moduli del computer è stata una ottima lezione.

## Link utili

- I video di Ben Eater <a href="https://www.youtube.com/watch?v=ObnosznZvHY" target="_blank">CPU flags register</a> e <a href="https://www.youtube.com/watch?v=Zg1NdPKoosU" target="_blank">Conditional jump instructions</a>, che spiegano la costruzione del modulo dei Flag e le modifiche necessarie al microcode per la gestione delle istruzioni di salto condizionale. Si noterà la differenza con l'approccio dell'NQSAP, che non richiede microcode ad-hoc per ogni flag e non abbisogna di linee di indirizzamento EEPROM dedicate.

- Tom segnala di aver preso ispirazione dal thread Reddit <a href="https://www.reddit.com/r/beneater/comments/jwxke0/how_to_add_a_decremental_and_incremental_circuit/" target="_blank">How to add a decremental and incremental circuit to the ALU ?</a> per l'idea di pilotare il caricamento del [Program Counter](../programcounter/) dal registro dei Flag anziché gestirli con copie multiple del microcode come avveniva come sul SAP-1 di Ben Eater.

- Inoltre, anche l'ispirazione per la realizzazione del Flag V deriva da un altro thread su Reddit, <a href="https://www.reddit.com/r/beneater/comments/kmuuex/question_for_all_74ls181_alu_people" target="_blank">Question for all 74ls181 alu people</a>; in particolar modo, l'utente <a href="https://www.reddit.com/user/SaltPeppah/" target="_blank">SaltPeppah</a> suggeriva l'uso del 74LS151 segnalando il link indicato nella sezione [Overflow](#overflow) della pagina dei Flag e nella sezione [Approfondimento Overlow](../math/#approfondimento-overflow) della pagina dedicata all'aritmetica binaria, dove l'argomento viene trattato per esteso.

- Tom notava anche l'approccio del thread <a href="https://www.reddit.com/r/beneater/comments/m76ijz/opcodes_and_flag_decoding_circuit/" target="_blank">Opcodes and Flag decoding circuit</a> per eseguire salti condizionali in hardware. Invece di pilotare la linea LOAD del Program Counter, il circuito dell'autore del thread si trova tra l'IR e la EEPROM e forza condizionatamente un'istruzione NOP o JMP a seconda dello stato dei flag. Gli opcode delle istruzioni di salto sono disposti in modo tale che il flag di interesse possa essere determinato dai bit in uscita dall'IR. Concetto interessante, ma Tom aveva già implementato una funzionalità simile con le linee di selezione dell'ALU hardwired all'IR, modalità utilizzata anche nella gestione dei [salti condizionali](#i-salti-condizionali-e-incondizionati).
