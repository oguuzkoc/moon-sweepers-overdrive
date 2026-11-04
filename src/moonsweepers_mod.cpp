// Moon Sweepers mod (version.dll proxy, raw IL2CPP runtime API - no MelonLoader needed).
//  1) "Hızlı Konveyör"  - Logistics node, 3 levels: belt speed + machine output interval scaled.
//  2) "Turbo Joonas"    - Cleaning node, 3 levels: Joonas robots move/clean faster (near + far simulation).
//  3) Map "Bombala"     - right-click on an unfinished sector in the station map: pay to complete it.
// Settings: MoonSweepers_ConveyorMod.ini next to MoonSweepers.exe. Log: ConveyorMod.log (same folder).
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <unordered_map>
#include <cctype>
#include "strings.h"

static volatile int g_lang;   // index into kLangs, follows MoonLoc.Code
static const char* Txt(TextId id) { return kLangs[g_lang].t[id]; }

#define FWD(n) __pragma(comment(linker, "/export:" #n "=version_sys." #n))
FWD(GetFileVersionInfoA) FWD(GetFileVersionInfoByHandle) FWD(GetFileVersionInfoExA) FWD(GetFileVersionInfoExW)
FWD(GetFileVersionInfoSizeA) FWD(GetFileVersionInfoSizeExA) FWD(GetFileVersionInfoSizeExW) FWD(GetFileVersionInfoSizeW)
FWD(GetFileVersionInfoW) FWD(VerFindFileA) FWD(VerFindFileW) FWD(VerInstallFileA) FWD(VerInstallFileW)
FWD(VerLanguageNameA) FWD(VerLanguageNameW) FWD(VerQueryValueA) FWD(VerQueryValueW)

#define IL2CPP_FNS(X) \
    X(void*, il2cpp_domain_get, ()) \
    X(void**, il2cpp_domain_get_assemblies, (void*, size_t*)) \
    X(void*, il2cpp_assembly_get_image, (void*)) \
    X(void*, il2cpp_class_from_name, (void*, const char*, const char*)) \
    X(void*, il2cpp_class_get_nested_types, (void*, void**)) \
    X(const char*, il2cpp_class_get_name, (void*)) \
    X(size_t, il2cpp_field_get_offset, (void*)) \
    X(void*, il2cpp_class_get_field_from_name, (void*, const char*)) \
    X(void*, il2cpp_class_get_method_from_name, (void*, const char*, int)) \
    X(void, il2cpp_field_static_get_value, (void*, void*)) \
    X(void, il2cpp_field_static_set_value, (void*, void*)) \
    X(void, il2cpp_runtime_class_init, (void*)) \
    X(void*, il2cpp_runtime_invoke, (void*, void*, void**, void**)) \
    X(void*, il2cpp_object_get_class, (void*)) \
    X(void*, il2cpp_object_new, (void*)) \
    X(void*, il2cpp_string_new, (const char*)) \
    X(void*, il2cpp_array_new, (void*, uintptr_t)) \
    X(uint32_t, il2cpp_gchandle_new, (void*, bool)) \
    X(int32_t, il2cpp_class_instance_size, (void*)) \
    X(void, il2cpp_gc_wbarrier_set_field, (void*, void**, void*)) \
    X(void*, il2cpp_get_corlib, ()) \
    X(void*, il2cpp_thread_attach, (void*)) \
    X(void, il2cpp_thread_detach, (void*)) \
    X(void*, il2cpp_class_get_methods, (void*, void**)) \
    X(const char*, il2cpp_method_get_name, (void*)) \
    X(uint32_t, il2cpp_method_get_param_count, (void*)) \
    X(void*, il2cpp_method_get_param, (void*, uint32_t)) \
    X(char*, il2cpp_type_get_name, (void*)) \
    X(void, il2cpp_free, (void*)) \
    X(void*, il2cpp_class_get_type, (void*)) \
    X(void*, il2cpp_type_get_object, (void*)) \
    X(void*, il2cpp_class_get_parent, (void*))

#define DECL(ret, name, args) typedef ret (*name##_t) args; static name##_t name;
IL2CPP_FNS(DECL)

// ---------------- configuration ----------------
static float g_beltMult[4] = {1.f, 2.f, 3.f, 5.f};
static int g_beltPrice[3] = {1000, 5000, 10000};
static bool g_scaleOutputs = true;
static float g_joonasMult[4] = {1.f, 2.f, 3.f, 5.f};
static int g_joonasPrice[3] = {10000, 20000, 50000};
static int g_bombPrice = 5000;
static bool g_bombEnabled = true;
static float g_chamberMult[4] = {1.f, 2.f, 3.f, 3.f};
static int g_chamberPrice[3] = {20000, 50000, 0};
static float g_pressMult[4] = {1.f, 2.f, 3.f, 3.f};
static bool g_batteryEnabled = true;
static int g_batteryPrice = 0;          // 0 = keep the game's own price
static float g_batteryKwh = 100.f;
static float g_batteryRate = 10.f;   // charging speed multiplier
static int g_pressPrice[3] = {50000, 100000, 0};

static const char* kBeltNode = "logi.conveyor_speed";
static const char* kJoonasNode = "clean.joonas_speed";
static const char* kChamberNode = "prod.chamber_speed";
static const char* kPressNode = "prod.press_speed";
static const int kProductionBranch = 6;           // new branch appended after Economy (5)

// game constants (RVA = offset from GameAssembly.dll base, this build only - every patch verifies first)
static const float kBaseSpeed = 1.1f;            // MoonWorld.BeltSpeed
static const uintptr_t kBeltConstRva = 0x1E71BAC; // shared 1.1f in .rdata
static const float kBaseEmit = 0.35f;            // Catcher/GarageSilo/Packager/ProductEmitSeconds
static const uintptr_t kEmitConstRva = 0x1E71B3C; // shared 0.35f in .rdata
static const uintptr_t kOpenMenuRva = 0x414040;   // MoonMapScreen.OpenMenu(MoonWorld, Vector2)
static const uintptr_t kSetSizeDeltaRva = 0x1AB3900; // RectTransform.set_sizeDelta (as called by OpenMenu)
static const float kMenuStep = 44.f, kMenuExtra = 46.f - 6.f + 14.f, kMenuW = 280.f; // OpenMenu's plate formula
static const int kJoonasKind = 99;                // MoonWorld.JoonasKind

