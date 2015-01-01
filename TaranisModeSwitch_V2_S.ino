/*
  Version 1.05 geänderte Pinbelegung für Arduino Nano V3.0

  APM Mode Switch - Flugmode einstellen für ArduPilot Mega autopilot 
  APM Werte in Kanal 1 , Kanal 2-8 frei verfügbar via Analogwerte einlesen
  8 Kanal PPM erzeugen für Trainer Eingang an Taranis, Th9x, 9XR oder anderen Sender
  dort dann PPM1-PPM8 weiterverarbeiten, auswerten, Ansagen machen usw.

  Da ist noch sehr viel Code vom Testen enthalten, den kann man rausnehmen wenn er stört.
  
  Es gibt auch eine fertige Arduino Bibliotek für RC-Technik mit sehr vielen Funktionen!
  
  
  -------------------------------------------------------------
  PPM Generator via Timer 1 Interrupt ist von Stefen Goffertje  
  ------------------------------------------------------------- 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
*/

//Grundeinstellungen 
#define numchan 8                // Number of chanels in PPM stream
#define servo_default 1500       // default servo mitte (1500 = center)
#define PPM_FrLen 22500          // PPM frame max length in microseconds
#define PPM_PulseLen 300         // PPM pulse length
#define PPM_Polarity 1           // PPM polarity -  0 = neg, 1 = pos
#define PPM_outPin 14            // PPM output DIO Pin 14 = PC0 = Analog A0

const byte LED[]={4, 5, 6, 7, 8, 9, 10, 11, 12, 13};       // LED Ausgänge 

int ppm[numchan];                // Feld für die Kanalwerte PPM 


// Einmaliger Durchlauf, Vorbelegen aller Werte--------------------------------
void setup(){
//  Serial.begin(9600);                //Schnittstelle nur für Kontrolle der Daten und Test
  
// LED Port definieren
  for(int i=0;i<=10;i++) {
    pinMode(LED[i], OUTPUT);
  }

// Test der 6 LED mit der Funktion setled()
  setled(-1);                        
  delay(100);
  setled(0);
  delay(100);
  setled(-1);
  delay(100);
  setled(0);
  delay(100);
  setled(-1);
  delay(100);
  setled(0);
  delay(100); 
   
// Kanal 1 mit 1150 vorbelegen
ppm[0]=1150;                     
    
// Kanal 2 bis Kanal 8 mit 1500us vorbelegen
  for(int i=1; i<numchan; i++){    
  ppm[i]= servo_default;
  }

// Ausgangspin für PPM vorbereiten und Pullup Widerstand setzen
  pinMode(PPM_outPin, OUTPUT);
  digitalWrite(PPM_outPin, !PPM_Polarity);

  
// Timer 1 16 Bit Interrupt einstellen/vorbereiten für PPM-Generator  Finger Weg!
  cli();
  TCCR1A = 0; 
  TCCR1B = 0;
  OCR1A = 100;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}


//----------------------------------------------------------------
//Funktion um die LED setzen und rücksetzen abhängig vom Wert nr
void setled(int nr) {
  if (nr==0) {
    for (int i=0;i<=10;i++) {
      digitalWrite(LED[i],LOW);
    }
  }
  else if (nr==-1) {
    for (int i=0;i<=10;i++) {
      digitalWrite(LED[i],HIGH);
    }
  }
  else {
    for (int i=0;i<=10;i++) {
      digitalWrite(LED[i],LOW);
    }
    digitalWrite(LED[nr-1],HIGH);   // hier wird die richtige LED gesetzt
  }
}  


// Programm Hautpschleife------------------------------------------------
void loop() {

// Analogwerte einlesen
  int sw1 = analogRead(A7);  // Emergency
  int sw2 = analogRead(A1);  // MANUAL
  int sw3 = analogRead(A2);  // FBWA & CRUISE
  int sw4 = analogRead(A3);  // ACRO & LOITER
  int sw5 = analogRead(A4);  // --- & AUTO
  int sw6 = analogRead(A5);  // --- & RTH

//Analogwerte PPM(0) zuweisen
       if (sw2 > 900) ppm[0]=1815;   // MANUAL
  else if (sw2 < 100) ppm[0]=1815;   // MANUAL
  else if (sw3 > 900) ppm[0]=1685;   // FBWA
  else if (sw3 < 100) ppm[0]=1555;   // CRUISE
//  else if (sw4 > 900) ppm[0]=1450;   // ACRO
  else if (sw4 < 100) ppm[0]=1425;   // LOITER
//  else if (sw5 > 900) ppm[0]=1650;   // ---
  else if (sw5 < 100) ppm[0]=1295;   // AUTO
//  else if (sw6 > 900) ppm[0]=1850;   // ---
  else if (sw6 < 100) ppm[0]=1165;   // RTH

       if (sw1 > 900) ppm[1]=1000;   // ---
  else if (sw1 < 100) ppm[1]=2000;   // EMERGENCY
  
//PPM-Wertebereiche abfragen und setled(nr) aufrufen
       if ((ppm[0] > 0)    && (ppm[0] < 1230)) setled(10);    // RTH
//  else if ((ppm[0] > 1100) && (ppm[0] < 1200)) setled(9);   // --- 
  else if ((ppm[0] > 1231) && (ppm[0] < 1360)) setled(8);     // AUTO
//  else if ((ppm[0] > 1361) && (ppm[0] < 1490)) setled(7);   // ---
  else if ((ppm[0] > 1361) && (ppm[0] < 1490)) setled(6);     // LOITER
//  else if ((ppm[0] > 1491) && (ppm[0] < 1620)) setled(5);   // ACRO
  else if ((ppm[0] > 1491) && (ppm[0] < 1620)) setled(4);     // CRUISE
  else if ((ppm[0] > 1621) && (ppm[0] < 1750)) setled(3);     // FBWA
//  else if ((ppm[0] > 1751) && (ppm[0] < 1900)) setled(2);   // MANUAL
  else if  (ppm[0] > 1751)                     setled(1);     // MANUAL

       if  (ppm[1] < 1500){
         setled(2);
         ppm[0] = 1425;               // LOITER
       }

// Test der Ausgabe via Serieller Schnittstelle 
//   Serial.print(ppm[0]); Serial.print("  ");
//   Serial.print(ppm[1]); Serial.print("  ");
//   Serial.print(ppm[2]); Serial.print("  ");
//   Serial.print(ppm[3]); Serial.print("  ");
//   Serial.print(ppm[4]); Serial.print("  ");
//   Serial.print(ppm[5]); Serial.print("  ");
//   Serial.print(ppm[6]); Serial.print("  ");
//   Serial.print(ppm[7]); Serial.print("  ");
//   Serial.println("----");
   delay(20);
}
// Hauptschleife Fertig----------------------------------------------



// Interrupt-Service Routine um PPM-Signal zu erzeugen  Finger weg!
ISR(TIMER1_COMPA_vect){
  static boolean state = true;
  
  TCNT1 = 0;
  
  if(state) {
    digitalWrite(PPM_outPin, PPM_Polarity);
    OCR1A = PPM_PulseLen * 2;
    state = false;
  }
  else{
    static byte channel;
    static unsigned int rest;
  
    digitalWrite(PPM_outPin, !PPM_Polarity);
    state = true;

    if(channel >= numchan){
      channel = 0;
      rest = rest + PPM_PulseLen;
      OCR1A = (PPM_FrLen - rest) * 2;
      rest = 0;
    }
    else{
      OCR1A = (ppm[channel] - PPM_PulseLen) * 2;
      rest = rest + ppm[channel];
      channel++;
    }     
  }
}


