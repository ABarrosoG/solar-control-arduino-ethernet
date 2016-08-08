/****************************************************************************************************************
 * **************************************************************************************************************
 *  Fecha: 05/07/2016       Autor: ABarrosoG         SolarWeb alias: esesperao
 *  Material: Arduino + ethernet shield + thermistor NTC-MF52-103 + relé + pinza amperimetrica SCT-013-000
 *  
 *  Objetivo: 
 *            -Primero.- Monitorización online del monitor de baterías, subiendo los datos a emoncms.org
 *            -Segundo.- Mantener apagado el híbrido esclavo tipo Voltronic, cuando la pinza amperimetrica
 *                        detecte menos de lo establecido en el parámetro "WtEsclavo" y enciende el híbrido si 
 *                        detecta más carga de la indicada.
 *  Conexión:   -  Pin digital 7, conexión al relé actuador del híbrido.
 *              -  Pin analógico A1, conexión del thermistor.
 *              -  Pin analógico A0, pinza amperimétrica no invasiva, he empleado el modelo 100A aunque 
 *                  recomiendo el de 30A que ofrece mejor resolución.
 *              -  El monitor BMV de Victron es de 3,3v y Arduino de 5v, la conexión será del monitor pin Tx a Rx
 *                 de Arduino, y GND de ambos, por seguridad he establecido un módulo "I2c CII Convertidor Nivel 
 *                 Logico 5v-3.3v Modulo Bidireccional"
 *                 
 * LICENCIA DE USO APACHE, si mejoras el programa o añades funcionalidades, por favor, compártelo!
 * 
 * https://github.com/electroduende/solar-control-arduino-ethernet/blob/master/LICENSE
 * *****************************************************************************************************************
 * ***************************************************************************************************************** */
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include "EmonLib.h"                   
#include <math.h>

  byte mac[] = {0xxx, 0xxx, 0xxx, 0x00, 0xxx, 0xxx}; //Escribe la tuya!
  IPAddress ip(xxx, xxx, xxx, xxx); //IP estática
  IPAddress subnet(xxx, xxx, xxx, xxx);
  IPAddress DNS(xxx, xxx , xxxx, xxx);
  IPAddress gw(xxx, xxxx, xxx, xxxx); // Puerta de enlace
  
  EthernetClient clientEmon;
  EnergyMonitor emon1;
  SoftwareSerial Victron(0,1); // (RX, TX) RX se ecuentra establecido en el 0
  char p_buffer[80];
  #define P(str) (strcpy_P(p_buffer, PSTR(str)), p_buffer)
  char c;
  String V_buffer;  // Buffer para datos del monitor Victron
  String E_buffer;  // Buffer para datos de ethernet shield
  
  float I; // I Intensidad instantanea
  float V; // V Voltaje instananeo
  int P; //P potencia instantanea     
  float SOC;// SOC estado de carga
  float TTG;// TTG time to go
  float CE;// Consumo Amp/h
  int H1;//Profundidad descarga máxima
  int H2;//Profundidad ultima descarga
  int H3;//Profundidad de la descarga media
  int H4;//Numero de ciclos de carga
  int H5;//Numero de descargas completas
  float H6;//Horas acumuladas amperaje
  float H7;//Minimo voltaje bateria
  float H8;//Maximo voltage bateria
  float H9;//Tiempo desde ultima descarga
  int H10;//Numero de sincronizaciones automaticas
  int H11;//Numero de alarmas voltaje bajo
  int H12;//Numero de alarmas voltaje alto
  int H17;//Cantidad energia descargada
  float H18;//Cantidad de energia cargada
  int Alarm_high_voltage;
  int Alarm_low_voltage;
  int Alarm_low_soc;
  String Alarm; 
  String Relay;  

  double Irms; //Calculo pinza amp
  int WtEsclavo = 9; // W Establece los amperios a los que se activa el hibrido esclavo
  int ReleHibrido = 7; // Pin para arrancar el hibrido paralelo
  
char serverEmon[] = "emoncms.org";     
String apikey = "XXXXa0XXXXX1dXXXX96e8XXXXX4b2XXX";  //api key, regístrate en emoncms.org e introduce tu api key
int node = 0; //if 0, not used, puede tener más nodos
unsigned long lastConnectionTime = 0;          // ultima vez que conectó en milisegundos
boolean lastConnected = false;                 // estado de la conexión última vez a través delloop
const unsigned long postingInterval = 10*1000;  // establece en milisegundos el delay en actualizaciones

