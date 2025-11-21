import time
import os
import threading
import pydirectinput
from pynput import keyboard, mouse
import json


recording = False

duration_data = []

press_time = {}
keys_press = {}

now = None
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
            start_data = [str(key), time.perf_counter_ns() - now] # noqa
            keys_press[key] = start_data
    except:
        pass

def on_release(key):
    global key_press
    try:
        if key in press_time:
            key_duration = [str(key), time.perf_counter_ns() - press_time[key]]
            keys_press[key] = [keys_press[key], key_duration]
            duration_data.append(keys_press[key])
            print(duration_data)
        with open("trace.json", "w") as f:
            json.dump(duration_data, f)
        del press_time[key]
        del keys_press[key]

    except:
        print("error")

    if key == keyboard.Key.f3 and recording:
        try: 
            dictionary = []
            for i in range(len(duration_data)):
                raw_key = duration_data[i][0][0].lower()
                clean_key = (raw_key.replace("'","")
                                    .replace('"',"")
                                    .replace("key.","")
                                    .replace("alt_l","alt"))
                start_ns = duration_data[i][0][1] # / 1_000_000_000
                hold_ns =  duration_data[i][1][1] #/ 1_000_000_000
                key_start_hold = {"Key": str(clean_key), "Start": start_ns, "Hold": hold_ns}
                dictionary.append(key_start_hold)
            with open("trace.json", "w") as f:
                json.dump(dictionary, f)
        except Exception as e:
            with open("trace.json", "w") as f:
                json.dump({"error": str(e)}, f)
        time.sleep(0.2)

        return False
            
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()






