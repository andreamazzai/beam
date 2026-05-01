---
title: "RAM e MAR"
permalink: /docs/ram/
excerpt: "Modulo di memoria del computer BEAM"
---
<small>[Primi studi](#primi-studi) - [Memorie con IO separati o IO comuni?](#memorie-con-io-separati-o-io-comuni) - [MUX, Program Mode e Run Mode](#mux-program-mode-e-run-mode) - [Gestione della RAM](#gestione-della-ram) - [Design dei moduli MAR e RAM](#design-dei-moduli-mar-e-ram) - [Schema](#schema) - [Note](#note) - [Link utili](#link-utili)</small>

[![Modulo di memoria del computer BEAM](../../assets/ram/20-ram-beam.png "Modulo di memoria del computer BEAM"){:width="100%"}](../../assets/ram/20-ram-beam.png)

Il limite principale del computer SAP-1 di Ben Eater era sicuramente la modesta quantità di RAM indirizzabile, pari a 16 byte; era possibile caricare un semplice contatore da 0 a 255 e viceversa, oppure un piccolo algoritmo di Fibonacci, ma nulla di più. Questo è stato lo stimolo primario per la realizzazione di un computer più potente.

All'approssimarsi del completamento della costruzione del SAP-1, avevo iniziato a documentarmi su vari blog e forum per raccogliere idee su possibili miglioramenti ed espansioni.

### Primi studi

Il <a href="https://www.reddit.com/r/beneater/" target="_blank">canale Reddit</a> dedicato ai progetti di Ben Eater è stato fondamentale in questo mio percorso.

Dal <a href="https://www.reddit.com/r/beneater/comments/crl270/,8_bit_computer_memory_bootloader_and_display/" target="_blank">primo articolo letto</a> avevo tratto queste note:

> Addressable memory - so the idea here is to have 16bit's of addressable memory (about 65KB). This would greatly expand the capabilities compared to the 16 bytes of Ben's PC. This would affect the following things
>> Memory Address register - after the expansion of course the MAR would have to be 16 bits wide. Here i was considering using 2 x LS273 8 bit flip-flop along with an AND gate to allow the chip to have Input Enable signal (that would be implemented by AND-ing the CLK with an Input signal, since the chip has no InputEnable and i was unable to find one that is 8 bit wide, has CLR and inputenable and did not want to use LS173 but might reconsider)

Questo utente desiderava fare una espansione radicale del computer, passando da 16 byte a 64K; il mio desiderio era quello di crescere fino a 256 byte (e non complicarmi troppo la vita con un bus a 16 bit), ma alcune informazioni erano state comunque molto utili per una comprensione generale della questione.

Per indirizzare 64K di memoria era necessario un registro MAR (Memory Address Register) a 16 bit (2^16 = 64K). Invece di utilizzare quattro registri tipo D <a href="https://www.ti.com/lit/ds/sdls067a/sdls067a.pdf" target="_blank">74LS173</a> da 4 bit, come nel SAP-1 originale, sembrava più pratico adottare due registri tipo D <a href="https://www.ti.com/lit/ds/symlink/sn74ls273.pdf" target="_blank">74LS273</a> a 8 bit. Tuttavia, questi ultimi presentavano uno svantaggio rispetto alla versione a 4 bit: mancavano di un ingresso di Enable. Il computer, invece, richiedeva questo segnale, poiché il MAR doveva caricare un indirizzo di memoria solo quando specificamente richiesto, e non ad ogni ciclo di clock. Il segnale MI (Memory Address Register In) del SAP-1 serviva proprio a questo: permetteva al MAR di memorizzare l'indirizzo presente sul bus solo quando necessario, in corrispondenza del fronte di salita del clock. Viceversa, in assenza di un ingresso di Enable, il flip-flop ‘273 avrebbe registrato il dato ad ogni ciclo di clock.

Nel mio progetto a 256 byte bastava un MAR a 8 bit (2^8 = 256), dunque si potevano semplicemente utilizzare due '173 a 4 bit continuando a sfruttare i segnali di Enable nativi. Per completezza, va detto che sarebbe stato possibile utilizzare anche il '273 e una porta AND per *costruire* un segnale di Enable artificiale: collegando i segnali CLK e MI agli ingressi della AND, l'output avrebbe pilotato l'ingresso CLK del FF, che così si sarebbe attivato solo quando entrambi i segnali, CLK e MI, fossero contemporaneamente presenti.

Il '273, al pari del '173, presenta un ingresso Clear / Reset (CLR), che nel MAR è necessario per resettare il registro - o almeno *credevo* fosse necessario. Sembrava anche interessante l'ipotesi alternativa di usare un registro a 8 bit <a href="https://www.ti.com/lit/ds/symlink/sn74ls377.pdf" target="_blank">74LS377</a>, che include 8 FF con Enable; inizialmente credevo che **non** fosse possibile procedere in tal senso, perché nel MAR serviva anche il CLR, non presente in questo chip. In seguito avevo realizzato che il MAR poteva funzionare perfettamente anche senza un segnale di Clear / Reset; il '377 sarebbe diventato uno dei chip più utilizzati nel BEAM.
>> Program counter - would have to be expanded to a 16 bit counter (should be trivial to do that) I currently have tons of 8 bit counters combined with a register (and the 4 bit 161 counters that Ben used)

Come nel caso del MAR, per indirizzare 256 byte di RAM era necessario un registro [Program Counter](../programcounter) (PC) a 8 bit. Nel computer SAP-1 era invece presente un contatore a 4 bit <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">74LS161</a> e dovevo pertanto cercare di combinarne due in cascata.

Sarebbe stato comodo utilizzare un singolo contatore a 8 bit, ma tra i chip disponibili sul mercato non ne ho trovato uno che includesse anche l'ingresso LOAD. Il LOAD permette il caricamento parallelo sul PC di uno specifico indirizzo al quale il computer deve saltare (ad esempio, per eseguire un'istruzione di salto assoluto o branch relativo).

