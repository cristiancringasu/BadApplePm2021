#pragma once
#include "Arduino.h"
#include <SD.h>
#include <SPI.h>

typedef struct {
    int** frame_matrix;
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
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (1 - (row / 8))); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, (1 << row % 8) * (row / 8)); // row 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~columns[1]); // column 8-15
    shiftOut(data_pin, clock_pin, MSBFIRST, ~columns[0]); // column 0-7
    digitalWrite(latch_pin, HIGH);
}

void setLeds_C(int* rows, int column) {
    register int rlow = 0;
    register int rhigh = 0;
    
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, rows[0]); // row 0-7
    shiftOut(data_pin, clock_pin, MSBFIRST, rows[1]); // row 8-15
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

void clear_frame();

void standby_animation() {
    iteration_row(100,1);
    iteration_row(100,-1);
    iteration_column(100,1);
    iteration_column(100,-1);
    clear_frame();
}

int read_file_int(File myFile) {
    register int c;
    register int current = 0;
    register bool numberRead = false;
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
    vfile->frame_matrix = (int**)malloc(16 * sizeof(int*));
    for (register int i = 0; i < 16; i ++) vfile->frame_matrix[i] = (int*)malloc(2 * sizeof(int));
    return vfile;
}

void play_frame(int** frame, int fps, int computeT) {
    //Serial.println("START Frame");
    long startT = millis();
    long frameT = 1000 / fps - computeT;
    if (frameT < 0) return;
    register int i;
    //Serial.println("END Frame");
    while(millis() - startT < frameT) {
        // Serial.print("Iteration Frame wait: ");
        // Serial.print(millis());
        // Serial.print(" ");
        // Serial.print(startT);
        // Serial.print(" ");
        // Serial.print(frameT);
        // Serial.print(" ");
        // Serial.println(millis() - startT);
        for (i = 0; i < 16; i ++) {
            setLeds_R(i, frame[i]);
        }
    }
    //Serial.println("END(1) Frame");
}

void read_frame(VideoFile* vfile) {
    register int** pi = vfile->frame_matrix;
    register int* pj;
    File file = vfile->file;
    for (register int i = 0; i < 16 && file.available(); i++, pi++) {
        pj = *pi;
        for (register int j = 0; j < 2 && file.available(); j++, pj++) {
            *pj = file.read();
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
    long startTG = millis();
    long frameT = 1000 / fps;
    int computeT;
    for (int k = 0; k < frames; k++) {
        //Serial.print("NEW: ");
        //startT = millis();
        clear_frame();
        read_frame(vfile);
        computeT = millis() - startTG - k * frameT;

        // Serial.print(vfile->frames);
        // Serial.print(" ");
        // Serial.print(vfile->fps);
        // Serial.print(" ");
        // Serial.print(computeT);
        // Serial.print(" ");
        // Serial.print(1000 / fps - computeT);
        // Serial.print(" ");
        // Serial.println(vfile->current_frame);

        play_frame(vfile->frame_matrix, fps, computeT);
        //vfile->current_frame++;
    }
    clear_frame();
    vfile->current_frame = frames;
    //Serial.println("Done");
    vfile->file.close();
}