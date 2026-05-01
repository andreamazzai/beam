---
title: "Control Logic"
permalink: /docs/it/control/
excerpt: "Control Logic del computer BEAM"
---
<small>[Instruction Register e Istruzioni](#instruction-register-e-istruzioni) - [Ring Counter e Microistruzioni](#ring-counter-e-microistruzioni) - [Fasi](#fasi) - [Clock, "glitching" delle EEPROM e Instruction Register (parte 2)](#clock-glitching-delle-eeprom-e-instruction-register-parte-2) - [Lunghezza delle istruzioni](#lunghezza-delle-istruzioni) - [I 74LS138 per la gestione dei segnali](#i-74ls138-per-la-gestione-dei-segnali) - [Caricamento di un programma dal Loader](#caricamento-di-un-programma-dal-loader) - [Riepilogo segnali dell’NQSAP e del BEAM](#riepilogo-segnali-dellnqsap-e-del-beam) - [Segnali di controllo](#segnali-di-controllo) - [Bus e altri segnali](#bus-e-altri-segnali) - [Microcode](#microcode) - [Differenze rispetto all’Instruction Set del 6502](#differenze-rispetto-allinstruction-set-del-6502) - [Schema](#schema) - [Differenze tra Control Logic dell’NQSAP e del BEAM](#differenze-tra-control-logic-dellnqsap-e-del-beam) - [Note](#note) - [Link Utili](#link-utili) - [Riflessione sul microcode](#riflessione-sul-microcode)</small>

[![Control Logic del computer BEAM](../../assets/control/40-beam-control.png "Control Logic del computer BEAM"){:width="100%"}](../../assets/control/40-beam-control.png)

In generale, la gestione delle istruzioni è affidata alla Control Logic, che consta di tre capisaldi: Instruction Register, Ring Counter e Microcode. L'Instruction Register contiene l'istruzione in esecuzione, il Ring Counter tiene traccia delle microistruzioni che compongono l'istruzione e il Microcode definisce i segnali di controllo necessari per eseguire le microistruzioni.

Questa pagina descrive le Control Logic dell'NQSAP e del BEAM, evidenzia alcune differenze con la Control Logic del SAP-1 di Ben Eater e approfondisce gli argomenti che avevo trovato più ostici o più interessanti.

Per facilità di consultazione e semplificazione del confronto fra i tre computer SAP, NQSAP e BEAM, è opportuno riepilogare in tabella alcuni degli aspetti ricorrenti nel testo.

| ↓ ↓ Caratteristica / Sistema → →       | SAP-1       | NQSAP      | BEAM          |
| -                                      | -         | -          | -             |
| Autore                                 | Ben Eater | Tom Nisbet | Andrea Mazzai |
| IR condiviso tra Opcode e Operando     | Sì        | No         | No            |
| Bit IR per Opcode                      | 4         | 8          | 8             |
| Bit IR per Operando                    | 4         | 0          | 0             |
| Ampiezza bus da RC a EEPROM (bit)      | 3         | 3          | 4             |
| Numero massimo Step (RC)               | 5         | 8          | 16            |
| Ampiezza bus da IR a EEPROM  (bit)     | 4         | 8          | 8             |
| Numero massimo Istruzioni (IR)         | 16        | 256        | 256           |
| Istruzioni implementate nel Microcode  | ~ 10      | 120        | > 110         |
| Lunghezza Istruzioni variabile         | No        | Sì         | Sì            |
| IR bufferizzato                        | No        | No         | Sì            |
| Caricamento IR a Rising o Falling Edge | Rising    | Rising     | Falling       |
| Caricamento RC a Rising o Falling Edge | Falling   | Falling    | Falling       |
| EEPROM                                 | 2x 28C16  | 4x 28C256  | 4x 28C256     |
| EEPROM (Kb)                            | 2x 16     | 4x 256     | 4x 256        |
| Dimensione Control Word (bit)          | 16        | 32         | 32            |
| RAM (byte)                             | 16        | 256        | 256           |

Legenda: IR = Instruction Register; RC = Ring Counter

Alcune note propedeutiche:

1. Nel computer SAP-1 di Ben Eater, la denominazione dei segnali di controllo è "modulo-centrica", riflettendo la funzione specifica di ciascun modulo: ad esempio, il segnale RO (RAM Out) esporta il contenuto della RAM sul bus, mentre AI (A Input) carica il registro A. Nel computer NQSAP di Tom Nisbet e nel BEAM, invece, la nomenclatura è "computer-centrica", adottando un punto di vista a livello di bus: per esempio, RO diventa RR (RAM Read) e AI diventa WA (Write A).

2. Nell'NQSAP e nel BEAM l'Instruction Register (IR) è incluso nello schema della Control Logic, mentre negli schemi del SAP-1 stava su un foglio separato.

[![Schema della Control Logic dell'NQSAP](../../assets/control/40-control-logic-schema-nqsap.png "Schema della Control Logic dell'NQSAP"){:width="100%"}](../../assets/control/40-control-logic-schema-nqsap.png)

*Schema della Control Logic dell'NQSAP, leggermente modificato al solo scopo di migliorarne la leggibilità.*

## Instruction Register e Istruzioni

Il ruolo dell'Instruction Register è di memorizzare l'istruzione corrente prelevandola dalla memoria.

L'Instruction Register del SAP-1 presentava una dimensione di un byte, all'interno del quale erano contenuti sia l'istruzione che l'operando:

- i 4 bit più significativi erano dedicati all'istruzione;
- i 4 bit meno significativi erano riservati a un operando o a un indirizzo opzionali.

Se i bit meno significativi contenevano un operando (ad esempio, un valore immediato da utilizzare in un'operazione aritmetica), questo valore veniva caricato in un registro per l'esecuzione dell'istruzione; se i bit meno significativi contenevano un indirizzo di memoria, questo indirizzo veniva caricato nel Memory Address Register (MAR), che puntava così alla posizione di memoria da cui leggere o scrivere dati.

Nell'immagine seguente, tratta dal video <a href="https://youtu.be/JUVt_KYAp-I?t=1837" target="_blank">Reprogramming CPU microcode with an Arduino</a> di Ben Eater, si vede come ogni byte di un semplice programma di somma e sottrazione includa sia l'operazione sia l'operando:

![Somma e sottrazione nel SAP](../../assets/control/40-lda-15-add-14.png "Somma e sottrazione nel SAP"){:width="50%"}

Ad esempio:

- L'istruzione LDA 15 all'indirizzo di memoria 0000 è composta dai 4 bit più significativi (MSB) 0001 (che nel microcode definiscono un'operazione di caricamento accumulatore) e dai 4 bit meno significativi (LSB) 1111, che indicano l'indirizzo di memoria 15, nel quale è presente il valore 5 da caricare nell'accumulatore A.
- L'istruzione ADD 14 all'indirizzo di memoria 0001 è composta dai 4 bit MSB 0010 (che nel microcode definiscono un'operazione di somma) e dai 4 bit LSB 1110, che indicano l'indirizzo di memoria 14, nel quale è presente il valore 6 da sommare al valore già presente nell'accumulatore A.
- L'istruzione OUT non necessita di operando: espone il contenuto di A sul modulo di Output, pertanto i 4 LSB sono irrilevanti.

| Mnemonico | Indirizzo | Istruzione.Operando |
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

*Rappresentazione di un programma di somma, sottrazione e output caricato nei 16 byte della memoria del SAP.*

In conseguenza del numero di bit utilizzato per l'istruzione, la connessione tra Instruction Register del SAP-1 ed EEPROM contenenti il microcode poteva avere una ampiezza di soli 4 bit, come visibile in figura:

[![Schema della Control Logic e dell'Instruction Register del SAP](../../assets/control/40-control-logic-schema-SAP.png "Schema della Control Logic e dell'Instruction Register del SAP"){:width="100%"}](../../assets/control/40-control-logic-schema-SAP.png)

*Schema della Control Logic e dell'Instruction Register del SAP.*

Una fondamentale differenza tra l'IR del SAP-1 e quello dell'NQSAP e del BEAM è la dimensione. Il 6502 ha un set di istruzioni *relativamente* piccolo, composto da 56 istruzioni di base; tuttavia, queste istruzioni possono essere utilizzate con diverse modalità di <a href="https://www.masswerk.at/6502/6502_instruction_set.html#modes" target="_blank">indirizzamento</a>, il che porta il numero totale di combinazioni possibili a circa 150.

Per poter gestire queste combinazioni ed emulare così il set di istruzioni del 6502, la dimensione dell'opcode deve essere di un intero byte e l'architettura del sistema deve gestire istruzioni di lunghezza variabile:

- a un solo byte per quelle con indirizzamento Implicito e Accumulatore e che, dunque, non hanno un operando;
- a due o tre\* byte per tutte le altre, che fanno invece uso di un operando:
  - a due byte quando l'operando è un valore (indirizzamento Immediato e Relativo);
  - a due byte quando l'operando è un indirizzo di pagina zero (entro i primi 256 byte del computer);
  - a tre* byte quando l'operando è un indirizzo entro i 64K indirizzabili dal 6502.

\* Un computer con 256 byte di RAM non necessita di istruzioni a 3 byte, perché un operando della lunghezza di un singolo byte è in grado di indirizzare tutta la memoria del computer, come brevemente discusso anche nella sezione [Indirizzamenti](../alu/#indirizzamenti) della pagina dedicata all'ALU.

[![Schemi dell'Instruction Register dell'NQSAP e del BEAM](../../assets/control/40-cl-ir-beam-nqsap.png "Schemi dell'Instruction Register dell'NQSAP e del BEAM"){:width="100%"}](../../assets/control/40-cl-ir-beam-nqsap.png)

*Schemi dell'Instruction Register dell'NQSAP e del BEAM.*

Tirando le fila, per un computer come l'NQSAP o il BEAM:

- l'Instruction Register deve essere dedicato alle sole istruzioni ed avere dimensione di un byte;
- la connessione tra IR ed EEPROM deve avere un'ampiezza di 8 bit e non più di soli 4 bit come nel SAP;
- sono necessarie EEPROM con 13 (NQSAP, 2^13 = 64Kb) o 14 (BEAM, 2^14 = 128Kb) pin di indirizzamento:
  - 8 pin per le istruzioni (2^8 = 256 istruzioni);
  - 3 o 4 pin per le microistruzioni (NQSAP, 2^3 = 8 step; BEAM, 2^4 = 16 step), delle quali si parla nella sezione dedicata al [Ring Counter](#ring-counter-e-microistruzioni);
  - 2 pin per la selezione delle EEPROM.

Per l'NQSAP, Tom ha deciso di utilizzare comunque EEPROM da 256Kb anziché da 64Kb; il BEAM richiede invece obbligatoriamente EEPROM da 256Kb (non sono disponibili in commercio <a href="https://eu.mouser.com/c/semiconductors/memory-ics/eeprom/?interface%20type=Parallel" target="_blank">EEPROM da 128Kb con interfaccia parallela</a>).

Come si vedrà in seguito parlando del Ring Counter, un aspetto importante del caricamento dei registri è il [*momento*](#clock-glitching-delle-eeprom-e-instruction-register-parte-2) in cui vengono caricati: al Falling Edge\* del clock, oppure al Rising Edge\*: il caricamento dell'Instruction Register del SAP-1 e dell'NQSAP avviene al Rising Edge, mentre quello del BEAM avviene al Falling Edge.

\* Questa pagina utilizza sempre i termini Rising Edge e Falling Edge in riferimento al clock normale (CLK). Alcuni componenti ricevono un segnale di clock invertito (/CLK), che in alcuni grafici è rappresentato solo per evidenziare visivamente la fase del segnale effettivamente ricevuto.

Prima di approfondire l'argomento, è opportuno iniziare a parlare anche del Ring Counter, che ha un ruolo primario nel caricamento di tutti i registri, IR compreso.

## Ring Counter e Microistruzioni

Per capire il funzionamento del Ring Counter, è necessario fare proprio il concetto di microistruzione: le *istruzioni* di un microprocessore sono composte da un certo numero di step, più precisamente chiamati *microistruzioni*.

Infatti, ogni istruzione di un microprocessore (ad esempio, "carica un valore nel registro X", "incrementa il contenuto della locazione $E5" o "esegui uno scorrimento a destra dell'accumulatore") è composta da una sequenza di microistruzioni elementari, che corrispondono ai singoli passi (step) necessari per completare l'operazione voluta.

Il Ring Counter (RC) tiene traccia dello stato di avanzamento delle microistruzioni. Ogni stato del RC corrisponde a un particolare step nel ciclo di esecuzione di un'istruzione, quindi può essere visto come un meccanismo che avanza attraverso le diverse microistruzioni necessarie per eseguire un'istruzione completa della CPU.

Nel BEAM, ad esempio, l'istruzione LDA #$94 (che nel linguaggio mnemonico del 6502 si traduce in "carica nell'accumulatore il valore esadecimale $94") è composta dai seguenti quattro step / microistruzioni:

~~~text
| ---- | -------------------------- |
| Step | Microistruzione            |
| ---- | -------------------------- |
| 0*   | RPC | WM                   |
| 1*   | RR  | WIR | PCI            |
| 2    | RPC | WM                   |
| 3    | RR  | FNZ | WAH | PCI | NI |
| ---- | -------------------------- |
~~~

*Scomposizione dell'istruzione LDA Immediato nelle sue quattro microistruzioni elementari*.

1. Il primo step carica l'indirizzo dell'istruzione corrente nel Memory Address Register:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'istruzione nel MAR
2. Il secondo step carica l'opcode dell'istruzione nell'IR e incrementa il PC per farlo puntare alla locazione di memoria successiva (che nel caso dell'istruzione LDA contiene l'operando):
    - RR, Read RAM - espone sul bus l'opcode dell'istruzione
    - WIR, Write Instruction Register - carica l'opcode nell'IR\*
    - PCI, Program Counter Increment - incrementa il PC
3. Il terzo step carica nel Memory Address Register l'indirizzo del Program Counter, che ora punta all'operando:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'operando nel MAR
4. Il quarto ed ultimo step carica l'operando nell'accumulatore, incrementa il PC per farlo puntare alla istruzione successiva e resetta il Ring Counter
    - RR, Read RAM - espone l'operando sul bus
    - FNZ, Flag N & Z - aggiorna i Flag N e Z
    - WAH, Write A & H - carica l'operando in A e H\*\*
    - PCI, Program Counter Increment - incrementa il PC
    - NI, Next Instruction - resetta il Ring Counter\*\*\*

\* Non bisogna trascurare il fatto che i primi due step di *tutte* le istruzioni sono *sempre* identici. Alla fine del secondo step, l'Instruction Register contiene l'opcode dell'istruzione, che, insieme alle microistruzioni, definisce le operazioni che gli step successivi devono eseguire. Questo vale per qualsiasi istruzione, compresa la prima che una CPU esegue all'accensione. Prima di costruire il SAP-1 di Ben Eater, non riuscivo a immaginare quale meccanismo permettesse ad una CPU di sapere cosa dovesse fare una volta accesa; l'averlo compreso è stato piuttosto appagante.

\*\* Perché anche H? Si veda la sezione dedicata alla spiegazione del [registro H](../alu/#il-registro-h) nella pagina dell'ALU.

\*\*\* Approfondimenti in merito nella sezione [Lunghezza delle istruzioni](#lunghezza-delle-istruzioni) in questa stessa pagina.

Uno schema che mostra chiaramente gli step di alcune istruzioni del SAP-1 è visibile in questa immagine tratta dal video <a href="https://www.youtube.com/watch?v=dHWFpkGsxOs" target="_blank">8-bit CPU control logic: Part 3</a> di Ben Eater; gli step 000 e 001 sono comuni per tutte le istruzioni e compongono quella che viene chiamata **Fase Fetch**, evidenziata in giallo.

[![Microcode del SAP](../../assets/control/40-cl-ben-step-microcode.png "Microcode del SAP"){:width="100%"}](../../assets/control/40-cl-ben-step-microcode.png)

*Microcode del SAP.*

Per finalizzare l'analisi dell'istruzione LDA #$94, riepiloghiamo lo stato del computer alla fine del quarto step:

- il Flag Z non sarà attivo (il risultato dell'operazione di caricamento dell'accumulatore non è uguale a zero);
- il Flag N sarà attivo (secondo il metodo di [rappresentazione dei numeri Signed](../math/#numeri-unsigned-e-numeri-signed) a 8 bit in Complemento a 2, $94 / 1001.0100 è un numero negativo, in quanto il bit più significativo è allo stato logico 1);
- i Flag V e C non saranno modificati rispetto allo stato precedente;
- l'accumulatore A e il registro H conterranno il valore $94 esadecimale.

### Fasi

Per garantire il corretto funzionamento del processore, la Control Logic deve impostare la giusta *Control Word* per ogni microistruzione. La Control Word è quella stringa di bit utilizzata per governare e coordinare il comportamento dei vari componenti del processore durante l'esecuzione di una microistruzione ed è definita nel microcode memorizzato nelle EEPROM; ad ogni bit / pin di output delle EEPROM corrisponde un segnale di controllo (come RPC, WM, PCI, RR eccetera).

Le operazioni di una CPU passano per diverse fasi, che possiamo riassumere in:

1. "Fetch" (prelievo), che preleva l'istruzione dalla locazione di memoria puntata dal PC e la memorizza nell'IR.
2. "Decode" (decodifica), che interpreta il contenuto dell'IR per determinare quale istruzione debba essere eseguita.
3. "Execute" (esecuzione), che include tutte le microistruzioni che realizzano effettivamente quanto deve essere svolto dall'istruzione (ad esempio: "incrementa il registro X").

La fase di prelievo è stata accennata nella [sezione precedente](#ring-counter-e-microistruzioni) ed è fondamentale che le microistruzioni di questa fase siano identiche per tutte le istruzioni implementate.

Seguiamo passo dopo passo quanto accade nell'istruzione più semplice tra quelle implementate nel BEAM, la NOP - No Operation:

~~~text
| ---- | ---------------------|
| Step | Microistruzione      |
| ---- | ---------------------|
| 0    | RPC | WM             |
| 1    | RR  | WIR | PCI      |
| 2    | NI                   |
| ---- | ---------------------|
~~~

*Scomposizione dell'istruzione NOP nelle sue tre microistruzioni elementari*.

1. Il primo step carica l'indirizzo dell'istruzione corrente nel Memory Address Register:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'istruzione nel MAR
2. Il secondo step carica l'opcode dell'istruzione nell'IR e incrementa il PC per farlo puntare alla locazione di memoria successiva (che nel caso dell'istruzione NOP, lunga un solo byte, sarà la prossima istruzione):
    - RR, Read RAM - espone sul bus l'opcode dell'istruzione
    - WIR, Write Instruction Register - carica l'opcode nell'IR\*
    - PCI, Program Counter Increment - incrementa il PC
3. Il terzo step riporta il Ring Counter a 0:
    - NI, Next Instruction - resetta il Ring Counter

Nel primo step, il MAR viene caricato con il valore del PC. Il RC viene incrementato portandoci allo step successivo.

Nel secondo step l'IR viene caricato e il PC viene incrementato, puntando così alla prossima locazione di memoria. Notare che il nuovo valore del PC non influisce sull'istruzione correntemente in esecuzione, poiché il PC non indirizza le EEPROM contenenti il microcodice. Il RC viene incrementato portandoci allo step successivo.

Nel terzo step, il segnale di controllo NI riporta il RC al valore iniziale 0.

Inizia ora l'esecuzione della prossima istruzione, ma l'*IR contiene ancora l'opcode dell'istruzione NOP*: l'IR, infatti, non è stato modificato. Poiché i primi due step di tutte le istruzioni sono identici, non c'è alcun problema: anche se stiamo iniziando la prossima istruzione, eseguiamo i primi due step dell'istruzione NOP tuttora presente nell'IR.

In altre parole, con il reset del RC stiamo avviando l'esecuzione della prossima istruzione, ma il valore dell'IR non è ancora cambiato. Di conseguenza, i primi due step dell'istruzione successiva sono eseguiti utilizzando il microcode dell'istruzione precedente. È per questo motivo che è fondamentale che il microcode dei primi due step sia identico per tutte le istruzioni.

Ora, nel primo step della "prossima" istruzione, il valore aggiornato del PC viene messo nel MAR, ed è a questo punto che il nuovo valore del PC inizia a essere rilevante. Nel secondo step, l'istruzione viene caricata nell'IR e, da questo momento in poi, il computer inizia a eseguire gli step specifici di decodifica ed esecuzione della nuova istruzione.

La fase di decodifica avviene grazie al microcodice memorizzato nelle EEPROM: l'istruzione caricata nell'IR ha un proprio opcode specifico (ad esempio, 0100.0110), che viene presentato agli ingressi delle EEPROM assieme agli output del Ring Counter. Questa combinazione indirizza una locazione di memoria specifica nelle EEPROM, che emettono in uscita i bit della Control Word e che, a loro volta, attivano i segnali di controllo necessari per eseguire la microistruzione corrente.

Il legame tra decodifica ed esecuzione è molto stretto, perché in ogni momento la Control Word dipende sia dall'opcode (Decode), sia dalla microistruzione (Execute).

Si intuisce che una CPU deve conoscere in ogni momento quale istruzione sia attualmente in esecuzione (ne riceviamo indicazioni dall'Instruction Register) e quale sia lo step correntemente attivo, per conoscere il quale ci viene in aiuto il Ring Counter. SAP, NQSAP e BEAM sviluppano il Ring Counter attorno a un contatore <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">74LS161</a>, in grado di contare da 0 a 15, e a un demultiplexer <a href="https://www.ti.com/lit/ds/symlink/sn74ls138.pdf" target="_blank">74LS138</a>, che ci aiuta ad avere riscontro visivo della microistruzione in esecuzione.

Come detto poc’anzi, la combinazione dell'opcode contenuto nell’Instruction Register e dallo step fornito dal Ring Counter indirizza una locazione di memoria specifica nelle EEPROM: tale locazione di memoria contiene la Control Word.

[![Output di IR ed RC e input delle EEPROM del BEAM](../../assets/control/40-cl-ir-cr-beam.png "Output di IR ed RC e input delle EEPROM del BEAM"){:width="100%"}](../../assets/control/40-cl-ir-cr-beam.png)

*Output di IR ed RC e input delle EEPROM del BEAM*.

Nello schema si evidenzia che le uscite del Ring Counter controllano 4 indirizzi delle EEPROM, mentre altri 8 indirizzi sono controllati dall'Instruction Register.

Utilizzando una logica combinatoria, è possibile costruire il microcode da caricare nelle EEPROM, che emetteranno gli opportuni segnali di output (Control Word) per ogni step di ogni istruzione.

Nell'immagine si può osservare che le uscite del contatore controllano anche il demultiplexer, che viene utilizzato per visualizzare lo stato dell'RC. Anziché impiegare 16 LED (e due '138), un singolo LED "esteso" è pilotato dal pin più significativo del '161, che ha un valore pari ad 8: lo step correntemente in esecuzione sarà indicato dal LED acceso dal '138, al quale sommare 8 se il LED "esteso" è acceso.

### Clock, "glitching" delle EEPROM e Instruction Register (parte 2)

In generale, i momenti essenziali di un ciclo di clock in un computer sono due: il Rising Edge ↗ (passaggio del segnale dallo stato logico LO allo stato logico HI) e il Falling Edge ↘ (viceversa).

- Rising Edge: la maggior parte dei componenti sequenziali* (quali contatori, registri, flip-flop) modifica il proprio stato durante la transizione del segnale di clock dallo stato logico LO allo stato logico HI; le azioni di caricamento di tutti i moduli del computer (PC, MAR, RAM, A, B, H, Registri Indice, Flag, SP, O) avvengono in questo momento, con qualche eccezione.

- Falling Edge: poiché ad ogni Rising Edge i componenti sequenziali caricano i dati in ingresso, si intuisce che è necessario trovare un momento *antecedente*, durante il quale impostare la Control Word di tutti i moduli del sistema in modo che i dati siano presenti agli ingressi dei componenti col dovuto anticipo. Il Falling Edge si configura come il momento migliore per settare la Control Word; invertendo la fase del clock inviato al Ring Counter, si esegue la configurazione della Control Word - e dunque della microistruzione - proprio in corrispondenza del Falling Edge.

\* I componenti sequenziali producono un output che dipende non solo dagli ingressi attuali, ma anche dallo stato precedente, a differenza dei circuiti combinatori che dipendono esclusivamente dagli ingressi presenti.

Per quale motivo si parla di eccezioni? Sicuramente il Ring Counter è una di queste, per il motivo spiegato al punto precedente; l'Instruction Register *può* essere un'altra eccezione.

In effetti, nel SAP-1 il caricamento dell'IR è sincrono con il Rising Edge del clock:

[![Dettaglio Instruction Register del SAP](../../assets/control/40-cl-sap-ir-detail.png "Dettaglio Instruction Register del SAP"){:width="66%"}](../../assets/control/40-cl-sap-ir-detail.png)

Tale sincronia si ritrova anche nell'NQSAP:

[![Dettaglio Instruction Register dell'NQSAP](../../assets/control/40-cl-nqsap-ir-detail.png "Dettaglio Instruction Register dell'NQSAP"){:width="66%"}](../../assets/control/40-cl-nqsap-ir-detail.png)

Quali sono le possibili conseguenze del caricamento dell'IR al Rising Edge del clock?

Bisogna prendere in considerazione una proprietà delle EEPROM: quando l'indirizzo di ingresso cambia, le uscite possono diventare instabili, oscillando ("glitching", problema tecnico) tra gli stati logici prima di assestarsi definitivamente sul valore corretto. Se il fenomeno non viene gestito, possono verificarsi effetti collaterali indesiderati, quali impulsi di clock non voluti se si usano gate per gestire l'Enable nei chip che ne sono sprovvisti (come il [registro B](../alu/#lalu-dellnqsap) o i [registri D, X e Y](../dxy/) dell'NQSAP), oppure l'output contemporaneo di più moduli sul bus, generando contese e assorbimenti di corrente elevati.

Nelle EEPROM come la <a href="https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf" target="_blank">AT28C256</a>, il parametro che indica la durata dell'incertezza all'output è tipicamente chiamato "Address Access Time" o "t<sub>ACC</sub>" e indica il periodo che intercorre tra l'applicazione di un nuovo indirizzo di ingresso e il momento in cui i dati corretti sono disponibili sull'uscita, come visibile in figura:

[![AC Read Waveforms EEPROM AT28C256](../../assets/control/40-28C256-read-waveform.png "AC Read Waveforms EEPROM AT28C256"){:width="50%"}](../../assets/control/40-28C256-read-waveform.png)

Ad esempio, un <a href="https://www.reddit.com/r/beneater/comments/f7gcvx/glitches_on_eeprom_datalines_when_their_adress/" target="_blank">thread su Reddit</a> di rolf-electronics evidenzia il fenomeno nei primi 3 quadranti della seguente immagine, con i segnali di output che mostrano oscillazioni significative al momento del cambiamento degli input delle EEPROM:

[![Glitching nel SAP-1 di Rolf Electronics](../../assets/control/40-glitching-rolf.png "Glitching nel SAP-1 di Rolf Electronics"){:width="66%"}](../../assets/control/40-glitching-rolf.png)

Ora, qual è la relazione tra il glitching e il caricamento dell'Instruction Register al Rising Edge del clock?

Il grafico seguente mostra i fronti di salita e di discesa dei soli segnali di controllo attivati nei quattro step dell'istruzione LDA del SAP. I colori indicano che il glitching è innescato da un cambiamento intenzionale, cioè dal microcode che modifica volutamente lo stato di un determinato segnale. Le aree grigie, invece, rappresentano il glitching dei segnali non modificati dalla microistruzione corrente.

Il glitching dovuto alle variazioni degli indirizzi di ingresso delle EEPROM del SAP-1 (ma è così anche nell'NQSAP) avviene:

- ad ogni Falling Edge del Clock come conseguenza del cambiamento delle uscite del Ring Counter (momenti 1, 5, 9, 13, 17)
- al Rising Edge del Clock come conseguenza del caricamento dell'istruzione nell'Instruction Register (momento 7 nello step 1).

Il fenomeno del glitching si manifesta su tutti i segnali di controllo gestiti dalle EEPROM, sia quelli variati di proposito, sia quelli che non vengono modificati nello step corrente. Come nota a latere, bisogna segnalare che *tutti* i segnali di controllo del computer sono soggetti a questo fenomeno, anche se non indicati nel grafico.

[![SAP computer - istruzione LDA](../../assets/control/40-wavedrom-sap-lda.png "SAP computer - istruzione LDA"){:width="100%"}](../../assets/control/40-wavedrom-sap-lda.png)

*SAP computer - istruzione LDA*.

Prima di continuare, è interessante esaminare gli step di questa istruzione e ricollegarsi alla spiegazione dell'istruzione [LDA #$94](#ring-counter-e-microistruzioni) dell'NQSAP per vedere le similitudini:

1. PC esposto sul bus (CO, Counter Out) e caricamento del MAR (MI, Memory Address Register In)
2. RAM esposta sul bus (RO, RAM Out), caricamento dell'IR (II, Instruction Register In) e incremento del PC (CE, Counter Enable)
3. IR esposto sul bus (IO, Instruction Register Out), caricamento del MAR (MI, Memory Address Register In)
4. RAM esposta sul bus (RO, RAM Out), caricamento di A (AI, A In)

Dopo questa breve digressione, ritorniamo al discorso principale.

Tutti questi segnali spuri generalmente non sono un problema per il SAP, perché le microistruzioni scrivono su registri tipo D <a href="https://www.ti.com/lit/ds/sdls067a/sdls067a.pdf" target="_blank">74LS173</a> attivati al Rising Edge del clock, cioè quando i segnali di controllo sono stabili. Ad esempio, il glitching di MI al momento 7 non è fonte di problemi, perché il '173 del MAR memorizza nuovi valori solo col segnale di Enable attivo ***e*** il Rising Edge del clock: in quel momento, il segnale MI si trova in uno stato stabile e non c'è rischio di caricare dati non corretti.

Vi è un'eccezione durante il caricamento dei Flag: poiché questi sono mappati direttamente sugli ingressi delle EEPROM, ogni variazione di C o F provoca glitching a ogni Rising Edge che li modifica. Per semplicità, il grafico precedente non include la rappresentazione di questo momento di instabilità.

Possiamo ora riprendere la domanda fatta in precedenza in questa sezione: "Quali sono le possibili conseguenze del caricamento dell'IR al Rising Edge del clock?"

Se nel computer sono presenti registri privi di un segnale di Enable, il loro caricamento può essere effettuato implementando una logica combinatoria tra il clock e il segnale di controllo dedicato. Ad esempio, nell'NQSAP i [registri D, X e Y](../dxy) e il [registro B](../alu/#lalu-dellnqsap) sono realizzati con <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> e porte NOR.

[![Registro Y dell'NQSAP](../../assets/control/40-NQSAP-dxy-y.png "Registro Y dell'NQSAP"){:width="75%"}](../../assets/control/40-NQSAP-dxy-y.png)

*Registro Y dell'NQSAP*.

La risposta alla domanda è che il caricamento dell'Instruction Register al momento 7 genera un glitch sul segnale /WY, che può causare un caricamento indesiderato di Y. L'Enable del '574 dipende infatti dall'operazione NOR tra il clock invertito e /WY. Se quest'ultimo è instabile, potrebbe verificarsi una scrittura non voluta del registro.

[![Glitching all'istruzione LDY nell'NQSAP](../../assets/control/40-nqsap-ldy.png "Glitching all'istruzione LDY nell'NQSAP"){:width="100%"}](../../assets/control/40-nqsap-ldy.png)

*Glitching all'istruzione LDY nell'NQSAP*.

E' dalla necessità di indirizzare il problema del glitching che prende forma il design dell'Instruction Register dell'<a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, evoluzione dell'NQSAP.

Per risolvere i problemi di glitching, Tom ha ridisegnato l'IR sostituendo i 74LS173 con due registri tipo D <a href="https://www.ti.com/lit/ds/symlink/sn74ls377.pdf" target="_blank">74LS377</a> in cascata. Il primo si aggiorna come di consueto durante il normale caricamento dell'IR, che avviene al Rising Edge del clock al momento 7 dello step 1 e mantiene inalterata l'operatività del computer. L'output del primo registro viene portato come input al secondo '377, che viene aggiornato al Falling Edge in contemporanea all'incremento del Ring Counter. In questo modo, tutti gli ingressi delle EEPROM vengono aggiornati simultaneamente al Falling Edge del clock, garantendo che i segnali di controllo in uscita siano ormai stabili quando i registri del computer vengono aggiornati al successivo Rising Edge.

Questa miglioria è stata recepita nel BEAM, che nel suo design cerca di includere anche gli aspetti positivi dell'NQSAP-PCB.

[![Schemi dell'Instruction Register dell'NQSAP e del BEAM](../../assets/control/40-cl-ir-beam-nqsap.png "Schemi dell'Instruction Register dell'NQSAP e del BEAM"){:width="100%"}](../../assets/control/40-cl-ir-beam-nqsap.png)

*Schemi dell'Instruction Register dell'NQSAP e del BEAM.*

Peraltro, *tutti* i registri a 8 bit del BEAM sono realizzati con componenti dotati di Enable e Clock separati. Conseguentemente, non si possono verificare caricamenti indesiderati poiché, al Rising Edge del clock, i segnali di controllo sono sempre stabili.

Risulta comunque interessante visualizzare il comportamento dei segnali di controllo al momento 7, durante il quale - come ormai assodato - l'unico registro aggiornato è l'IR:

[![Nessun glitching sul BEAM al momento 7 nello step 1](../../assets/control/40-beam-ldy.png "Nessun glitching sul BEAM al momento 7 nello step 1"){:width="100%"}](../../assets/control/40-beam-ldy.png)

*Nessun glitching sul BEAM al momento 7 nello step 1*.

Il primo dei due '377 si aggiorna al Rising Edge al momento 7, senza causare glitching nelle EEPROM, poiché i loro ingressi non vengono modificati. Le uscite di questo primo registro vengono quindi inviate come input al secondo '377, che si aggiorna al Falling Edge del clock al momento 9, contemporaneamente all'incremento del RC. Solo ora tutti gli ingressi delle EEPROM vengono aggiornati simultaneamente, consentendo ai segnali di controllo di stabilizzarsi in attesa del momento 11, quando i registri vengono caricati secondo le microistruzioni impostate nello step 2.

Come possiamo essere certi che l'eliminazione del glitching nel BEAM derivi effettivamente dalla doppia bufferizzazione del Program Counter? Tutti i registri a 8 bit realizzati con componenti dotati di Enable e Clock separati sono immuni al fenomeno, ma alcuni altri registri sono privi di tale ingresso, come il Flip-Flop 74LS74 utilizzato per memorizzare i Flag. Una porta AND consente di realizzare un segnale di Enable artificiale, similarmente allo schema del *Registro Y dell’NQSAP*.

![Registro Flag C del BEAM](../../assets/control/40-beam-c-flag.png "Registro Flag C del BEAM"){:width="66%"}

*Registro Flag C del BEAM*.

Esaminiamo la semplice istruzione SEC, che imposta il Carry.

~~~text
| ---- | ---------------------|
| Step | Microistruzione      |
| ---- | ---------------------|
| 0*   | RPC | WM             |
| 1*   | RR  | WIR | PCI      |
| 2    | CC  | FC  | RL  | NI |
| ---- | ---------------------|
~~~

*Scomposizione dell'istruzione SEC nelle sue tre microistruzioni elementari*.

1. Il primo step carica l'indirizzo dell'istruzione corrente nel Memory Address Register:
    - RPC, Read Program Counter - espone l'indirizzo del PC sul bus
    - WM, Write Memory Address Register - carica l'indirizzo dell'istruzione nel MAR
2. Il secondo step carica l'opcode dell'istruzione nell'IR e incrementa il PC per farlo puntare alla locazione di memoria successiva (che nel caso dell'istruzione SEC, lunga un solo byte, sarà la prossima istruzione):
    - RR, Read RAM - espone sul bus l'opcode dell'istruzione
    - WIR, Write Instruction Register - carica l'opcode nell'IR\*
    - PCI, Program Counter Increment - incrementa il PC
3. Il terzo step scrive 1 sul registro C del 74LS74:
    - CC, Clear Carry - imposta l'ingresso ALU-Cin dell'ALU (ricordare che il Carry del '181 è [invertito](../alu/#funzioni-logiche-e-operazioni-aritmetiche): stato HI = inattivo)
    - FC, Flag C - predispone il caricamento del Flag C
    - RL, Read ALU - espone sul bus il contenuto dell'ALU
    - NI, Next Instruction - resetta il Ring Counter

\* I primi due step di *tutte* le istruzioni sono *sempre* identici.

CC attivo al momento 9 invia un segnale HI al pin ALU-Cin e l'[opcode 03](..alu/#relazione-diretta-hardwired-tra-instruction-register-e-alu), senza Carry, configura l'ALU per emettere un output di tutti 1 sul bus.

[![Nessun glitching sul BEAM al caricamento del Flag C](../../assets/control/40-beam-sec.png "Nessun glitching sul BEAM al caricamento del Flag C"){:width="100%"}](../../assets/control/40-beam-sec.png)

*Nessun glitching sul BEAM al caricamento del Flag C*.

 Al Rising Edge del clock, il valore 1 presente al pin D del Flip-Flop viene caricato e mantenuto, impostando cosi il Flag di Carry: al momento 11 il segnale FC è stabile e il Flip-Flop che contiene il Flag C viene aggiornato senza effetti collaterali.

---

Concludendo la sezione, è importante ricordare che tutti i segnali di una microistruzione sono attivati contemporaneamente, ma che le operazioni di lettura e scrittura impostate dalla Control Word vengono eseguite secondo tempistiche diverse. Al Falling Edge del clock:

- I segnali di lettura impostati dalla Control Word attivano immediatamente l'eventuale modulo interessato da una Read, il quale presenta subito il suo output sul bus; ad esempio, l'attivazione di un bus transceiver <a href="https://www.mouser.com/datasheet/2/308/74LS245-1190460.pdf" target="_blank">74LS245</a> è immediata.
- Viceversa, i segnali di caricamento preparano i moduli interessati, ma le operazioni di Write vengono eseguite solo al successivo Rising Edge del clock, assicurando così che i registri da aggiornare ricevano segnali già stabilizzati. Un esempio è il registro tipo D 74LS377 citato in precedenza.

## Lunghezza delle istruzioni

Altro aspetto importante da prendere in considerazione è il numero di microistruzioni che possono comporre ogni istruzione.

Il SAP-1 prevedeva un numero fisso di 5 step; conseguentemente, tutte le istruzioni avevano la stessa durata, indipendentemente dalla loro complessità. Tuttavia, nel microcode che segue possiamo vedere che in realtà l'istruzione di caricamento immediato LDA potrebbe essere eseguita in soli tre step, mentre somma e sottrazione necessitano di cinque step:

[![Microcode del computer SAP](../../assets/control/40-cl-sap-microcode.png "Microcode del computer SAP"){:width="66%"}](../../assets/control/40-cl-sap-microcode.png)

*Microcode del computer SAP.*

Nello schema del *Ring Counter del SAP-1* si nota che il contatore '161 presenta le sue uscite agli ingressi di selezione del demultiplexer '138, che attiva in sequenza le uscite invertite (active = LO) da 00 a 05: ad ogni attivazione di quest'ultima, le due NAND attivano l'ingresso di Reset /MR del '161, che riporta il conteggio degli step allo zero iniziale, cominciando così una nuova istruzione.

E' facile notare come questa architettura comporti uno spreco di cicli di elaborazione durante l'esecuzione di istruzioni che richiedono pochi passaggi, in quanto il RC deve comunque attendere l'attivazione dell'ultima uscita 05 per essere resettato.

[![Ring Counter del SAP](../../assets/control/40-control-sap-rc.png "Ring Counter del SAP"){:width="50%"}](../../assets/control/40-control-sap-rc.png)

*Ring Counter del SAP.*

L'NQSAP di Tom prevede un accorgimento molto furbo (tra gli altri) e migliora le prestazioni del computer introducendo la durata variabile delle istruzioni; infatti, l'ultima microistruzione di ogni istruzione include un segnale N (NI nel BEAM), che attiva il pin di caricamento parallelo del '161: poiché tutti gli input del contatore sono impostati sullo 0, il conteggio ritorna allo zero iniziale.

In altre parole, si mette anticipatamente fine ad ogni istruzione inserendo nell'ultimo step del microcode un segnale di Load del RC, così da non dover attendere l'esecuzione di tutti gli step vuoti; il vantaggio nell'operare questa scelta aumenta man mano che si desidera implementare istruzioni sempre più complesse che necessitano di un numero massimo di step sempre maggiore. Ad esempio, nel BEAM la lunghezza massima possibile di un'istruzione è di 16 step, ma un'istruzione semplice come TXA può essere eseguita in soli 3 step, senza sprecare gli altri 13 cicli.

Il momento del caricamento del contatore è visibile a pagina 11 del <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">datasheet</a>: con /Load allo stato LO*, al successivo Rising Edge** del clock le uscite QA-QD assumono gli stati LO presenti agli ingressi A-D (istante Preset nella ascissa).

In pratica, il Ring Counter ritorna allo step iniziale.

[![Segnale Next Instruction nel Ring Counter del BEAM](../../assets/control/40-beam-ni.png "Segnale Next Instruction nel Ring Counter del BEAM"){:width="66%"}](../../assets/control/40-beam-ni.png)

*Segnale Next Instruction nel Ring Counter del BEAM.*

Potrebbe sorgere una domanda: perché non collegare il segnale N del microcode direttamente al pin di Reset del contatore?

Il reset del '161 è *asincrono*, che significa che è indipendente dal clock: di conseguenza, il contatore verrebbe resettato al Falling Edge del clock nel momento stesso dell'impostazione della Control Word, non permettendo il completamento dello step al Rising Edge!

In realtà, fa notare Tom, sarebbe comunque possibile utilizzare il Reset asincrono del '161 collegato direttamente al segnale N, ma questo significherebbe dover aggiungere uno step dedicato al reset come ultima microistruzione di ogni istruzione. Utilizzando invece il caricamento sincrono, non è necessario uno step di reset dedicato.

\* e \*\*: ricordando quanto esposto alla fine della sezione precedente in relazione all'impostazione della Control Word e ai momenti di caricamento dei registri, troviamo qui un primo esempio concreto: il segnale /Load viene settato dalla Control Word durante il Falling Edge del clock, mentre l'effettivo caricamento del registro avviene in concomitanza con il Rising Edge.

Come indicato anche nella sezione [Differenze](.../alu/#differenze-tra-moduli-alu-dellnqsap-e-del-beam) della pagina dell'ALU, bisogna notare che il computer NQSAP prevede solo 8 step per le microistruzioni. Per emulare alcune istruzioni del 6502 di scorrimento e rotazione servono più step, pertanto, sul computer BEAM ne sono stati previsti 16.

## I 74LS138 per la gestione dei segnali

La complessità dell'NQSAP è tale per cui i soli 16 segnali di controllo disponibili nella Control Logic del SAP-1 non sarebbero stati sufficienti per pilotare moduli complessi come ad esempio l'ALU e il registro dei Flag; in conseguenza di questo, diventava necessario ampliare in maniera considerevole il numero di linee di controllo utilizzabili.

L'aumento del numero di EEPROM e l'inserimento di quattro demultiplexer <a href="https://www.ti.com/lit/ds/symlink/sn74ls138.pdf" target="_blank">74LS138</a> consente di gestire l'elevato numero di segnali richiesti dall'NQSAP e dal BEAM.

Come visibile nello schema, ogni '138 presenta 8 pin di output, 3 pin di selezione e 3 pin di Enable; connettendo opportunamente i pin di selezione ed Enable, è possibile pilotare ben quattro '138 (per un totale di 32 segnali di output) usando solo 8 segnali in uscita da una singola EEPROM. In altre parole, i '138 fungono da *demoltiplicatori* e permettono di indirizzare un numero elevato di segnali a partire da un numero limitato di linee in ingresso.

[![Demultiplexer 74LS138 nel BEAM](../../assets/control/40-cl-beam-eeprom-138.png "Demultiplexer 74LS138 nel BEAM"){:width="100%"}](../../assets/control/40-cl-beam-eeprom-138.png)

*Demultiplexer 74LS138 nel BEAM.*

Quando attive, le uscite dei '138 presentano uno stato LO; questa circostanza risulta molto comoda per la gestione dei segnali del computer, in quanto molti dei chip presenti nei vari moduli utilizzano ingressi di Enable invertiti (ad esempio i transceiver 74LS245 e i registri tipo D <a href="https://www.ti.com/lit/ds/symlink/sn74ls377.pdf" target="_blank">74LS377</a>).

I '138 presentano un solo output attivo alla volta; la configurazione dei pin di selezione ed Enable adottata nello schema permette di creare due coppie di '138, ognuna delle quali presenta un solo output attivo alla volta:

- una coppia dedicata ai segnali di lettura dai registri;
- una coppia dedicata ai segnali di scrittura sui registri.

Un effetto collaterale positivo in questo tipo di gestione sta nel fatto che risulterà impossibile attivare più letture contemporanee, prevenendo così il rischio di cortocircuiti involontari tra uscite allo stato HI e uscite allo stato LO di moduli diversi.

Il ragionamento per le operazioni di scrittura è diverso, poiché è effettivamente necessario poter scrivere su più registri contemporaneamente. Un operazione di questo tipo non causa conflitti sul bus ed è utilizzata, ad esempio, dall'istruzione di somma ADC, che prevede uno step in cui i dati vengono scritti simultaneamente sia nel registro A sia nel registro dei Flag.

Nello schema si può notare che tutti i registri del computer che *non hanno tra loro* la necessità di essere attivi contemporaneamente - tanto in lettura quanto in scrittura - sono indirizzati con i demultiplexer.

Sono invece indispensabili segnali di controllo provenienti direttamente dalle EEPROM in tre casi:

- quando un registro presenta più segnali di ingresso che possono essere attivi contemporaneamente (ad esempio il registro dei [Flag](../flags/#componenti-e-funzionamento), oppure il registro [H](../alu/#il-registro-h));
- quando è necessario poter scrivere su più registri contemporaneamente (ad esempio A e H, oppure Flag e A, oppure Flag e H*);
- quando occorrono altri segnali di controllo totalmente indipendenti (ad esempio per lo Stack, oppure per la gestione del [Carry Input](../flags/#il-carry-e-i-registri-h-e-alu) per ALU ed H).

\* In questo secondo caso, i segnali provenienti direttamente dalle EEPROM vengono utilizzati per gestire altri registri che devono poter essere attivi contemporaneamente ad uno dei registri singolarmente indirizzabili dalla coppia di '138 adibiti ai segnali di scrittura.

Riassumendo:

- una prima EEPROM gestisce quattro demultiplexer che pilotano i segnali di *lettura* di tutti i registri e i segnali di *caricamento* di tutti i registri (eccetto H e Flag);
- altre tre EEPROM gestiscono tutti gli altri segnali, inclusi quelli che governano H e Flag.

Notare che i segnali di uscita dei '138 realmente utilizzabili sono 30 e non 32, perché il microcode deve prevedere situazioni in cui nessun registro pilotato deve essere attivo. Ad esempio, un output 0000.0000 della prima EEPROM attiverà i pin D0 del primo e del terzo demultiplexer: poiché entrambi i pin sono scollegati, sarà sufficiente impostare l'output sulla prima EEPROM a 0x00 per evitare l'attivazione di qualsiasi registro gestito dai '138.

## Caricamento di un programma dal Loader

NQSAP e BEAM consentono di automatizzare il caricamento di un programma grazie alla presenza di un Loader basato su Arduino Nano.

Il Loader controlla alcuni segnali della Control Logic e del modulo di clock. Il segnale LDR-Active può inibire le prime due EEPROM e consente al Loader di sostituirsi nel controllo dei '138 utilizzando i segnali N0-N7. Inibendo il clock principale del computer, il Loader può iniettare un proprio clock ed utilizzarlo per caricare i registri MAR e RAM.

Una spiegazione più dettagliata è presente nella pagina dedicata al [Loader](../loader/).

## Riepilogo segnali dell'NQSAP e del BEAM

La prima tabella riassume i segnali di controllo originati dalla Control Logic. La seconda tabella comprende una descrizione dei bus esistenti nel computer e la lista dei segnali di controllo non provenienti dalla Control Logic.

La colonna "Ambito o direzione segnale" indica il contesto di un bus, oppure sorgente e destinazione di un segnale di controllo.

### Segnali di controllo

| NQSAP         | BEAM           | Ambito o direzione segnale | Descrizione                                                                                                  |
| -----         | ----           | -------------------------- | -----------                                                                                                  |
| N             | NI             | CL                         | Next Instruction; [spiegazione](#lunghezza-delle-istruzioni).                                                |
| LF            | LF             | CL → ALU                   | ALU Force; [spiegazione 1](../alu/#istruzioni-di-comparazione) e [spiegazione 2](../alu/#riepilogo-sottrazioni-comparazioni-e-indirizzamenti).  |
| HL-HR         | HL-HR          | CL → ALU                   | Definiscono l'operazione da eseguire sul registro H (caricamento parallelo, scorrimento / rotazione dx o sx).                                   |
| IR-Q0 / IR-Q4 | IR-S0..3, IR-M | CL → ALU                   | Determinano l'operazione che l'ALU deve eseguire; [spiegazione](../alu/#funzioni-logiche-e-operazioni-aritmetiche).                             |
| HLT           | HLT            | CL → Clock                 | Interrompe il programma in esecuzione; [spiegazione](../clock/#note-sul-microcode).                                                             |
| DY-DZ         | DX/Y-DZ        | CL → DXY                   | DX/Y HI espone X agli adder, LO espone Y; DZ espone zero; [spiegazione](../dxy/#utilizzo-con-le-modalità-di-indirizzamento-indicizzate).        |
| C0-C1         | C0-C1          | CL → Flag                  | Determinano se il Carry da salvare nel Flag C provenga dal Carry Output dell'ALU o da H (scorrimento e rotazione); [spiegazione](../flags/#carry).   |
| CC-CS         | CC-CS          | CL → Flag                  | Selezionano quale Carry presentare agli input di ALU e H (quello reale, oppure 0 o 1 fissi); [spiegazione](../flags/#il-carry-e-i-registri-h-e-alu). |
| FC            | FC             | CL → Flag                  | Caricamento del Flag C nel registro dei flag.                                                                                   |
| FN            | FN             | CL → Flag                  | Caricamento del Flag N nel registro dei flag.                                                                                   |
| FB            | FS             | CL → Flag                  | Origine dei Flag da caricare nel registro dei Flag (computo oppure bus); [spiegazione](../flags/#componenti-e-funzionamento).   |
| FV            | FV             | CL → Flag                  | Caricamento del Flag V nel registro dei flag.                                                                |
| FZ            | FZ             | CL → Flag                  | Caricamento del Flag Z nel registro dei flag.                                                                |
| JE            | JE             | CL → Flag                  | Attiva le istruzioni di salto condizionale; [spiegazione](../flags/#i-salti-condizionali-e-incondizionati).  |
| PI            | PCI            | CL → PC                    | Incrementa il Program Counter.                                                                               |
| SCE\*         | SE             | CL → SP                    | Attiva incremento/decremento dello Stack Pointer.                                                            |
| SPI\*         | SU/D           | CL → SP                    | Indica se lo Stack Pointer deve contare verso l'alto (HI) o verso il basso (LO).                             |
| RA            | RA             | CL → ALU                   | Espone sul bus il contenuto dell'accumulatore A.                                                             |
| RB            | RB             | CL → ALU                   | Espone sul bus il contenuto del registro B.                                                                  |
| RD            | RD             | CL → DXY                   | Espone sul bus il contenuto del registro D.                                                                  |
| RF            | RF             | CL → Flag                  | Espone sul bus il contenuto del registro dei Flag.                                                           |
| RH            | RH             | CL → ALU                   | Espone sul bus il contenuto del registro H.                                                                  |
| RL            | RL             | CL → ALU                   | Espone sul bus l'output della ALU.                                                                           |
| RP            | RPC            | CL → PC                    | Espone sul bus il contenuto del Program Counter.                                                             |
| RR            | RR             | CL → RAM                   | Espone sul bus il contenuto della RAM.                                                                       |
| RX            | RX             | CL → DXY                   | Espone sul bus il contenuto del registro X.                                                                  |
| RY            | RY             | CL → DXY                   | Espone sul bus il contenuto del registro Y.                                                                  |
| RS            | RS             | CL → SP                    | Espone sul bus il valore dello Stack Pointer.                                                                |
| WI            | WIR            | CL                         | Scrive il contenuto del bus nell'Instruction Register.                                                       |
| WA            | WA             | CL → ALU                   | Scrive il contenuto del bus nell'accumulatore A.                                                             |
| WB            | WB             | CL → ALU                   | Scrive il contenuto del bus nel registro B.                                                                  |
| WD            | WD             | CL → DXY                   | Scrive il contenuto del bus nel registro D.                                                                  |
| WX            | WX             | CL → DXY                   | Scrive il contenuto del bus nel registro X.                                                                  |
| WY            | WY             | CL → DXY                   | Scrive il contenuto del bus nel registro Y.                                                                  |
| WM            | WM             | CL → MAR                   | Scrive il contenuto del bus nel Memory Address Register.                                                     |
| WO            | WO             | CL → Output                | Scrive il contenuto del bus nel registro di Output.                                                          |
| WR            | WR             | CL → RAM                   | Scrive il contenuto del bus nella RAM.                                                                       |
| WS            | WS             | CL → SP                    | Scrive il contenuto del bus nello Stack Pointer.                                                             |
| WP            | WPC            | CL → PC                    | Scrive il contenuto del bus nel Program Counter.                                                             |

### Bus e altri segnali

| NQSAP           | BEAM                      | Ambito o direzione segnale | Descrizione                                                |
| -----           | ----                      | -------------------------- | -----------                                                |
| CLK             | CLK                       | Computer                   | Segnale di clock inviato a tutti i moduli del computer.    |
| D0..7           | D0..7                     | Computer                   | Bus del computer.                                          |
| MA0 - MA10      | A0..11                    | CL                         | Bus tra output di RC ed IR e input delle EEPROM; spiegazione in questa stessa pagina.                                                 |
| IR-Q5 / IR-Q7   | IR-A0 / IR-A2             | CL → Flag                  | Ingressi di selezione dei salti condizionali del registro dei Flag; [spiegazione](../flags/#i-salti-condizionali-e-incondizionati).   |
| ALU-to-register interconnect | H0..7, B0..7 | ALU                        | Bus tra output dei registri B e H e input dell'ALU; [spiegazione](../alu/#lalu-dellnqsap).                                            |
| ALU output      | Q0..7                     | ALU                        | Bus tra output dell'ALU e transceiver di output al bus del computer; [spiegazione](../alu/#lalu-dellnqsap).                           |
| Selector Inputs | X0..7, Y0..7              | DXY                        | Bus tra output di X e Y e input dei selettori X/Y; [spiegazione](../dxy/#utilizzo-con-le-modalità-di-indirizzamento-indicizzate).     |
| Adder Inputs    | DQ0..7 XY0..7             | DXY                        | Bus tra output di D e selettori X/Y e input degli adder; [spiegazione](../dxy/#utilizzo-con-le-modalità-di-indirizzamento-indicizzate).                  |
| Adder Outputs   | AQ0..7                    | DXY                        | Bus tra output degli adder e transceiver di output al bus del computer; [spiegazione](../dxy/#utilizzo-con-le-modalità-di-indirizzamento-indicizzate).   |
| MC-RR0..3       | N0..3                     | Loader → CL                | Utilizzati dal Loader per impostare i '138 dei segnali di lettura; [spiegazione](#i-74ls138-per-la-gestione-dei-segnali).                                |
| MC-RW0..3       | N4..7                     | Loader → CL                | Utilizzati dal Loader per impostare i '138 dei segnali di scrittura; [spiegazione](#i-74ls138-per-la-gestione-dei-segnali).                              |
| PC-Load         | PCJ                       | Flag → PC                  | Controlla il caricamento del PC per i salti condizionali e incondizionati; [spiegazione](../flags/#i-salti-condizionali-e-incondizionati).               |
| ?               | MA0-MA7                   | MAR → RAM                  | Bus tra output del MAR e input della RAM; [spiegazione](../ram/#design-dei-moduli-mar-e-ram).                                         |
| PROG            | PROG                      | MAR → RAM                  | Selezione tra modalità di programmazione della RAM o di esecuzione del programma; [spiegazione](../ram/#mux-program-mode-e-run-mode). |
| RST             | RST                       | Computer                   | Reset asincrono del computer; [spiegazione](../loader/#caricamento-di-un-programma).                                                  |
| LDR-ACTIVE      | LDR-Active                | Loader → Clock e → CL      | Disattivazione clock e EEPROM Control Logic; [spiegazione](../loader/#caricamento-di-un-programma).                                   |
| LDR-CLK         | LDR-CLK                   | Loader → Clock             | Iniezione del clock del Loader nel computer; [spiegazione](../loader/#caricamento-di-un-programma).                                   |
| CLK-Start       | CLK-Start                 | Loader → Clock             | (Re-)Start del clock di sistema dopo il caricamento del programma in RAM; [spiegazione](../loader/#caricamento-di-un-programma).      |
| ALU-Cin         | ALU-Cin                   | Flag → ALU                 | Selezione del Carry da inviare in input ai '181; [spiegazione](../flags/#il-carry-e-i-registri-h-e-alu).                              |
| H-Cin           | H-Cin                     | Flag → ALU                 | Selezione del Carry da inviare in input ad H; [spiegazione 1](../flags/#il-carry-e-i-registri-h-e-alu) e [spiegazione 2](../alu/#il-registro-h).        |
| ALU-Cout        | ALU-Cout                  | ALU → Flag                 | Carry output dell'ALU da inviare al registro dei Flag; [spiegazione Flag C](../flags/#carry).                    |
| ALU-Q7          | ALU-Q7                    | ALU → Flag                 | MSB dell'ALU da inviare al registro dei Flag; [spiegazione Flag V](../flags/#overflow).                          |
| B-Q7            | B-Q7                      | ALU → Flag                 | MSB di B da inviare al registro dei Flag; [spiegazione Flag V](../flags/#overflow).                              |
| H-Q0\*          | H-Q0                      | ALU → Flag                 | LSB di H da inviare al registro dei Flag; [spiegazione Flag C](../flags/#carry).                                 |
| H-Q7            | H-Q7                      | ALU → Flag                 | MSB di H da inviare al registro dei Flag; [spiegazione Flag V](../flags/#overflow) e [Flag C](../flags/#carry).  |

\* Manca nel modulo ALU; dimenticanza nello schema di Tom.

## Microcode

La fase di scrittura del microcode non è stata *troppo* complessa. L'esperienza fatta col SAP, lo studio approfondito dell'NQSAP e molta pazienza mi avevano portato a comprendere piuttosto bene come sviluppare gli step delle istruzioni tenendo in considerazione le diverse modalità di indirizzamento del 6502.

Solo poche istruzioni hanno richiesto più tempo per essere assimilate, in particolare quelle di [comparazione](../alu/#istruzioni-di-comparazione), di [salto a subroutine](../stack/#implementazione-del-microcode-dello-stack-pointer) e di salto condizionale. Le istruzioni di comparazione hanno implicato una comprensione approfondita del risultato per impostarne correttamente i flag, mentre per le altre è stato necessario apprendere come utilizzare un registro temporaneo per memorizzare un'informazione da ripristinare in uno step successivo.

[![Scrittura del microcode del BEAM con VScode](../../assets/control/40-microcode-vscode.png "Scrittura del microcode del BEAM con VScode"){:width="100%"}](../../assets/control/40-microcode-vscode.png)

*Scrittura del microcode del BEAM con VScode.*

E' risultata invece particolarmente difficile l'*organizzazione* dell'Instruction Set, sulla quale, col senno di poi, avrei dovuto investire più tempo. Purtroppo, ho realizzato di non essere riuscito ad organizzare in maniera stutturata il posizionamento degli opcode solo durante la scrittura del microcode, ma in quel momento avevo già iniziato a lavorare sulla realizzazione hardware - con i forti legami hardwired tra IR, ALU e Flag - e non volevo più tornare indietro.

Tom aveva automatizzato parte della generazione del microcodice attraverso un opportuno raggruppamento logico delle istruzioni. Personalmente, non sono riuscito a ottenere risultati comparabili, poiché la mia conoscenza del linguaggio C, sia all'epoca sia al momento della scrittura di questa documentazione, è modesta. Questo mi ha impedito di comprendere chiaramente come strutturare l'Instruction Set per sfruttare appieno tali vantaggi.

La <a href="../../assets/BEAM computer.xlsx" target="_blank">cartella di lavoro Excel</a> che ho realizzato presenta l'Instruction Set del 6502, l'analisi delle istruzioni per determinare le modalità di indirizzamento e lo sviluppo dell'Instruction Set del BEAM, considerando la necessità di utilizzare il [segnale di controllo LF](../alu/#istruzioni-di-comparazione) per mettere in Subtract Mode l'ALU ed effettuare le operazioni di comparazione.

[![Definizione dell'Instruction Set del BEAM](../../assets/control/40-control-inst-set.png "Definizione dell'Instruction Set del BEAM"){:width="100%"}](../../assets/control/40-control-inst-set.png)

*Definizione dell'Instruction Set del BEAM.*

Ho speso molto tempo anche nella scrittura dello sketch Arduino utilizzato per programmare le EEPROM. Il programmatore di Ben Eater poteva impiegare *alcuni* interminabili minuti per ogni EEPROM, mentre la programmazione a blocchi implementata per il BEAM - frutto dello studio del codice e del circuito di Tom - ha permesso di ridurre il tempo di scrittura di una AT28C256 a soli 14 secondi. Per ulteriori note, si rimanda alla [pagina dedicata](../eeprom-programmer).

Concludendo, la realizzazione fisica del BEAM e la scrittura del microcode non hanno avuto un percorso molto lungo di *trial and error*, perché le lunghe analisi avevano sortito l'effetto di far funzionare i moduli sin dai primi tentativi, o comunque con poche variazioni finali.

Il codice è parzialmente commentato e dovrebbe essere abbastanza esplicativo.

Alcuni link:

- Un <a href="https://www.atarimania.com/documents/6502%20(65xx)%20Microprocessor%20Instant%20Reference%20Card.pdf" target="_blank">compendio della Micro Logic</a> incredibilmente utile, che in sole due pagine include opcode, modalità di indirizzamento, flag e istruzioni che li modificano, funzionamento delle istruzioni di scorrimento e molto altro. Insostituibile.
- Un validissimo riferimento per l'analisi della relazione tra Control Logic (CL) ed IR è stata la pagina <a href="https://www.masswerk.at/6502/6502_instruction_set.html" target="_blank">6502 Instruction Set</a> di Norbert Landsteiner. Inquadra l'Instruction Set in una comoda vista tabellare, dalla quale ho ricavato la <a href="../../assets/BEAM computer.xlsx" target="_blank">vista Excel</a> utilizzata per definire gli opcode delle istruzioni del BEAM.
- Sempre di Norbert, invito a consultare anche il <a href="https://www.masswerk.at/6502/assembler.html" target="_blank">6502 Assembler</a> e il <a href="https://www.masswerk.at/6502/" target="_blank">Virtual 6502</a> che ho utilizzato in fase di debug del microcode: utilissimi per simulare l'esecuzione passo dopo passo delle istruzioni, visualizzando gli aggiornamenti dei flag ed aggiustando di conseguenza il microcode del BEAM.

### Differenze rispetto all'Instruction Set del 6502

Il computer BEAM non implementa gli Interrupt e la modalità Decimale del 6502, pertanto le istruzioni SEI, CLI, RTI e SED, CLD non fanno parte dell'Instruction Set.

Sono state aggiunte le seguenti istruzioni: INA, DEA, OUT.

Anche l'istruzione BRK non è stata implementata, ma si trova un comportamento simile nella [nuova HLT](../clock/#listruzione-hlt).

## Schema

[![Schema della Control Logic del computer BEAM](../../assets/control/40-control-logic-schema-beam.png "Schema della Control Logic del computer BEAM"){:width="100%"}](../../assets/control/40-control-logic-schema-beam.png)

*Schema della Control Logic del computer BEAM.*

## Differenze tra Control Logic dell'NQSAP e del BEAM

La Control Logic del computer BEAM riprende tutto ciò che è stato sviluppato da Tom Nisbet nell'NQSAP.

- Una differenza sostanziale sta nell'Instruction Register, sviluppato in modalità bufferizzata come nell'NQSAP-PCB di Tom per rimediare ai problemi di glitching riscontrati nell'NQ-SAP.
- Il BEAM prevede 16 step per le microistruzioni anziché solo 8. L'emulazione di alcune istruzioni di scorrimento e rotazione richiede più degli 8 step disponibili nell'NQSAP, che pertanto non le include.

## Note

- Per motivi di spazio, nello schema del BEAM non sono presenti la LED bar che mostra l'output del contatore '161 (nella realizzazione, è affiancata alla LED bar connessa all'uscita dell'Instruction Register, etichetta "EEPROM Address" nell'immagine ad inizio pagina) e la LED bar inserita tra i due 74LS377 dell'IR (etichetta "Instruction Register").

## Link utili

- I video di Ben Eater che descrivono la <a href="https://eater.net/8bit/control" target="_blank">Control Logic e il Microcode</a>.
- La <a href="https://tomnisbet.github.io/nqsap/docs/control/" target="_blank">Control Logic dell'NQSAP</a> di Tom Nisbet

## Riflessione sul microcode

Più o meno regolarmente si scoprono vulnerabilità nelle CPU: ad esempio, una macchina virtuale (VM) potrebbe essere in grado di <a href="https://en.wikipedia.org/wiki/Meltdown_(security_vulnerability)" target="_blank">leggere la memoria di un'altra VM</a>; per indirizzare le vulnerabilità, i produttori di sistemi rilasciano aggiornamenti del firmware per indirizzare le falle di sicurezza.

Prima della realizzazione del progetto SAP, non riuscivo a comprendere il legame tra aggiornamento del firmware e risoluzione del problema di sicurezza identificato in una CPU. Poiché una CPU non è propriamente un componente programmabile, non capivo come un aggiornamento potesse risolvere i problemi di sicurezza nati da una progettazione parzialmente problematica di un componente hardware.

Dopo aver costruito il SAP, ho compreso il ruolo del <a href="https://en.wikipedia.org/wiki/Microcode" target="_blank">microcode</a>. Le CPU industriali contengono un proprio microcode, similarmente a quello del SAP, dell'NQSAP, del BEAM. Tale microcode è scritto in una memoria non volatile della CPU e dunque non può essere modificato, ma la CPU comprende anche un'area di memoria volatile nella quale possono essere caricati aggiornamenti del microcode.

Quando viene distribuito un aggiornamento del microcode, il sistema operativo carica la  versione aggiornata del microcode nella CPU ad ogni boot; questa modifica è temporanea e risiede nella RAM della CPU, dove rimane caricata fino al prossimo riavvio.
