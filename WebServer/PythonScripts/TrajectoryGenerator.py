import math
import time
import numpy as np

class TrajectoryGenerator:
    def __init__(self, t_interpolation, pi, pf):
        self.constants = {"A":1, "B":1, "C":1, "D":1, "E":1, "F":1}
        self.calcConstants(t_interpolation, pi, pf)
        self.t_interpolation = t_interpolation
        self.t_init = self.currentTime()

    def currentTime(self):
        ms = float(round(time.time() * 1000))
        s = ms/1000
        return s

    def is_interpolation_complete(self):
        return self.currentTime() - self.t_init > self.t_interpolation

    def calcConstants(self, t_interpolation, pi, pf, vi = 0.0, vf = 0.0, ai = 0.0, af = 0.0):
        ti = 0.0
        tf = t_interpolation
        eq = np.array([[ pow(ti,5),     pow(ti,4),     pow(ti,3),    pow(ti,2),    pow(ti,1),  1],
                       [ pow(tf,5),     pow(tf,4),     pow(tf,3),    pow(tf,2),    pow(tf,1),  1],
                       [ 5*pow(ti,4),   4*pow(ti,3),   3*pow(ti,2),  2*pow(ti,1),  1,          0],
                       [ 5*pow(tf,4),   4*pow(tf,3),   3*pow(tf,2),  2*pow(tf,1),  1,          0],
                       [ 20*pow(ti,3),  12*pow(ti,2),  6*pow(ti,1),  2,            0,          0],
                       [ 20*pow(tf,3),  12*pow(tf,2),  6*pow(tf,1),  2,            0,          0] ])
        targets = np.array([[pi],
                            [pf],
                            [vi],
                            [vf],
                            [ai],
                            [af]])
        coeff = np.linalg.inv(eq).dot(targets)

        i = 0
        for key in self.constants.keys():
            self.constants[key] = coeff[i]
            i += 1

    def getPosition(self):
        t = self.currentTime() - self.t_init
        if t > self.t_interpolation:
            t = self.t_interpolation
        position = self.constants["A"]*pow(t,5) + self.constants["B"]*pow(t,4) + self.constants["C"]*pow(t,3) + self.constants["D"]*pow(t,2) + self.constants["E"]*pow(t,1) + self.constants["F"]*1
        return position

if __name__ == '__main__':
    tg = TrajectoryGenerator(10, 3.0, -4.0)
    while not tg.is_interpolation_complete():
        print(tg.getPosition())
