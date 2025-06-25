#include <VarSpeedServo.h>

// Servos
VarSpeedServo servo1; // BASE
VarSpeedServo servo2; // OMBRO
VarSpeedServo servo3; // COTOVELO
VarSpeedServo servo4; // GARRA

// Últimos valores usados
int lastBase = 0;
int lastOmbro = 0;
int lastCotovelo = 0;
int lastGarra = 0;

const int velocidade = 30;       // Velocidade do movimento (1 = rápido, 255 = lento)
const int tolerancia = 3;        // Tolerância mínima para mudança (suavização)

void setup() {
  Serial.begin(9600);

  servo1.attach(11); // BASE
  servo2.attach(10); // OMBRO
  servo3.attach(9);  // COTOVELO
  servo4.attach(6);  // GARRA
}

void loop() {
  // Leitura dos potenciômetros com mapeamento
  int valBase     = map(analogRead(A0), 0, 1023, 0, 179);
  int valOmbro    = map(analogRead(A1), 0, 1023, 0, 100);
  int valCotovelo = map(analogRead(A2), 0, 1023, 35, 140);
  int valGarra    = map(analogRead(A3), 0, 1023, 10, 100);

  // Atualiza somente se a diferença for maior que a tolerância
  if (abs(valBase - lastBase) > tolerancia) {
    servo1.write(valBase, velocidade);
    lastBase = valBase;
  }

  if (abs(valOmbro - lastOmbro) > tolerancia) {
    servo2.write(valOmbro, velocidade);
    lastOmbro = valOmbro;
  }

  if (abs(valCotovelo - lastCotovelo) > tolerancia) {
    servo3.write(valCotovelo, velocidade);
    lastCotovelo = valCotovelo;
  }

  if (abs(valGarra - lastGarra) > tolerancia) {
    servo4.write(valGarra, velocidade);
    lastGarra = valGarra;
  }

  // Mostra os valores no Serial Monitor
  Serial.print("Base: "); Serial.print(valBase);
  Serial.print(" | Ombro: "); Serial.print(valOmbro);
  Serial.print(" | Cotovelo: "); Serial.print(valCotovelo);
  Serial.print(" | Garra: "); Serial.println(valGarra);

  delay(100);  // Delay para estabilidade da leitura
}
