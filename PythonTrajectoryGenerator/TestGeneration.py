from TrajectoryGenerator import TrajectoryGenerator
import matplotlib.pyplot as plt
import time

# Test Sepoints (x,y)
setpoints = [(0,0), (10,10), (3,7), (7,3), (0,0)]

# Recorded Interpolation for Plotting
x_interp = []
y_interp = []
times = []

# Time per Interpolation and Calculation Freq
t_i = 2
f = 20

for i in range(len(setpoints)):
    if i != 0:
        # X related trajectory
        tg_x = TrajectoryGenerator(t_i, setpoints[i-1][0], setpoints[i][0])
        tg_y = TrajectoryGenerator(t_i, setpoints[i-1][1], setpoints[i][1])

        while not tg_x.is_interpolation_complete() and not tg_y.is_interpolation_complete():
            times.append(tg_x.currentTime() - tg_x.t_init)
            x_interp.append(tg_x.getPosition())
            y_interp.append(tg_y.getPosition())
            time.sleep(1/f)

plt.scatter(x_interp, y_interp, s=20, alpha=0.5)
plt.show()
