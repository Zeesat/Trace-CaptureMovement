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
    unsigned long long hold;
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

int any_mouse_hold = 0;
int any_key_hold = 0;


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
    }

    LeaveCriticalSection(&cs);
    SetEvent(logger_event);
}


// ======================================================================
// Logger Thread
// ======================================================================
DWORD WINAPI logger_thread(LPVOID arg) {

    SetThreadAffinityMask(GetCurrentThread(), 1);

    FILE *f = fopen("trace.json", "a");
    if (!f) return 1;

    FILE *fraw = fopen("trace.raw", "ab");
    if (!fraw) fraw = NULL;

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

            if (e.type == 0) {
                fprintf(f,
                    "{\"Type\":0,\"Key\":\"%u\",\"Start\":%llu,\"Hold\":%llu},\n",
                    e.code, e.start, e.hold
                );
            }
            else if (e.type == 1) {
                if (e.code == 100) {
                    fprintf(f,
                        "{\"Type\":1,\"Event\":100,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.x, e.y, e.start
                    );
                } else if (e.code == 200) {
                    fprintf(f,
                        "{\"Type\":1,\"Event\":200,\"x\":%ld,\"y\":%ld,\"Time\":%llu,\"Delta\":%lld},\n",
                        e.x, e.y, e.start, (long long)e.hold
                    );
                } else if (e.code >= 7 && e.code <= 10) {
                    fprintf(f,
                        "{\"Type\":1,\"Event\":%u,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.code, e.x, e.y, e.start
                    );
                } else {
                    fprintf(f,
                        "{\"Type\":1,\"Event\":%u,\"x\":%ld,\"y\":%ld,\"Time\":%llu},\n",
                        e.code, e.x, e.y, e.start
                    );
                }
            }

            fflush(f);
            if (fraw) {
                fwrite(&e, sizeof(Event), 1, fraw);
                fflush(fraw);
            }
        }
    }

    return 0;
}


// ======================================================================
// Keyboard Hook
// ======================================================================
LRESULT CALLBACK hook_proc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {

        KBDLLHOOKSTRUCT *k = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vk = k->vkCode;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (!ks[vk].active) {
                ks[vk].active = 1;
                any_key_hold++;

                QueryPerformanceCounter(&ks[vk].press_time);
                ks[vk].start = now_ns();
            }
        }

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
// Mouse Hook — sekarang ditambah movement (WM_MOUSEMOVE)
// ======================================================================
LRESULT CALLBACK mouse_proc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {

        MSLLHOOKSTRUCT *m = (MSLLHOOKSTRUCT*)lParam;
        unsigned long long t = now_ns();

        switch (wParam) {

            case WM_MOUSEMOVE:
                push_event(1, 100, m->pt.x, m->pt.y, t, 0);
                break;

            case WM_LBUTTONDOWN: any_mouse_hold = 1; push_event(1, 1, m->pt.x, m->pt.y, t, 0); break;
            case WM_LBUTTONUP:   any_mouse_hold = 0; push_event(1, 2, m->pt.x, m->pt.y, t, 0); break;

            case WM_RBUTTONDOWN: any_mouse_hold = 1; push_event(1, 3, m->pt.x, m->pt.y, t, 0); break;
            case WM_RBUTTONUP:   any_mouse_hold = 0; push_event(1, 4, m->pt.x, m->pt.y, t, 0); break;

            case WM_MBUTTONDOWN: any_mouse_hold = 1; push_event(1, 5, m->pt.x, m->pt.y, t, 0); break;
            case WM_MBUTTONUP:   any_mouse_hold = 0; push_event(1, 6, m->pt.x, m->pt.y, t, 0); break;

            case WM_XBUTTONDOWN: {
                UINT which = HIWORD(m->mouseData);
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

            case WM_MOUSEWHEEL: {
                short delta = GET_WHEEL_DELTA_WPARAM(m->mouseData);
                push_event(1, 200, m->pt.x, m->pt.y, t, (unsigned long long)(int)delta);
            } break;
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
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

    HHOOK kb_hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        hook_proc,
        GetModuleHandle(NULL),
        0
    );

    HHOOK ms_hook = SetWindowsHookEx(
        WH_MOUSE_LL,
        mouse_proc,
        GetModuleHandle(NULL),
        0
    );

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
