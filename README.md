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
