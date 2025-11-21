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


        sleep_until_ns(960645900LL);  key_evt(160,  0);
    sleep_until_ns(1378715700LL);  key_evt(65,  0);
    sleep_until_ns(1466686100LL);  key_evt(160,  1);
    sleep_until_ns(1487996500LL);  key_evt(75,  0);
    sleep_until_ns(1496633400LL);  key_evt(65,  1);
    sleep_until_ns(1584627700LL);  key_evt(75,  1);
    sleep_until_ns(1702625400LL);  key_evt(85,  0);
    sleep_until_ns(1770631200LL);  key_evt(85,  1);
    sleep_until_ns(1965627700LL);  key_evt(32,  0);
    sleep_until_ns(2097641200LL);  key_evt(32,  1);
    sleep_until_ns(2150636900LL);  key_evt(77,  0);
    sleep_until_ns(2250622400LL);  key_evt(77,  1);
    sleep_until_ns(2301628900LL);  key_evt(65,  0);
    sleep_until_ns(2394628100LL);  key_evt(65,  1);
    sleep_until_ns(2437626200LL);  key_evt(85,  0);
    sleep_until_ns(2512637900LL);  key_evt(85,  1);
    sleep_until_ns(2610693800LL);  key_evt(32,  0);
    sleep_until_ns(2649639900LL);  key_evt(32,  1);
    sleep_until_ns(2777906400LL);  key_evt(80,  0);
    sleep_until_ns(2894624400LL);  key_evt(80,  1);
    sleep_until_ns(2946639300LL);  key_evt(69,  0);
    sleep_until_ns(3025641100LL);  key_evt(82,  0);
    sleep_until_ns(3071640300LL);  key_evt(69,  1);
    sleep_until_ns(3122655500LL);  key_evt(82,  1);
    sleep_until_ns(3173861000LL);  key_evt(71,  0);
    sleep_until_ns(3250625700LL);  key_evt(71,  1);
    sleep_until_ns(3350707200LL);  key_evt(73,  0);
    sleep_until_ns(3446726300LL);  key_evt(73,  1);
    sleep_until_ns(3517661000LL);  key_evt(32,  0);
    sleep_until_ns(3598639000LL);  key_evt(32,  1);
    sleep_until_ns(3662630300LL);  key_evt(77,  0);
    sleep_until_ns(3746621600LL);  key_evt(77,  1);
    sleep_until_ns(3793632300LL);  key_evt(65,  0);
    sleep_until_ns(3874635500LL);  key_evt(65,  1);
    sleep_until_ns(3900625400LL);  key_evt(75,  0);
    sleep_until_ns(3978708200LL);  key_evt(75,  1);
    sleep_until_ns(3979316900LL);  key_evt(65,  0);
    sleep_until_ns(4041647200LL);  key_evt(65,  1);
    sleep_until_ns(4152631600LL);  key_evt(66,  0);
    sleep_until_ns(4153038800LL);  key_evt(78,  0);
    sleep_until_ns(4185629000LL);  key_evt(78,  1);
    sleep_until_ns(4187596500LL);  key_evt(66,  1);
    sleep_until_ns(4672695500LL);  key_evt(8,  0);
    sleep_until_ns(4739677300LL);  key_evt(8,  1);
    sleep_until_ns(4814623300LL);  key_evt(8,  0);
    sleep_until_ns(4878623400LL);  key_evt(8,  1);
    sleep_until_ns(5054620600LL);  key_evt(78,  0);
    sleep_until_ns(5122616200LL);  key_evt(78,  1);
    sleep_until_ns(5192657600LL);  key_evt(32,  0);
    sleep_until_ns(5266623300LL);  key_evt(32,  1);
    sleep_until_ns(5440725100LL);  key_evt(65,  0);
    sleep_until_ns(5511633900LL);  key_evt(65,  1);
    sleep_until_ns(5547628100LL);  key_evt(76,  0);
    sleep_until_ns(5639925400LL);  key_evt(76,  1);
    sleep_until_ns(5668675800LL);  key_evt(65,  0);
    sleep_until_ns(5760619000LL);  key_evt(65,  1);
    sleep_until_ns(6022640100LL);  key_evt(8,  0);
    sleep_until_ns(6090620100LL);  key_evt(8,  1);
    sleep_until_ns(6171609000LL);  key_evt(8,  0);
    sleep_until_ns(6250617500LL);  key_evt(8,  1);
    sleep_until_ns(6318641600LL);  key_evt(8,  0);
    sleep_until_ns(6387615800LL);  key_evt(8,  1);
    sleep_until_ns(6672689700LL);  key_evt(77,  0);
    sleep_until_ns(6746630600LL);  key_evt(77,  1);
    sleep_until_ns(6819625100LL);  key_evt(65,  0);
    sleep_until_ns(6881654600LL);  key_evt(76,  0);
    sleep_until_ns(6897617200LL);  key_evt(65,  1);
    sleep_until_ns(6985621900LL);  key_evt(76,  1);
    sleep_until_ns(6995675400LL);  key_evt(65,  0);
    sleep_until_ns(7080621600LL);  key_evt(65,  1);
    sleep_until_ns(7104614700LL);  key_evt(77,  0);
    sleep_until_ns(7162631100LL);  key_evt(77,  1);
    sleep_until_ns(8130625600LL);  key_evt(190,  0);
    sleep_until_ns(8218614300LL);  key_evt(190,  1);
    sleep_until_ns(8329610700LL);  key_evt(32,  0);
    sleep_until_ns(8391663000LL);  key_evt(32,  1);
    sleep_until_ns(8410619400LL);  key_evt(160,  0);
    sleep_until_ns(8596611500LL);  key_evt(160,  1);
    sleep_until_ns(8612626200LL);  key_evt(72,  0);
    sleep_until_ns(8682633500LL);  key_evt(72,  1);
    sleep_until_ns(8788637500LL);  key_evt(65,  0);
    sleep_until_ns(8883663700LL);  key_evt(65,  1);
    sleep_until_ns(8914615700LL);  key_evt(76,  0);
    sleep_until_ns(8995661200LL);  key_evt(76,  1);
    sleep_until_ns(9394619400LL);  key_evt(79,  0);
    sleep_until_ns(9468624500LL);  key_evt(79,  1);
    sleep_until_ns(9612615600LL);  key_evt(32,  0);
    sleep_until_ns(9714614400LL);  key_evt(32,  1);
    sleep_until_ns(9733603400LL);  key_evt(68,  0);
    sleep_until_ns(9842643800LL);  key_evt(68,  1);
    sleep_until_ns(10464615600LL);  key_evt(85,  0);
    sleep_until_ns(10522604200LL);  key_evt(85,  1);
    sleep_until_ns(10649616500LL);  key_evt(78,  0);
    sleep_until_ns(10707680100LL);  key_evt(78,  1);
    sleep_until_ns(10756615000LL);  key_evt(73,  0);
    sleep_until_ns(10835606900LL);  key_evt(73,  1);
    sleep_until_ns(10865605200LL);  key_evt(65,  0);
    sleep_until_ns(10955604800LL);  key_evt(65,  1);
    sleep_until_ns(11067614600LL);  key_evt(188,  0);
    sleep_until_ns(11155609400LL);  key_evt(188,  1);
    sleep_until_ns(11260618900LL);  key_evt(32,  0);
    sleep_until_ns(11329624500LL);  key_evt(32,  1);
    sleep_until_ns(11332580800LL);  key_evt(65,  0);
    sleep_until_ns(11417606200LL);  key_evt(65,  1);
    sleep_until_ns(11459607300LL);  key_evt(75,  0);
    sleep_until_ns(11524610800LL);  key_evt(75,  1);
    sleep_until_ns(11640349600LL);  key_evt(85,  0);
    sleep_until_ns(11707647600LL);  key_evt(85,  1);
    sleep_until_ns(11833609000LL);  key_evt(32,  0);
    sleep_until_ns(11891601100LL);  key_evt(32,  1);
    sleep_until_ns(11923672700LL);  key_evt(83,  0);
    sleep_until_ns(11995603500LL);  key_evt(65,  0);
    sleep_until_ns(12032599400LL);  key_evt(83,  1);
    sleep_until_ns(12090607800LL);  key_evt(65,  1);
    sleep_until_ns(12127603400LL);  key_evt(89,  0);
    sleep_until_ns(12192606800LL);  key_evt(89,  1);
    sleep_until_ns(12208615700LL);  key_evt(65,  0);
    sleep_until_ns(12305653500LL);  key_evt(65,  1);
    sleep_until_ns(12376683300LL);  key_evt(78,  0);
    sleep_until_ns(12449620400LL);  key_evt(78,  1);
    sleep_until_ns(12580686700LL);  key_evt(71,  0);
    sleep_until_ns(12628599400LL);  key_evt(71,  1);
    sleep_until_ns(12731604300LL);  key_evt(32,  0);
    sleep_until_ns(12825609600LL);  key_evt(32,  1);
    sleep_until_ns(12925604500LL);  key_evt(75,  0);
    sleep_until_ns(13001624200LL);  key_evt(75,  1);
    sleep_until_ns(13025604700LL);  key_evt(65,  0);
    sleep_until_ns(13113609300LL);  key_evt(65,  1);
    sleep_until_ns(13127600400LL);  key_evt(76,  0);
    sleep_until_ns(13220609600LL);  key_evt(76,  1);
    sleep_until_ns(13264615000LL);  key_evt(73,  0);
    sleep_until_ns(13338669100LL);  key_evt(73,  1);
    sleep_until_ns(13384615800LL);  key_evt(65,  0);
    sleep_until_ns(13484689600LL);  key_evt(65,  1);
    sleep_until_ns(13508645600LL);  key_evt(78,  0);
    sleep_until_ns(13586604300LL);  key_evt(78,  1);
    sleep_until_ns(13656647800LL);  key_evt(32,  0);
    sleep_until_ns(13723607200LL);  key_evt(83,  0);
    sleep_until_ns(13730662600LL);  key_evt(32,  1);
    sleep_until_ns(13802668500LL);  key_evt(69,  0);
    sleep_until_ns(13828635200LL);  key_evt(83,  1);
    sleep_until_ns(13907681000LL);  key_evt(77,  0);
    sleep_until_ns(13916593800LL);  key_evt(69,  1);
    sleep_until_ns(13964661000LL);  key_evt(77,  1);
    sleep_until_ns(14090613700LL);  key_evt(85,  0);
    sleep_until_ns(14148641500LL);  key_evt(85,  1);
    sleep_until_ns(14269604500LL);  key_evt(65,  0);
    sleep_until_ns(14380618700LL);  key_evt(65,  1);
    sleep_until_ns(15125602600LL);  key_evt(190,  0);
    sleep_until_ns(15201598400LL);  key_evt(190,  1);
    sleep_until_ns(16970577400LL);  key_evt(165,  0);
    sleep_until_ns(16971961100LL);  key_evt(165,  1);
    sleep_until_ns(19121597100LL);  key_evt(68,  0);
    sleep_until_ns(19215827300LL);  key_evt(68,  1);
    sleep_until_ns(19293592400LL);  key_evt(68,  0);
    sleep_until_ns(19420876800LL);  key_evt(32,  0);
    sleep_until_ns(19832108300LL);  key_evt(32,  1);
    sleep_until_ns(20123596100LL);  key_evt(32,  0);
    sleep_until_ns(21283792100LL);  key_evt(68,  1);
    sleep_until_ns(21318591400LL);  key_evt(32,  1);
    sleep_until_ns(21353586200LL);  key_evt(65,  0);
    sleep_until_ns(21429659200LL);  key_evt(65,  1);
    sleep_until_ns(21521593500LL);  key_evt(65,  0);
    sleep_until_ns(21612601400LL);  key_evt(32,  0);
    sleep_until_ns(22351295900LL);  key_evt(32,  1);
    sleep_until_ns(24310644600LL);  key_evt(83,  0);
    sleep_until_ns(24468571700LL);  key_evt(65,  1);
    sleep_until_ns(24542670600LL);  key_evt(83,  1);
    sleep_until_ns(24730582200LL);  key_evt(68,  0);
    sleep_until_ns(24827581200LL);  key_evt(68,  1);
    sleep_until_ns(24883678000LL);  key_evt(68,  0);
    sleep_until_ns(25343958300LL);  key_evt(32,  0);
    sleep_until_ns(25549610800LL);  key_evt(32,  1);
    sleep_until_ns(25794648100LL);  key_evt(32,  0);
    sleep_until_ns(26653584400LL);  key_evt(32,  1);
    sleep_until_ns(26715575200LL);  key_evt(68,  1);
    sleep_until_ns(27845052100LL);  key_evt(164,  0);
    sleep_until_ns(27975850100LL);  key_evt(9,  0);
    sleep_until_ns(28101603600LL);  key_evt(9,  1);
    sleep_until_ns(28528577600LL);  key_evt(164,  1);
    sleep_until_ns(30043580400LL);  key_evt(32,  0);
    sleep_until_ns(30183629600LL);  key_evt(32,  1);
    sleep_until_ns(30368564800LL);  key_evt(160,  0);
    sleep_until_ns(30496645000LL);  key_evt(65,  0);
    sleep_until_ns(30561635400LL);  key_evt(160,  1);
    sleep_until_ns(30568546900LL);  key_evt(160,  0);
    sleep_until_ns(30572684300LL);  key_evt(160,  1);
    sleep_until_ns(30584590600LL);  key_evt(65,  1);
    sleep_until_ns(30702617200LL);  key_evt(65,  0);
    sleep_until_ns(30741572500LL);  key_evt(65,  1);
    sleep_until_ns(30843789500LL);  key_evt(65,  0);
    sleep_until_ns(30902619100LL);  key_evt(65,  1);
    sleep_until_ns(30992564600LL);  key_evt(65,  0);
    sleep_until_ns(31078566000LL);  key_evt(65,  1);
    sleep_until_ns(31252562600LL);  key_evt(65,  0);
    sleep_until_ns(31329560300LL);  key_evt(65,  1);
    sleep_until_ns(31364572700LL);  key_evt(65,  0);
    sleep_until_ns(31486571700LL);  key_evt(65,  1);
    sleep_until_ns(31561557900LL);  key_evt(65,  0);
    sleep_until_ns(31647571700LL);  key_evt(65,  1);
    sleep_until_ns(31709576500LL);  key_evt(65,  0);
    sleep_until_ns(31862560000LL);  key_evt(65,  1);
    sleep_until_ns(32208616700LL);  key_evt(164,  0);
    sleep_until_ns(32359564400LL);  key_evt(9,  0);
    sleep_until_ns(32489562300LL);  key_evt(9,  1);
    sleep_until_ns(34224557500LL);  key_evt(9,  0);
    sleep_until_ns(34344556700LL);  key_evt(9,  1);
    sleep_until_ns(34407575100LL);  key_evt(164,  1);
    sleep_until_ns(37953605100LL);  key_evt(116,  0);
    sleep_until_ns(38057548000LL);  key_evt(116,  1);

        return 0;
    }
    