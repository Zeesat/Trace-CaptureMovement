from __future__ import annotations

from dataclasses import dataclass
import os
from pathlib import Path
import shutil
import signal
import struct
import subprocess
import threading
import time

from pynput import keyboard


ROOT_DIR = Path(__file__).resolve().parents[2]
SRC_C_DIR = ROOT_DIR / "src" / "c"
DATA_DIR = ROOT_DIR / "data"
BIN_DIR = ROOT_DIR / "bin"
GENERATED_DIR = ROOT_DIR / "generated"

RECORDER_C = SRC_C_DIR / "trace_input_recorder_relative.c"
RECORDER_EXE = BIN_DIR / "trace_input_recorder_relative.exe"
TRACE_BIN = DATA_DIR / "trace_events_relative.bin"
REPLAY_C = GENERATED_DIR / "trace_output_relative_static.c"
REPLAY_EXE = BIN_DIR / "trace_output_relative_static.exe"

COMMON_COMPILER_DIRS = (
    Path(r"C:\msys64\ucrt64\bin"),
    Path(r"C:\msys64\mingw64\bin"),
    Path(r"C:\msys64\clang64\bin"),
    Path(r"C:\mingw64\bin"),
)

EVENT_STRUCT = struct.Struct("<qBBHiii")
TRACE_INJECT_TAG = 0x54524345
MOUSE_FLAG_INITIAL_ABS = 0x80

EV_KEY_DOWN = 0
EV_KEY_UP = 1
EV_MOUSE_MOVE = 2
EV_MOUSE_LEFT_DOWN = 3
EV_MOUSE_LEFT_UP = 4
EV_MOUSE_RIGHT_DOWN = 5
EV_MOUSE_RIGHT_UP = 6
EV_MOUSE_MIDDLE_DOWN = 7
EV_MOUSE_MIDDLE_UP = 8
EV_MOUSE_X1_DOWN = 9
EV_MOUSE_X1_UP = 10
EV_MOUSE_X2_DOWN = 11
EV_MOUSE_X2_UP = 12
EV_MOUSE_WHEEL = 13

key_a = keyboard.Key.f4  # Start Recording
key_b = keyboard.Key.f5  # Stop Recording
key_c = keyboard.Key.f6  # Compile
key_d = keyboard.Key.f2  # Start Action
key_e = keyboard.Key.f3  # Stop Action
key_f = keyboard.Key.f7  # Mouse
key_g = keyboard.Key.f8  # Loop

record_proc: subprocess.Popen[str] | None = None
play_proc: subprocess.Popen[str] | None = None
play_thread: threading.Thread | None = None
play_stop = threading.Event()
play_lock = threading.Lock()
mouse_on = True
loop_on = False


@dataclass(frozen=True)
class TraceEvent:
    t_ns: int
    event_type: int
    flags: int
    code: int
    x: int
    y: int
    wheel: int


def collect_compiler_dirs() -> list[str]:
    directories: list[str] = []
    seen: set[str] = set()

    for path in COMMON_COMPILER_DIRS:
        if not path.exists():
            continue
        normalized = str(path).lower()
        if normalized in seen:
            continue
        seen.add(normalized)
        directories.append(str(path))

    return directories


def build_process_env() -> dict[str, str]:
    env = os.environ.copy()
    path_parts = collect_compiler_dirs()
    current_path = env.get("PATH", "")

    if current_path:
        path_parts.extend(part for part in current_path.split(os.pathsep) if part)

    if path_parts:
        merged_parts: list[str] = []
        seen: set[str] = set()
        for part in path_parts:
            normalized = part.lower()
            if normalized in seen:
                continue
            seen.add(normalized)
            merged_parts.append(part)
        env["PATH"] = os.pathsep.join(merged_parts)

    return env


def resolve_gcc() -> tuple[str, dict[str, str]]:
    env = build_process_env()
    compiler = shutil.which("gcc", path=env.get("PATH", ""))
    if compiler is not None:
        return compiler, env

    fallback_dirs = collect_compiler_dirs()
    checked_dirs = ", ".join(fallback_dirs) if fallback_dirs else "(no known compiler directories found)"
    raise FileNotFoundError(
        "gcc was not found. Add your compiler bin directory to PATH "
        f"or install GCC in a standard MSYS2 location. Checked: {checked_dirs}"
    )


