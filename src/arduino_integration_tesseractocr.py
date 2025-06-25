import cv2
import pytesseract
import numpy as np
import serial
import time
import threading



# Caminho do Tesseract
pytesseract.pytesseract.tesseract_cmd = r'C:\Program Files\Tesseract-OCR\tesseract.exe'

# Fragmentos para reconhecimento
fragmentos_shampoo = ['sham', 'ham', 'amp', 'mpo', 'poo', 'hoo', 'shampoo']
fragmentos_condicionador = ['con', 'ond', 'ndi', 'dic', 'icio', 'ion', 'nado', 'ador', 'condicionador']
fragmentos_locao = ['loc', 'oca', 'cao', 'ção', 'hidra', 'idr', 'rat', 'atina', 'hidratante', 'loção']

def contem_fragmentos(lista_fragmentos, texto):
    return sum(1 for frag in lista_fragmentos if frag in texto)

# Conexão serial
arduino_esteira = serial.Serial('COM14', 9600, timeout=1)
arduino_braco = serial.Serial('COM5', 9600, timeout=1)

time.sleep(2)

print("✅ Conectado aos Arduinos!")

# Estado
braco_disponivel = True
comando_enviado = False

def enviar_braco(comando):
    arduino_braco.write(comando.encode())
    print(f"✅ Enviado para braço: {comando.strip()}")

# Leitura do Arduino da esteira
def ler_esteira():
    global comando_enviado, braco_disponivel
    while True:
        if arduino_esteira.in_waiting:
            msg = arduino_esteira.readline().decode().strip()
            if msg:
                print(f"[Esteira] {msg}")
                msg_lower = msg.lower()

                if "detecção de peça no final" in msg_lower or "objeto detectado no final" in msg_lower:
                    print(f"🟢 Chegou no final da esteira")
                    print(f"➡️ Estado atual: comando_enviado={comando_enviado}, braco_disponivel={braco_disponivel}")

                    if comando_enviado and braco_disponivel:
                        enviar_braco("ready\n")
                        comando_enviado = False
                        braco_disponivel = False
                        print("📤 Enviado 'ready' ao braço.")
                    else:
                        print("⏹️ Aguardando braço ou comando para enviar 'ready'.")

# Leitura do Arduino do braço
def ler_braco():
    global braco_disponivel
    while True:
        if arduino_braco.in_waiting:
            msg = arduino_braco.readline().decode().strip()
            if msg:
                print(f"[Braço] {msg}")
                if ("pronto" in msg.lower() and "comando" in msg.lower()) or \
                   ("pronto" in msg.lower() and "peça" in msg.lower()) or \
                   ("retornando ao centro" in msg.lower()):
                    braco_disponivel = True
                    print("✅ Braço pronto para o próximo comando.")

# Inicia as threads
threading.Thread(target=ler_esteira, daemon=True).start()
threading.Thread(target=ler_braco, daemon=True).start()

# Câmera
cap = cv2.VideoCapture(1)

ultimo_resultado = ""
ultimo_envio = time.time()
intervalo_envio = 3

while True:
    ret, frame = cap.read()
    if not ret:
        break

    height, width, _ = frame.shape
    roi = frame[3 * height // 4: height, 0: width]

    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
    blur = cv2.bilateralFilter(gray, 9, 75, 75)
    thresh = cv2.adaptiveThreshold(blur, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY_INV, 15, 10)

    texto = pytesseract.image_to_string(thresh, lang='por').lower().strip()

    cont_shampoo = contem_fragmentos(fragmentos_shampoo, texto)
    cont_cond = contem_fragmentos(fragmentos_condicionador, texto)
    cont_locao = contem_fragmentos(fragmentos_locao, texto)

    tempo_atual = time.time()
    resultado = ""

    if cont_shampoo > cont_cond and cont_shampoo > cont_locao and cont_shampoo >= 1:
        resultado = "Shampoo"
    elif cont_cond > cont_shampoo and cont_cond > cont_locao and cont_cond >= 1:
        resultado = "Condicionador"
    elif cont_locao > cont_shampoo and cont_locao > cont_cond and cont_locao >= 1:
        resultado = "Locao Hidratante"
    elif texto != "" and (tempo_atual - ultimo_envio > intervalo_envio) and braco_disponivel and "produto indefinido" != ultimo_resultado.lower():
        resultado = "Produto indefinido"
        enviar_braco("box4\n")
        print("📦 Enviado comando: box4 (Produto indefinido)")
        comando_enviado = True
        ultimo_resultado = resultado
        ultimo_envio = tempo_atual
    else:
        resultado = "Aguardando produto!"

    if resultado in ["Shampoo", "Condicionador", "Locao Hidratante"] and resultado != ultimo_resultado and (tempo_atual - ultimo_envio > intervalo_envio) and braco_disponivel:
        if resultado == "Shampoo":
            enviar_braco("box1\n")
            print("📦 Enviado comando: box1 (Shampoo)")
        elif resultado == "Condicionador":
            enviar_braco("box2\n")
            print("📦 Enviado comando: box2 (Condicionador)")
        elif resultado == "Locao Hidratante":
            enviar_braco("box3\n")
            print("📦 Enviado comando: box3 (Loção)")

        comando_enviado = True
        ultimo_resultado = resultado
        ultimo_envio = tempo_atual

    # Corrige exibição caso sem produto
    if resultado == "Produto indefinido" and texto == "":
        resultado_visivel = "Aguardando produto!"
    else:
        resultado_visivel = resultado

    # Mostra na tela
    cv2.putText(frame, f'OCR Raw: {texto[:60]}', (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
    cv2.putText(frame, f'Resultado: {resultado_visivel}', (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
    cv2.imshow('Frame', frame)
    cv2.imshow('ROI Threshold', thresh)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
arduino_esteira.close()
arduino_braco.close()
