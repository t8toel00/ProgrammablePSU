//I2C 2004 LCD SETUP
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);



/*
   ROTARY ENCODER SETUP ALKAA
*/
#define ROTARY_0_SWPIN 8
#define ROTARY_0_DPIN 9
#define ROTARY_0_CLK 10

#define ROTARY_1_SWPIN 11
#define ROTARY_1_DPIN 12
#define ROTARY_1_CLK 13

#define LED_0_PIN 5

//Kaikki muuttujat, joita muokataan ISR:n sisältä, merkataan volatileksi
//ROTARY ENCODER 0 JA 1 GLOBAALIT MUUTTUJAT
volatile unsigned long isrTime = 0;
//ROTARY ENCODER 0 GLOBAALIT MUUTTUJAT
volatile boolean button_0_pressed = false;
volatile byte button_0_state = 0;
volatile byte dPin_0_state = 0;
volatile byte clkPin_0_state = 0;
volatile byte last_dPin_0_state = 0;
volatile byte last_clkPin_0_state = 0;
volatile byte lastbutton_0_state = 0;
volatile unsigned long lastdebounceTime_0 = 0;
volatile int count0 = 0;
//ROTARY ENCODER 1 GLOBAALIT MUUTTUJAT
volatile boolean button_1_pressed = false;
volatile byte button_1_state = 0;
volatile byte dPin_1_state = 0;
volatile byte clkPin_1_state = 0;
volatile byte last_dPin_1_state = 0;
volatile byte last_clkPin_1_state = 0;
volatile byte lastbutton_1_state = 0;
volatile unsigned long lastdebounceTime_1 = 0;
volatile int count1 = 0;
/*
   ROTARY ENCODER SETUP PÄÄTTYY
*/

//DAC SETUP ALKAA
#define DATAPIN 2
#define STCLK 3
#define SHCLK 4
int  voltsBits = 0;
int ampsBits = 0;
//DAC SETUP PÄÄTTYY


#define READ_VOLTS_PIN A0
#define READ_AMPS_PIN A1
#define READ_TEMP_PIN A2
#define READ_IN_VOLTS_PIN A3

//CRITICAL SECTION MUUTTUJAT - Ei saa muokata missään tapauksessa!
//Näihin ei kosketa ISR:n sitältä, joten näitä ei merkata volatileksi. Näihin ei saa koskea mistään critical sectionin ulkopuolelta!
int laskuri0 = 0;
int laskuri1 = 0;
boolean painike_0_painettu = false;
boolean painike_1_painettu = false;




void setup() {
  Serial.begin(9600);

  lcd.begin(20, 4);
  lcd.clear();
  lcd.setCursor(4, 1);
  lcd.print("Ohjelmoitava");
  lcd.setCursor(4, 2);
  lcd.print("Jannitelahde");
  lcd.setCursor(7, 3);
  lcd.print("v 1.1");

  //ROTARY ENCODER SETUP ALKAA
  pinMode(ROTARY_0_SWPIN, INPUT_PULLUP);
  pinMode(ROTARY_0_DPIN, INPUT_PULLUP);
  pinMode(ROTARY_0_CLK, INPUT_PULLUP);

  pinMode(ROTARY_1_SWPIN, INPUT_PULLUP);
  pinMode(ROTARY_1_DPIN, INPUT_PULLUP);
  pinMode(ROTARY_1_CLK, INPUT_PULLUP);

  pinMode(LED_0_PIN, OUTPUT);

  //Asetetaan interrupt-vektori ja pinmask
  PCICR = PCICR | (1 << PCIE0); //PCI0 enabled
  PCMSK0 = PCMSK0 | (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT3) | (1 << PCINT4); //Asetetaan interruptit pinneille PCINT0 jne.

  //Luetaan rotary encoderin PCI-pinni jo setupin aikana, ettei ensimmäinen interrupti laske väärin.
  dPin_0_state = digitalRead(ROTARY_0_DPIN);
  last_dPin_0_state = dPin_0_state;
  dPin_1_state = digitalRead(ROTARY_1_DPIN);
  last_dPin_1_state = dPin_1_state;
  //ROTARY ENCODER SETUP PÄÄTTYY

  //DAC SETUP ALKAA
  pinMode(DATAPIN, OUTPUT);
  pinMode(SHCLK, OUTPUT);
  pinMode(STCLK, OUTPUT);

  for (int i = 0; i < 24; i++) {
    digitalWrite(DATAPIN, LOW);
    digitalWrite(SHCLK, HIGH);
    digitalWrite(SHCLK, LOW);
  }

  digitalWrite(STCLK, HIGH);
  digitalWrite(STCLK, LOW);
  //DAC SETUP PÄÄTTYY
  delay(2000);
}


