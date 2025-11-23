
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
    in.ki.wScan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    in.ki.dwFlags = KEYEVENTF_SCANCODE | (isUp ? KEYEVENTF_KEYUP : 0);
    SendInput(1, &in, sizeof(INPUT));
}

int main() {
    // =========================
    // STATIC TIMELINE (ns)
    // =========================

   sleep_until_ns(13428180700LL);  key_evt(116,  0);
   sleep_until_ns(13565170400LL);  key_evt(116,  1);

        return 0;
    }
    