void setup() {
  Serial.begin(19200);
  Victron.begin(19200);
  Serial.println("Emoncms client starting...");
  Ethernet.begin(mac, ip, DNS, gw, subnet);
    emon1.current(0, 56.606);// Pinza amperimetrica:( input pin, calibration)//
  pinMode(ReleHibrido, OUTPUT);
}

// <-- Inicio loop
void loop() { 
  
// <-- Victron 
  if (Victron.available()) {
    c = Victron.read();
 
    if (V_buffer.length() <80) {
      V_buffer += c;
    }
 
    if (c == '\n') {  // New line.
  
      if (V_buffer.startsWith("SOC")) {//SOC
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        SOC = (float) temp_int/10; }  
      if (V_buffer.startsWith("V")) {//Voltaje bateria
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        V = (float) temp_int/1000;}     
      if (V_buffer.startsWith("I")) {//Corriente bateria
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        I = (float) temp_int/1000;}
       if (V_buffer.startsWith("P")) {//Potencia instantanea
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        P = (int) temp_int;}   
      if (V_buffer.startsWith("CE")) {//Consumido kwh
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        CE = (float) temp_int/1000;}
       if (V_buffer.startsWith("TTG")) {//Time to go
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        TTG = (float) temp_int/60;}
      if (V_buffer.startsWith("H1")) {//Profundidad descarga máxima
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H1 = (int) temp_int/10;}  
      if (V_buffer.startsWith("H2")) {//Profundidad ultima descarga
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H2 = (int) temp_int/10;}  
      if (V_buffer.startsWith("H3")) {//Profundidad de la descarga media
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H3 = (int) temp_int;}  
      if (V_buffer.startsWith("H4")) {//Numero de ciclos de carga
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H4 = (int) temp_int;}    
      if (V_buffer.startsWith("H5")) {//Numero de descargas completas
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H5 = (int) temp_int;}    
      if (V_buffer.startsWith("H6")) {//Horas acumuladas amperaje
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H6 = (float) temp_int/1000;}
      if (V_buffer.startsWith("H7")) {//Minimo voltaje bateria
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H7 = (float) temp_int/1000;}
      if (V_buffer.startsWith("H8")) {//Maximo voltage bateria
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H8 = (float) temp_int/1000;}
      if (V_buffer.startsWith("H9")) {//Tiempo desde ultima descarga
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H9 = (float) temp_int/86400;}
      if (V_buffer.startsWith("H10")) {//Numero de sincronizaciones automaticas
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H10 = (int) temp_int;}
      if (V_buffer.startsWith("H11")) {//Numero de alarmas voltaje bajo
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H11 = (int) temp_int;} 
      if (V_buffer.startsWith("H12")) {//Numero de alarmas voltaje alto
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        H12 = (int) temp_int;} 
      if (V_buffer.startsWith("H17")) {//Cantidad energia descargada
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H17 = (float) temp_int/1000;}
      if (V_buffer.startsWith("H18")) {//Cantidad de energia cargada
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        double temp_int = temp_string.toInt();
        H18 = (float) temp_int/1000;}
      if (V_buffer.startsWith("Alarm")) {
        Alarm = V_buffer.substring(V_buffer.indexOf("\t")+1);
        Alarm.trim();}     
      if (V_buffer.startsWith("Relay")) {
        Relay = V_buffer.substring(V_buffer.indexOf("\t")+1);
        Relay.trim();}     
      if (V_buffer.startsWith("AR")) {
        String temp_string = V_buffer.substring(V_buffer.indexOf("\t")+1);
        int temp_int = temp_string.toInt();
        if (bitRead(temp_int,0)) {
          Alarm_low_voltage = 1;}
        else {Alarm_low_voltage = 0;}
        if (bitRead(temp_int,1)) {
          Alarm_high_voltage = 1;}
          else {Alarm_high_voltage = 0;}
        if (bitRead(temp_int,2)) {Alarm_low_soc = 1;}
        else {Alarm_low_soc = 0;}
      }
      V_buffer="";
    }
  }
// Victron -->
  if (!clientEmon.connected() && lastConnected) {
    Serial.println();
    Serial.println("Desconectando...");
    clientEmon.stop();
  }
  if(!clientEmon.connected() && (millis() - lastConnectionTime > postingInterval)) {
    // <-- Pinza amperimetrica 
    Irms = emon1.calcIrms(1480);  // Calculate Irms only
    // <-- Activa esclavo
    if (Irms >= WtEsclavo){
      digitalWrite(ReleHibrido,LOW);// Activa el hibrido esclavo
    } else {
      digitalWrite(ReleHibrido,HIGH);
      }
    // Activa esclavo -->
    
    sendData();
  }
  lastConnected = clientEmon.connected();
}
//  Fin Loop -->

