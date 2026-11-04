// Buildable battery: unlocks the game's own unused "Battery bank" structure (kind 6) in the build menu
// and connects it to the solar power grid as storage.
// Included from the main source after the IL2CPP helpers.
//
// How it works:
//  * MoonWorld.Update is hooked so that Unity/IL2CPP work runs on the main thread.
//  * A CatalogEntry for kind 6 is appended to MoonDepotBuilder.baseStructures (own name and card image).
//  * The power solver treats kind 6 as a zero-output panel with a large battery (TryPanel, IsPowerNode,
//    IsSolarPanel and two SolvePower checks are patched).
//  * Each frame the grid's wasted solar surplus is moved into its battery banks.
#pragma once
#include <vector>
#include <map>

static const int kSolarPostKind = 93;
static const int kMaxStructureKind = 192;           // MoonWorld.MaxStructureKind
static const uintptr_t kWorldUpdateRva = 0x769090;  // MoonWorld.Update()
static int g_batteryKind = -1;
static void* g_batteryTitle;   // rooted string used as PanelSpec.name (the info panel's title)
static void* g_batterySub;     // rooted, localized info panel subtitle (set by UpdateLanguage)
static void* g_emptyStr;       // rooted ""

// ---------------- reflection helpers ----------------
static std::string TypeNameOf(void* type) {
    char* n = il2cpp_type_get_name(type);
    std::string s = n ? n : "";
    if (n) il2cpp_free(n);
    return s;
}
// Find a method by name/argc and (optionally) the full type names of its parameters, walking base classes.
static void* MethodBy(void* klass, const char* name, int argc, std::initializer_list<const char*> params = {}) {
    for (void* k = klass; k; k = il2cpp_class_get_parent(k)) {
        void* it = nullptr;
        while (void* m = il2cpp_class_get_methods(k, &it)) {
            if (strcmp(il2cpp_method_get_name(m), name) || (int)il2cpp_method_get_param_count(m) != argc) continue;
            bool ok = true; uint32_t i = 0;
            for (const char* p : params) {
                if (p && TypeNameOf(il2cpp_method_get_param(m, i)) != p) { ok = false; break; }
                i++;
            }
            if (ok) return m;
        }
    }
    return nullptr;
}
static void* CallM(void* method, void* self, void** args, const char* what) {
    if (!method) { Log("HATA: metod bulunamadi: %s", what); return nullptr; }
    void* exc = nullptr;
    void* r = il2cpp_runtime_invoke(method, self, args, &exc);
    if (exc) { Log("HATA: %s istisna atti", what); return nullptr; }
    return r;
}
static void* TypeObj(void* klass) { return il2cpp_type_get_object(il2cpp_class_get_type(klass)); }
static void* ValueArray(void* elemClass, size_t count, const void* data, size_t elemSize) {
    void* a = il2cpp_array_new(elemClass, count);
    memcpy(ArrPtrs(a), data, count * elemSize);
    return a;
}

// ---------------- definition + catalog ----------------
static bool g_batteryDone = false;

// ---------------- unlock the game's own "Battery bank" (kind 6) ----------------
static const int kBatteryBankKind = 6;