static FILE* g_log;
static uintptr_t g_base;
static void* g_domain;
static float* g_speed;   // within +-2GB of GameAssembly so rip-relative loads reach it
static float* g_emit;
static volatile LONG g_joonasLevel;

static void Log(const char* fmt, ...) {
    if (!g_log) return;
    SYSTEMTIME t; GetLocalTime(&t);
    fprintf(g_log, "[%02d:%02d:%02d] ", t.wHour, t.wMinute, t.wSecond);
    va_list a; va_start(a, fmt); vfprintf(g_log, fmt, a); va_end(a);
    fputc('\n', g_log); fflush(g_log);
}

// ---------------- managed helpers ----------------
template <class T> static T& At(void* o, size_t off) { return *(T*)((char*)o + off); }
static size_t ArrLen(void* a) { return a ? *(size_t*)((char*)a + 0x18) : 0; }
static void** ArrPtrs(void* a) { return (void**)((char*)a + 0x20); }
static void SetRef(void* obj, size_t off, void* val) { il2cpp_gc_wbarrier_set_field(obj, (void**)((char*)obj + off), val); }
static std::string Str(void* s) {
    if (!s) return "";
    int len = At<int>(s, 0x10);
    const wchar_t* w = (const wchar_t*)((char*)s + 0x14);
    int n = WideCharToMultiByte(CP_UTF8, 0, w, len, nullptr, 0, nullptr, nullptr);
    std::string out(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w, len, out.data(), n, nullptr, nullptr);
    return out;
}
static void* NewStr(const char* utf8) { void* s = il2cpp_string_new(utf8); il2cpp_gchandle_new(s, false); return s; }
static void* FindClass(const char* ns, const char* name) {
    size_t n = 0; void** asms = il2cpp_domain_get_assemblies(g_domain, &n);
    for (size_t i = 0; i < n; i++)
        if (void* k = il2cpp_class_from_name(il2cpp_assembly_get_image(asms[i]), ns, name)) return k;
    return nullptr;
}
static void* Nested(void* k, const char* name) {
    void* it = nullptr;
    while (void* n = il2cpp_class_get_nested_types(k, &it)) if (!strcmp(il2cpp_class_get_name(n), name)) return n;
    return nullptr;
}
static void* GetStatic(void* k, const char* f) {
    void* fld = il2cpp_class_get_field_from_name(k, f); void* v = nullptr;
    if (fld) il2cpp_field_static_get_value(fld, &v);
    return v;
}
static size_t FieldOffset(void* k, const char* name) {
    void* f = il2cpp_class_get_field_from_name(k, name);
    return f ? il2cpp_field_get_offset(f) : (size_t)-1;
}
static void* Invoke(void* klass, const char* method, int argc, void* self, void** args, bool* ok = nullptr) {
    void* m = il2cpp_class_get_method_from_name(klass, method, argc);
    if (ok) *ok = false;
    if (!m) { Log("HATA: metod yok %s/%d", method, argc); return nullptr; }
    void* exc = nullptr;
    void* r = il2cpp_runtime_invoke(m, self, args, &exc);
    if (exc) { Log("HATA: %s istisna atti", method); return nullptr; }
    if (ok) *ok = true;
    return r;
}
// Clone a delegate and point it at a native function with the same shape (self, args..., MethodInfo*).
static void* RetargetDelegate(void* src, void* fn) {
    if (!src) return nullptr;
    void* dk = il2cpp_object_get_class(src);
    int32_t size = il2cpp_class_instance_size(dk);
    void* d = il2cpp_object_new(dk);
    il2cpp_gchandle_new(d, false);
    memcpy((char*)d + 0x10, (char*)src + 0x10, size - 0x10);
    size_t mp = FieldOffset(dk, "method_ptr"), ii = FieldOffset(dk, "invoke_impl");
    if (mp != (size_t)-1) At<void*>(d, mp) = fn;
    if (ii != (size_t)-1) At<void*>(d, ii) = fn;
    return d;
}

// System.Decimal (flags, hi, lo, mid)
struct Dec { int32_t flags, hi; uint32_t lo, mid; };
static Dec DecOf(int v) { Dec d{}; if (v < 0) { d.flags = (int32_t)0x80000000; v = -v; } d.lo = (uint32_t)v; return d; }
static double DecToDouble(const Dec& d) {
    double v = (double)d.lo + (double)d.mid * 4294967296.0 + (double)(uint32_t)d.hi * 18446744073709551616.0;
    int scale = (d.flags >> 16) & 0xff;
    v /= pow(10.0, scale);
    return (d.flags < 0) ? -v : v;
}