// <-- Devuelve grados celsius
double Thermister(int RawADC) {  
double Temp;
Temp = log(((10240000/RawADC) - 10000));
Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
Temp = Temp - 273.15;// Converierte de Kelvin a Celsius
//Para convertir Celsius a Farenheith esriba en esta linea: Temp = (Temp * 9.0)/ 5.0 + 32.0; 
return Temp;
} 
//  Devuelve grados celsius --> 

// <-- Envía datos a emoncms 
void sendData() {
  if (clientEmon.connect(serverEmon, 80)) {
    Serial.println("Connecting...");
    clientEmon.print("GET /api/post?apikey=");
    clientEmon.print(apikey);
    if (node > 0) {
      clientEmon.print("&node=");
      clientEmon.print(node);
    }
    clientEmon.print("&json={Current");
    clientEmon.print(":");
    clientEmon.print(I);    
    clientEmon.print(",Voltage:");
    clientEmon.print(V);
    clientEmon.print(",Potencia:");
    clientEmon.print(P);
    clientEmon.print(",SOC:");
    clientEmon.print(SOC);
    clientEmon.print(",TTG:");
    clientEmon.print(TTG);
    clientEmon.print(",CE:");
    clientEmon.print(CE);
    clientEmon.print(",H1:");
    clientEmon.print(H1);
    clientEmon.print(",H2:");
    clientEmon.print(H2);
    clientEmon.print(",H3:");
    clientEmon.print(H3);
    clientEmon.print(",H4:");
    clientEmon.print(H4);
    clientEmon.print(",H5:");
    clientEmon.print(H5);
    clientEmon.print(",H6:");
    clientEmon.print(H6);
    clientEmon.print(",H7:");
    clientEmon.print(H7);
    clientEmon.print(",H8:");
    clientEmon.print(H8);
    clientEmon.print(",H9:");
    clientEmon.print(H9);
    clientEmon.print(",H10:");
    clientEmon.print(H10);
    clientEmon.print(",H11:");
    clientEmon.print(H11);
    clientEmon.print(",H12:");
    clientEmon.print(H12);
    clientEmon.print(",H17:");
    clientEmon.print(H17);
    clientEmon.print(",H18:");
    clientEmon.print(H18);
    clientEmon.print(",Alarm_high_voltage:");
    clientEmon.print(Alarm_high_voltage);
    clientEmon.print(",Alarm_low_voltage:");
    clientEmon.print(Alarm_low_voltage);
    clientEmon.print(",Alarm_low_soc:");
    clientEmon.print(Alarm_low_soc);
    clientEmon.print(",Alarm:");
    clientEmon.print(Alarm);
    clientEmon.print(",Relay:");
    clientEmon.print(Relay);
    // <-- Thermister
    int val=analogRead(A1);//Lee el valor del pin analogo 0 y lo mantiene como val
    double temp=Thermister(val);//Realiza la conversión del valor analogo a grados Celsius
    clientEmon.print(",Temp:");
    clientEmon.print(temp);
    Serial.println("temp: ");//Escribe la temperatura en el monitor serial debug
    Serial.println(temp);//Escribe la temperatura en el monitor serial debug
    // Thermister -->
    // <-- Pinza amp
    clientEmon.print(",Consumo_W_Hibrid:");
    clientEmon.print(Irms*230); // Potencia W aparente
    clientEmon.print(",Consumo_I:");
    clientEmon.print(Irms); // Irms
    // Pinza amp -->
    clientEmon.println("} HTTP/1.1");
    clientEmon.println("Host:emoncms.org");
    clientEmon.println("User-Agent: Arduino-ethernet");
    clientEmon.println("Connection: close");
    clientEmon.println();

    lastConnectionTime = millis();
  } 
  else {
    // Si no puede establecer conexión:
    Serial.println("Conexion fallida");
    Serial.println("Desconectando...");
    clientEmon.stop();
  }
}
//  Envía datos a emoncms -->
