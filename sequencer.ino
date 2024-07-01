#define STEPS_COUNT 5

// pins
#define CV_OUT 3
#define GATE_OUT 10
#define TEMPO_BUTTON 2
#define TEMPO_KNOB 0
#define LIGHT_STARTS 4

unsigned long timer;
unsigned long current_step_time;

// vars
int tempo = 750;
int current_bpm = 80;

int to_bpm(int ms) {
  return 60000 / ms;
} 

void setup() {
  pinMode(TEMPO_BUTTON, INPUT);
  digitalWrite(TEMPO_BUTTON, HIGH);
}

void loop() {
  int val = 0;
  byte read_tempo = 0;
  for (int i = 0; i < STEPS_COUNT; i++) {
    current_step_time = timer;

    if (i == 0) {
      digitalWrite(STEPS_COUNT + LIGHT_STARTS - 1, LOW);
    } else {
      digitalWrite(i + LIGHT_STARTS - 1, LOW); 
    }
    
    digitalWrite(i + LIGHT_STARTS, HIGH);
    
    for(;;) {
      timer = millis();
      if (timer - current_step_time >= tempo)
        break;
      
      read_tempo = digitalRead(TEMPO_BUTTON);
      if (read_tempo == LOW) {
        tempo = analogRead(TEMPO_KNOB) * 2;
        current_bpm = to_bpm(tempo);
      } else {
        val = analogRead(i);
        analogWrite(CV_OUT, val/4);
      }
    }
  }
}
