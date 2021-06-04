#include "led_player_fast.h"
#include <SD.h>
#include <SPI.h>
#include <string.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define latch_pin 22
#define data_pin 26
#define clock_pin 24

#define x_key A9
#define y_key A8
#define select_pin 30

uint16_t data = 0;

const int pinSelectSD = 53; // Sparkfun SD shield Chip Select pin.
const int pinLed = 28;     // the Arduino LED pin
boolean videoInit = false;
const char fName[] = "badapple.vid"; // The SD card Midi file to read.
int fCount;

File root;
File entry;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void testing() {
  iteration_row(dtime, 0);
  iteration_column(dtime, 0);
  iteration_row(dtime, 1);
  iteration_column(dtime, 1);
  full_matrix(500);
}

void play_video_file() {
  // Open the file to read
  if (!strstr(entry.name(), ".VID")) {
    lcd.clear();
    lcd.print("File isnt playable!");
    flick_led(1000);
    return;
  }
  lcd.setCursor(3, 1);
  lcd.print("NOW PLAYING!");
  
  entry = SD.open(entry.name());
  Serial.write(entry.name());
  VideoFile* vfile = init_vfile(entry);
  //Serial.println("Video read...");
  play_video(vfile);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print(entry.name());
}

void select_file() {
  delay(500);
  play_video_file();
}

void next_file() {
  entry = root.openNextFile();
  if (!entry) {
    root.close();
    root = SD.open("/");
  }
  while(entry && entry.isDirectory()) {entry.close(); entry = root.openNextFile();}

  if (!entry) {
    root.close();
    root = SD.open("/");
  }
  
}

void file_inspect() {
  next_file();
  while(entry) {
    fCount++;
    next_file();
  }
}

int shifted;
void shift_file(int dir) {
  if (dir == 1) {
    next_file();
    shifted++;
    shifted %= fCount;
    if (!shifted) shifted++;
  } else if (dir == -1) {
    shifted -= 1;
    if (shifted <= 0) shifted = fCount;
    root.close();
    root = SD.open("/");
    entry = root;
    int shifted_temp = shifted;
    while(shifted_temp--) next_file();
  }
  if (!entry) {
    lcd.clear();
    lcd.print("No files on the card");
    return;
  }
  lcd.clear();
  lcd.print(entry.name());
  entry.close();
  delay(500);
}

void check_analog_input() {
  int x_pos, y_pos;
  int x_map, y_map;
  x_pos = analogRead(x_key);
  y_pos = analogRead(y_key);
  x_map = map(x_pos, 0,1023, 0, 7);
  y_map = map(y_pos,0,1023,7,0);
  if (x_map <= 1)
    shift_file(-1);
  if(x_map >= 5)
    shift_file(1);
  if (digitalRead(select_pin) == LOW)
    select_file();
}

void flick_led(long timeT) {
  long startT = millis();
  while(millis() - startT < timeT) {
  digitalWrite(pinLed, HIGH);
  delay(100);
  digitalWrite(pinLed, LOW);
  delay(100);
  }
  digitalWrite(pinLed, HIGH);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(latch_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  init_matrix(data_pin, clock_pin, latch_pin);
  
  lcd.begin(16,2); 
  lcd.clear();

  pinMode (x_key, INPUT) ;                     
  pinMode (y_key, INPUT) ;
  pinMode (select_pin, INPUT_PULLUP);

  pinMode(pinSelectSD, OUTPUT);
  pinMode(pinLed, OUTPUT);

  //Serial.println("Calling SD.begin()...");
  if (!SD.begin(pinSelectSD)) {
    //Serial.println("SD.begin() failed. Check: ");
    //Serial.println("  Card insertion,");
    //Serial.println("  SD shield I/O pins and chip select,");
    //Serial.println("  Card formatting.");
    flick_led(10000);
    return;
  }
  digitalWrite(pinLed, HIGH);

//  entry = SD.open("badapple.vid");
//  VideoFile* vfile = init_vfile(entry);
//  Serial.println("Video read...");
//  play_video(vfile);
  
  root = SD.open("/");
  entry = root;
  file_inspect();
  //Serial.println("...succeeded.");
  
  lcd.print("Program Started!");
}

void loop() {
  // put your main code here, to run repeatedly
  check_analog_input();
}
