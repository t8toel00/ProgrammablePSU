float heatLimitedAmps = 0.000;
float heatLimitFactor = 1.000;
float shortCircuitFactor = 1.000;

void checkLimits(void) {

  //Suojataan laite alhaiselta sisääntulojännitteeltä
  if (inVolts < 14.500) {
    volts = 0;
    amps = 0;
    DACprint();

    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("ERROR!");
    lcd.setCursor(3, 1);
    lcd.print("INPUT VOLTAGE");
    lcd.setCursor(3, 2);
    lcd.print("BELOW ACCEPTED");
    lcd.setCursor(7, 3);
    lcd.print("RANGE!");

    while (1) {} //Virheellisen sisääntulojännitteen kanssa ei tehdä mitään.
  }

  //Suojataan laite ylisuurelta sisääntulojännitteeltä
  if (inVolts > 25.000) {
    volts = 0;
    amps = 0;
    DACprint();

    lcd.clear();
    lcd.setCursor(7, 0);
    lcd.print("ERROR!");
    lcd.setCursor(3, 1);
    lcd.print("INPUT VOLTAGE");
    lcd.setCursor(1, 2);
    lcd.print("EXCEEDED ACCEPTED");
    lcd.setCursor(7, 3);
    lcd.print("RANGE!");

    while (1) {} //Virheellisen sisääntulojännitteen kanssa ei tehdä mitään.
  }

  //Oikosulkusuoja estää lähtövirran kasvattamisen, jos lähdössä ei havaita minkäänlaista jännitettä
  if (volts > 0.100 && amps >= 0.100 && readVolts < 0.020) {
    shortCircuit = true;
    amps = 0.100;
    ampsUpdated = true;
    ampsBits = ((amps / 2.550) * 4095 + 57 - (volts / 24.000 * 29.500)) * shortCircuitFactor;
    if (readAmps < 0.100) heat = (inVolts - readVolts) * amps;

    if (readAmps > amps) { //Jos oikosulun aikana virta karkaa isommaksi kuin on tarkoitus, niin pakotetaan se pienemmäksi.
      shortCircuitFactor = shortCircuitFactor - 0.001;
    }
  } else {
    shortCircuit = false;
    shortCircuitFactor = 1.000;
  }


  /*
    //Tdp-limit 1. vaihe. Laskennallinen ensimmäinen tdp-limit: asetetaan virran maksimiarvo pienimmällä toteutuvalla jännitehäviöllä.
    if ((inVolts - volts) * amps > heatLimit) {
      //amps = heatLimit / (inVolts - volts);
      ampsBits = ((heatLimit / (inVolts - volts)) / 2.550) * 4095 + 57 - (volts / 24.000 * 29.500);
    }*/


  //Tdp-limit 2. vaihe. Mitattu todellinen säätyvä tdp-limit: lasketaan virtaa koska edellinen ei kuitenkaan riitä.
  if (heat > heatLimit) tdpLimit = true;
  else tdpLimit = false;

  heatLimitFactor = heatLimit / (0.010 + heat);
  heatLimitedAmps = (0.010 + readAmps) * heatLimitFactor;

  if (heatLimitedAmps < amps) {
    ampsBits = (heatLimitedAmps / 2.550) * 4095 + 57 - (volts / 24.000 * 29.500);
  }


}
