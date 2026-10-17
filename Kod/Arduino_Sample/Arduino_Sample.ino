

// ******************************************************************
//                  NIE EDYTUJ TEGO PLIKU
// ******************************************************************



// Wszystkie piny masz opisane na schemacie
#define PUL 2
#define DIR 8

// Informacje o czasach są na stronie 7 w dokumentacji sterownika DM556
// Stabilne wartości początkowe to: 400 5 1500 400 15
#define STEPS 400          // Ilość kroków potrzebnych do wykonania pełnego obrotu; wartość ustawiana fizycznie na sterowniku
#define PULSE_US 5         // Szerokość impulsu - min. 2.5µs
#define START_DELAY 500    // µs przerwy między krokami podczas startu
#define MIN_DELAY 350      // µs przerwy przy pełnej prędkości
#define RAMP_STEPS 15      // Ile kroków trwa rozpędzanie / hamowanie

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
  Serial.println(ElapsedTime);    // Dotychczasowy rekord to 38ms dla obrotu o 90 i -90 stopni oraz 74ms dla 180 stopni

  delay(2500);
}

void motor(int rotation) {
  int steps = STEPS / 4;          // Ilość kroków potrzebnych do wykonania obrotu o 90 stopni

  if (rotation == 1) {            // Obrót o 90 stopni w jedną stronę
    digitalWrite(DIR, LOW);
  } else if (rotation == 3) {     // Obrót o 90 stopni w drugą stronę
    digitalWrite(DIR, HIGH);
  } else {                        // Obrót o 180 stopni w pierwszą stronę (tak wiem zajebisty opis XDD)
    steps = STEPS / 2;            // 180 stopni to 2x 90 stopni dlatego dzielimy przez 2 
    digitalWrite(DIR, LOW);
  }
  delayMicroseconds(10);          // W teorii minimalny delay to 5µs - minimalny czas potrzebny na zmiane kierunku przez sygnałem pulse

  // Wykonywanie potrzebnych kroków dla danego obrotu wraz ze zmniejszaniem się rampa (silnik przyśpiesza w czasie). Te dwie linijki kodu z rampem to już wynalazek Claude'a ale działa fajnie więc git
  for (int i = 0; i < steps; i++) {
    int ramp = min(min(i, steps - 1 - i), RAMP_STEPS);
    unsigned int d = START_DELAY - (long)(START_DELAY - MIN_DELAY) * ramp / RAMP_STEPS;

    digitalWrite(PUL, LOW);
    delayMicroseconds(PULSE_US);
    digitalWrite(PUL, HIGH);
    delayMicroseconds(d);
  }
}