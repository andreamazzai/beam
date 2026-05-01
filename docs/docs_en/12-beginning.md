---
title: "Building notes"
lang: en
locale: en-US
sidebar:
    nav: "docs-en"
permalink: /docs/en/buildnotes/
excerpt: "Building notes"
---
<small>[Components](#components) - [LED](#led) - [Miscellaneous](#miscellaneous) - [Breadboard and Wires](#breadboard-and-wires) - [Power Supply](#power-supply) - [Test Circuit](#test-circuit)</small>

[![Note costruttive](../../../assets/varie/cassetti.png "Note costruttive"){:width="100%"}](../../../assets/varie/cassetti.png)

Some general notes on the construction of the BEAM computer.

## Components

Almost all the components used in this project were purchased from <a href="https://www.mouser.com" target="_blank">Mouser.com</a>. The costs are in line with other platforms, but shipping and customs duties are covered by Mouser for orders over €100. Shipping from the United States to Italy is very fast; I received all orders within two business days.

### LED

For convenience, the single red, yellow, and green LEDs used in the BEAM computer come with an internal resistor that allows for direct connection to 5V and, thus, directly to TTL logic outputs. They are <a href="https://www.mouser.it/c/optoelectronics/led-lighting/leds-light-emitting-diodes/single-color-leds/?m=Kingbright&mounting%20style=Through%20Hole&package%20%2F%20case=T-1%203%2F4%20%285%20mm%29&vf%20-%20forward%20voltage=5%20V" target="_blank">available at Mouser</a> for about 20/25 cents each for quantities of 10 or more.

For space reasons, the single LEDs were not included in the Kicad schematics.

Lastly, since Kicad symbols only include 10-LED BARs, in cases where not all 10 LEDs are needed, some of them appear disconnected.

![LED BAR with 2 disconnected LEDs](../../../assets/varie/ledbar.png "LED BAR with 2 disconnected LEDs"){:width="50%"}

*LED BAR with 2 disconnected LEDs.*

### Miscellaneous

Instead of the more common dip-switches, I used very convenient Rocker Switches (like the one in the image) in the MAR and RAM modules; sthey are easily available from <a href="https://us.rs-online.com/product/te-connectivity/5435640-5/70156004/" target="_blank">electronic component distributors</a>. Note that the original pins are quite short and don't secure the switch properly to the breadboard, so I added a socket for integrated circuits.

[![Rocker Switch](../../../assets/ram/20-ram-rocker.png "Rocker Switch"){:width="33%"}](../../../assets/ram/20-ram-rocker.png)

*Rocker Switch.*

### Breadboard and Wires

The breadboards used are the <a href="https://eu.mouser.com/ProductDetail/BusBoard-Prototype-Systems/BB830?qs=VEfmQw3KOauhPeTwYxNCaA%3D%3D" target="_blank">BB830</a>, which are also included in Ben Eater’s kits. The quality of the contacts is significantly higher than that of the cheap breadboards found online for just a few Euros. The additional cost is well worth it, as it eliminates the need to worry about false connections and the consequent issues that are often difficult to identify.

For the wires, I also relied on the excellent <a href="https://www.jameco.com/z/JMS9313-01G-22-AWG-6-Color-Solid-Tinned-Copper-Hook-Up-Wire-Assortment-25-Feet_2153705.html" target="_blank">AWG22</a> that I discovered with Ben Eater's kits for the <a href="https://eater.net/8bit" target="_blank">8-bit SAP computer</a> and the <a href="https://eater.net/6502" target="_blank">6502 computer</a>.

The advantage of these wires over other AWG22 ones I found (and tried) is the thinner insulation, which allows for greater wiring density. Unfortunately, I couldn't find them in Europe or on Mouser, so I had to order them from Jameco, bearing the additional cost of shipping and customs. On the other hand, the AWG22 wires sold by Amazon, although listed in Ben Eater's parts list, have thicker insulation which, in my preference, is less suitable.

## Power Supply

Reading the experiences of other users on <a href="https://www.reddit.com/r/beneater/" target="_blank">reddit</a>, it seems that power supply issues are quite common. The FAQ and other documents <a href="https://www.reddit.com/r/beneater/wiki/tips/" target="_blank">written by users</a> provide excellent tips and help address power distribution issues. As for me, after starting with a simple lateral distribution like I did for the SAP-1, I added a power line in the center of the computer by inserting an additional row of lateral sections cut from the breadboards.

[![Power bus](../../../assets/varie/power-bus.png "Power bus"){:width="40%"}](../../../assets/varie/power-bus.png)

As can be seen in the pictures, I did not use capacitors in the BEAM, just like I did not use them in the SAP-1: I relied on the extremely low frequency of the computer's operation, and I must say that both systems are stable.

Finally, in my builds, I noticed that TTL chips seem to tolerate lower voltages than those specified in the datasheets. In some cases, before improving the power distribution with the central bus described earlier, some chips worked fine even at 4.3 volts.

## Test Circuit

Instead of using jumper wires and temporary LEDs to set and display inputs and outputs on the bus and on the modules under construction, I built a simple test circuit, which proved to be particularly useful for speeding up tests and troubleshooting phases.

[![Test Module for Reading from the Bus](../../../assets/varie/test-board.png "Test Module for Reading from the Bus"){:width="100%"}](../../../assets/varie/test-board.png)

*Test Module for Reading from the Bus.*

With the switch pressed, an output corresponding to the bits set on the rocker switch is emitted, while releasing the switch allows the input signals to be displayed on the LED bar. The first two LEDs on the left represent the setting of the circuit in either output or input mode.

[![Test module schematic](../../../assets/varie/test-schema.png "Test module schematic"){:width="100%"}](../../../assets/varie/test-schema.png)

*Test module schematic.*
