// Localized texts for everything the mod adds to the UI.
// Language is taken from the game's own MoonLoc.Code; unknown languages fall back to English.
// Effect strings take the multiplier (%g); the bomb label takes the formatted price (%s).
#pragma once

enum TextId {
    T_BELT_TITLE, T_BELT_SUMMARY, T_BELT_EFFECT, T_BELT_EFFECT_NO_OUTPUT,
    T_JOONAS_TITLE, T_JOONAS_SUMMARY, T_JOONAS_EFFECT,
    T_CHAMBER_TITLE, T_CHAMBER_SUMMARY, T_CHAMBER_EFFECT,
    T_PRESS_TITLE, T_PRESS_SUMMARY, T_PRESS_EFFECT,
    T_BRANCH_NAME, T_BRANCH_TAGLINE, T_BOMB,
    T_COUNT
};

struct LangTexts { const char* code; const char* t[T_COUNT]; };

static const LangTexts kLangs[] = {
    {"en", {
        "Fast Conveyors", "Uprated belt motors: every conveyor and machine output moves capsules faster.",
        "%gx conveyor and output speed", "%gx conveyor speed",
        "Turbo Joonas", "High-revving motors for Joonas diggers: they travel and clean faster.", "%gx Joonas speed",
        "Fast Pressure Chamber", "High-pressure pumps: the pressure chamber processes each batch in less time.", "%gx pressure chamber speed",
        "Fast Stone Press", "Reinforced hydraulics: the stone press forms blocks faster.", "%gx stone press speed",
        "PRODUCTION", "Pressure chambers and stone presses work faster.", "Bomb (%s)"}},
    {"tr", {
        "Hızlı Konveyör", "Güçlendirilmiş bant motorları: tüm konveyörler ve çıkışlar kapsülleri daha hızlı taşır.",
        "%gx konveyör ve çıkış hızı", "%gx konveyör hızı",
        "Turbo Joonas", "Joonas kazıcılarına yüksek devirli motorlar: daha hızlı hareket eder ve temizler.", "%gx Joonas hızı",
        "Hızlı Basınç Odası", "Yüksek basınçlı pompalar: basınç odası her partiyi daha kısa sürede işler.", "%gx basınç odası üretim hızı",
        "Hızlı Taş Presi", "Güçlendirilmiş hidrolik: taş presi blokları daha hızlı basar.", "%gx taş presi üretim hızı",
        "ÜRETİM", "Basınç odası ve taş presi daha hızlı üretir.", "Bombala (%s)"}},
    {"de", {
        "Schnelle Förderbänder", "Verstärkte Bandmotoren: Alle Förderbänder und Maschinenausgänge transportieren Kapseln schneller.",
        "%gx Förderband- und Ausgabegeschwindigkeit", "%gx Förderbandgeschwindigkeit",
        "Turbo-Joonas", "Hochdrehende Motoren für Joonas-Gräber: Sie fahren und reinigen schneller.", "%gx Joonas-Geschwindigkeit",
        "Schnelle Druckkammer", "Hochdruckpumpen: Die Druckkammer verarbeitet jede Charge schneller.", "%gx Druckkammer-Geschwindigkeit",
        "Schnelle Steinpresse", "Verstärkte Hydraulik: Die Steinpresse formt Blöcke schneller.", "%gx Steinpressen-Geschwindigkeit",
        "PRODUKTION", "Druckkammern und Steinpressen arbeiten schneller.", "Bombardieren (%s)"}},
    {"fr", {
        "Convoyeurs rapides", "Moteurs de tapis renforcés : tous les convoyeurs et sorties de machines déplacent les capsules plus vite.",
        "%gx vitesse des convoyeurs et des sorties", "%gx vitesse des convoyeurs",
        "Joonas Turbo", "Moteurs à haut régime pour les foreuses Joonas : elles se déplacent et nettoient plus vite.", "%gx vitesse de Joonas",
        "Chambre de pression rapide", "Pompes haute pression : la chambre de pression traite chaque lot plus vite.", "%gx vitesse de la chambre de pression",
        "Presse à pierre rapide", "Hydraulique renforcée : la presse à pierre forme les blocs plus vite.", "%gx vitesse de la presse à pierre",
        "PRODUCTION", "Les chambres de pression et presses à pierre travaillent plus vite.", "Bombarder (%s)"}},
    {"es", {
        "Cintas rápidas", "Motores de cinta mejorados: todas las cintas y salidas de máquinas mueven las cápsulas más rápido.",
        "%gx velocidad de cintas y salidas", "%gx velocidad de cintas",
        "Joonas Turbo", "Motores de altas revoluciones para las excavadoras Joonas: se desplazan y limpian más rápido.", "%gx velocidad de Joonas",
        "Cámara de presión rápida", "Bombas de alta presión: la cámara de presión procesa cada lote en menos tiempo.", "%gx velocidad de la cámara de presión",
        "Prensa de piedra rápida", "Hidráulica reforzada: la prensa de piedra forma bloques más rápido.", "%gx velocidad de la prensa de piedra",
        "PRODUCCIÓN", "Las cámaras de presión y prensas de piedra trabajan más rápido.", "Bombardear (%s)"}},
    {"it", {
        "Nastri veloci", "Motori dei nastri potenziati: tutti i nastri e le uscite delle macchine spostano le capsule più velocemente.",
        "%gx velocità di nastri e uscite", "%gx velocità dei nastri",
        "Joonas Turbo", "Motori ad alti giri per gli scavatori Joonas: si muovono e puliscono più velocemente.", "%gx velocità di Joonas",
        "Camera di pressione veloce", "Pompe ad alta pressione: la camera di pressione lavora ogni lotto in meno tempo.", "%gx velocità della camera di pressione",
        "Pressa per pietra veloce", "Idraulica rinforzata: la pressa per pietra forma i blocchi più velocemente.", "%gx velocità della pressa per pietra",
        "PRODUZIONE", "Camere di pressione e presse per pietra lavorano più velocemente.", "Bombarda (%s)"}},
    {"pt", {
        "Esteiras rápidas", "Motores de esteira reforçados: todas as esteiras e saídas de máquinas movem as cápsulas mais rápido.",
        "%gx velocidade de esteiras e saídas", "%gx velocidade das esteiras",
        "Joonas Turbo", "Motores de alta rotação para as escavadoras Joonas: deslocam-se e limpam mais rápido.", "%gx velocidade do Joonas",
        "Câmara de pressão rápida", "Bombas de alta pressão: a câmara de pressão processa cada lote em menos tempo.", "%gx velocidade da câmara de pressão",
        "Prensa de pedra rápida", "Hidráulica reforçada: a prensa de pedra forma blocos mais rápido.", "%gx velocidade da prensa de pedra",
        "PRODUÇÃO", "Câmaras de pressão e prensas de pedra trabalham mais rápido.", "Bombardear (%s)"}},
    {"pl", {
        "Szybkie przenośniki", "Wzmocnione silniki taśm: wszystkie przenośniki i wyjścia maszyn szybciej przesuwają kapsuły.",
        "%gx prędkość przenośników i wyjść", "%gx prędkość przenośników",
        "Turbo Joonas", "Wysokoobrotowe silniki dla koparek Joonas: szybciej jeżdżą i sprzątają.", "%gx prędkość Joonas",
        "Szybka komora ciśnieniowa", "Pompy wysokociśnieniowe: komora ciśnieniowa przetwarza każdą partię szybciej.", "%gx prędkość komory ciśnieniowej",
        "Szybka prasa do kamienia", "Wzmocniona hydraulika: prasa do kamienia szybciej formuje bloki.", "%gx prędkość prasy do kamienia",
        "PRODUKCJA", "Komory ciśnieniowe i prasy do kamienia pracują szybciej.", "Zbombarduj (%s)"}},
    {"ru", {
        "Быстрые конвейеры", "Усиленные моторы лент: все конвейеры и выходы машин перемещают капсулы быстрее.",
        "%gx скорость конвейеров и выходов", "%gx скорость конвейеров",
        "Турбо Joonas", "Высокооборотные моторы для копателей Joonas: они быстрее ездят и чистят.", "%gx скорость Joonas",
        "Быстрая камера давления", "Насосы высокого давления: камера давления обрабатывает каждую партию быстрее.", "%gx скорость камеры давления",
        "Быстрый каменный пресс", "Усиленная гидравлика: каменный пресс быстрее формирует блоки.", "%gx скорость каменного пресса",
        "ПРОИЗВОДСТВО", "Камеры давления и каменные прессы работают быстрее.", "Бомбить (%s)"}},
    {"uk", {
        "Швидкі конвеєри", "Посилені мотори стрічок: усі конвеєри та виходи машин переміщують капсули швидше.",
        "%gx швидкість конвеєрів і виходів", "%gx швидкість конвеєрів",
        "Турбо Joonas", "Високообертові мотори для копачів Joonas: вони швидше їздять і чистять.", "%gx швидкість Joonas",
        "Швидка камера тиску", "Насоси високого тиску: камера тиску обробляє кожну партію швидше.", "%gx швидкість камери тиску",
        "Швидкий кам'яний прес", "Посилена гідравліка: кам'яний прес швидше формує блоки.", "%gx швидкість кам'яного преса",
        "ВИРОБНИЦТВО", "Камери тиску та кам'яні преси працюють швидше.", "Бомбардувати (%s)"}},
    {"ja", {
        "高速コンベア", "強化ベルトモーター：すべてのコンベアと機械の出力がカプセルをより速く運びます。",
        "コンベアと出力の速度 %gx", "コンベア速度 %gx",
        "ターボ Joonas", "Joonas 掘削機用の高回転モーター：移動と清掃が速くなります。", "Joonas の速度 %gx",
        "高速加圧室", "高圧ポンプ：加圧室が各バッチをより短時間で処理します。", "加圧室の速度 %gx",
        "高速ストーンプレス", "強化油圧：ストーンプレスがブロックをより速く成形します。", "ストーンプレスの速度 %gx",
        "生産", "加圧室とストーンプレスがより速く稼働します。", "爆破 (%s)"}},
    {"ko", {
        "고속 컨베이어", "강화된 벨트 모터: 모든 컨베이어와 기계 출력이 캡슐을 더 빠르게 옮깁니다.",
        "컨베이어 및 출력 속도 %gx", "컨베이어 속도 %gx",
        "터보 Joonas", "Joonas 굴착기용 고회전 모터: 더 빠르게 이동하고 청소합니다.", "Joonas 속도 %gx",
        "고속 압력실", "고압 펌프: 압력실이 각 배치를 더 짧은 시간에 처리합니다.", "압력실 속도 %gx",
        "고속 석재 프레스", "강화 유압: 석재 프레스가 블록을 더 빠르게 성형합니다.", "석재 프레스 속도 %gx",
        "생산", "압력실과 석재 프레스가 더 빠르게 작동합니다.", "폭파 (%s)"}},
    {"zh-Hans", {
        "高速传送带", "强化皮带电机：所有传送带和机器输出都能更快地运送胶囊。",
        "传送带与输出速度 %gx", "传送带速度 %gx",
        "涡轮 Joonas", "为 Joonas 挖掘机配备高转速电机：移动和清理更快。", "Joonas 速度 %gx",
        "高速压力舱", "高压泵：压力舱能更快地处理每一批次。", "压力舱速度 %gx",
        "高速石材压机", "强化液压：石材压机更快地压制石块。", "石材压机速度 %gx",
        "生产", "压力舱和石材压机运转更快。", "轰炸 (%s)"}},
    {"zh-Hant", {
        "高速輸送帶", "強化皮帶馬達：所有輸送帶和機器輸出都能更快地運送膠囊。",
        "輸送帶與輸出速度 %gx", "輸送帶速度 %gx",
        "渦輪 Joonas", "為 Joonas 挖掘機配備高轉速馬達：移動和清理更快。", "Joonas 速度 %gx",
        "高速壓力艙", "高壓泵：壓力艙能更快地處理每一批次。", "壓力艙速度 %gx",
        "高速石材壓機", "強化液壓：石材壓機更快地壓製石塊。", "石材壓機速度 %gx",
        "生產", "壓力艙和石材壓機運轉更快。", "轟炸 (%s)"}},
    {"sv", {
        "Snabba transportband", "Förstärkta bandmotorer: alla transportband och maskinutgångar flyttar kapslar snabbare.",
        "%gx hastighet för band och utgångar", "%gx bandhastighet",
        "Turbo-Joonas", "Högvarviga motorer för Joonas-grävare: de färdas och städar snabbare.", "%gx Joonas-hastighet",
        "Snabb tryckkammare", "Högtryckspumpar: tryckkammaren bearbetar varje sats snabbare.", "%gx tryckkammarhastighet",
        "Snabb stenpress", "Förstärkt hydraulik: stenpressen formar block snabbare.", "%gx stenpresshastighet",
        "PRODUKTION", "Tryckkammare och stenpressar arbetar snabbare.", "Bomba (%s)"}},
    {"fi", {
        "Nopeat kuljettimet", "Tehostetut hihnamoottorit: kaikki kuljettimet ja koneiden ulostulot siirtävät kapseleita nopeammin.",
        "%gx kuljettimien ja ulostulojen nopeus", "%gx kuljettimien nopeus",
        "Turbo-Joonas", "Korkeakierroksiset moottorit Joonas-kaivureille: ne liikkuvat ja siivoavat nopeammin.", "%gx Joonasin nopeus",
        "Nopea painekammio", "Korkeapainepumput: painekammio käsittelee jokaisen erän nopeammin.", "%gx painekammion nopeus",
        "Nopea kivipuristin", "Vahvistettu hydrauliikka: kivipuristin muovaa harkot nopeammin.", "%gx kivipuristimen nopeus",
        "TUOTANTO", "Painekammiot ja kivipuristimet toimivat nopeammin.", "Pommita (%s)"}},
    {"da", {
        "Hurtige transportbånd", "Forstærkede båndmotorer: alle transportbånd og maskinudgange flytter kapsler hurtigere.",
        "%gx hastighed for bånd og udgange", "%gx båndhastighed",
        "Turbo-Joonas", "Højtomdrejende motorer til Joonas-gravere: de kører og rengør hurtigere.", "%gx Joonas-hastighed",
        "Hurtigt trykkammer", "Højtrykspumper: trykkammeret behandler hver batch hurtigere.", "%gx trykkammerhastighed",
        "Hurtig stenpresse", "Forstærket hydraulik: stenpressen former blokke hurtigere.", "%gx stenpressehastighed",
        "PRODUKTION", "Trykkamre og stenpresser arbejder hurtigere.", "Bomb (%s)"}},
    {"no", {
        "Raske transportbånd", "Forsterkede båndmotorer: alle transportbånd og maskinutganger flytter kapsler raskere.",
        "%gx hastighet for bånd og utganger", "%gx båndhastighet",
        "Turbo-Joonas", "Høyturtallsmotorer for Joonas-gravere: de kjører og rengjør raskere.", "%gx Joonas-hastighet",
        "Raskt trykkammer", "Høytrykkspumper: trykkammeret behandler hver batch raskere.", "%gx trykkammerhastighet",
        "Rask steinpresse", "Forsterket hydraulikk: steinpressen former blokker raskere.", "%gx steinpressehastighet",
        "PRODUKSJON", "Trykkamre og steinpresser jobber raskere.", "Bomb (%s)"}},
    {"nl", {
        "Snelle transportbanden", "Versterkte bandmotoren: alle transportbanden en machine-uitgangen verplaatsen capsules sneller.",
        "%gx snelheid van banden en uitgangen", "%gx bandsnelheid",
        "Turbo Joonas", "Hoogtoerige motoren voor Joonas-graafmachines: ze rijden en reinigen sneller.", "%gx Joonas-snelheid",
        "Snelle drukkamer", "Hogedrukpompen: de drukkamer verwerkt elke partij sneller.", "%gx drukkamersnelheid",
        "Snelle steenpers", "Versterkte hydrauliek: de steenpers vormt blokken sneller.", "%gx steenperssnelheid",
        "PRODUCTIE", "Drukkamers en steenpersen werken sneller.", "Bombarderen (%s)"}},
    {"cs", {
        "Rychlé dopravníky", "Posílené motory pásů: všechny dopravníky a výstupy strojů přesouvají kapsle rychleji.",
        "%gx rychlost dopravníků a výstupů", "%gx rychlost dopravníků",
        "Turbo Joonas", "Vysokootáčkové motory pro rypadla Joonas: jezdí a čistí rychleji.", "%gx rychlost Joonas",
        "Rychlá tlaková komora", "Vysokotlaká čerpadla: tlaková komora zpracuje každou dávku rychleji.", "%gx rychlost tlakové komory",
        "Rychlý lis na kámen", "Zesílená hydraulika: lis na kámen tvaruje bloky rychleji.", "%gx rychlost lisu na kámen",
        "VÝROBA", "Tlakové komory a lisy na kámen pracují rychleji.", "Bombardovat (%s)"}},
};
static const int kLangCount = sizeof kLangs / sizeof kLangs[0];