static __declspec(noinline) void BatteryInit(void* world) {
    g_batteryDone = true;
    int k = kBatteryBankKind; void* ka[] = {&k};
    void* def = CallM(MethodBy(g_worldK, "BuildDefinition", 1), world, ka, "BuildDefinition(6)");
    if (!def) { Log("HATA: Battery bank tanimi yok"); return; }
    void* prefab = At<void*>(def, 0x18);
    std::string prefabName = prefab ? Str(CallM(MethodBy(FindClass("UnityEngine", "Object"), "get_name", 0), prefab, nullptr, "get_name")) : "yok";
    Log("akü: tur 6 '%s' fiyat %d, prefab '%s', ayak izi %.2f x %.2f, yukseklik %.2f, modul %p, ek tanim %zu",
        Str(At<void*>(def, 0x68)).c_str(), At<int>(def, 0x28), prefabName.c_str(), At<float>(def, 0x30) * 2,
        At<float>(def, 0x34) * 2, At<float>(def, 0x3c), At<void*>(def, 0x80), ArrLen(At<void*>(def, 0x70)));
    if (!prefab) { Log("HATA: Battery bank prefab'i yok, eklenmedi"); return; }
    if (g_batteryPrice > 0) At<int>(def, 0x28) = g_batteryPrice;
    if (void* title = At<void*>(def, 0x68)) { il2cpp_gchandle_new(title, false); g_batteryTitle = title; }
    if (void* f = il2cpp_class_get_field_from_name(g_worldK, "PostPanel")) {
        uint8_t spec[0x20] = {}; il2cpp_field_static_get_value(f, spec);
        Log("akü: gunes diregi: %.2f kW uretim, %.2f kWh aku, menzil %.1f m", *(float*)(spec + 8), *(float*)(spec + 0xC), *(float*)(spec + 0x10));
    }

    void* builderK = FindClass("MoonCleaners", "MoonDepotBuilder");
    void* entryK = builderK ? Nested(builderK, "CatalogEntry") : nullptr;
    if (!entryK) { Log("HATA: katalog sinifi yok"); return; }
    il2cpp_runtime_class_init(builderK);

    void* da[] = {def};
    void* avail = CallM(MethodBy(builderK, "Available", 1), nullptr, da, "Available");
    Log("akü: oyunun Available() sonucu: %s", avail ? (At<bool>(avail, 0x10) ? "evet" : "hayir") : "?");

    void* entries = GetStatic(builderK, "baseStructures");
    size_t en = ArrLen(entries);
    void* solarEntry = nullptr; bool present = false;
    std::string list;
    for (size_t i = 0; i < en; i++) {
        void* e = ArrPtrs(entries)[i];
        int kind = At<int>(e, 0x10);
        list += std::to_string(kind) + ":" + Str(At<void*>(e, 0x18)) + "/" + Str(At<void*>(e, 0x20)) + "/" + Str(At<void*>(e, 0x28)) + " ";
        if (kind == kSolarPostKind) solarEntry = e;
        if (kind == kBatteryBankKind) present = true;
    }
    Log("katalog: %s", list.c_str());
    if (present) { Log("akü: Battery bank zaten katalogda"); g_batteryKind = kBatteryBankKind; return; }

    void* entry = il2cpp_object_new(entryK);
    il2cpp_gchandle_new(entry, false);
    int kind = kBatteryBankKind;
    // card art key = the structure's resource name (Assets/Resources/Moon/Structures/battery_bank.asset)
    void* ea[] = {&kind, At<void*>(def, 0x68),
                  solarEntry ? At<void*>(solarEntry, 0x20) : NewStr("Power"), NewStr("battery_bank")};
    CallM(MethodBy(entryK, ".ctor", 4), entry, ea, "CatalogEntry.ctor");
    void* grownE = il2cpp_array_new(entryK, en + 1);
    for (size_t i = 0; i < en; i++) SetRef(grownE, 0x20 + i * 8, ArrPtrs(entries)[i]);
    SetRef(grownE, 0x20 + en * 8, entry);
    il2cpp_gchandle_new(grownE, false);
    il2cpp_field_static_set_value(il2cpp_class_get_field_from_name(builderK, "baseStructures"), grownE);
    g_batteryKind = kBatteryBankKind;
    Log("akü: Battery bank katalog'a eklendi (%zu -> %zu)", en, en + 1);
}

// ---------------- power: make kind 6 a zero-output "panel" with a big battery ----------------
// MoonWorld.SolvePower (1) keeps a structure as a power node if 93<=kind<=96 || kind==97,
// (2) flags it as a panel if 93<=kind<=95 and (3) calls TryPanel(kind, &spec) for panels.
// The two inlined checks are redirected to small stubs that also accept kind 6, and TryPanel(6)
// returns the solar post's spec with peakKw = 0 and batteryKwh = g_batteryKwh.
static const uintptr_t kFilterRva = 0x7450B8, kFilterAccept = 0x7450C2, kFilterSkip = 0x745030;
static const uintptr_t kPanelFlagRva = 0x745326, kPanelFlagDone = 0x745337;
static const uintptr_t kTryPanelRva = 0x75F8D0;
static uint8_t* g_cave; static size_t g_caveAt;
static float g_postBatteryKwh = 0;

