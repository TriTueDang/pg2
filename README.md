# Chicken Gun Story
**Chicken Gun Story** je 3D akční survival hra zasazená do prostředí divokého západu. Projekt byl vytvořen jako závěrečná práce pro předmět **PG2 (Počítačová grafika 2)**. Hra kombinuje pokročilé techniky vykreslování v OpenGL s akční hratelností a fyzikou.

## Hlavní vlastnosti

### Grafika a vizuální efekty
- **Skybox**: Realistické okolní prostředí tvořené kostkovou mapou.
- **Post-processing**: Systém využívající Framebuffery pro celoobrazovkové efekty (např. barevné filtry).
- **Billboarding**: Efektivní vykreslování vegetace (stepní běžci - tumbleweed) pomocí billboardingu.
- **Texturování**: Pokročilé materiály s podporou difúzních map a korekcí barev.
- **Multisampling (MSAA)**: Hardwarové vyhlazování hran pro čistší obraz.

### Kamera a animace
- **Kinematická kamera**: Úvodní průlet městem využívající **Catmull-Rom spliny** pro plynulý pohyb.
- **Third-Person kamera**: Hráčská kamera s vylepšenou logikou detekce kolizí s prostředím (kamera neprochází zdmi).
- **Interpolace**: Plynulé přechody mezi kinematickou sekvencí a samotnou hrou.

### Engine a Fyzika
- **BVH Kolize**: Prostředí je indexováno pomocí Bounding Volume Hierarchy pro extrémně rychlé kolizní dotazy.
- **Kinematic Character Controller (KCC)**: Robustní pohyb hráče včetně detekce schodů, tření a gravitace.
- **Projektilová fyzika**: Realistický let kulek a dynamitů s detekcí zásahů.

### Hratelnost a AI
- **Smarter AI State Machine**: Bandité se chovají inteligentně – pronásledují hráče, hledají krytí za překážkami a útočí (střelba, házení dynamitu).
- **Vlnový systém**: Postupné zvyšování obtížnosti s každou další vlnou nepřátel.
- **HUD (Head-Up Display)**: Integrované rozhraní pomocí Dear ImGui zobrazující zdraví, skóre, počet nepřátel a stav invulnerability.

## Ovládání

| Klávesa | Akce |
| :--- | :--- |
| **W, A, S, D** | Pohyb postavy |
| **Mezerník** | Skok |
| **Myš** | Rozhlížení |
| **Levé tlačítko myši** | Střelba |
| **TAB** | Uvolnění/zachycení kurzoru |
| **R** | Restart hry / Respawn |
| **V** | Přepnout VSync |
| **M** | Přepnout Antialiasing (MSAA) |
| **O** | Přepnout Post-processing efekty |
| **P** | Snímek obrazovky (Screenshot) |
| **F** | Přepnout do celoobrazovkového režimu |
| **G** | Skrýt/zobrazit herní rozhraní (HUD) |
| **ESC** | Ukončení hry |

## Technologie
- **Jazyk**: C++
- **Grafické API**: OpenGL 4.6
- **Knihovny**: GLFW (okna a vstup), GLEW (rozšíření), GLM (matematika), ImGui (UI), OpenCV (načítání textur), nlohmann/json (konfigurace).

## Jak spustit
1. Ujistěte se, že máte nainstalované ovladače s podporou OpenGL 4.6.
2. Sestavte projekt pomocí CMake.
3. Spusťte výsledný binární soubor. Pro rychlé spuštění na Linuxu lze použít `./start.sh`.

---
*Vytvořeno jako semestrální projekt pro PG2.*
# Splnění požadavků zkoušky (PG2)

Tento dokument mapuje požadavky zadání na konkrétní místa v kódu aplikace **Chicken Gun Story**.

## 1. ESSENTIALS (Základní požadavky)

### [x] 3D GL Core profile + shaders version 4.6
- **Kontext:** `app.cpp` (řádky 228–230) – Inicializace pomocí GLFW: `GLFW_OPENGL_CORE_PROFILE` a verze 4.6.
- **Shadery:** `shader.frag` (řádek 1) a další shadery používají `#version 460 core`.

### [x] GL debug enabled
- **Aktivace:** `app.cpp` (řádek 231) – Nastavení `GLFW_OPENGL_DEBUG_CONTEXT`.
- **Callback:** `app.cpp` (řádky 286–293) – Registrace `glDebugMessageCallback` ve funkci `init_gl_debug`.
- **Implementace:** `app.cpp` (řádky 48–75) – Funkce `gl_debug_callback` vypisuje chyby do konzole.

