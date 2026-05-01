---
title: "Program Counter"
permalink: /docs/programcounter/
excerpt: "Program Counter del computer BEAM"
---
<small>[Note sui segnali](#note-sui-segnali) - [Link utili](#link-utili)</small>

[![Program Counter del computer BEAM](../../assets/pc/35-beam-pc.png "Program Counter del computer BEAM"){:width="100%"}](../../assets/pc/35-beam-pc.png)

Il Program Counter (PC) del computer BEAM presenta poche differenze rispetto al PC del SAP-1 di Ben Eater.

E' ora un registro ad 8 bit anziché a 4 bit, perciò permette di indirizzare 256 byte e non più solo 16:

[![Schema del Program Counter del computer BEAM](../../assets/pc/35-program-counter-schema.png "Schema del Program Counter del computer BEAM"){:width="100%"}](../../assets/pc/35-program-counter-schema.png)

*Schema del Program Counter del computer BEAM.*

I due Synchronous 4-Bit Binary Counters <a href="https://www.ti.com/lit/ds/symlink/sn54ls161a-sp.pdf" target="_blank">74LS161</a> sono connessi in cascata secondo la modalità illustrata a pagina 21 del datasheet:

[![Contatori in cascata](../../assets/pc/35-program-counter-161-rco.png "Contatori in cascata"){:width="66%"}](../../assets/pc/35-program-counter-161-rco.png)

Il Carry Out di un contatore binario a 4 bit, attivo al raggiungimento del conteggio 2^4, permette al contatore successivo di incrementare di un'unità il suo conteggio. Due contatori in cascata permettono di contare fino a 2^4 \* 2^4 = 16 \* 16 = 256.

## Note sui segnali

- L'incremento del PC viene effettuato attivando il segnale PCI nel microcode (Program Counter Increment).
- Il caricamento del PC a un valore specifico a seguito di una istruzione di salto o di ritorno da subroutine viene eseguito attivando il segnale PCJ (Program Counter Jump). La pagina dei Flag include una sezione dedicata all'approfondimento delle [operazioni di salto](../flags/#i-salti-condizionali-e-incondizionati).

## Link utili

- I <a href="https://eater.net/8bit/pc" target="_blank">video di Ben Eater</a> che descrivono il funzionamento dei Flip-Flop e la costruzione del PC.