static uint8_t* AllocNearExec(uintptr_t base) {
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t gran = si.dwAllocationGranularity;
    for (uintptr_t d = 0x10000000; d < 0x70000000; d += gran)
        for (int sign = -1; sign <= 1; sign += 2) {
            uintptr_t a = (base + (sign < 0 ? -(intptr_t)d : d)) & ~(gran - 1);
            if (void* p = VirtualAlloc((void*)a, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE)) return (uint8_t*)p;
        }
    return nullptr;
}
static uint8_t* CaveHere() { return g_cave + g_caveAt; }
static void Emit(std::initializer_list<uint8_t> bytes) { for (uint8_t b : bytes) g_cave[g_caveAt++] = b; }
static void EmitRel32(uint8_t op1, int op2, uintptr_t targetRva) {   // jmp/jcc rel32 to game code
    g_cave[g_caveAt++] = op1;
    if (op2 >= 0) g_cave[g_caveAt++] = (uint8_t)op2;
    int32_t rel = (int32_t)((intptr_t)(g_base + targetRva) - (intptr_t)(CaveHere() + 4));
    memcpy(CaveHere(), &rel, 4); g_caveAt += 4;
}
static void EmitAbsJmp(void* target) { Emit({0xFF, 0x25, 0, 0, 0, 0}); memcpy(CaveHere(), &target, 8); g_caveAt += 8; }

