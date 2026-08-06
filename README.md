# Gavity Simulator

An N-body gravity simulator written in C, rendered in real time with SDL2.
Forces are approximated with a Barnes-Hut algorithm, using a linear BH tree
(built over a Morton-code sort) for the acceleration structure.

## Features

-Barnes-Hut approximation (`theta` configurable) for O(n log n) force
calculation
- Kahan summation in the integrator for numerical stability over long runs
- SDL2 real-time rendering with pan/zoom camera controls
- Support for Stars, planets, moons and procedurally generated asteroid belts
- Included setups: real Solar System and a synthetic base system

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
make run                      #defaults to setups/solar_system.txt
make run FILE=setups/base.txt
./gravitySimulator setups/base,txt
```

## Controls

| Key           | Action     |
|---------------|------------|
| Arrow Keys    | Pan Camera |
| `+` / Keypad+ | Zoom In    |
| `-` / Keypad- | Zoom Out   |

## Project Structure

```
.
├── Makefile
├── src/
│   ├── gravitySimulator.c   # entry point: setup, integration loop, SDL rendering
│   ├── vec.c / vec.h        # 3D vector math
│   ├── linearBH.c / .h      # flat Barnes-Hut tree (Morton-code sort based)
│   ├── bodyStore.h          # struct-of-arrays body/star data layout
│   └── legacy/
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
# planets and moons and stars
type position velocity acceleration mass radius colours(r g b)

# asteroids (procedurally generated ring)
type amount central_body_idx min_distance_from_central_body max_distance_from_central_body colours(r g b)
```
Types: `s` = star, `p` = planet, `m` = moon, `a` = asteroid.

Example (the Sun):

```
s 0 0 0   0 0 0   0 0 0   1   10   255 244 214
```

## License

MIT — see [LICENSE](LICENSE).
