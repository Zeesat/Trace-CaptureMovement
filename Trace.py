import time
import os
import threading
import pydirectinput
from pynput import keyboard, mouse
import json


recording = False

end_data = []
duration_data = [[],[]]

press_time = {}

def on_press(key):
    global recording, now
    if key == keyboard.Key.f2:
        if not recording:
            recording = True
            print("Start")
            now = time.perf_counter_ns() # / 1.000.000.000
        return
    
    if not recording:
        return
    
    try:
        if key not in press_time:
            press_time[key] = time.perf_counter_ns()
            start_data = [str(key), time.perf_counter_ns() - now]
            duration_data[0].append(start_data)
        with open("trace.json", "w") as f:
            json.dump(duration_data, f)
    except:
        pass

def on_release(key):
    try:
        if key in press_time:
            key_duration = [str(key), time.perf_counter_ns() - press_time[key]]
            duration_data[1].append(key_duration)
            print(duration_data)
        with open("trace.json", "w") as f:
            json.dump(duration_data, f)
        del press_time[key]

    except:
        print("error")
    
    if key == keyboard.Key.esc:
        dictionary = {}
        for i in range(len(duration_data[0])):
            for item in duration_data[0][i]:
                key = duration_data[0][i][0]
                key = key.replace("Key.","").replace("'","").replace('"',"")
                duration_data[0][i][0] = key
                times = duration_data[0][i][1]
                times = duration_data[0][i][1] * 1_000_000_000
                duration_data[0][i][1] = times

            for item in duration_data[1][i]:
                key = duration_data[1][i][0]
                key = key.replace("Key.","").replace("'","").replace('"',"")
                duration_data[1][i][0] = key
                times = duration_data[1][i][1]
                times = duration_data[1][i][1] * 1_000_000_000
                duration_data[1][i][1] = times

            dictionary = {"key": str(duration_data[0][i][0]), "start": duration_data[0][i][1], "hold": duration_data[1],} 
        with open("trace.json", "w") as f:
            json.dump

        return False
    
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()






