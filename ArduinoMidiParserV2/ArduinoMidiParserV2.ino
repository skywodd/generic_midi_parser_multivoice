/* Includes */
#include <SD.h>
#include "GenericMidiParser.h"

File fi;

/* Low-level functions */
uint8_t file_read_fnct(void) {
  int buf = fi.read();
  if(buf == -1)
    buf = 0;
  return buf;
}

void file_fseek_fnct(uint32_t address) {
  fi.seek(address);
}

uint32_t file_ftell_fnct(void) {
  fi.position();
}

uint8_t file_eof_fnct(void) {
  return (fi.available() == 0);
}

void us_delay_fnct(uint32_t us) {
  //Serial.print("Delay ");
  //Serial.println(us / 1000, DEC);
  delay(us / 1000);
}

void assert_error_callback(uint8_t errorCode) {
  //Serial.print("Erreur midi ");
  //Serial.println(errorCode, DEC);
}

GenericMidiParser midi(file_read_fnct, file_fseek_fnct, file_ftell_fnct, file_eof_fnct, us_delay_fnct, assert_error_callback);

/* Callback function */
void note_on_callback(uint8_t channel, uint8_t key, uint8_t velocity) {
  Serial.write(0x90 | channel);
  Serial.write(key);
  Serial.write(velocity);
  //Serial.println("Note on");
}

void note_off_callback(uint8_t channel, uint8_t key, uint8_t velocity) {
  Serial.write(0x80 | channel);
  Serial.write(key);
  Serial.write(velocity);
  //Serial.println("Note off");
}

void patch_change_callback(uint8_t channel, uint8_t instrument) {
  Serial.write(0xC0 | channel);
  Serial.write(instrument);
  //Serial.println("Program Change");
}

void setup()
{
  Serial.begin(31250);
  //Serial.begin(9600);

  pinMode(10, OUTPUT);
  if (!SD.begin(4)) {
    //Serial.println("SD error");
    return;
  }

  fi = SD.open("test.mid", FILE_READ);

  midi.setNoteOnCallback(note_on_callback);
  midi.setNoteOffCallback(note_off_callback);
  midi.setPatchChangeCallback(patch_change_callback);

  //Serial.println("Playing ...");
  midi.play();

  fi.close();
}

void loop()
{

}



