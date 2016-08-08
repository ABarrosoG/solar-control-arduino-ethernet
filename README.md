# solar control online arduino ethernet Monitoring solar energy equipment

The brico tries to perform online monitoring of a photovoltaic solar energy system, in this case we will use a battery monitor Victron BMV-700 and Arduino.


In the image of the BMV-700 you will see that is supplied with a shunt to interleave wiring battery, measure the intensity direction and intensity batteries that supply them.
Materials used:

    Arduino-A + Power Supply
    Ethernet shield
    NTC thermistors-MF52-103
    Clamp SCT-013-000 (100A)
    A relay module optocoupler comes with two LEDs, one red and one green power status.
    A Converter Module IIC Bidirectional I2C logic level 5v-3.3v
    A small sealed box to put it all.

In this case, the program will rise 24 relative to buffer the BMV device, many of them currently not'm using, but maybe tomorrow if.

The program is also scheduled to use two-hybrid type equipment Voltronic or PIP, usually the slave unit will be off and only lights up when reaches or exceeds the level set in the WtEsclavo variable, for this solder some small wires that go from the relay until the pins hybrid switch, the switch place it in the off position to be the one who opens or closes relay contact. the pin 7 is used.

The BMV will be connected by cable to the Tx and Rx also share Arduino GND.
 For safety I installed between the BMV and arduino module Bidirectional I2C CII converter logic level 5v-3.3 to separate the voltage of both, although in this case it is not necessary for not sharing the pin power, never hurts a little protection .

In parameter "emon1.current (0, 56,606); // Clamp :( input pin, calibration)" you must change the values ​​for calibration of the clamp.

The relay is used and provided with optoacloplador and a pair of LEDs, power status in red and green.

Once you have installed the system, you must register in emoncms.org. when you enter your profile you can view the api key you have to change it which is written in the program, data will start to get on the website, you can view the inputs that reach the portal and decide what you're going to become feed for finally set the graphical interface using feeds. You will find more information on the API documentation https://emoncms.org/site/api

In the next brico and from this system I will make a referral system surpluses, the project will be mounted in a box sealed one WEMOS that collects system information and make referrals to resistive loads while no consumption occurs in batteries.