// ---------------- settings ----------------
static void ReadTriple(const char* ini, const char* sec, const char* fmt, float* mult) {
    char key[16], buf[32];
    for (int i = 0; i < 3; i++) {
        snprintf(key, sizeof key, fmt, i + 1);
        GetPrivateProfileStringA(sec, key, "", buf, sizeof buf, ini);
        if (*buf) { float v = (float)atof(buf); if (v >= 0.1f && v <= 50.f) mult[i + 1] = v; }
    }
}
static void ReadPrices(const char* ini, const char* sec, int* price) {
    char key[16];
    for (int i = 0; i < 3; i++) {
        snprintf(key, sizeof key, "Fiyat%d", i + 1);
        int p = GetPrivateProfileIntA(sec, key, price[i], ini);
        if (p >= 0) price[i] = p;
    }
}
static void LoadSettings(const char* dir) {
    char ini[MAX_PATH]; snprintf(ini, MAX_PATH, "%s\\MoonSweepers_ConveyorMod.ini", dir);
    // append missing sections so older ini files get the new options
    auto ensure = [&](const char* sec, const char* body) {
        char probe[8];
        if (GetPrivateProfileSectionA(sec, probe, sizeof probe, ini) > 0) return;
        if (FILE* f = fopen(ini, "a")) { fputs(body, f); fclose(f); }
    };
    ensure("Konveyor",
           "; Hizli Konveyor (Lojistik). HizX = carpan, FiyatX = o seviyenin fiyati.\n"
           "; CikislarHizlansin=1: yakalayici/silo/paketleyici/uretim banda ayni carpanla daha sik birakir.\n"
           "[Konveyor]\nHiz1=2\nHiz2=3\nHiz3=5\nFiyat1=1000\nFiyat2=5000\nFiyat3=10000\nCikislarHizlansin=1\n");
    ensure("Joonas",
           "\n; Turbo Joonas (Temizlik). HizX = Joonas hiz carpani, FiyatX = o seviyenin fiyati.\n"
           "[Joonas]\nHiz1=2\nHiz2=3\nHiz3=5\nFiyat1=10000\nFiyat2=20000\nFiyat3=50000\n");
    ensure("Bomba",
           "\n; Harita sag tik > Bombala: bitmemis sektoru aninda tamamlar. Acik=0 ile kapatilir.\n"
           "[Bomba]\nAcik=1\nFiyat=5000\n");
    ensure("Uretim",
           "\n; Uretim dali: basinc odasi ve tas presi uretim dongusu hizlari (2 seviye).\n"
           "[Uretim]\nBasincHiz1=2\nBasincHiz2=3\nBasincFiyat1=20000\nBasincFiyat2=50000\n"
           "PresHiz1=2\nPresHiz2=3\nPresFiyat1=50000\nPresFiyat2=100000\n");
    ensure("Aku",
           "\n; Battery bank: oyunun gizli akusu insa menusune eklenir ve gunes paneli agina depolama olarak baglanir.\n"
           "; KapasiteKwh = bir akunun depoladigi enerji. Fiyat=0 ise oyunun kendi fiyati kullanilir.\n"
           "; SarjCarpani = sarj hizi carpani (1 = gercek zaman, 10 = 10 kat hizli).\n"
           "[Aku]\nAcik=1\nKapasiteKwh=100\nFiyat=0\nSarjCarpani=10\n");

    ReadTriple(ini, "Konveyor", "Hiz%d", g_beltMult);
    ReadPrices(ini, "Konveyor", g_beltPrice);
    g_scaleOutputs = GetPrivateProfileIntA("Konveyor", "CikislarHizlansin", 1, ini) != 0;
    ReadTriple(ini, "Joonas", "Hiz%d", g_joonasMult);
    ReadPrices(ini, "Joonas", g_joonasPrice);
    g_bombEnabled = GetPrivateProfileIntA("Bomba", "Acik", 1, ini) != 0;
    g_bombPrice = GetPrivateProfileIntA("Bomba", "Fiyat", g_bombPrice, ini);
    auto readF = [&](const char* key, float& v) {
        char buf[32]; GetPrivateProfileStringA("Uretim", key, "", buf, sizeof buf, ini);
        if (*buf) { float f = (float)atof(buf); if (f >= 0.1f && f <= 50.f) v = f; }
    };
    readF("BasincHiz1", g_chamberMult[1]); readF("BasincHiz2", g_chamberMult[2]); g_chamberMult[3] = g_chamberMult[2];
    readF("PresHiz1", g_pressMult[1]); readF("PresHiz2", g_pressMult[2]); g_pressMult[3] = g_pressMult[2];
    g_chamberPrice[0] = GetPrivateProfileIntA("Uretim", "BasincFiyat1", g_chamberPrice[0], ini);
    g_chamberPrice[1] = GetPrivateProfileIntA("Uretim", "BasincFiyat2", g_chamberPrice[1], ini);
    g_pressPrice[0] = GetPrivateProfileIntA("Uretim", "PresFiyat1", g_pressPrice[0], ini);
    g_pressPrice[1] = GetPrivateProfileIntA("Uretim", "PresFiyat2", g_pressPrice[1], ini);
    g_batteryEnabled = GetPrivateProfileIntA("Aku", "Acik", 1, ini) != 0;
    g_batteryPrice = GetPrivateProfileIntA("Aku", "Fiyat", 0, ini);
    { char buf[32]; GetPrivateProfileStringA("Aku", "KapasiteKwh", "100", buf, sizeof buf, ini);
      float f = (float)atof(buf); if (f > 0.f && f <= 100000.f) g_batteryKwh = f; }
    { char buf[32]; GetPrivateProfileStringA("Aku", "SarjCarpani", "10", buf, sizeof buf, ini);
      float f = (float)atof(buf); if (f > 0.f && f <= 1000.f) g_batteryRate = f; }
    Log("aku: acik=%d kapasite %.1f kWh fiyat %d", g_batteryEnabled, g_batteryKwh, g_batteryPrice);
    Log("uretim: basinc %.2g/%.2g fiyat %d/%d | pres %.2g/%.2g fiyat %d/%d", g_chamberMult[1], g_chamberMult[2],
        g_chamberPrice[0], g_chamberPrice[1], g_pressMult[1], g_pressMult[2], g_pressPrice[0], g_pressPrice[1]);
    Log("konveyor %.2g/%.2g/%.2g fiyat %d/%d/%d cikislar=%d | joonas %.2g/%.2g/%.2g fiyat %d/%d/%d | bomba=%d fiyat %d",
        g_beltMult[1], g_beltMult[2], g_beltMult[3], g_beltPrice[0], g_beltPrice[1], g_beltPrice[2], g_scaleOutputs,
        g_joonasMult[1], g_joonasMult[2], g_joonasMult[3], g_joonasPrice[0], g_joonasPrice[1], g_joonasPrice[2],
        g_bombEnabled, g_bombPrice);
}

// ---------------- constant-load redirection (belt speed / output interval) ----------------
struct Site { const char* name; uintptr_t dispRva; };
static const Site kBeltSites[] = {
    {"StepConveyors#1", 0x74d8f2}, {"StepConveyors#2", 0x74ded8}, {"StepConveyors#3", 0x74e10c},
    {"BeltSurfaceVelocity", 0x6aa9ed}, {"ApplyCrewBelts", 0x69c06f},
    {"ConveyorNetwork.TreadPhase", 0x78d721}, {"ConveyorNetwork.LateUpdate", 0x7891e8},
    {"ConveyorNetwork.Spawn", 0x78d194}, {"DepotBuilder.DrawConveyorChevrons", 0x437df5},
};
static const Site kEmitSites[] = {
    {"StepCatchers", 0x74c41b}, {"StepGarageSilos", 0x74fbd3}, {"StepPackagers", 0x750807}, {"StepProduction", 0x750e6e},
};

