#include "sequencer.h"

#define STEPS_COUNT 5
#define TABS 1

unsigned long timer;
unsigned long current_step_time;

// vars
int tempo = 750;
int current_bpm = 80;

int steps[TABS][STEPS_COUNT];
int current_tab = 0;

int to_bpm(int ms) {
  return 60000 / ms;
}

void print_tabs() {
  for (int tab = 0; tab < TABS; tab++) {
    Serial.print("tab: ");
    Serial.println(tab);
    for (int i = 0; i < STEPS_COUNT; i++) {
      Serial.print(steps[tab][i]);
      Serial.print(",");
    }
    Serial.println();
  }
}

void read_tab_cv(int tab) {
  for (int i = 0; i < STEPS_COUNT; i++) {
    steps[tab][i] = analogRead(i);
  }
  Serial.println("tab saved");
}

void setup() {
  Serial.begin(31250);
  
  pinMode(TEMPO_BUTTON, INPUT);
  digitalWrite(TEMPO_BUTTON, HIGH);

  pinMode(TAB_2, INPUT);

  pinMode(SAVE_TAB, INPUT);
  digitalWrite(SAVE_TAB, HIGH);

  read_tab_cv(0);
  read_tab_cv(1);
}

void step_led(int i) {
  if (i == 0) {
    digitalWrite(STEPS_COUNT + LIGHT_STARTS - 1, LOW);
  } else {
    digitalWrite(i + LIGHT_STARTS - 1, LOW); 
  }
  digitalWrite(i + LIGHT_STARTS, HIGH);
}

void midiMessage(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void check_tab() {
  if (digitalRead(TAB_2) == HIGH) {
    current_tab = 1;
    digitalWrite(TAB_2_LED, HIGH);
  } else {
    current_tab = 0;
    digitalWrite(TAB_2_LED, LOW);
  }
}

void check_tempo(int tab, int i) {
  if (digitalRead(TEMPO_BUTTON) == LOW) {
    if (i == TEMPO_KNOB) {
      analogWrite(CV_OUT, steps[tab][i]/4);
    }
    tempo = analogRead(TEMPO_KNOB) * 2;
    current_bpm = to_bpm(tempo);
  }
}

void output(int tab, int i, int *last, int *val) {
  if (tab == current_tab) {
    *val = analogRead(i);
    digitalWrite(GATE_OUT, HIGH);
    write_cv(last, val);
  } else {
    *val = steps[tab][i];
    digitalWrite(GATE_OUT, HIGH);
    write_cv(last, val);
  }
}

void write_cv(int *val, int *last) {
  if (*val != *last) {
    *last = *val;
    analogWrite(CV_OUT, (*val)/4);
  }
}

void loop() {
  int last = 0;
  int val = 0;
  for (int tab = 0; tab < TABS; tab++) {
    for (int i = 0; i < STEPS_COUNT; i++) {
      current_step_time = timer;
      step_led(i);
      check_tab();

      if (digitalRead(SAVE_TAB) == LOW)
        read_tab_cv(current_tab);
    
      for(;;) {
        timer = millis();
        if (timer - current_step_time >= tempo)
          break;

        check_tempo(tab, i);
        output(tab, i, &last, &val);        
      }
    }
  }
}
