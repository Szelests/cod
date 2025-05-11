import serial
import serial.tools.list_ports
import tkinter as tk
import re

def find_arduino_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        if 'Arduino' in p.description or 'USB Serial' in p.description:
            return p.device
    return None

arduino_port = find_arduino_port()
if arduino_port is None:
    print("Arduino não encontrado. Verifique a conexão.")
    exit()

ser = serial.Serial(arduino_port, 9600, timeout=1)
ser.flushInput()

def update_color():
    try:
        if ser.in_waiting > 0:
            data = ser.readline().decode().strip()
            if not data:
                return

            # Usa regex para extrair números mesmo com espaços ou formatos variados
            matches = re.findall(r'R:\s*(\d+).*?G:\s*(\d+).*?B:\s*(\d+)', data)
            if matches:
                red, green, blue = map(int, matches[0])
                color = '#{:02x}{:02x}{:02x}'.format(red, green, blue)
                color_label.config(
                    bg=color,
                    text=f"RGB: {red}, {green}, {blue}\nHEX: {color}"
                )
            else:
                print(f"Dados fora do padrão: {data}")
    except Exception as e:
        print(f"Erro: {e}")
    root.after(100, update_color)

root = tk.Tk()
color_label = tk.Label(root, width=50, height=20, font=('Arial', 16))
color_label.pack()

update_color()
root.mainloop()