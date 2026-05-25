---

```markdown
# tri v ryad

> match-3 game built on native win32 and gdi+. because modern engines are bloated.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/gameplay.gif)

* **OS:** Windows 10/11
* **Language:** C++17
* **Build System:** CMake 3.16+
* **License:** MIT

---

## What is this?

A basic match-3 puzzle game built entirely on bare-metal Windows APIs. No Unity, no Unreal, no massive runtimes. Just native GDI+ and C++17.

* Swap gems to match 3 or more in a row.
* Matches trigger chain reactions and board state updates.
* Supports custom PNG textures, with an automatic fallback to vector shapes if no images are loaded.

```
selected gem `[X: 2, Y: 3]` ↔ swap with `[X: 3, Y: 3]`
result: match found! gems go boom.
gravity pulls everything down, new gems fall.
```

---

## Key Features

* **No idle CPU usage:** When nothing is moving or updating, the game suspends active rendering to save laptop battery.
* **Fair spawn logic:** The board generator ensures no pre-existing matches are present when a new game starts.
* **Asynchronous loading:** Loading custom PNG directories runs in a background thread to prevent the UI from freezing.
* **Registry-based configuration:** If enabled, settings like grid size, rendering quality, and paths are saved to the Windows Registry.

### Settings
Adjust grid sizes, animation speeds, or rendering quality (from low-res nearest-neighbor to high-quality bicubic interpolation).

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/images/settings_preview.jpg)

### Vector Fallback
If no custom textures are specified, GDI+ renders anti-aliased geometric circles instead.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/vector_fallback.gif)

---

## Under the Hood

1. **Double Buffering:** To prevent window flickering, everything is rendered to an in-memory bitmap first, then copied to the screen in a single `BitBlt` call.
2. **Animation Timing:** Uses `QueryPerformanceCounter` for high-precision frame delta calculations. (Note: `SpeedAnimationA4ChangePos` has a typo in the name—do not rename it, or the physics might break).
3. **Quality Scaling:** High-quality rendering mode forces GDI+ to use bicubic interpolation for custom textures.

---

## Quick Start

### Prerequisites
* Windows 10/11
* CMake 3.16+
* MSVC or MinGW (C++17 compatible)

```bash
# Clone the repository
git clone https://github.com/homkee1/tri-Vryad.git
cd tri-Vryad

# Build
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Run
./Release/TriVRyad.exe
```

### Building with MinGW
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

---

## Troubleshooting

* **Crash on huge grids (e.g. 9999x9999):** You attempted to allocate a massive 2D vector of smart pointers. Keep the grid size under 50.
* **Black screen:** The window was minimized or resized to 0x0.
* **Gems are just circles:** The executable cannot locate your images. Place your custom textures in a folder named `png` next to the executable, or click "Обзор" in the settings to set the path manually.
* **Settings are not saving:** Ensure the "Сохранение настроек" checkbox is ticked and the process has sufficient permissions to write to the registry.

---

*Written in raw Win32, C++, and pain.*
```
