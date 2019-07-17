import math
import matplotlib.pyplot as plt
import numpy as np


def sine(resolution, length, harmonic):
    wave = []
    step_size = math.pi / length
    for x in range(length):
        y = math.sin(x*step_size*harmonic) * (resolution/2)
        wave.append(y)
    return wave


def square(resolution, length, harmonic):
    wave = []
    for x in range(length):
        if (x*harmonic)%length < length/2: y = 0
        else: y = resolution
        wave.append(y)
    return wave



def saw(resolution, length, harmonic):
    wave = []
    slope = resolution / (length / harmonic)
    for x in range(length):
        y = ((x*harmonic)%length)*slope
        wave.append(y)
    return wave



def triangle(resolution, length, harmonic):
    wave = []
    slope = resolution / (length / harmonic)
    for x in range(length):
        if (x*harmonic)%length < length/2: y = ((x*harmonic)%length)*slope
        else: y = ((x*harmonic)%length)*(-1*slope)
        wave.append(y)
    return wave



def tangent(resolution, length, harmonic):
    wave = []
    step_size = math.pi / length
    for x in range(length):
        y = math.tan(x*step_size*harmonic) * (resolution/2)
        wave.append(y)
    return wave



def hyperbolic_tangent(resolution, length, harmonic):
    wave = []
    step_size = math.pi / length
    for x in range(length):
        y = math.tanh(x*step_size*harmonic) * (resolution/2)
        wave.append(y)
    return wave



def rounded_square(resolution, length, harmonic):
    wave = []
    for x in range(length):
        if (x*harmonic)%length < length/2: y = y = math.tanh(x*harmonic) * (resolution/2)
        else: y = math.tanh((length - x)*harmonic) * (resolution/2)
        wave.append(y)
    return wave





'''##################'''
'''####TEST TIME!####'''
'''##################'''
resolution = int(1024)
length = 2**13
waves = [sine(resolution/3, length, 2),
         sine(resolution/6, length, 4),
         sine(resolution/6, length, 6),
         sine(resolution/6, length, 8),
         sine(resolution/12, length, 10),
         sine(resolution/12, length, 12),
         sine(resolution/12, length, 14)]
table_y = np.zeros(length)
table_x = np.zeros(length)
wavetable = []
for i in range(length):
    table_x[i] += i
    for wave in waves:
        table_y[i] += math.floor(wave[i])
    wavetable.append(table_y[i])
        
print(wavetable)
print(len(wavetable), max(wavetable), min(wavetable))


plt.plot(table_x, table_y, label='Waveform')
plt.xlabel('Wavetable Index')
plt.ylabel('Value')
plt.title('Waveform Plot')
plt.show()














    
