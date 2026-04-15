#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    EV_KEY_DOWN = 0,
    EV_KEY_UP = 1,
    EV_MOUSE_MOVE = 2,
    EV_MOUSE_LEFT_DOWN = 3,
    EV_MOUSE_LEFT_UP = 4,
    EV_MOUSE_RIGHT_DOWN = 5,
    EV_MOUSE_RIGHT_UP = 6,
    EV_MOUSE_MIDDLE_DOWN = 7,
    EV_MOUSE_MIDDLE_UP = 8,
    EV_MOUSE_X1_DOWN = 9,
    EV_MOUSE_X1_UP = 10,
    EV_MOUSE_X2_DOWN = 11,
    EV_MOUSE_X2_UP = 12,
    EV_MOUSE_WHEEL = 13
};

#pragma pack(push, 1)
typedef struct TraceEvent {
    int64_t t_ns;
    uint8_t type;
    uint8_t flags;
    uint16_t code;
    int32_t x;
    int32_t y;
    int32_t wheel;
} TraceEvent;
#pragma pack(pop)

static const DWORD TRACE_INJECT_TAG = 0x54524345u;
static const uint8_t MOUSE_FLAG_INITIAL_ABS = 0x80u;

static FILE *g_out = NULL;
static LARGE_INTEGER g_qpc_freq;
static LARGE_INTEGER g_qpc_start;
static uint64_t g_event_count = 0;
static DWORD g_main_thread_id = 0;
static HWND g_message_window = NULL;

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE ((HANDLE)-3)
#endif

#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
#endif

static void enable_dpi_awareness(void) {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32 != NULL) {
        typedef BOOL(WINAPI *SetProcessDpiAwarenessContext_t)(HANDLE);
        FARPROC raw_proc = GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        SetProcessDpiAwarenessContext_t set_context = NULL;
        if (raw_proc != NULL) {
            union {
                FARPROC raw;
                SetProcessDpiAwarenessContext_t typed;
            } resolver;
            resolver.raw = raw_proc;
            set_context = resolver.typed;
        }
        if (set_context != NULL) {
            if (set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
                return;
            }
            if (set_context(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE)) {
                return;
            }
        }

        typedef BOOL(WINAPI *SetProcessDPIAware_t)(void);
        SetProcessDPIAware_t set_legacy = NULL;
        raw_proc = GetProcAddress(user32, "SetProcessDPIAware");
        if (raw_proc != NULL) {
            union {
                FARPROC raw;
                SetProcessDPIAware_t typed;
            } resolver;
            resolver.raw = raw_proc;
            set_legacy = resolver.typed;
        }
        if (set_legacy != NULL) {
            set_legacy();
        }
    }
}

static int64_t now_ns(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (int64_t)(((long double)(now.QuadPart - g_qpc_start.QuadPart) * 1000000000.0L) /
                     (long double)g_qpc_freq.QuadPart);
}

static void write_event_at(
    int64_t t_ns,
    uint8_t type,
    uint8_t flags,
    uint16_t code,
    int32_t x,
    int32_t y,
    int32_t wheel
) {
    TraceEvent event;
    event.t_ns = t_ns;
    event.type = type;
    event.flags = flags;
    event.code = code;
    event.x = x;
    event.y = y;
    event.wheel = wheel;

    if (fwrite(&event, sizeof(event), 1, g_out) == 1) {
        g_event_count += 1;
        if ((g_event_count & 0xFFu) == 0) {
            fflush(g_out);
        }
    }
}

static void write_event(
    uint8_t type,
    uint8_t flags,
    uint16_t code,
    int32_t x,
    int32_t y,
    int32_t wheel
) {
    write_event_at(now_ns(), type, flags, code, x, y, wheel);
}

static void write_initial_cursor_anchor(void) {
    POINT cursor;
    if (g_out == NULL) {
        return;
    }
    if (!GetCursorPos(&cursor)) {
        return;
    }
    write_event(EV_MOUSE_MOVE, MOUSE_FLAG_INITIAL_ABS, 0, cursor.x, cursor.y, 0);
}

static LRESULT CALLBACK keyboard_hook(int n_code, WPARAM w_param, LPARAM l_param) {
    if (n_code == HC_ACTION && g_out != NULL) {
        const KBDLLHOOKSTRUCT *kb = (const KBDLLHOOKSTRUCT *)l_param;
        if ((kb->flags & LLKHF_INJECTED) == 0 && (DWORD_PTR)kb->dwExtraInfo != (DWORD_PTR)TRACE_INJECT_TAG) {
            uint8_t type = 0xFFu;
            if (w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN) {
                type = EV_KEY_DOWN;
            } else if (w_param == WM_KEYUP || w_param == WM_SYSKEYUP) {
                type = EV_KEY_UP;
            }

            if (type != 0xFFu) {
                uint8_t flags = (kb->flags & LLKHF_EXTENDED) ? 1u : 0u;
                write_event(type, flags, (uint16_t)kb->scanCode, 0, 0, 0);
            }
        }
    }
    return CallNextHookEx(NULL, n_code, w_param, l_param);
}

