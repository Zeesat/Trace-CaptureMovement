
#include <windows.h>
#include <stdio.h>

// =========================
//  MOUSE-ONLY REPLAY ENGINE
// =========================
// Format mengikuti file yang kamu kirim, tetapi TANPA keyboard.
// Tinggal tempel timeline mouse hasil parsing-mu ke dalam main().

// High precision sleep (NtDelayExecution)
typedef LONG NTSTATUS;
typedef NTSTATUS (WINAPI *NtDelayExecution_t)(BOOL Alertable, PLARGE_INTEGER DelayInterval);

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
        interval.QuadPart = - (delta_ns / 100);  // ns → 100ns units
        NtDelayExecution(FALSE, &interval);
    }
    current_ns = target_ns;
}

// =========================
// MOUSE FUNCTIONS
// =========================

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

// =========================
// MAIN (ISI TIMELINE DI SINI)
// =========================

int main() {

       sleep_until_ns(786714300LL);  mouse_move(1615, 945);
   sleep_until_ns(794670600LL);  mouse_move(1615, 942);
   sleep_until_ns(803320100LL);  mouse_move(1615, 939);
   sleep_until_ns(821682600LL);  mouse_move(1615, 935);
   sleep_until_ns(824672300LL);  mouse_move(1617, 932);
   sleep_until_ns(826737600LL);  mouse_move(1617, 930);
   sleep_until_ns(834699100LL);  mouse_move(1617, 928);
   sleep_until_ns(843664800LL);  mouse_move(1617, 927);
   sleep_until_ns(867666000LL);  mouse_move(1617, 926);
   sleep_until_ns(914696300LL);  mouse_move(1617, 926);
   sleep_until_ns(1226699300LL);  mouse_move(1617, 926);
   sleep_until_ns(1442680400LL);  mouse_move(1617, 926);
   sleep_until_ns(1738856300LL);  mouse_click_down(100);
   sleep_until_ns(1858803000LL);  mouse_click_up(100);
   sleep_until_ns(2178813300LL);  mouse_click_down(100);
   sleep_until_ns(2274854200LL);  mouse_click_up(100);
   sleep_until_ns(2746899000LL);  mouse_click_down(100);
   sleep_until_ns(2866911000LL);  mouse_click_up(100);
   sleep_until_ns(3050741500LL);  mouse_move(1617, 924);
   sleep_until_ns(3082716100LL);  mouse_move(1617, 923);
   sleep_until_ns(3090640200LL);  mouse_move(1617, 922);
   sleep_until_ns(3098638000LL);  mouse_move(1617, 921);
   sleep_until_ns(3106635000LL);  mouse_move(1619, 918);
   sleep_until_ns(3114651500LL);  mouse_move(1620, 913);
   sleep_until_ns(3122626800LL);  mouse_move(1621, 906);
   sleep_until_ns(3130635700LL);  mouse_move(1623, 897);
   sleep_until_ns(3138633500LL);  mouse_move(1623, 888);
   sleep_until_ns(3146650500LL);  mouse_move(1623, 877);
   sleep_until_ns(3154637800LL);  mouse_move(1623, 864);
   sleep_until_ns(3162638800LL);  mouse_move(1623, 849);
   sleep_until_ns(3170636000LL);  mouse_move(1623, 831);
   sleep_until_ns(3178641500LL);  mouse_move(1620, 812);
   sleep_until_ns(3186660500LL);  mouse_move(1615, 792);
   sleep_until_ns(3194645400LL);  mouse_move(1605, 769);
   sleep_until_ns(3202643500LL);  mouse_move(1592, 740);
   sleep_until_ns(3210631300LL);  mouse_move(1573, 709);
   sleep_until_ns(3218662800LL);  mouse_move(1547, 674);
   sleep_until_ns(3226645700LL);  mouse_move(1519, 637);
   sleep_until_ns(3234667600LL);  mouse_move(1487, 600);
   sleep_until_ns(3242623000LL);  mouse_move(1449, 562);
   sleep_until_ns(3250671200LL);  mouse_move(1410, 523);
   sleep_until_ns(3258629800LL);  mouse_move(1368, 484);
   sleep_until_ns(3266681900LL);  mouse_move(1327, 451);
   sleep_until_ns(3274636200LL);  mouse_move(1291, 423);
   sleep_until_ns(3282660100LL);  mouse_move(1260, 401);
   sleep_until_ns(3290695100LL);  mouse_move(1229, 382);
   sleep_until_ns(3298639700LL);  mouse_move(1202, 364);
   sleep_until_ns(3306630700LL);  mouse_move(1177, 352);
   sleep_until_ns(3314652300LL);  mouse_move(1149, 341);
   sleep_until_ns(3322637100LL);  mouse_move(1122, 333);
   sleep_until_ns(3330632900LL);  mouse_move(1101, 327);
   sleep_until_ns(3338629600LL);  mouse_move(1079, 324);
   sleep_until_ns(3346654000LL);  mouse_move(1059, 322);
   sleep_until_ns(3354641000LL);  mouse_move(1040, 320);
   sleep_until_ns(3362655000LL);  mouse_move(1022, 320);
   sleep_until_ns(3370643200LL);  mouse_move(1009, 321);
   sleep_until_ns(3378637800LL);  mouse_move(996, 323);
   sleep_until_ns(3386678000LL);  mouse_move(982, 326);
   sleep_until_ns(3394630600LL);  mouse_move(969, 329);
   sleep_until_ns(3402687700LL);  mouse_move(953, 335);
   sleep_until_ns(3410671100LL);  mouse_move(940, 340);
   sleep_until_ns(3418854300LL);  mouse_move(928, 346);
   sleep_until_ns(3426655200LL);  mouse_move(918, 351);
   sleep_until_ns(3434696300LL);  mouse_move(910, 357);
   sleep_until_ns(3442641900LL);  mouse_move(904, 363);
   sleep_until_ns(3450678100LL);  mouse_move(898, 371);
   sleep_until_ns(3458635700LL);  mouse_move(895, 378);
   sleep_until_ns(3466700100LL);  mouse_move(892, 386);
   sleep_until_ns(3474653500LL);  mouse_move(891, 397);
   sleep_until_ns(3482638400LL);  mouse_move(891, 408);
   sleep_until_ns(3490677500LL);  mouse_move(891, 423);
   sleep_until_ns(3498634900LL);  mouse_move(895, 441);
   sleep_until_ns(3506639400LL);  mouse_move(902, 459);
   sleep_until_ns(3514657900LL);  mouse_move(912, 482);
   sleep_until_ns(3522637300LL);  mouse_move(927, 508);
   sleep_until_ns(3530781700LL);  mouse_move(946, 538);
   sleep_until_ns(3538640200LL);  mouse_move(970, 570);
   sleep_until_ns(3546646700LL);  mouse_move(998, 603);
   sleep_until_ns(3554647200LL);  mouse_move(1033, 641);
   sleep_until_ns(3562736800LL);  mouse_move(1074, 673);
   sleep_until_ns(3570647900LL);  mouse_move(1123, 708);
   sleep_until_ns(3578678200LL);  mouse_move(1175, 740);
   sleep_until_ns(3586660500LL);  mouse_move(1230, 766);
   sleep_until_ns(3594666000LL);  mouse_move(1293, 790);
   sleep_until_ns(3602811700LL);  mouse_move(1355, 808);
   sleep_until_ns(3610655500LL);  mouse_move(1411, 819);
   sleep_until_ns(3618691200LL);  mouse_move(1461, 824);
   sleep_until_ns(3626653800LL);  mouse_move(1501, 824);
   sleep_until_ns(3634682200LL);  mouse_move(1534, 821);
   sleep_until_ns(3642650700LL);  mouse_move(1570, 811);
   sleep_until_ns(3650708300LL);  mouse_move(1602, 800);
   sleep_until_ns(3658635700LL);  mouse_move(1631, 786);
   sleep_until_ns(3666672000LL);  mouse_move(1653, 770);
   sleep_until_ns(3674654100LL);  mouse_move(1673, 754);
   sleep_until_ns(3682661100LL);  mouse_move(1687, 738);
   sleep_until_ns(3690627000LL);  mouse_move(1696, 720);
   sleep_until_ns(3698636500LL);  mouse_move(1703, 702);
   sleep_until_ns(3706640700LL);  mouse_move(1707, 683);
   sleep_until_ns(3714647500LL);  mouse_move(1709, 663);
   sleep_until_ns(3722632700LL);  mouse_move(1708, 643);
   sleep_until_ns(3730656600LL);  mouse_move(1702, 622);
   sleep_until_ns(3738642700LL);  mouse_move(1693, 598);
   sleep_until_ns(3746646600LL);  mouse_move(1679, 576);
   sleep_until_ns(3754665500LL);  mouse_move(1659, 553);
   sleep_until_ns(3762646100LL);  mouse_move(1631, 526);
   sleep_until_ns(3770636600LL);  mouse_move(1591, 500);
   sleep_until_ns(3778633900LL);  mouse_move(1540, 474);
   sleep_until_ns(3786652500LL);  mouse_move(1477, 450);
   sleep_until_ns(3794639900LL);  mouse_move(1406, 429);
   sleep_until_ns(3802779000LL);  mouse_move(1332, 413);
   sleep_until_ns(3810634900LL);  mouse_move(1259, 401);
   sleep_until_ns(3818644800LL);  mouse_move(1192, 395);
   sleep_until_ns(3826644100LL);  mouse_move(1134, 393);
   sleep_until_ns(3834683000LL);  mouse_move(1085, 393);
   sleep_until_ns(3842622400LL);  mouse_move(1047, 396);
   sleep_until_ns(3850675000LL);  mouse_move(1013, 403);
   sleep_until_ns(3858632600LL);  mouse_move(978, 413);
   sleep_until_ns(3866675600LL);  mouse_move(950, 423);
   sleep_until_ns(3874635900LL);  mouse_move(928, 437);
   sleep_until_ns(3882654500LL);  mouse_move(909, 453);
   sleep_until_ns(3890635300LL);  mouse_move(892, 469);
   sleep_until_ns(3898647400LL);  mouse_move(880, 487);
   sleep_until_ns(3906643000LL);  mouse_move(870, 507);
   sleep_until_ns(3914646500LL);  mouse_move(865, 527);
   sleep_until_ns(3922636800LL);  mouse_move(861, 547);
   sleep_until_ns(3930650200LL);  mouse_move(861, 568);
   sleep_until_ns(3938626600LL);  mouse_move(862, 592);
   sleep_until_ns(3946638200LL);  mouse_move(870, 618);
   sleep_until_ns(3954634000LL);  mouse_move(881, 646);
   sleep_until_ns(3962626100LL);  mouse_move(900, 676);
   sleep_until_ns(3970633100LL);  mouse_move(928, 713);
   sleep_until_ns(3978626800LL);  mouse_move(962, 747);
   sleep_until_ns(3986660600LL);  mouse_move(999, 778);
   sleep_until_ns(3994627800LL);  mouse_move(1044, 809);
   sleep_until_ns(4002689300LL);  mouse_move(1095, 834);
   sleep_until_ns(4010640300LL);  mouse_move(1149, 854);
   sleep_until_ns(4018934900LL);  mouse_move(1201, 868);
   sleep_until_ns(4026634900LL);  mouse_move(1247, 873);
   sleep_until_ns(4034680600LL);  mouse_move(1287, 873);
   sleep_until_ns(4042627200LL);  mouse_move(1326, 867);
   sleep_until_ns(4050727000LL);  mouse_move(1361, 858);
   sleep_until_ns(4058636100LL);  mouse_move(1394, 844);
   sleep_until_ns(4066690700LL);  mouse_move(1424, 828);
   sleep_until_ns(4074641200LL);  mouse_move(1447, 808);
   sleep_until_ns(4082636900LL);  mouse_move(1465, 788);
   sleep_until_ns(4090627300LL);  mouse_move(1479, 766);
   sleep_until_ns(4098639600LL);  mouse_move(1487, 743);
   sleep_until_ns(4106630200LL);  mouse_move(1491, 714);
   sleep_until_ns(4114642300LL);  mouse_move(1491, 687);
   sleep_until_ns(4122636600LL);  mouse_move(1490, 659);
   sleep_until_ns(4130636200LL);  mouse_move(1482, 632);
   sleep_until_ns(4138634500LL);  mouse_move(1469, 603);
   sleep_until_ns(4146639200LL);  mouse_move(1448, 576);
   sleep_until_ns(4154651300LL);  mouse_move(1419, 550);
   sleep_until_ns(4162640400LL);  mouse_move(1375, 519);
   sleep_until_ns(4170628700LL);  mouse_move(1315, 492);
   sleep_until_ns(4178646500LL);  mouse_move(1253, 468);
   sleep_until_ns(4186643700LL);  mouse_move(1187, 454);
   sleep_until_ns(4194655000LL);  mouse_move(1127, 442);
   sleep_until_ns(4202638400LL);  mouse_move(1077, 437);
   sleep_until_ns(4210632500LL);  mouse_move(1040, 435);
   sleep_until_ns(4218685400LL);  mouse_move(1012, 436);
   sleep_until_ns(4226641000LL);  mouse_move(990, 439);
   sleep_until_ns(4234643700LL);  mouse_move(971, 443);
   sleep_until_ns(4242626100LL);  mouse_move(953, 450);
   sleep_until_ns(4250658600LL);  mouse_move(935, 459);
   sleep_until_ns(4258629600LL);  mouse_move(916, 470);
   sleep_until_ns(4266663000LL);  mouse_move(898, 483);
   sleep_until_ns(4274639600LL);  mouse_move(882, 499);
   sleep_until_ns(4282648900LL);  mouse_move(867, 515);
   sleep_until_ns(4290624800LL);  mouse_move(854, 534);
   sleep_until_ns(4298628400LL);  mouse_move(844, 550);
   sleep_until_ns(4306630300LL);  mouse_move(835, 568);
   sleep_until_ns(4314645100LL);  mouse_move(828, 584);
   sleep_until_ns(4322617700LL);  mouse_move(820, 602);
   sleep_until_ns(4330803900LL);  mouse_move(815, 619);
   sleep_until_ns(4338630200LL);  mouse_move(812, 635);
   sleep_until_ns(4346640300LL);  mouse_move(808, 650);
   sleep_until_ns(4354629000LL);  mouse_move(805, 664);
   sleep_until_ns(4362641600LL);  mouse_move(803, 677);
   sleep_until_ns(4370636700LL);  mouse_move(802, 688);
   sleep_until_ns(4378629900LL);  mouse_move(802, 699);
   sleep_until_ns(4386648600LL);  mouse_move(802, 710);
   sleep_until_ns(4394625100LL);  mouse_move(802, 719);
   sleep_until_ns(4402632600LL);  mouse_move(805, 729);
   sleep_until_ns(4410629800LL);  mouse_move(807, 738);
   sleep_until_ns(4418687900LL);  mouse_move(810, 745);
   sleep_until_ns(4426641000LL);  mouse_move(813, 752);
   sleep_until_ns(4434651100LL);  mouse_move(816, 760);
   sleep_until_ns(4442615900LL);  mouse_move(820, 767);
   sleep_until_ns(4450666600LL);  mouse_move(825, 775);
   sleep_until_ns(4458623000LL);  mouse_move(829, 783);
   sleep_until_ns(4466678900LL);  mouse_move(834, 790);
   sleep_until_ns(4474631800LL);  mouse_move(838, 796);
   sleep_until_ns(4482644800LL);  mouse_move(842, 801);
   sleep_until_ns(4490625100LL);  mouse_move(847, 807);
   sleep_until_ns(4498622900LL);  mouse_move(850, 811);
   sleep_until_ns(4506624600LL);  mouse_move(853, 816);
   sleep_until_ns(4514640900LL);  mouse_move(855, 820);
   sleep_until_ns(4522635500LL);  mouse_move(858, 825);
   sleep_until_ns(4530631500LL);  mouse_move(860, 829);
   sleep_until_ns(4538652600LL);  mouse_move(863, 832);
   sleep_until_ns(4546647500LL);  mouse_move(865, 838);
   sleep_until_ns(4554641600LL);  mouse_move(868, 843);
   sleep_until_ns(4562651000LL);  mouse_move(869, 848);
   sleep_until_ns(4570632100LL);  mouse_move(872, 855);
   sleep_until_ns(4578630400LL);  mouse_move(875, 863);
   sleep_until_ns(4586656300LL);  mouse_move(877, 868);
   sleep_until_ns(4594631600LL);  mouse_move(878, 871);
   sleep_until_ns(4602717000LL);  mouse_move(879, 876);
   sleep_until_ns(4610643600LL);  mouse_move(880, 880);
   sleep_until_ns(4618804000LL);  mouse_move(881, 883);
   sleep_until_ns(4626650700LL);  mouse_move(883, 890);
   sleep_until_ns(4634677000LL);  mouse_move(886, 897);
   sleep_until_ns(4642652700LL);  mouse_move(889, 905);
   sleep_until_ns(4650689100LL);  mouse_move(891, 912);
   sleep_until_ns(4658631800LL);  mouse_move(896, 922);
   sleep_until_ns(4666681300LL);  mouse_move(899, 929);
   sleep_until_ns(4674638200LL);  mouse_move(902, 934);
   sleep_until_ns(4682633900LL);  mouse_move(905, 940);
   sleep_until_ns(4690667200LL);  mouse_move(906, 945);
   sleep_until_ns(4698662100LL);  mouse_move(907, 948);
   sleep_until_ns(4706636600LL);  mouse_move(909, 952);
   sleep_until_ns(4714643400LL);  mouse_move(911, 956);
   sleep_until_ns(4722636500LL);  mouse_move(912, 959);
   sleep_until_ns(4730641000LL);  mouse_move(913, 963);
   sleep_until_ns(4738632800LL);  mouse_move(914, 966);
   sleep_until_ns(4746633400LL);  mouse_move(915, 970);
   sleep_until_ns(4754643400LL);  mouse_move(917, 972);
   sleep_until_ns(4762633000LL);  mouse_move(918, 974);
   sleep_until_ns(4770625400LL);  mouse_move(918, 976);
   sleep_until_ns(4778642900LL);  mouse_move(918, 977);
   sleep_until_ns(4786660100LL);  mouse_move(918, 978);
   sleep_until_ns(4794631100LL);  mouse_move(918, 979);
   sleep_until_ns(4802892300LL);  mouse_move(918, 980);
   sleep_until_ns(4810634600LL);  mouse_move(918, 981);
   sleep_until_ns(4826644200LL);  mouse_move(918, 982);
   sleep_until_ns(4842640600LL);  mouse_move(918, 983);
   sleep_until_ns(5002702700LL);  mouse_move(918, 984);
   sleep_until_ns(5082897200LL);  mouse_click_down(100);
   sleep_until_ns(5202812100LL);  mouse_click_up(100);
   sleep_until_ns(5938869300LL);  mouse_click_down(100);
   sleep_until_ns(6042812800LL);  mouse_click_up(100);
   sleep_until_ns(6074816400LL);  mouse_move(920, 984);
   sleep_until_ns(6082651200LL);  mouse_move(922, 984);
   sleep_until_ns(6090621400LL);  mouse_move(924, 984);
   sleep_until_ns(6098619100LL);  mouse_move(926, 984);
   sleep_until_ns(6106639000LL);  mouse_move(928, 984);
   sleep_until_ns(6114945600LL);  mouse_move(930, 984);
   sleep_until_ns(6122616800LL);  mouse_move(932, 984);
   sleep_until_ns(6130645500LL);  mouse_move(934, 984);
   sleep_until_ns(6138631600LL);  mouse_move(936, 984);
   sleep_until_ns(6146639200LL);  mouse_move(938, 984);
   sleep_until_ns(6154639100LL);  mouse_move(941, 984);
   sleep_until_ns(6162630200LL);  mouse_move(943, 984);
   sleep_until_ns(6170632000LL);  mouse_move(945, 984);
   sleep_until_ns(6178639500LL);  mouse_move(947, 984);
   sleep_until_ns(6186646500LL);  mouse_move(949, 984);
   sleep_until_ns(6194635900LL);  mouse_move(951, 984);
   sleep_until_ns(6202667300LL);  mouse_move(952, 984);
   sleep_until_ns(6210625600LL);  mouse_move(954, 984);
   sleep_until_ns(6218798700LL);  mouse_move(955, 984);
   sleep_until_ns(6226648500LL);  mouse_move(957, 984);
   sleep_until_ns(6234679300LL);  mouse_move(958, 984);
   sleep_until_ns(6242616300LL);  mouse_move(960, 985);
   sleep_until_ns(6250686700LL);  mouse_move(961, 985);
   sleep_until_ns(6258625000LL);  mouse_move(963, 985);
   sleep_until_ns(6266680200LL);  mouse_move(963, 985);
   sleep_until_ns(6274631500LL);  mouse_move(964, 985);
   sleep_until_ns(6282662900LL);  mouse_move(965, 986);
   sleep_until_ns(6290619300LL);  mouse_move(966, 986);
   sleep_until_ns(6306639300LL);  mouse_move(967, 987);
   sleep_until_ns(6322649300LL);  mouse_move(968, 987);
   sleep_until_ns(6330637100LL);  mouse_move(969, 987);
   sleep_until_ns(6338618700LL);  mouse_move(969, 988);
   sleep_until_ns(6346645300LL);  mouse_move(970, 988);
   sleep_until_ns(6386685400LL);  mouse_move(971, 988);
   sleep_until_ns(6394628800LL);  mouse_move(972, 988);
   sleep_until_ns(6410650700LL);  mouse_move(973, 989);
   sleep_until_ns(6418653000LL);  mouse_move(974, 989);
   sleep_until_ns(6434725300LL);  mouse_move(975, 989);
   sleep_until_ns(6442627600LL);  mouse_move(975, 990);
   sleep_until_ns(6450673600LL);  mouse_move(975, 990);
   sleep_until_ns(6458621600LL);  mouse_move(976, 990);
   sleep_until_ns(6466678500LL);  mouse_move(978, 992);
   sleep_until_ns(6474634100LL);  mouse_move(982, 993);
   sleep_until_ns(6482619100LL);  mouse_move(984, 994);
   sleep_until_ns(6490627000LL);  mouse_move(985, 995);
   sleep_until_ns(6498634800LL);  mouse_move(986, 995);
   sleep_until_ns(6506628200LL);  mouse_move(987, 995);
   sleep_until_ns(6506936900LL);  mouse_click_down(100);
   sleep_until_ns(6538644100LL);  mouse_move(988, 995);
   sleep_until_ns(6570647700LL);  mouse_move(988, 996);
   sleep_until_ns(6642868100LL);  mouse_click_up(100);
   sleep_until_ns(6778851000LL);  mouse_click_down(100);
   sleep_until_ns(6858646400LL);  mouse_move(989, 996);
   sleep_until_ns(6866713400LL);  mouse_move(990, 996);
   sleep_until_ns(6867162400LL);  mouse_click_up(100);
   sleep_until_ns(6874629700LL);  mouse_move(993, 996);
   sleep_until_ns(6882654100LL);  mouse_move(998, 996);
   sleep_until_ns(6890619500LL);  mouse_move(1004, 996);
   sleep_until_ns(6898647800LL);  mouse_move(1011, 996);
   sleep_until_ns(6906622300LL);  mouse_move(1018, 996);
   sleep_until_ns(6915015400LL);  mouse_move(1027, 996);
   sleep_until_ns(6922625600LL);  mouse_move(1035, 996);
   sleep_until_ns(6930626800LL);  mouse_move(1044, 998);
   sleep_until_ns(6938618100LL);  mouse_move(1052, 999);
   sleep_until_ns(6946644100LL);  mouse_move(1057, 1001);
   sleep_until_ns(6954635800LL);  mouse_move(1060, 1001);
   sleep_until_ns(6962618900LL);  mouse_move(1061, 1001);
   sleep_until_ns(6970621600LL);  mouse_move(1062, 1001);
   sleep_until_ns(6978621800LL);  mouse_move(1063, 1001);
   sleep_until_ns(6986651400LL);  mouse_move(1064, 1001);
   sleep_until_ns(7098673400LL);  mouse_move(1065, 1001);
   sleep_until_ns(7106760000LL);  mouse_click_down(100);
   sleep_until_ns(7202879300LL);  mouse_click_up(100);
   sleep_until_ns(7338860500LL);  mouse_click_down(100);
   sleep_until_ns(7442691100LL);  mouse_move(1066, 1002);
   sleep_until_ns(7443074500LL);  mouse_click_up(100);
   sleep_until_ns(7450704500LL);  mouse_move(1066, 1002);
   sleep_until_ns(7458628600LL);  mouse_move(1068, 1002);
   sleep_until_ns(7466661900LL);  mouse_move(1072, 1002);
   sleep_until_ns(7474634300LL);  mouse_move(1075, 1002);
   sleep_until_ns(7482637200LL);  mouse_move(1077, 1002);
   sleep_until_ns(7490622200LL);  mouse_move(1081, 1002);
   sleep_until_ns(7498619600LL);  mouse_move(1083, 1002);
   sleep_until_ns(7506621000LL);  mouse_move(1086, 1002);
   sleep_until_ns(7514876900LL);  mouse_move(1089, 1002);
   sleep_until_ns(7522626500LL);  mouse_move(1091, 1002);
   sleep_until_ns(7530971900LL);  mouse_move(1092, 1002);
   sleep_until_ns(7538631200LL);  mouse_move(1092, 1002);
   sleep_until_ns(7546640100LL);  mouse_move(1093, 1002);
   sleep_until_ns(7554644800LL);  mouse_move(1095, 1002);
   sleep_until_ns(7562618700LL);  mouse_move(1096, 1002);
   sleep_until_ns(7570635000LL);  mouse_move(1098, 1002);
   sleep_until_ns(7578632500LL);  mouse_move(1100, 1002);
   sleep_until_ns(7586653100LL);  mouse_move(1102, 1002);
   sleep_until_ns(7594919900LL);  mouse_move(1105, 1002);
   sleep_until_ns(7602665200LL);  mouse_move(1107, 1002);
   sleep_until_ns(7610630500LL);  mouse_move(1108, 1002);
   sleep_until_ns(7618692400LL);  mouse_move(1109, 1002);
   sleep_until_ns(7626636300LL);  mouse_move(1111, 1001);
   sleep_until_ns(7634670900LL);  mouse_move(1112, 1001);
   sleep_until_ns(7642637600LL);  mouse_move(1112, 1001);
   sleep_until_ns(7706854600LL);  mouse_click_down(100);
   sleep_until_ns(7802863500LL);  mouse_click_up(100);
   sleep_until_ns(7994820900LL);  mouse_click_down(100);
   sleep_until_ns(8098854300LL);  mouse_click_up(100);
   sleep_until_ns(8210687000LL);  mouse_move(1113, 1001);
   sleep_until_ns(8218701100LL);  mouse_move(1114, 1001);
   sleep_until_ns(8226624400LL);  mouse_move(1115, 1001);
   sleep_until_ns(8234668500LL);  mouse_move(1116, 1001);
   sleep_until_ns(8242616800LL);  mouse_move(1118, 1001);
   sleep_until_ns(8250695000LL);  mouse_move(1120, 1001);
   sleep_until_ns(8258623200LL);  mouse_move(1121, 1001);
   sleep_until_ns(8266674400LL);  mouse_move(1122, 1001);
   sleep_until_ns(8274638400LL);  mouse_move(1123, 1001);
   sleep_until_ns(8282627700LL);  mouse_move(1125, 1001);
   sleep_until_ns(8290619800LL);  mouse_move(1127, 1001);
   sleep_until_ns(8298622300LL);  mouse_move(1129, 1001);
   sleep_until_ns(8306626400LL);  mouse_move(1131, 1001);
   sleep_until_ns(8314628600LL);  mouse_move(1133, 1001);
   sleep_until_ns(8322651700LL);  mouse_move(1134, 1001);
   sleep_until_ns(8330637300LL);  mouse_move(1135, 1001);
   sleep_until_ns(8338626400LL);  mouse_move(1136, 1001);
   sleep_until_ns(8346627000LL);  mouse_move(1138, 1001);
   sleep_until_ns(8354652300LL);  mouse_move(1139, 1001);
   sleep_until_ns(8362626400LL);  mouse_move(1141, 1001);
   sleep_until_ns(8370624500LL);  mouse_move(1143, 1001);
   sleep_until_ns(8378614600LL);  mouse_move(1144, 1001);
   sleep_until_ns(8386663000LL);  mouse_move(1146, 1001);
   sleep_until_ns(8394629600LL);  mouse_move(1148, 1001);
   sleep_until_ns(8402620300LL);  mouse_move(1149, 1001);
   sleep_until_ns(8410619000LL);  mouse_move(1151, 1001);
   sleep_until_ns(8418653100LL);  mouse_move(1152, 1001);
   sleep_until_ns(8426636600LL);  mouse_move(1154, 1001);
   sleep_until_ns(8434642400LL);  mouse_move(1155, 1002);
   sleep_until_ns(8442617400LL);  mouse_move(1156, 1002);
   sleep_until_ns(8450658900LL);  mouse_move(1156, 1002);
   sleep_until_ns(8458618800LL);  mouse_move(1157, 1002);
   sleep_until_ns(8466650400LL);  mouse_move(1158, 1002);
   sleep_until_ns(8474621500LL);  mouse_move(1159, 1002);
   sleep_until_ns(8490637200LL);  mouse_move(1160, 1002);
   sleep_until_ns(8498636800LL);  mouse_move(1161, 1002);
   sleep_until_ns(8514640700LL);  mouse_move(1162, 1002);
   sleep_until_ns(8530684900LL);  mouse_move(1163, 1002);
   sleep_until_ns(8538623600LL);  mouse_move(1164, 1002);
   sleep_until_ns(8554642000LL);  mouse_move(1165, 1002);
   sleep_until_ns(8570640600LL);  mouse_move(1166, 1002);
   sleep_until_ns(8594646100LL);  mouse_move(1167, 1002);
   sleep_until_ns(8658648400LL);  mouse_move(1168, 1002);
   sleep_until_ns(8682691300LL);  mouse_move(1168, 1002);
   sleep_until_ns(8714688100LL);  mouse_move(1169, 1002);
   sleep_until_ns(8730790800LL);  mouse_click_down(100);
   sleep_until_ns(8858840100LL);  mouse_click_up(100);
   sleep_until_ns(9170678100LL);  mouse_move(1170, 1002);
   sleep_until_ns(9178650700LL);  mouse_move(1171, 1002);
   sleep_until_ns(9186679700LL);  mouse_move(1172, 1002);
   sleep_until_ns(9194638900LL);  mouse_move(1173, 1002);
   sleep_until_ns(9242663600LL);  mouse_move(1174, 1002);
   sleep_until_ns(9274638900LL);  mouse_move(1175, 1002);
   sleep_until_ns(9490660100LL);  mouse_move(1175, 1002);
   sleep_until_ns(9498629900LL);  mouse_move(1175, 1001);
   sleep_until_ns(9522643000LL);  mouse_move(1175, 1001);
   sleep_until_ns(9530721000LL);  mouse_move(1175, 1002);
   sleep_until_ns(9538634000LL);  mouse_move(1175, 1003);
   sleep_until_ns(9546635600LL);  mouse_move(1176, 1004);
   sleep_until_ns(9586679900LL);  mouse_move(1176, 1005);

        return 0;
    }
    