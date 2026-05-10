import socket
import threading
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button
from collections import deque

# ---------------- CONFIG ----------------
HOST = "192.168.1.64"  # "192.168.10.205"
PORT = 23
MAX_POINTS = 1500
# ----------------------------------------

paused = False
lock = threading.Lock()

pressure = deque(maxlen=MAX_POINTS)
temperature = deque(maxlen=MAX_POINTS)
relative = deque(maxlen=MAX_POINTS)
altitude = deque(maxlen=MAX_POINTS)

# ---------- TCP RECEIVER THREAD ----------
def tcp_reader():
    global paused
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))
        file = sock.makefile("r")

        print("Connected to ESP32")

        for line in file:
            if paused:
                continue

            line = line.strip()
            if not line:
                continue

            parts = line.split(",")
            if len(parts) != 5 or parts[0] != "Altimeter":
                continue

            try:
                p = float(parts[1])
                t = float(parts[2])
                r = float(parts[3])
                a = float(parts[4])
            except ValueError:
                continue

            with lock:
                pressure.append(p)
                temperature.append(t)
                relative.append(r)
                altitude.append(a)

# ---------- PLOTTING ----------
fig, axs = plt.subplots(4, 1, sharex=True)
fig.suptitle("ESP32 Altimeter (TCP)")

lines = [
    axs[0].plot([], [], label="Pressure (hPa)")[0],
    axs[1].plot([], [], label="Temperature (°C)", color="orange")[0],
    axs[2].plot([], [], label="Relative")[0],
    axs[3].plot([], [], label="Altitude (m)", color="green")[0],
]

for ax in axs:
    ax.grid(True)
    ax.legend(loc="upper left")

axs[-1].set_xlabel("Samples")

def update(frame):
    with lock:
        ydata = [pressure, temperature, relative, altitude]

        for i, line in enumerate(lines):
            line.set_data(range(len(ydata[i])), ydata[i])
            axs[i].relim()
            axs[i].autoscale_view()

    return lines

# ---------- PAUSE BUTTON ----------
def toggle_pause(event):
    global paused
    paused = not paused
    btn.label.set_text("Resume" if paused else "Pause")

ax_btn = plt.axes([0.82, 0.01, 0.15, 0.05])
btn = Button(ax_btn, "Pause")
btn.on_clicked(toggle_pause)

# ---------- START THREAD ----------
threading.Thread(target=tcp_reader, daemon=True).start()

ani = FuncAnimation(fig, update, interval=200)

plt.show()

