#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

LARGE_INTEGER freq;
LARGE_INTEGER t0;

typedef struct {
    DWORD vk;
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

#define MAX_EVENTS 4096
Event buffer[MAX_EVENTS];
int buf_head = 0, buf_tail = 0;

CRITICAL_SECTION cs;
HANDLE logger_event;


// ========================================
// Waktu nanosecond dengan QPC (monotonic setelah affinity fix)
// ========================================
unsigned long long now_ns() {
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);

    long long diff = t.QuadPart - t0.QuadPart;  // signed
    if (diff < 0) diff = 0; // fallback safety

    return (unsigned long long)diff * 1000000000ULL / freq.QuadPart;
}


// ========================================
// Buffer
// ========================================
void push_event(DWORD vk, unsigned long long start, unsigned long long hold) {
    EnterCriticalSection(&cs);
    int next = (buf_head + 1) % MAX_EVENTS;

    if (next != buf_tail) {
        buffer[buf_head].vk = vk;
        buffer[buf_head].start = start;
        buffer[buf_head].hold = hold;
        buf_head = next;
    }

    LeaveCriticalSection(&cs);
    SetEvent(logger_event);
}


// ========================================
// Logger Thread
// ========================================
DWORD WINAPI logger_thread(LPVOID arg) {

    // FIX TERPENTING → kunci thread ke CPU core 0
    SetThreadAffinityMask(GetCurrentThread(), 1);

    FILE *f = fopen("trace.json","a");
    if (!f) return 1;

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

            fprintf(f,
                "{\"Key\":\"%u\",\"Start\":%llu,\"Hold\":%llu},\n",
                e.vk, e.start, e.hold
            );
            fflush(f);
        }
    }
}


// ========================================
// Keyboard Hook
// ========================================
LRESULT CALLBACK hook_proc(int code, WPARAM wParam, LPARAM lParam) {
    if (code == HC_ACTION) {

        KBDLLHOOKSTRUCT *k = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vk = k->vkCode;

        // KEYDOWN
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (!ks[vk].active) {
                ks[vk].active = 1;
                QueryPerformanceCounter(&ks[vk].press_time);
                ks[vk].start = now_ns();
            }
        }

        // KEYUP
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (ks[vk].active == 1) {
                ks[vk].active = 0;

                LARGE_INTEGER t;
                QueryPerformanceCounter(&t);

                long long diff = t.QuadPart - ks[vk].press_time.QuadPart;
                if (diff < 0) diff = 0;

                unsigned long long hold =
                    (unsigned long long)diff * 1000000000ULL / freq.QuadPart;

                push_event(vk, ks[vk].start, hold);
            }
        }
    }

    return CallNextHookEx(NULL, code, wParam, lParam);
}


// ========================================
// MAIN
// ========================================
int main() {

    // FIX TERPENTING: lock thread main ke core 0
    SetThreadAffinityMask(GetCurrentThread(), 1);

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t0);

    FILE *fw = fopen("trace.json","w");
    if (fw) { fprintf(fw, "[\n"); fclose(fw); }

    InitializeCriticalSection(&cs);
    logger_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    CreateThread(NULL, 0, logger_thread, NULL, 0, NULL);

    HHOOK hook = SetWindowsHookEx(
        WH_KEYBOARD_LL,
        hook_proc,
        GetModuleHandle(NULL),
        0
    );

    MSG msg;
    while (GetMessage(&msg,NULL,0,0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