Per combinare due chip a 4 bit è stato necessario, nonché molto utile, comprendere la differenza fra "Ripple Mode Carry" e "Carry Look Ahead": il datasheet del contatore 74LS161 riportava infatti questi due diversi esempi di collegamento di chip in cascata tra di loro. Un approfondimento sul tema si trova in una apposita sezione della pagina dedicata all'[Aritmetica Binaria](../math/#gli-adder).

>> Instruction register - now sh!t gets fun. Since i want to bomb completely, I am considering having a 3 byte Instruction register in a way mimicking the BIU block & instruction queue in a 8086. The idea here is to split the instruction register in 3 bytes. 1 byte would be for the instruction/opcode alone, the second and third are going to hold the data of the instruction (either address or an immediate value). This would allow you to address the entire memory space. Also instructions can be of different size. For example. OUT (move to out register) would be just 1 byte wide, LDI (load immediate) would be 2 bytes and LDA (load address/absolute) would be 3 bytes wide. The control logic would take care of the fetch cycle. Or in other words since you know the instruction you are execution you would know how much bytes the instruction is and thus fetch either 1, 2 or 3 bytes from the RAM.

Dopo aver letto questo punto avevo iniziato a raccogliere i miei pensieri per l'espansione di memoria a 256 byte, idea che riprendo in seguito in questa pagina. Di certo, non avevo intenzione di costruire un IR a più byte, o comunque un IR in grado di ospitare sia l'opcode, sia l'operando. Immaginavo di leggere dapprima l'opcode e di caricarlo nell'Instruction Register e, successivamente, di leggere l'operando e di trattarlo adeguatamente.

### Memorie con IO separati o IO comuni?

Fino ad ora, avevo quasi sostanzialmente dato per scontato di continuare ad usare chip di memoria con porte di Input e Output separati ("dual-port"), esattamente come accade nel <a href="https://eater.net/datasheets/74189.pdf" target="_blank">74189</a> utilizzato nel SAP. Tuttavia, in questo <a href="https://www.reddit.com/r/beneater/comments/hon6ar/74189_alternative/" target="_blank">post su Reddit</a>, un utente evidenziava difficoltà nell'approvvigionamento dei 74189 e chiedeva lumi sull'uso del <a href="https://www.alliancememory.com/wp-content/uploads/pdf/AS6C62256.pdf" target="_blank">62256</a>; ho così iniziato ad approfondire le caratteristiche di questo chip "single-port", aumentando la mia comprensione di queste due diverse architetture.

In origine avevo evidenziato questi pochi appunti presenti nel post, riflettendo sul fatto che l'approccio di questo utente alla gestione dei segnali di controllo mi sembrava un po' troppo semplificato, ma più in là nel tempo avevo realizzato che, tutto sommato, la scrittura sulla RAM non è *eccessivamente* complessa:

> 62256 - The I/O lines are controlled by OE and CE.
When either are high puts the I/O lines at high impedance.
When both are low, the RAM outputs data onto the I/O lines.
Writing takes place (two ways, but this is one way) CE low and OE high. A low pulse on WE will write the data on the I/O lines into the RAM chip.

- **CE** = Chip Enable
- **OE** = Output Enable
- **WE** = Write Enable

L'utente segnala che ci sono due modalità di scrittura; quella evidenziata da lui prevede OE HI, CE LO e l'impulso WE LO; l'altra modalità riportata nel datasheet ("WE Controlled") prevede sia OE sia CE allo stato LO e l'impulso WE LO. In questa seconda modalità l'unico segnale da gestire è WE, che dovrà essere HI nelle fasi di lettura e *pulsato* LO nelle fasi di scrittura. Nel primo e nel secondo schema del modulo RAM di mia ideazione avevo adottato la prima delle due modalità, mentre nel modulo definitivo - e dopo aver compreso meglio i risvolti dell'una e dell'altra - ho utilizzato la seconda modalità. Altre note relative all'argomento si trovano in seguito in questa stessa pagina.

Un altro aspetto che avevo notato immediatamente, ipotizzando l'uso del 62256, era l'impossibilità di mantenere la visibilità del contenuto della cella di RAM indirizzata dal MAR utilizzando i LED (o almeno così credevo): se con i '189 le porte di output erano sempre attive e potevo vedere in ogni momento il valore contenuto della cella di memoria correntemente indirizzata dal MAR, con il 62256 avrei avuto visibilità del contenuto della cella solo nel momento in cui la RAM veniva letta - e dunque non costantemente.

