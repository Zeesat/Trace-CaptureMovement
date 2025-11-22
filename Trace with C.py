from pynput import keyboard
import time
import json
import subprocess


key_a = keyboard.Key.f4 #Start Recording
key_b = keyboard.Key.f5 #Stop Recording
key_c = keyboard.Key.f6 #Compile
key_d = keyboard.Key.f2 #Start Action
key_e = keyboard.Key.f3 #Stop Action
key_f = keyboard.Key.f7 #Mouse
start = None
record = None
mouse_on = True
# COMPILE
def compile():
    print("Compiling")
    with open("trace.json", "r", encoding="utf-8") as read_data:
        text = read_data.read().rstrip(", \n")
        text = "[" + text.split("[",1)[1] + "]"
        raw_data = json.loads(text)

    # Format Change
    data_keyboard_down = []
    for action in raw_data:
        if action["Type"] == 0:
            type = "keyboard"
            key = int(action["Key"])
            down = int(action["Start"])
            so = [type, key, down, 0]
            data_keyboard_down.append(so)

    data_keyboard_up = []
    for action in raw_data:
        if action["Type"] == 0:
            type = "keyboard"
            key = int(action["Key"])
            up = int(action["Start"] + action["Hold"])
            so = [type, key, up, 1]
            data_keyboard_up.append(so)

    data_mouse = []
    event_map = {
        1: ("mouse_click_down", 100),
        2: ("mouse_click_up", 100),
        3: ("mouse_click_down", 101),
        4: ("mouse_click_up", 101),
        5: ("mouse_click_down", 102),
        6: ("mouse_click_up", 102),
        7: ("mouse_click_down", 103),
        8: ("mouse_click_up", 103),
        9: ("mouse_click_down", 104),
        10: ("mouse_click_up", 104),
        # 100: "mouse_move"
    }
    for action in raw_data:
        if action["Type"] == 1 and action["Event"] == 100:
            type = "mouse"
            event = "mouse_move"
            x = int(action["x"])
            y = int(action["y"])
            time = int(action["Time"])
            so = [type, event, time, x, y]
            data_mouse.append(so)
        elif action["Type"] == 1 and action["Event"] == 200:
            type = "mouse"
            event = "mouse_scroll"
            delta = action["Delta"]
            time = int(action["Time"])
            so = [type, event, time, delta]
            data_mouse.append(so)
        elif action["Type"] == 1:
            type = "mouse"
            event, internal_code = event_map.get(int(action["Event"]))
            x = int(action["x"])
            y = int(action["y"])
            time = int(action["Time"])
            so = [type, event, time, x, y, internal_code]
            data_mouse.append(so)
        else:
            continue

    data_join = data_keyboard_down + data_keyboard_up + data_mouse
    data_sorted = sorted(data_join, key=lambda x: x[2])

    code_C = r'''#include <windows.h>
    #include <stdio.h>

    typedef LONG NTSTATUS;
    typedef NTSTATUS (WINAPI *NtDelayExecution_t)(BOOL Alertable, PLARGE_INTEGER DelayInterval);

    // High precision sleep (NtDelayExecution)
    void sleep_until_ns(long long target_ns) {
        static long long current_ns = 0;
        static NtDelayExecution_t NtDelayExecution = NULL;

        if (!NtDelayExecution) {
            NtDelayExecution = (NtDelayExecution_t)
                GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtDelayExecution");
        }

        long long delta_ns = target_ns - current_ns;
        if (delta_ns > 0) {
            LARGE_INTEGER interval;
            interval.QuadPart = - (delta_ns / 100);  // convert ns → 100ns units (negative=relative)
            NtDelayExecution(FALSE, &interval);
        }

        current_ns = target_ns;
    }

    void key_evt(int vk, int isUp) {
        INPUT in = {0};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = vk;
        in.ki.dwFlags = isUp ? KEYEVENTF_KEYUP : 0;
        SendInput(1, &in, sizeof(INPUT));
    }
    void mouse_move(int x, int y) {
    INPUT in = {0};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    in.mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN);
    in.mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN);
    SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_click_down(int event) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;

        if (event == 100)      in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        else if (event == 101) in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        else if (event == 102) in.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

        SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_click_up(int event) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;

        if (event == 100)      in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        else if (event == 101) in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        else if (event == 102) in.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;

        SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_scroll(int amount) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_WHEEL;
        in.mi.mouseData = amount;
        SendInput(1, &in, sizeof(INPUT));
    }

    int main() {
        // =========================
        // STATIC TIMELINE (ns)
        // =========================


    '''
    with open("CTraceOutputKMGPT.c", "w", encoding="utf-8") as write_code:
        write_code.write(code_C)

    #  format C:   sleep_until_ns(8525331900LL);  key_evt(68,  1);
    #  format C:   sleep_until_ns(1290400LL);  mouse_move(382, 259);
    with open("CTraceOutputKMGPT.c", "a", encoding="utf-8") as append_code:
        for action in data_sorted:
            if action[0] == "keyboard":
                C_action = f"   sleep_until_ns({action[2]}LL);  key_evt({action[1]},  {action[3]});\n"
            else: 
                if not mouse_on:
                    continue
                if action[1] == "mouse_move":
                    C_action = f"   sleep_until_ns({action[2]}LL);  {action[1]}({action[3]}, {action[4]});\n"
                elif action[1] == "mouse_scroll":
                    C_action = f"   sleep_until_ns({action[2]}LL);  {action[1]}({action[4]});\n"
                else:
                    C_action = f"   sleep_until_ns({action[2]}LL);  {action[1]}({action[5]});\n"
            append_code.write(C_action)



    code_C_close = '''
        return 0;
    }
    '''
    with open("CTraceOutputKMGPT.c", "a", encoding="utf-8") as close_code:
        close_code.write(code_C_close)

    # [{"Key":"65","Start":3050367400,"Hold":933969400},
    # {"Key":"160","Start":1551421000,"Hold":3498006900},
    # {"Key":"83","Start":6349526300,"Hold":735409600},
    # {"Key":"68","Start":7856364800,"Hold":668967100}]

    subprocess.run(["gcc", "CTraceOutputKMGPT.c", "-o", "CTraceOutputKMGPT.exe"])
    print("Compiled")

# RECORD ACTION
def record_action():
    global record
    if record is not None:
        return
    record = subprocess.Popen(["CTraceInputKMGPT.exe"])
    print("Recording (START)")

# STOP RECORD
def stop_record():
    global record
    if record is None:
        return
    try:
        record.terminate()
        time.sleep(2)
    except:
        record.kill()
    record = None
    print("Recorded (STOP)")

# START ACTION
def start_action():
    global start
    if start is not None:
        return
    print("Action (START)")
    start = subprocess.Popen(["CTraceOutputKMGPT.exe"])
# STOP ACTION
def stop_action():
    global start
    if start is None:
        return
    try:
        start.terminate()
        time.sleep(2)
    except:
        start.kill()
    start = None
    print("Terminate Action (STOP)")

def on_press(key):
    if key == keyboard.Key.f9:
        print("nothing")

def on_release(key):
    if key == key_a:
        record_action()
    if key == key_b:
        stop_record()
    if key == key_c:
        compile()
    if key == key_d:
        start_action()
    if key == key_e:
        stop_action()
    if key == key_f:
        global mouse_on
        if mouse_on:
            mouse_on = False
            print("Mouse OFF")
        if not mouse_on:
            mouse_on = True
            print("Mouse ON")
        
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()