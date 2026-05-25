# tri v ryad

> match-3 game built on native win32 and gdi+ cuz modern engines r fat🥱🥱🥱🥱

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/gameplay.gif)

win10/11 | c++17 | cmake 3.16+

---

## what is this even

its just a classic match-3 puzzle game built on win32 apis. no unity, no unreal, NO PYTHON🙄🙄🙄🙄 just native gdi+ and c++.

* swap gems to make 3+ in a row
* gravity pulls everything down
* feed it custom pngs or let it draw colored circles when u r too lazy to setup textures

```
selected gem `[X: 2, Y: 3]` ↔ swap with `[X: 3, Y: 3]`
result: match found! gems go boom.
gravity pulls everything down, new gems fall.
```

---

## why its actually good

* **0% cpu idle:** when u r not clicking anything and animations r not running, the game just chill in da background
* **anticheat spawn:** board generator checks for matches on startup
* **saves settings:** check "сохранение настроек" and your grid size, quality, and paths r saved straight to registry.

### settings modal
change grid size, animation speed, or rendering quality from low-res nearest-neighbor to high-quality bicubic.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/images/settings_preview.jpg)

### vector fallback
no custom pngs? gdi+ will automatically draw smooth colored circles instead. no crashes, no missing textures.

![](https://raw.githubusercontent.com/homkee1/tri-Vryad/assets/readme/gifs/vector_fallback.gif)

---

## how it actually works

1. **no flickering:** game paints background and gems to a virtual bitmap in ram first, then slam it to screen using bitblt. 
2. **timer:** uses `QueryPerformanceCounter` to calculate elapsed time.
3. **rendering quality:** high quality setting tells gdi+ to use bicubic interpolation so your custom pngs look crisp.

---

## quick start

### build options

```bash
git clone https://github.com/homkee1/tri-Vryad.git
cd tri-Vryad
#compile
mkdir build && cd build
cmake ..
cmake --build . --config Release

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
  🥶🥶🥶
* **"why is screen black"**
  idk fr

* **"gems r just circles"**
  the game cant find your png folder. open settings and choose some folder which contains png files.

* **"settings didnt save"**
  check checkbox in settings(haha checkcheckbox lol) first. also check registry permissions.
```
