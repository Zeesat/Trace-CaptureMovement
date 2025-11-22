#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

LARGE_INTEGER freq;
LARGE_INTEGER t0;

// ======================================================================
// Struktur Event
// ======================================================================
typedef struct {
    DWORD type;  // 0=keyboard, 1=mouse
    DWORD code;  // keycode atau mouse-event (1..6=clicks, 100=move, 200=wheel, 7-10=xbutton)
    long x, y;   // posisi mouse (hanya mouse)
    unsigned long long start;
    unsigned long long hold; // untuk keyboard=hold duration, untuk wheel=delta, otherwise 0
} Event;

typedef struct {
    DWORD vk;
    unsigned long long start;
    int active;
    LARGE_INTEGER press_time;
} KeyState;

KeyState ks[256];

#define MAX_EVENTS 16384
Event buffer[MAX_EVENTS];
int buf_head = 0, buf_tail = 0;

CRITICAL_SECTION cs;
HANDLE logger_event;

int any_mouse_hold = 0;   // Tracking mouse hold untuk menentukan interval
int any_key_hold = 0;     // Tracking keyboard hold (multiple keys)


// ======================================================================
// Waktu nanosecond
// ======================================================================
unsigned long long now_ns() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    long long diff = t.QuadPart - t0.QuadPart;
    if (diff < 0) diff = 0;

    return (unsigned long long)diff * 1000000000ULL / freq.QuadPart;
}


// ======================================================================
// Ring Buffer
// ======================================================================
void push_event(DWORD type, DWORD code, long x, long y,
                unsigned long long start, unsigned long long hold)
{
    EnterCriticalSection(&cs);
    int next = (buf_head + 1) % MAX_EVENTS;

    if (next != buf_tail) {
        buffer[buf_head].type  = type;
        buffer[buf_head].code  = code;
        buffer[buf_head].x     = x;
        buffer[buf_head].y     = y;
        buffer[buf_head].start = start;
        buffer[buf_head].hold  = hold;

        buf_head = next;
    } else {
        // buffer full - drop oldest (optional)
        // buf_tail = (buf_tail + 1) % MAX_EVENTS;
        // buffer[buf_head] = ...
    }

    LeaveCriticalSection(&cs);
    SetEvent(logger_event);
}


// ======================================================================
// Logger Thread (menulis JSON + raw binary)
// ======================================================================
DWORD WINAPI logger_thread(LPVOID arg) {

    SetThreadAffinityMask(GetCurrentThread(), 1);

    FILE *f = fopen("trace.json", "a");
    if (!f) return 1;

    FILE *fraw = fopen("trace.raw", "ab"); // full raw binary
    if (!fraw) {
        // continue without raw if can't open
        fraw = NULL;
    }

    while (1) {
        WaitForSingleObject(logger_event, INFINITE);

        while (1) {
            EnterCriticalSection(&cs);
            if (buf_tail == buf_head) {
                LeaveCriticalSection(&cs);
                break;
            }

            Event e = buffer[buf_tail];
            buf_tail = (buf_tail + 1) % MAX_EVENTS;
            LeaveCriticalSection(&cs);

            // Keyboard (Type = 0)
            if (e.type == 0) {
                // {"Type":0,"Key":"65","Start":3050367400,"Hold":933969400},
                fprintf(f,
                    "{\"Type\":0,\"Key\":\"%u\",\"Start\":%llu,\"Hold\":%llu},\n",
                    e.code, e.start, e.hold
                );
            }
            // Mouse (Type = 1)
            else if (e.type == 1) {
                // Different formats depending on e.code
                if (e.code == 100) {
                    // movement
                    fprintf(f,
                        "{\"Type\":1,\"Event\":100,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.x, e.y, e.start
                    );
                } else if (e.code == 200) {
                    // wheel (hold contains delta)
                    fprintf(f,
                        "{\"Type\":1,\"Event\":200,\"x\":%ld,\"y\":%ld,\"Time\":%llu,\"Delta\":%lld},\n",
                        e.x, e.y, e.start, (long long)e.hold
                    );
                } else if (e.code >= 7 && e.code <= 10) {
                    // X buttons: 7= X1 down,8= X1 up,9= X2 down,10= X2 up
                    fprintf(f,
                        "{\"Type\":1,\"Event\":%u,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.code, e.x, e.y, e.start
                    );
                } else {
                    // clicks 1..6 (left/right/middle)
                    fprintf(f,
                        "{\"Type\":1,\"Event\":%u,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.code, e.x, e.y, e.start
                    );
                }
            }

            fflush(f);

            // write raw binary copy if available
            if (fraw) {
                fwrite(&e, sizeof(Event), 1, fraw);
                fflush(fraw);
            }
        }
    }

    if (f) fclose(f);
    if (fraw) fclose(fraw);
    return 0;
}