[![Schema del modulo RAM di Ben Eater basato su 74189: le porte nativamente designate per l'Output consentono la visualizzazione ininterrotta del contenuto della locazione RAM indirizzata dal MAR](../../assets/ram/20-be-ram-detail.png "Schema del modulo RAM basato su 74189: le porte nativamente designate per l'Output consentono la visualizzazione continua del contenuto della locazione RAM indirizzata dal MAR"){:width="66%"}](../../assets/ram/20-be-ram-full.png)

*Schema del modulo RAM di Ben Eater: si notano i 74189 con le porte di Input dedicate D1-D4 e le porte di Output dedicate O1-O4.*

Quello che iniziavo a capire era che per utilizzare una RAM con Common IO dovevo fare un "doppio passaggio" o qualcosa di simile. Come faccio ad avere sempre visibile il contenuto della locazione di memoria anche nel momento in cui setto le porte di IO del chip in modalità input? Come faccio a mantenere la visibilità del contenuto della RAM quando questa non è attiva in output? Devo forse memorizzare il contenuto delle uscite della RAM in qualche latch e solo a quel punto disabilitare il chip prima di andarvi a scrivere? In seguito avrei capito che non era necessario un latch, ma che c'era un'altra strada.

In <a href="https://www.reddit.com/r/beneater/comments/uot8pk/ram_module_using_65256/" target="_blank">questo post</a> un utente esponeva un disegno che credevo potesse andare bene, ma <a href="https://imgur.com/upvYjUX" target="_blank">nel suo schema</a> le uscite dei multiplexer (MUX) sono sempre attive (i <a href="https://www.ti.com/lit/ds/symlink/sn74ls157.pdf" target="_blank">multiplexer 74LS157</a> non sono tri-state) e potrebbero creare contenzioso con le uscite della RAM quando questa è attiva in output; la soluzione poteva essere quella di aggiungere un altro <a href="https://www.mouser.com/datasheet/2/308/74LS245-1190460.pdf" target="_blank">bus transceiver 74LS245</a>, oppure di utilizzare dei <a href="https://www.ti.com/lit/ds/symlink/sn54ls257b.pdf" target="_blank">MUX tri-state 74LS257</a>; intuivo qualcosa relativamente alla necessità di gestire i segnali di controllo della RAM in maniera più ampia e controllare le interazioni con i MUX e con il/i transceiver di interfacciamento verso il bus del computer.

[![Disegno su carta del modulo RAM e MAR per capire la migliore disposizione dei chip in base ai collegamenti necessari](../../assets/ram/20-ram-mar-drawing.png "Disegno su carta del modulo RAM e MAR per capire la migliore disposizione dei chip in base ai collegamenti necessari"){:width="66%"}](../../assets/ram/20-ram-mar-drawing.png)

*Disegno su carta del modulo RAM e MAR per capire la migliore disposizione dei chip in base ai collegamenti necessari.*

## MUX, Program Mode e Run Mode

A cosa servono i MUX nel modulo RAM (e nel MAR)? All'accensione, il contenuto della memoria RAM è vuoto / casuale, dunque dobbiamo prima avere la possibilità di programmare la RAM ("Program Mode") e  poi di renderla visibile al bus del computer durante la normale esecuzione dei programmi ("Run Mode").

- La modalità **Program Mode** è manuale e sfrutta dei dip-switch per indirizzare e programmare manualmente la RAM.

- La modalità **Run Mode** è la modalità di esecuzione, nella quale la RAM viene indirizzata esclusivamente dal MAR e altrettanto esclusivamente viene acceduta in lettura / scrittura solo dal bus del computer; da notare che è possibile passare manualmente al Program Mode per leggere o scrivere il contenuto della RAM in stile "debug mode" e riprendere poi la normale esecuzione del programma.

La selezione di cosa passare a RAM e MAR avviene mediante un MUX (nel nostro caso 2:1, cioè ad ogni uscita corrispondono due ingressi selezionabili): gli ingressi del MUX sono connessi sia ai dip-switch che utilizzeremo per la programmazione manuale del computer, sia al bus dati del computer; le uscite sono connesse agli ingressi della RAM e del MAR. Un semplice interruttore connesso all'ingresso di selezione del MUX consente di scegliere quali ingressi attivare.

Ad esempio, nello schema del SAP-1 visibile più in alto in questa pagina, i multiplexer '157 gestiscono gli ingressi della RAM: gli ingressi dei MUX sono connessi sia al dip-switch sia al bus del computer, mentre le uscite sono connesse alle porte di ingresso D1-D4 dei chip di RAM '189.

Riprendevo il tema del "doppio passaggio" (o meglio "doppio bus", come capirò in seguito), che da quanto iniziavo a comprendere poteva rendere possibile la visualizzazione persistente del contenuto della RAM. A pagina 17 e 18 del "Building the SAP-3 rev 3.3.pdf" presente nel repository GitHub di <a href="https://github.com/rolf-electronics/The-8-bit-SAP-3" target="_blank">rolf-electronics</a>, altro utente del canale Reddit, avevo notato che era stato inserito un secondo transceiver '245.

[![Modulo RAM di rolf electronics](../../assets/ram/20-rolf-ram.png "Modulo RAM di rolf electronics"){:width="50%"}](../../assets/ram/20-rolf-ram.png)

Il funzionamento e la necessità dei transceiver mi erano chiarissimi, in quanto ampiamente utilizzati nel SAP-1 computer per poter attivare i vari moduli del computer solo nel momento in cui fosse necessario farlo: tipicamente ogni modulo ha bisogno di un unico transceiver di interconnessione verso il bus.

Lo schema del modulo RAM di Rolf ne prevede invece due, uno "interno" e uno "esterno", per separare il percorso dei dati *verso* la RAM da quello dei dati *dalla* RAM:

- Il transceiver di sinistra è attivo quando si scrive *sulla* RAM, selezionando quale sia la sorgente mediante il MUX '157 (che in "Program Mode" attiva gli ingressi connessi al dip-switch, mentre in "Run Mode" attiva gli ingressi connessi al bus); i LED mostrano il valore che viene scritto sulla RAM.
- Il transceiver di destra è attivo quando si legge *dalla* RAM; i LED mostrano il valore che viene letto dalla RAM e trasferito sul bus del computer.

Un latch per memorizzare lo stato dei LED, come erroneamente ipotizzavo inizialmente, non era necessario.

Proseguendo nello studio, avevo trovato <a href="https://imgur.com/a/ruclh" target="_blank">questo schema</a> dell'utente jaxey1631, che aveva lasciato un commento nel video di Ben Eater <a href="https://www.youtube.com/watch?v=JUVt_KYAp-I&lc=UgjusLoROw6az3gCoAEC" target="_blank">Reprogramming CPU microcode with an Arduino</a>:

[![RAM e MAR con doppio bus](../../assets/ram/20-ram-ruclh.png "RAM e MAR con doppio bus"){:width="100%"}](../../assets/ram/20-ram-ruclh.png)

*Schema di RAM e MAR con bus interno.*

In questo schema troviamo:

- Il chip di RAM 62256.

- Un FF '273 in alto a destra che, sprovvisto di Enable come discusso in precedenza, si attiva in corrispondenza di un Enable fittizio costruito con l'operazione logica "Clock *AND* Memory Address Register In" (ingressi 1A e 1B del NAND a sinistra nello schema); il segnale MI indica che il computer si prepara a settare l'indirizzo di RAM sul quale eseguirà la prossima operazione.

- I due MUX '157 in alto che consentono la selezione degli indirizzi (tra quelli settati sui dip-switch o quelli presenti sulle uscite del FF '273) da esportare verso i pin A0-A7 del chip di RAM. La selezione degli ingressi attivati dal MUX avviene grazie all'interruttore (in alto a sinistra) di selezione della modalità connesso agli ingressi SEL: in Program Mode è acceso il LED rosso e gli ingressi SEL sono allo stato LO, attivando gli ingressi A1-A4 dei MUX; in Run Mode è acceso il LED verde e gli ingressi SEL sono allo stato HI, attivando gli ingressi B1-B4.

- Altri due MUX '157 in basso che consentono la selezione di cosa esportare verso i pin dati D0-D7 del chip di RAM; anche questi MUX sono connessi all'interruttore di selezione della modalità e attivano gli ingressi connessi al dip-switch di selezione degli indirizzi o quelli connessi al bus del computer, a seconda dello stato dell'interruttore (Program Mode o Run Mode).

- Un primo transceiver '245 (in basso) i cui ingressi sono connessi alle uscite dei due MUX citati nel punto precedente. Questo transceiver funge da interfaccia *verso* la RAM (il pin DIR settato a LO configura i pin A1-A8 come ingressi e i pin B1-B8 come uscite) e si attiva nel momento in cui si deve scrivere in memoria; il segnale OE di questo primo transceiver è infatti attivo quando quando si preme il pulsante Write Button (in basso a sinistra) se in Program Mode, o in corrispondenza di Clock + RI (RAM In) (vedi ingressi 4A e 4B del NAND centrale a sinistra nello schema) quando in Run Mode;

- Un secondo transceiver '245 che si attiva nel momento in cui si deve leggere *dalla* RAM e trasferirne l'output verso il bus dati (anche in questo caso il pin DIR del '245 settato a LO configura i pin A1-A8 come ingressi e i pin B1-B8 come uscite); notare il suo ingresso OE connesso al segnale RO (RAM Output) del computer.

[![Write cicle del 62256](../../assets/ram/20-ram-write-cycle.png "Write cicle del 62256"){:width="50%"}](../../assets/ram/20-ram-write-cycle.png)

*Write Cycle "WE Controlled" del 62256.*

Notare la configurazione del chip di RAM: i segnali CE ed OE sono sempre attivi, che significa che l'utente ha deciso di utilizzare la modalità di scrittura definita come "WE# Controlled" definita a pagina 6 del <a href="https://www.alliancememory.com/wp-content/uploads/pdf/AS6C62256.pdf" target="_blank">datasheet</a> del 62256. Rileggendo questi appunti diverso tempo dopo aver completato il mio progetto, mi sembra tutto facile, ma la comprensione delle modalità di scrittura della RAM è stata in realtà abbastanza lunga.

Da notare inoltre che anche questo utente non usa il CLR sui FF '273 di input del MAR - a pensarci, potrebbe realmente non servire, perché ogni volta che ho bisogno di accedere alla RAM, vado preventivamente a settare sul MAR l'indirizzo desiderato. Probabilmente il reset all'accensione era dunque più estetico che altro.

E' stato in questo momento (agosto 2022) che ho scoperto l'**NQSAP**, inserendolo tra i miei appunti come "c'è questo <a href="https://tomnisbet.github.io/nqsap/docs/ram/" target="_blank">https://tomnisbet.github.io/nqsap/docs/ram/</a> che sembra aver fatto delle belle modifiche al suo computer" 😁; ho deciso di seguire questo progetto perché permetteva di costruire un Instruction Set come quello del 6502 che, come scoprirò in seguito, richiederà un numero elevato di indirizzi per il microcode delle EEPROM.

Tra i vari link sondati per l'upgrade della RAM, c'era anche <a href="https://www.reddit.com/r/beneater/comments/h8y28k/stepbystep_guide_to_upgrading_the_ram_with/" target="_blank">Step-by-Step Guide to Upgrading the RAM with Pictures</a>, che molti utenti hanno trovato ben fatto, ma che io ho trovato particolarmente difficile da digerire in quanto mancante di uno schema.

Per aggiungere un ulteriore link utile per la comprensione delle architetture del modulo di RAM, evidenzio questo <a href="https://www.reddit.com/r/beneater/comments/ad2uko/upgrading_the_ram_module_to_256_bytes/" target="_blank">post su Reddit</a>. Le spiegazioni sono molto ben fatte e utili. Il chip di RAM utilizzato è interessante perché si presenta come due RAM distinte, ognuna con accessi dedicati e un segnale di Busy per gestire le richieste parallele sulla stessa locazione. Altro aspetto degno di nota nell'implementazione di questo utente è la possibilità di aumentare fino a 256 il numero di istruzioni del computer, grazie alla scelta di utilizzare un byte intero per l'istruzione ed un eventuale byte successivo per l'operando, anziché avere un unico byte di cui i 4 Most Significant Bit (MSB) rappresentano l'opcode e di cui i 4 Least Significant Bit (LSB) sono l'operando, come nel SAP-1 di Ben Eater.

Un aspetto collaterale (ma importantissimo) dell'aumento del numero di istruzioni era la necessità di aumentare la [dimensione](../control/#instruction-register-e-istruzioni) delle EEPROM ospitanti il microcode: volendo gestire (fino a) 256 istruzioni, erano necessari 8 bit di istruzioni, 3 di step e 2 di flag = 13 pin totali, portanto si rendevano necessarie delle 28C64... e avevo dimenticato che mi sarebbe servito un bit aggiuntivo per la selezione delle due EEPROM! In quel momento, non sapevo ancora che avrei speso *intere settimane* a comprendere il fantastico modulo dei [Flag](../flags) dell'NQSAP di Tom Nisbet, che ha un approccio completamente diverso e che non necessita di segnali in uscita dalle EEPROM.

## Gestione della RAM

Tra i post più utili relativi alla comprensione dei segnali di gestione di RAM e MAR per il modulo di memoria con IO comuni, c'è certamente il <a href="https://www.reddit.com/r/beneater/comments/ut1oud/8bit_question_about_ram_replacement/" target="_blank">Question about RAM replacement</a>, nel quale il moderatore The8BitEnthusiast invita a consultare la sua (*eccellente*, aggiungo io) realizzazione, che ho preso ad esempio e ispirazione.

[![Modulo RAM dell'utente Reddit The8BitEnthusiast](../../assets/ram/20-ram-the8bit_enthusiast.png "Modulo RAM dell'utente Reddit The8BitEnthusiast"){:width="100%"}](../../assets/ram/20-ram-the8bit_enthusiast.png)

*Modulo RAM dell'utente e moderatore Reddit The8BitEnthusiast.*

Da un lato, lo schema appariva semplificato rispetto a quelli che avevo visto in precedenza; dall'altro, l'uso di un solo MUX '157, con segnali che uscivano per rientrare in altre porte, lo rendeva difficile da comprendere analizzare. Avevo pertanto provato a costruirmi i grafici temporali degli schemi dei segnali per questi tre possibili casi:

- Scrittura sulla RAM in Run Mode

[![Scrittura sulla RAM in Run Mode](../../assets/ram/20-ram-run-mode-write-t8be.png "Scrittura sulla RAM in Run Mode"){:width="40%"}](../../assets/ram/20-ram-run-mode-write-t8be.png)

- Lettura dalla RAM in Run Mode

[![Lettura dalla RAM in Run Mode](../../assets/ram/20-ram-run-mode-read-t8be.png "Lettura dalla RAM in Run Mode"){:width="40%"}](../../assets/ram/20-ram-run-mode-read-t8be.png)

- Scrittura sulla RAM in Program Mode

[![Scrittura sulla RAM in Program Mode](../../assets/ram/20-ram-program-mode-write-t8be.png "Scrittura sulla RAM in Program Mode"){:width="40%"}](../../assets/ram/20-ram-program-mode-write-t8be.png)

The8BitEnthusiast segnalava di *aver sfruttato il ritardo di propagazione dei '245 per gestire i requisiti di temporizzazione*, al che avevo provato a chiedergli se fosse necessario gestire le temporizzazioni in maniera così precisa perché il suo progetto lavorava in modalità "just in time" ogni volta che sopraggiungeva un impulso di clock.

Per esempio, ipotizzavo che nel primo caso "Scrittura sulla RAM in Run Mode" accadesse quanto segue.

- **Prima del Rising Edge del CLK**:

  - siamo in Run Mode, dunque PROG è HI
  - il MUX abilita gli ingressi I1a, I1b, I1c
  - Zb è HI (in quanto l'input I1b è connesso a Vcc) e dunque il '245 di destra, che connette il dip-switch di programmazione, è disabilitato
  - il segnale RI è HI (RI = RAM In, cioè scrittura sulla RAM), dunque stiamo preparando una scrittura
  - il segnale /RO è HI (RO = RAM Out, cioè lettura dalla RAM), dunque la lettura dalla RAM è disabilitata
  - Zc = /(CLK LO * RI HI) = HI, dunque la direzione del '245 di sinistra, che connette il data bus, è A-->B (Output)
  - Za = //(/RO HI * Zc HI) = HI, dunque il '245 dal/al data bus è disabilitato

- **CLK attivo**:

  - Zc = /(CLK HI * RI HI) = LO, dunque la direzione del '245 dal/al data bus è B-->A (Input)
  - Za = //(/RO HI * Zc LO) = LO, dunque il '245 dal/al data bus è attivo
  - /WE = Zc = LO, dunque la RAM riceve il Falling Edge del segnale di Write e trova sui suoi ingressi quanto le viene proiettato dal '245 dal/al data bus

- **Quando l'impulso di Clock termina**:

  - Zc = /(CLK LO * RI HI) = HI, dunque la direzione del '245 dal/al data bus è A-->B (Output)
  - /WE = Zc = HI, dunque l'impulso di Write sulla RAM termina con il Rising Edge
  - Za = //(/RO HI * Zc HI) = HI, dunque il '245 dal/al data bus viene disabilitato

Legenda:

- **PROG** è il segnale dell'interruttore di selezione della modalità Program Mode (LO) / Run Mode (HI); negli schemi originali del SAP-1 computer si trova nel MAR
- **/** significa NOT
- **\*** significa AND

[![Scrittura sulla RAM in Run Mode](../../assets/ram/20-ram-run-mode-write-large-t8be.png "Scrittura sulla RAM in Run Mode"){:width="100%"}](../../assets/ram/20-ram-run-mode-write-large-t8be.png)

*Scrittura sulla RAM in Run Mode.*

Da quanto leggevo, immaginavo che il momento critico fosse il Rising Edge del Clock, perché in quel mentre è necessario attendere che la RAM sia pronta per la scrittura e proprio qui è necessario sfruttare il ritardo introdotto dal '245 per mostrare i dati alla RAM "un po' più tardi", ma non capivo esattamente il motivo.

The8BitEnthusiast aveva gentilmente risposto al mio quesito:

> Dovevo assicurarmi che i ‘245 non consegnassero dati alla RAM quando questa non era ancora pronta per accettare dati in Input perché le sue uscite erano ancora attive in output\*\*. Il datasheet segnala che la RAM disabilita l’output ed è pronta per l’input 20 nanosecondi dopo che WE viene portato allo stato LO.

[![Timing RAM 62256](../../assets/ram/20-ram-62256-timing.png "Timing RAM 62256"){:width="66%"}](../../assets/ram/20-ram-62256-timing.png)

\*\* Nello schema del modulo RAM di The8BitEnthusiast si nota che il segnale OE della RAM è connesso a ground, che significa che i pin dati sono sempre attivi in output, *tranne* quando si deve effettuare una scrittura. Quando si attiva il segnale di write WE, vi è un tempo tWHZ durante il quale la RAM è ancora attiva in output; trascorso questo tempo, è possibile mettere dei dati in input sulla RAM.

In altre parole, la RAM è normalmente attiva in output; per scrivere su di essa, la si deve attivare in input col segnale WE. Poiché la RAM impiega 20 nanosecondi per commutare i pin dati da output a input, prima di applicarle dei segnali in input è necessario attendere almeno 20 nanosecondi dal momento in cui si attiva WE (altrimenti si potrebbero provocare dei cortocircuiti: ad esempio avere un pin di output sulla RAM HI e il corrispondente pin di input sul transceiver LO significherebbe portare l'output della RAM direttamente a ground, cortocircuitandola).

[![Write Cycle](../../assets/ram/20-ram-write-cycle-twhz.png "Write Cycle"){:width="50%"}](../../assets/ram/20-ram-write-cycle-twhz.png)

The8BitEnthusiast continuava:

> Lo stesso segnale Zc che attiva la scrittura su RAM (WE) abilita anche i due '245; il datasheet del '245 specifica che la sua attivazione richiede 25 nanosecondi, che è un valore superiore a quello necessario alla RAM per attivarsi in input, dunque il requisito è rispettato.

Detto diversamente, la RAM riceve il segnale di scrittura nello stesso momento in cui il transceiver riceve il segnale di attivazione, ma il transceiver è più lento della RAM e dunque i pin della RAM saranno già nella corretta modalità di input *prima* che il transceiver metta in output quanto presente nei suoi ingressi.

Molto, molto clever.

## Design dei moduli MAR e RAM

Parallelamente agli studi dei lavori di altri utenti, avevo iniziato a lavorare sul disegno dei miei moduli MAR e RAM, non senza continuare ad aprire altri filoni di studio per approfondire temi ancora parzialmente oscuri o affrontare argomenti nuovi. Avevo regolarmente alcune *decine* di pagine web aperte contemporaneamente.

### Il MAR

Il MAR era progettato a 8 bit per pilotare 256 indirizzi di memoria. Inizialmente avevo utilizzato due FF '173 a 4 bit, sostituiti in seguito da un unico FF '273 a 8 bit (sprovvisto di controllo dell'attivazione dell'output, che però non è necessario). Infine ho utilizzato un registro a 8 bit '377, altrettanto adatto al nostro scopo.

[![Memory Address Register](../../assets/ram/20-mar-beam.png "Memory Address Register"){:width="100%"}](../../assets/ram/20-mar-beam.png)

*Memory Address Register (MAR) del BEAM.*

Notare l'interruttore di selezione Program Mode / Run Mode (segnale /PROG, che è connesso anche al modulo RAM):

- alla chiusura dei contatti 1-2 i pin di selezione dei MUX '157 si trovano allo stato logico LO, attivando gli ingressi I0a, I0b, I0c ed I0d, che trasmettono così al bus indirizzi della RAM i valori settati sul dip-switch;
- alla chiusura dei contatti 2-3 i pin di selezione dei MUX '157 si trovano allo stato logico HI, attivando gli ingressi I1a, I1b, I1c ed I1d, che trasmettono così al bus indirizzi della RAM i valori presenti in output sul '377.

### Prima versione del modulo RAM

Come già detto, per quanto riguarda la realizzazione del modulo RAM avevo deciso di procedere con il chip 62256 con IO comuni. Per cercare di fissare i concetti, avevo trascritto nuovamente le differenze tra le architetture con chip con IO separati ed IO comuni:

- Con le RAM dual-port, avevo i segnali di RAM IN e RAM OUT su bus "separati":
  - il bus "IN" collegava le uscite dei MUX 74LS157 ai pin Data In delle RAM: i MUX erano sempre attivi sulle porte "Write" delle RAM e mostravano ad esse tutto quello che accadeva sul bus principale del computer in Run Mode o sul DIP-Switch in Program Mode, ma non era un problema, in quanto le RAM scrivevano solo in corrispondenza del segnale RI (RAM In);
  - il bus "OUT" collegava il transceiver '245 e le uscite Data Out delle RAM; l'uscita del transceiver veniva abilitata solo in corrispondenza del segnale /RO (RAM Out).

- Con la RAM single-port le linee Data sono invece le stesse per Write e Read. A seconda dell'operazione da eseguire, si attivano due percorsi diversi, come già discusso parlando del doppio bus. Per scrivere sulla RAM la sequenza era questa:

  - Output Enable /OE LO fisso;
  - Chip Enable /CE *pulsato* LO ("↘↗");
  - /WE ↘↗ (che deve essere "contenuto" all'interno del ciclo ↘↗ di /CE).

[![Prima versione del modulo RAM](../../assets/ram/20-ram-1st.png "Prima versione del modulo RAM"){:width="100%"}](../../assets/ram/20-ram-1st.png)

*Prima versione del Modulo di memoria (RAM) del BEAM.*

Riponevo grandi speranze su questo primo design.

Per scrivere in RAM in Run Mode:

- RI (che nel ciclo di Write è HI) abilita il '245 superiore mediante una NOT;
- /RO (che nel ciclo di Write è HI) porta /OE a HI, disabilitando l'output del 62256;
- /CE (aka /CS, Chip Select) è fisso LO in quanto connesso a ground;
- i valori presenti nel bus o nel dip-switch vengono presentati via MUX al transceiver che, a sua volta, presenta tali valori alle porte della RAM;
- Zd, e dunque /WE,  è HI, in quanto il pin 1 della NAND è LO
- al Rising Edge del Clock l'uscita della NAND passa da HI a LO per il tempo RC e l'impulso /WE ↘↗ viene trasmesso alla RAM attraverso il MUX '157: la RAM memorizza quanto trova nelle sue porte di ingresso D0-D7.

In Program Mode l'ingresso I0d del MUX è HI grazie alla resistenza di pull-up da 1K, ma premendo il bottone si crea un impulso negativo che attiva un ciclo /WE ↘↗: anche in questo caso la RAM memorizza quanto trova nelle sue porte di ingresso D0-D7.

Per leggere dalla RAM si devono soddisfare le seguenti condizioni:

- /CE LO, cioè il chip deve essere abilitato
- /OE LO, cioè le uscite devono essere attivate
- /WE HI, dunque non devo attivare il segnale di scrittura

La sequenza degli eventi è dunque la seguente:

- RI è LO, dunque la NOT disabiliterà il transceiver superiore, che non metterà dunque alcunché in output verso la RAM;
- RO LO abilita il XCVR inferiore e /OE della RAM, che viene attivata in output;
- /CE (aka /CS, Chip Select) è fisso LO in quanto connesso a ground;
- nel frattempo WE è HI (e dunque la scrittura è inibita) perché:
  - in Run Mode RI LO mantiene HI all'uscita della NAND connessa a I1d del MUX (se uno dei due input della RAM è LO, l'uscita è HI) e dunque anche /WE è fisso HI, inibendo la scrittura;
  - in Program Mode l'ingresso I0d del MUX è fisso HI grazie alla resistenza da 1K, perciò anche in questo caso non vi è scrittura.

Una analisi successiva di questo schema, che "all'occhio" era molto bello, mi evidenziava che probabilmente avevo gestito correttamente il discorso del "bus interno" e che anche la fase di output poteva essere funzionale, mentre nella fase di input notavo ridondanze superflue: avrei potuto far lavorare il transceiver inferiore bidirezionalmente a seconda della necessità per far interagire RAM e BUS e avrei potuto anche eliminare i MUX e collegare il transceiver superiore direttamente al dip-switch, attivandolo solo al momento opportuno per la programmazione manuale della RAM. In pratica, sarei ritornato alla soluzione concepita da The8BitEnthusiast, senza tuttavia aver ancora acquisito capacità e autonomia sufficienti per progettare una soluzione di attivazione just-in-time dei transceiver come aveva fatto lui.

### Seconda versione del modulo RAM

Stabilito che una soluzione just-in-time sarebbe stata prematura per le mie competenze, avevo dunque provato a ridisegnare lo schema con un solo transceiver mantenendo i MUX per gestire l'input della RAM, facendolo provenire dal bus o dal dip-switch a seconda della modalità Program o Run-Mode.

[![Seconda versione del modulo RAM](../../assets/ram/20-ram-2nd.png "Seconda versione del modulo RAM"){:width="100%"}](../../assets/ram/20-ram-2nd.png)

*Seconda versione del Modulo di memoria (RAM) del BEAM.*

A livello temporale questo è anche il momento ufficiale della nascita del nome BEAM 😁.

Dopo aver realizzato il disegno, mi sembrava di aver aggiunto più chip (per la logica) rispetto a prima = maggior complessità. Tuttavia, l'idea continuava a piacermi ed era un interessante esercizio logico per provare a sfruttare l'unico transceiver invertendone la direzione a seconda dell'operazione da fare.

Avevo provato a chiedere un <a href="https://www.reddit.com/r/beneater/comments/10inkvs/8bit_computer_ram_module_would_this_work/" target="_blank">consiglio</a> su Reddit e il solito The8BitEnthusiast, instancabile, mi aveva dato alcune indicazioni e una risposta tutto sommato positiva.

Per analizzare tutti gli stati logici possibili avevo preparato una tabella di riepilogo con la quale verificare se il comportamento del modulo fosse in linea con le aspettative; la tabella mostrata in seguito è solo una parte di quella completa.

![Tabella analisi modulo RAM](../../assets/ram/20-ram-2nd-table.png "Tabella analisi modulo RAM"){:width="100%"}

*Tabella riepilogativa analisi stati logici seconda versione modulo RAM.*

A un certo punto ho capito che nello schema c'era un problema piuttosto importante: i due MUX 157 proiettavano in continuazione i loro input verso il bus interno, causando un possibile short con la RAM quando questa si trovava in output mode. Ho dunque pensato di sostituire i 2x '157 con i '257, che sono tri-state.

![Tabella analisi rivista modulo RAM](../../assets/ram/20-ram-2nd-table2.png){:width="100%"}

*Tabella riepilogativa rivista analisi stati logici seconda versione modulo RAM.*

Quando sono arrivato a questo punto ho realizzato di aver lavorato inutilmente: a cosa servono due MUX e un '245? Sono sufficienti due '245, uno per gestire l'I/O sul bus e uno per gestire il dip-switch, che in effetti è quanto aveva fatto The8BitEnthusiast! Ero stato illuminato e in quel momento mi è sembrato tutto chiaro!

Nel frattempo avevo anche iniziato a rinominare i segnali logici prendendo come punto di vista il computer e non il modulo:

| Vecchio | Descrizione | Nuovo | Descrizione |
|-        |-            |-      |-            |
| RI      | RAM In      | WR    | Write RAM   |
| RO      | RAM Out     | RR    | Read RAM    |

e così via.

### Terza versione del modulo RAM

Avevo dunque infine riscritto la "truth table" del modulo RAM:

![Tabella analisi modulo RAM - terza versione](../../assets/ram/20-ram-3rd-table.png "Tabella analisi modulo RAM - terza versione"){:width="100%"}

*Tabella analisi modulo RAM - terza versione.*

I due asterischi in tabella \*\* mi servivano a ricordare che dovevo stare particolarmente attento alla preparazione del microcode e che non dovevo mai avere /WR e /RR attivi contemporaneamente, perché non avevo previsto un controllo hardware per prevenire un eventuale conflitto (per evitare conflitti avrei dovuto mettere su /WR una NAND a 3 ingressi che si attiva solo per CLK, RR e /WR, cioè /WE = CLK \* RR \* /WR, in altre parole: posso scrivere sulla RAM *solo* se non la sto leggendo).

## Schema

[![Terza versione del Modulo di memoria (RAM) del computer BEAM](../../assets/ram/20-ram-3rd.png "Terza versione del Modulo di memoria (RAM) del computer BEAM"){:width="100%"}](../../assets/ram/20-ram-3rd.png)

*Terza versione del Modulo di memoria (RAM) del computer BEAM.*

Riprendendo il datasheet del 62256, a pagina 6 troviamo entrambe le modalità di scrittura: quella indicata come "WRITE CYCLE 2 (CE# Controlled)" era stata utilizzata per le prime due revisioni del modulo RAM; l'altra, "WRITE CYCLE 1 (WE# Controlled)", è quella utilizzata per il disegno definitivo del modulo, perché mantenere /OE e /CE LO fissi sia per le letture sia per le scritture rappresentava una semplificazione importante.

[![Write Cycles](../../assets/ram/20-ram-write-cycles.png "Write Cycles"){:width="100%"}](../../assets/ram/20-ram-write-cycles.png)

La gestione dei segnali passa in maniera importante attraverso il MUX '157, così come avviene anche nello schema di The8BitEnthusiast, ma con qualche differenza.

Ecco un riassunto degli stati possibili:

- **/PROG**: In Program Mode il MUX attiva gli ingressi I0a, I0b, I0c e I0d. In Run Mode sono attivi I1a, I1b, I1c e I1d.
  - Za è normalmente HI attraverso la resistenza che lo collega a Vcc, ma passa a LO premendo il tasto di programmazione e attiva /WE, scrivendo sulla RAM.
  - Zb è normalmente HI attraverso la resistenza che lo collega a Vcc, ma passa a LO premendo il tasto di programmazione e attiva il transceiver che interconnette RAM e dip-switch.
  - Zc è fisso HI e disattiva il transceiver che interconnette RAM e bus.
- **RR WR**: Read RAM e Write RAM non sono attivi.
  - Za è fisso HI (perché la NAND su I1a ha un ingresso fisso LO) e la scrittura su RAM è inibita.
  - Zb è fisso HI e inibisce il transceiver tra RAM e dip-switch.
  - Zc è fisso HI e disattiva il transceiver che interconnette RAM e bus.
- **RR /WR**: Read RAM non è attivo, Write RAM è attivo.
  - Za è normalmente HI (perché la NAND su I1a ha un'ingresso LO), ma in corrispondenza dell'impulso di clock passa a LO e attiva /WE, scrivendo sulla RAM.
  - Zb è fisso HI e inibisce il transceiver tra RAM e dip-switch.
  - Zc è normalmente HI, dunque disattiva il transceiver che interconnette RAM e bus, ma in corrispondenza dell'impulso di clock passa a LO (perché Za passa a LO), attivando il transceiver.
- **/RR WR**: Read RAM è attivo, Write RAM non è attivo.
  - Za è fisso HI (perché la NAND su I1a ha un ingresso fisso LO) e la scrittura su RAM è inibita.
  - Zb è fisso HI e inibisce il transceiver tra RAM e dip-switch.
  - Zc è fisso LO e attiva il transceiver tra RAM e bus.
- **/RR /WR**: Stato illegale.

## Note

Invece dei più comuni dip-switch, ho utilizzato dei comodissimi Rocker Switch ("a bilanciere") come quelli in figura; si trovano facilmente presso i distributori di <a href="https://us.rs-online.com/product/te-connectivity/5435640-5/70156004/" target="_blank">materiale elettronico</a>. Notare che i pin originali sono piuttosto corti e non fissano correttamente lo switch alla breadboard, pertanto ho aggiunto uno zoccolo per circuiti integrati.

[![Rocker Switch](../../assets/ram/20-ram-rocker.png "Rocker Switch"){:width="33%"}](../../assets/ram/20-ram-rocker.png)

*Rocker Switch.*

## Link utili

- La documentazione "Building the SAP-3 rev 3.3.pdf" scritta da rolf-electronics e disponibile su <a href="https://github.com/rolf-electronics/The-8-bit-SAP-3" target="_blank">GitHub</a>.