### [x] JSON config file
- **Soubor:** `config.json` v kořenovém adresáři.
- **Načítání:** `app.cpp` (řádky 170–189) – Implementace funkce `load_config` využívající knihovnu `nlohmann/json`.

### [x] High performance (min. 60 FPS)
- **Sledování:** FPS se zobrazuje v ImGui okně "Info" (`app.cpp`, řádek 535).
- **Optimalizace:** 
    - Použití **BVH** (Bounding Volume Hierarchy) pro fyziku (`Physics.cpp`).
    - **Instancování** banditů pro snížení počtu draw calls (`app.cpp`, řádky 1130–1152).
    - **Pre-kalkulace** názvů uniformů pro zamezení alokací v každém snímku (`app.cpp`, řádky 150–157).

### [x] VSync, Antialiasing, Fullscreen
- **VSync:** Ovládání klávesou **'V'** (`app.cpp`, řádek 1305).
- **Antialiasing (MSAA):** Aktivace `GL_MULTISAMPLE` (`app.cpp`, řádek 118), přepínání klávesou **'M'** (`app.cpp`, řádek 1333).
- **Fullscreen:** Přepínání klávesou **'F'** (`app.cpp`, řádek 1309).

### [x] Event processing (Mouse, Keyboard)
- **Klávesnice:** `app.cpp` (řádky 1278–1365) – `glfw_key_callback`.
- **Myš (pohyb):** `app.cpp` (řádky 260, 508) – `cursorPositionCallback`.
- **Myš (kolečko):** `app.cpp` (řádek 261) – `glfw_scroll_callback`.

### [x] Nezávisle se pohybující 3D modely
- **Načítání:** Využívá `Model.hpp` a `OBJloader.cpp`.
- **Modely:** Město (`western.obj`), Hráč (`Rango.obj`), Banditi (`Offensive Idle.obj`), Dynamit, Whiskey.
- **Pohyb:** Nezávislá AI logika pro bandity (`app.cpp`, řádky 744–800).

### [x] Custom shader effect
- **Post-processing:** `post_process.frag` – implementuje vinětaci, ACES tone mapping a efekt "zranění" (červené pulzování).

### [x] Minimálně 3 různé textury
- **Implementace:** `app.cpp` (řádky 332–338). Použito více než 6 unikátních textur (pro město, hráče, bandity, dynamit, atd.).

### [x] Lighting model (všechny typy)
- **Ambient:** Globální složka v `shader.frag` (řádek 100).
- **Directional:** Slunce v `app.cpp` (řádky 418–421), rotuje v `run` smyčce (řádek 1029).
- **Point:** Dvě světla v `app.cpp` (řádky 424–436), jedno obíhá scénu (řádek 1034).
- **Reflector (Spotlight):** "Čelovka" připojená ke kameře (`app.cpp`, řádky 439–447 a 1039–1040).

### [x] Alpha scale transparency
- **Nastavení:** `app.cpp` (řádky 121–122) – Zapnutý `GL_BLEND` a `glBlendFunc`.
- **Využití:** Průhledné billboardy (tumbleweeds) a částicový systém.

### [x] Correct collisions
- **Implementace:** `Physics.cpp` / `Physics.hpp`. Používá robustní **KCC** (Kinematic Character Controller) s detekcí kolizí proti BVH stromu scény.

---

## 2. EXTRAS (Bonusy)

### [x] Particle effects
- **Implementace:** `app.cpp` (řádky 663–673 – update, `render_particles` – vykreslení) a shader `particle.frag`.

### [x] Height map (Ground snapping)
- **Funkce:** `physics.get_ground_height` v `Physics.cpp`.
- **Využití:** Automatické přichytávání hráče a banditů k nerovnostem terénu (`app.cpp`, řádky 515 a 748).

### [x] Scripting / AI
- **AI Logic:** `app.cpp` (řádky 754–800) – Stavový automat banditů (Chase -> Seek Cover -> Shoot).

### [x] Další efekty
- **Cinematic Camera:** Filmový průlet městem na začátku hry pomocí Catmull-Rom spline (`app.cpp`, řádky 453–464).
- **MSA Resolve:** Manuální resolve multisamplingu v post-process shaderu pro vyšší kvalitu.

---

## 3. INSTAFAIL CHECK
- **GLUT:** Nepoužito (nahrazeno moderním GLFW).
- **Core profile:** Striktně dodržen.
- **DSA (Direct State Access):** Použito (např. `glCreateBuffers`, `glNamedBufferData`, `glCreateVertexArrays`).
