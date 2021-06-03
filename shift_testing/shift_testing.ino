#define latch_pin 8
#define data_pin 10
#define clock_pin 9
uint16_t data = 0;
void setup() {
  // put your setup code here, to run once:
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, 255);
  shiftOut(data_pin, clock_pin, MSBFIRST, ~(1 << 7));
  shiftOut(data_pin, clock_pin, MSBFIRST, 0);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1);
  digitalWrite(latch_pin, HIGH);
}

void setLed_old(int i, int j) {
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, 0);
  shiftOut(data_pin, clock_pin, MSBFIRST, 255 - (1<<((j%8) + 1)) * ((j + 1)/8));
  shiftOut(data_pin, clock_pin, MSBFIRST, 254 - (1<<((j%8) + 1)) * (1 - (j + 1)/8) + (1<<(i%8)) * (i/8));
  shiftOut(data_pin, clock_pin, MSBFIRST, (1<<(i%8)) * (1 - i/8));
  digitalWrite(latch_pin, HIGH);
  delay(500);
}

void check_pins_old() {
  for(int i = 0; i < 9; i ++) {
    for(int j = 0; j < 11; j ++) {
      setLed_old(i,j);
      delay(100);  
    }
  }
}

void check_regs() {
  for(int i = 0; i < 8; i ++) {
    digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
  shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
  digitalWrite(latch_pin, HIGH);
  delay(100);
  }
}

void loop() {
  // put your main code here, to run repeatedly
  //check_pins();
  //check_regs();
}
