#include <DHT.h>
#include <DHT_U.h>

#include <OneWire.h>
#include <DallasTemperature.h>

////////////////////////////termistor////////////
int analogPin = 0;   // Pin donde esta conectado el divisor de tension en el Arduino
#define sample 50
float Vin = 5.0;     // [V]       Voltage de entrada en el divisor de tension
float R = 37700;  // [ohm]     Valor de la resistencia secundaria del divisor de tension
float Rt, AVo[sample], VRt;       // Resistencia del termistor
float Vo = 0;     // Voltaje de salida del divisor
float T0 = 298.15;   // [K] (25ºC)
float R0 = 100000;    //R termistor a 25°
float beta = 3950;    // [K]        Parametro Beta
float TemC = 0.0;   // [ºC]       Temperatura de salida en grados Celsius

////////////////////////////////////////////////////

float tempDS, Tlm;
/////Dimmer///////
int AC_LOAD = 3;    // Output to Opto Triac pin
int dimming = 100;  // Dimming level (0-128)  0 = ON, 128 = OFF
//////////////
OneWire ourWire(7);                //Se establece el pin 2  como bus OneWire
DallasTemperature sensors(&ourWire); //Se declara una variable u objeto para nuestro sensor

#define DHTPIN 22
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//dht DHT;

float lm[sample],temDHT;
int i,tm;
///////////////////////////////////
void setup() {
  Serial.begin(9600);
  sensors.begin();   //Se inicia el sensor
  //analogReference(EXTERNAL);
  pinMode(analogPin, INPUT);
  pinMode(AC_LOAD, OUTPUT);// Set AC Load pin as output
  attachInterrupt(0, zero_crosss_int, RISING);  // Choose the zero cross interrupt # from the table above
  dht.begin();
}

void loop() {
  
  sensors.requestTemperatures();   //Se envía el comando para leer la temperatura
  tempDS = sensors.getTempCByIndex(0); //Se obtiene la temperatura en ºC

  temDHT = dht.readTemperature();
  
  //Se obtiene el voltaje en el Termistor y LM35///
  for (i = 0; i < sample; i++) {
    AVo[i] = analogRead(analogPin);
    lm[i] = analogRead(A2);
    delay(10);
  }
  // average all the samples out
  float media =0,mediaLm=0;
  for (i = 0; i < sample; i++) {
    media += AVo[i];
    mediaLm += lm[i];
  }
  media /= sample;
  mediaLm /= sample;
  
  Tlm = (mediaLm * 5 * 100) / 1023;
  
  Vo = Vin * ((float)(media)) / 1023;

  
  VRt = Vin - Vo;
  Rt = ((R * VRt) / (Vin)) / (1 - (VRt / Vin));
  TemC =  1 / (((log(Rt / R0)) / beta) + 1 / T0);
  TemC -= 273.15;
  //TemC = (VRt + 12.29) / (0.0491) - 273;
  //Rt = R * ( (Vin / VRt) - 1);
  //////////////////////
  //temDHT=1;
  //Vo=5;
  Serial.print(TemC);
  Serial.print(",");
  Serial.print(tempDS);
  Serial.print(",");
  Serial.print(Tlm);
  Serial.print(",");
  Serial.print(temDHT);
  Serial.print(",");
  Serial.print(VRt);
  Serial.print(",");
  Serial.print(Rt);
  Serial.print(",");
  Serial.println(Tlm/10);


  if (TemC > 38.8)
    dimming += 10;
  if (TemC < 36.8)
    dimming -= 1;

  delay(1000);
}


void zero_crosss_int()  //function to be fired at the zero crossing to dim the light
{
  int dimtime = (65 * dimming);  // For 60Hz =>65
  delayMicroseconds(dimtime);    // Wait till firing the TRIAC
  digitalWrite(AC_LOAD, HIGH);   // Fire the TRIAC
  delayMicroseconds(10);         // triac On propogation delay
  // (for 60Hz use 8.33) Some Triacs need a longer period
  digitalWrite(AC_LOAD, LOW);    // No longer trigger the TRIAC (the next zero crossing will swith it off) TRIAC
}
