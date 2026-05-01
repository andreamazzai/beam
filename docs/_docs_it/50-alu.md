---
title: "ALU"
permalink: /docs/alu/
excerpt: "Unità Aritmetica e Logica del computer BEAM"
---
<small>[Il 74LS181](#il-74ls181) - [Il registro A](#il-registro-a) - [L’ALU dell’NQSAP](#lalu-dellnqsap) - [Il registro H](#il-registro-h) - [Funzioni logiche e operazioni aritmetiche](#funzioni-logiche-e-operazioni-aritmetiche) - [Relazione diretta (hardwired) tra Instruction Register e ALU](#relazione-diretta-hardwired-tra-instruction-register-e-alu) - [Indirizzamenti](#indirizzamenti) - [Istruzioni di comparazione](#istruzioni-di-comparazione) - [Le istruzioni di comparazione e i Flag](#le-istruzioni-di-comparazione-e-i-flag) - [Riepilogo: sottrazioni, comparazioni e indirizzamenti](#riepilogo-sottrazioni-comparazioni-e-indirizzamenti) - [Carry, addizioni e sottrazioni](#carry-addizioni-e-sottrazioni) - [Schema](#schema) - [Differenze tra Moduli ALU dell’NQSAP e del BEAM](#differenze-tra-moduli-alu-dellnqsap-e-del-beam) - [Link Utili](#link-utili)</small>

[![Unità Aritmetica e Logica del computer BEAM](../../assets/alu/50-alu-beam.png "Unità Aritmetica e Logica del computer BEAM"){:width="100%"}](../../assets/alu/50-alu-beam.png)

L'Unità Aritmetica e Logica (ALU) del SAP-1 computer di Ben Eater era limitata a addizioni e sottrazioni. L'NQSAP di Tom Nisbet aggiungeva operazioni logiche e di scorrimento (shift) e avevo iniziato a studiarla in dettaglio.

### Il 74LS181

Sviluppata attorno ai chip <a href="https://www.ti.com/lit/ds/symlink/sn54ls181.pdf" target="_blank">74LS181</a>, l'ALU dell'NQSAP presentava delle caratteristiche molto interessanti, ma di comprensione particolarmente difficile: sarà uno dei moduli sui quali spenderò più tempo, ma che alla fine darà maggiori soddisfazioni per il risultato.

Il '181 è un'ALU a 4 bit sviluppata negli anni '70 che può eseguire 16 operazioni aritmetiche e 16 funzioni logiche. E' possibile concatenare più chip per elaborare word di dimensioni maggiori.

**Utilizzo dei termini '181 e ALU come equivalenti**: in questa pagina, troveremo i termini ALU e '181 che vengono spesso utilizzati come equivalenti. Notare che ALU potrebbe indicare sia il modulo Unità Aritmetica e Logica nella sua interezza, sia il solo chip '181. Il contesto aiuterà a comprendere se per ALU si intenda il modulo, oppure il solo chip.

Inoltre, poiché nel modulo si utilizzano due '181 per poter comporre una word di 8 bit, in determinate situazioni si definiscono primo e secondo '181 oppure '181 inferiore e superiore rispettivamente quello che contiene i 4 bit meno significativi (Least Significant Bit, LSB) e quello che contiene i 4 bit più significativi (Most Significant Bit, MSB).

## Il registro A

Il registro A non è direttamente connesso alla ALU, ma, analizzando a livello logico quanto accade nel computer, ha senso includerne la descrizione questa pagina. Il registro A è un registro molto semplice, simile ai registri A e B già visti nel SAP-1 di Ben Eater; il BEAM lo implementa utilizzando un registro tipo D <a href="https://www.ti.com/lit/ds/symlink/sn54ls377.pdf" target="_blank">74LS377</a>.

Il registro A dell'NQSAP e del BEAM sono diversi per costruzione, ma identici dal punto di vista funzionale. Lo schema seguente, pur facendo riferimento al BEAM, è riutilizzabile in tutte le spiegazioni che seguono, anche quando principalmente rivolte alla descrizione del modulo ALU dell'NQSAP.

[![Schema del Registro A del computer BEAM](../../assets/alu/50-a-beam-schema.png "Schema del Registro A del computer BEAM"){:width="100%"}](../../assets/alu/50-a-beam-schema.png)

## L'ALU dell'NQSAP

Tra le caratteristiche che spiccavano nello schema dell'ALU dell'NQSAP, notavo soprattutto un numero elevato di chip - tra i quali i 4-bit Bidirectional Universal Shift Register <a href="https://www.ti.com/lit/ds/symlink/sn74ls194a.pdf" target="_blank">74LS194</a> - e un modo particolare di indirizzare i '181, che erano "strettamente legati" all'istruzione presente nell'Instruction Register della [Control Logic](../control). Anche il legame con la Control Logic è stato tra i più complessi da analizzare e comprendere, ma quello con il modulo dei Flag non è meno importante e la sua comprensione è stata altrettanto difficile: ad ogni operazione dell'ALU (e non solo) corrisponde infatti un'azione sul registro dei Flag.

[![Schema dell'ALU di Tom Nisbet](../../assets/alu/50-alu-nqsap.png "Schema dell'ALU di Tom Nisbet"){:width="100%"}](../../assets/alu/50-alu-nqsap.png)

*Schema dell'ALU di Tom Nisbet, leggermente modificato al solo scopo di migliorarne la leggibilità. Manca il segnale H-Q0, probabile dimenticanza di Tom.*

Il modulo ALU è sommariamente composto da due registri di input H e B e da una coppia di '181 interconnessi, che permettono di gestire una word di 8 bit: H e B sono i registri di input dei '181.

- Il registro H è in realtà uno Shift Register in grado sia di comportarsi come un normale registro a 8 bit, sia di *shiftare* a destra o a sinistra il valore presente in ingresso.
- Il registro B è un Octal D-Type Flip-Flop with 3-State Outputs <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> a 8 bit. Il '574 non include un ingresso Enable, che Tom ha dunque realizzato in maniera artificiale mettendo una NOR su /Clock e /WB ("Write B"); in questo modo il registro si attiva solo in corrispondenza di /WB (che è attivo LO) e del Falling Edge del clock invertito, equivalente al Rising Edge del clock normale, che è il momento in cui si caricano i registri (riferimento: video di Ben Eater <a href="https://www.youtube.com/watch?v=X7rCxs1ppyY" target="_blank">8-bit CPU control logic: Part 2</a>). Si veda anche la nota sul glitching nella sezione <a href="../control/#il-clock-il-glitching-delle-eeprom-e-linstruction-register-parte-2" target="_blank">Il clock e il “glitching” delle EEPROM</a> nella pagina della Control Logic.
- Tre transceiver '245 permettono di poter leggere i valori contenuti in H, B ed L (L è l'output dell'A**L**U).

## Il registro H

Nel computer SAP-1 di Ben Eater i registri di input all'ALU erano A e B, mentre nell'NQSAP sono H e B. Come accennato nella sezione precedente, il registro H si può comportare come un comune registro a 8 bit e può sostituire il registro A come input dell'ALU.

Poiché le istruzioni del 6502 fanno riferimento al registro A, è necessario che A ed H siano sempre allineati, così che i '181 ritrovino trasparentemente in H il contenuto di A (ad esempio una somma ADC sarà effettivamente realizzata dando in input ai '181 i registri H e B: essendo H una copia di A, il risultato della somma sarà uguale ad A + B).

Qual è l'utilità di un registro "ombra" come H? Alcune delle istruzioni che agiscono direttamente su una locazione di memoria o su un registro possono essere eseguite senza interferire col contenuto del registro A, ad esempio INX implicito, ASL Assoluto Indicizzato X e molte altre.

In tutti questi casi, uno degli ultimi step eseguiti dal microcode sarà la copia di A su H:

~~~text
| ---- | -------------------------- |
| Step | Microistruzione            |
| ---- | -------------------------- |
| 0*   | RPC | WM                   |
| 1*   | RR  | WIR | PCI            |
| 2    | RX  | WH                   |
| 3    | CS  | FNZ | RL | WX        |
| 4    | RA  | WH  | NI             |
| ---- | -------------------------- |
~~~

*Scomposizione dell'istruzione INX nelle sue cinque microistruzioni elementari*.

Nell'esempio dell'istruzione INX del 6502, dopo la [fase Fetch](../control/#fasi)* comune a tutte le istruzioni:

- X viene letto (RX) e copiato in H (WH), che presenta il suo contenuto agli ingressi "A" dei '181;
- i '181 eseguono l'operazione **A Plus 1**, il cui risultato viene esposto sul bus (RL) e copiato in X (WX);
- A, non modificato, viene esposto sul bus (RA) ed H viene riallineato (WH).

Altra caratteristica rilevante del registro H realizzato con Shift Register '194 è la capacità di effettuare operazioni di scorrimento e rotazione. I due pin di ingresso S0 ed S1 definiscono il comportamento dello Shift Register al Rising Edge del clock:

| S1 | S0 | Operazione                                                                                            |
|  - | -  |  -                                                                                                    |
| LO | LO | Mantiene lo stato precedente                                                                          |
| LO | HI | Scorre a sinistra i bit di output (Q3 ← Q2 \| Q2 ← Q1 \| Q1 ← Q0) e carica l'input Serial Right in Q0 |
| HI | LO | Scorre a destra i bit di output (Q3 → Q2 \| Q2 → Q1 \| Q1 → Q0) e carica l'input Serial Left in Q3    |
| HI | HI | Carica gli input P0-P3 in Q0-Q3                                                                       |

Lo schema mostra l'esecuzione di un'operazione di scorrimento a sinistra, che richiede l'attivazione del segnale di controllo HL/S0:

[![Scorrimento a sinistra nel registro H del BEAM](../../assets/alu/50-alu-beam-h.png "Scorrimento a sinistra nel registro H del BEAM"){:width="100%"}](../../assets/alu/50-alu-beam-h.png)

*Scorrimento a sinistra nel registro H del BEAM.*

Al rising Edge del clock i '194 traslano verso sinistra gli output Q0-Q3 e caricano in Q0 i valori presenti agli ingressi Serial Right (Dsr), eseguendo tutte le operazioni seguenti nello stesso momento:

- il '194 di destra carica in Q0/H0 il segnale H-Cin presente al suo ingresso Dsr, che diventa il nuovo LSB del byte traslato;
- il '194 di sinistra carica in Q0/H4 il valore di H3 presente al suo ingresso Dsr;
- gli ouput Q0-Q3 di entrambi i '194 scorrono verso sinistra.

Le istruzioni di scorrimento / rotazione a sinistra del 6502 memorizzano nel Carry il bit più significativo del byte da traslare. L'immagine mostra le diverse istruzioni di scorrimento e rotazione del 6502 ed evidenzia lo scorrimento a sinistra.

![Istruzioni di scorrimento e rotazione del 6502](../../assets/alu/50-alu-shift-rotate-6502.png "Istruzioni di scorrimento e rotazione del 6502"){:width="40%"}

*Istruzioni di scorrimento e rotazione del 6502.*

L'MSB di H / output H-Q7 evidenziato in giallo nello schema è connesso al modulo dei Flag e viene salvato dal microcode prima di eseguire lo scorrimento:

~~~text
| ---- | -------------------------- |
| Step | Microistruzione            |
| ---- | -------------------------- |
| 0*   | RPC | WM                   |
| 1*   | RR  | WIR | PCI            |
| 2    | C1  | FS  | FC             |
| 3    | HL  | CC                   |
| 4    | FNZ | RH  | WA  | NI       |
| ---- | -------------------------- |
~~~

*Scomposizione dell'istruzione ASL Accumulatore nelle sue cinque microistruzioni elementari*.

1. Il primo step carica l'indirizzo del Program Counter nel Memory Address Register
2. Il secondo step carica l'opcode dell'istruzione nell'IR e incrementa il PC per farlo puntare alla locazione di memoria successiva (che nel caso dell'istruzione ASL Accumulatore contiene l'istruzione successiva)
3. Il terzo step memorizza il valore di H7 nel Carry:
    - C1 - seleziona la [provenienza del Carry](../flags/#carry) dall’MSB (H-Q7) del registro H
    - FS, Flag Select - origine del Flag, in questo caso [da un altro modulo](../flags/#componenti-e-funzionamento)
    - FC, Flag C - aggiorna il Flag C
4. Il quarto step trasla il contenuto del registro H verso sinistra e carica uno zero nell'LSB
    - CC, Carry Clear - presenta un [valore 0](../flags/#il-carry-e-i-registri-h-e-alu) all'input di H
    - HL, H Left - esegue lo scorrimento
5. Il quinto ed ultimo step scrive i Flag N e Z ed aggiorna A
    - RH, Read H - espone il contenuto del Registro H sul bus
    - FNZ, Flag N & Z - aggiorna i Flag N e Z computati nel modulo Flag
    - WA, Write A - scrive il contenuto del bus in A
    - NI, Next Instruction - resetta il Ring Counter

\* I primi due step di tutte le istruzioni sono sempre uguali, come annotato in [Ring Counter e Microistruzioni](../control/#ring-counter-e-microistruzioni).

 Nello schema il '194 è rappresentato con gli output Q0, Q1, Q2 e Q3, che sono rispettivamente equivalenti agli ouput Q<sub>A</sub>, Q<sub>B</sub>, Q<sub>C</sub> e Q<sub>D</sub> indicati nel <a href="https://www.ti.com/lit/ds/symlink/sn74ls194a.pdf" target="_blank">datasheet</a> del '194.

Vista la flessibilità e l'utilità del Registro H, questo è stato implementato anche nel BEAM, con una differenza nella scrittura del microcode: l'NQSAP implementa scorrimento e rotazione a sinistra sfruttando l'operazione A Plus A dei '181, mentre il BEAM sfrutta i '194 sia verso sinistra sia verso destra.

## Funzioni logiche e operazioni aritmetiche

Come detto nell'introduzione, il computer BEAM, al pari dell'NQSAP, include il set di istruzioni completo del 6502, comprese quelle logiche e aritmetiche. Ricordavo discretamente le principali operazioni del 6502 e sapevo *abbastanza* bene quale dovesse essere il risultato di quello che stavo facendo, ma in quel momento non avevo ancora idea di come fosse possibile ottenerlo.

Avevo intanto deciso di comprendere le operazioni messe a disposizione dal '181 e se vi fosse una logica nella loro disposizione, una sorta di raggruppamento.

[![Funzioni logiche e operazioni aritmetiche del 74LS181](../../assets/alu/50-alu-operations.png "Funzioni logiche e operazioni aritmetiche del 74LS181"){:width="100%"}](../../assets/alu/50-alu-operations.png)

*Funzioni logiche e operazioni aritmetiche del 74LS181.*

Il datasheet del '181 era abbastanza criptico e dunque ho avevo fatto ricorso anche alle molte risorse disponibili in rete riportate a fondo pagina. Dal datasheet si comprendeva che vi sono 4 segnali S0, S1, S2 ed S3 ("*Select*") per la selezione della funzione / operazione e un segnale di controllo della modalità M ("*Mode*", M = HI per le funzioni logiche; M = LO per le operazioni aritmetiche); A e B sono gli input dei dati. Nel datasheet venivano menzionati anche il Carry Look-Ahead e il Ripple-Carry, approfonditi in una delle sezioni dedicate all'[Aritmetica Binaria](../math/#multiple-bit-adder).

Inizialmente avevo trascritto la tabella delle funzioni / operazioni in un foglio Excel per poter lavorare più agevolmente:

![Funzioni logiche e operazioni aritmetiche del 74LS181](../../assets/alu/50-alu-operations-xls.png)

*Funzioni logiche e operazioni aritmetiche del 74LS181 - su Excel.*

Avevo evidenziato le operazioni ripetute più volte, non trovando però alcun raggruppamento o filo conduttore tra righe e colonne. Cercavo  di capire quale fosse il senso di quella disposizione così apparentemente disordinata, ma non l'avevo trovato. Illuminante fu l'articolo di Ken Shirriff <a href="https://www.righto.com/2017/03/inside-vintage-74181-alu-chip-how-it.html" target="_blank">Inside the vintage 74181 ALU chip: how it works and why it's so strange</a>.

L'aspetto più importante che avevo capito è cosa accomunava le due colonne delle operazioni aritmetiche (eseguite in corrispondenza della modalità M = LO):

- senza Carry ("Cn = HI  --> No Carry")
- con Carry   ("Cn = LO  --> With Carry")

Ad un occhio esperto tutto questo sarebbe sembrato ovvio, ma io mi stavo avvicinando per la prima volta al tema dell'ALU e molte cose non mi erano immediatamente chiare.

Per quanto bizzarre possano apparire alcune delle operazioni disponibili (e Ken spiega anche il perché), il filo conduttore tra le due colonne è che l'output del '181 *è sempre lo stesso*, con l'unica differenza data dalla assenza o presenza del Carry in ingresso. Si noti infatti che, in ogni riga, l'output ha sempre una differenza pari ad 1, che corrisponde al valore del Carry; ad esempio, nella decima riga troviamo la semplice operazione di somma **A Plus B**: la differenza tra quanto computato nelle due colonne è 1 (**A Plus B** in caso di nessun Carry in ingresso e **A Plus B Plus 1** in caso di Carry in ingresso).

Lo stesso ragionamento è valido per in tutte le altre operazioni aritmetiche disponibili. Prendiamo come altro esempio la prima riga: in assenza di Carry, l'ALU restituisce quanto presente agli ingressi A, mentre in presenza di Carry aggiunge 1 a quanto presente in A (l'operazione aritmetica **A Plus 1** sarà poi sfruttata per creare l'istruzione INC del computer "iniettando" un Carry artificiale nel '181; similarmente, l'istruzione DEC è stata costruita intorno all'operazione **A Minus 1** della tabella, ma in questo caso senza essere "dopata" da un Carry artificiale).

A questo punto è anche opportuno segnalare che il '181 mette a disposizione due modalità di utilizzo: una con la logica attiva bassa ("Active-Low data") e una con la logica attiva alta ("Active-High data") che è quella utilizzata nell'NQSAP; quest'ultima, per complicare un po' le cose, si attende in ingresso un *Carry In negato*, nel senso che un segnale Cn (Carry In) = LO viene interpretato come Carry attivo, mentre un segnale Cn = HI viene interpretato come Carry non presente. Allo stesso modo, anche il *Carry Out* è negato: Cn+4 è infatti HI per indicare che non c'è Carry in uscita, mentre è LO per indicare che è presente un Carry.

Ritornando alla tabella delle funzioni / operazioni e cercando di seguire le spiegazioni e logica dell'NQSAP, avevo compreso che il sottoinsieme visibile in questa tabella fosse sufficiente per lo scopo prefissato, che era quello di poter emulare le istruzioni del 6502:

![Operazioni logiche e aritmetiche utili del 74LS181](../../assets/alu/50-alu-operations-xls-subset.png)

*Operazioni logiche e aritmetiche utili del 74LS181.*

Successivamente capirò che le istruzioni necessarie erano in realtà ancora meno di quelle che ipotizzavo.

## Relazione diretta (*hardwired*) tra Instruction Register e ALU

Un altro degli aspetti di più difficile comprensione, come anticipato in precedenza, è stata l'associazione diretta tra l'istruzione correntemente contenuta nell'Instruction Register e la funzione logica / operazione aritmetica eseguita dal '181.

Provando a sintetizzare quando disegnato nell'NQSAP, avevo costruito questa tabella per avere un riepilogo dei segnali applicati all'ALU, dei loro valori esadecimali corrispondenti e delle operazioni risultanti, estrapolandola dalla tabella completa delle istruzioni visibile in <a href="https://tomnisbet.github.io/nqsap/docs/in-by-mode-group/" target="_blank">NQSAP Instructions by Address Mode Group</a> e nella quale si notano piuttosto chiaramente i raggruppamenti delle istruzioni dell'NQSAP rispetto alle funzioni logiche / operazioni aritmetiche dei '181:

| Cn | M  | S3 | S2 | S1 | S0 | Operazione  | M-S3/S0 Hex     |
|  - | -  |  - |  - |  - |  - |          -  |   -             |
| 0  | 0  | 0  | 0  | 0  | 1  | A Plus 1    |  0x00 + C*      |
| 0  | 0  | 0  | 0  | 1  | 1  | Tutti 0     |  0x03 + C*      |
| 0  | 0  | 0  | 1  | 1  | 0  | A Minus B   |  0x06 + C*      |
| 0  | 0  | 0  | 1  | 1  | 1  | CMP         |  0x07\*\*       |
| 1  | 0  | 0  | 0  | 1  | 1  | Tutti 1     |  0x03           |
| 1  | 0  | 1  | 0  | 0  | 1  | A Plus B    |  0x09           |
| 1  | 0  | 1  | 1  | 0  | 0  | A Plus A    |  0x0C\*\*\*     |
| 1  | 0  | 1  | 1  | 1  | 1  | A Minus 1   |  0x0F           |
| x  | 1  | 0  | 0  | 0  | 0  | Not A       |  0x10\*\*\*\*   |
| x  | 1  | 0  | 1  | 1  | 0  | A XOR B     |  0x16\*\*\*\*   |
| x  | 1  | 1  | 0  | 1  | 1  | A AND B     |  0x1B\*\*\*\*   |
| x  | 1  | 1  | 1  | 1  | 0  | A OR B      |  0x1E\*\*\*\*   |

*Sintesi operazioni dell'ALU dell'NQSAP.*

Legenda tabella *Sintesi operazioni dell'ALU dell'NQSAP*:

- \* Avevo evidenziato queste righe per ricordare che su queste tre operazioni si doveva "iniettare" un Carry artificiale (invertito, dunque il segnale effettivamente applicato sul Carry In del primo '181 doveva essere LO).

- \*\* Le istruzioni che modificano lo stato dei flag del 6502 sono molteplici: aritmetiche, logiche, incremento/decremento, rotazione, stack, flag, caricamento/trasferimento registri e *comparazione*. Queste ultime (CMP, CPX e CPY) hanno effetto solo sui flag N, Z e C, che vengono computati effettuando una sottrazione (SBC nella terminologia del 6502) fittizia tra due valori, scartandone il risultato e tenendo in considerazione solo i flag risultanti dalla sottrazione (non avevo mai realmente approfondito come i flag fossero generati e questo esercizio è stato utilissimo). Funzionamento e differenze tra sottrazioni e comparazioni sono piuttosto semplici da comprendere:

  - nelle sottrazioni il valore risultante dalla sottrazione viene calcolato e *mantenuto*;
  - nelle comparazioni il valore risultante dalla sottrazione viene calcolato e *scartato*;
  - per entrambe le istruzioni i flag sono mantenuti.

  Per eseguire le comparazioni si eseguono dunque delle sottrazioni scartando il risultato, tuttavia le operazioni di sottrazione del '181 sono già utilizzate per eseguire le sottrazioni vere e proprie (SBC) e sono codificate nella terza riga della tabella con M-S3/S0 = **00110**:
  
  | Cn | M     | S3    | S2    | S1    | S0    | Operazione | S3/S0 Hex |
  |  - | -     |  -    |  -    |  -    |  -    |          - |   -       |
  | 1  | **0** | **0** | **1** | **1** | **0** | A Minus B  | 0x06 + C* |

  come è possibile eseguire altre operazioni di sottrazione utilizzando un opcode diverso da **00110** pur sapendo che i '181 eseguono le sottrazioni solo con questa codifica in ingresso? Lo vedremo tra poco nella sezione [Istruzioni di comparazione](#istruzioni-di-comparazione).

- \*\*\* L'operazione A + A veniva usata nell'NQSAP per fare lo shift verso sinistra dei bit; vista la presenza dello Shift Register H, ho preferito riversare su di esso tutte le operazioni di rotazione (a destra e a sinistra, sia con Carry sia senza Carry).

- \*\*\*\* Il Carry è ininfluente in quanto si tratta di funzione logica e non di operazione aritmetica.

### Un esempio pratico

Si stava sostanzialmente dicendo che per eseguire una somma ("**A Plus B**", vedi sesta riga; "ADC" nella terminologia del 6502) era necessario avere:

- Cn = 1 (che, ricordiamo, è gestito con stato logico invertito, dunque in questo caso l'ALU considera il Carry non presente)
- M = **0**
- S3/S2/S1/S0 = **1001**

cioè:

| Cn | M     | S3    | S2    | S1    | S0    | Operazione | S3/S0 Hex |
|  - | -     |  -    |  -    |  -    |  -    |          - |   -       |
| 1  | **0** | **1** | **0** | **0** | **1** | A Plus B   |  0x09     |

In pratica, poiché gli ingressi M ed S3/S0 dei '181 sono direttamente connessi all'[Instruction Register](../control), l'istruzione di somma dovrà forzatamente essere codificata nel microcode presentando **01001** sui 5 bit comuni tra Instruction Register e ALU.

![Output dell'Instruction Register verso il modulo ALU con evidenza dei 5 bit di selezione della funzione / operazione dei '181](../../assets/alu/50-alu-cl-ir-out.png)

*Output dell'Instruction Register verso il modulo ALU con evidenza dei 5 bit di selezione della funzione / operazione dei '181.*

Attivando questa istruzione, il risultato esposto in output dai '181 sarebbe stato esattamente **A Plus B**, proprio come indicato nella decima riga / colonna Cn = HI della tabella estratta dal datasheet "*Funzioni logiche e operazioni aritmetiche del 74LS181.*"; se avessimo invece avuto un Carry in ingresso, il risultato esposto sarebbe stato A + B + 1, come indicato nella decima riga / colonna Cn = LO. La somma (almeno in teoria) funzionava e iniziavo anche a vedere in pratica il legame tra le due colonne Cn = LO / Cn = HI: il risultato in output era sempre lo stesso e variava solo in conseguenza del fatto che in ingresso ci fosse un Carry o meno.

Gli altri 3 bit di output dell'IR venivano scelti arbitrariamente, pur se con qualche accorgimento strutturato, per gestire le diverse modalità di indirizzamento disponibili sul 6502 e che dovevano essere emulate dal computer NQSAP.

## Indirizzamenti

Ogni istruzione del 6502, come ad esempio la succitata ADC, offre infatti un certo numero di indirizzamenti tra le 13 modalità totali disponibili (Accumulatore, Assoluto, Assoluto Indicizzato X o Y, Immediato, Implicito, Indiretto, Indiretto Indicizzato X, Indicizzato Y Indiretto, Relativo, Pagina Zero, Pagina Zero Indicizzato X o Y).

Un valido riferimento per l'analisi della relazione tra IR ed ALU è stata la pagina <a href="https://www.masswerk.at/6502/6502_instruction_set.html" target="_blank">6502 Instruction Set</a> di Norbert Landsteiner, che invito a consultare anche per il <a href="https://www.masswerk.at/6502/assembler.html" target="_blank">6502 Assembler</a> e il <a href="https://www.masswerk.at/6502" target="_blank">Virtual 6502</a> che avrei utilizzato in seguito in fase di debug del microcode.

Dalla tabella HTML delle istruzioni avevo ricavato una tabella Excel a partire dalla quale ho ragionato sugli indirizzamenti, notando che ogni istruzione del 6502 non presentava mai più di 8 diverse modalità di indirizzamento: ecco che gli altri 3 bit di output dell'IR permettevano dunque di costruire un set di istruzioni basato sulle funzioni logiche / operazioni aritmetiche del '181 incrociandole con le (non più di) 8 modalità di indirizzamento usate da ogni istruzione (2^3 = 8 modalità di indirizzamento sfruttabili da ogni istruzione).

Lo spreadsheet Excel citato nel paragrafo precedente è [scaricabile qui](../../assets/BEAM computer.xlsx); le tabelle appena menzionate sono presenti nel foglio "6502 Inst. Set".

Vale anche la pena notare che, in un computer con soli 256 byte di RAM, gli indirizzamenti Zero Page e Assoluti risultano ridondanti, poiché entrambi possono accedere agli stessi 256 byte di memoria. Di conseguenza, le modalità di indirizzamento Zero Page (ZP, "Zero Page" in inglese) non verranno implementate, essendo equivalenti a quelle Assolute.

Riprendendo le operazioni ADC / A Plus B e SBC / A Minus B ed integrandole con i 3 bit utilizzati per gestire le modalità di indirizzamento si costruisce - ad esempio - questa tabella degli opcode:

| Cn | I3    | I2    | I1    | M     | S3    | S2    | S1    | S0    | Operazione | Indirizzamento          | Hex   |
|  - | -     | -     | -     | -     | -     | -     | -     | -     | -          | -                       | -     |
| 1  | **0** | **0** | **1** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Immediato               | 0x29  |
| 1  | **0** | **1** | **0** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Assoluto                | 0x49  |
| 1  | **0** | **1** | **1** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Assoluto, X             | 0x69  |
| 1  | **1** | **0** | **0** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Assoluto, Y             | 0x89  |
| 1  | **1** | **0** | **1** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Indiretto Indicizzato X | 0xA9  |
| 1  | **1** | **1** | **0** | **0** | **1** | **0** | **0** | **1** | A Plus B   | Indicizzato Y Indiretto | 0xC9  |
|  - | -     |-      | -     | -     | -     | -     | -     | -     | -          | -                       | -     |
| 0  | **0** | **0** | **1** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Immediato               | 0x26  |
| 0  | **0** | **1** | **0** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Assoluto                | 0x46  |
| 0  | **0** | **1** | **1** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Assoluto, X             | 0x66  |
| 0  | **1** | **0** | **0** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Assoluto, Y             | 0x86  |
| 0  | **1** | **0** | **1** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Indiretto Indicizzato X | 0xA6  |
| 0  | **1** | **1** | **0** | **0** | **0** | **1** | **1** | **0** | A Minus B  | Indicizzato Y Indiretto | 0xC6  |

*Esempio della relazione tra IR ed ALU per tutte le modalità di indirizzamento delle istruzioni ADC e SBC del 6502.*

Si nota che i 5 bit dell'operazione sono sempre gli stessi per ogni modalità di indirizzamento e che solamente i 3 bit di indirizzamento variano a seconda della modalità prescelta.

Partendo da questo ragionamento diventava possibile definire gli opcode di ogni istruzione da codificare poi nel microcode.

## Istruzioni di comparazione

Abbiamo evidenziato in precedenza che le istruzioni di modificano lo stato dei flag sono molte e che tra queste ci sono anche quelle di comparazione.

Per approfondirle, partiamo ad esempio da una istruzione che fa uso dei flag risultanti da una comparazione: l'istruzione di salto condizionale BMI (**B**ranch on **MI**nus). Questa istruzione viene eseguita solo in presenza del flag N, che indica che il numero risultante dalla comparazione *con segno*\* è **N**egativo, cioè compreso tra -128 e -1 (0x80 e 0xFF in esadecimale).

\* = si veda la sezione riservata all'[Aritmetica Binaria](../math/#numeri-unsigned-e-numeri-signed).

Come anticipato, i flag delle istruzioni di comparazione sono calcolati eseguendo una sottrazione fittizia tra il valore contenuto nel registro A, X o Y e il valore indicato dall'operando dell'istruzione di comparazione; si prendono in considerazione solo i flag risultanti dall'operazione e il risultato della sottrazione viene scartato. I flag sono dunque generati sfruttando l'operazione di sottrazione del '181, che è però già utilizzata per eseguire l'operazione standard di sottrazione **A Minus B** (terza riga della tabella *Sintesi operazioni dell'ALU dell'NQSAP* dove M-S3/S2/S1/S0 = 00110 e il microcodice dovrà perciò presentare **00110** sui 5 bit comuni tra Instruction Register e ALU).

Tutti i segnali che pilotano i '181 derivano direttamente dall'Instruction Register (IR), eccetto per il Carry In. Si può dire che l'ALU è *hardwired* all'IR e che pertanto il microcode del computer deve essere scritto in modo tale che le istruzioni che utilizzano l'ALU rispecchino i segnali in ingresso del '181: ad esempio, osservando la tabella precedente *Esempio della relazione tra IR ed ALU per tutte le modalità di indirizzamento delle istruzioni ADC e SBC del 6502*, l'istruzione di somma **A Plus B** dovrà avere i bit comuni tra IR ed ALU codificati come **01001**, mentre l'istruzione di sottrazione **A Minus B** dovrà averli codificati come **00110**.

| Cn | M  | S3 | S2 | S1 | S0 | Operazione  | M-S3/S0 Hex     |
|  - | -  |  - |  - |  - |  - |          -  |   -             |
| 0  | 0  | 0  | 1  | 1  | 0  | A Minus B   |  0x06 + C*      |
| 1  | 0  | 1  | 0  | 0  | 1  | A Plus B    |  0x09           |

Effetto benefico collaterale molto importante del collegamento hardwired tra IR e ALU è che non è necessario dedicare preziose uscite delle ROM per le linee di controllo delle funzioni / operazioni dei '181.

![Ingressi di selezione della funzione logica / operazione aritmetica dell'ALU e connessione "hardwired" con l'IR](../../assets/alu/50-alu-select-in.png)

*Ingressi di selezione della funzione logica / operazione aritmetica dell'ALU e connessione "hardwired" con l'IR.*

Come si nota nell'estratto dello schema *Output dell’Instruction Register verso il modulo ALU con evidenza dei 5 bit di selezione della funzione / operazione dei ‘181*, il segnale S0 è in realtà solo "parzialmente diretto" verso i '181, perché transita prima attraverso una NOR: questa viene pilotata da una ROM (pin 8) ed attivata solo in corrispondenza delle istruzioni di comparazione, cosicché la codifica 0011**1**, da noi arbitrariamente designata per indicare le istruzioni di comparazione, venga presentata ai '181 come 0011**0**, che è il codice per l'istruzione di sottrazione (Subtract Mode) e del quale abbiamo bisogno per eseguire la comparazione!

In altre parole, il microcode delle istruzioni di comparazione (che nella mnemonica del 6502 sono CMP, CPX o CPY) dovrà attivare un segnale ("*LF*") in una delle EEPROM: questo segnale attiverà la porta NOR per trasmettere ai '181 il codice 0011**0** della sottrazione anziché 0011**1**, che corrisponde originariamente al codice dell'operazione **A And Not B**, non necessario per simulare le istruzioni del 6502 e dunque inutilizzato. LF è l'abbreviazione di A**L**U **F**orce.

Detto in altre parole ancora:

- Le istruzioni di comparazione del 6502 sono eseguite simulando una sottrazione.
- L'operazione di sottrazione è codificata nel '181 come M-S3/S0 = **00110** (e non è modificabile).
- Come è possibile gestire sia le sottrazioni reali sia le comparazioni, considerando che entrambe necessitano di mettere in input sui '181 la stessa codifica **01110**, la quale deve però essere assegnata sia alle istruzioni di sottrazione sia a quelle di comparazione, che devono in realtà avere opcode diversi - e dunque anche codifiche diverse?
- Si identifica un opcode arbitrario per le operazioni di comparazione utilizzandone uno che è assegnato a un'operazione non necessaria, ad esempio **A And Not B**, che ha come codice M/S3-S0 = **00111**.
- La differenza tra l'operazione A Minus B e l'operazione A And Not B sta nell'ultimo bit: la prima si attiva con M-S3/S0 = 0011**0**, la seconda con M-S3/S0 = 0011**1**.
- Quando l'IR carica una istruzione 00111 di comparazione, metterà tale codifica in output verso le EEPROM e verso l'ALU, ma l'ultimo bit di tale codifica raggiungerà l'ALU solo dopo aver attraversato la NOR.
- Una delle EEPROM ospitanti il microcode, quando troverà in ingresso xxx00111, attiverà il segnale LF sul pin 8 della NOR: la NOR invertirà l'ultimo bit 0011**1** e i '181 troveranno in realtà in ingresso 0011**0**, configurandosi dunque in Subtract Mode ed effettuando la sottrazione, della quale scarteremo il risultato per mantenere solo i flag.

## Le istruzioni di comparazione e i Flag

Inizialmente avevo incontrato qualche difficoltà nel comprendere la logica della variazione dei Flag nelle istruzioni di comparazione. Un supporto eccellente si trova in un <a href="http://www.6502.org/tutorials/compare_beyond.html" target="_blank">tutorial</a> su 6502.org, che descrive come un'operazione di confronto equivalga ad impostare il Carry e ad [eseguire la differenza](../alu/#relazione-diretta-hardwired-tra-instruction-register-e-alu), mantenendo solamente i Flag modificati e scartando il valore della sottrazione.

Se dopo l'operazione di comparazione CMP NUM, che equivale a SEC seguito da SBC NUM:

- il Flag Z è 0, allora A <> NUM e il salto condizionale BNE viene eseguito
- il Flag Z è 1, allora A = NUM e il salto condizionale BEQ viene eseguito
- il Flag C è 0, allora A (senza segno) <  NUM (senza segno) e il salto condizionale BCC viene eseguito
- il Flag C è 1, allora A (senza segno) >= NUM (senza segno) e il salto condizionale BCS viene eseguito
- il Flag N è 0, allora A (senza segno) >= NUM (senza segno) e il salto condizionale BPL viene eseguito
- il Flag N è 1, allora A (senza segno) <  NUM (senza segno) e il salto condizionale BMI viene eseguito

Dopo aver metabolizzato l'argomento, mi sono dilettato in alcune prove.

Il codice seguente compara l'operando di CPY con Y e, se Y >= operando, il salto condizionale viene eseguito:

~~~text
LDY #$40
CPY #$30
BCS $60
~~~

Prima di eseguire la sottrazione simulata, il microcode dell'istruzione CPY imposta il Carry; poiché il valore dell'operando è inferiore al valore contenuto in Y, la sottrazione non ricorre al "prestito" (borrow) del Carry, che alla fine dell'operazione risulta ancora impostato, così come lo era all'inizio dell'istruzione. Trovando il Carry attivo, la successiva istruzione BCS (Branch on Carry Set) viene eseguita.

Come sopra, il codice seguente compara l'operando di CPY con Y:

~~~text
LDY #$40
CPY #$40
BNE $60
~~~

La comparazione attiva i Flag Z e C: #$40 - #$40 = 0, dunque il risultato della sottrazione simulata è pari a zero e Z viene settato; inoltre, poiché il numero da comparare è uguale, non si ricorre al prestito e C rimane attivo. Trovando Z attivo, la successiva istruzione BNE (Branch on Not Equal) non viene eseguita.

Il codice:

~~~text
LDY #$40
CPY #$50
BMI $60
~~~

non attiva né Z, né C, coerentemente con quanto esposto in precedenza; attiva invece N, perché la sottrazione simulata genera un risultato negativo: l'MSB assume valore 1, attivando così il Flag N. Il Flag C, settato all'inizio della comparazione, assume il valore 0 perché viene "preso in prestito". Trovando N attivo, la successiva istruzione BMI (Branch on MInus) viene eseguita.

## Riepilogo: sottrazioni, comparazioni e indirizzamenti

La documentazione dell'NQSAP segnalava che "poiché la ALU è legata all'IR, ci sono solo 8 Opcode disponibili per metterla in Subtract Mode", ma non capivo cosa volesse dire. "Per creare i 16 Opcode necessari per tutte le combinazioni di Subtract e Compare, si mette una NOR su ALU-S0 (IR 0) e su LF, così da  riutilizzare la Selection 0111 come se fosse 0110, che è la modalità Subtract".

Importante evidenziare che la modalità **Subtract Mode** del '181 altro non è che la configurazione di Input M-S3/S0 = 00110 che equivale alla operazione aritmetica di sottrazione, come chiarito da David Courtney nel video <a href="https://www.youtube.com/watch?v=jmROTNtoUGI" target="_blank">Comparator Functions of 74LS181 (74HCT181) ALU</a>. Il datasheet non era così chiaro relativamente alla definizione di questa modalità.

La tabella successiva evidenzia come con la disponibilità di 8 bit per la codifica delle istruzioni (256 combinazioni possibili), solo 8 combinazioni (2^3, descritte dai bit I2-I1-I0) siano quelle degli opcode che permettono di avere M e S3/S0 in Subtract Mode, cioè **00110**: tuttavia, la gestione di tutte le operazioni di sottrazione SBC e di comparazione CMP, CPX e CPY richiede ben più di 8 combinazioni, poiché si devono poter gestire anche tutte le combinazioni degli indirizzamenti del 6502. Ecco che il segnale LF (ALU Force) trasforma la codifica 00111 (corrispondente all'operazione inutilizzata A AND NOT B) in 00110, che attiva nuovamente l'operazione aritmetica di sottrazione del '181 e che ci permette di ottenere 8 + 8 = 16 opcode totali da inserire nel microcode per la gestione di sottrazioni e comparazioni in tutte le <a href="https://www.masswerk.at/6502/6502_instruction_set.html#modes" target="_blank">modalità di indirizzamento</a> previste nel 6502.

| Bit IR  | 7      | 6      | 5      | 4  | 3   | 2   | 1   | 0   |
|  -      | -      | -      | -      | -  | -   | -   | -   | -   |
| Segnale | I2     | I1     | I0     | M  | S3  | S2  | S1  | S0  |
| Opcode  | **0**  | **0**  | **0**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **0**  | **0**  | **1**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **0**  | **1**  | **0**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **0**  | **1**  | **1**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **1**  | **0**  | **0**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **1**  | **0**  | **1**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **1**  | **1**  | **0**  | 0  | 0   | 1   | 1   | 0   |
| Opcode  | **1**  | **1**  | **1**  | 0  | 0   | 1   | 1   | 0   |

*Codifica dell'istruzione di sottrazione SBC con le 8 combinazioni possibili di indirizzamento determinate dai 3 bit I0, I1, I2.*

Letto dopo averlo capito mi sembra ora molto semplice; inizialmente non lo era proprio.

## Carry, addizioni e sottrazioni

Alcuni esempi chiariranno il funzionamento del Carry utilizzando due '181 messi in cascata tra di loro per comporre una word di 8 bit.

![Interconnessione di due ALU '181 in cascata](../../assets/alu/50-alu-nqsap-cascade.png)

*Interconnessione di due ALU '181 in cascata.*

Supponiamo di fare un'operazione **A Plus B** con due ALU. Si deve inviare in ingresso sul /Cn del primo '181 un segnale allo stato logico HI, che corrisponde a non avere un Carry (ricordiamo che nella logica "Active-High Data" il Carry è negato). Il **/Cn+4** (Carry Out) del primo '181 entra nel **/Cn** (Carry In) del secondo:

- Se l'operazione eseguita sui primi 4 bit non genera un riporto, il /Cn+4 del primo '181 è HI ad indicare assenza di Carry; questo segnale viene propagato al secondo '181, che trova dunque l'ingresso /Cn allo stato HI: entrambi i '181 eseguono **A Plus B**, dunque una normale operazione senza riporto.

- Viceversa, se l'operazione eseguita sui primi 4 bit genera un riporto, il /Cn+4 del primo '181 è LO ad indicare presenza di Carry; questo segnale viene propagato al secondo '181, che trova dunque l'ingresso /Cn allo stato LO: il primo '181 esegue dunque l'operazione **A Plus B**, mentre il secondo esegue l'operazione **A Plus B plus 1**: il '181 inferiore va sostanzialmente a generare un riporto che viene propagato al '181 superiore.

Per eseguire invece una sottrazione **A minus B** dobbiamo attivare preventivamente il Carry, cioè settare /Cn = LO.

- Se il primo '181 non genera un prestito ("borrow"), il /Cn+4 è allo stato logico LO, che viene propagato al /Cn del secondo '181 che esegue dunque l'operazione **A Minus B**.
- Se invece il primo '181 genera un borrow, il /Cn+4 è allo stato logico HI, che viene propagato al /Cn del secondo '181 che esegue l'operazione **A Minus B - 1**: il '181 inferiore va sostanzialmente a prendere un prestito dal '181 superiore.

Possiamo dire che il Carry In del '181 inferiore è utilizzato, insieme ai bit M-S3/S0, per selezionare l'operazione che il modulo ALU dovrà eseguire.

Questi ed altri punti sono spiegati molto bene da Tom nella sezione <a href="https://tomnisbet.github.io/nqsap/docs/74181-alu-notes/#carry-flag" target="_blank">Carry Flag</a> della sua pagina *74181 ALU Notes* dedicata all'ALU.

L'uso del Carry nel '181 (e di conseguenza nell'NQSAP) è simile a quanto avviene nel 6502, con il Carry che viene normalmente azzerato (CLC) prima di fare una addizione e settato (SEC) prima di fare una sottrazione:

- se al completamento della addizione il Carry risultante in uscita dal '181 superiore è settato, significa che vi è un riporto che si propaga oltre gli 8 bit degli operandi;
- viceversa, se al completamento della sottrazione il Carry risultante in uscita dal '181 superiore è azzerato, significa che è stato chiesto un prestito che si propaga oltre gli 8 bit degli operandi.

### L'Overflow

Dalle interessantissime note di Tom trascrivevo anche che, per le caratteristiche di funzionamento del '181 e provando a fare delle addizioni o sottrazioni con e senza Carry, si potesse pensare di eseguire un semplice OR esclusivo (XOR) tra i Carry Out (/Cn+4) dei due chip per individuare se il risultato dell'operazione genera un Overflow. Tuttavia il meccanismo non funziona in caso di istruzioni **A + 1** e **A - 1** (e dunque per la verifica dell'esistenza dell'Overflow si ricorrerà ad un altro metodo, come discusso nella pagina dedicata all'[Aritmetica Binaria](../math/#approfondimento-overflow)).

Per quale motivo la verifica suddetta non è valida in caso di istruzioni di incremento e decremento?

- Eseguendo un'operazione **A + 1** (si vedano i segnali da applicare al '181 *Sintesi operazioni dell'ALU dell'NQSAP*) si possono verificare due casi - facciamo due esempi:

  - A = 0000.0101 che, incrementato di un valore 1, diventa 0000.0110; l'incremento avviene iniettando un segnale LO sul Carry In (/Cn) del primo '181; il suo Carry Out (/Cn+4) è HI, cioè non attivo; il risultato dell'operazione svolta dal secondo '181 non comporta un Carry, pertanto il suo Carry Out (/Cn+4) è ancora HI.
  - A = 0000.1111 che, incrementato di un valore 1, diventa 0001.0000; l'incremento avviene iniettando un segnale LO sul Carry In (/Cn) del primo '181; il suo Carry in uscita (/Cn+4) è LO, cioè attivo; il risultato dell'operazione svolta dal secondo '181 non comporta un Carry, pertanto il suo Carry Out (/Cn+4) è HI.

- Nel primo caso entrambi i /Cn+4 sono LO e dunque una XOR con gli ingressi connessi a tali uscite non segnalerebbe uno stato di Overflow, correttamente.

- Nel secondo caso non si ha un reale Overflow incrementando la word iniziale da 0000.1111 a 0001.0000, ma se si andassero ad interpretare i Carry Out dei due '181 con una funzione XOR, si incorrerebbe in un errore, in quanto i due segnali sono invertiti e la XOR segnalerebbe Overflow, sbagliando.

Tutto questo è spiegato molto bene da Tom nella stessa pagina citata poche righe più sopra; come detto poc'anzi, l'argomento dell'Overflow è anche diffusamente ripreso [in una sezione dedicata](../math/#approfondimento-overflow).

## Schema

[![Schema dell'ALU del computer BEAM](../../assets/alu/50-alu-beam-schematics.png "Schema dell'ALU del computer BEAM"){:width="100%"}](../../assets/alu/50-alu-beam-schematics.png)

*Schema dell'ALU del computer BEAM.*

## Differenze tra Moduli ALU dell'NQSAP e del BEAM

Come si può vedere dallo schema del modulo ALU del computer BEAM, questo è quasi una copia 1:1 del modulo ALU del computer NQSAP: non avevo certamente la capacità di sviluppare autonomamente un modulo ALU così complesso e legato a doppio filo con altri moduli del computer, ma la comprensione completa del funzionamento dell'ALU sviluppata da Tom ha rappresentato comunque un traguardo molto importante.

Ecco una lista delle differenze:

- Per il registro B è stato utilizzato un registro tipo D <a href="https://www.ti.com/lit/ds/symlink/sn54ls377.pdf" target="_blank">74LS377</a> al posto del <a href="https://www.onsemi.com/pdf/datasheet/74vhc574-d.pdf" target="_blank">74LS574</a> utilizzato da Tom. A differenza del '574, il '377 è dotato di ingresso Enable, che solo quando attivo permette il caricamento del registro in corrispondenza del Rising Edge del clock: così facendo si elimina la necessità di un gate in ingresso sul clock per realizzare un Enable artificiale, come descritto nella sezione [L'ALU dell'NQSAP](#lalu-dellnqsap).

![Schema di uno degli 8 Flip-Flop del 74LS377](../../assets/alu/50-alu-377.png){:width="66%"}

*Schema di uno degli 8 Flip-Flop del 74LS377.*

- Il computer NQSAP prevedeva 8 step per le microistruzioni, mentre il BEAM ne prevede 16. Con soli 8 step non sarebbe stato possibile emulare alcune istruzioni di scorrimento e rotazione del 6502. Questa è in realtà una differenza architetturale più legata alla Control Logic, però il maggior numero di step disponibili ha un importante impatto su questo modulo e ha dunque sicuramente senso citarla anche in questa sezione.

- Tom ha utilizzato l'operazione A Plus A dei '181 per implementare lo scorrimento a sinistra delle istruzioni ASL e ROL del 6502 e il registro H per lo scorrimento a destra delle istruzioni LSR e ROR, mentre il BEAM utilizza H in [entrambe le situazioni](#il-registro-h).

## Link utili

- <a href="https://www.righto.com/2017/03/inside-vintage-74181-alu-chip-how-it.html" target="_blank">Inside the vintage 74181 ALU chip: how it works and why it's so strange</a> di Ken Shirriff. Fondamentale per capire il perché dell'implementazione apparentemente così strana del chip; eccellente anche lo **schema interattivo**, che permette di visualizzare l'attivazione dei segnali interni e degli output come conseguenza degli input applicati.
- La pagina delle <a href="https://tomnisbet.github.io/nqsap/docs/74181-alu-notes" target="_blank">note sul 74181</a> di Tom Nisbet.
- <a href="https://web.archive.org/web/20160326004629/http://www.ti.com/product/sn74ls181" target="_blank">Home page</a> del 74181 su ti.com preservata da Internet Archive.
- <a href="https://www.youtube.com/watch?v=Fq0MIJjlGsw" target="_blank">Demo of 74LS181 (74HCT181) ALU</a> e <a href="https://www.youtube.com/watch?v=jmROTNtoUGI" target="_blank">Comparator Functions of 74LS181 (74HCT181) ALU</a>: due ottimi video di David Courtney.
- Il <a href="https://www.atarimania.com/documents/6502%20(65xx)%20Microprocessor%20Instant%20Reference%20Card.pdf" target="_blank">compendio della Micro Logic</a>, dal quale è tratta l'immagine sulle modalità di [scorrimento e rotazione](#il-registro-h) del 6502.
