#include "MidiFileStream.h"
#include <SD.h>
#include <SPI.h>
#include "SerialStepperPlayerLight.h"       
  
const int stepsPerRevolution = 2038;

#define latch_pin 5
#define clock_pin 6
#define data_pin 7
#define pin_count 4

const int pinSelectSD = 4; // Sparkfun SD shield Chip Select pin.
const int pinLed = 3;     // the Arduino LED pin

float tempo;
float TPB;
float tdelay;
unsigned long nextEvent = 0;

const char fName[] = "badapple.mid"; // The SD card Midi file to read.

boolean errorOccurred;  // if true, there's a problem.
boolean isFinished;     // if true, we successfully finished reading the file.

MidiFileStream midiFile; // the current Midi file being read.
File file;               // the underlying SD card file.

void setup() {
  Serial.begin(115200);
  
  // If the setup succeeds, we'll update errorOccurred.
  errorOccurred = true;
  isFinished = false;

  // Setup steppers
  init_steppers(4, latch_pin, clock_pin, data_pin, pin_count);
  test_steppers();
  
  pinMode(pinSelectSD, OUTPUT);
  pinMode(pinLed, OUTPUT);
  
  // Setup the SD card
  
  Serial.println("Calling SD.begin()...");
  if (!SD.begin(pinSelectSD)) {
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  Card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  Card formatting.");
    return;
  }
  Serial.println("...succeeded.");
  
  // Open the file to read
  
  file = SD.open(fName, FILE_READ);
  if (!file) {
    Serial.print("Failed to open SD card file ");
    Serial.println(fName);
    Serial.println("Does that file exist?");
    return;
  }
  
  /*
   * Read the Midi header from that file
   * and prepare to read Midi events from that file.
   */
  
  if (!midiFile.begin(file)) {
    Serial.print("Failed to open Midi file: ");
    Serial.println(fName);
    Serial.print("Check: is it a Midi-format file?");
    
    // Clean up
    midiFile.end();
    file.close();
    
    return;
  }
  
  // Say what the Midi Header told us.
  Serial.print("Midi file format: ");
  Serial.println(midiFile.getFormat());
  Serial.print("Number of tracks: ");
  Serial.println(midiFile.getNumTracks());
  Serial.print("Ticks per beat: ");
  Serial.println(midiFile.getTicksPerBeat());
  TPB = midiFile.getTicksPerBeat();
  
  // Open the first track of the file.
  if (midiFile.openChunk() != CT_MTRK) {
    Serial.println("Failed to open the first track of the file.");
    
    // Clean up
    midiFile.end();
    file.close();
    
    return;
  }
  
  // remember that everything is ok so far.
  errorOccurred = false;

}

void loop() {
  // If we're finished reading, do nothing.
  if (isFinished) {
    return;
  }
  
  // If there is a problem, just blink the LED.
  if (errorOccurred) {
    if ((millis() % 1000) < 500) {
      digitalWrite(pinLed, HIGH);
    } else {
      digitalWrite(pinLed, LOW);
    }
    
    return;
  }
  
  // read the next Midi event from the file.
  event_t eventType = midiFile.readEvent();
  if (eventType == ET_END) {
    
    // The track has ended; open the next track.
    chunk_t chunkType = midiFile.openChunk();
    if (chunkType != CT_MTRK) {
      if (chunkType == CT_END) {
        Serial.println("File ended normally.");
        
        // Clean up
        midiFile.end();
        file.close();
        
        isFinished = true;
        return;
      }
      
      // File error: Chunk is not a track.
      Serial.println("Failed to open file track.");
    
      // Clean up
      midiFile.end();
      file.close();
    
      errorOccurred = true;
      return;
    }
    
    // Successfully opened the track
    // loop again to read the next event.
    
    return;
  }
  
  // Say something about this event
  
//  Serial.print("delay ");
//  Serial.print(midiFile.getEventDeltaTicks());
//  Serial.println(" ticks.");
  nextEvent = millis() + (tdelay * midiFile.getEventDeltaTicks());
  while (millis() < nextEvent) play_steppers_notes();
  //delay((tdelay * midiFile.getEventDeltaTicks()));
//  Serial.print("Next event is after: ");
//  Serial.print(tdelay * midiFile.getEventDeltaTicks());
//  Serial.println(" ms");
  
  if (eventType == ET_TEMPO) {
//    Serial.print("New Tempo: ");
//    Serial.print(midiFile.getEventDataP()->tempo.uSecPerBeat);
//    Serial.println(" microseconds per beat.");
    
    tempo = 60000000 / midiFile.getEventDataP()->tempo.uSecPerBeat;
//    Serial.print("Tempo is ");
//    Serial.print(tempo);
//    Serial.println(" BPM.");
    
      tdelay = 60000 / (tempo * TPB);
//    Serial.print("Tick delay is ");
//    Serial.println(tdelay);
    
    return;
  }
  
  if (eventType == ET_END_TRACK) {
    Serial.println("End of Track event.");
    
    return;
  }
  
  if (eventType == ET_CHANNEL) {
    char code = midiFile.getEventDataP()->channel.code;
    
    if (code == CH_NOTE_ON) {
      //Serial.print("Note On: Midi note number ");
      //Serial.println(midiFile.getEventDataP()->channel.param1);
      play_midi_note(midiFile.getEventDataP()->channel.param1, midiFile.getEventDataP()->channel.param2);
      
      
      return;
    }
    
    if (code == CH_NOTE_OFF) {
//      Serial.print("Note Off: Midi note number ");
//      Serial.println(midiFile.getEventDataP()->channel.param1);
      play_midi_note(midiFile.getEventDataP()->channel.param1, 0);
      return;
    }
    
    // Otherwise, it's a Channel event that we don't care about
    // Ignore it.
    
    return;
  }
  
  // Otherwise, it's an event we don't care about.
  // Ignore it.
  
  // return;

}
