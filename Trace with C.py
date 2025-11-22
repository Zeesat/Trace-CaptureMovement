from pynput import keyboard
import time
import json
import subprocess


key_a = keyboard.Key.f4 #Start Recording
key_b = keyboard.Key.f5 #Stop Recording
key_c = keyboard.Key.f6 #Compile
key_d = keyboard.Key.f2 #Start Action
key_e = keyboard.Key.f3 #Stop Action

start = None
record = None
# COMPILE
def compile():
    print("Compiling")
    with open("trace.json", "r", encoding="utf-8") as read_data:
        text = read_data.read().rstrip(", \n")
        text = "[" + text.split("[",1)[1] + "]"
        raw_data = json.loads(text)

    # Format Change
    data_down = []
    for action in raw_data:
        key = int(action["Key"])
        down = int(action["Start"])
        so = [key, down, 0]
        data_down.append(so)

    data_up = []
    for action in raw_data:
        key = int(action["Key"])
        up = int(action["Start"] + action["Hold"])
        so = [key, up, 1]
        data_up.append(so)

    data_join = data_down + data_up
    data_sorted = sorted(data_join, key=lambda x: x[1])

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

    int main() {
        // =========================
        // STATIC TIMELINE (ns)
        // =========================


    '''
    with open("CTraceOutputGPT.c", "w", encoding="utf-8") as write_code:
        write_code.write(code_C)

    #    sleep_until_ns(8525331900LL);  key_evt(68,  1);
    with open("CTraceOutputGPT.c", "a", encoding="utf-8") as append_code:
        for action in data_sorted:
            C_action = f"    sleep_until_ns({action[1]}LL);  key_evt({action[0]},  {action[2]});\n"
            append_code.write(C_action)

    code_C_close = '''
        return 0;
    }
    '''
    with open("CTraceOutputGPT.c", "a", encoding="utf-8") as close_code:
        close_code.write(code_C_close)

    # [{"Key":"65","Start":3050367400,"Hold":933969400},
    # {"Key":"160","Start":1551421000,"Hold":3498006900},
    # {"Key":"83","Start":6349526300,"Hold":735409600},
    # {"Key":"68","Start":7856364800,"Hold":668967100}]

    subprocess.run(["gcc", "CTraceOutputGPT.c", "-o", "CTraceOutputGPT.exe"])
    print("Compiled")

# RECORD ACTION
def record_action():
    global record
    if record is not None:
        return
    record = subprocess.Popen(["CTraceInputGPT.exe"])
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
    start = subprocess.Popen(["CTraceOutputGPT.exe"])

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
        print("hey")
        
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()