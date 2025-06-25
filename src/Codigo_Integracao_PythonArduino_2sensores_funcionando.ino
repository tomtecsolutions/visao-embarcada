const int trigFim = 11;
const int echoFim = 12;
const int trigCamera = 2;
const int echoCamera = 3;

const int motorIN1 = 8;
const int motorIN2 = 9;
const int motorENA = 10;

const int botaoInicio = 7;
const int botaoDesligar = 4;
const int botaoFimCurso = 6;  // agora o pino do fim de curso

const int distanciaLimite = 10;

bool esteiraAtiva = false;
bool sensorCameraAtivo = true;
bool aguardandoRetornoBraco = false;

String comando = "";
String comandoRecebido = "";
unsigned long tempoUltimoPrint = 0;
const unsigned long intervaloPrint = 5000;

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH);
  return duracao * 0.034 / 2;
}

void setup() {
  pinMode(trigCamera, OUTPUT);
  pinMode(echoCamera, INPUT);
  pinMode(trigFim, OUTPUT);
  pinMode(echoFim, INPUT);

  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  pinMode(motorENA, OUTPUT);

  pinMode(botaoInicio, INPUT_PULLUP);
  pinMode(botaoDesligar, INPUT_PULLUP);
  pinMode(botaoFimCurso, INPUT_PULLUP);  // novo botão de fim de curso

  Serial.begin(9600);
  Serial.println("🟡 Aguardando botão de início...");

  while (digitalRead(botaoInicio) == LOW);
  while (digitalRead(botaoInicio) == HIGH);

  Serial.println("🟢 Botão de início pressionado!");
  ativarEsteira();
  esteiraAtiva = true;
}

void loop() {
  // RESET AUTOMÁTICO VIA FIM DE CURSO
  if (digitalRead(botaoFimCurso) == LOW) {
    delay(50);
    if (digitalRead(botaoFimCurso) == LOW) {
      Serial.println("🔄 Reset automático via botão de fim de curso!");

      pararEsteira();
      esteiraAtiva = false;
      sensorCameraAtivo = true;
      aguardandoRetornoBraco = false;
      comandoRecebido = "";

      delay(400); // pequena pausa de segurança

      Serial.println("✅ Reset completo. Religa automaticamente a esteira.");
      ativarEsteira();
      esteiraAtiva = true;
    }
  }

  // DESLIGAR E RELIGAR MANUALMENTE (botão físico)
  if (digitalRead(botaoDesligar) == LOW && esteiraAtiva) {
    pararEsteira();
    esteiraAtiva = false;
    sensorCameraAtivo = false;
    comandoRecebido = "";
    Serial.println("🔴 Sistema desligado.");
    Serial.println("🟡 Aguardando botão de religar...");
    while (digitalRead(botaoInicio) == HIGH);
    Serial.println("🟢 Botão de religar pressionado!");
    sensorCameraAtivo = true;
    comandoRecebido = "";
    ativarEsteira();
    esteiraAtiva = true;
  }

  long distanciaCamera = medirDistancia(trigCamera, echoCamera);
  long distanciaFim = medirDistancia(trigFim, echoFim);

  if (millis() - tempoUltimoPrint >= intervaloPrint) {
    Serial.print("📡 Sensor Câmera: ");
    Serial.print(distanciaCamera);
    Serial.print(" cm | 📦 Fim Esteira: ");
    Serial.print(distanciaFim);
    Serial.println(" cm");
    tempoUltimoPrint = millis();
  }

  if (sensorCameraAtivo && distanciaCamera < distanciaLimite && esteiraAtiva) {
    Serial.print("📸 Objeto detectado na CÂMERA! Distância: ");
    Serial.print(distanciaCamera);
    Serial.println(" cm");

    pararEsteira();
    esteiraAtiva = false;
    sensorCameraAtivo = false;
    Serial.println("⏳ Pausa de 2 segundos para leitura da câmera...");
    delay(3000);
    ativarEsteira();
    esteiraAtiva = true;
  }

  if (Serial.available() > 0) {
    char recebido = Serial.read();
    comando += recebido;
    if (comando.endsWith("\n")) {
      comando.trim();
      Serial.print("📥 Comando recebido: ");
      Serial.println(comando);

      if (aguardandoRetornoBraco && comando.equalsIgnoreCase("retornou")) {
        Serial.println("✅ Braço retornou. Reiniciando leitura...");
        sensorCameraAtivo = true;
        ativarEsteira();
        esteiraAtiva = true;
        aguardandoRetornoBraco = false;
      } else {
        comandoRecebido = comando;
      }

      comando = "";
    }
  }

  if (distanciaFim < distanciaLimite && esteiraAtiva) {
    Serial.print("📦 Objeto detectado no FINAL da esteira! Distância: ");
    Serial.print(distanciaFim);
    Serial.println(" cm");

    pararEsteira();
    esteiraAtiva = false;

    Serial.println("⏳ Aguardando retorno do braço robótico...");
    aguardandoRetornoBraco = true;
  }

  delay(100);
}

void ativarEsteira() {
  digitalWrite(motorIN1, HIGH);
  digitalWrite(motorIN2, LOW);
  analogWrite(motorENA, 110);
  Serial.println("🟢 Esteira ativada.");
}

void pararEsteira() {
  digitalWrite(motorIN1, LOW);
  digitalWrite(motorIN2, LOW);
  analogWrite(motorENA, 0);
  Serial.println("🛑 Esteira parada.");
}