//YLEISET GLOBAALIT MUUTTUJAT
//Näihin tallennetaan loopissa halutut lähtöarvot. Kalibrointiyhtälöt ja DAC-funktio hoitaa muunnokset ja bittien siirrot.
float volts = 0.000;
float amps = 0.000;
float readVolts = 0.000;
float readAmps = 0.000;
float inVolts = 24.000; //katso readings()
float temp = 0.000;
float power = 0.000;
float heat = 0.000;
float heatLimit = 10.000;

unsigned long loopTime = 0;

boolean shortCircuit = false;
boolean tdpLimit = false;

boolean voltsActive = false;
boolean ampsActive = false;


//LCD-näytön kohteiden päivittämiseen käytettävät booleanit.
//Näyttöä ei päivitetä turhaan, ettei se vilku.
//Näyttö päivitetään osa kerrallaan.
boolean voltsUpdated = true;
boolean ampsUpdated = true;
/* //Nämä ei ole vielä kaytössä
  boolean readVoltsUpdated = false;
  boolean readAmpsUpdated = false;
  boolean tempUpdated = false;
  boolean inVoltsUpdated = false;
  boolean powerUpdated = false;
  boolean heatUpdated = false;
*/


void loop() {
  //CRITICAL SECTION: Tätä ei missään tapauksessa saa muokata!
  PCICR = PCICR & ~(1 << PCIE0); //Ei interrupteja silloin kun luetaan muuttujia, joita muokataan ISR:n sisältä, jottei ne muutu kesken lukemisen.
  /*
    Kahden rotary encoderin toiminta keskeytyksillä. Kaksi rotarya muuttaa kahden laskurin arvoa.
    Nämä laskurit kopioidaan erillisiin muuttujiin looppiin, jotta laskurien arvo ei muutu suorituksen aikana.
    Laskurien arvon voi lisätä mihin tahansa muuttujaan, jonka arvoa halutaan muuttaa.

    Rotaryjen painonapit asettaa kaksi booleania trueksi painettaessa.
    Kun boolean on luettu ja kopioitu, se asetetaan falseksi.
    Lukemalla loopissa seuraavat muuttujat voidaan toteuttaa kaikki tarpeelliset toiminnot:

    #laskuri0 - Yhden loopin aikana tulleiden interruptien määrä: lisätään tai vähennetään halutusta arvosta. Nollautuu joka kierros.
    #laskuri1 - Yhden loopin aikana tulleiden interruptien määrä: lisätään tai vähennetään halutusta arvosta. Nollautuu joka kierros.
    #painike_0_painettu - Onko painiketta painettu viimeisen loopin jälkeen. Nollautuu joka kierros.
    #painike_1_painettu - Onko painiketta painettu viimeisen loopin jälkeen. Nollautuu joka kierros.
  */
  laskuri0 = count0;
  laskuri1 = count1;
  count0 = 0;
  count1 = 0;
  painike_0_painettu = button_0_pressed;
  painike_1_painettu = button_1_pressed;
  button_0_pressed = false;
  button_1_pressed = false;
  PCICR = PCICR | (1 << PCIE0); //PCI0 enabled. Interruptit takaisin päälle.
  //END OF CRITICAL SECTION: Tätä ei missään tapauksessa saa muokata!


  loopTime = millis(); //Tämä aika on globaali tieto jokaisella loopin kierroksella.

  if (painike_0_painettu) {
    voltsActive = !voltsActive;
    ampsActive = false;
  }
  if (painike_1_painettu) {
    ampsActive = !ampsActive;
    voltsActive = false;
  }

  if (voltsActive && (laskuri0 != 0 || laskuri1 != 0)) {
    volts = volts + laskuri0;
    volts = volts + laskuri1 * 0.010;
    voltsUpdated = true;
  }

  if (ampsActive && (laskuri0 != 0 || laskuri1 != 0)) {
    amps = amps + laskuri0;
    amps = amps + laskuri1 * 0.010;
    ampsUpdated = true;
  }


  //Tarkistetaan että haluttu arvo on sallitulla alueella.
  if (volts > 20.000) volts = 20.000;
  if (amps > 2.000) amps = 2.000;
  if (volts < 0.000) volts = 0.000;
  if (amps < 0.000) amps = 0.000;

  //Muunnetaan haluttu lähtöjännite vastaamaan bittiarvoa alueella 0 - 4095 kalibrointiyhtälöiden avulla.
  voltsBits = (volts / 24.875) * 4095;
  ampsBits = (amps / 2.550) * 4095 + 57 - (volts / 24.000 * 29.500);


  if (tdpLimit || shortCircuit) digitalWrite(LED_0_PIN, HIGH);
  else digitalWrite(LED_0_PIN, LOW);

  //Nämä funktiot toimivat nimiensä mukaisesti.
  doReadings();
  checkLimits();
  DACprint();
  updateDisplay();
}





