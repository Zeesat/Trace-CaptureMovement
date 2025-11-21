import time
import json
from pynput import keyboard, mouse
import pydirectinput
import threading
import sys

with open("trace.json", "r") as f:
    raw_data = json.load(f) 
# events = sorted(raw_data, key=lambda x: x["Start"])
# print(events)

timeline = []
for e in raw_data:
    timeline.append(("down", e["Key"], e["Start"]))
    timeline.append(("up", e["Key"], e["Start"] + e["Hold"]))

print(timeline)
# events = sorted(timeline, key=lambda x: x)
timeline.sort(key=lambda x: x[2])
print()
print(timeline)

is_running = False
emergency = False
delay_events = []
prev_time = 0
for action, key, t in timeline:
    delay_events.append((action, key, t - prev_time))
    prev_time = t
    
def precise_sleep(delay):
    end = time.perf_counter_ns() + delay
    while time.perf_counter_ns() < end:
        pass

def timeline_press(timeline):
    global is_running, emergency
    # for i in range(len(timeline)):
        # delay = timeline[i][2] - (0 if i == 0 else timeline[i-1][2])
    for action, key, delay in delay_events:
        precise_sleep(delay)
        if action == "down":
            pydirectinput.keyDown(key)
        # if timeline[i][0] == "up":
        else:
            pydirectinput.keyUp(key)
        print(action)
    is_running = False

 #   for tick in range(int(hold * 1000)):
  #      if emergency:
   #         pydirectinput.keyUp(key)
    #        return
     #   time.sleep(0.001)

def on_press(key):
    global is_running, emergency
    if key == keyboard.Key.f2 and not is_running: 
        is_running = True
        threading.Thread(target=timeline_press, args=(timeline,), daemon=True).start()
    if key == keyboard.Key.esc:
        emergency = True
        time.sleep(0.2)
        sys.exit(0)
def on_release(key):
    pass

with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()