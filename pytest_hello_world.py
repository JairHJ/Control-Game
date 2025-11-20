import serial
import serial.tools.list_ports
import pygame
import sys
import time

# --- Buscar puerto ESP32 ---
def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    esp_ports = []
    for port in ports:
        print(f"Puerto: {port.device} - {port.description}")
        if any(k in port.description.lower() for k in ["espressif", "ch340", "cp210", "silicon"]):
            esp_ports.append(port.device)
    if esp_ports:
        print(f"ESP32 encontrado en: {esp_ports[0]}")
        return esp_ports[0]
    else:
        print("No se encontró ESP32. Conecta y reintenta.")
        input("Presiona Enter para salir...")
        sys.exit(1)

port_name = find_esp32_port()
ser = serial.Serial(port_name, 115200, timeout=0.1)
time.sleep(2)

# --- Inicializar Pygame ---
pygame.init()
WIDTH, HEIGHT = 400, 300
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Control con 4 Touch Pads")
clock = pygame.time.Clock()

# --- Jugador ---
player = pygame.Rect(WIDTH // 2 - 20, HEIGHT // 2 - 20, 40, 40)
SPEED = 5

# --- Bucle principal ---
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # --- Leer comandos del ESP32 ---
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            print(f"Recibido: {line}")
            if line == "UP" and player.top > 0:
                player.y -= SPEED
            elif line == "DOWN" and player.bottom < HEIGHT:
                player.y += SPEED
            elif line == "LEFT" and player.left > 0:
                player.x -= SPEED
            elif line == "RIGHT" and player.right < WIDTH:
                player.x += SPEED
    except Exception as e:
        pass  # Ignorar errores de lectura

    # --- Dibujar ---
    screen.fill((30, 30, 30))
    pygame.draw.rect(screen, (0, 200, 255), player)
    pygame.display.flip()
    clock.tick(60)

# --- Cerrar ---
ser.close()
pygame.quit()
sys.exit()