// The game uses Steam language names as codes ("turkish", "schinese", "koreana", "latam", ...).
static const struct { const char* steam; const char* lang; } kSteamLangs[] = {
    {"english", "en"}, {"turkish", "tr"}, {"german", "de"}, {"french", "fr"}, {"spanish", "es"}, {"latam", "es"},
    {"italian", "it"}, {"brazilian", "pt"}, {"portuguese", "pt"}, {"polish", "pl"}, {"russian", "ru"},
    {"ukrainian", "uk"}, {"japanese", "ja"}, {"koreana", "ko"}, {"korean", "ko"}, {"schinese", "zh-Hans"},
    {"tchinese", "zh-Hant"}, {"swedish", "sv"}, {"finnish", "fi"}, {"danish", "da"}, {"norwegian", "no"},
    {"dutch", "nl"}, {"czech", "cs"},
};

// Map a game language code (Steam name, or ISO like "tr", "pt-BR", "zh-Hant", "nb") to an index in kLangs.
static int LangIndexFor(const char* code) {
    if (!code || !*code) return 0;
    char c[16] = {0};
    for (int i = 0; i < 15 && code[i]; i++) c[i] = (char)tolower((unsigned char)code[i]);
    for (const auto& s : kSteamLangs)
        if (!strcmp(c, s.steam))
            for (int i = 0; i < kLangCount; i++) if (!strcmp(kLangs[i].code, s.lang)) return i;
    if (!strncmp(c, "zh", 2)) {
        bool hant = strstr(c, "hant") || strstr(c, "tw") || strstr(c, "hk") || strstr(c, "mo");
        for (int i = 0; i < kLangCount; i++) if (!strcmp(kLangs[i].code, hant ? "zh-Hant" : "zh-Hans")) return i;
    }
    if (!strncmp(c, "nb", 2) || !strncmp(c, "nn", 2)) { c[0] = 'n'; c[1] = 'o'; c[2] = 0; }
    char base[8] = {0};
    for (int i = 0; i < 7 && c[i] && c[i] != '-' && c[i] != '_'; i++) base[i] = c[i];
    for (int i = 0; i < kLangCount; i++) if (!strcmp(kLangs[i].code, base)) return i;
    return 0;
}
