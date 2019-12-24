void DACprint(void) {

  if (voltsBits > 4095) voltsBits = 4095;
  if (ampsBits > 4095) ampsBits = 4095;
  if (voltsBits < 0) voltsBits = 0;
  if (ampsBits < 0) ampsBits = 0;

  unsigned long voltsAmpsBits = 0;
  voltsAmpsBits = voltsAmpsBits | ampsBits;
  voltsAmpsBits = voltsAmpsBits << 12;
  voltsAmpsBits = voltsAmpsBits | voltsBits;

  unsigned long printSequence = voltsAmpsBits;
  unsigned long invertedSequence = 0;
  byte b = 0;


  //printSequence inversion
  for (int i = 0; i < 23; i++) {
    b = printSequence & 1;
    invertedSequence = invertedSequence | b;
    invertedSequence = invertedSequence << 1;
    printSequence = printSequence >> 1;
  }
  b = printSequence & 1;
  invertedSequence = invertedSequence | b;
  printSequence = invertedSequence;




  for (int i = 0; i < 24; i++) {

    byte a = printSequence & 1;

    if (a > 0) {
      digitalWrite(DATAPIN, HIGH);
      //      if (i % 12 == 0 && i > 0) Serial.print(" ");
      //      Serial.print("1");
    }

    else {
      digitalWrite(DATAPIN, LOW);
      //      if (i % 12 == 0 && i > 0) Serial.print(" ");
      //      Serial.print("0");
    }

    digitalWrite(SHCLK, HIGH);
    digitalWrite(SHCLK, LOW);

    printSequence = printSequence >> 1;
  }
  //  Serial.println();
  digitalWrite(STCLK, HIGH);
  digitalWrite(STCLK, LOW);
  /*
    Serial.print("Volts ");
    Serial.print(voltsBits);
    Serial.print("\t");
    Serial.print("Amps ");
    Serial.print(ampsBits);
    Serial.println("\t");
  */
}
