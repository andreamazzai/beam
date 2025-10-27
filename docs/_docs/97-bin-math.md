---
title: "Aritmetica binaria"
permalink: /docs/math/
excerpt: "Aritmetica binaria"
---
<small>[Concetti importanti](#concetti-importanti) - [Fonti](#fonti) - [Numeri Unsigned e numeri Signed](#numeri-unsigned-e-numeri-signed) - [Approfondimento Overflow](#approfondimento-overflow) - [L'Overflow e l'hardware](#loverflow-e-lhardware) - [Metodi di identificazione dell'Overflow](#metodi-di-identificazione-delloverflow) - [Semplificazione con 74LS151](#semplificazione-con-74ls151) - [Verifica addizioni e sottrazioni con 74LS151](#verifica-addizioni-e-sottrazioni-con-74ls151) - [Gli Adder](#gli-adder) - [Le somme con gli Adder](#le-somme-con-gli-adder) - [Multiple Bit Adder](#multiple-bit-adder) - [Ripple Carry Adder](#ripple-carry-adder) - [Carry Look Ahead Adder](#carry-look-ahead-adder) - [Implementazione nel 74LS181](#implementazione-nel-74ls181) - [Le sottrazioni con gli Adder](#le-sottrazioni-con-gli-adder) - [Link utili](#link-utili)</small>

## Concetti importanti

Studiando il funzionamento dei moduli Flag e ALU, avevo scoperto diverse mie lacune nell'aritmetica binaria:

1. Numeri binari senza segno (Unsigned) e con segno (Signed)
2. Modalità di rappresentazione dei numeri binari con segno:
   - Modulo e Segno (Signed Magnitude)
   - Complemento a 1 (One’s Complement, o anche 1C)
   - Complemento a 2 (Two’s Complement, o anche 2C)
3. Somma e sottrazione di numeri binari
4. Overflow senza segno e Overflow con segno

## Fonti

Lungi dal voler (e poter) spiegare tutto in questo pagina, cerco di riassumere quanto avevo appreso per rimediare alle mie lacune e suggerisco una serie di fonti di approfondimento.

Per una comprensione completa dei temi trattati, raccomando di accedere più volte alle informazioni, anche in tempi diversi e in sequenze diverse: con letture ed approfondimenti ripetuti avevo raggiunto lo scopo prefissato, mentre più visualizzazioni dei video in modalità *brainstorming* mi avevano permesso di riempire i buchi fino a colmarli.

Ecco una serie di video su YouTube:

1. **Binary Overflow** - Mr Powell's Computer Science Channel - <a href="https://www.youtube.com/watch?v=Q7t9-Sq_4Ww&lc=Ugy04nCzuiaOJIqK_5J4AaABAg" target="_blank">link</a>: concetto generico di Overflow.

    Ipotizziamo di avere un computer a 8 bit, in grado di sommare due numeri a 8 bit: sia gli addendi sia la somma sono numeri a 8 bit che possono dunque andare da 0 a 255.
      - Sommando ad esempio 73 + 114 si ottiene 187, che è un numero rappresentabile con 8 bit;
      - sommando invece ad esempio 175 + 92 si ottiene 267, che non è rappresentabile con un numero a 8 bit: abbiamo un errore di Overflow (o **Overflow senza segno**), perché i bit a nostra disposizione (8) non ci consentono di rappresentare il risultato della somma: il risultato dell'operazione aritmetica eccede la capacità di rappresentazione dei bit disponibili senza considerare il segno.

    Abbiamo in pratica un riporto che dovremmo portare al 9° bit del nostro computer, che però ne ha solo 8. In questo caso il segnale di Carry in uscita dall'ALU ci segnala che il numero risultante dalla somma dei due addendi è più grande del numero calcolabile dall'ALU.

2. **Somme di numeri Unsigned, il concetto di Carry ed eventuale Overflow** - Mr Dimmick's Computing Channel - <a href="https://www.youtube.com/watch?v=nKxjLM6ePcI" target="_blank">link</a>: molto ben fatto; spiega anche le regole di base applicabili a tutte le somme di numeri binari.

3. **Numeri negativi in binario** dello stesso autore del video 1 - <a href="https://www.youtube.com/watch?v=dHB7jFjESLY" target="_blank">link</a>: un ottimo video che spiega come rappresentare i numeri negativi in complemento a 2 e come convertire un numero positivo in negativo e viceversa.

    - Con il complemento a 2, vi è un solo 0, a differenza di quanto accade con Modulo e Segno (si veda più avanti in questa pagina).
    - Per convertire un numero positivo in negativo è sufficiente invertire tutti i bit ed aggiungere 1.
    - Lo stesso procedimento è perfettamente valido anche al contrario (da negativo a positivo).
    - Prestare attenzione a un errore riconosciuto anche dall'autore nei commenti: nella discussione di Segno e Modulo il range non va da -64 a +64, ma da -128 a +128.

4. **Twos complement: Negative numbers in binary** di Ben Eater - <a href="https://www.youtube.com/watch?v=dHB7jFjESLY" target="_blank">link</a>: ancora su numeri negativi, complementi e sottrazioni. Ottimo per capire come il complemento a 2 indirizzi le lacune del complemento a 1.

5. **Binary Addition and Subtraction With Negative Numbers, 2's Complements & Signed Magnitude** - The Organic Chemistry Tutor - <a href="https://www.youtube.com/watch?v=sJXTo3EZoxM" target="_blank">link</a>: ottimo video con visualizzazione grafica molto efficace.

    - Particolarmente interessante un <a href="https://www.youtube.com/watch?v=sJXTo3EZoxM&lc=Ugyl7WIoyqVR8wM0dAJ4AaABAg" target="_blank">commento</a> che segnala che per invertire il segno di un numero binario rappresentato in 2C è sufficiente seguire due passaggi:
      1) partire da destra e non modificare nulla fino al primo bit a 1 incluso;
      2) invertire tutti i bit rimanenti.

      Esempio: per invertire il numero 01010100 (84 decimale) si lasciano invariati i primi 3 bit partendo da destra (01010**100**) e si invertono poi tutti gli altri da **01010**100 a **10101**100, ottenendo come risultato finale 10101100 (-84).

6. **Two's Complement Arithmetic** di Brock LaMeres - <a href="https://www.youtube.com/watch?v=cbVTaQFPwik&list=PL643xA3Ie_Et2uM4xu1yFk-A5ZQQ8gQ5e&index=10&pp=iAQB" target="_blank">link</a>: a dispetto della qualità audio non perfetta (ma la pronuncia rimane ottima e facilmente comprensibile), l'esposizione dell'argomento è eccellente.

E qualche pagina da visitare:

