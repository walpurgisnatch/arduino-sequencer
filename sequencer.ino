#define STEPS_COUNT 2

#define CV_OUT 9
#define GATE_OUT 10
#define TEMPO_BUTTON 4
#define TEMPO_KNOB 0

unsigned long timer;
unsigned long current_step_time;

int tempo = 750;
int current_bpm = 80;

int to_bpm(int ms) {
  return 60000 / ms;
} 

void setup() {
  Serial.begin(9600);
  pinMode(TEMPO_BUTTON, INPUT);
  digitalWrite(TEMPO_BUTTON, HIGH);
}

void loop() {
  int val = 0;
  byte read_tempo = 0;
  for (int i = 0; i < STEPS_COUNT; i++) {
    current_step_time = timer;
    
    for(;;) {
      timer = millis();
      if (timer - current_step_time >= tempo)
        break;
      
      read_tempo = digitalRead(TEMPO_BUTTON);
      if (read_tempo == LOW) {
        tempo = analogRead(TEMPO_KNOB) * 4;
        current_bpm = to_bpm(tempo);
      } else {
        val = analogRead(i);
        analogWrite(CV_OUT, val/4);
      }
    }
    Serial.print("tempo: ");
    Serial.println(tempo);
    Serial.print("value: ");
    Serial.println(val);
  }
}
