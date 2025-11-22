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
    void mouse_move(int x, int y) {
    INPUT in = {0};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    in.mi.dx = (x * 65535) / GetSystemMetrics(SM_CXSCREEN);
    in.mi.dy = (y * 65535) / GetSystemMetrics(SM_CYSCREEN);
    SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_click_down(int event) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;

        if (event == 100)      in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        else if (event == 101) in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        else if (event == 102) in.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;

        SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_click_up(int event) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;

        if (event == 100)      in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        else if (event == 101) in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        else if (event == 102) in.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;

        SendInput(1, &in, sizeof(INPUT));
    }

    void mouse_scroll(int amount) {
        INPUT in = {0};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_WHEEL;
        in.mi.mouseData = amount;
        SendInput(1, &in, sizeof(INPUT));
    }

    int main() {
        // =========================
        // STATIC TIMELINE (ns)
        // =========================


        sleep_until_ns(561411100LL);  key_evt(160,  0);
    sleep_until_ns(825846600LL);  key_evt(65,  0);
    sleep_until_ns(930047200LL);  key_evt(160,  1);
    sleep_until_ns(944860000LL);  key_evt(65,  1);
    sleep_until_ns(983097700LL);  key_evt(80,  0);
    sleep_until_ns(1064320400LL);  key_evt(65,  0);
    sleep_until_ns(1080503500LL);  key_evt(80,  1);
    sleep_until_ns(1183703900LL);  key_evt(65,  1);
    sleep_until_ns(1452037300LL);  key_evt(75,  0);
    sleep_until_ns(1575173000LL);  key_evt(75,  1);
    sleep_until_ns(1724132200LL);  key_evt(65,  0);
    sleep_until_ns(1827520000LL);  key_evt(65,  1);
    sleep_until_ns(1875807800LL);  key_evt(66,  0);
    sleep_until_ns(1975744000LL);  key_evt(66,  1);
    sleep_until_ns(1982135100LL);  key_evt(65,  0);
    sleep_until_ns(2241875300LL);  key_evt(82,  0);
    sleep_until_ns(2325549000LL);  key_evt(65,  1);
    sleep_until_ns(2385411200LL);  key_evt(82,  1);
    sleep_until_ns(2505107700LL);  key_evt(32,  0);
    sleep_until_ns(2572514600LL);  key_evt(32,  1);
    sleep_until_ns(2617034000LL);  key_evt(68,  0);
    sleep_until_ns(2739951800LL);  key_evt(85,  0);
    sleep_until_ns(2751904500LL);  key_evt(68,  1);
    sleep_until_ns(2841582100LL);  key_evt(85,  1);
    sleep_until_ns(3079326800LL);  key_evt(73,  0);
    sleep_until_ns(3194371000LL);  key_evt(73,  1);
    sleep_until_ns(3545059100LL);  key_evt(8,  0);
    sleep_until_ns(3616908700LL);  key_evt(8,  1);
    sleep_until_ns(4405099700LL);  key_evt(78,  0);
    sleep_until_ns(4507002400LL);  key_evt(78,  1);
    sleep_until_ns(4544597200LL);  key_evt(73,  0);
    sleep_until_ns(4659074800LL);  key_evt(73,  1);
    sleep_until_ns(4687485600LL);  key_evt(65,  0);
    sleep_until_ns(4810916300LL);  key_evt(65,  1);
    sleep_until_ns(5151447800LL);  key_evt(190,  0);
    sleep_until_ns(5244348500LL);  key_evt(190,  1);
    sleep_until_ns(5349163700LL);  key_evt(32,  0);
    sleep_until_ns(5452410800LL);  key_evt(32,  1);
    sleep_until_ns(5492293900LL);  key_evt(160,  0);
    sleep_until_ns(5629439800LL);  key_evt(72,  0);
    sleep_until_ns(5692363100LL);  key_evt(160,  1);
    sleep_until_ns(5720038200LL);  key_evt(72,  1);
    sleep_until_ns(5802599800LL);  key_evt(65,  0);
    sleep_until_ns(5863981700LL);  key_evt(76,  0);
    sleep_until_ns(5926091200LL);  key_evt(65,  1);
    sleep_until_ns(5951125900LL);  key_evt(76,  1);
    sleep_until_ns(6042377600LL);  key_evt(79,  0);
    sleep_until_ns(6140140400LL);  key_evt(79,  1);
    sleep_until_ns(6243706600LL);  key_evt(32,  0);
    sleep_until_ns(6342222600LL);  key_evt(32,  1);
    sleep_until_ns(6435363300LL);  key_evt(78,  0);
    sleep_until_ns(6447937200LL);  key_evt(78,  1);
    sleep_until_ns(6541318400LL);  key_evt(65,  0);
    sleep_until_ns(6637993000LL);  key_evt(77,  0);
    sleep_until_ns(6651537100LL);  key_evt(65,  1);
    sleep_until_ns(6742457000LL);  key_evt(77,  1);
    sleep_until_ns(6750789900LL);  key_evt(65,  0);
    sleep_until_ns(6843768200LL);  key_evt(65,  1);
    sleep_until_ns(7022733700LL);  key_evt(85,  0);
    sleep_until_ns(7112376200LL);  key_evt(85,  1);
    sleep_until_ns(7471856100LL);  key_evt(8,  0);
    sleep_until_ns(7584937400LL);  key_evt(8,  1);
    sleep_until_ns(7735175900LL);  key_evt(75,  0);
    sleep_until_ns(7847040900LL);  key_evt(75,  1);
    sleep_until_ns(8544472500LL);  key_evt(85,  0);
    sleep_until_ns(8632057100LL);  key_evt(85,  1);
    sleep_until_ns(8726570100LL);  key_evt(32,  0);
    sleep_until_ns(8803355100LL);  key_evt(32,  1);
    sleep_until_ns(8861777400LL);  key_evt(160,  0);
    sleep_until_ns(8994383900LL);  key_evt(65,  0);
    sleep_until_ns(9062617200LL);  key_evt(160,  1);
    sleep_until_ns(9095850900LL);  key_evt(76,  0);
    sleep_until_ns(9107573200LL);  key_evt(65,  1);
    sleep_until_ns(9165370300LL);  key_evt(76,  1);
    sleep_until_ns(9229855500LL);  key_evt(76,  0);
    sleep_until_ns(9323534300LL);  key_evt(76,  1);
    sleep_until_ns(9387614900LL);  key_evt(69,  0);
    sleep_until_ns(9515563200LL);  key_evt(69,  1);
    sleep_until_ns(9735068500LL);  key_evt(32,  0);
    sleep_until_ns(9822425700LL);  key_evt(32,  1);
    sleep_until_ns(10489659800LL);  key_evt(116,  0);
    sleep_until_ns(10552074500LL);  key_evt(116,  1);

        return 0;
    }
    