ISR (PCINT0_vect) { //PCI D13-8

  //Päätellään mitä rotary encodereissa tapahtuu

  button_0_state = digitalRead(ROTARY_0_SWPIN);
  dPin_0_state = digitalRead(ROTARY_0_DPIN);
  clkPin_0_state = digitalRead(ROTARY_0_CLK);

  button_1_state = digitalRead(ROTARY_1_SWPIN);
  dPin_1_state = digitalRead(ROTARY_1_DPIN);
  clkPin_1_state = digitalRead(ROTARY_1_CLK);

  isrTime = millis();

  //Rotary0 painokytkimen tila
  if (button_0_state == 0) {
    if (isrTime - lastdebounceTime_0 > 200) {
      //Komento joka tehdään, kun nappulaa painetaan
      button_0_pressed = true;
      lastdebounceTime_0 = isrTime;
    }
  }

  //Rotary1 painokytkimen tila
  if (button_1_state == 0) {
    if (isrTime - lastdebounceTime_1 > 200) {
      //Komento joka tehdään, kun nappulaa painetaan
      button_1_pressed = true;
      lastdebounceTime_1 = isrTime;
    }
  }

  //Rotary0 pyörimissuunnan selvitys
  if (dPin_0_state != last_dPin_0_state) {
    if (dPin_0_state == 0 && clkPin_0_state == 1) {
      count0--;
    }
    if (dPin_0_state == 1 && clkPin_0_state == 1) {
      count0++;
    }
  }

  //Rotary1 pyörimissuunnan selvitys
  if (dPin_1_state != last_dPin_1_state) {
    if (dPin_1_state == 0 && clkPin_1_state == 1) {
      count1--;
    }
    if (dPin_1_state == 1 && clkPin_1_state == 1) {
      count1++;
    }
  }


  lastbutton_0_state = button_0_state;
  last_dPin_0_state = dPin_0_state;
  last_clkPin_0_state = clkPin_0_state;

  lastbutton_1_state = button_1_state;
  last_dPin_1_state = dPin_1_state;
  last_clkPin_1_state = clkPin_1_state;
}