static float* AllocNear(uintptr_t base) {
    SYSTEM_INFO si; GetSystemInfo(&si);
    uintptr_t gran = si.dwAllocationGranularity;
    for (uintptr_t d = 0x10000000; d < 0x70000000; d += gran)
        for (int sign = -1; sign <= 1; sign += 2) {
            uintptr_t a = (base + (sign < 0 ? -(intptr_t)d : d)) & ~(gran - 1);
            if (void* p = VirtualAlloc((void*)a, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) return (float*)p;
        }
    return nullptr;
}

static int Redirect(const Site* sites, size_t count, uintptr_t constRva, float* to, const char* what) {
    int ok = 0;
    for (size_t i = 0; i < count; i++) {
        uint8_t* disp = (uint8_t*)(g_base + sites[i].dispRva);
        uintptr_t next = (uintptr_t)disp + 4;
        if (next + *(int32_t*)disp != g_base + constRva) { Log("ATLANDI %s: beklenen sabit degil (oyun guncellenmis olabilir)", sites[i].name); continue; }
        intptr_t nd = (intptr_t)((uintptr_t)to - next);
        if (nd != (int32_t)nd) { Log("ATLANDI %s: mesafe 2GB disinda", sites[i].name); continue; }
        DWORD old;
        VirtualProtect(disp, 4, PAGE_EXECUTE_READWRITE, &old);
        *(int32_t*)disp = (int32_t)nd;
        VirtualProtect(disp, 4, old, &old);
        FlushInstructionCache(GetCurrentProcess(), disp, 4);
        ok++;
    }
    Log("%s: %d/%zu yer yonlendirildi", what, ok, count);
    return ok;
}

// ---------------- upgrade-tree nodes ----------------
static int Clamp3(int l) { return l < 1 ? 1 : (l > 3 ? 3 : l); }
static void* Effect(TextId fmt, float mult) {
    char b[160];
    snprintf(b, sizeof b, Txt(fmt), mult);
    return il2cpp_string_new(b);
}
static void* BeltEffect(void*, int32_t level, const void*) {
    return Effect(g_scaleOutputs ? T_BELT_EFFECT : T_BELT_EFFECT_NO_OUTPUT, g_beltMult[Clamp3(level)]);
}
static void* JoonasEffect(void*, int32_t level, const void*) { return Effect(T_JOONAS_EFFECT, g_joonasMult[Clamp3(level)]); }
static void* ChamberEffect(void*, int32_t level, const void*) { return Effect(T_CHAMBER_EFFECT, g_chamberMult[Clamp3(level)]); }
static void* PressEffect(void*, int32_t level, const void*) { return Effect(T_PRESS_EFFECT, g_pressMult[Clamp3(level)]); }

struct NodeDef {
    const char* id; TextId title;   // summary text is title + 1
    int branch, glyph, levels; const int* prices; const char* requiresFrom; // copy requires[] from this node
    const char* effectFrom; void* effectFn; float angleDeg, radius;
};

// nodes we added, so their texts can follow the game's language
struct OurNode { void* node; TextId title; };
static OurNode g_ourNodes[8]; static int g_ourNodeCount;

static void* g_treeK; static void* g_nodeK;

// Append a 7th branch ("ÜRETİM"): the tree UI sizes its tabs from BranchNames.Length, and looks up
// BranchColors/BranchTaglines by branch index, so all three arrays grow together.
static bool ExtendBranches() {
    void* names = GetStatic(g_treeK, "BranchNames");
    void* tags = GetStatic(g_treeK, "BranchTaglines");
    void* colors = GetStatic(g_treeK, "BranchColors");
    size_t n = ArrLen(names);
    if (n != (size_t)kProductionBranch) { Log("dal: beklenmeyen dal sayisi %zu (zaten eklenmis olabilir)", n); return n > (size_t)kProductionBranch; }
    if (ArrLen(tags) != n || ArrLen(colors) != n) { Log("HATA: dal dizileri uyumsuz"); return false; }

    void* strK = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "String");
    void* colK = FindClass("UnityEngine", "Color");
    if (!strK || !colK) { Log("HATA: String/Color sinifi yok"); return false; }

    void* names2 = il2cpp_array_new(strK, n + 1);
    void* tags2 = il2cpp_array_new(strK, n + 1);
    for (size_t i = 0; i < n; i++) {
        SetRef(names2, 0x20 + i * 8, ArrPtrs(names)[i]);
        SetRef(tags2, 0x20 + i * 8, ArrPtrs(tags)[i]);
    }
    SetRef(names2, 0x20 + n * 8, NewStr(Txt(T_BRANCH_NAME)));
    SetRef(tags2, 0x20 + n * 8, NewStr(Txt(T_BRANCH_TAGLINE)));
    void* colors2 = il2cpp_array_new(colK, n + 1);
    memcpy((char*)colors2 + 0x20, (char*)colors + 0x20, n * 16);       // Color = 4 floats, inline
    float amber[4] = {0.95f, 0.62f, 0.22f, 1.f};
    memcpy((char*)colors2 + 0x20 + n * 16, amber, 16);
    for (void* a : {names2, tags2, colors2}) il2cpp_gchandle_new(a, false);

    il2cpp_field_static_set_value(il2cpp_class_get_field_from_name(g_treeK, "BranchNames"), names2);
    il2cpp_field_static_set_value(il2cpp_class_get_field_from_name(g_treeK, "BranchTaglines"), tags2);
    il2cpp_field_static_set_value(il2cpp_class_get_field_from_name(g_treeK, "BranchColors"), colors2);
    Log("dal eklendi: %zu -> %zu (ÜRETİM)", n, n + 1);
    return true;
}

