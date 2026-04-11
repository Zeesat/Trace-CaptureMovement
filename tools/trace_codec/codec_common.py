from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import struct


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parents[1]
TRACE_BIN = ROOT_DIR / "data" / "trace_events.bin"
REPLAY_C = ROOT_DIR / "generated" / "trace_output_static.c"
KEYBOARD_TEXT = SCRIPT_DIR / "keyboard_readable.txt"
MOUSE_TEXT = SCRIPT_DIR / "mouse_readable.txt"

EVENT_STRUCT = struct.Struct("<qBBHiii")

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

TIME_TOKEN_RE = re.compile(r"([+-]?(?:\d+(?:[.,]\d+)?|[.,]\d+))\s*([a-zA-Z]*)")
EVENT_ROW_RE = re.compile(
    r"\{\s*(-?\d+)LL\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\}"
)
COORD_RE = re.compile(r"\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)")


@dataclass(frozen=True)
class TraceEvent:
    t_ns: int
    event_type: int
    flags: int
    code: int
    x: int
    y: int
    wheel: int


KEY_SPECS = [
    (1, 0, "Esc", ["Escape"]),
    (2, 0, "1", []),
    (3, 0, "2", []),
    (4, 0, "3", []),
    (5, 0, "4", []),
    (6, 0, "5", []),
    (7, 0, "6", []),
    (8, 0, "7", []),
    (9, 0, "8", []),
    (10, 0, "9", []),
    (11, 0, "0", []),
    (12, 0, "Minus", ["-"]),
    (13, 0, "Equals", ["="]),
    (14, 0, "Backspace", ["Bksp"]),
    (15, 0, "Tab", []),
    (16, 0, "Q", []),
    (17, 0, "W", []),
    (18, 0, "E", []),
    (19, 0, "R", []),
    (20, 0, "T", []),
    (21, 0, "Y", []),
    (22, 0, "U", []),
    (23, 0, "I", []),
    (24, 0, "O", []),
    (25, 0, "P", []),
    (26, 0, "Left Bracket", ["["]),
    (27, 0, "Right Bracket", ["]"]),
    (28, 0, "Enter", ["Return"]),
    (28, 1, "Numpad Enter", ["Num Enter"]),
    (29, 0, "Left Ctrl", ["Ctrl", "Control", "LCtrl", "LControl"]),
    (29, 1, "Right Ctrl", ["RCtrl", "RControl"]),
    (30, 0, "A", []),
    (31, 0, "S", []),
    (32, 0, "D", []),
    (33, 0, "F", []),
    (34, 0, "G", []),
    (35, 0, "H", []),
    (35, 1, "End", []),
    (36, 0, "J", []),
    (36, 1, "Home", []),
    (37, 0, "K", []),
    (37, 1, "Left Arrow", ["Arrow Left", "Left"]),
    (38, 0, "L", []),
    (38, 1, "Up Arrow", ["Arrow Up", "Up"]),
    (39, 0, "Semicolon", [";"]),
    (39, 1, "Page Down", ["PgDn"]),
    (40, 0, "Apostrophe", ["'"]),
    (40, 1, "Page Up", ["PgUp"]),
    (41, 0, "Grave", ["`", "Backtick"]),
    (42, 0, "Left Shift", ["Shift", "LShift"]),
    (43, 0, "Backslash", ["\\"]),
    (44, 0, "Z", []),
    (45, 0, "X", []),
    (46, 0, "C", []),
    (47, 0, "V", []),
    (48, 0, "B", []),
    (49, 0, "N", []),
    (50, 0, "M", []),
    (51, 0, "Comma", [","]),
    (52, 0, "Period", ["."]),
    (53, 0, "Slash", ["/"]),
    (53, 1, "Numpad Divide", ["Num Divide"]),
    (54, 0, "Right Shift", ["RShift"]),
    (55, 0, "Numpad Multiply", ["Num Multiply", "Numpad *"]),
    (55, 1, "Print Screen", ["PrtSc", "PrtScr"]),
    (56, 0, "Left Alt", ["Alt", "LAlt"]),
    (56, 1, "Right Alt", ["AltGr", "RAlt"]),
    (57, 0, "Space", ["Spacebar"]),
    (58, 0, "Caps Lock", ["CapsLock"]),
    (59, 0, "F1", []),
    (60, 0, "F2", []),
    (61, 0, "F3", []),
    (62, 0, "F4", []),
    (63, 0, "F5", []),
    (64, 0, "F6", []),
    (65, 0, "F7", []),
    (66, 0, "F8", []),
    (67, 0, "F9", []),
    (68, 0, "F10", []),
    (69, 0, "Num Lock", ["NumLock"]),
    (69, 1, "Pause", ["Break"]),
    (70, 0, "Scroll Lock", ["ScrollLock"]),
    (71, 0, "Numpad 7", ["Num 7"]),
    (71, 1, "Home", []),
    (72, 0, "Numpad 8", ["Num 8"]),
    (72, 1, "Up Arrow", ["Arrow Up", "Up"]),
    (73, 0, "Numpad 9", ["Num 9"]),
    (73, 1, "Page Up", ["PgUp"]),
    (74, 0, "Numpad Minus", ["Num Minus"]),
    (75, 0, "Numpad 4", ["Num 4"]),
    (75, 1, "Left Arrow", ["Arrow Left", "Left"]),
    (76, 0, "Numpad 5", ["Num 5"]),
    (77, 0, "Numpad 6", ["Num 6"]),
    (77, 1, "Right Arrow", ["Arrow Right", "Right"]),
    (78, 0, "Numpad Plus", ["Num Plus"]),
    (79, 0, "Numpad 1", ["Num 1"]),
    (79, 1, "End", []),
    (80, 0, "Numpad 2", ["Num 2"]),
    (80, 1, "Down Arrow", ["Arrow Down", "Down"]),
    (81, 0, "Numpad 3", ["Num 3"]),
    (81, 1, "Page Down", ["PgDn"]),
    (82, 0, "Numpad 0", ["Num 0"]),
    (82, 1, "Insert", ["Ins"]),
    (83, 0, "Numpad Decimal", ["Num Decimal"]),
    (83, 1, "Delete", ["Del"]),
    (87, 0, "F11", []),
    (88, 0, "F12", []),
    (91, 1, "Left Win", ["LWin", "Windows"]),
    (92, 1, "Right Win", ["RWin"]),
    (93, 1, "Menu", ["Apps"]),
]