// overwrite `len` bytes at rva with "jmp stub" + nops, after checking the original bytes
static bool JumpToStub(uintptr_t rva, const uint8_t* expect, size_t len, uint8_t* stub, const char* name) {
    uint8_t* at = (uint8_t*)(g_base + rva);
    if (memcmp(at, expect, len)) { Log("ATLANDI %s: beklenen kod degil", name); return false; }
    uint8_t patch[32]; patch[0] = 0xE9;
    int32_t rel = (int32_t)((intptr_t)stub - (intptr_t)(at + 5)); memcpy(patch + 1, &rel, 4);
    for (size_t i = 5; i < len; i++) patch[i] = 0x90;
    DWORD old; VirtualProtect(at, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(at, patch, len);
    VirtualProtect(at, len, old, &old); FlushInstructionCache(GetCurrentProcess(), at, len);
    return true;
}

typedef bool (*TryPanel_t)(int32_t kind, void* spec, const void* method);
static TryPanel_t g_tryPanelOrig;
// g_batteryTitle declared at the top of the file
static bool TryPanelHook(int32_t kind, void* spec, const void* method) {
    if (kind != kBatteryBankKind) return g_tryPanelOrig(kind, spec, method);
    bool ok = g_tryPanelOrig(kSolarPostKind, spec, method);   // copies PostPanel (name, range...) with proper barriers
    if (ok) {
        // name: the string is pinned by a GC handle, so a plain store into the struct is safe
        if (g_batteryTitle) *(void**)spec = g_batteryTitle;
        At<float>(spec, 0x08) = 0.f;            // peakKw: stores, never generates
        At<float>(spec, 0x0C) = g_batteryKwh;   // batteryKwh
        At<bool>(spec, 0x1C) = false;           // tracker
    }
    return ok;
}

// ---------------- charging: route each grid's wasted surplus into its battery banks ----------------
// The game charges a panel's battery only from that panel's own surplus (output - supplied), and the
// part that does not fit is wasted. After the game's SolvePower ran this frame, that wasted energy is
// summed per grid and given to the grid's battery banks. Discharging already pools all batteries.
// PowerNode: record @0x10 (DepotSnapshot, kind @0x14), state @0x18, panel @0x50.
// MoonProductionState: Grid @0x4c, UsedKw (output) @0x30, ChargeKwh @0x50 (double), SuppliedKw @0x58.
static std::unordered_map<void*, double> g_prevCharge;

static __declspec(noinline) void ShareSurplus(void* world) {
    void* list = At<void*>(world, 0x8c0);
    if (!list) return;
    void* items = At<void*>(list, 0x10); int count = At<int>(list, 0x18);
    if (!items || count <= 0) return;
    // frame time from the Windows clock (no managed call / allocation every frame)
    static LARGE_INTEGER freq, last;
    LARGE_INTEGER nowT; QueryPerformanceCounter(&nowT);
    if (!freq.QuadPart) { QueryPerformanceFrequency(&freq); last = nowT; return; }
    float dt = (float)((double)(nowT.QuadPart - last.QuadPart) / (double)freq.QuadPart);
    last = nowT;
    if (!(dt > 0.f) || dt > 1.f) return;
    double hours = dt / 3600.0;

    std::map<int, double> wasted;
    std::map<int, std::vector<void*>> banks;
    for (int i = 0; i < count && (size_t)i < ArrLen(items); i++) {
        void* node = ArrPtrs(items)[i];
        if (!node) continue;
        void* rec = At<void*>(node, 0x10); void* st = At<void*>(node, 0x18);
        if (!rec || !st) continue;
        int grid = At<int>(st, 0x4c);
        if (At<int>(rec, 0x14) == kBatteryBankKind) { banks[grid].push_back(st); continue; }
        if (!At<bool>(node, 0x50)) continue;   // panels only
        double charge = At<double>(st, 0x50);
        auto it = g_prevCharge.find(st);
        double stored = it == g_prevCharge.end() ? 0.0 : std::max(0.0, charge - it->second);
        g_prevCharge[st] = charge;
        double surplus = std::max(0.f, At<float>(st, 0x30) - At<float>(st, 0x58)) * hours * g_batteryRate;
        wasted[grid] += std::max(0.0, surplus - stored);
    }
    for (auto& [grid, list2] : banks) {
        double pool = wasted[grid];
        for (size_t k = 0; k < list2.size() && pool > 1e-9; k++) {
            void* st = list2[k];
            double room = g_batteryKwh - At<double>(st, 0x50);
            double give = std::min(room, pool / (double)(list2.size() - k));
            if (give > 0) { At<double>(st, 0x50) += give; pool -= give; }
        }
    }
}

static void InstallBatteryPower() {
    g_cave = AllocNearExec(g_base);
    if (!g_cave) { Log("HATA: akü kod alani ayrilamadi"); return; }

    // (C) TryPanel: 5-byte jmp -> abs jmp stub -> TryPanelHook; trampoline runs the moved instruction
    static const uint8_t tpExpect[5] = {0x48, 0x89, 0x5C, 0x24, 0x08};   // mov [rsp+8],rbx
    uint8_t* tramp = CaveHere();
    Emit({0x48, 0x89, 0x5C, 0x24, 0x08});
    EmitAbsJmp((void*)(g_base + kTryPanelRva + 5));
    uint8_t* hookStub = CaveHere();
    EmitAbsJmp((void*)&TryPanelHook);
    g_tryPanelOrig = (TryPanel_t)tramp;
    if (!JumpToStub(kTryPanelRva, tpExpect, 5, hookStub, "TryPanel")) return;

    // (A) node filter: ... || kind == 97 || kind == 6
    static const uint8_t fExpect[10] = {0x41, 0x83, 0xFE, 0x61, 0x0F, 0x85, 0x6E, 0xFF, 0xFF, 0xFF};
    uint8_t* fStub = CaveHere();
    Emit({0x41, 0x83, 0xFE, 0x61}); EmitRel32(0x0F, 0x84, kFilterAccept);   // cmp r14d,97 ; je accept
    Emit({0x41, 0x83, 0xFE, 0x06}); EmitRel32(0x0F, 0x84, kFilterAccept);   // cmp r14d,6  ; je accept
    EmitRel32(0xE9, -1, kFilterSkip);                                         // jmp skip
    bool a = JumpToStub(kFilterRva, fExpect, 10, fStub, "guc dugumu filtresi");

    // (B) panel flag: al = (93 <= kind <= 95) || kind == 6
    static const uint8_t pExpect[17] = {0x41, 0x83, 0xFC, 0x5D, 0x7D, 0x04, 0x32, 0xC0, 0xEB, 0x07, 0x41, 0x83, 0xFC, 0x5F, 0x0F, 0x9E, 0xC0};
    uint8_t* pStub = CaveHere();
    Emit({0x41, 0x83, 0xFC, 0x06, 0x0F, 0x94, 0xC0});   // cmp r12d,6 ; sete al
    Emit({0x41, 0x83, 0xFC, 0x5D, 0x7C, 0x08});         // cmp r12d,93 ; jl done
    Emit({0x41, 0x83, 0xFC, 0x5F, 0x7F, 0x02});         // cmp r12d,95 ; jg done
    Emit({0xB0, 0x01});                                 // mov al,1
    EmitRel32(0xE9, -1, kPanelFlagDone);                // done: jmp back
    bool b = JumpToStub(kPanelFlagRva, pExpect, 17, pStub, "panel bayragi");

    // (D) IsSolarPanel(kind): tiny pure function, replaced with "kind == 6 || 93 <= kind <= 95".
    // ProductionOf(id) only creates the per-structure power state when IsPowerNode(kind) is true, and
    // SolvePower dereferences that state - so the real (non-inlined) helpers must accept kind 6 too.
    static const uint8_t spExpect[15] = {0x83, 0xF9, 0x5D, 0x7D, 0x03, 0x32, 0xC0, 0xC3, 0x83, 0xF9, 0x5F, 0x0F, 0x9E, 0xC0, 0xC3};
    uint8_t* spStub = CaveHere();
    // offsets: [0]cmp6 [3]je [5]cmp93 [8]jl [10]cmp95 [13]jle [15]no: [18]yes:
    Emit({0x83, 0xF9, 0x06, 0x74, 0x0D});         // cmp ecx,6 ; je yes (+13 -> 18)
    Emit({0x83, 0xF9, 0x5D, 0x7C, 0x05});         // cmp ecx,93 ; jl no (+5 -> 15)
    Emit({0x83, 0xF9, 0x5F, 0x7E, 0x03});         // cmp ecx,95 ; jle yes
    Emit({0x32, 0xC0, 0xC3});                     // no: xor al,al ; ret
    Emit({0xB0, 0x01, 0xC3});                     // yes: mov al,1 ; ret
    bool d = JumpToStub(0x6EF730, spExpect, 15, spStub, "IsSolarPanel");

    // (E) IsPowerNode(kind): "kind == 6" fast path, otherwise the original (prologue moved to a trampoline)
    static const uint8_t pnExpect[6] = {0x40, 0x53, 0x48, 0x83, 0xEC, 0x20};   // push rbx ; sub rsp,20h
    uint8_t* pnTramp = CaveHere();
    Emit({0x40, 0x53, 0x48, 0x83, 0xEC, 0x20});
    EmitAbsJmp((void*)(g_base + 0x6EF4A0 + 6));
    uint8_t* pnStub = CaveHere();
    Emit({0x83, 0xF9, 0x06, 0x75, 0x03, 0xB0, 0x01, 0xC3});   // cmp ecx,6 ; jne orig ; mov al,1 ; ret
    Emit({0xFF, 0x25, 0, 0, 0, 0}); memcpy(CaveHere(), &pnTramp, 8); g_caveAt += 8;
    bool e = JumpToStub(0x6EF4A0, pnExpect, 6, pnStub, "IsPowerNode");

    // (F) info panel title: no size suffix " (S)/(M)/(L)" for the battery bank (spec.name at [rbp-68h])
    g_emptyStr = NewStr("");
    static const uint8_t tExpect[5] = {0x41, 0x83, 0x7E, 0x14, 0x5D};
    uint8_t* tStub = CaveHere();
    { void* a1 = &g_batteryTitle; void* a2 = &g_emptyStr;
      Emit({0x48, 0xBA}); memcpy(CaveHere(), &a1, 8); g_caveAt += 8;   // mov rdx, &g_batteryTitle
      Emit({0x48, 0x8B, 0x12, 0x48, 0x85, 0xD2, 0x74, 24});             // mov rdx,[rdx]; test; jz orig
      Emit({0x48, 0x39, 0x55, 0x98, 0x75, 18});                         // cmp [rbp-68h],rdx; jne orig
      Emit({0x48, 0xBA}); memcpy(CaveHere(), &a2, 8); g_caveAt += 8;   // mov rdx, &g_emptyStr
      Emit({0x48, 0x8B, 0x12});                                         // mov rdx,[rdx]
      EmitRel32(0xE9, -1, 0x49C4A8);                                    // -> Concat(name, "")
      Emit({0x41, 0x83, 0x7E, 0x14, 0x5D});                             // orig: cmp [r14+14h],93
      EmitRel32(0x0F, 0x85, 0x49C494);
      EmitRel32(0xE9, -1, 0x49C48B); }
    bool f = JumpToStub(0x49C484, tExpect, 5, tStub, "panel basligi");

    // (G) info panel subtitle: the battery bank gets its own line instead of the solar hint
    static const uint8_t sExpect[7] = {0x48, 0x8B, 0xD8, 0x80, 0x7F, 0x64, 0x00};
    uint8_t* sStub = CaveHere();
    { void* a1 = &g_batteryTitle; void* a2 = &g_batterySub;
      Emit({0x48, 0x8B, 0xD8});                                         // mov rbx,rax
      Emit({0x48, 0xB8}); memcpy(CaveHere(), &a1, 8); g_caveAt += 8;   // mov rax, &g_batteryTitle
      Emit({0x48, 0x8B, 0x00, 0x48, 0x85, 0xC0, 0x74, 27});             // mov rax,[rax]; test; jz skip
      Emit({0x48, 0x39, 0x45, 0x98, 0x75, 21});                         // cmp [rbp-68h],rax; jne skip
      Emit({0x48, 0xB8}); memcpy(CaveHere(), &a2, 8); g_caveAt += 8;   // mov rax, &g_batterySub
      Emit({0x48, 0x8B, 0x00, 0x48, 0x85, 0xC0, 0x74, 0x03});           // mov rax,[rax]; test; jz +3
      Emit({0x48, 0x8B, 0xD8});                                         // mov rbx,rax
      Emit({0x80, 0x7F, 0x64, 0x00});                                   // skip: cmp byte [rdi+64h],0
      EmitRel32(0xE9, -1, 0x49C629); }
    bool g = JumpToStub(0x49C622, sExpect, 7, sStub, "panel alt yazisi");

    FlushInstructionCache(GetCurrentProcess(), g_cave, g_caveAt);
    Log("akü panel yazilari: baslik=%d alt yazi=%d, sarj carpani %.1f", f, g, g_batteryRate);
    Log("akü enerji yamalari: TryPanel=1 filtre=%d panel=%d IsSolarPanel=%d IsPowerNode=%d, kapasite %.1f kWh", a, b, d, e, g_batteryKwh);
}

// ---------------- main-thread hook: MoonWorld.Update ----------------
static uint8_t* g_updateTramp;
typedef void (*WorldUpdate_t)(void* self, const void* method);
static void ModTick(void* world);   // main source: upgrade levels, Joonas, production, language
static __declspec(noinline) void SafeModTick(void* world) {
    __try { ModTick(world); }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("HATA: periyodik islerde istisna"); }
}
static void WorldUpdateHook(void* self, const void* method) {
    if (g_batteryEnabled && !g_batteryDone && self) {
        __try { BatteryInit(self); }
        __except (EXCEPTION_EXECUTE_HANDLER) { Log("HATA: akü kurulumunda istisna"); }
    }
    ((WorldUpdate_t)g_updateTramp)(self, method);
    if (!self) return;
    SafeModTick(self);
    static bool shareFailed = false;
    if (g_batteryEnabled && !shareFailed) {
        __try { ShareSurplus(self); }
        __except (EXCEPTION_EXECUTE_HANDLER) { shareFailed = true; Log("HATA: akü sarj paylasiminda istisna, kapatildi"); }
    }
}