def run_command(args: list[str], env: dict[str, str] | None = None) -> None:
    result = subprocess.run(args, cwd=ROOT_DIR, env=env, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed: {' '.join(args)}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )


def compile_c(source: Path, output: Path) -> None:
    output.parent.mkdir(parents=True, exist_ok=True)
    compiler, env = resolve_gcc()
    run_command([compiler, str(source), "-O2", "-Wall", "-Wextra", "-o", str(output)], env=env)


def load_trace_events(path: Path) -> list[TraceEvent]:
    if not path.exists():
        raise FileNotFoundError(f"Trace file not found: {path}")

    blob = path.read_bytes()
    if len(blob) == 0:
        raise RuntimeError("Trace file is empty. Record input first (F4/F5).")
    if len(blob) % EVENT_STRUCT.size != 0:
        raise RuntimeError("Trace file size is invalid (format mismatch).")

    events = [TraceEvent(*row) for row in EVENT_STRUCT.iter_unpack(blob)]
    if not events:
        raise RuntimeError("Trace has no events.")
    return events


def generate_replay_source(events: list[TraceEvent]) -> None:
    GENERATED_DIR.mkdir(parents=True, exist_ok=True)

    lines: list[str] = [
        "#include <windows.h>",
        "#include <stdint.h>",
        "#include <stdio.h>",
        "#include <stddef.h>",
        "#include <string.h>",
        "",
        "#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION",
        "#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002",
        "#endif",
        "",
        "#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE",
        "#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((HANDLE)-3)",
        "#endif",
        "",
        "#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2",
        "#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)",
        "#endif",
        "",
        "#ifndef MOUSEEVENTF_MOVE_NOCOALESCE",
        "#define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000",
        "#endif",
        "",
        f"static const DWORD TRACE_INJECT_TAG = 0x{TRACE_INJECT_TAG:08X}u;",
        f"static const uint8_t MOUSE_FLAG_INITIAL_ABS = 0x{MOUSE_FLAG_INITIAL_ABS:02X}u;",
        "",
        "enum {",
        f"    EV_KEY_DOWN = {EV_KEY_DOWN},",
        f"    EV_KEY_UP = {EV_KEY_UP},",
        f"    EV_MOUSE_MOVE = {EV_MOUSE_MOVE},",
        f"    EV_MOUSE_LEFT_DOWN = {EV_MOUSE_LEFT_DOWN},",
        f"    EV_MOUSE_LEFT_UP = {EV_MOUSE_LEFT_UP},",
        f"    EV_MOUSE_RIGHT_DOWN = {EV_MOUSE_RIGHT_DOWN},",
        f"    EV_MOUSE_RIGHT_UP = {EV_MOUSE_RIGHT_UP},",
        f"    EV_MOUSE_MIDDLE_DOWN = {EV_MOUSE_MIDDLE_DOWN},",
        f"    EV_MOUSE_MIDDLE_UP = {EV_MOUSE_MIDDLE_UP},",
        f"    EV_MOUSE_X1_DOWN = {EV_MOUSE_X1_DOWN},",
        f"    EV_MOUSE_X1_UP = {EV_MOUSE_X1_UP},",
        f"    EV_MOUSE_X2_DOWN = {EV_MOUSE_X2_DOWN},",
        f"    EV_MOUSE_X2_UP = {EV_MOUSE_X2_UP},",
        f"    EV_MOUSE_WHEEL = {EV_MOUSE_WHEEL}",
        "};",
        "",
        "typedef struct TraceEvent {",
        "    int64_t t_ns;",
        "    uint8_t type;",
        "    uint8_t flags;",
        "    uint16_t code;",
        "    int32_t x;",
        "    int32_t y;",
        "    int32_t wheel;",
        "} TraceEvent;",
        "",
        "static const TraceEvent EVENTS[] = {",
    ]

    for event in events:
        lines.append(
            f"    {{{event.t_ns}LL, {event.event_type}, {event.flags}, "
            f"{event.code}, {event.x}, {event.y}, {event.wheel}}},"
        )

    lines.extend(
        [
            "};",
            "",
            "static const size_t EVENT_COUNT = sizeof(EVENTS) / sizeof(EVENTS[0]);",
            "static LARGE_INTEGER g_qpc_freq;",
            "static HANDLE g_timer = NULL;",
            "",
            "static void enable_dpi_awareness(void) {",
            "    HMODULE user32 = GetModuleHandleW(L\"user32.dll\");",
            "    if (user32 != NULL) {",
            "        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);",
            "        FARPROC raw_proc = GetProcAddress(user32, \"SetProcessDpiAwarenessContext\");",
            "        SetProcessDpiAwarenessContext_t set_context = NULL;",
            "        if (raw_proc != NULL) {",
            "            union {",
            "                FARPROC raw;",
            "                SetProcessDpiAwarenessContext_t typed;",
            "            } resolver;",
            "            resolver.raw = raw_proc;",
            "            set_context = resolver.typed;",
            "        }",
            "        if (set_context != NULL) {",
            "            if (set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {",
            "                return;",
            "            }",
            "            if (set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {",
            "                return;",
            "            }",
            "        }",
            "        typedef BOOL(WINAPI *SetProcessDPIAware_t)(void);",
            "        SetProcessDPIAware_t set_legacy = NULL;",
            "        raw_proc = GetProcAddress(user32, \"SetProcessDPIAware\");",
            "        if (raw_proc != NULL) {",
            "            union {",
            "                FARPROC raw;",
            "                SetProcessDPIAware_t typed;",
            "            } resolver;",
            "            resolver.raw = raw_proc;",
            "            set_legacy = resolver.typed;",
            "        }",
            "        if (set_legacy != NULL) {",
            "            set_legacy();",
            "        }",
            "    }",
            "}",
            "",
            "static HANDLE create_timer(void) {",
            "    HANDLE timer = CreateWaitableTimerExW(",
            "        NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE",
            "    );",
            "    if (timer == NULL) {",
            "        timer = CreateWaitableTimerW(NULL, FALSE, NULL);",
            "    }",
            "    return timer;",
            "}",
            "",
            "static int64_t elapsed_ns(const LARGE_INTEGER *start) {",
            "    LARGE_INTEGER now;",
            "    QueryPerformanceCounter(&now);",
            "    return (int64_t)(",
            "        ((long double)(now.QuadPart - start->QuadPart) * 1000000000.0L) /",
            "        (long double)g_qpc_freq.QuadPart",
            "    );",
            "}",
            "",
            "static void sleep_ns(int64_t ns) {",
            "    LARGE_INTEGER due;",
            "    if (g_timer == NULL || ns <= 0) {",
            "        return;",
            "    }",
            "    due.QuadPart = -(ns / 100);",
            "    if (due.QuadPart == 0) {",
            "        due.QuadPart = -1;",
            "    }",
            "    if (SetWaitableTimer(g_timer, &due, 0, NULL, NULL, FALSE)) {",
            "        WaitForSingleObject(g_timer, INFINITE);",
            "    }",
            "}",
            "",
            "static void wait_until_ns(const LARGE_INTEGER *start, int64_t target_ns) {",
            "    for (;;) {",
            "        int64_t remain = target_ns - elapsed_ns(start);",
            "        if (remain <= 0) {",
            "            return;",
            "        }",
            "        if (remain > 2000000LL) {",
            "            sleep_ns(remain - 500000LL);",
            "            continue;",
            "        }",
            "        if (remain > 50000LL) {",
            "            SwitchToThread();",
            "            continue;",
            "        }",
            "        YieldProcessor();",
            "    }",
            "}",
            "",
            "static LONG normalize_mouse_coord(int value, int origin, int span) {",
            "    long long normalized;",
            "    long long numerator;",
            "    long long denominator;",
            "    if (span <= 1) {",
            "        return 0;",
            "    }",
            "    numerator = (long long)(value - origin) * 65535LL;",
            "    denominator = (long long)(span - 1);",
            "    if (numerator >= 0) {",
            "        normalized = (numerator + (denominator / 2)) / denominator;",
            "    } else {",
            "        normalized = (numerator - (denominator / 2)) / denominator;",
            "    }",
            "    if (normalized < 0) {",
            "        normalized = 0;",
            "    }",
            "    if (normalized > 65535) {",
            "        normalized = 65535;",
            "    }",
            "    return (LONG)normalized;",
            "}",
            "",
            "static void send_key(uint16_t scan_code, uint8_t flags, int key_up) {",
            "    INPUT in;",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_KEYBOARD;",
            "    in.ki.wScan = scan_code;",
            "    in.ki.dwFlags = KEYEVENTF_SCANCODE;",
            "    in.ki.dwExtraInfo = (ULONG_PTR)TRACE_INJECT_TAG;",
            "    if ((flags & 0x01u) != 0) {",
            "        in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;",
            "    }",
            "    if (key_up) {",
            "        in.ki.dwFlags |= KEYEVENTF_KEYUP;",
            "    }",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_move_abs(int32_t x, int32_t y) {",
            "    INPUT in;",
            "    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);",
            "    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);",
            "    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);",
            "    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_MOUSE;",
            "    in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;",
            "    in.mi.dx = normalize_mouse_coord(x, vx, vw);",
            "    in.mi.dy = normalize_mouse_coord(y, vy, vh);",
            "    in.mi.dwExtraInfo = (ULONG_PTR)TRACE_INJECT_TAG;",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_move_rel(int32_t dx, int32_t dy) {",
            "    INPUT in;",
            "    if (dx == 0 && dy == 0) {",
            "        return;",
            "    }",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_MOUSE;",
            "    in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;",
            "    in.mi.dx = dx;",
            "    in.mi.dy = dy;",
            "    in.mi.dwExtraInfo = (ULONG_PTR)TRACE_INJECT_TAG;",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_button(uint8_t type) {",
            "    INPUT in;",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_MOUSE;",
            "    in.mi.dwExtraInfo = (ULONG_PTR)TRACE_INJECT_TAG;",
            "    switch (type) {",
            "        case EV_MOUSE_LEFT_DOWN: in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break;",
            "        case EV_MOUSE_LEFT_UP: in.mi.dwFlags = MOUSEEVENTF_LEFTUP; break;",
            "        case EV_MOUSE_RIGHT_DOWN: in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break;",
            "        case EV_MOUSE_RIGHT_UP: in.mi.dwFlags = MOUSEEVENTF_RIGHTUP; break;",
            "        case EV_MOUSE_MIDDLE_DOWN: in.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN; break;",
            "        case EV_MOUSE_MIDDLE_UP: in.mi.dwFlags = MOUSEEVENTF_MIDDLEUP; break;",
            "        case EV_MOUSE_X1_DOWN:",
            "            in.mi.dwFlags = MOUSEEVENTF_XDOWN;",
            "            in.mi.mouseData = XBUTTON1;",
            "            break;",
            "        case EV_MOUSE_X1_UP:",
            "            in.mi.dwFlags = MOUSEEVENTF_XUP;",
            "            in.mi.mouseData = XBUTTON1;",
            "            break;",
            "        case EV_MOUSE_X2_DOWN:",
            "            in.mi.dwFlags = MOUSEEVENTF_XDOWN;",
            "            in.mi.mouseData = XBUTTON2;",
            "            break;",
            "        case EV_MOUSE_X2_UP:",
            "            in.mi.dwFlags = MOUSEEVENTF_XUP;",
            "            in.mi.mouseData = XBUTTON2;",
            "            break;",
            "        default:",
            "            return;",
            "    }",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_wheel(int32_t amount) {",
            "    INPUT in;",
            "    int32_t clamped = amount;",
            "    if (clamped > 32767) {",
            "        clamped = 32767;",
            "    }",
            "    if (clamped < -32768) {",
            "        clamped = -32768;",
            "    }",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_MOUSE;",
            "    in.mi.dwFlags = MOUSEEVENTF_WHEEL;",
            "    in.mi.mouseData = (DWORD)(SHORT)clamped;",
            "    in.mi.dwExtraInfo = (ULONG_PTR)TRACE_INJECT_TAG;",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void replay_event(const TraceEvent *event, int allow_mouse) {",
            "    switch (event->type) {",
            "        case EV_KEY_DOWN:",
            "            send_key(event->code, event->flags, 0);",
            "            break;",
            "        case EV_KEY_UP:",
            "            send_key(event->code, event->flags, 1);",
            "            break;",
            "        case EV_MOUSE_MOVE:",
            "            if (allow_mouse) {",
            "                if ((event->flags & MOUSE_FLAG_INITIAL_ABS) != 0) {",
            "                    send_mouse_move_abs(event->x, event->y);",
            "                } else {",
            "                    send_mouse_move_rel(event->x, event->y);",
            "                }",
            "            }",
            "            break;",
            "        case EV_MOUSE_WHEEL:",
            "            if (allow_mouse) {",
            "                send_mouse_wheel(event->wheel);",
            "            }",
            "            break;",
            "        default:",
            "            if (allow_mouse) {",
            "                send_mouse_button(event->type);",
            "            }",
            "            break;",
            "    }",
            "}",
            "",
            "int main(int argc, char **argv) {",
            "    size_t i;",
            "    int allow_mouse = 1;",
            "    int64_t base_ns;",
            "    LARGE_INTEGER start;",
            "",
            "    for (i = 1; i < (size_t)argc; ++i) {",
            "        if (strcmp(argv[i], \"--no-mouse\") == 0) {",
            "            allow_mouse = 0;",
            "        }",
            "    }",
            "",
            "    enable_dpi_awareness();",
            "",
            "    if (EVENT_COUNT == 0) {",
            "        fprintf(stderr, \"No events compiled.\\n\");",
            "        return 1;",
            "    }",
            "    if (!QueryPerformanceFrequency(&g_qpc_freq)) {",
            "        fprintf(stderr, \"QPC not available.\\n\");",
            "        return 1;",
            "    }",
            "",
            "    g_timer = create_timer();",
            "    QueryPerformanceCounter(&start);",
            "    base_ns = EVENTS[0].t_ns;",
            "",
            "    for (i = 0; i < EVENT_COUNT; ++i) {",
            "        int64_t target_ns = EVENTS[i].t_ns - base_ns;",
            "        if (target_ns < 0) {",
            "            target_ns = 0;",
            "        }",
            "        wait_until_ns(&start, target_ns);",
            "        replay_event(&EVENTS[i], allow_mouse);",
            "    }",
            "",
            "    if (g_timer != NULL) {",
            "        CloseHandle(g_timer);",
            "    }",
            "    return 0;",
            "}",
            "",
        ]
    )

    REPLAY_C.write_text("\n".join(lines), encoding="ascii")


def build_recorder() -> None:
    compile_c(RECORDER_C, RECORDER_EXE)


def refresh_replay_source() -> int:
    events = load_trace_events(TRACE_BIN)
    generate_replay_source(events)
    return len(events)


def build_replay() -> int:
    if not TRACE_BIN.exists():
        raise FileNotFoundError(f"Trace file not found: {TRACE_BIN}")

    source_refresh_needed = not REPLAY_C.exists()

    if not source_refresh_needed and TRACE_BIN.exists():
        source_refresh_needed = REPLAY_C.stat().st_mtime < TRACE_BIN.stat().st_mtime

    if source_refresh_needed:
        event_count = refresh_replay_source()
        print(f"Generated relative replay source ({event_count} events).")
    else:
        event_count = 0

    compile_c(REPLAY_C, REPLAY_EXE)
    print("Compiled relative replay executable.")
    return event_count


def compile_all() -> None:
    try:
        DATA_DIR.mkdir(parents=True, exist_ok=True)
        BIN_DIR.mkdir(parents=True, exist_ok=True)
        GENERATED_DIR.mkdir(parents=True, exist_ok=True)

        build_recorder()
        if TRACE_BIN.exists():
            count = build_replay()
            print(f"Compiled relative recorder + replay ({count} events refreshed).")
        else:
            print("Relative recorder compiled. No trace_events_relative.bin yet, replay was skipped.")
    except Exception as exc:
        print(f"Compile failed: {exc}")


def terminate_process(proc: subprocess.Popen[str], timeout_sec: float = 2.0) -> None:
    if proc.poll() is not None:
        return
    try:
        proc.send_signal(signal.CTRL_BREAK_EVENT)
        proc.wait(timeout=timeout_sec)
        return
    except Exception:
        pass
    try:
        proc.terminate()
        proc.wait(timeout=timeout_sec)
        return
    except Exception:
        pass
    proc.kill()


def record_action() -> None:
    global record_proc
    if record_proc is not None and record_proc.poll() is None:
        return

    try:
        if not RECORDER_EXE.exists() or RECORDER_EXE.stat().st_mtime < RECORDER_C.stat().st_mtime:
            build_recorder()
        DATA_DIR.mkdir(parents=True, exist_ok=True)
        record_proc = subprocess.Popen(
            [str(RECORDER_EXE), str(TRACE_BIN)],
            creationflags=subprocess.CREATE_NEW_PROCESS_GROUP,
        )
        print("Relative recording (START)")
    except Exception as exc:
        record_proc = None
        print(f"Recording start failed: {exc}")


def stop_record() -> None:
    global record_proc
    if record_proc is None:
        return
    terminate_process(record_proc)
    record_proc = None
    try:
        event_count = refresh_replay_source()
        print(f"Relative replay source updated ({event_count} events).")
    except Exception as exc:
        print(f"Replay source update failed: {exc}")
    print("Relative recording (STOP)")


def replay_worker(repeat: bool, allow_mouse: bool) -> None:
    global play_proc
    try:
        while not play_stop.is_set():
            cmd = [str(REPLAY_EXE)]
            if not allow_mouse:
                cmd.append("--no-mouse")

            proc = subprocess.Popen(cmd, creationflags=subprocess.CREATE_NEW_PROCESS_GROUP)
            with play_lock:
                play_proc = proc

            while proc.poll() is None and not play_stop.is_set():
                time.sleep(0.005)

            if play_stop.is_set() and proc.poll() is None:
                terminate_process(proc, timeout_sec=1.0)

            with play_lock:
                play_proc = None

            if not repeat:
                break
    finally:
        with play_lock:
            play_proc = None


def start_action() -> None:
    global play_thread

    if play_thread is not None and play_thread.is_alive():
        return

    if not TRACE_BIN.exists():
        print(f"Cannot start action: trace file not found: {TRACE_BIN}")
        return

    try:
        if (not REPLAY_EXE.exists()) or (REPLAY_C.exists() and REPLAY_EXE.stat().st_mtime < REPLAY_C.stat().st_mtime):
            count = build_replay()
            print(f"Relative replay rebuilt ({count} events refreshed).")
    except Exception as exc:
        print(f"Cannot start action: {exc}")
        return

    play_stop.clear()
    play_thread = threading.Thread(target=replay_worker, args=(loop_on, mouse_on), daemon=True)
    play_thread.start()
    print("Relative action (START)")


def stop_action() -> None:
    global play_thread
    if play_thread is None or not play_thread.is_alive():
        return

    play_stop.set()
    with play_lock:
        if play_proc is not None and play_proc.poll() is None:
            terminate_process(play_proc, timeout_sec=1.0)

    play_thread.join(timeout=3.0)
    play_thread = None
    print("Relative action (STOP)")


def on_press(key: keyboard.Key | keyboard.KeyCode) -> None:
    if key == keyboard.Key.f9:
        print("idle")


def on_release(key: keyboard.Key | keyboard.KeyCode) -> None:
    global mouse_on, loop_on

    if key == key_a:
        record_action()
    elif key == key_b:
        stop_record()
    elif key == key_c:
        compile_all()
    elif key == key_d:
        start_action()
    elif key == key_e:
        stop_action()
    elif key == key_f:
        mouse_on = not mouse_on
        print("Mouse ON" if mouse_on else "Mouse OFF")
    elif key == key_g:
        loop_on = not loop_on
        print("Loop ON" if loop_on else "Loop OFF")


def main() -> None:
    with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()


if __name__ == "__main__":
    main()