1. <a href="http://www.6502.org/tutorials/vflag.html" target="_blank">The Overflow (V) Flag Explained</a> di Bruce Clark su [6502.org](http://6502.org) - linguaggio essenziale e tante informazioni.
2. <a href="https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html" target="_blank">The 6502 overflow flag explained mathematically</a> di Ken Shirriff - una pagina incredibilmente ben fatta che tratta Complementi, numeri Signed, Overflow.
3. <a href="https://www.righto.com/2013/01/a-small-part-of-6502-chip-explained.html" target="_blank">The 6502 CPU's overflow flag explained at the silicon level</a> di Ken Shirriff - interessantissima analisi dell'Overflow al livello del silicio del 6502.
4. <a href="https://madformath.com/calculators/digital-systems/signed-arithmetic/signed-binary-addition-calculator/signed-binary-addition-calculator" target="_blank">Signed Binary Addition Calculator</a> - calcolatore per somme di numeri Signed con evidenza dei vari passaggi di calcolo.

Evidenzio in particolare <a href="https://www.righto.com/2012/12/the-6502-overflow-flag-explained.html" target="_blank">The 6502 overflow flag explained mathematically</a> per una comprensione completa dell'Overflow, che sarà ripreso e approfondito in seguito in questa pagina.

## Numeri Unsigned e numeri Signed

E' necessario comprendere a fondo logica e differenze relative ai numeri Unsigned e Signed.

Approfondendo i numeri Signed, ero giunto a questa <a href="https://sandbox.mc.edu/~bennet/cs110" target="_blank">interessantissima pagina</a> dell'Università del Mississippi dedicata alla matematica binaria.

Grazie alla sezione *Textbook ==> Module 3: Computer Integer Arithmetic ==> 2. Negative binary numbers* avevo perfettamente compreso la rappresentazione binaria dei numeri negativi.

Il metodo **Signed Magnitude** è molto facile da comprendere: si sacrifica un bit dedicandolo alla rappresentazione del segno; il bit sacrificabile è quello più significativo (MSB), pertanto se un numero Unsigned a 8 bit può andare da 0 a 255 (2^8 = 256 combinazioni rappresentabili), un numero Unsigned avrà 7 bit disponibili per il Modulo (2^7 = 128 combinazioni) e un bit per il segno, dunque potrà andare da -128 a +127 (sempre 256 numeri rappresentabili, ma metà negativi e metà positivi).

![Rappresentazione Modulo e Segno dei numeri a 4 bit](../../assets/math/math_signed_magnitude.gif){:width="100%"}

*Rappresentazione Modulo e Segno dei numeri a 4 bit.*

Nell'immagine (semplificata a soli 4 bit per ragioni di spazio) si nota un problema non secondario: lo zero appare due volte (0000 e 1000): noi sappiamo che lo zero non ha segno, pertanto questa rappresentazione non è la migliore possibile.

NB: nella pagina citata, vi è un'immagina errata nella sottrazione "5 - 2". L'immagine è la seguente:

![Errore sottrazione](../../assets/math/math_mistake.gif
){:width="15%"}

La pagina desiderava in realtà rappresentare era una sottrazione tra A e B utilizzando il metodo Modulo e Segno, che *dovrebbe* permettere di trasformare l'operazione da "5 - 2" in "5 + (-2)" invertendo il primo bit del numero 2, che dunque da 0010 diventa 1010:

~~~text
   Dec        Bin 
A     5  ==>  0101+
B    -2  ==>  1010
   ----       ----
Q     3       1111 ==> -7
~~~

Tuttavia, la somma binaria di 0101 (5) e 1010 (-2) non porta al risultato aspettato (3), bensì a un chiaro errore: è chiaro che la rappresentazione Modulo e Segno non permette di trasformare un'operazione "A - B" in "A + (-B)".

Tralasciando la spiegazione del metodo **Complemento a 1 (1C)** (inversione di tutti i bit del numero), anch'esso non ottimale, il **Complemento a 2 (2C)** risulta invece perfetto per la rappresentazione dei numeri negativi, portando tra l'altro in dote una grandissima semplificazione nell'esecuzione delle sottrazioni.

![Rappresentazione in Complemento a 2 dei numeri a 4 bit](../../assets/math/math_2c.gif){:width="100%"}

*Rappresentazione in Complemento a 2 dei numeri a 4 bit.*

Due sono gli aspetti da evidenziare:

- Risoluzione del problema del doppio zero.
  - La regola che sta alla base della teoria del complemento a 2 è: come posso rappresentare il numero "-1" in modo che, aggiungendovi "1", si ottenga "0"?
  - Similarmente ai vecchi contachilometri meccanici delle automobili, che una volta giunti a 99.999 passavano a 00.000, il 99.999 del contachilomentri corrisponde allo 1111.1111 dell'aritmetica binaria a 8 bit: sommato a 0000.0001, genera come risultato 0000.0000.
  
  Nota: in questa pagina, il punto separatore fra i primi 4 bit e i secondi 4 bit dei numeri binari a 8 bit è inserito solamente per agevolarne la lettura e non ne modifica in alcun modo i valori.

- Le sottrazioni si possono eseguire senza errori sommando minuendo e complemento a 2 del sottraendo.
  - Ad esempio, invece di eseguire "15 - 7", si effettua "15 + (-7)": l'addizione è l'operazione più semplice in assoluto da eseguire e implementare; utilizzando il complemento a 2 diventa possibile applicare alle sottrazioni le stesse regole già applicate alle addizioni, semplificandone enormemente i calcoli.
  - Per approfondire la sottrazione in 2C, si vedano i [collegamenti](#fonti) evidenziati in precedenza, in particolar modo il <a href="https://www.youtube.com/watch?v=sJXTo3EZoxM" target="_blank">video numero 4</a>.

Riprendendo anche quanto esposto nella pagina dei [Flag](../flags/#overflow), in un byte sono possibili 256 combinazioni:

- trattando i numeri come Unsigned, è possibile contare da 0 a 255;
- trattando invece i numeri come Signed in complemento a 2:
  - i valori da 0 a 127 sono rappresentati allo stesso modo dei numeri Unsigned da 0 a 127 (da Hex 0x00 a 0x7F);
  - i valori da -128 a -1 fanno il paio con le rappresentazioni esadecimali e binarie dei numeri Unsigned da 128 a 255 (da Hex 0x80 a 0xFF).

Questa tabella dovrebbe chiarire il concetto:

~~~text
----------------------------------------
| Hex  | Binary    | Signed | Unsigned | 
----------------------------------------
| 0x80 | 1000.0000 |   -128 |      128 | 
| 0x81 | 1000.0001 |   -127 |      129 | 
| 0x82 | 1000.0010 |   -126 |      130 | 
| 0x83 | 1000.0011 |   -125 |      131 | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| 0x90 | 1001.0000 |   -112 |      144 | 
| 0xA0 | 1010.0000 |   - 96 |      160 | 
| 0xB0 | 1011.0000 |   - 80 |      176 | 
| 0xC0 | 1100.0000 |   - 64 |      192 | 
| 0xD0 | 1101.0000 |   - 48 |      208 | 
| 0xE0 | 1110.0000 |   - 32 |      224 | 
| 0xF0 | 1111.0000 |   - 16 |      240 | 
| 0xF1 | 1111.0001 |   - 15 |      241 | 
| 0xF2 | 1111.0010 |   - 14 |      242 | 
| 0xF3 | 1111.0011 |   - 13 |      243 | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| 0xFD | 1111.1101 |   -  3 |      253 | 
| 0xFE | 1111.1110 |   -  2 |      254 | 
| 0xFF | 1111.1111 |   -  1 |      255 | 
----------------------------------------
| Hex  | Binary    | Signed | Unsigned | 
----------------------------------------
| 0x00 | 0000.0000 |      0 |        0 | 
| 0x01 | 0000.0001 |      1 |        1 | 
| 0x02 | 0000.0010 |      2 |        2 | 
| 0x03 | 0000.0011 |      3 |        3 | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| 0x10 | 0001.0000 |     16 |       16 | 
| 0x20 | 0010.0000 |     32 |       32 | 
| 0x30 | 0011.0000 |     48 |       48 | 
| 0x40 | 0100.0000 |     64 |       64 | 
| 0x50 | 0101.0000 |     80 |       80 | 
| 0x60 | 0110.0000 |     96 |       96 | 
| 0x70 | 0111.0000 |    112 |      112 | 
| 0x71 | 0111.0001 |    113 |      113 | 
| 0x72 | 0111.0010 |    114 |      114 | 
| 0x73 | 0111.0011 |    115 |      115 | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| .... | ......... |    ... |      ... | 
| 0x7C | 0111.1100 |    124 |      124 | 
| 0x7D | 0111.1101 |    125 |      125 | 
| 0x7E | 0111.1110 |    126 |      126 | 
| 0x7F | 0111.1111 |    127 |      127 | 
~~~

*Relazione tra numeri Hex, Bin, Signed e Unsigned a 8 bit.*

Come già visto nell'immagine *Rappresentazione in Complemento a 2 dei numeri a 4 bit*, è importante notare anche qui il passaggio dei numeri Signed da -1 a 0 in corrispondenza del passaggio binario da 1111.1111 a 0000.0000.

Il complemento a 2 è, dunque, un modo molto pratico per rappresentare i numeri Signed, nei quali un MSB = 0 indica un numero positivo e un MSB = 1 indica un numero negativo.

L'MSB a 1 assume il valore negativo 2^n, dove n è la posizione dell'MSB stesso (il conteggio della posizione si esegue partendo da zero e spostandosi da destra verso sinistra). Nel caso di un numero a 8 bit, l'MSB vale -(2^7) = -128.

La somma tra l'MSB e i valori assunti dagli altri 7 bit corrisponde al valore del numero espresso nella notazione Signed. Facciamo due esempi sommando i valori dei bit a 1:

~~~text
- 0010.0011 =      0 + 0 + 2^5 + 0 + 0 + 0 + 2^1 + 2^0 =        32 + 2 + 1 =  35
- 1010.0011 = -(2^7) + 0 + 2^5 + 0 + 0 + 0 + 2^1 + 2^0 = -128 + 32 + 2 + 1 = -93
~~~

## Approfondimento Overflow

L'approfondimento dell'Overflow è stato un passaggio obbligato; prima di capire il funzionamento della parte di circuito dedicata al [flag V](../flags/#overflow), dovevo comprenderne bene la definizione e quali metodi si usassero per determinarne una eventuale presenza.

Tom Nisbet segnalava che l'ispirazione per l'uso di un Data Selector/Multiplexer <a href="https://www.ti.com/lit/ds/symlink/sn54s151.pdf" target="_blank">74LS151</a> per individuare situazioni di Overflow nel suo NQSAP derivava da <a href="https://www.reddit.com/r/beneater/comments/kmuuex/question_for_all_74ls181_alu_people/" target="_blank">un thread su Reddit</a>, che rimandava poi a pagine scritte da Dieter Mueller (aka ttlworks) su 6502.org.

Nel <a href="http://6502.org/users/dieter/v_flag/v_0.htm" target="_blank">link di riferimento</a>, Dieter inizialmente spiegava in maniera visiva in quali situazioni di somma o sottrazione tra due numeri Signed a 8 bit si generasse un Overflow, utilizzando quella che si potrebbe forse definire una sorta di tavola pitagorica.

Ricordiamo che l'Overflow indica un errore nel processo di somma o sottrazione di due numeri Signed: se il numero risultante ha un segno errato rispetto al previsto, si ha un Overflow.

Nelle due tabelle sottostanti, che rappresentano rispettivamente operazioni di addizione e di sottrazione, le combinazioni Colonna/Riga al cui incrocio vi è un asterisco "\*" indicano situazioni di Overflow, cioè di un risultato che non può essere correttamente rappresentato con gli 8 bit disponibili.

~~~text
Operazione       Sum  (Column + Row)       Sub (Column - Row)
                   +0123456789ABCDEF         +0123456789ABCDEF
                 +  0000000000000000       -  0000000000000000
Overflow:        00 ................       00 ................
Overflow:        10 .......*........       10 ........*.......
Overflow:        20 ......**........       20 ........**......
Overflow:        30 .....***........       30 ........***.....
Overflow:        40 ....****........       40 ........****....
Overflow:        50 ...*****........       50 ........*****...
Overflow:        60 ..******........       60 ........******..
Overflow:        70 .*******........       70 ........*******.
Overflow:        80 ........********       80 ********........
Overflow:        90 ........*******.       90 .*******........
Overflow:        A0 ........******..       A0 ..******........
Overflow:        B0 ........*****...       B0 ...*****........
Overflow:        C0 ........****....       C0 ....****........
Overflow:        D0 ........***.....       D0 .....***........
Overflow:        E0 ........**......       E0 ......**........
Overflow:        F0 ........*.......       F0 .......*........

Esempio:        0x70 + 0x40 = Overflow    0xA0 - 0x30 = Overflow
~~~

Note:

- Le tabelle sono semplificate: sono mostrati solo i numeri multipli di 16 (0x**X**0), dunque 0x00, 0x10, 0x20 e così via.
- Sono rappresentati numeri Signed a 8 bit, dunque, ad esempio, 0x20 corrisponde a 32 decimale, mentre 0xA0 corrisponde a -96 decimale.
- Come esposto nella tabella *Relazione tra numeri Hex, Bin, Signed e Unsigned a 8 bit*, il range dei numeri Signed a 8 bit si estende da -128 (0x80) a 127 (0x7F) passando per lo zero (0x00).

Riprendiamo i due esempi di Overflow riportati in calce alle tabelle:

- **0x70 + 0x40 =** in decimale 112 + 64 = 176, che però non rientra nel range -128 / + 127 dei numeri Signed a 8 bit. In effetti, la somma tra 112 e 64 genera un risultato il cui MSB è 1, che secondo la notazione Signed è un numero negativo. Poiché abbiamo sommato numeri Signed, anche il risultato deve essere letto come Signed: 0xB0 = -80, che è palesemente errato. Ne ricaviamo che se la somma di due Signed positivi genera un risultato Signed negativo, siamo in una situazione di Overflow, cioè di numero non rappresentabile con i bit a disposizione.

~~~text
  Hex       Dec       Bin
                      1
  0x70 ==>  112  ==>  0111.0000 +
 +0x40 ==>   64  ==>  0100.0000 = 
           ----     ----------- 
            176       1011.0000 ==> 0xB0
~~~

- **0xA0 - 0x30 =** -96 - 48 = -144, che però non rientra nel range -128 / + 127 dei numeri Signed a 8 bit. In effetti, la sottrazione tra -96 e 48\* genera un risultato il cui MSB è 0, che secondo la notazione Signed è un numero positivo. Poiché abbiamo sottratto numeri Signed, anche il risultato deve essere letto come Signed: 0x70 = 112, che è palesemente errato. Ne ricaviamo che se la sottrazione tra un Signed negativo e un Signed positivo genera un risultato Signed positivo, siamo in una situazione di Overflow, cioè di numero non rappresentabile con i bit a disposizione.

~~~text
  Hex       Dec       Bin
                     1
  0xA0 ==>  -96  ==>  1010.0000 +
 -0x30 ==>  -48  ==>  1101.0000 = 
           ----     ----------- 
           -144      10111.0000** ==> 0111.0000 ==> 0x70
~~~

\* Notare che in questo secondo caso l'ALU esegue internamente una operazione A + (-B), dunque la rappresentazione di B è  invertita secondo la regola del complemento a 2 (2C).

\*\* Il 9° bit viene troncato, perché la dimensione della word usata nell'operazione è di 8 bit.

Oltre ai due esempi citati poco sopra, è opportuno anticipare che sono dimostrabili anche le ipotesi contrarie:

- se la somma di due Signed negativi genera un risultato Signed positivo, siamo in una situazione di Overflow;
- se la sottrazione tra un Signed positivo e un Signed negativo genera un risultato Signed negativo, siamo in una situazione di Overflow.

In definitiva, possiamo dire che se il bit del segno viene corrotto si è verificato un Overflow e il flag V viene conseguentemente settato.

### L'Overflow e l'hardware

Dieter esponeva una rappresentazione logica di un Adder in grado di effettuare sia somme sia sottrazioni, segnalando che "le somme A+B sono facili; per eseguire invece le sottrazioni, il metodo più semplice è quello di invertire B e procedere poi esattamente come per le somme: A+(-B)".

![Adder hardware per somme A+B e sottrazioni A-B](../../assets/math/dieter-alu.png){:width="66%"}

*Adder hardware per somme A+B e sottrazioni A-B.*

In un Adder a 8 bit abbiamo 8 Adder a 1 bit in cascata; quello rappresentato di seguito è l'8° ed ultimo, i cui ingressi A e B' sono i bit più significativi A7 e B7' dei numeri da sommare; l'ingresso Carry-in C_IN è connesso all'output Carry-Out C_OUT proveniente dall'Adder precedente.

![Ultimo stadio di un Adder a 8 bit](../../assets/math/dieter-8th-adder.png){:width="25%"}

*Ultimo stadio di un Adder a 8 bit.*

La tabella seguente sintetizza quanto accade nell'8° Adder:

| C7    | B7'   | A7    | Q7    | C8    |  V       |
| -     | -     | -     | -     | -     | -        |
| 0     | 0     | 0     | 0     | 0     | 0        |
| 0     | 0     | 1     | 1     | 0     | 0        |
| 0     | 1     | 0     | 1     | 0     | 0        |
| **0** | **1** | **1** | **0** | **1** | **1**\*  |
| **1** | **0** | **0** | **1** | **0** | **1**\*\*|
| 1     | 0     | 1     | 0     | 1     | 0        |
| 1     | 1     | 0     | 0     | 1     | 0        |
| 1     | 1     | 1     | 1     | 1     | 0        |

- C7 (C_IN nel disegno) proviene dal C_OUT dell'Adder precedente;
- B7' è l'MSB del numero da sommare B (normale o invertito dalla circuiteria interna dell'ALU, a seconda che si esegua una somma o una sottrazione);
- A7 è l'MSB del numero da sommare A;
- Q7 è l'MSB del risultato di A7 + B7' + C7;
- C8 è il Carry in uscita (C_OUT);
- C8 risulta a 1 se almeno due tra A7, B7' e C7 sono a 1;
- V è il Flag di Overflow.

Nella sezione [Approfondimento Overflow](#approfondimento-overflow) abbiamo visto che la somma di due Signed positivi non può avere come risultato un Signed negativo e che anche la situazione opposta è vera. Nelle due righe evidenziate troviamo infatti:

- \* due Signed negativi (A7 = B7' = 1) la cui somma genera un risultato positivo (Q7 = 0) --> corrisponde a una situazione di Overflow.
- \*\* due Signed positivi (A7 = B7' = 0) la cui somma genera un risultato negativo (Q7 = 1) --> corrisponde a una situazione di Overflow.

### Metodi di identificazione dell'Overflow

Mettendo a fattor comune quanto abbiamo visto fino ad ora, possiamo identificare 3 situazioni misurabili che determinano un Overflow in una somma:

1. A7 e B7' sono dello stesso segno e Q è invertito, cioè **(A7 == B7') AND (A7 <> Q7)** (notare che non stiamo specificando un valore assoluto 1 o 0 dei bit: stiamo considerando le relazioni tra i segnali);
2. C7 e C8 sono invertiti tra loro, cioè **C7 <> C8** (anche in questo caso si esegue una comparazione relativa);
3. **(A7 = B7' = 1 AND Q7 = 0) OR (A7 = B7' = 0 AND Q7 = 1)** è simile al punto 1, ma, anziché porre i bit in una comparazione relativa, ne stiamo specificando il valore assoluto.

La truth table **(A7 == B7') AND (A7 <> Q7)** del primo caso si tradurrebbe nella logica in figura; purtroppo, il computer basato su [74LS181](../alu/#il-74ls181) non offre visibilità del valore di B7', che è computato internamente all'ALU e non esposto, pertanto questo metodo non è utilizzabile:

![Primo metodo](../../assets/math/overflow-detector-xor-not-and.png)

Per dovere di cronaca 😊 riporto lo schema di due registri A e B e di un generico Half Adder che avevo disegnato per comprendere meglio le addizioni; si noti la circuiteria preposta ad individuare situazioni di Overflow.

[![Adder su carta](../../assets/math/adder.png "Adder su carta"){:width="50%"}](../../assets/math/adder.png){:width="75%"}

---
Anche nel secondo caso **C7 <> C8** manca una informazione, perché C7 è computato internamente all'ALU e non esposto:

![Secondo metodo](../../assets/math/overflow-detector-xor.png)

---
Nemmeno il terzo metodo **(A7 = B7' = 1 AND Q7 = 0) OR (A7 = B7' = 0 AND Q7 = 1)** sembra utilizzabile, perché B7' è confinato alla circuiteria interna dell'ALU:

![Terzo metodo](../../assets/math/overflow-detector-and-or.png){:width="45%"}

I tre metodi esaminati sembrano portare a una strada chiusa; tuttavia, è possibile ricostruire artificialmente il segnale B7' basandosi sugli altri segnali disponibili nel computer.

- In una *addizione*, il valore B7' in ingresso all'ultimo Adder del '181 è uguale al valore di B7 dato in input al chip (in una somma A7 + B7, B7' non subisce modifiche dalla circuiteria interna dell'ALU e possiamo dunque usare B7 come input del circuito che determina l'eventuale stato di Overflow).

![Overflow somma](../../assets/math/overflow-detector-a+b.png){:width="43%"}

Qualche considerazione permette di riutilizzare lo stesso metodo anche per la verifica dell'Overflow nelle *sottrazioni*. Infatti, nelle sezioni [Fonti](#fonti) e [Numeri Unsigned e numeri Signed](#numeri-unsigned-e-numeri-signed) di questa pagina abbiamo appreso come una sottrazione possa essere svolta eseguendo una addizione tra minuendo e complemento a 2 del sottraendo, dunque:

- Il valore B7' in ingresso all'ultimo Adder del '181 è invertito rispetto al valore di B7 dato in input al chip (in una sottrazione A7 - B7, B7' viene invertito dalla circuiteria interna dell'ALU e possiamo dunque usare l'inverso di B7 come input del circuito che determina l'eventuale stato di Overflow):

![Overflow sottrazione](../../assets/math/overflow-detector-a-b.png){:width="43%"}

In definitiva, il terzo metodo è utilizzabile per la verifica dell'Overflow sia per le addizioni, sia per le sottrazioni.

A questo punto, possiamo realizzare un circuito in grado di rilevare l'overflow. È necessario integrare la gestione delle istruzioni: il circuito di rilevamento dell'overflow per le addizioni deve includere un controllo che verifichi se l'operazione in esecuzione è un'addizione; lo stesso vale per la sottrazione.

[![Overflow somma e sottrazione](../../assets/math/overflow-detector-a+b-a-b.png){:width="100%"}](../../assets/math/overflow-detector-a+b-a-b.png)

### Semplificazione con 74LS151

Ora le cose si fanno interessanti: Dieter prosegue indicando che un unico chip 74LS151, opportunamente connesso agli ingressi A, B, Q, A + B e A - B, è in grado di identificare tutte le situazioni di Overflow.

![Schema logico del 74LS151](../../assets/math/overflow-74151.png){:width="80%"}

*Schema logico del 74LS151.*

Evidenziamo i due punti da prendere in considerazione per vedere se possiamo farli combaciare con il '151:

- l'equazione dell'Overflow è basata su 3 input (A, B, Q);
- l'eventuale Overflow è anche conseguenza dell'operazione eseguita; l'Instruction Register ci può indicare se si sta eseguendo una addizione o una sottrazione.

Un esempio concreto con input uguali, ma diverse situazioni:

- se A = 1, B = 1 e Q = 0 e stiamo eseguendo una somma, abbiamo un Overflow;
- se A = 1, B = 1 e Q = 0 e stiamo facendo una sottrazione, allora non vi è Overflow.

In effetti, il '151 consente di selezionare una sorgente di dati (tra le 8 disponibili I0-I7) in funzione di una codifica univoca presentata ai suoi ingressi di selezione S2-S1-S0.

Provando a scrivere un flusso logico:

1. la presenza di uno specifico stato logico agli ingressi di selezione **S2-S1-S0** connessi a **Q7**, **B7** e **A7**
2. attiva un determinato ingresso **Ix** tra quelli connessi all'Instruction Register, che
3. grazie alla opportuna scelta dell'opcode delle istruzioni di somma e sottrazione
4. può risultare in uno stato logico 1 sull'uscita **Z** uscita sul '151
5. evidenziando una condizione di Overflow.

Ipotizziamo ad esempio di eseguire una somma con:

- input sull'ALU A7 = B7 = 1 e output Q7 = 0, cioè due Signed negativi in ingresso e un Signed positivo in uscita, che determinano una situazione di Overflow;
- I3 del '151 connesso a una linea dell'Instruction Register attiva in caso di istruzione di somma A + B.

La combinazione 011 agli ingressi S2-S1-S0 del '151 attiverà l'ingresso I3, che porterà all'uscita Z il segnale allo stato logico 1 proveniente dall'IR, evidenziando una situazione di Overflow.

![74LS151](../../assets/math/overflow-74151-i3-sum.png){:width="40%"}

Dovendo indirizzare 8 combinazioni di Q, B e A (000, 001 etc. fino a 111), una configurazione dei pin di ingresso come evidenziato nella successiva figura risolve le equazioni di Overflow sia per le addizioni A + B, sia per le sottrazioni A - B e B - A, purché i pin I0-I7 del '151 siano correttamente connessi all'IR per rilevare l'esecuzione di istruzioni di somma A + B e sottrazione A - B e B - A:

![74LS151](../../assets/math/overflow-74151-i3-sum-full.png){:width="50%"}

La seguente tabella riepiloga le situazioni di Overflow in base agli stati di Q, B ed A ed al fatto che si stiano eseguendo addizioni o sottrazioni:

| Q7-S2 | B7-S1 | A7-S0 | Sum/Sub   | Input  | Overflow     |
| -     | -     | -     |  -        | -      |  -     |
|   0   |   0   |   0   |           | I0     |  -     |
| **0** | **0** | **1** | **A - B** | **I1** |  **1** |
| **0** | **1** | **0** | **B - A** | **I2** |  **1** |
| **0** | **1** | **1** | **A + B** | **I3** |  **1** |
| **1** | **0** | **0** | **A + B** | **I4** |  **1** |
| **1** | **0** | **1** | **B - A** | **I5** |  **1** |
| **1** | **1** | **0** | **A - B** | **I6** |  **1** |
|   1   |   1   |   1   |           | I7     |  -     |

Si noti però che l'operazione B - A non è necessaria per simulare le istruzioni di sottrazione del 6502.

Riepilogando:

- le combinazioni degli ingressi S2-S1-S0 del '151 (connessi agli MSB Q7, B7 e A7 dei registri A e B e dell'uscita Q dell'ALU) selezionano quale tra gli ingressi I0-I7 sia necessario attivare per portare in uscita una eventuale segnalazione di Overflow;

- alcuni degli ingressi I0-I7 sono opportunamente connessi in modalità "Hardwired" all'Instruction Register per determinare l'operazione correntemente in esecuzione.

### Verifica addizioni e sottrazioni con 74LS151

Testiamo alcuni casi di addizione e sottrazione tra A e B. Nella righe C e Q sono indicati rispettivamente il Carry, quando presente, e il risultato dell'operazione.

- Nella colonna **Hex** è esposta la rappresentazione esadecimale dei numeri che vogliamo sommare o sottrarre, con il simbolo dell'operazione alla sinistra del secondo numero; desideriamo eseguire l'operazione 0x70 - 0x30.
- La colonna **Dec** mostra il valore decimale ricavato dalla tabella *Relazione tra numeri Hex, Bin, Signed e Unsigned a 8 bit*; 0x70 corrisponde a 112 decimale, mentre 0x30 corrisponde a 48: l'operazione è quindi 112 - 48 (che avrà come risultato 64).
- La colonna **Bin** espone la rappresentazione binaria dei numeri (in complemento a 2 se negativi): 112 corrisponde a 0111.0000, mentre 48 corrisponde a 0011.0000.
  - NB: se invece di una sottrazione 112 - 48 avessimo voluto eseguire ad esempio un'operazione di addizione 112 + (-48) tra un Signed positivo e un Signed negativo, il -48 sarebbe stato qui rappresentato colonna nella sua forma in complemento a 2, cioè 1101.000.
- La colonna **2C** è infine utilizzata per eseguire l'operazione di somma invertendo l'eventuale sottraendo positivo: il sottraendo 48 viene convertito in 2C 1101.0000 (che in decimale è -48) e sommato al minuendo.

~~~text
    Hex        Dec        Bin             2C
C                                        1111 
A   0x70  ==>   112  ==>  0111.0000  ==>  0111.0000 +
B  -0x30  ==>   -48  ==>  0011.0000  ==>  1101.0000 = 
               ----                      ----------
Q                64                      10100.0000 ==> 0100.0000 ==> 0x40 = 64, no Overflow
~~~

Come riportato in più occasioni, la sottrazione di un numero positivo (nel nostro caso 48) viene eseguita sommando il minuendo con il valore invertito del sottraendo, cioè -48; il grande vantaggio del complemento a 2 è proprio quello di permettere la trasformazione di una sottrazione in addizione, pertanto l'operazione originaria 112 - 48 diventa 112 + (-48), cioè 0111.0000 + 1101.0000 nella colonna 2C. Nel risultato, l'eventuale 9° bit deve essere scartato, in quanto il calcolo è effettuato su una word a 8 bit; l'8° bit (MSB) del risultato rappresenta il segno, che nel nostro esempio è 0 ad indicare un Signed positivo.

In altre parole: quando devo effettuare la sottrazione di un numero positivo, ne calcolo il complemento a 2 e lo sommo al minuendo.

Nel caso specifico di questa sottrazione di esempio, non c'è Overflow, in quanto il valore 64 risultante dalla sottrazione 0x70 - 0x30 è incluso nel range di numeri Signed ad 8 bit.

- **Caso 1:** 0x20 + 0xC0; somma A + B di un Signed positivo e un Signed negativo.

~~~text
    Hex        Dec        Bin             2C
C                                                     
A   0x20  ==>    32  ==>  0010.0000  ==>  0010.0000 +
B  +0xC0  ==>   -64  ==>  1100.0000  ==>  1100.0000 = 
               ----                      ----------
Q               -32                       1110.0000 ==> 0xE0 = -32, no Overflow
~~~

Il microcode opportunamente codificato dell'istruzione A + B porterebbe a 1 gli ingressi I3 e I4 del '151, mentre tutti gli altri ingressi sarebbero a 0; l'operazione produrrebbe Q7=1, B7=1 e A7=0 sugli ingressi di selezione (S2-S1-S0 = 110), che attiverebbero l'ingresso **I6** che risulta a 0 in quanto non attivato dal microcode, pertanto l'uscita Q del '151 sarebbe a 0, indicando che non vi è Overflow --> situazione verificata correttamente:

![74LS151](../../assets/math/overflow-74151-i6.png){:width="50%"}

---

- **Caso 2:** 0x20 + 0x70; somma A + B di due Signed positivi.

~~~text
    Hex        Dec        Bin             2C
C                                         11
A   0x20  ==>    32  ==>  0010.0000  ==>  0010.0000 +
B  +0x70  ==>   112  ==>  0111.0000  ==>  0111.0000 = 
               ----                      ----------
Q               144                       1001.0000 ==> 0x90 = -112, Overflow
~~~

Il microcode opportunamente codificato dell'istruzione A + B porterebbe a 1 gli ingressi I3 e I4 del '151, mentre tutti gli altri ingressi sarebbero a 0; l'operazione produrrebbe Q7=1, B7=0 e A7=0 sugli ingressi di selezione (S2-S1-S0 = 100), che attiverebbero l'ingresso **I4** che risulta a 1 in quanto attivato dal microcode, pertanto l'uscita Q del '151 sarebbe a 1, indicando che vi è Overflow --> situazione verificata correttamente.

![74LS151](../../assets/math/overflow-74151-i4.png){:width="50%"}

---

- **Caso 3:** 0x50 - 0x30; sottrazione A - B tra due Signed positivi.

~~~text
    Hex        Dec        Bin             2C
C                                        11 1
A   0x50  ==>    80  ==>  0101.0000  ==>  0101.0000 +
B  -0x30  ==>   -48  ==>  0011.0000  ==>  1101.0000 = 
               ----                      ----------
Q                32                      10010.0000 ==> 0010.0000 ==> 0x20 = 32, no Overflow
~~~

Il microcode opportunamente codificato dell'istruzione A - B porterebbe a 1 gli ingressi I1 e I6 del '151, mentre tutti gli altri ingressi sarebbero a 0; l'operazione produrrebbe Q7=0, B7=0 e A7=0 sugli ingressi di selezione (S2-S1-S0 = 000), che attiverebbero l'ingresso **I0** che risulta a 0, pertanto l'uscita Q del '151 sarebbe a 0, indicando che non vi è Overflow --> situazione verificata correttamente.

![74LS151](../../assets/math/overflow-74151-i0.png){:width="50%"}

---

- **Caso 4:** 0x90 - 0x30; sottrazione A - B tra un Signed negativo e un Signed positivo.

~~~text
    Hex        Dec        Bin             2C
C                                        1  1
A   0x90  ==>  -112  ==>  1001.0000  ==>  1001.0000 +
B  -0x30  ==>   -48  ==>  0011.0000  ==>  1101.0000 = 
               ----                      ---------- 
Q              -160                      10110.0000 ==> 0110.0000 ==> 0x60 = 96, Overflow
~~~

Il microcode opportunamente codificato dell'istruzione A - B porterebbe a 1 gli ingressi I1 e I6 del '151, mentre tutti gli altri ingressi sarebbero a 0; l'operazione produrrebbe Q7=0, B7=0 e A7=1 sugli ingressi di selezione (S2-S1-S0 = 001), che attiverebbero l'ingresso **I1** che risulta a 1 in quanto attivato dal microcode, pertanto l'uscita Q del '151 sarebbe a 1, indicando che vi è Overflow --> situazione verificata correttamente.

![74LS151](../../assets/math/overflow-74151-i1.png){:width="50%"}

Riprendendo la spiegazione dell'esempio svolto in testa ai quattro casi appena discussi, si noti che anche i casi 3 e 4 sono sottrazioni nelle quali il sottraendo è positivo: in entrambi i casi l'ALU eseguirà internamente una somma del minuendo nel suo stato originario e del sottraendo invertito col complemento a 2.

Tornando poi all'interpretazione dell'hardware, abbiamo anticipato che i moduli ALU del computer NQSAP e del computer BEAM utilizzano solo le istruzioni A + B e A - B, dunque possiamo semplificare le connessioni del '151 eliminando B - A:

![74LS151](../../assets/math/overflow-74151-a+b-a-b.png){:width="50%"}

I segnali (A + B) e (A - B) dovranno avere una connessione hardwired con l'Instruction Register per permetterci di identificare rispettivamente le istruzioni di somma e di sottrazione. Quando nella descrizione dei quattro casi appena visti si indica "Il microcode opportunamente codificato..." si intende infatti che tali istruzioni dovranno rispettare dei vincoli nella scelta degli opcode, in modo da poter sapere con certezza se stiamo eseguendo l'una o l'altra.

Riprendendo lo schema della sezione [Overflow](../flags/#overflow) dalla pagina del modulo Flag, possiamo ora applicare quanto visto in questa pagina per comprenderne il funzionamento e visualizzare la truth table definitiva:

![Utilizzo del 74LS151 nell'NQSAP per il calcolo dell'Overflow con evidenza degli MSB di H, B e dell'ALU e degli ingressi di selezione dell'operazione IR-Q1 e IR-Q3.](../../assets/flags/30-flag-v-151.png){:width="50%"}

*Utilizzo del 74LS151 nell'NQSAP per il calcolo dell'Overflow con evidenza degli MSB di H, B e dell'ALU e degli ingressi di selezione dell'operazione IR-Q1 e IR-Q3.*

Alla luce di tutte le considerazioni fatte, questa tabella può assumere ora un significato più semplice rispetto a quanto non si potesse inizialmente pensare:

| Q7-S2 | B7-S1 | A7-S0 | Sum/Sub   | Input  | Overflow     |
| -     | -     | -     |  -        | -      |  -     |
|   0   |   0   |   0   |           | I0     |  -     |
| **0** | **0** | **1** | **A - B** | **I1** |  **1** |
|   0   |   1   |   0   |           | I2     |  -     |
| **0** | **1** | **1** | **A + B** | **I3** |  **1** |
| **1** | **0** | **0** | **A + B** | **I4** |  **1** |
|   1   |   0   |   1   |           | I5     |  -     |
| **1** | **1** | **0** | **A - B** | **I6** |  **1** |
|   1   |   1   |   1   |           | I7     |  -     |

Il flag Overflow si attiva se:

- **(A7 = B7 = 1 AND Q7 = 0)** *E* stiamo eseguendo una addizione (IR-Q3 attivo), oppure
- **(A7 = B7 = 0 AND Q7 = 1)** *E* stiamo eseguendo una addizione (IR-Q3 attivo), oppure
- **(A = 1 AND B = 0 AND Q = 0)** *E* stiamo eseguendo una sottrazione (IR-Q1 attivo), oppure
- **(A = 0 AND B = 1 AND Q = 1)** *E* stiamo eseguendo una sottrazione (IR-Q1 attivo).

Per identificare l'esecuzione di un'operazione di addizione o di sottrazione, si utilizzano due linee dell'Instruction Register:

| IR-Q3 | IR-Q1 | Operazione  |
| -     | -     | -           |
| LOW   | HIGH  | Sottrazione |
| HIGH  | LOW   | Addizione   |

Detto in altri termini, gli opcode delle istruzioni di addizione e sottrazione dovranno avere i bit 1 e 3 settati come in tabella, mentre gli altri bit non avranno tali vincoli:

| bit |  Addizione | Sottrazione |
| -   |  -         | -           |
|   0 |      X     |           X |
|   1 | **LOW**    |    **HIGH** |
|   2 |      X     |           X |
|   3 | **HIGH**   |     **LOW** |
|   4 |      X     |           X |
|   5 |      X     |           X |
|   6 |      X     |           X |
|   7 |      X     |           X |

Questo corrisponde esattamente a quanto indicato nella sezione [Un esempio pratico](../alu/#un-esempio-pratico) della pagina del modulo ALU, nella quale si evidenzia ad esempio che le istruzioni di somma dovranno presentare "01001 sui 5 bit comuni tra Instruction Register e ALU": il bit numero 3 è a 1, mentre il bit numero 1 è a 0, esattamente come richiesto dalla tabella precedente.

![Connessioni in uscita dall'Instruction Register dell'NQSAP.](../../assets/math/IR-to-74151.png){:width="66%"}

Per finire, da quanto visto fino ad ora possiamo dedurre un'altra regola: la somma di due Signed di segno opposto e la sottrazione di due Signed dello stesso segno non possono causare Overflow.

## Gli Adder

Abbiamo discusso di somme e sottrazioni, dando quasi per scontato che esistano dei circuiti che le implementano. Un accenno si trova in uno schema presente nella sezione [Metodi di identificazione dell’Overflow](#metodi-di-identificazione-delloverflow), ma non ci accontentiamo e vogliamo saperne di più. Come si effettuano le addizioni? E le sottrazioni?

Un Adder è un unità logica basilare che permette di eseguire somme e, opportunamente configurata, sottrazioni.

### Le somme con gli Adder

Quali sono i possibili casi di somme tra due bit A e B?

~~~text
Ipotesi     1a     2a     3a     4a
C                               1   
A           0+     0+     1+     1+
B           0=     1=     0=     1=
          ----   ----   ----   ----
Q           0      1      1     10 
~~~

Nella quarta ipotesi notiamo che la somma di due bit entrambi a 1 genera un riporto, cioè un Carry "C". Scrivendo la truth table e considerando il Carry generato dalla somma dei bit A e B, otteniamo:

| A | B | **Q** | C<sub>OUT</sub> |
| - | - | -     | -               |
| 0 | 0 | 0     | 0               |
| 0 | 1 | 1     | 0               |
| 1 | 0 | 1     | 0               |
| 1 | 1 | 0     | 1               |

Il risultato Q della somma dei due bit A e B si può ottenere con una porta logica XOR, mentre il Carry C<sub>OUT</sub> è chiaramente ottenibile con una porta AND. Ecco come si potrebbe costruire il circuito equivalente:

![Half Adder](../../assets/math/half-adder.png){:width="50%"}

*Half Adder.*

Il circuito appena creato viene definito Half Adder. Il Carry, quando attivo, evidenzia una situazione di overflow, cioè di risultato dell'operazione che eccede la capacità di rappresentazione permessa dai bit disponibili.

Ipotizziamo ora di voler effettuare una somma a più bit, ad esempio di due nibble (4 bit) o di due byte:

~~~text
C       1 11           1  1
A        1011+        10110011+
B        1001=        00101010=
        ------       ----------
Q       10100         11011101
~~~

Si noti che per svolgere l'intera operazione è necessario fornire all'Adder 3 valori in input: A, B e il Carry eventualmente derivante dalla precedente colonna.

Scrivendo la truth table per sommare i tre bit di ogni colonna, assegneremo il nome C<sub>IN</sub> al carry derivante dalla precedente colonna e, similarmente, chiameremo C<sub>OUT</sub> il carry risultante dalla somma di ogni tripletta C<sub>IN</sub>, A e B:

| C<sub>IN</sub> | A | B | Q | C<sub>OUT</sub> |
| -              | - | - | - | -               |
| 0              | 0 | 0 | 0 | 0               |
| 0              | 0 | 1 | 1 | 0               |
| 0              | 1 | 0 | 1 | 0               |
| 0              | 1 | 1 | 0 | 1               |
| 1              | 0 | 0 | 1 | 0               |
| 1              | 0 | 1 | 0 | 1               |
| 1              | 1 | 0 | 0 | 1               |
| 1              | 1 | 1 | 1 | 1               |

Anziché ricorrere alla combinazione di AND e OR (Sum of Products, esposta nel video di Brock LaMeres <a href="https://www.youtube.com/watch?v=04qG6HhC0wM" target="_blank">Combinational Logic Synthesis: SOP Forms & Minterms</a>), proviamo a semplificare utilizzando le mappe di Karnaugh (spiegate nel video <a href="https://www.youtube.com/watch?v=Y18RPvtS9AU" target="_blank">Combinational Logic Minimization: K-map Formation</a>, ancora di LaMeres).

L'analisi di **Q** produce la seguente mappa (nella quale C corrisponde al C<sub>IN</sub> visto nella truth table):

![Karnaugh Map](../../assets/math/Kmap1.png){:width="35%"}

Pur non potendo semplificare la truth table perché non è possibile creare gruppi di 1, si può notare il pattern "a scacchiera", che indica che il circuito logico equivalente è una porta XOR con tre ingressi (si veda il video <a href="https://www.youtube.com/watch?v=3SwLBw7RYiI" target="_blank">Combinational Logic Minimization: XORs</a>, sempre di Lameres).

L'analisi di **C<sub>OUT</sub>** produce quest'altra mappa (nella quale, come in precedenza, C corrisponde al C<sub>IN</sub> visto nella truth table):

![Karnaugh Map](../../assets/math/Kmap2.png){:width="35%"}

Utilizzando la proprietà distributiva dell'algebra booleana, è possibile semplificare la funzione

F = B\*C<sub>IN</sub> + A\*C<sub>IN</sub> + A\*B in

F = C<sub>IN</sub>\*(A+B) + A*B, cioé

C<sub>OUT</sub> = C<sub>IN</sub>\*(A+B) + A*B

Avendo a disposizione l'Adder a due bit visto in precedenza, ci si chiede ora se non sia possibile riutilizzarlo in scala per semplificare la costruzione di un Adder in grado di realizzare somme tra word di lunghezza ad esempio di 4 bit, 8 bit e così via.

Tentiamo di rispondere al quesito. Si è visto che ogni Half Adder mette a disposizione una porta XOR e una porta AND, entrambe a due ingressi.

1) Per realizzare la somma **Q** è necessaria una porta XOR a tre ingressi, ma è dimostrabile che due porte XOR a due ingressi posizionate in cascata producono lo stesso risultato. Infatti, ipotizzando di avere due Half Adder, è possibile ottenere il risultato Q sfruttando le due porte XOR disponibili:

![Due Half Adder](../../assets/math/full-adder-1.png){:width="100%"}

*Due Half Adder.*

2) Per realizzare **C<sub>OUT</sub>** dobbiamo invece implementare la funzione F = C<sub>IN</sub>\*(A+B) + A\*B.

A\*B è disponibile sull'output della porta AND del primo Adder; C<sub>IN</sub>\*(A+B) richiede una OR tra A e B e una successiva AND con C<sub>IN</sub>. Purtroppo, nessuno dei due Adder mette a disposizione una porta OR per effettuare (A+B); tuttavia, è dimostrabile che la funzione:

C<sub>OUT</sub> = C<sub>IN</sub>\*(A+B) + A\*B, non realizzabile con le porte a disposizione, è equivalente alla funzione

C<sub>OUT</sub> = C<sub>IN</sub>\*(A⊕B) + A\*B, realizzabile con le porte a disposizione (si noti la sostituzione della OR "+" con la XOR "⊕").

| C<sub>IN</sub> | A | B | A\*B | A⊕B | A+B | Q | C<sub>IN</sub>\*(A+B) | C<sub>IN</sub>\*(A+B)<br><center>+ A*B |C<sub>IN</sub>\*(A⊕B) |C<sub>IN</sub>\*(A⊕B)<br><center>+ A*B |
| -              | - | - |  -   |  -   |  -  | - | -                     | -                                      | -                     | -                                      |
| 0              | 0 | 0 |  0   |  0   |  0  | 0 | <center>0             |  <center>0                             | <center>0             | <center>0                              |
| 0              | 0 | 1 |  0   |  1   |  1  | 1 | <center>0             |  <center>0                             | <center>0             | <center>0                              |
| 0              | 1 | 0 |  0   |  1   |  1  | 1 | <center>0             |  <center>0                             | <center>0             | <center>0                              |
| 0              | 1 | 1 |  1   |  0   |  1  | 0 | <center>0             |  <center>1                             | <center>0             | <center>1                              |
| 1              | 0 | 0 |  0   |  0   |  0  | 1 | <center>0             |  <center>0                             | <center>0             | <center>0                              |
| 1              | 0 | 1 |  0   |  1   |  1  | 0 | <center>1             |  <center>1                             | <center>1             | <center>1                              |
| 1              | 1 | 0 |  0   |  1   |  1  | 0 | <center>1             |  <center>1                             | <center>1             | <center>1                              |
| 1              | 1 | 1 |  1   |  0   |  1  | 1 | <center>1             |  <center>1                             | <center>0             | <center>1                              |

Come si può vedere dalla truth table, l'output della terz'ultima colonna **C<sub>IN</sub>\*(A+B) + A\*B** è uguale a quello dell'ultima colonna **C<sub>IN</sub>\*(A⊕B) + A\*B**, pertanto, possiamo utilizzare l'Half Adder con porta XOR per completare il nostro Adder migliorato.

A questo punto, possiamo realizzare la funzione necessaria aggiungendo una semplice OR per effettuare la somma logica tra **C<sub>IN</sub>\*(A⊕B)** e **A\*B**.

![Full Adder](../../assets/math/full-adder-2.png){:width="100%"}

*Full Adder.*

Il **Full Adder** appena creato è in grado di effettuare la somma di due termini tenendo in considerazione il Carry in ingresso e generando un eventuale Carry in uscita.

## Multiple Bit Adder

Per effettuare somme di più bit si utilizzano i Multiple Bit Adder. Il ruolo del Carry negli Adder è fondamentale e vi sono almeno due modalità di gestione del Carry. Nelle prossime sezioni si descrivono le modalità Ripple Carry Adder e Carry Look Ahead, implementate anche nei <a href="https://www.ti.com/lit/ds/symlink/sn54ls181.pdf" target="_blank">74LS181</a> utilizzati nell'[ALU](../alu/#carry-addizioni-e-sottrazioni).

### Ripple Carry Adder

Avendo a disposizione i Full Adder costituiti da due Half Adder e una porta OR, possiamo ora creare un Multiple Bit Adder per effettuare somme di nibble, byte e, più in generale, word di qualsiasi dimensione. Si noterà che questo paragrafo permette di rispondere positivamente al quesito che ci interrogava sulla possibilità di implementare un circuito complesso a partire da una logica di base semplice e ripetibile.

![Multiple Bit Adder, o Ripple Carry Adder](../../assets/math/multiple-bit-adder.png){:width="100%"}

*Multiple Bit Adder, o Ripple Carry Adder, a 4 bit.*

La disposizione del Multiple Bit Adder in figura segue l'ordine di presentazione dei numeri binari, con il bit più significativo a sinistra e quello meno significativo a destra. Il C<sub>IN</sub> deve essere mantenuto allo stato LO, così il risultato dell'operazione dipenderà esclusivamente dagli input presenti in A0-A3 e B0-B3.

Il Multiple Bit Adder creato ha un nome univoco: Ripple Carry Adder (RCA), perché il Carry "ondeggia", si muove tra i Full Adder. Una considerazione da non tralasciare è, infatti, quella relativa al timing: come per ogni circuito logico, possiamo prelevare i segnali in output solo una volta che l'output col ritardo maggiore si è stabilizzato.

L'immagine seguente, tratta dal video <a href="https://www.youtube.com/watch?v=TNoQ_djJW0I" target="_blank">Ripple Carry Adders Timing</a>, aiuta a visualizzare come il segnale C<sub>OUT</sub> dipenda da ben 9 livelli (nell'analisi di un circuito, si deve considerare il caso peggiore).

![Timing di un Ripple Carry Adder](../../assets/math/rca-timing.png){:width="80%"}

*Timing di un Ripple Carry Adder.*

Di conseguenza, la frequenza operativa massima del circuito non potrà superare l’inverso della somma dei ritardi introdotti dai livelli 1-9.

### Carry Look Ahead Adder

Un Adder di tipo Carry Look Ahead (CLA) affronta il problema del ritardo causato dalla lunga catena di livelli presente in un Ripple Carry Adder.

Se in un Ripple Carry Adder a 4 bit il ritardo può essere modesto, il ritardo in un Adder a 16 o più bit può ridurre significativamente la frequenza massima di lavoro. Infatti, ogni Full Adder di un RCA dipende dal risultato dell'Adder precedente; dovendo considerare il caso peggiore, il risultato dell'ultimo Full Adder dipende da tutti i Full Adder precedenti.

Per indirizzare la problematica appena esposta, è necessario che ogni Full Adder possa calcolare il proprio Carry Out in maniera indipendente dal Carry Out reso disponibile dallo stadio precedente. Perché questo accada, ad ogni Full Adder deve essere aggiunta circuiteria in grado di calcolare il proprio Carry In, senza che si utilizzi il Carry Out del Full Adder precedente.

Chiaramente, questo porta ad un compromesso: la velocità di risposta di ogni Full Adder aumenta, a discapito della aumentata complessità dovuta all'incremento di porte logiche.

![Carry Look Ahead Adder](../../assets/math/carry-look-ahead-schema.png){:width="80%"}

*Carry Look Ahead Adder.*

L'immagine mostra che gli ingressi dei vari stadi di un Carry Look Ahead Adder (CLA) dipendono solamente dai termini A e B e dal Carry C<sub>0</sub>; questo è possibile grazie a una serie di sostituzioni algebriche che permettono ad ogni singolo Full Adder di poter computare la propria somma e il proprio Carry Out partendo dagli ingressi A, B e da C<sub>0</sub>.

In altre parole, si crea una logica dipendente dai soli termini A e B e dal Carry C<sub>0</sub>. Come si ottiene questo risultato?

Identificando le situazioni nelle quali un Carry viene *generato* o *propagato*, ogni Adder<sub>(i)</sub> può essere dotato di un circuito in grado di sapere se troverà un Carry in ingresso computandolo a partire *dagli ingressi* di tutti gli Adder precedenti e da C<sub>0</sub>, ossia senza dipendere da quanto presente *dall'uscita* C<sub>OUT</sub> dell'Adder<sub>(i-1)</sub> precedente.

Si deve trovare risposta alla domanda chiave "in quali situazioni un Adder<sub>(i)</sub> trova un Carry In a 1 sul proprio ingresso?"

Riducendo questo concetto a espressioni logiche, due sono i casi da analizzare:

1. In quali situazioni un Full Adder<sub>(i-1)</sub>, il cui Carry In è a 0, ***genera*** certamente un Carry Out che viene passato al prossimo Full Adder<sub>(i)</sub>?
2. In quali situazioni un Full Adder<sub>(i-1)</sub> ***propaga*** un potenziale Carry In al prossimo Full Adder<sub>(i)</sub>?

Le due situazioni appena descritte vengono tradotte in espressioni denominate **Generate** e **Propagate**.

1. In quali casi un Full Adder **genera** un Carry Out senza che al suo ingresso Carry In sia presente un Carry? Analizzandone la truth table, troviamo che se C<sub>IN</sub> è a 0, il C<sub>OUT</sub> è a 1 solo se entrambi A **e** B sono a 1: dunque, per realizzare questo circuito possiamo utilizzare una porta AND. Questo caso viene descritto con l'espressione **g = A\*B** e si può leggere come "la logica Generate di ogni Full Adder corrisponde ad A AND B".

2. Quando, invece, un Full Adder **propaga** un Carry presente sul suo ingresso Carry In? Se il C<sub>IN</sub> di quell'Adder è a 1, il C<sub>OUT</sub> è a 1 quando A **o** B sono a 1: dunque, questo circuito può essere realizzato utilizzando una porta OR. Questo caso viene descritto con l'espressione **p = A+B** e si può leggere come "la logica Propagate di ogni Full Adder corrisponde ad A OR B".

Unendo i puntini, per ogni posizione si crea un segnale **G**enerate se entrambi i bit A e B sono a 1 e un segnale **P**ropagate se almeno uno dei due bit è a 1:

| C<sub>IN</sub> | A     | B     | Q | C<sub>OUT</sub> | Generate / Propagate    |
| -              | -     | -     | - | -               | -                       |
| 0              | 0     | 0     | 0 | 0               |                         |
| 0              | 0     | 1     | 1 | 0               |                         |
| 0              | 1     | 0     | 1 | 0               |                         |
| **0**          | **1** | **1** | 0 | **1**           | **Generate** (A AND B)  |
| 1              | 0     | 0     | 1 | 0               |                         |
| **1**          | 0     | **1** | 0 | **1**           | **Propagate** (A OR B)  |
| **1**          | **1** | 0     | 0 | **1**           | **Propagate** (A OR B)  |
| **1**          | **1** | **1** | 1 | **1**           | **Propagate** (A OR B)  |

*Truth table di un Full Adder.*

Detto diversamente, la circuiteria Look Ahead valuta se lo stadio precedente introduce un Carry Out analizzando le due condizioni precedenti. Dati gli ingressi A, B e C<sub>IN</sub>:

- l'espressione Generate viene utilizzata da un Adder "i" per identificare quando, in assenza di un Carry agli ingressi dello stadio precedente "i-1", questi produrrà ("genererà") un Carry C<sub>OUT</sub> (condizione verificata solo se A*B = 1), Carry Out che l'Adder<sub>(i)</sub> ritroverà sul suo Carry In;
- l'espressione Propagate viene utilizzata da un Adder "i" per identificare quando, in presenza di un Carry agli ingressi dello stadio precedente "i-1", questi produrrà ("propagherà") un Carry C<sub>OUT</sub> (condizione verificata solo se C<sub>IN</sub> = 1 e A+B = 1), Carry Out che l'Adder<sub>(i)</sub> ritroverà sul suo Carry In.

A questo punto, possiamo identificare l'eventuale presenza del Carry Out di ogni stadio con una generica espressione:

C<sub>OUT</sub> = g + p\*C<sub>IN</sub>, cioè, effettuando le sostituzioni di **g** e **p**:

C<sub>OUT</sub> = A*B + (A+B)\*C<sub>IN</sub>, con la quale abbiamo già familiarità, perché altri non è il C<sub>OUT</sub> di ogni Full Adder.

Utilizziamo ora **i** per identificare la posizione di ogni bit all'interno del Multiple Bit Adder, ad esempio 0-3 per un Adder a 4 bit, e scrivere le espressioni generali per **p** e **g**:

g<sub>i</sub> = A<sub>i</sub>\*B<sub>i</sub> (ad esempio, g<sub>2</sub> = A<sub>2</sub>\*B<sub>2</sub>)\
p<sub>i</sub> = A<sub>i</sub>+B<sub>i</sub> (ad esempio, p<sub>3</sub> = A<sub>3</sub>+B<sub>3</sub>)

Andiamo a computare il Carry In "C" di un generico Adder **i+1**, che equivale al Carry Out dell'Adder **i** che lo precede, analizzando gli input di quest'ultimo:

C<sub>i+1</sub> = g<sub>i</sub> + p<sub>i</sub>\*C<sub>i</sub>, che equivale a\
C<sub>i+1</sub> = A<sub>i</sub>\*B<sub>i</sub> + (A<sub>i</sub>+B<sub>i</sub>)\*C<sub>i</sub>

Possiamo ora scrivere le espressioni per i Carry In di tutti gli stadi. L'espressione per il Carry In del secondo Adder è:

C<sub>1</sub> = g<sub>0</sub> + p<sub>0</sub>\*C<sub>0</sub>, cioè,\
C<sub>1</sub> = A<sub>0</sub>\*B<sub>0</sub> + (A<sub>0</sub>+B<sub>0</sub>)\*C<sub>0</sub>

Si noti che questa espressione dipende dai soli input A, B e C<sub>0</sub>, che corrisponde esattamente a quanto si desidera fare: non dipende dal Carry Out dell'Adder precedente.\
Andiamo ora a scrivere l'espressione per il Carry In del terzo Adder:

C<sub>2</sub> = g<sub>1</sub> + p<sub>1</sub>\*C<sub>1</sub>

Dobbiamo sbarazzarci di C1, perché stiamo cercando di rendere ogni Adder indipendente dal Carry Out dell'Adder precedente, dunque, sostituendo C<sub>1</sub>:

C<sub>2</sub> = g<sub>1</sub> + p<sub>1</sub>\*(g<sub>0</sub> + p<sub>0</sub>\*C<sub>0</sub>), che significa che C<sub>2</sub> non dipende dal risultato dell'Adder precedente, ma solo dagli input A e B degli Adder precedenti e da C<sub>0</sub>. Applicando la proprietà distributiva, si ottiene:

C<sub>2</sub> = g<sub>1</sub> + p<sub>1</sub>\*g<sub>0</sub> + p<sub>1</sub>\*p<sub>0</sub>*C<sub>0</sub>, cioè:\
C<sub>2</sub> = A<sub>1</sub>\*B<sub>1</sub> + (A<sub>1</sub>+B<sub>1</sub>)\*A<sub>0</sub>\*B<sub>0</sub> + (A<sub>1</sub>+B<sub>1</sub>)\*(A<sub>0</sub>+B<sub>0</sub>)\*C<sub>0</sub>

Anche questa espressione dipende, dunque, dai soli input A, B e C<sub>0</sub>.\
Procediamo scrivendo l'espressione per il Carry In del quarto Adder:

C<sub>3</sub> = g<sub>2</sub> + p<sub>2</sub>\*C<sub>2</sub>, cioè, sostituendo C<sub>2</sub>:\
C<sub>3</sub> = g<sub>2</sub> + p<sub>2</sub>\*(g<sub>1</sub> + p<sub>1</sub>\*g<sub>0</sub> + p<sub>1</sub>\*p<sub>0</sub>\*C<sub>0</sub>), che significa che C<sub>3</sub> non dipende dal risultato dell'Adder precedente, ma solo dagli input A e B degli Adder precedenti e da C<sub>0</sub>. Applicando la proprietà distributiva, si ottiene:

C<sub>3</sub> = g<sub>2</sub> + p<sub>2</sub>\*g<sub>1</sub> + p<sub>2</sub>\*p<sub>1</sub>\*g<sub>0</sub> + p<sub>2</sub>\*p<sub>1</sub>\*p<sub>0</sub>\*C<sub>0</sub>, cioé:\
C<sub>3</sub> = A<sub>2</sub>\*B<sub>2</sub> + (A<sub>2</sub>+B<sub>2</sub>)\*A<sub>1</sub>\*B<sub>1</sub> + (A<sub>2</sub>+B<sub>2</sub>)\*(A<sub>1</sub>+B<sub>1</sub>)\*A<sub>0</sub>\*B<sub>0</sub> + (A<sub>2</sub>+B<sub>2</sub>)\*(A<sub>1</sub>+B<sub>1</sub>)\*(A<sub>0</sub>+B<sub>0</sub>)\*C<sub>0</sub>

Come nei casi precedenti, anche questa espressione dipende dai soli input A, B e C<sub>0</sub>.\
Andiamo infine a scrivere l'espressione per C<sub>4</sub>, che è il Carry Out del quarto Adder:

C<sub>4</sub> = g<sub>3</sub> + p<sub>3</sub>\*C<sub>3</sub>, cioè, sostituendo C<sub>3</sub>:\
C<sub>4</sub> = g<sub>3</sub> + p<sub>3</sub>\*(g<sub>2</sub> + p<sub>2</sub>\*g<sub>1</sub> + p<sub>2</sub>\*p<sub>1</sub>\*g<sub>0</sub> + p<sub>2</sub>\*p<sub>1</sub>\*p<sub>0</sub>\*C<sub>0</sub>), che significa che C<sub>4</sub> non dipende dal risultato dell'Adder precedente, ma solo dagli input A e B degli Adder precedenti e da C<sub>0</sub>. Applicando la proprietà distributiva, si ottiene:

C<sub>4</sub> = g<sub>3</sub> + p<sub>3</sub>\*g<sub>2</sub> + p<sub>3</sub>\*p<sub>2</sub>\*g<sub>1</sub> + p<sub>3</sub>\*p<sub>2</sub>\*p<sub>1</sub>\*g<sub>0</sub> + p<sub>3</sub>\*p<sub>2</sub>\*p<sub>1</sub>\*p<sub>0</sub>\*C<sub>0</sub>, cioé:\
C<sub>4</sub> = A<sub>3</sub>\*B<sub>3</sub> + (A<sub>3</sub>+B<sub>3</sub>)\*A<sub>2</sub>\*B<sub>2</sub> + (A<sub>3</sub>+B<sub>3</sub>)\*(A<sub>2</sub>+B<sub>2</sub>)\*A<sub>1</sub>\*B<sub>1</sub> + (A<sub>3</sub>+B<sub>3</sub>)\*(A<sub>2</sub>+B<sub>2</sub>)\*(A<sub>1</sub>+B<sub>1</sub>)\*A<sub>0</sub>\*B<sub>0</sub> + (A<sub>3</sub>+B<sub>3</sub>)\*(A<sub>2</sub>+B<sub>2</sub>)\*(A<sub>1</sub>+B<sub>1</sub>)\*(A<sub>0</sub>+B<sub>0</sub>)*C<sub>0</sub>

Osservando le espressioni di ogni Adder, si deduce che tutte dipendono dai soli input A, B e C<sub>0</sub>, che è esattamente lo scopo da perseguire: ogni stadio calcola il proprio Carry senza dover attendere il calcolo del Carry effettuato dallo stadio precedente.

Andiamo ora a realizzare un Carry Look Ahead Adder, ma prima, considerando quanto visto sopra, modifichiamo i Full Adder creando dei *Modified* Full Adder.

La somma **Q**, come sempre, corrisponde ad A<sub>0</sub>⊕B<sub>0</sub>⊕C<sub>0</sub>. Aggiungiamo al Full Adder una porta OR e una porta AND con il solo scopo di ottenere i termini **p** (cioè A<sub>0</sub>+B<sub>0</sub> per il primo Adder) e **g** (cioé A<sub>0</sub>*B<sub>0</sub> per il primo Adder).

![Logica della somma del Modified Full Adder](../../assets/math/modified-full-adder-1.png){:width="20%"}

*Logica della somma del Modified Full Adder.*

Aggiungiamo ora la logica del Carry C<sub>1</sub> = g<sub>0</sub> + p<sub>0</sub>\*C<sub>0</sub>:

![Logica del Carry del Modified Full Adder](../../assets/math/modified-full-adder-2.png){:width="23%"}

*Logica del Carry del Modified Full Adder.*

Si noti che il Carry Out C<sub>1</sub> del primo Adder si trova al livello 3, che significa che il percorso più lungo è rappresentato da una OR che ha in ingresso A<sub>0</sub> e B<sub>0</sub>, il cui output p<sub>0</sub> entra in una AND, il cui output entra in una OR finale dalla quale si ottiene C<sub>1</sub>. In effetti, a questo punto, siamo nella stessa situazione del RCA, il cui C<sub>1</sub> veniva generato al 3° livello.

Aggiungendo il prossimo Modified Full Adder, si verifica che la somma è computata al 4° livello, mentre il Carry è, come nell'adder precedente, computato ancora al 3° livello (OR in basso a sinistra).

![Modified Full Adder a due stadi](../../assets/math/modified-full-adder-3.png){:width="42%"}

*Modified Full Adder a due stadi.*

Proseguendo con il terzo e con il quarto Modified Full Adder, la somma permane al 4° livello, mentre il Carry continua ad essere generato al 3° livello.

![Modified Full Adder a quattro stadi](../../assets/math/modified-full-adder-4.png){:width="84%"}

*Modified Full Adder a quattro stadi.*

Come si può notare dagli screenshot, utilizzati per gentile concessione del [professor Lameres](#link-utili), ogni Adder successivo aggiunge molta logica, ma il Multiple Bit Adder creato permette di effettuare somme con un numero fisso di livelli indipendentemente dal numero di bit in input, a differenza del Ripple Carry Adder (per il quale, peraltro, la somma di una word di soli 4 bit richiedeva ben 9 livelli).

In definitiva, il lavoro svolto per creare il Carry in un CLA Adder è molto elevato, ma si hanno in cambio le massime prestazioni.

Un aspetto da tenere in considerazione potrebbe essere il fattore fan-in, che limita il numero di ingressi che una porta logica può avere. La porta OR finale, che genera il Carry Out, avrà un numero di input sempre maggiore all'aumentare del numero di bit dell'Adder, così come le AND connesse a questa OR. A un certo punto sarà necessario suddividere le OR e le AND in due livelli, causando così un incremento di 2 livelli nel computo totale dei livelli dell'Adder. Pertanto, il numero di livelli aumenta anche all'aumentare del numero dei bit del CLA Adder, ma con una velocità decisamente inferiore rispetto a quella del Ripple Carry Adder, nel quale ogni singolo bit aggiunge ben due livelli da attraversare.

### Implementazione nel 74LS181

Il <a href="https://www.ti.com/lit/ds/symlink/sn54ls181.pdf" target="_blank">74LS181</a> implementa internamente un meccanismo di Carry Look Ahead leggermente diverso da quello descritto nella sezione precedente, ma i concetti di base sono simili (Propagate e Generate utilizzati per calcolare il Carry di ogni posizione).

Il meccanismo di Carry Look Ahead (CLA) nel circuito ‘181 può risultare poco intuitivo, poiché strettamente legato ai segnali di selezione S3/S0 che governano le sue 32 funzioni logiche e aritmetiche. Una rappresentazione più semplice da analizzare si trova nel 4-Bit Binary Full Adders With Fast Carry <a href="https://www.ti.com/lit/ds/symlink/sn74ls283.pdf" target="_blank">74LS283</a>, progettato esclusivamente per eseguire somme. Qui, la logica CLA è notevolmente semplificata: per ogni coppia di bit in ingresso (A3/A0 e B3/B0), una porta NAND implementa la funzione di Generate, mentre una porta NOR realizza la funzione di Propagate.

Quando la word è più lunga dei 4 bit gestiti da un singolo chip,  si possono utilizzare le modalità RCA e CLA per l'interconnessione di più ALU:

- La modalità RCA è la più semplice da implementare e consiste nel mettere in cascata più ALU, connettendo il Carry Out di ognuna al Carry In della successiva, come mostrato nell'apposita sezione [Carry, addizioni e sottrazioni](../alu/#carry-addizioni-e-sottrazioni) della pagina ALU.

- La modalità CLA richiede l'utilizzo di un Look-Ahead Carry Generator <a href="https://www.ti.com/lit/ds/symlink/sn54s182.pdf" target="_blank">74S182</a> esterno, che individua la presenza del Carry tra più 74LS181 interconnessi secondo lo schema visibile a pagina 5.

## Le sottrazioni con gli Adder

Nel corso di questa pagina, si è appreso che le sottrazioni binarie si effettuano sommando minuendo e [complemento a due del sottraendo](#numeri-unsigned-e-numeri-signed): anziché eseguire "A - B", si inverte B e lo si somma ad A, ottenendo un'operazione "A + (-B)".

Fin qui tutto facile: possiamo utilizzare un Multiple bit Adder, ad esempio un Ripple Carry Adder o un Carry Look Ahead Adder, e mettere in ingresso A e -B, cioè il complemento a 2 di B. [Ricordiamo](#fonti) che "per convertire un numero positivo in negativo è sufficiente invertire tutti i bit ed aggiungere 1".

Come possiamo invertire i bit di un numero binario ed aggiungere 1? L'*inversione* può essere facilmente effettuata da logica XOR.

| Ctl | B | Q |
| -   | - | - |
| 0   | 0 | 0 |
| 0   | 1 | 1 |
| 1   | 0 | 1 |
| 1   | 1 | 0 |

Ipotizzando di utilizzare un segnale Ctl come segnale di controllo, si può notare che Q è uguale a B se Ctl è a 0, mentre Q è uguale all'inverso di B se Ctl è a 1: dunque, impostando Ctl, si possono invertire tutti i bit del minuendo B.

Rimane l'ultimo passaggio, cioè *aggiungere 1*. Il primo Adder di un Multiple Bit Adder è dotato di un segnale Carry In che, come notato nella sezione [Ripple Carry Adder](#ripple-carry-adder), è normalmente a 0; impostando il Carry In ad 1, otteniamo il risultato desiderato.

Dunque, per negare un numero binario trasformandolo nel proprio complemento a due, possiamo utilizzare delle porte XOR per invertire tutti i bit di ingresso e sfruttare il segnale di Carry In del primo Adder per aggiungere 1: Ben Eater lo illustra molto chiaramente nel video <a href="https://www.youtube.com/watch?v=mOVOS9AjgFs" target="_blank">ALU Design</a> per il SAP-1 computer.

![Dettaglio degli input del Multiple Bit Adder del SAP-1 Computer di Ben Eater](../../assets/math/adder-input-xor.png){:width="80%"}

*Dettaglio degli input del Multiple Bit Adder del SAP-1 Computer di Ben Eater.*

Nello schema abbiamo due RCA <a href="https://www.ti.com/lit/ds/symlink/sn54s283.pdf" target="_blank">74LS283</a> a 4 bit in cascata e un segnale Subtract (SU, da noi rinominato Ctl) che permette di selezionare se gli ingressi B debbano essere normali o invertiti e, contemporaneamente all'eventuale inversione, di aggiungere un Carry In al primo Adder, trasformando B nel suo complemento a 2 per sommarlo ad A ed effettuare finalmente la sottrazione A + (-B).

## Link utili

- <a href="https://www.youtube.com/watch?v=Fu5LfmjhEBA" target="_blank">Ripple Carry Adders (RCA)</a>, <a href="https://www.youtube.com/watch?v=TNoQ_djJW0I" target="_blank">Ripple Carry Adders Timing</a> e <a href="https://www.youtube.com/watch?v=6kTdgkbYZqA" target="_blank">Carry Look Ahead (CLA) Adders</a> di Brock LaMeres. Questi video illustrano la logica degli Half Adder e dei Full Adder, dei Ripple Carry Adder e dei Carry Look Ahead Adders. Non si può non menzionare lo stile espositivo del professor LaMeres, la cui grande carica di energia incolla lo spettatore al video.
- Le criptiche, ma interessantissime e <a href="http://www.6502.org/users/dieter/index.htm" target="_blank">dettagliatissime analisi</a> di Dieter Müller su Adder, Multiplexer e molto, molto altro ancora.
- La playlist completa <a href="https://www.youtube.com/playlist?list=PL643xA3Ie_Et2uM4xu1yFk-A5ZQQ8gQ5e" target="_blank">Intro to Logic Circuits</a> di Lameres, già suggerita anche nella [pagina iniziale](/beam/) della documentazione del BEAM.