static bool AddNodes(const NodeDef* defs, int count) {
    g_treeK = FindClass("MoonCleaners", "MoonUpgradeTree");
    g_nodeK = g_treeK ? Nested(g_treeK, "Node") : nullptr;
    if (!g_treeK || !g_nodeK) { Log("HATA: MoonUpgradeTree/Node bulunamadi"); return false; }
    il2cpp_runtime_class_init(g_treeK);
    bool haveProduction = ExtendBranches();

    void* nodes = GetStatic(g_treeK, "Nodes");
    size_t n = ArrLen(nodes);
    auto byName = [&](const char* id) -> void* {
        for (size_t i = 0; i < n; i++)
            if (Str(At<void*>(ArrPtrs(nodes)[i], 0x10)) == id) return ArrPtrs(nodes)[i];
        return nullptr;
    };
    void* intK = il2cpp_class_from_name(il2cpp_get_corlib(), "System", "Int32");
    void* added[8]; int addedCount = 0;
    for (int d = 0; d < count; d++) {
        const NodeDef& def = defs[d];
        if (byName(def.id)) { Log("dugum zaten var: %s", def.id); continue; }
        if (def.branch == kProductionBranch && !haveProduction) { Log("ATLANDI %s: uretim dali yok", def.id); continue; }
        void* req = byName(def.requiresFrom); void* eff = byName(def.effectFrom);
        if (!req || !eff) { Log("HATA: %s icin kaynak dugum yok", def.id); continue; }

        void* node = il2cpp_object_new(g_nodeK);
        il2cpp_gchandle_new(node, false);
        if (void* ctor = il2cpp_class_get_method_from_name(g_nodeK, ".ctor", 0)) { void* e = nullptr; il2cpp_runtime_invoke(ctor, node, nullptr, &e); }
        SetRef(node, 0x10, NewStr(def.id));
        SetRef(node, 0x18, NewStr(Txt(def.title)));
        SetRef(node, 0x20, NewStr(Txt((TextId)(def.title + 1))));
        if (g_ourNodeCount < 8) g_ourNodes[g_ourNodeCount++] = {node, def.title};
        At<int>(node, 0x28) = def.branch;
        At<int>(node, 0x2c) = def.glyph;
        At<int>(node, 0x30) = def.levels;
        void* prices = il2cpp_array_new(intK, def.levels);
        for (int i = 0; i < def.levels; i++) ((int*)ArrPtrs(prices))[i] = def.prices[i];
        SetRef(node, 0x38, prices);
        SetRef(node, 0x40, At<void*>(req, 0x40));
        At<bool>(node, 0x48) = false;
        SetRef(node, 0x50, nullptr);
        float a = def.angleDeg * 3.14159265f / 180.f;
        At<float>(node, 0x58) = cosf(a) * def.radius;
        At<float>(node, 0x5c) = sinf(a) * def.radius;
        At<bool>(node, 0x60) = false;
        SetRef(node, 0x68, RetargetDelegate(At<void*>(eff, 0x68), def.effectFn));
        added[addedCount++] = node;
        Log("dugum hazir: %s", def.id);
    }
    if (!addedCount) return true;

    void* grown = il2cpp_array_new(g_nodeK, n + addedCount);
    for (size_t i = 0; i < n; i++) SetRef(grown, 0x20 + i * 8, ArrPtrs(nodes)[i]);
    for (int i = 0; i < addedCount; i++) SetRef(grown, 0x20 + (n + i) * 8, added[i]);
    void* f = il2cpp_class_get_field_from_name(g_treeK, "Nodes");
    il2cpp_field_static_set_value(f, grown);
    void* dict = Invoke(g_treeK, "Index", 0, nullptr, nullptr);
    if (!dict) return false;
    il2cpp_field_static_set_value(il2cpp_class_get_field_from_name(g_treeK, "byId"), dict);
    Log("agac: %zu -> %zu dugum", n, n + addedCount);
    return true;
}

// ---------------- world access ----------------
static void* g_worldK;
static void* FindWorld() {
    static const struct { const char* cls; const char* field; } src[] = {
        {"MoonAchievements", "world"}, {"MoonMapScreen", "meshWorld"}, {"MoonCollectorGlobe", "vehiclesWorld"}};
    for (auto& s : src)
        if (void* k = FindClass("MoonCleaners", s.cls)) {
            il2cpp_runtime_class_init(k);
            if (void* w = GetStatic(k, s.field)) return w;
        }
    return nullptr;
}
static int TreeLevel(void* world, void* idStr) {
    static void* team = NewStr("team");
    void* args[] = {team, idStr};
    void* r = Invoke(g_worldK, "TreeLevel", 2, world, args);
    int l = r ? At<int>(r, 0x10) : 0;
    return l < 0 ? 0 : (l > 3 ? 3 : l);
}

// ---------------- Joonas speed ----------------
struct RobotOrig { float clean, travel, turn, accel, motion; float applied; };
static std::unordered_map<void*, RobotOrig> g_robots;
static std::unordered_map<void*, float> g_farSpecs;   // FarRobotSpec* -> original rate

static __declspec(noinline) void ApplyJoonas(void* world, float mult) {
    static void* robotK = FindClass("MoonCleaners", "MoonRobotVacuum");
    if (!robotK) return;
    il2cpp_runtime_class_init(robotK);
    void* list = GetStatic(robotK, "active");
    if (list) {
        void* items = At<void*>(list, 0x10);
        int size = At<int>(list, 0x18);
        for (int i = 0; i < size && items && (size_t)i < ArrLen(items); i++) {
            void* r = ArrPtrs(items)[i];
            if (!r) continue;
            void* rig = At<void*>(r, 0x260);     // MoonRobotVacuum.Launcher (Joonas only)
            if (!rig) continue;
            auto it = g_robots.find(r);
            float* f = &At<float>(r, 0x70);       // cleaningSpeed, travelSpeed, turnRate, acceleration
            float* mr = &At<float>(rig, 0x98);    // MoonJoonasRig.motionRate
            bool fresh = it == g_robots.end() || fabsf(f[1] - it->second.travel * it->second.applied) > 1e-3f;
            if (fresh) {
                g_robots[r] = {f[0], f[1], f[2], f[3], *mr, 1.f};
                it = g_robots.find(r);
            }
            RobotOrig& o = it->second;
            if (o.applied != mult) {
                f[0] = o.clean * mult; f[1] = o.travel * mult; f[2] = o.turn * mult; f[3] = o.accel * mult;
                *mr = o.motion * mult;
                o.applied = mult;
                Log("Joonas %p: x%g (seyahat %.2g, temizlik %.2g, hareket %.2g)", r, mult, f[1], f[0], *mr);
            }
        }
    }
    // far-away simulation: cached FarRobotSpec for kind 99
    void* specs = At<void*>(world, 0x5d8);
    if (specs) {
        int key = kJoonasKind; void* spec = nullptr;
        void* args[] = {&key, &spec};
        void* r = Invoke(il2cpp_object_get_class(specs), "TryGetValue", 2, specs, args);
        if (r && At<bool>(r, 0x10) && spec) {
            auto it = g_farSpecs.find(spec);
            if (it == g_farSpecs.end()) it = g_farSpecs.emplace(spec, At<float>(spec, 0x10)).first;
            float want = it->second * mult;
            if (At<float>(spec, 0x10) != want) { At<float>(spec, 0x10) = want; Log("uzak Joonas hizi: %.3g", want); }
        }
    }
}

static __declspec(noinline) void SafeApplyJoonas(void* world, float mult) {
    __try { ApplyJoonas(world, mult); }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("HATA: Joonas uygulanirken istisna"); }
}

