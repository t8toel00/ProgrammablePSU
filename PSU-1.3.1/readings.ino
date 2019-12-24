void doReadings(void) {
  readVolts = (analogRead(READ_VOLTS_PIN) / 1023.000) * 24.726;
  readAmps = (analogRead(READ_AMPS_PIN) - 30) / 375.000;
  if (readAmps < 0) readAmps = 0;

  //  inVolts = analogRead(READ_IN_VOLTS_PIN);

  temp = (analogRead(READ_TEMP_PIN) - 275) / 9.0574;
  power = readVolts * readAmps;
  heat = (inVolts - readVolts) * readAmps;
}
