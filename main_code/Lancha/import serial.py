"""
Visualizador 3D en VPython para un IMU “casero”
Lee líneas tipo:
    AX=1.66 AY=4.90 AZ=7.70 GX=0.07 GY=0.01 GZ=-0.02
por el puerto serie y orienta una pirámide según la gravedad
(eje Z del acelerómetro).
Autor: Miguel A.
"""

import serial
import math
import time
from vpython import canvas, vector, pyramid, rate, color

# ---------- Configuración ----------
PORT      = "COM15"     # <-- Ajusta tu puerto
BAUD      = 115200     # <-- Igual que en tu micro
REFRESH_HZ = 60        # FPS de la animación
G_SMOOTH   = 0.2       # Suavizado exponencial de la gravedad
# -----------------------------------

def parse_line(line):
    """Convierte 'AX=1.23 AY=...' en dict {'AX':1.23, ...}"""
    out = {}
    for token in line.strip().split():
        if '=' in token:
            k, v = token.split('=')
            try:
                out[k] = float(v)
            except ValueError:
                pass
    return out

def accel_to_orientation(ax, ay, az):
    """
    Calcula pitch y roll a partir del vector de gravedad.
    Convención:
      • pitch (+): nariz arriba (rotación en X)
      • roll  (+): giro antihorario mirando hacia delante (rot. en Y)
    """
    pitch = math.atan2(-ax, math.sqrt(ay*ay + az*az))
    roll  = math.atan2( ay,         az)
    return pitch, roll

# ---------- Serie ----------
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # espera a que el micro reinicie
ser.flush()

# ---------- Escena ----------
scene = canvas(title="IMU – Pirámide 3D",
               width=800, height=600, background=color.white)
pyr = pyramid(size=vector(2, 2, 2), color=color.red, make_trail=False)


# Mantén la pirámide apuntando a +Z cuando está nivelada
pyr.axis = vector(0, 0, 1)
pyr.up   = vector(0, 1, 0)

# Variables para suavizado
g_vec = vector(0, 0, 9.81)

while True:
    rate(REFRESH_HZ)
    if ser.in_waiting:
        line = ser.readline().decode(errors="ignore")
        data = parse_line(line)
        # Necesitamos al menos los tres ejes de aceleración
        if all(k in data for k in ("AX", "AY", "AZ")):
            ax, ay, az = data["AX"], data["AY"], data["AZ"]

            # Suavizado simple: low-pass exponencial
            g_vec = g_vec * (1 - G_SMOOTH) + vector(ax, ay, az) * G_SMOOTH

            pitch, roll = accel_to_orientation(g_vec.x, g_vec.y, g_vec.z)

            # Aplicar rotaciones: primero reset, luego roll, luego pitch
            pyr.up = vector(0, 1, 0)
            pyr.axis = vector(0, 0, 1)

            # Rotamos utilizando la función rotate de VPython
            # (los signos pueden requerir ajuste según montaje)
            pyr.rotate(angle=roll,  axis=vector(0, 0, 1))  # roll (Y-Z)
            pyr.rotate(angle=pitch, axis=vector(1, 0, 0))  # pitch (X-Z)

            # Opcional: imprimir datos por consola
            # print(f"Pitch {math.degrees(pitch):6.1f}°  Roll {math.degrees(roll):6.1f}°")
