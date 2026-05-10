# -------- SERIAL CONFIG --------
SERIAL_PORT = "/dev/tty.usbmodem101"

import serial
import matplotlib.pyplot as plt
from matplotlib.widgets import Button
import time

# ================= SERIAL CONFIG =================
#SERIAL_PORT = "/dev/ttyUSB0"   # macOS: /dev/tty.usbmodemXXXX
BAUD_RATE = 115200               # must match Serial.begin()
# =================================================

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
time.sleep(2)  # allow Arduino reset

# ================= DATA BUFFERS =================
time_data = []
pressure_data = []
temperature_data = []
relative_data = []
altitude_data = []

MAX_POINTS = 1000   # ~60 seconds at 10 Hz
start_time = time.time()
paused = False

# ================= MATPLOTLIB SETUP =================
plt.ion()
fig, (ax1, ax2, ax3, ax4) = plt.subplots(4, 1, sharex=True, figsize=(10, 10))

line_p, = ax1.plot([], [], label="Pressure (hPa)")
line_t, = ax2.plot([], [], label="Temperature (°C)")
line_r, = ax3.plot([], [], label="Relative Altitude (m)")
line_a, = ax4.plot([], [], label="Altitude (m)")

ax1.set_ylabel("Pressure (hPa)")
ax2.set_ylabel("Temperature (°C)")
ax3.set_ylabel("Relative (m)")
ax4.set_ylabel("Altitude (m)")
ax4.set_xlabel("Time (s)")

for ax in (ax1, ax2, ax3, ax4):
    ax.grid(True)
    ax.legend(loc="upper right")

fig.tight_layout(rect=[0, 0.06, 1, 1])

# ================= PAUSE BUTTON =================
ax_pause = plt.axes([0.4, 0.01, 0.2, 0.04])
btn_pause = Button(ax_pause, "Pause", color="lightgray", hovercolor="0.85")

def toggle_pause(event):
    global paused
    paused = not paused
    btn_pause.label.set_text("Resume" if paused else "Pause")
    print("Paused" if paused else "Running")

btn_pause.on_clicked(toggle_pause)

# ================= MAIN LOOP =================
try:
    while True:
        if not paused:
            line = ser.readline().decode("utf-8").strip()

            if line:
                parts = [p.strip() for p in line.split(",")]

                # Expect: pressure, temperature, relative, altitude
                if len(parts) == 4:
                    pressure = float(parts[0])
                    temperature = float(parts[1])
                    relative = float(parts[2])
                    altitude = float(parts[3])

                    t = time.time() - start_time

                    time_data.append(t)
                    pressure_data.append(pressure)
                    temperature_data.append(temperature)
                    relative_data.append(relative)
                    altitude_data.append(altitude)

                    # Keep buffer length fixed
                    if len(time_data) > MAX_POINTS:
                        time_data.pop(0)
                        pressure_data.pop(0)
                        temperature_data.pop(0)
                        relative_data.pop(0)
                        altitude_data.pop(0)

                    # Update plot data
                    line_p.set_data(time_data, pressure_data)
                    line_t.set_data(time_data, temperature_data)
                    line_r.set_data(time_data, relative_data)
                    line_a.set_data(time_data, altitude_data)

                    # Autoscale only while running
                    for ax in (ax1, ax2, ax3, ax4):
                        ax.relim()
                        ax.autoscale_view()

        fig.canvas.draw_idle()
        plt.pause(0.01)

except KeyboardInterrupt:
    print("Exiting...")

finally:
    ser.close()
