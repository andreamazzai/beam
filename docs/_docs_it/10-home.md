---
title: "BEAM"
permalink: /
excerpt: "Il mio breadboard computer TTL a 8 bit basato sulla realizzazione di Ben Eater e sui miglioramenti di Tom Nisbet"
header:
#   image: /assets/home/beam.png
  overlay_image: /assets/home/beam.png
  overlay_filter: 0.5
---
## BEAM 8-bit computer

<small>[Il primo progetto: 6502](#il-primo-progetto-6502) - [Computer a 8 bit in logica TTL: SAP-1](#computer-a-8-bit-in-logica-ttl-sap) - [Evoluzione del computer a 8 bit: BEAM](#evoluzione-del-computer-a-8-bit-beam) - [BEAM?](#beam) - [Studio e progettazione](#studio-e-progettazione) - [Realizzazione](#realizzazione) - [Note](#note) - [Crediti e ringraziamenti](#crediti-e-ringraziamenti) - [Letteratura](#letteratura) - [Avviso sui Diritti di Proprietà Intellettuale](#avviso-sui-diritti-di-proprietà-intellettuale)</small>

<!-- TLDR: vai subito alla [documentazione del BEAM 8-bit computer](https://andreamazzai.github.io/beam/) su GitHub Pages -->

>NOTA: in alcuni punti della documentazione si presuppone che il lettore abbia realizzato o comprenda il progetto del SAP-1 Computer di Ben Eater. Ove possibile, vengono fornite delle referenze per facilitare la consultazione.

Dopo diversi decenni dalla fine degli studi (diploma di scuola superiore ad indirizzo elettronico conseguito nel 1989), nel 2021 ho deciso di rimettere mano all'elettronica, in particolar modo a quella digitale. Ho iniziato ad interessarmi ad Arduino e Raspberry Pi, riscontrando subito una certa difficoltà: le mie basi di programmazione e di amministrazione di sistemi erano da una parte ben poco sviluppate, dall'altra molto arrugginite; non avevo alcuna conoscenza di C o di altri linguaggi compilati e la mia conoscenza di Linux era molto modesta.

### Il primo progetto: 6502

Verso la fine dell'anno ho scoperto casualmente il blog di <a href="https://eater.net/" target="_blank">Ben Eater</a> e i suoi <a href="https://www.youtube.com/@BenEater/playlists" target="_blank">video</a> e, memore del mio primo computer (Commodore 64, 1984) e del fatto che all'epoca avevo fatto un po' di programmazione in assembly, ho costruito il breadboard computer di Ben basato su 6502 e ho iniziato a leggere parecchi blog e forum sul tema, riesumando un po' di vecchie conoscenze.

### Computer a 8 bit in logica TTL: SAP

Nei primi mesi del 2022, dopo aver completato i primi moduli base del computer 6502, ho seguito la <a href="https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU" target="_blank">playlist</a> del computer **SAP-1** (Simple As Possible) in logica TTL a 8-bit, al quale mi sono appassionato ancora di più. Durante la costruzione ho appreso moltissimo - sia per l'eccellente stile di esposizione di Ben, sia per il grande impegno che ho profuso nell'approfondimento della materia su altri siti e libri.

[![La mia realizzazione del computer SAP-1](assets/home/sap.png "La mia realizzazione del computer SAP-1"){:width="66%"}](assets/home/sap.png)

*La mia realizzazione del computer SAP-1.*

### Evoluzione del computer a 8 bit: BEAM

Ho completato il SAP-1 computer a settembre, ma già in precedenza avevo cominciato a studiare i miglioramenti e le espansioni fatte da altri follower di Ben, soprattutto in merito alla possibilità di automatizzare l'inserimento del programma da eseguire con un bootloader, nonché alla necessità di incrementare la RAM oltre i 16 byte; durante l'estate di quell'anno ho scoperto il fantastico progetto <a href="https://tomnisbet.github.io/nqsap/" target="_blank">NQSAP</a> di **Tom Nisbet**: un computer a 8-bit basato su quello di Ben Eater, ma ingegnerizzato per emulare le istruzioni del 6502. Ho realizzato velocemente che sarebbe stato il mio successivo progetto, perché metteva insieme molti aspetti che trovavo di grande interesse:

1. Evoluzione del computer di Ben Eater, col quale avevo iniziato a riacquistare confidenza con l'elettronica digitale.
2. Emulazione del processore del mio primo computer, completa di registri indice, modalità di indirizzamento e Stack Pointer.
3. Nel progetto originale di Ben Eater, il computer disponeva di soli 16 byte di RAM. Questa quantità era sufficiente per programmare una sequenza di Fibonacci minimale, ma assolutamente inadeguata per realizzare programmi più complessi. Un'espansione della RAM, quindi, diventava essenziale.
4. Possibilità / necessità di imparare almeno le basi del C, utilizzato per la creazione del [programmatore di EEPROM](docs/eeprom-programmer) e del [Loader](docs/loader) basato su Arduino.

### BEAM?

Cercando un nome per il mio progetto, ho identificato in "BEAM" quello ideale: desideravo dare merito a Ben Eater per avermi fatto riavvicinare a questo meraviglioso mondo e aggiungere qualcosa di mio, perciò è nato il **B**en **E**ater **A**ndrea **M**azzai ==> **BEAM**.

## Studio e progettazione

Nei primi otto mesi di lavoro sul progetto BEAM, mi sono dedicato interamente allo studio. L'obiettivo era comprendere ogni dettaglio dell'NQSAP, che introduceva numerose aggiunte al SAP-1 e implementava diverse idee davvero ingegnose. Questo periodo di analisi è stato cruciale per padroneggiare i nuovi concetti e le innovazioni del progetto.

La documentazione dell'NQSAP non era completa e avevo scovato anche alcuni errori condividendoli con Tom, che li aveva prontamente corretti. Per aumentare la mia comprensione della materia, dopo lo studio dell'NQSAP avevo analizzato anche l'evoluzione NQSAP-PCB, alla quale Tom aveva apportato diversi miglioramenti. Nella realizzazione del BEAM, ho cercato di prendere il meglio da entrambi i progetti.

Tra gli aspetti da citare e sui quali ho speso molto, **molto**, ***davvero molto*** tempo:

- comprensione del funzionamento dell'ALU 74181, per la quale ho realizzato un bench di test sulla base di quanto appreso da [David Courtney](docs/alu/#link-utili);
- comprensione dell'aritmetica binaria in complemento di 2;
- comprensione dell'overflow, strettamente legato al punto precedente;
- integrazione dell'ALU 74181 e dell'Instruction Register (IR) in modalità "[hardwired](docs/alu/#relazione-diretta-hardwired-tra-instruction-register-e-alu)" per poter generare "automaticamente" i segnali di ingresso dell'ALU a seconda dell'istruzione presente nell'IR;
- nuovo modulo di memoria basato su SRAM con pin IO comuni, a differenza dei 74189 utilizzati nel SAP-1 con porte di Input e di Output dedicate;
- il registro dei flag - sicuramente il più complesso del computer:
  - utilizzo del 74151 per decodificare lo stato di overflow e la gestione del flag stesso - argomento strettamente legato alla perfetta comprensione dell'aritmetica binaria;
  - utilizzo del 74151 per poter gestire le istruzioni di [salto condizionale in modalità hardwired](docs/flags/#i-salti-condizionali-e-incondizionati), similarmente a quanto fatto per la ALU;
- comprensione dello Stack Pointer e scrittura del microcode per le istruzioni che ne fanno uso;
- sviluppo del software del programmatore di EEPROM / microcode basato su Arduino: ho studiato a fondo il codice di Tom, che ho compreso in buona parte:
  - schematizzazione delle istruzioni del 6502 e suddivisione in categorie;
  - comprensione del metodo molto smart sviluppato da Tom per poter indirizzare più di 16 istruzioni aritmetiche avendo a disposizione solo 5 segnali da dedicare ai segnali di selezione dell'operazione dell'ALU;
- comprensione del problema del "glitch";
- sviluppo del software del bootloader;

[![Schema logico del computer BEAM, luglio 2023](assets/home/hand-drawn-logic.jpg "Schema logico del computer BEAM, luglio 2023"){:width="66%"}](assets/home/hand-drawn-logic.jpg)

*Schema logico del computer BEAM, luglio 2023.*

A luglio 2023 ho cercato di ricomporre tutto ciò che avevo appreso e ho provato trascriverlo su carta: avevo compreso molto bene il funzionamento di alcuni moduli, mentre su altri avevo ancora molti dubbi. Inoltre, non mi erano ancora chiare diverse interazioni tra i moduli, soprattutto nei segnali di controllo dei flag; rileggendo il materiale che avevo studiato e gli appunti che avevo trascritto su OneNote nei mesi precedenti, ho unito i puntini e realizzato lo schema logico del computer, iniziando a intravedere non più una serie di moduli a se' stanti, bensì un costrutto logico sensato:

[![Rappresentazione su carta di una possibile disposizione dei moduli del computer BEAM](assets/home/beam-paper-breadboards.png "Rappresentazione su carta di una possibile disposizione dei moduli del computer BEAM"){:width="66%"}](assets/home/beam-paper-breadboards.png)

*Rappresentazione su carta di una possibile disposizione dei moduli del computer BEAM.*

## Realizzazione

La fase costruttiva è iniziata a ottobre 2023 e si è conclusa nei primi giorni di maggio 2024. Il risultato è quello visibile in foto.

In queste pagine si trovano note, appunti e insegnamenti appresi ed applicati nella realizzazione del BEAM, analisi dei moduli dell'NQSAP e dell'NQSAP-PCB, differenze tra i sistemi, qualche confronto con il SAP-1 ed altro ancora.

[![BEAM Breadboard Computer](assets/home/beam.png "BEAM breadboard computer"){:width="66%"}](assets/home/beam.png)

Tra le cose apprese:

1. i flag (del 6502), il loro ruolo e le interazioni all'interno di un computer
2. il ruolo di una ALU e la rappresentazione dei numeri Signed
3. realizzazione di un programmatore di EEPROM molto veloce
4. basi del linguaggio C
5. basi di Arduino
6. utilizzo di Visual Studio Code
7. il linguaggio Markdown
8. utilizzo di GitHub e GitHub Pages
9. disegno degli <a href="https://github.com/andreamazzai/beam/raw/refs/heads/master/schematics/KiCad_BEAM_schematics.zip" target="_blank">schemi con KiCad</a>

### Note

- Il range di frequenza del computer BEAM varia da circa un Hertz a circa 2 KHz. Non sono state testate frequenze superiori, poiché l'obiettivo principale non era tanto quello di ottenere prestazioni elevate, quanto piuttosto di acquisire conoscenze.
- Gli schemi KiCad non includono la gran parte dei LED singoli: sono indicati solamente i LED del modulo Clock e i LED che segnalano le modalità Program Mode / Run Mode nel modulo RAM / MAR. Sono invece rappresentate quasi tutte le LED Bar.

## Crediti e ringraziamenti

- Ben Eater per la sua grande capacità di creazione di <a href="https://www.youtube.com/@BenEater/playlists/" target="_blank">contenuti così interessanti</a> e in particolar modo per la playlist <a href="https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU" target="_blank">Building an 8-bit breadboard computer!</a>;
- Tom Nisbet per l'<a href="https://tomnisbet.github.io/nqsap/" target="_blank">NQSAP</a> e l'<a href="https://tomnisbet.github.io/nqsap-pcb/" target="_blank">NQSAP-PCB</a>, che espandono e migliorano in maniera sostanziale il progetto di Ben Eater;
- il subreddit <a href="https://www.reddit.com/r/beneater/" target="_blank">r/beneater</a> - in particolar modo The8BitEnthusiast - grazie!
- Ken Shirrif per alcuni <a href="https://www.righto.com/" target="_blank">interessantissimi articoli</a> su Overflow, 6502 e 74181 (e molto altro!);
- l'esplosivo Dr. Brock LaMeres dell'università del Montana per l'eccellente playlist <a href="https://www.youtube.com/playlist?list=PL643xA3Ie_Et2uM4xu1yFk-A5ZQQ8gQ5e" target="_blank">Intro to Logic Circuits</a> su YouTube; un <a href="https://www.montana.edu/blameres/book_content_vhdl.html" target="_blank">indice più leggibile</a> dello stesso materiale è disponibile presso il sito ufficiale dell'Università del Montana;
- il sito <a href="http://6502.org" target="_blank">6502.org</a> e il suo forum: documentazione dettagliata senza fronzoli e utenti davvero esperti;
- Dieter Müller, le cui pagine sul <a href="http://6502.org/users/dieter/v_flag/v_0.htmmi " target="_blank">74LS151</a> mi hanno spinto ad approfondire in dettaglio l'argomento Overflow e col quale ho sviluppato una amichevole conoscenza telematica;
- Norbert Landsteiner per il <a href="https://www.masswerk.at/6502/6502_instruction_set.html" target="_blank">6502 Instruction Set</a>, il <a href="https://www.masswerk.at/6502/assembler.html" target="_blank">6502 Assembler</a> e il <a href="https://www.masswerk.at/6502" target="_blank">Virtual 6502</a>, strumenti eccezionali che ho utilizzato ampiamente per la creazione e il debug del microcode. Norbert è stato anche estremamente reattivo nel correggere un bug che gli avevo segnalato.

Ringraziamenti speciali: Tom Nisbet e Dieter Müller, che anche in fase di redazione della documentazione non hanno esitato a darmi qualche chiarimento.

## Letteratura

- <a href="https://archive.org/details/Programming_the_6502_OCR" target="_blank">Programming the 6502</a> di Rodnay Zaks.
- <a href="https://archive.org/details/367026792DigitalComputerElectronicsAlbertPaulMalvinoAndJeraldABrownPdf1/" target="_blank">Digital Computer Electronics</a> di Albert Paul Malvino e Jerald A. Brown.

## Avviso sui Diritti di Proprietà Intellettuale

Questo progetto potrebbe includere link, documenti, o immagini di proprietà di terzi. Mi impegno a rispettare pienamente i diritti di proprietà intellettuale di tutti i proprietari e non intendo violare alcun diritto d'autore.

Se si ritiene che uno o più dei materiali presenti in questa documentazione violi i propri diritti di copyright, prego di segnalarmelo. Provvederò prontamente alla verifica e, in caso di violazione, alla rimozione immediata del materiale in questione.

D'altro canto, tutto il materiale originale prodotto in questo progetto è rilasciato sotto la licenza MIT. Questo significa che è liberamente utilizzabile, modificabile e distribuibile da altri, a condizione che venga mantenuta l'indicazione di copyright originale e questa stessa licenza sia inclusa in eventuali copie o versioni modificate del progetto.
