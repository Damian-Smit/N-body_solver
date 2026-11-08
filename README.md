# Gravity Simulator

An N-body gravity simulator written in C, rendered in real time with SDL2.
Forces are approximated with a Barnes-Hut algorithm, using a linear BH tree
(built over a Morton-code sort) for the acceleration structure.

## Features

-Barnes-Hut approximation (`theta` configurable) for O(n log n) force
calculation
- Kahan summation in the integrator for numerical stability over long runs
- SDL2 real-time rendering with pan/zoom camera controls
- Support for Stars, planets, moons and procedurally generated asteroid belts
- Included setups: real Solar System and a 1000 and 2000 body mock galaxy

## Dependencies

- `gcc` (C11)
- `make`
-  SDL2 (`libsdl2-dev` on Debian/Ubuntu, `sdl2` via Homebrew on macOS)

```bash
# debian / Ubuntu
sudo apt install build-essential libsdl2-dev

# macOS (Homebrew)
brew install sdl2
```

## Building
``` bash
make
```

## Running

```bash
make run                             #defaults to setups/solar_system.txt and trails off
make run FILE=setups/mock_galaxy_1000.txt       #to run a specific setup file
make run TRAILS=true                       #to turn trails on for planets
./gravitySimulator mock_galaxy_1000.txt
```

## Controls

| Key           | Action     |
|---------------|------------|
| Arrow Keys/ wasd    | Pan Camera |
| `+` / Keypad+ | Zoom In    |
| `-` / Keypad- | Zoom Out   |
| `Spacebar` | Pause/Resume   |
|  `R` |  Reset Camera |
| `F` / `F11` | Fullscreen  |
| `Q` | Close  |
| `[` | Halve Sub steps   |
| `]` | Double Sub steps   |
| `TAB` | Advance Follow Camera  |
| `SHIFT+TAB` | Regress Follow Camera  |


## Project Structure

```
.
├── Makefile
├── src/
│   ├── gravitySimulator.c   # Entry point and main loop
│   ├── linearBH.c / .h      # flat Barnes-Hut tree (Morton-code sort based)
│   ├── hud.c / .h           # display of simulation metrics
│   ├── render.c / .h        # rendering of bodies
│   ├── setup.c / .h         # reads input file and creates objects
│   ├── camera.c / .h        # camera control functions
│   ├── input.c / .h         # input control manager
│   ├── bodyStore.h          # struct-of-arrays body/star data layout
│   ├── constants.h          # defined physics constants used throughout
│   └── legacy/
|       ├── vec.c / vec.h    # 3D vector math (legacy, require for octree.c)
│       └── octree.c / .h    # earlier pointer-based octree implementation,
│                             # kept for reference; not part of the current build
└─── setups/                  # input scenes (see format below)
```
> `src/legacy/octree.c` is an earlier, pointer-based Barnes-Hut tree that predates
> `linearBH.c`. It's no longer wired into the Makefile or included from
> `gravitySimulator.c`, but is kept around since it's a clearer (if slower) reference
> implementation of the same algorithm.

## Input file format

Setup files are plain text, one body per line. Lines starting with `//` are comments.

**Units:** position in AU, velocity in AU/year, mass in solar masses, time step (`dt`) in years.
```
# planets, moons and stars
type position velocity acceleration mass radius colours(r g b)

# asteroids (procedurally generated ring)
type amount central_body_idx min_distance_from_central_body max_distance_from_central_body colours(r g b)
```
Types: `s` = star, `p` = planet, `m` = moon, `a` = asteroid.

Example 1 (the Sun):

```
s  0 0 0   0 0 0   0 0 0   1   10   255 244 214
```

Example 2 (Earth):

```
p -0.220189 0.958319 0.000000   -6.226820 -1.430708 -0.000000   0 0 0   3e-06   4.2587e-5   70 130 220
``` 


## License

MIT — see [LICENSE](LICENSE).
