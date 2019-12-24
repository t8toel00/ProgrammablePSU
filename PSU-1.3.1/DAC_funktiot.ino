int increment = 1;
int level = 0;

void DACkolmio(void) {
  volts = 768; // 768 = 0011 0000 0000
  amps = 2050; //2050 = 1000 0000 0010


  level = level + increment;
  volts  = map(level, 0, 255, 0, 4095);
  amps  = map(level, 0, 255, 0, 4095);

  if (level >= 255 || level <= 0) {
    increment = -increment;
  }
}