// 14-byte absolute jump hook; `expect` = the exact prologue bytes being moved (no rip-relative code).
static uint8_t* InstallHook(uintptr_t rva, const uint8_t* expect, size_t len, void* detour, const char* name) {
    uint8_t* target = (uint8_t*)(g_base + rva);
    if (len < 14 || memcmp(target, expect, len)) { Log("ATLANDI %s kancasi: beklenen kod degil", name); return nullptr; }
    uint8_t* tramp = (uint8_t*)VirtualAlloc(nullptr, 64, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!tramp) return nullptr;
    memcpy(tramp, target, len);
    tramp[len] = 0xFF; tramp[len + 1] = 0x25; *(int32_t*)(tramp + len + 2) = 0; *(uint64_t*)(tramp + len + 6) = (uint64_t)(target + len);
    uint8_t jmp[32] = {0xFF, 0x25, 0, 0, 0, 0};
    *(uint64_t*)(jmp + 6) = (uint64_t)detour;
    for (size_t i = 14; i < len; i++) jmp[i] = 0x90;
    DWORD old;
    VirtualProtect(target, len, PAGE_EXECUTE_READWRITE, &old);
    memcpy(target, jmp, len);
    VirtualProtect(target, len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, len);
    Log("%s kancasi kuruldu", name);
    return tramp;
}

static void InstallBattery() {
    // mov [rsp+10h],rbx | mov [rsp+8],rcx | push rsi | push rdi | push r14   (14 bytes)
    static const uint8_t expect[14] = {0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x56, 0x57, 0x41, 0x56};
    // the Update hook is always needed: all periodic mod work runs there
    g_updateTramp = InstallHook(kWorldUpdateRva, expect, 14, (void*)&WorldUpdateHook, "MoonWorld.Update");
    if (!g_updateTramp) { Log("HATA: ana thread kancasi kurulamadi; yukseltme seviyeleri uygulanamaz"); return; }
    if (!g_batteryEnabled) { Log("akü kapali (ayar)"); return; }
    InstallBatteryPower();
}
