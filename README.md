# **toastershield**

An SMT reflow oven shield for the Arduino Uno.

## 1. Software
 
 The following steps were tested in Linux (Ubuntu).
 
### Set the following environment variables

`PATH_TO_ARDUINO_IDE = /path/to/your/arduinoIDE`

`ARDUINO_IDE_VER = 1.6.X`  (depending on your version)

`GITHUB_MIRROR = /path/to/your/github/mirror` (optional, local github mirror otherwise https://github.com is used per default)

### Connect Arduino Uno via USB cable

Make sure you have write permissions to `/dev/ttyUSB0` and execute the following commands:

`$make`

`$make upload`

If the shield is attached to the Arduino Uno board, you should hear a beep  and see something on the display.

## 2. Hardware

**_WARNING - DANGER - HIGH VOLTAGE - ELECTRIC SHOCK HAZARD - EXPERTS ONLY
(shield, heater, fan and transformer are connected to 120VAC)_**

### Parts List:

- toaster oven, to be converted (for use on 120VAC)
- Arduino Uno board
- toastershield PCB (see Eagle files and parts list)
- transformer PCB (see Eagle files and parts list)
- SH1106 OLED module
- 2 pushbuttons
- 1 LED,
- 2 wafer connectors (4 pin)
- some ribbon cable (to connect the OLED module and the switches+LED with wafer connectors)
- power cable with ground / 20 A
- power cable feedthrough coupler
- K-type thermocouple wire pair (about 50cm)
- some cable (AWG 16 for heater, AWG 24 for fan and 12VAC supply)
- 6.3mm crimp terminal spade connectors (female, insulated)
- ring tongue crimp terminals (for grounding)
- hardware for mounting boards and grounding
- fiberglass cloth (for thermal insulation)
- silicone glue

PCBs can be made with the [toner transfer method](http://www.instructables.com/id/Cheap-and-Easy-Toner-Transfer-for-PCB-Making) easily (single layer). All goes inside the oven (original buttons, switches, timers, bell aso. are stripped out). Make sure that there is sufficient air ventilation in order to keep the temperature around the boards and cables below 70C (a cooling fan might be necessary). Some thermal insulation on the outside of the oven compartment is necessary (treat fiberglass cloth with silicon glue on the cutting lines before cutting). Cut out an opening for the USB port, preferably on the front (so you can easily update the firmware and optionally monitor and control the oven from a computer).  Drill a hole into the oven compartment wall and feed the thermocouple wire pair through it and seal the hole with silicone glue later (after everything is working and tested). 

**_WARNING - DANGER - HIGH VOLTAGE - ELECTRIC SHOCK HAZARD - EXPERTS ONLY
(shield, heater, fan and transformer are connected to 120VAC)_**

## 3. To Do
- menu for adjusting temperature/time (at the moment fixed for PbSn solder) via pushbuttons
- interface for Processing
- Processing GUI  

***
## Pictures:
![](/pics/shieldtop.jpg)

Picture 1: Assembled shield top.
***

![](/pics/shieldbottom.jpg) 

Picture 2: Assembled shield bottom.
***

![](/pics/unoboard.jpg)

Picture 3: Modified power connection, plug replaced by screw terminals.
***

![](/pics/shield.jpg)  

Picture 4: Wired shield sitting on Uno board.
***

![](/pics/transformer.jpg) 

Picture 5: Wired transformer board.
***

![](/pics/fullwiring.jpg)

Picture 6: Complete wiring.
***
 
![](/pics/wiringfrontpanel.jpg)

Picture 7: Wiring of front panel.
***

![](/pics/frontpanel.jpg)

Pincture 8: Front panel.
