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

int count = 1;
long prevTime = 0;


// Einmaliger Durchlauf, Vorbelegen aller Werte--------------------------------
void setup(){
  Serial.begin(9600);                //Schnittstelle nur für Kontrolle der Daten und Test
  
// LED Port definieren
  for(int i=0;i<=10;i++) {
    pinMode(LED[i], OUTPUT);
  }

// Test der 6 LED mit der Funktion setled() / clrled()
  clrled(0);                        
  delay(100);
  setled(0);
  delay(100);
  clrled(0);
  delay(100);
  setled(0);
  delay(100);
  clrled(0);
  delay(100);
  setled(0);
  delay(100); 
  clrled(0);
   
// Kanal 1 bis Kanal 8 mit 1500us vorbelegen
  for(int i=0; i<numchan; i++){    
  ppm[i]= servo_default;
  }
  ppm[3]=2000;

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
//Funktion um die LED zu setzen abhängig vom Wert nr
void setled(int nr) {
  if (nr==0) {
    for (int i=0; i<sizeof(LED); i++) {
      digitalWrite(LED[i],HIGH);   // hier werden alle LEDs gesetzt
    }
  }
  else {
    digitalWrite(LED[nr-1],HIGH);   // hier wird die richtige LED gesetzt
  }
}  
//Funktion um die LED zu rücksetzen abhängig vom Wert nr
void clrled(int nr) {
  if (nr==0) {
    for (int i=0; i<sizeof(LED); i++) {
      digitalWrite(LED[i],LOW);   // hier werden alle LEDs gesetzt
    }
  }
  else {
    digitalWrite(LED[nr-1],LOW);   // hier wird die richtige LED gesetzt
  }
}  
//Funktion um die LED zu rücksetzen abhängig vom Wert nr
void blinkled(int nr, int maxCount) {
  long time = millis();
  
  if (time-prevTime < 100) setled(nr);
  if ((time-prevTime > 200) && (count < maxCount)) {
    clrled(nr);
    count++;
    prevTime = time;
  }
  if ((time-prevTime > 200) && (count >= maxCount)) {
    clrled(nr);
    if (time-prevTime > 1000) {
      count = 1;
      prevTime = time;
    }
  }
}  


// Programm Hautpschleife------------------------------------------------
void loop() {

// Analogwerte einlesen
  int sw1 = analogRead(A1);  // Emergency
  int sw2 = analogRead(A2);  // MANUAL
  int sw3 = analogRead(A3);  // FBWA & CRUISE
  int sw4 = analogRead(A4);  // ACRO & LOITER
  int sw5 = analogRead(A5);  // --- & AUTO
  int sw6 = analogRead(A6);  // --- & RTH

//Analogwerte PPM(1) zuweisen (DR/EXPO)
       if (sw1 > 900) ppm[0]=1000;   // ---
  else if (sw1 < 100) ppm[0]=2000;   // EMERGENCY

       if                  (sw2 < 100)  ppm[1]=1000;  // EXPO-1
  else if ((sw2 >= 100) && (sw2 < 900)) ppm[1]=1500;  // EXPO-2
  else if  (sw2 >= 900)                 ppm[1]=2000;  // EXPO-3

       if                  (sw3 < 100)  ppm[2]=1000;  // OSD-1
  else if ((sw3 >= 100) && (sw3 < 900)) ppm[2]=1500;  // OSD-2
  else if  (sw3 >= 900)                 ppm[2]=2000;  // OSD-3

 
// PPM3 for ArduPlane
 if (sw4 > 900) ppm[3]=1815;   // MODE-1 --> MANUAL
  else if (sw4 < 100) ppm[3]=1685;   // MODE-2 --> LOITER
  else if (sw5 > 900) ppm[3]=1555;   // MODE-3 --> FBWA
  else if (sw5 < 100) ppm[3]=1425;   // MODE-4 --> AUTO
  else if (sw6 > 900) ppm[3]=1295;   // MODE-5 --> STAB
  else if (sw6 < 100) ppm[3]=1165;   // MODE-6 --> RTH

//PPM4 for PX4
 if       (sw4 > 900) ppm[4]=1100;   // MODE-1 --> Manual
  else if (sw4 < 100) ppm[4]=1300;   // MODE-2 --> Assist
  else if (sw5 > 900) ppm[4]=1400;   // MODE-3 --> PosCtl
  else if (sw5 < 100) ppm[4]=1600;   // MODE-4 --> Auto
  else if (sw6 > 900) ppm[4]=1700;   // MODE-5 --> Loiter
  else if (sw6 < 100) ppm[4]=1900;   // MODE-6 --> Return

  
//PPM-Wertebereiche abfragen und setled(nr) aufrufen
  clrled(0);
//       if  (ppm[0] > 1500)                     setled(1);

//       if ((ppm[1] > 0)    && (ppm[1] < 1300)) blinkled(2,1);
//  else if ((ppm[1] > 1300) && (ppm[1] < 1700)) blinkled(2,2);
//  else if  (ppm[1] > 1700)                     blinkled(2,3);

//       if ((ppm[2] > 0)    && (ppm[2] < 1300)) setled(4);
//  else if ((ppm[2] > 1300) && (ppm[2] < 1700)) {setled(4);setled(3);}
//  else if  (ppm[2] > 1700)                     setled(3);

       if ((ppm[3] > 0)    && (ppm[3] < 1230)) setled(10);
  else if ((ppm[3] > 1230) && (ppm[3] < 1360)) setled(9);
  else if ((ppm[3] > 1360) && (ppm[3] < 1490)) setled(8);
  else if ((ppm[3] > 1490) && (ppm[3] < 1620)) setled(7);
  else if ((ppm[3] > 1620) && (ppm[3] < 1750)) setled(6);
  else if  (ppm[3] > 1750)                     setled(5);

// Test der Ausgabe via Serieller Schnittstelle 
//   Serial.print(sw1); Serial.print("  ");
//   Serial.print(sw2); Serial.print("  ");
//   Serial.print(sw3); Serial.print("  ");
//   Serial.print(sw4); Serial.print("  ");
//   Serial.print(sw5); Serial.print("  ");
//   Serial.print(sw6); Serial.print("  ");
//   Serial.println("----");

//   Serial.print(ppm[0]); Serial.print("  ");
//   Serial.print(ppm[1]); Serial.print("  ");
//   Serial.print(ppm[2]); Serial.print("  ");
//   Serial.print(ppm[3]); Serial.print("  ");
//   Serial.print(ppm[4]); Serial.print("  ");
//   Serial.print(ppm[5]); Serial.print("  ");
//   Serial.print(ppm[6]); Serial.print("  ");
//   Serial.print(ppm[7]); Serial.print("  ");
//   Serial.println("----");
//   delay(120);
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