// ======================================================================
// Keyboard Hook (ASLI, tidak diubah)
// ======================================================================
LRESULT CALLBACK hook_proc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {

        KBDLLHOOKSTRUCT *k = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vk = k->vkCode;

        // KEYDOWN
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (!ks[vk].active) {
                ks[vk].active = 1;
                any_key_hold++;

                QueryPerformanceCounter(&ks[vk].press_time);
                ks[vk].start = now_ns();
            }
        }

        // KEYUP
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (ks[vk].active == 1) {
                ks[vk].active = 0;
                any_key_hold--;

                LARGE_INTEGER t;
                QueryPerformanceCounter(&t);

                long long diff = t.QuadPart - ks[vk].press_time.QuadPart;
                if (diff < 0) diff = 0;

                unsigned long long hold =
                    (unsigned long long)diff * 1000000000ULL / freq.QuadPart;

                push_event(0, vk, 0, 0, ks[vk].start, hold);
            }
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}


// ======================================================================
// Mouse Hook – klik direkam instan, plus wheel & X buttons
// ======================================================================
LRESULT CALLBACK mouse_proc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {

        MSLLHOOKSTRUCT *m = (MSLLHOOKSTRUCT*)lParam;
        unsigned long long t = now_ns();

        switch (wParam) {
            // Left
            case WM_LBUTTONDOWN: any_mouse_hold = 1; push_event(1, 1, m->pt.x, m->pt.y, t, 0); break;
            case WM_LBUTTONUP:   any_mouse_hold = 0; push_event(1, 2, m->pt.x, m->pt.y, t, 0); break;

            // Right
            case WM_RBUTTONDOWN: any_mouse_hold = 1; push_event(1, 3, m->pt.x, m->pt.y, t, 0); break;
            case WM_RBUTTONUP:   any_mouse_hold = 0; push_event(1, 4, m->pt.x, m->pt.y, t, 0); break;

            // Middle
            case WM_MBUTTONDOWN: any_mouse_hold = 1; push_event(1, 5, m->pt.x, m->pt.y, t, 0); break;
            case WM_MBUTTONUP:   any_mouse_hold = 0; push_event(1, 6, m->pt.x, m->pt.y, t, 0); break;

            // X buttons (side)
            case WM_XBUTTONDOWN: {
                UINT which = HIWORD(m->mouseData); // XBUTTON1=1, XBUTTON2=2
                any_mouse_hold = 1;
                if (which == XBUTTON1) push_event(1, 7, m->pt.x, m->pt.y, t, which);
                else if (which == XBUTTON2) push_event(1, 9, m->pt.x, m->pt.y, t, which);
            } break;
            case WM_XBUTTONUP: {
                UINT which = HIWORD(m->mouseData);
                any_mouse_hold = 0;
                if (which == XBUTTON1) push_event(1, 8, m->pt.x, m->pt.y, t, which);
                else if (which == XBUTTON2) push_event(1, 10, m->pt.x, m->pt.y, t, which);
            } break;

            // Wheel
            case WM_MOUSEWHEEL: {
                // wheel delta is GET_WHEEL_DELTA_WPARAM(wParam) => signed short *120 per notch
                short delta = GET_WHEEL_DELTA_WPARAM(m->mouseData);
                push_event(1, 200, m->pt.x, m->pt.y, t, (unsigned long long)(int)delta);
            } break;
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}


// ======================================================================
// Mouse Movement Sampler Thread (1ms saat hold, 10ms idle)
// - Movement already records drag when any_mouse_hold==1
// ======================================================================
DWORD WINAPI mouse_sampler_thread(LPVOID arg) {

    SetThreadAffinityMask(GetCurrentThread(), 1);

    POINT p;
    long lx = -99999, ly = -99999;

    while (1) {
        GetCursorPos(&p);

        if (p.x != lx || p.y != ly) {
            push_event(1, 100, p.x, p.y, now_ns(), 0); // Event 100 = movement
            lx = p.x;
            ly = p.y;
        }

        int hold_active = (any_key_hold > 0 || any_mouse_hold > 0);
        Sleep(hold_active ? 1 : 10);
    }
}


// ======================================================================
// MAIN
// ======================================================================
int main() {

    SetThreadAffinityMask(GetCurrentThread(), 1);

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);

    FILE *fw = fopen("trace.json", "w");
    if (fw) { fprintf(fw, "[\n"); fclose(fw); }

    InitializeCriticalSection(&cs);
    logger_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    CreateThread(NULL, 0, logger_thread, NULL, 0, NULL);

    // Keyboard hook
    HHOOK kb_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        hook_proc,
        GetModuleHandle(NULL),
        0
    );

    // Mouse hook (klik + wheel + xbuttons)
    HHOOK ms_hook = SetWindowsHookEx(
        WH_MOUSE_LL,
        mouse_proc,
        GetModuleHandle(NULL),
        0
    );

    // Mouse movement sampler
    CreateThread(NULL, 0, mouse_sampler_thread, NULL, 0, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
