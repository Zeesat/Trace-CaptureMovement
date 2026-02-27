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

static FILE *g_out = NULL;
static LARGE_INTEGER g_qpc_freq;
static LARGE_INTEGER g_qpc_start;
static uint64_t g_event_count = 0;
static DWORD g_main_thread_id = 0;

static int64_t now_ns(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (int64_t)(((long double)(now.QuadPart - g_qpc_start.QuadPart) * 1000000000.0L) /
                     (long double)g_qpc_freq.QuadPart);
}

static void write_event(uint8_t type, uint8_t flags, uint16_t code, int32_t x, int32_t y, int32_t wheel) {
    TraceEvent event;
    event.t_ns = now_ns();
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

static LRESULT CALLBACK keyboard_hook(int n_code, WPARAM w_param, LPARAM l_param) {
    if (n_code == HC_ACTION && g_out != NULL) {
        const KBDLLHOOKSTRUCT *kb = (const KBDLLHOOKSTRUCT *)l_param;
        if ((kb->flags & LLKHF_INJECTED) == 0) {
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

static LRESULT CALLBACK mouse_hook(int n_code, WPARAM w_param, LPARAM l_param) {
    if (n_code == HC_ACTION && g_out != NULL) {
        const MSLLHOOKSTRUCT *ms = (const MSLLHOOKSTRUCT *)l_param;
        if ((ms->flags & LLMHF_INJECTED) == 0) {
            switch (w_param) {
                case WM_MOUSEMOVE:
                    write_event(EV_MOUSE_MOVE, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_LBUTTONDOWN:
                    write_event(EV_MOUSE_LEFT_DOWN, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_LBUTTONUP:
                    write_event(EV_MOUSE_LEFT_UP, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_RBUTTONDOWN:
                    write_event(EV_MOUSE_RIGHT_DOWN, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_RBUTTONUP:
                    write_event(EV_MOUSE_RIGHT_UP, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_MBUTTONDOWN:
                    write_event(EV_MOUSE_MIDDLE_DOWN, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_MBUTTONUP:
                    write_event(EV_MOUSE_MIDDLE_UP, 0, 0, ms->pt.x, ms->pt.y, 0);
                    break;
                case WM_MOUSEWHEEL:
                    write_event(EV_MOUSE_WHEEL, 0, 0, ms->pt.x, ms->pt.y, (int16_t)HIWORD(ms->mouseData));
                    break;
                case WM_XBUTTONDOWN: {
                    WORD button = HIWORD(ms->mouseData);
                    if (button == XBUTTON1) {
                        write_event(EV_MOUSE_X1_DOWN, 0, 0, ms->pt.x, ms->pt.y, 0);
                    } else if (button == XBUTTON2) {
                        write_event(EV_MOUSE_X2_DOWN, 0, 0, ms->pt.x, ms->pt.y, 0);
                    }
                    break;
                }
                case WM_XBUTTONUP: {
                    WORD button = HIWORD(ms->mouseData);
                    if (button == XBUTTON1) {
                        write_event(EV_MOUSE_X1_UP, 0, 0, ms->pt.x, ms->pt.y, 0);
                    } else if (button == XBUTTON2) {
                        write_event(EV_MOUSE_X2_UP, 0, 0, ms->pt.x, ms->pt.y, 0);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return CallNextHookEx(NULL, n_code, w_param, l_param);
}

static BOOL WINAPI console_handler(DWORD control_type) {
    switch (control_type) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            PostThreadMessageW(g_main_thread_id, WM_QUIT, 0, 0);
            return TRUE;
        default:
            return FALSE;
    }
}

int main(int argc, char **argv) {
    const char *output_path = (argc > 1) ? argv[1] : "data\\trace_events.bin";
    HHOOK keyboard = NULL;
    HHOOK mouse = NULL;

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

    MSG msg;
    PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
    g_main_thread_id = GetCurrentThreadId();
    SetConsoleCtrlHandler(console_handler, TRUE);

    keyboard = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_hook, GetModuleHandleW(NULL), 0);
    mouse = SetWindowsHookExW(WH_MOUSE_LL, mouse_hook, GetModuleHandleW(NULL), 0);
    if (keyboard == NULL || mouse == NULL) {
        fprintf(stderr, "Failed to install global hooks.\n");
        if (keyboard != NULL) {
            UnhookWindowsHookEx(keyboard);
        }
        if (mouse != NULL) {
            UnhookWindowsHookEx(mouse);
        }
        fclose(g_out);
        return 1;
    }

    printf("Recording input to %s\n", output_path);
    printf("Press Ctrl+Break to stop recording.\n");

    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(keyboard);
    UnhookWindowsHookEx(mouse);
    fflush(g_out);
    fclose(g_out);

    printf("Saved %llu events.\n", (unsigned long long)g_event_count);
    return 0;
}
