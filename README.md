# tri v ryad

> match-3 game built on native win32 and gdi+. cuz modern engines r fat and bloated.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/gameplay.gif)

win10/11 | c++17 | cmake 3.16+

---

## what is this even

its just a classic match-3 puzzle game built on bare-metal win32 apis. no unity, no unreal, no 100gb installs. just native gdi+ and c++17.

* swap gems to make 3+ in a row
* gems go boom, gravity pulls everything down
* feed it custom pngs or let it draw colored circles when u r too lazy to setup textures

```
selected gem `[X: 2, Y: 3]` ↔ swap with `[X: 3, Y: 3]`
result: match found! gems go boom.
gravity pulls everything down, new gems fall.
```

---

## why its actually good

* **0% cpu idle:** when u r not clicking anything and animations r not running, the game just chill-ticks in the background. saves your laptop battery.
* **anti-cheat spawn:** board generator checks for matches on startup, so u dont get free wins on spawn.
* **no laggy freezes:** loading custom folders of pngs runs in a background thread, so your window doesnt crash or white-out.
* **saves settings:** check "сохранение настроек" and your grid size, quality, and paths r saved straight to registry.

### settings modal
change grid size, animation speed, or rendering quality from low-res nearest-neighbor to high-quality bicubic.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/images/settings_preview.jpg)

### vector fallback
no custom pngs? gdi+ will automatically draw smooth colored circles instead. no crashes, no missing textures.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/vector_fallback.gif)

---

## how it actually works

1. **no flickering:** we paint background and gems to a virtual bitmap in ram first, then slam it to screen using `BitBlt`. 
2. **timer:** uses `QueryPerformanceCounter` to calculate elapsed time. variable `SpeedAnimationA4ChangePos` has a typo in the middle. dont touch it or gems will fly away.
3. **rendering quality:** high quality setting tells gdi+ to use bicubic interpolation so your custom pngs look crisp.

---

## quick start

### build options

```bash
# clone the repo
git clone https://github.com/homkee1/tri-Vryad.git
cd tri-Vryad

# generate build files and compile
mkdir build && cd build
cmake ..
cmake --build . --config Release

# run
./Release/TriVRyad.exe
```

### build with mingw
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

---

## its broken now what

* **"i set grid to 99999 and the game crashed"**
  yeah, u tried to allocate billions of smart pointers in a 2d vector. your ram died, windows said no. keep grid under 50 buddy.

* **"why is screen black"**
  u probably minimized the window or resized it to 0x0. 

* **"gems r just circles"**
  the game cant find your png folder. put some square pngs into a folder named `png` next to exe or click "обзор" and pick it.

* **"settings didnt save"**
  check "сохранение настроек" checkbox first. also check registry permissions.

---

*made with win32 raw pointers and a lot of patience*
```