static void handle_raw_mouse(HRAWINPUT input_handle) {
    RAWINPUT raw;
    UINT size = (UINT)sizeof(raw);
    const RAWMOUSE *mouse;
    int64_t stamp_ns;
    uint8_t raw_flags;

    if (g_out == NULL) {
        return;
    }
    if (GetRawInputData(input_handle, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) != size) {
        return;
    }
    if (raw.header.dwType != RIM_TYPEMOUSE) {
        return;
    }

    mouse = &raw.data.mouse;
    if ((DWORD)mouse->ulExtraInformation == TRACE_INJECT_TAG) {
        return;
    }

    stamp_ns = now_ns();
    raw_flags = (uint8_t)(mouse->usFlags & 0xFFu);

    if ((mouse->lLastX != 0 || mouse->lLastY != 0) && (mouse->usFlags & MOUSE_MOVE_ABSOLUTE) == 0) {
        write_event_at(stamp_ns, EV_MOUSE_MOVE, raw_flags, 0, (int32_t)mouse->lLastX, (int32_t)mouse->lLastY, 0);
    }

    if ((mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_LEFT_DOWN, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_LEFT_UP, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_RIGHT_DOWN, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_RIGHT_UP, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_MIDDLE_DOWN, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_MIDDLE_UP, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_X1_DOWN, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_BUTTON_4_UP) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_X1_UP, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_X2_DOWN, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_BUTTON_5_UP) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_X2_UP, raw_flags, 0, 0, 0, 0);
    }
    if ((mouse->usButtonFlags & RI_MOUSE_WHEEL) != 0) {
        write_event_at(stamp_ns, EV_MOUSE_WHEEL, raw_flags, 0, 0, 0, (SHORT)mouse->usButtonData);
    }
}

static LRESULT CALLBACK message_window_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    (void)w_param;

    switch (message) {
        case WM_INPUT:
            handle_raw_mouse((HRAWINPUT)l_param);
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            if (g_message_window == hwnd) {
                g_message_window = NULL;
            }
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, message, w_param, l_param);
    }
}

static BOOL register_raw_mouse(HWND hwnd) {
    RAWINPUTDEVICE device;

    device.usUsagePage = 0x01;
    device.usUsage = 0x02;
    device.dwFlags = RIDEV_INPUTSINK;
    device.hwndTarget = hwnd;

    return RegisterRawInputDevices(&device, 1, sizeof(device));
}

static HWND create_message_window(HINSTANCE instance) {
    static const wchar_t CLASS_NAME[] = L"TraceRelativeRawInputRecorder";
    WNDCLASSEXW wc;

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = message_window_proc;
    wc.hInstance = instance;
    wc.lpszClassName = CLASS_NAME;

    if (RegisterClassExW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return NULL;
    }

    return CreateWindowExW(
        0,
        CLASS_NAME,
        L"Trace Relative Recorder",
        WS_OVERLAPPED,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
        instance,
        NULL
    );
}

static BOOL WINAPI console_handler(DWORD control_type) {
    switch (control_type) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            if (g_message_window != NULL) {
                PostMessageW(g_message_window, WM_CLOSE, 0, 0);
            }
            PostThreadMessageW(g_main_thread_id, WM_QUIT, 0, 0);
            return TRUE;
        default:
            return FALSE;
    }
}

int main(int argc, char **argv) {
    const char *output_path = (argc > 1) ? argv[1] : "data\\trace_events_relative.bin";
    HHOOK keyboard = NULL;
    HINSTANCE instance = GetModuleHandleW(NULL);
    MSG msg;

    enable_dpi_awareness();

    g_out = fopen(output_path, "wb");
    if (g_out == NULL) {
        fprintf(stderr, "Failed to open output file: %s\n", output_path);
        return 1;
    }
    setvbuf(g_out, NULL, _IOFBF, 1 << 20);

    if (!QueryPerformanceFrequency(&g_qpc_freq) || !QueryPerformanceCounter(&g_qpc_start)) {
        fprintf(stderr, "QPC is not available on this system.\n");
        fclose(g_out);
        return 1;
    }

    PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    g_main_thread_id = GetCurrentThreadId();
    SetConsoleCtrlHandler(console_handler, TRUE);

    g_message_window = create_message_window(instance);
    if (g_message_window == NULL) {
        fprintf(stderr, "Failed to create raw input window.\n");
        fclose(g_out);
        return 1;
    }
    if (!register_raw_mouse(g_message_window)) {
        fprintf(stderr, "Failed to register raw mouse input.\n");
        DestroyWindow(g_message_window);
        g_message_window = NULL;
        fclose(g_out);
        return 1;
    }

    keyboard = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_hook, instance, 0);
    if (keyboard == NULL) {
        fprintf(stderr, "Failed to install keyboard hook.\n");
        DestroyWindow(g_message_window);
        g_message_window = NULL;
        fclose(g_out);
        return 1;
    }

    printf("Recording relative input to %s\n", output_path);
    printf("Press Ctrl+Break to stop recording.\n");

    write_initial_cursor_anchor();

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(keyboard);
    if (g_message_window != NULL) {
        DestroyWindow(g_message_window);
        g_message_window = NULL;
    }
    fflush(g_out);
    fclose(g_out);

    printf("Saved %llu events.\n", (unsigned long long)g_event_count);
    return 0;
}
