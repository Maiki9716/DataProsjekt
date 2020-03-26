/*
  Sensornode

  Describe what it does in layman's terms.  Refer to the components
  attached to the various pins.

  The circuit:
    list the components attached to each input
    list the components attached to each output

  Datatek prosjekt
  Av Michael Berg
*/
// ===============================================================================================================
//          INKLUDERING AV BIBLIOTEK OG DEFINISJON AV OBJEKTER
#define BLYNK_PRINT Serial
#include <analogWrite.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "Adafruit_VL6180X.h"
WidgetTerminal terminal(V5);
BlynkTimer timer;
Adafruit_VL6180X vl = Adafruit_VL6180X();

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================


// ===============================================================================================================
//          GLOBALE VARIABLER
// unsigned long tid = 0;    //Definerer tid for millis funksjon DENNE BRUKES IKKE ENDA!!!¤%&¤#"W¤Q#E%¤&/Y¤#%#"
//int period = 1000;        //Definerer periode (Tid mellom hver gang koden skal kjøre)
float aRead = 0;              //Analog avlesning (0-4095)
float R = 0;                  //Termistor resistans som skal utregnes
float b = 4500;               //Termistor verdi
float R_0 = 10000;            //10k resistans i spenningsdeler med 10k NTC termistor
float T_0 = 20 + 273.15;      //Start temperatur [°C]
float temp = 0;               //Temperatur [°C]

int gass = 0;                 //Analog avlesning for gass høyere = mer "ugass"

int selectedreading = 0;      //Select sensor reading

const int numReadings = 50;        //Lager en 50 lang array
int relevantnumReadings = 10;     //Det er 2-50 elementer som brukes (relevante)
float avlesninger[numReadings];      //Lager en array med lengde 50 som kan holde floats m
int readIndex = 0;                //Indeksen til nåværende avlesning
float total = 0;                  //Total for å finne gjennomsnitt
float average = 0;                //Gjennomsnittet

bool en_boolsk_verdi_for_utregning = 0;

const int gassPin = 33;
const int tempPin = 32;

char auth[] = "thi6MWmSU17ZP4nTzTsTdojm2wV5hJ2x";
char ssid[] = "PBM";
char pass[] = "pbmeiendom";
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================


// ===============================================================================================================
//          SETUP


void setup() {
  Serial.begin(115200);

  //Fyller array med 0
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    avlesninger[thisReading] = 0;
  }


  pinMode(tempPin, INPUT);
  pinMode(gassPin, INPUT);


  //Vent for Serial kommunikasjon
  while (!Serial) {
    delay(1);
  }
  //Vent til I2C kommunikasjon er startet mellom ESP32 og VL6180x
  if (! vl.begin()) {
    Serial.println("Failed to find sensor");
    while (1);
  }

  Blynk.begin(auth, ssid, pass, IPAddress(91, 192, 221, 40), 8080);
  timer.setInterval(50L, myTimerEvent);

}


// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================



// ===============================================================================================================
//          FUNKSJONER


BLYNK_WRITE(V3) {
  switch (param.asInt()) {


    case 1: {
        selectedreading = 1;
        terminal.clear();
        Blynk.setProperty(V4, "label", "Temperatur");
        Blynk.setProperty(V4, "suffix", "Temp");
        Blynk.setProperty(V4, "min", 0);
        Blynk.setProperty(V4, "max", 50);
        Blynk.setProperty(V4, "/pin/", "C");
        break;
      }

    case 2: {
        selectedreading = 2;
        terminal.clear();
        Blynk.setProperty(V4, "label", "Gass");
        Blynk.setProperty(V4, "Name", "Gass");
        Blynk.setProperty(V4, "min", 0);
        Blynk.setProperty(V4, "max", 4095);
        break;
      }



    case 3: {
        selectedreading = 3;
        terminal.clear();
        Blynk.setProperty(V4, "name", "lux");
        Blynk.setProperty(V4, "label", "Lux");
        Blynk.setProperty(V4, "min", 0);
        Blynk.setProperty(V4, "max", 4095);
        break;
      }

  }
}
//---------------------------------------------------------------------------------


//Slider for numreadings
BLYNK_WRITE(V7) {
  relevantnumReadings = param.asInt();
  en_boolsk_verdi_for_utregning = 0;
  total = 0;
  Serial.println(relevantnumReadings);
}


//---------------------------------------------------------------------------------
void myTimerEvent()
{
  aRead = analogRead(gassPin);                             //Leser av analog spenningsverdi
  R = aRead / (4095 - aRead) * R_0;                   //Regner ut termistorresistansen
  temp = - 273.15 + 1 / ((1 / T_0) + (1 / b) * log(R / R_0)); //Regner ut temperaturen i C
  gass = analogRead(tempPin);
  float lux = vl.readLux(VL6180X_ALS_GAIN_5);

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, gass);
  Blynk.virtualWrite(V2, lux);


  if (selectedreading == 1) {
    Blynk.virtualWrite(V4, temp);
    String printstring = "The temperature is: " + String(temp) + "°C\n";
    terminal.print(printstring);
  }

  if (selectedreading == 2) {
    Blynk.virtualWrite(V4, gass);
    String printstring = "The analog gas reading is: " + String(gass) + "\n";
    terminal.print(printstring);
  }

  if (selectedreading == 3) {
    Blynk.virtualWrite(V4, lux);
    String printstring = "The lux measurement is: " + String(lux) + "\n";
    terminal.print(printstring);
  }






  // read from the sensor:
  avlesninger[readIndex] = float(temp);
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  Serial.println(readIndex);


  // Dersom vi er på enden av det relevante spektrumet av arrayen..
  if (readIndex >= relevantnumReadings) {
    // Start om igjen..
    en_boolsk_verdi_for_utregning = 1;
    readIndex = 0;
  }

  if (en_boolsk_verdi_for_utregning == 1) {
    total = 0;

    for (int i = 0; i < relevantnumReadings; i++)
    {
      total += float(avlesninger[i]);
    }

    average = total / float(relevantnumReadings);
    Blynk.virtualWrite(V6, average);

    Serial.println(average);
    Serial.println(relevantnumReadings);
  }
}


// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================

//void averageFilter(readIndex, ){
//  }



// ===============================================================================================================
//          SETUP

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================


// ===============================================================================================================
//          SETUP

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ===============================================================================================================






void loop() {
  Blynk.run();
  timer.run();
}
