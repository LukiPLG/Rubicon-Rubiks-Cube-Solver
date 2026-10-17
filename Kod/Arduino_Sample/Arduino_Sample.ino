#define PUL 2
#define DIR 8

#define STEPS 200          // zgodnie z ustawieniem DM556 (impulsów na obrót)
#define PULSE_US 5         // szerokość impulsu
#define START_DELAY 1500   // µs przerwy między krokami na starcie
#define MIN_DELAY 500      // µs przerwy przy pełnej prędkości (dostrój)
#define RAMP_STEPS 15      // ile kroków trwa rozpędzanie / hamowanie

void setup() {
  randomSeed(analogRead(A0));

  digitalWrite(PUL, HIGH);
  digitalWrite(DIR, HIGH);
  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);

  delay(500);
}

void loop() {
  motor(random(1, 4));
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