// ---------------- production speed (MoonWorld.ChamberSpec / PressSpec .cycleSeconds) ----------------
static std::unordered_map<void*, float> g_specCycle;   // ProcessorSpec* -> original cycleSeconds
static void ApplyProcessor(const char* field, float mult, int level, int& lastLevel) {
    void* spec = GetStatic(g_worldK, field);
    if (!spec) return;
    float* cycle = &At<float>(spec, 0x24);
    auto it = g_specCycle.find(spec);
    if (it == g_specCycle.end()) {
        it = g_specCycle.emplace(spec, *cycle).first;
        Log("%s: orijinal dongu %.3gs", field, *cycle);
    }
    float want = it->second / mult;
    if (*cycle != want) *cycle = want;
    if (level != lastLevel) { Log("%s seviye %d -> x%g, dongu %.3gs", field, level, mult, want); lastLevel = level; }
}

// ---------------- map "Bombala" ----------------
static uint8_t* g_openMenuTramp;
typedef void (*OpenMenu_t)(void* self, void* world, uint64_t at, const void* method);
static void* g_bombWorld; static int g_bombRegion = -1;
static void* g_localPlayer;

static double Wallet(void* world) {
    void* args[] = {g_localPlayer};
    void* r = Invoke(g_worldK, "WalletDollars", 1, world, args);
    return r ? DecToDouble(At<Dec>(r, 0x10)) : 0.0;
}

static void BombAction(void* /*target*/, const void* /*method*/) {
    void* world = g_bombWorld; int id = g_bombRegion;
    if (!world || id < 0) return;
    double have = Wallet(world);
    if (have < g_bombPrice) { Log("bomba: para yetmiyor (%.0f < %d)", have, g_bombPrice); return; }
    bool no = false;
    void* chk[] = {&id};
    void* done = Invoke(g_worldK, "IsRegionComplete", 1, world, chk);
    if (done && At<bool>(done, 0x10)) { Log("bomba: sektor %d zaten temiz", id); return; }
    Dec price = DecOf(-g_bombPrice);   // ChargeBuild takes a signed delta: the game passes -price for purchases
    void* pay[] = {g_localPlayer, &price};
    bool ok = false;
    Invoke(g_worldK, "ChargeBuild", 2, world, pay, &ok);
    if (!ok) return;
    void* args[] = {&id, &no, nullptr, &no};
    Invoke(g_worldK, "CommitRegion", 4, world, args, &ok);
    Log("bomba: sektor %d %s, %d odendi (bakiye %.0f -> %.0f)", id, ok ? "tamamlandi" : "TAMAMLANAMADI", g_bombPrice, have, Wallet(world));
}

static __declspec(noinline) void AddBombRow(void* self, void* world, uint64_t at) {
    {
        if (!At<bool>(self, 0x1a8)) return;                                // menu did not open
        if (!At<bool>(self, 0x1a9)) { Log("menu: isaret/robot uzerinde acildi, bomba yok"); return; } // menuOnGround
        // Direction under the right-click: MoonMapScreen.Pick(screen) -> Nullable<Vector3> (boxed: null or Vector3).
        // `hovered` is only a fallback - it isn't refreshed when the player right-clicks without moving/left-clicking.
        float dir[3];
        void* pargs[] = {&at};
        void* picked = Invoke(il2cpp_object_get_class(self), "Pick", 1, self, pargs);
        if (picked) memcpy(dir, (char*)picked + 0x10, sizeof dir);
        else if (At<bool>(self, 0xf4)) { dir[0] = At<float>(self, 0xf8); dir[1] = At<float>(self, 0xfc); dir[2] = At<float>(self, 0x100); }
        else { Log("menu: tiklanan nokta bulunamadi, bomba yok"); return; }
        void* regions = Invoke(g_worldK, "get_Regions", 0, world, nullptr);
        if (!regions) return;
        void* fargs[] = {dir};
        void* r = Invoke(il2cpp_object_get_class(regions), "Find", 1, regions, fargs);
        if (!r) return;
        int id = At<int>(r, 0x10);
        void* cargs[] = {&id};
        void* done = Invoke(g_worldK, "IsRegionComplete", 1, world, cargs);
        if (!done) return;
        if (At<bool>(done, 0x10)) { Log("menu: sektor %d zaten temiz, bomba yok", id); return; }

        void* rows = At<void*>(self, 0x1a0);
        if (!rows || At<int>(rows, 0x18) < 1) { Log("menu: satir listesi bos, bomba yok"); return; }
        void* firstRow = ArrPtrs(At<void*>(rows, 0x10))[0];
        void* act = RetargetDelegate(At<void*>(firstRow, 0x28), (void*)&BombAction);
        if (!act) return;

        Dec price = DecOf(g_bombPrice);
        void* margs[] = {&price};
        void* money = Invoke(g_worldK, "FormatMoney", 1, nullptr, margs);
        std::string priceText = money ? Str(money) : std::to_string(g_bombPrice);
        char label[160];
        snprintf(label, sizeof label, Txt(T_BOMB), priceText.c_str());
        std::string text = label;
        bool enabled = Wallet(world) >= g_bombPrice, danger = true;
        void* aargs[] = {il2cpp_string_new(text.c_str()), &enabled, act, &danger};
        Invoke(il2cpp_object_get_class(self), "AddMenuRow", 4, self, aargs);
        g_bombWorld = world; g_bombRegion = id;

        // OpenMenu already sized the plate for the old row count; grow it with the same formula so our
        // row sits inside the background (and clicks on it count as "on the menu").
        typedef void (*SetSizeDelta_t)(void* rt, uint64_t v, const void* method);
        SetSizeDelta_t setSize = (SetSizeDelta_t)(g_base + kSetSizeDeltaRva);
        int count = At<int>(rows, 0x18);
        float size[2] = {kMenuW, count * kMenuStep + kMenuExtra};
        uint64_t packed; memcpy(&packed, size, 8);
        for (size_t off : {(size_t)0x180, (size_t)0x188})   // menu, menuPlate
            if (void* rt = At<void*>(self, off)) setSize(rt, packed, nullptr);
        Log("menu: sektor %d icin '%s' eklendi (aktif=%d, hoveredSector=%d)", id, text.c_str(), enabled, At<int>(self, 0xd4));
    }
}

