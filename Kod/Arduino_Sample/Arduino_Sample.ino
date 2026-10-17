#define PUL 2
#define DIR 8

#define STEPS 400          // zgodnie z ustawieniem DM556 (impulsów na obrót)
#define PULSE_US 5         // szerokość impulsu
#define START_DELAY 500   // µs przerwy między krokami na starcie
#define MIN_DELAY 350      // µs przerwy przy pełnej prędkości (dostrój)
#define RAMP_STEPS 15      // ile kroków trwa rozpędzanie / hamowanie
// 400 5 1500 350 15 - stabilne początkowe wartości
void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(A0));

  digitalWrite(PUL, HIGH);
  digitalWrite(DIR, HIGH);
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);

  delay(2500);
}

void loop() {
  unsigned long StartTime = millis();
  motor(random(1, 4));
  unsigned long EndTime = millis();
  unsigned long ElapsedTime = EndTime - StartTime;
  Serial.println(ElapsedTime);

  delay(2500);
}

void motor(int rotation) {
  int steps = STEPS / 4;

  if (rotation == 1) {
    digitalWrite(DIR, LOW);
  } else if (rotation == 3) {
    digitalWrite(DIR, HIGH);
  } else {
    steps = STEPS / 2;
    digitalWrite(DIR, LOW);
  }
  delayMicroseconds(10);

  for (int i = 0; i < steps; i++) {
    int ramp = min(min(i, steps - 1 - i), RAMP_STEPS);
    unsigned int d = START_DELAY - (long)(START_DELAY - MIN_DELAY) * ramp / RAMP_STEPS;

    digitalWrite(PUL, LOW);
    delayMicroseconds(PULSE_US);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(d);
  }
}