# Control online y seguimiento del equipo de energía solar con Arduino Ethernet
El brico trata de realizar la monitorización online de un sistema de energía solar fotovoltaico, en este caso emplearemos un monitor de baterías Victron BMV-700 y Arduino.


En la imagen del BMV-700 podrás ver que se suministra con un shunt para intercalar en el cableado de las baterías, medirá la intensidad en dirección a las baterías como la intensidad que suministren éstas.
Materiales empleados:

    Arduino-UNO + Fuente de alimentación
    Ethernet shield
    Termistor NTC-MF52-103  
    Pinza amperimétrica SCT-013-000 (100A) 
    Un módulo relé, viene optoacoplado y con dos leds, uno rojo de alimentación y otro verde de estado.
    Un Modulo Bidireccional I2c CII Convertidor Nivel Logico 5v-3.3v
    Una pequeña caja estanca para meterlo todo. 

En éste caso, el programa subirá 24 relativos al buffer del dispositivo BMV, muchos de ellos actualmente no los estoy usando, pero quizás mañana si.

El programa también está programado para el uso de dos equipos híbridos tipo Voltronic o PIP, normalmente el equipo esclavo se encontrará apagado, y sólo se encenderá cuando alcance o supere el nivel establecido en la variable WtEsclavo, para ello soldaremos unos pequeños hilos que irán desde el relé hasta las patillas del interruptor del híbrido, el interruptor lo colocaremos en la posición apagado para que sea el relé quien abra o cierre el contacto. Se utilizará el pin 7.

El BMV se conectará mediante el cable Tx al Rx de Arduino y compartirán también GND.
 Por seguridad he instalado  entre el BMV y el arduino un Modulo Bidireccional I2c CII Convertidor Nivel Logico 5v-3.3 para separar las tensión de ambos, aunque en este caso no es necesario por no compartir el pin power, nunca esta de más un poco de protección.

En el parámetro "emon1.current(0, 56.606);// Pinza amperimetrica:( input pin, calibration)" deberás cambiar los valores para la calibración de la pinza.

El relé utilizado ya viene provisto de optoacloplador y un par de leds, alimentación en rojo y de estado en color verde.

Una vez tengas instalado el sistema, deberás registrarte en emoncms.org. cuando entres en tu perfil podrás visualizar la api key que tendrás que cambiarla por la que está escrita en el programa, lo datos comenzarán a subirse  al sitio web, podrás visualizar los inputs que llegan al portal y decidir cuales vas a convertir en feed, para finalmente configurar la interfaz grafica haciendo uso de los feeds. Encontrarás más información en la documentación API https://emoncms.org/site/api

En el próximo brico y a partir de este sistema realizaré un sistema de derivación de excedentes, el proyecto consistirá en montar en una cajita estanca un WeMos que recoja información del sistema y realice derivación a cargas resistivas mientras no se produzca consumo en las baterías.
