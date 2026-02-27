#include <windows.h>
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

    sleep_until_ns(1551421000LL);  key_evt(160, 0);   // SHIFT down
    sleep_until_ns(3050367400LL);  key_evt(65,  0);   // A down
    sleep_until_ns(3984336800LL);  key_evt(65,  1);   // A up
    sleep_until_ns(5042227900LL);  key_evt(160, 1);   // SHIFT up
    sleep_until_ns(6349526300LL);  key_evt(83,  0);   // S down
    sleep_until_ns(7084935900LL);  key_evt(83,  1);   // S up
    sleep_until_ns(7856364800LL);  key_evt(68,  0);   // D down
    sleep_until_ns(8525331900LL);  key_evt(68,  1);   // D up

    return 0;
}
