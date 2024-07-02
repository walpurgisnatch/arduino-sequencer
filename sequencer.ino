#define STEPS_COUNT 5
#define TABS 2

// pins
#define CV_OUT 3
#define GATE_OUT 10
#define TEMPO_BUTTON 2
#define TEMPO_KNOB 0
#define LIGHT_STARTS 4
#define TAB_2 13
#define TAB_2_LED 11
#define SAVE_TAB 12

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

void setup() {
  Serial.begin(9600);
  
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

void read_tab_cv(int tab) {
  for (int i = 0; i < STEPS_COUNT; i++) {
    steps[tab][i] = analogRead(i);
  }
  Serial.println("tab saved");
}

void loop() {
  int last = 0;
  int val = 0;
  for (int tab = 0; tab < TABS; tab++) {
    Serial.print("tab: ");
    Serial.println(tab);
    for (int i = 0; i < STEPS_COUNT; i++) {
      current_step_time = timer;
      step_led(i);

      if (digitalRead(TAB_2) == HIGH) {
        current_tab = 1;
        digitalWrite(TAB_2_LED, HIGH);
      } else {
        current_tab = 0;
        digitalWrite(TAB_2_LED, LOW);
      }

      if (digitalRead(SAVE_TAB) == LOW)
        read_tab_cv(current_tab);
    
      for(;;) {
        timer = millis();
        if (timer - current_step_time >= tempo)
          break;

        // read tempo input
        if (digitalRead(TEMPO_BUTTON) == LOW) {
          if (i == TEMPO_KNOB) {
            analogWrite(CV_OUT, steps[tab][i]/4);
          }
          tempo = analogRead(TEMPO_KNOB) * 2;
          current_bpm = to_bpm(tempo);
        }

        // cv out
        if (tab == current_tab) {
          val = analogRead(i);
          if (val != last) {
            last = val;
            analogWrite(CV_OUT, val/4);
          }
        } else {
          val = steps[tab][i];
          if (val != last) {
            last = val;
            analogWrite(CV_OUT, val/4);
          }
        }
      }
      
      Serial.print("cv: ");
      Serial.println(val);
    }
  }
}