static void OpenMenuHook(void* self, void* world, uint64_t at, const void* method) {
    ((OpenMenu_t)g_openMenuTramp)(self, world, at, method);
    if (!g_bombEnabled || !world) return;
    __try { AddBombRow(self, world, at); }
    __except (EXCEPTION_EXECUTE_HANDLER) { Log("HATA: menu kancasinda istisna"); }
}

static bool InstallOpenMenuHook() {
    // prologue: mov rax,rsp | mov [rax+10h],rbx | mov [rax+20h],rsi | mov [rax+8],rcx  (15 bytes, no rip-relative)
    static const uint8_t expect[15] = {0x48, 0x8B, 0xC4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x20, 0x48, 0x89, 0x48, 0x08};
    uint8_t* target = (uint8_t*)(g_base + kOpenMenuRva);
    if (memcmp(target, expect, sizeof expect)) { Log("ATLANDI menu kancasi: OpenMenu beklenen kod degil"); return false; }
    g_openMenuTramp = (uint8_t*)VirtualAlloc(nullptr, 64, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (!g_openMenuTramp) return false;
    uint8_t* t = g_openMenuTramp;
    memcpy(t, target, 15);
    t[15] = 0xFF; t[16] = 0x25; *(int32_t*)(t + 17) = 0; *(uint64_t*)(t + 21) = (uint64_t)(target + 15);
    uint8_t jmp[15] = {0xFF, 0x25, 0, 0, 0, 0};
    *(uint64_t*)(jmp + 6) = (uint64_t)&OpenMenuHook;
    jmp[14] = 0x90;
    DWORD old;
    VirtualProtect(target, 15, PAGE_EXECUTE_READWRITE, &old);
    memcpy(target, jmp, 15);
    VirtualProtect(target, 15, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, 15);
    Log("menu kancasi kuruldu");
    return true;
}

#include "battery.h"

// ---------------- language ----------------
static void* g_locK;
static void LogLanguages() {
    g_locK = FindClass("MoonCleaners", "MoonLoc");
    if (!g_locK) { Log("dil: MoonLoc yok, Ingilizce kullanilacak"); return; }
    il2cpp_runtime_class_init(g_locK);
    void* langs = GetStatic(g_locK, "Languages");
    std::string list;
    for (size_t i = 0; i < ArrLen(langs); i++) {
        void* l = ArrPtrs(langs)[i];
        if (!l) continue;
        std::string code = Str(At<void*>(l, 0x10));
        int idx = LangIndexFor(code.c_str());
        bool exact = idx != 0 || code.rfind("en", 0) == 0;
        list += code + "=" + Str(At<void*>(l, 0x18)) + (exact ? "" : "(!ceviri yok, Ingilizce)") + " ";
    }
    Log("oyun dilleri: %s", list.c_str());
}

// Re-reads the active language; when it changes, swaps our node texts and the branch name/tagline.
// Reads MoonLoc.current.code straight from memory: calling MoonLoc.get_Code from this background
// thread can resolve the language through Unity APIs, which crashes off the main thread.
static void UpdateLanguage() {
    if (!g_locK) return;
    void* current = GetStatic(g_locK, "current");   // MoonLoc.Language (null until the game picks one)
    if (!current) return;
    std::string c = Str(At<void*>(current, 0x10));  // Language.code
    if (c.empty()) return;
    int idx = LangIndexFor(c.c_str());
    static int applied = -1;
    if (idx == applied) return;
    g_lang = idx;
    for (int i = 0; i < g_ourNodeCount; i++) {
        SetRef(g_ourNodes[i].node, 0x18, NewStr(Txt(g_ourNodes[i].title)));
        SetRef(g_ourNodes[i].node, 0x20, NewStr(Txt((TextId)(g_ourNodes[i].title + 1))));
    }
    if (g_treeK) {
        void* names = GetStatic(g_treeK, "BranchNames");
        void* tags = GetStatic(g_treeK, "BranchTaglines");
        if (ArrLen(names) > (size_t)kProductionBranch && ArrLen(tags) > (size_t)kProductionBranch) {
            SetRef(names, 0x20 + kProductionBranch * 8, NewStr(Txt(T_BRANCH_NAME)));
            SetRef(tags, 0x20 + kProductionBranch * 8, NewStr(Txt(T_BRANCH_TAGLINE)));
        }
    }
    g_batterySub = NewStr(Txt(T_BATTERY_SUB));
    if (applied != -1 || g_ourNodeCount) Log("dil: '%s' -> %s", c.c_str(), kLangs[idx].code);
    applied = g_ourNodeCount ? idx : -1;   // re-apply once nodes exist
}

// ---------------- periodic work (main thread, from the MoonWorld.Update hook) ----------------
static void* g_tickIds[4];   // belt, joonas, chamber, press node ids
static void ModTick(void* world) {
    static ULONGLONG next = 0;
    ULONGLONG now = GetTickCount64();
    if (now < next || !g_tickIds[0]) return;
    next = now + 500;
    static int lastBelt = -1, lastJoonas = -1, lastChamber = -1, lastPress = -1;
    UpdateLanguage();
    int belt = TreeLevel(world, g_tickIds[0]), joonas = TreeLevel(world, g_tickIds[1]);
    if (g_speed) {
        g_speed[0] = kBaseSpeed * g_beltMult[belt];
        *g_emit = kBaseEmit / g_beltMult[belt];
    }
    if (belt != lastBelt) { Log("konveyor seviye %d -> hiz %.3g (x%g), cikis araligi %.3gs", belt, g_speed ? g_speed[0] : 0.f, g_beltMult[belt], g_emit ? *g_emit : 0.f); lastBelt = belt; }
    if (joonas != lastJoonas) { Log("joonas seviye %d -> x%g", joonas, g_joonasMult[joonas]); lastJoonas = joonas; }
    SafeApplyJoonas(world, g_joonasMult[joonas]);
    int chamber = TreeLevel(world, g_tickIds[2]), press = TreeLevel(world, g_tickIds[3]);
    ApplyProcessor("ChamberSpec", g_chamberMult[chamber], chamber, lastChamber);
    ApplyProcessor("PressSpec", g_pressMult[press], press, lastPress);
}

// ---------------- shutdown detection ----------------
// The game's main window (UnityWndClass) disappears before the scripting runtime shuts down.
static BOOL CALLBACK FindUnityWindow(HWND h, LPARAM found) {
    DWORD pid = 0;
    GetWindowThreadProcessId(h, &pid);
    if (pid != GetCurrentProcessId()) return TRUE;
    char cls[64];
    if (GetClassNameA(h, cls, sizeof cls) && !strcmp(cls, "UnityWndClass")) { *(bool*)found = true; return FALSE; }
    return TRUE;
}
static bool GameWindowAlive() {
    bool found = false;
    EnumWindows(FindUnityWindow, (LPARAM)&found);
    return found;
}

// ---------------- main worker ----------------
static DWORD WINAPI Worker(LPVOID) {
    HMODULE ga = nullptr;
    for (int i = 0; i < 600 && !(ga = GetModuleHandleA("GameAssembly.dll")); i++) Sleep(100);
    if (!ga) return 0;
    g_base = (uintptr_t)ga;
#define RES(ret, name, args) name = (name##_t)GetProcAddress(ga, #name); if (!name) { Log("HATA: export yok %s", #name); return 0; }
    IL2CPP_FNS(RES)

    // This thread is not registered with the GC yet, so it must not call anything that can allocate.
    // il2cpp_domain_get creates the domain when it does not exist yet, so read the runtime's domain
    // pointer straight from memory instead (export -> jmp Domain::GetCurrent -> mov rax,[rip+x]).
    void** domainSlot = nullptr;
    {
        uint8_t* p = (uint8_t*)il2cpp_domain_get;
        if (p[0] == 0xE9) p += 5 + *(int32_t*)(p + 1);
        for (int k = 0; k < 16 && !domainSlot; k++)
            if (p[k] == 0x48 && p[k + 1] == 0x8B && p[k + 2] == 0x05)
                domainSlot = (void**)(p + k + 7 + *(int32_t*)(p + k + 3));
    }
    if (!domainSlot) { Log("HATA: domain adresi bulunamadi"); return 0; }
    for (int i = 0; i < 1200 && !(g_domain = *(void* volatile*)domainSlot); i++) Sleep(100);
    if (!g_domain) { Log("HATA: domain olusmadi"); return 0; }
    // Let the runtime finish starting (game window up) before joining it.
    for (int i = 0; i < 1200; i++) {
        HWND w = nullptr; DWORD pid = 0;
        while ((w = FindWindowExA(nullptr, w, "UnityWndClass", nullptr)))
            if (GetWindowThreadProcessId(w, &pid), pid == GetCurrentProcessId()) break;
        if (w) break;
        Sleep(100);
    }
    Sleep(2000);
    void* thread = il2cpp_thread_attach(g_domain);
    for (int i = 0; i < 600; i++) {
        size_t n = 0;
        if (il2cpp_domain_get_assemblies(g_domain, &n) && n > 10 && FindClass("MoonCleaners", "MoonUpgradeTree")) break;
        Sleep(100);
    }

    g_speed = AllocNear(g_base);
    if (g_speed) {
        g_speed[0] = kBaseSpeed; g_emit = g_speed + 1; *g_emit = kBaseEmit;
        Redirect(kBeltSites, sizeof kBeltSites / sizeof *kBeltSites, kBeltConstRva, g_speed, "hiz yamasi");
        if (g_scaleOutputs) Redirect(kEmitSites, sizeof kEmitSites / sizeof *kEmitSites, kEmitConstRva, g_emit, "cikis yamasi");
    } else Log("HATA: yakin bellek ayrilamadi");

    LogLanguages();
    UpdateLanguage();

    const NodeDef defs[] = {
        {kBeltNode, T_BELT_TITLE,
         4 /*Logistics*/, 20 /*Motor*/, 3, g_beltPrice, "logi.tether", "logi.transfer", (void*)&BeltEffect, 225.f, 470.f},
        {kJoonasNode, T_JOONAS_TITLE,
         2 /*Cleaning*/, 24 /*Turbo*/, 3, g_joonasPrice, "clean.reach", "clean.width", (void*)&JoonasEffect, 0.f, 600.f},
        // Production branch: requires[] copied from logi.transfer (core >= 1, always met)
        {kChamberNode, T_CHAMBER_TITLE,
         kProductionBranch, 21 /*Hopper*/, 2, g_chamberPrice, "logi.transfer", "logi.transfer", (void*)&ChamberEffect, 260.f, 300.f},
        {kPressNode, T_PRESS_TITLE,
         kProductionBranch, 2 /*Compactor*/, 2, g_pressPrice, "logi.transfer", "logi.transfer", (void*)&PressEffect, 280.f, 300.f},
    };
    AddNodes(defs, 4);

    g_worldK = FindClass("MoonCleaners", "MoonWorld");
    if (void* f = g_worldK ? il2cpp_class_get_field_from_name(g_worldK, "LocalPlayer") : nullptr) {
        il2cpp_field_static_get_value(f, &g_localPlayer);
        if (g_localPlayer) il2cpp_gchandle_new(g_localPlayer, false);
    }
    Log("LocalPlayer = '%s'", Str(g_localPlayer).c_str());
    if (g_bombEnabled && g_localPlayer) InstallOpenMenuHook();
    InstallBattery();

    g_tickIds[0] = NewStr(kBeltNode);
    g_tickIds[1] = NewStr(kJoonasNode);
    g_tickIds[2] = NewStr(kChamberNode);
    g_tickIds[3] = NewStr(kPressNode);
    // Setup is done. Everything periodic now runs on the game's main thread (MoonWorld.Update hook ->
    // ModTick), so this thread leaves the runtime and ends: no attach/detach churn (which raced with
    // the GC and crashed) and nothing left attached at shutdown (which made the game hang on exit).
    il2cpp_thread_detach(thread);
    Log("kurulum bitti, periyodik isler ana thread'de");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        char dir[MAX_PATH]; GetModuleFileNameA(nullptr, dir, MAX_PATH);
        // Other executables in the game folder (e.g. UnityCrashHandler64.exe) also load version.dll:
        // act as a plain forwarder there.
        const char* exe = strrchr(dir, '\\');
        if (!exe || _stricmp(exe + 1, "MoonSweepers.exe") != 0) return TRUE;
        if (char* s = strrchr(dir, '\\')) *s = 0;
        char logp[MAX_PATH]; snprintf(logp, MAX_PATH, "%s\\ConveyorMod.log", dir);
        g_log = fopen(logp, "w");
        Log("Moon Sweepers modu yuklendi (konveyor + joonas + bomba + aku)");
        LoadSettings(dir);
        CloseHandle(CreateThread(nullptr, 0, Worker, nullptr, 0, nullptr));
    }
    return TRUE;
}
