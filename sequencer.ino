#include "sequencer.h"

#define STEPS_COUNT 3

unsigned long timer;
unsigned long current_step_time;

int midi_channel = 1;
int midi_mode = 0;

// vars
int current_bpm = 80;
int current_octave = 60;

int steps[2][STEPS_COUNT];

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)

int to_tempo(int bpm) {
  return 60000 / bpm;
}

/* void print_tabs() { */
/*   for (int tab = 0; tab < TABS; tab++) { */
/*     Serial.print("tab: "); */
/*     Serial.println(tab); */
/*     for (int i = 0; i < STEPS_COUNT; i++) { */
/*       Serial.print(steps[tab][i]); */
/*       Serial.print(","); */
/*     } */
/*     Serial.println(); */
/*   } */
/* } */

void PinA() {
  volatile byte value = 0;
  cli();
  value = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(value == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (current_bpm > MIN_TEMPO)
      current_bpm -= 5;
    bFlag = 0;
    aFlag = 0;
  }
  else if (value == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei();
}

void PinB() {
  volatile byte value = 0;
  cli();
  value = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (value == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    if (current_bpm != 255)
      current_bpm += 5;
    bFlag = 0;
    aFlag = 0;
  }
  else if (value == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei();
}

void setup() {
  Serial.begin(31250);

  pinMode(TEMPO_KNOB_A, INPUT_PULLUP);
  pinMode(TEMPO_KNOB_B, INPUT_PULLUP);
  attachInterrupt(0, PinA, RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1, PinB, RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  
  pinMode(MIDI_MODE_SWITCH, INPUT);

  pinMode(SAVE, INPUT);
  digitalWrite(SAVE, HIGH);

  save_steps();
}

void check_midi_mode() {
  if (digitalRead(MIDI_MODE_SWITCH) == LOW)
    midi_mode = 1;
  else
    midi_mode = 0;
}

void save_steps() {
  for (int i = 0; i < STEPS_COUNT; i++) {
    steps[midi_mode][i] = analogRead(i);
  }
  Serial.println("tab saved");
}

void step_led(int i) {
  if (i == 0) {
    digitalWrite(STEPS_COUNT + LIGHT_STARTS - 1, LOW);
  } else {
    digitalWrite(i + LIGHT_STARTS - 1, LOW); 
  }
  digitalWrite(i + LIGHT_STARTS, HIGH);
}

void midi_message(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

int midi_note(int cv) {
  return cv/80;
}

void write_cv(int val) {
  analogWrite(CV_OUT, val/4);
}

void output(int val, int step) {
  if (midi_mode) {
    midi_message(noteOn + midi_channel-1, midi_note(val), 0xf);
    write_cv(steps[1][step]);
  } else {
    write_cv(val);
    midi_message(noteOn + midi_channel-1, midi_note(steps[0][step]), 0xf);
  }
  digitalWrite(GATE_OUT, HIGH);
}

void step_off(int val) {
  digitalWrite(GATE_OUT, LOW);
  midi_message(noteOff + midi_channel-1, midi_note(val), 0x00);
}

void loop() {
  int val = 0;
  for (int i = 0; i < STEPS_COUNT; i++) {
    val = analogRead(i);
    current_step_time = timer;
    check_midi_mode();
    step_led(i);

    if (digitalRead(SAVE) == LOW)
      save_steps();
    
    output(val, i);
    
    for(;;) {
      timer = millis();
      if (timer - current_step_time >= to_tempo(current_bpm)) {
        step_off(val);
        break;
      }
    }
  }
}
