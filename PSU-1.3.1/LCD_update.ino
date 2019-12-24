/*Nämä muuttujat löytyy globaaleista muuttujista. Lista vain tässä että löytyy nopeasti.
  boolean voltsUpdated = false;
  boolean ampsUpdated = false;
  boolean readVoltsUpdated = false;
  boolean readAmpsUpdated = false;
  boolean tempUpdated = false;
  boolean inVoltsUpdated = false;
  boolean powerUpdated = false;
  boolean heatUpdated = false;
*/
unsigned long lastDisplayUpdate = 0;

void updateDisplay(void) {
  if (voltsUpdated) {
    lcd.setCursor(0, 0);
    lcd.print("          ");
    lcd.setCursor(0, 0);
    lcd.print("U ");
    lcd.print(volts);
    voltsUpdated = false;
  }



  if (ampsUpdated) {
    lcd.setCursor(0, 1);
    lcd.print("          ");
    lcd.setCursor(0, 1);
    lcd.print("I ");
    lcd.print(amps);
    ampsUpdated = false;
  }


  if (loopTime - lastDisplayUpdate >= 500) {
    lastDisplayUpdate = loopTime;

    lcd.setCursor(10, 0);
    lcd.print("          ");
    lcd.setCursor(10, 0);
    lcd.print("Uout ");
    lcd.print(readVolts);

    lcd.setCursor(10, 1);
    lcd.print("          ");
    lcd.setCursor(10, 1);
    lcd.print("Iout ");
    lcd.print(readAmps);

    lcd.setCursor(0, 2);
    lcd.print("          ");
    lcd.setCursor(0, 2);
    lcd.print("T ");
    lcd.print(temp);

    lcd.setCursor(0, 3);
    lcd.print("          ");
    lcd.setCursor(0, 3);
    lcd.print("Uin ");
    lcd.print(inVolts);

    lcd.setCursor(10, 2);
    lcd.print("          ");
    lcd.setCursor(10, 2);
    lcd.print("P    ");
    lcd.print(power);

    lcd.setCursor(10, 3);
    lcd.print("          ");
    lcd.setCursor(10, 3);
    lcd.print("Heat ");
    lcd.print(heat);
  }

}