SCANCODE_TO_KEY: dict[tuple[int, int], str] = {}
KEY_NAME_TO_SPEC: dict[str, tuple[int, int]] = {}


def normalize_name(text: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", text.lower())


for code, flags, label, aliases in KEY_SPECS:
    SCANCODE_TO_KEY[(code, flags)] = label
    for alias in [label, *aliases]:
        KEY_NAME_TO_SPEC.setdefault(normalize_name(alias), (code, flags))


MOUSE_BUTTONS = {
    "leftclick": (EV_MOUSE_LEFT_DOWN, EV_MOUSE_LEFT_UP, "Left Click"),
    "rightclick": (EV_MOUSE_RIGHT_DOWN, EV_MOUSE_RIGHT_UP, "Right Click"),
    "middleclick": (EV_MOUSE_MIDDLE_DOWN, EV_MOUSE_MIDDLE_UP, "Middle Click"),
    "x1click": (EV_MOUSE_X1_DOWN, EV_MOUSE_X1_UP, "X1 Click"),
    "x2click": (EV_MOUSE_X2_DOWN, EV_MOUSE_X2_UP, "X2 Click"),
}

for alias, canonical in {
    "left": "leftclick",
    "leftbutton": "leftclick",
    "lmb": "leftclick",
    "right": "rightclick",
    "rightbutton": "rightclick",
    "rmb": "rightclick",
    "middle": "middleclick",
    "middlebutton": "middleclick",
    "mmb": "middleclick",
    "x1": "x1click",
    "xbutton1": "x1click",
    "x2": "x2click",
    "xbutton2": "x2click",
}.items():
    MOUSE_BUTTONS[alias] = MOUSE_BUTTONS[canonical]


def key_label_from_event(code: int, flags: int) -> str:
    return SCANCODE_TO_KEY.get((code, flags & 0x01), f"ScanCode {code}{' Ext' if (flags & 0x01) else ''}")


def key_spec_from_label(label: str) -> tuple[int, int]:
    normalized = normalize_name(label)
    if normalized.startswith("key"):
        normalized = normalized[3:]
    if normalized not in KEY_NAME_TO_SPEC:
        raise ValueError(f"Unknown key label: {label}")
    return KEY_NAME_TO_SPEC[normalized]


def mouse_button_spec(label: str) -> tuple[int, int, str]:
    normalized = normalize_name(label)
    if normalized not in MOUSE_BUTTONS:
        raise ValueError(f"Unknown mouse label: {label}")
    return MOUSE_BUTTONS[normalized]


def format_seconds(value: float) -> str:
    text = f"{value:.9f}".rstrip("0").rstrip(".")
    return text if text else "0"


def format_time_ns(ns_value: int) -> str:
    total_seconds = ns_value / 1_000_000_000.0
    hours = int(total_seconds // 3600)
    total_seconds -= hours * 3600
    minutes = int(total_seconds // 60)
    total_seconds -= minutes * 60

    parts: list[str] = []
    if hours > 0:
        parts.append(f"{hours} h")
    if minutes > 0:
        parts.append(f"{minutes} m")
    parts.append(f"{format_seconds(total_seconds)} s")
    return " ".join(parts)


def parse_time_expression(text: str) -> int:
    lowered = text.strip().lower()
    if not lowered:
        raise ValueError("Empty time expression")

    compact = lowered.replace(",", ".")
    compact = re.sub(r"(?<=\d)\s+(?=\d)", " ", compact)

    if ":" in compact:
        parts = [part.strip() for part in compact.split(":") if part.strip()]
        if not parts:
            raise ValueError(f"Invalid time expression: {text}")
        total_seconds = 0.0
        for part in parts:
            total_seconds = total_seconds * 60.0 + float(part)
        return max(0, int(round(total_seconds * 1_000_000_000.0)))

    tokens = []
    for raw_value, raw_unit in TIME_TOKEN_RE.findall(compact):
        tokens.append((float(raw_value), raw_unit.strip()))

    if not tokens:
        raise ValueError(f"Invalid time expression: {text}")

    explicit_seen = any(unit for _, unit in tokens)
    if not explicit_seen:
        values = [value for value, _ in tokens]
        if len(values) == 1:
            total_seconds = values[0]
        elif len(values) == 2:
            total_seconds = values[0] * 60.0 + values[1]
        else:
            total_seconds = values[0] * 3600.0 + values[1] * 60.0 + values[2]
        return max(0, int(round(total_seconds * 1_000_000_000.0)))

    total_seconds = 0.0
    loose_values: list[float] = []
    for value, unit in tokens:
        if unit.startswith("h"):
            total_seconds += value * 3600.0
        elif unit.startswith("m"):
            total_seconds += value * 60.0
        elif unit.startswith("s"):
            total_seconds += value
        else:
            loose_values.append(value)

    if loose_values:
        if len(loose_values) == 1:
            total_seconds += loose_values[0]
        elif len(loose_values) == 2:
            total_seconds += loose_values[0] * 60.0 + loose_values[1]
        else:
            total_seconds += loose_values[0] * 3600.0 + loose_values[1] * 60.0 + loose_values[2]

    return max(0, int(round(total_seconds * 1_000_000_000.0)))


def strip_comment(line: str) -> str:
    if "#" in line:
        return line.split("#", 1)[0].rstrip()
    return line.rstrip()


def load_trace_events_from_bin(path: Path) -> list[TraceEvent]:
    if not path.exists():
        raise FileNotFoundError(f"Trace file not found: {path}")

    blob = path.read_bytes()
    if not blob:
        return []
    if len(blob) % EVENT_STRUCT.size != 0:
        raise RuntimeError(f"Invalid binary trace size in {path}")

    return [TraceEvent(*row) for row in EVENT_STRUCT.iter_unpack(blob)]


def load_trace_events_from_c(path: Path) -> list[TraceEvent]:
    if not path.exists():
        raise FileNotFoundError(f"Replay C file not found: {path}")

    events: list[TraceEvent] = []
    text = path.read_text(encoding="ascii", errors="ignore")
    for match in EVENT_ROW_RE.finditer(text):
        events.append(
            TraceEvent(
                int(match.group(1)),
                int(match.group(2)),
                int(match.group(3)),
                int(match.group(4)),
                int(match.group(5)),
                int(match.group(6)),
                int(match.group(7)),
            )
        )
    return events


def load_trace_events(path: Path) -> list[TraceEvent]:
    if path.suffix.lower() == ".bin":
        return load_trace_events_from_bin(path)
    return load_trace_events_from_c(path)


def ensure_sorted(events: list[TraceEvent]) -> list[TraceEvent]:
    return sorted(
        events,
        key=lambda event: (
            event.t_ns,
            event.event_type,
            event.flags,
            event.code,
            event.x,
            event.y,
            event.wheel,
        ),
    )


def generate_replay_source(events: list[TraceEvent], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

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

    for event in ensure_sorted(events):
        lines.append(
            f"    {{{event.t_ns}LL, {event.event_type}, {event.flags}, {event.code}, {event.x}, {event.y}, {event.wheel}}},"
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
            '    HMODULE user32 = GetModuleHandleW(L"user32.dll");',
            "    if (user32 != NULL) {",
            "        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);",
            '        FARPROC raw_proc = GetProcAddress(user32, "SetProcessDpiAwarenessContext");',
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
            '        raw_proc = GetProcAddress(user32, "SetProcessDPIAware");',
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
            "    if ((flags & 0x01u) != 0) {",
            "        in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;",
            "    }",
            "    if (key_up) {",
            "        in.ki.dwFlags |= KEYEVENTF_KEYUP;",
            "    }",
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_move_abs(int x, int y) {",
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
            "    SendInput(1, &in, sizeof(in));",
            "}",
            "",
            "static void send_mouse_button(uint8_t type) {",
            "    INPUT in;",
            "    ZeroMemory(&in, sizeof(in));",
            "    in.type = INPUT_MOUSE;",
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
            "                send_mouse_move_abs(event->x, event->y);",
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
            '        if (strcmp(argv[i], "--no-mouse") == 0) {',
            "            allow_mouse = 0;",
            "        }",
            "    }",
            "",
            "    enable_dpi_awareness();",
            "",
            "    if (EVENT_COUNT == 0) {",
            '        fprintf(stderr, "No events compiled.\\n");',
            "        return 1;",
            "    }",
            "    if (!QueryPerformanceFrequency(&g_qpc_freq)) {",
            '        fprintf(stderr, "QPC not available.\\n");',
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

    output_path.write_text("\n".join(lines), encoding="ascii")
