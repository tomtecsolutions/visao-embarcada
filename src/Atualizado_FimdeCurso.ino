#include <VarSpeedServo.h>

// Servos com nomes claros
VarSpeedServo base;
VarSpeedServo ombro;
VarSpeedServo cotovelo;
VarSpeedServo garra;

bool comandoRecebido = false;
int destinoCaixa = 0;
bool prontoParaExecutar = false;

void setup() {
  Serial.begin(9600);

  base.attach(11);
  ombro.attach(10);
  cotovelo.attach(9);
  garra.attach(6);

  base.write(0);
  ombro.write(0);
  cotovelo.write(140);
  garra.write(10);

  delay(500);
  Serial.println("[Braço] 🤖 Braço pronto para comandos.");
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando.equalsIgnoreCase("box1")) {
      destinoCaixa = 1;
      comandoRecebido = true;
      Serial.println("[Braço] 📦 Comando armazenado: box1");
    } else if (comando.equalsIgnoreCase("box2")) {
      destinoCaixa = 2;
      comandoRecebido = true;
      Serial.println("[Braço] 📦 Comando armazenado: box2");
    } else if (comando.equalsIgnoreCase("box3")) {
      destinoCaixa = 3;
      comandoRecebido = true;
      Serial.println("[Braço] 📦 Comando armazenado: box3");
    } else if (comando.equalsIgnoreCase("box4")) {
      destinoCaixa = 4;
      comandoRecebido = true;
      Serial.println("[Braço] 📦 Comando armazenado: box4");
    } else if (comando.equalsIgnoreCase("ready")) {
      if (comandoRecebido && destinoCaixa > 0) {
        Serial.println("[Braço] ✅ Comando 'ready' recebido! Executando...");
        prontoParaExecutar = true;
      } else {
        Serial.println("[Braço] ⚠️ Comando 'ready' recebido, mas nenhum destino definido.");
      }
    }
  }

  if (prontoParaExecutar) {
    executarMovimento(destinoCaixa);
    comandoRecebido = false;
    prontoParaExecutar = false;
    destinoCaixa = 0;
    Serial.println("[Braço] ✅ Braço pronto para próxima peça.");
  }
}

void executarMovimento(int destino) {
  garra.write(140, 30, true); delay(500);
  base.write(83, 30, true); delay(200);
  garra.write(80, 30, true); delay(500);
  ombro.write(0); cotovelo.write(140);
  cotovelo.write(45, 30, true); delay(200);
  ombro.write(48, 30, true); delay(200);
  cotovelo.write(65, 30, true); delay(200);
  garra.write(15, 30, true); delay(500); // fechar garra

  // Movimentação para a caixa
  if (destino == 1) {
    Serial.println("[Braço] 🦾 Levando peça para Box 1 (esquerda)...");
    cotovelo.write(120, 30, true); delay(500);
    ombro.write(50, 30, true); delay(500);
    base.write(24, 30, true); delay(200);
    ombro.write(45, 30, true); delay(200);
    cotovelo.write(39, 30, true); delay(200);
    garra.write(100, 30, true); delay(500);
  } else if (destino == 2) {
    Serial.println("[Braço] 🦾 Levando peça para Box 2 (centro)...");
    cotovelo.write(120, 30, true); delay(500);
    ombro.write(50, 30, true); delay(500);
    base.write(38, 30, true); delay(200);
    ombro.write(64, 30, true); delay(200);
    cotovelo.write(59, 30, true); delay(200);
    garra.write(100, 30, true); delay(500);
  } else if (destino == 3) {
    Serial.println("[Braço] 🦾 Levando peça para Box 3 (direita)...");
    cotovelo.write(120, 30, true); delay(500);
    ombro.write(50, 30, true); delay(500);
    base.write(47, 30, true); delay(200);
    ombro.write(100, 30, true); delay(200);
    cotovelo.write(106, 30, true); delay(200);
    garra.write(100, 30, true); delay(500);
  } else if (destino == 4) {
    Serial.println("[Braço] 🦾 Levando peça para Box 4 (produto não reconhecido/indefinido)...");
    cotovelo.write(120, 30, true); delay(500);
    ombro.write(50, 30, true); delay(500);
    base.write(180, 30, true); delay(200);
    ombro.write(50, 30, true); delay(200);
    cotovelo.write(106, 30, true); delay(200);
    garra.write(100, 30, true); delay(500);
    //cotovelo.write(90, 20, true); delay(500);
    //ombro.write(40, 20, true); delay(200);
  }

  // ➕ Pressiona botão fim de curso antes de retornar
  Serial.println("[Braço] 🟢 Executando movimento de fim de curso...");
  cotovelo.write(95, 20, true); delay(300); 
  garra.write(10, 20, true); delay(200);     // GARRA FECHADA
  base.write(62, 20, true); delay(300);
  ombro.write(42, 20, true); delay(300);
  cotovelo.write(51, 20, true); delay(300);
  ombro.write(68, 20, true); delay(500);     // Pressiona botão
  ombro.write(25, 20, true); delay(300);     // Sobe
  cotovelo.write(95, 20, true); delay(300);  // Sobe

  // Retorna ao centro
  Serial.println("[Braço] ↩️ Retornando ao centro...");
  garra.write(10, 20, true); delay(200);
  base.write(0, 20, true); delay(200);
  cotovelo.write(120, 20, true); delay(500);
  ombro.write(10, 20, true); delay(200);

  Serial.println("retornou");
}
