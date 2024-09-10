import random
import os
NUM_TRAFFIC_LIGHTS = 1000
times = []

for hr in range(24):
    for min in [0, 15, 30, 45]:
        times.append(f"{hr:02}{min:02}")

os.remove("./data") 
file_path = "./data"

with open(file_path, 'w') as file:
    for time in times:
        for i in range(1, NUM_TRAFFIC_LIGHTS + 1):
            cars = random.randint(0, 100000)
            file.write(f"{time} {i} {cars}\n")