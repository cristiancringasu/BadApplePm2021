#pragma once
#include "Arduino.h"
#include <SD.h>
#include <SPI.h>

typedef struct {
    int frame_matrix[16][16];
    int frames;
    int current_frame;
    int fps;
    File file;
} VideoFile;

VideoFile* global_vid;

int fullLine[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int fullColumn[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
int data_pin, clock_pin, latch_pin;
void init_matrix(int d, int c, int l) {
    data_pin = d;
    clock_pin = c;
    latch_pin = l;
}

void check_regs() {
    for(int i = 0; i < 8; i ++) {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1 << i);
    digitalWrite(latch_pin, HIGH);
    //delay(10);
    }
}

void setLeds_R(int row, int* columns) {
    int clow = 255;
    int chigh = 255;
    
    for (int j = 0; j < 8; j++) clow  &= ~((1<<j) * columns[j]);
    for (int j = 0; j < 8; j++) chigh &= ~((1<<j) * columns[j + 8]);
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (1 - (row / 8))); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (row / 8)); // row 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, chigh); // column 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, clow); // column 0-7
    digitalWrite(latch_pin, HIGH);
}

void setLeds_C(int* rows, int column) {
    int rlow = 0;
    int rhigh = 0;
    
    for (int i = 0; i < 8; i++) rlow  |= (1<<i) * rows[i];
    for (int i = 0; i < 8; i++) rhigh |= (1<<i) * rows[i + 8];
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, rlow); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, rhigh); // row 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~((1 << column % 8) * (column / 8))); // column 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~((1 << column % 8) * (1 - (column / 8)))); // column 0-8
    digitalWrite(latch_pin, HIGH);
}

void setLed(int row, int column) {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (1 - (row / 8))); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (row / 8)); // row 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~((1 << column % 8) * (column / 8))); // column 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~((1 << column % 8) * (1 - (column / 8)))); // column 0-8
    digitalWrite(latch_pin, HIGH);
}

void testLine() {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, 0);
    shiftOut(data_pin, clock_pin, MSBFIRST, 255);
    shiftOut(data_pin, clock_pin, MSBFIRST, 0);
    shiftOut(data_pin, clock_pin, MSBFIRST, 1);
    digitalWrite(latch_pin, HIGH);
}

const int dtime = 100;

void iteration_row(const int dtime, int dir) {
  for (int i = 0 + dir * 15; i < 16 && i >= 0; i += 1 - 2 * dir) {
    setLeds_R(i, fullLine);
    delay(dtime);
  }
}

void iteration_column(const int dtime, int dir) {
  for (int j = 0 + dir * 15; j < 16 && j >= 0; j += 1 - 2 * dir) {
    setLeds_C(fullColumn, j);
    delay(dtime);
  }
}

void full_matrix(long timeT) {
  long startT = millis();
  while(millis() - startT < timeT) {
    iteration_row(0, 0);
  }
}

int read_file_int(File myFile) {
    int c;
    int current = 0;
    bool numberRead = false;
    while (myFile.available()) {
        c = myFile.read();
        if ('0' <= c && c <= '9') {
            current = current * 10 + (c - '0'), numberRead = true;
        } else {
            if (numberRead)
                return current;
            else
                continue;
        }
    }
    return 0;
}

VideoFile* init_vfile(File myFile) {
    VideoFile* vfile = (VideoFile*) malloc(sizeof(VideoFile));
    vfile->frames = read_file_int(myFile);
    vfile->current_frame = 0;
    vfile->fps = read_file_int(myFile);
    vfile->file = myFile;
    return vfile;
}

void play_frame(int frame[16][16], int fps, int computeT) {
    long startT = millis();
    long frameT = 1000 / fps - computeT;
    while(millis() - startT < frameT)
    for (int i = 0; i < 16; i ++) {
        setLeds_R(i, frame[i]);
    }
}

void read_frame(VideoFile* vfile) {
    for (int i = 0; i < 16 && vfile->file.available(); i++) {
        for (int j = 0; j < 16 && vfile->file.available(); j++) {
            vfile->frame_matrix[i][j] = read_file_int(vfile->file);
            //Serial.print(vfile->frame_matrix[k][i][j]);
            //Serial.print(" ");
        }
        //Serial.println();
    }
    //Serial.println();
}

void clear_frame() {
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, 0); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, 0); // row 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, 255); // column 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, 255); // column 0-8
    digitalWrite(latch_pin, HIGH);
}

void play_video(VideoFile* vfile) {
    int fps = vfile->fps;
    int frames = vfile->frames;
    long startT;
    int computeT;
    for (int k = 0; k < frames; k++) {
        startT = millis();
        clear_frame();
        read_frame(vfile);
        computeT = millis() - startT;
        play_frame(vfile->frame_matrix, fps, computeT);
        vfile->current_frame++;
        //Serial.println(vfile->current_frame);
    }
    vfile->file.close();
}